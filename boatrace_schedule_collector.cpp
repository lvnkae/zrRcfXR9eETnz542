/*!
 *  @file   boatrace_schedule_collector.cpp
 *  @brief  競艇データ収集：開催スケジュールデータ収集
 *  @date   2018/03/07
 */
#include "boatrace_schedule_collector.h"

#include "boatrace_data.h"
#include "boatrace_data_utility.h"
#include "environment.h"

#include "utility/utility_datetime.h"
#include "utility/utility_http.h"
#include "utility/utility_python.h"
#include "utility/utility_string.h"
#include "yymmdd.h"

#include "cpprest/http_client.h"
#include "cpprest/filestream.h"
#include <mutex>
#include <unordered_map>

namespace boatrace
{

/*!
 *
 */
class ScheduleCollector::PIMPL
{
private:
    enum eSequence
    {
        SEQ_ERROR,      //!< エラー停止

        SEQ_INITIALIZE, //!< 初期化
        SEQ_MAIN,       //!< 主処理
        SEQ_COMPLETE,   //!< 正常終了
    };

    std::recursive_mutex m_mtx; //!< 排他制御子
    eSequence m_sequence;       //!< シーケンス
    bool m_wait_response;       //!< 応答待ちフラグ
    int64_t m_last_rcv_tick;    //!< 最終応答受信時刻(tickCount)

    boost::python::api::object m_python;    //!< pythonスクリプトオブジェクト

    const garnet::YYMM m_start_ym;  //!< 収集開始年月
    const garnet::YYMM m_end_ym;    //!< 収集終了年月
    garnet::YYMM m_next_req_ym;     //!< 次に要求する年月

    //!< レース固有ID発行元
    uint32_t m_race_id_issue;
    //!< レース名テーブル
    BoatraceNames m_race_name;
    //!< 開催スケジュールテーブル<年月文字列, 一か月分のスケジュール>
    std::unordered_map<std::string, ScheduleAtMonth> m_schedule;

    PIMPL(const PIMPL&);
    PIMPL& operator= (const PIMPL&);

    /*!
     *  @brief  レース固有ID発行
     *  @param[in]  mon_rc_id       レースID(月ローカル)
     *  @param[in]  mon_race_name   レース名テーブル(月ローカル)
     *  @param[out] mon_race_id_map レースID対応表(月ローカル)
     *  @return 固有レースID
     */ 
    uint32_t IssueRaceID(int32_t mon_rc_id,
                         const boost::python::list& mon_race_name,
                         std::unordered_map<int32_t, uint32_t>& mon_race_id_map)
    {
        const uint32_t rc_id = ++m_race_id_issue;
        m_race_name.entry(rc_id,
            boost::python::extract<std::string>(mon_race_name[mon_rc_id-1]));
        mon_race_id_map.emplace(mon_rc_id, rc_id);
        return rc_id;
    }

    /*!
     *  @brief  (適合する)発行済レース固有IDを探す
     *  @param  yymm_str        今月(文字列)
     *  @param  prev_yymm_str   前月(文字列）
     *  @param  st_id           レース場ID
     *  @param  mon_rc_id       レースID(月ローカル)
     *  @param  mon_race_name   レース名テーブル(月ローカル)
     *  @param  mon_race_id_map レースID対応表(月ローカル)
     *  @return 固有レースID
     */ 
    uint32_t SearchIssuedRaceID(const std::string& yymm_str,
                                const std::string& prev_yymm_str,
                                eStadiumID st_id,
                                int32_t mon_rc_id,
                                const boost::python::list& mon_race_name,
                                std::unordered_map<int32_t, uint32_t>& mon_race_id_map)
    {
        const auto it_id = mon_race_id_map.find(mon_rc_id);
        if (it_id != mon_race_id_map.end()) {
            // 当月対応表にあればそのまま使用
            return it_id->second;
        } else {
            const auto it_prev_mon = m_schedule.find(prev_yymm_str);
            if (it_prev_mon != m_schedule.end()) {
                // 月跨ぎ → 前月最終レースのIDを使用
                const sRaceAtDay last_race(std::move(it_prev_mon->second.get_last_race(st_id)));
                if (!last_race.empty()) {
                    return last_race.m_id;
                } else {
                    PRINT_MESSAGE("[error] fail to get last race : " + yymm_str
                                + " : " + StadiumIDToStr(st_id));
                }
            } else if (m_schedule.empty()) {
                // 月跨ぎ → 初日ではないが新規発行(収集初月限定処理)
                PRINT_MESSAGE("first month : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
                return IssueRaceID(mon_rc_id, mon_race_name, mon_race_id_map);
            } else {
                // 前月データがない(error)
                PRINT_MESSAGE("not found prev month data : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            }
        }
        return BlankRaceID();
    }

    /*!
     *  @brief  開催n日目補正
     *  @param  yymm_str        今月(文字列)
     *  @param  prev_yymm_str   前月(文字列）
     *  @param  lc_days         補正前データ
     *  @param  st_id           レース場ID
     *  @param  sam             当月スケジュールデータ
     */
    int32_t CorrectRaceDays(const std::string& yymm_str,
                            const std::string& prev_yymm_str,
                            int32_t lc_days,
                            eStadiumID st_id,
                            const ScheduleAtMonth& sam)
    {
        if (sam.empty(st_id)) {
            // 対象レース場の当月データが空ならば前月末+1を返す
            const auto it_prev_mon = m_schedule.find(prev_yymm_str);
            if (it_prev_mon != m_schedule.end()) {
                const sRaceAtDay pm_last_race(std::move(it_prev_mon->second.get_last_race(st_id)));
                if (!pm_last_race.empty()) {
                    return pm_last_race.m_days + 1;
                } else {
                    // なぜか前月データが空(error)
                    PRINT_MESSAGE("[error]fail to get prev month last race : " + yymm_str
                                + " : " + StadiumIDToStr(st_id));
                }
            } else if (m_schedule.empty()) {
                // 初月限定処理 → 補正なし
                // ※初月は捨てデータなのでずれててもOK
                PRINT_MESSAGE("first month : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            } else {
                // 前月データがない(error)
                PRINT_MESSAGE("[error]not found prev month data : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            }
        } else {
            const sRaceAtDay last_race(std::move(sam.get_last_race(st_id)));
            if (!last_race.empty()) {
                return last_race.m_days + 1;
            } else {
                PRINT_MESSAGE("[error] fail to get last race : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            }
        }
        return lc_days;
    }

    /*!
     *  @brief  開催スケジュールデータ取り込み(一ヶ月分)
     *  @param  t           pythonスクリプト(html-parse)戻り値(tuple)
     *  @param  src_yymm    スクリプトに渡したhtmlの開催年月
     *  @return 成否
     *  @note   t[0]:parse成否
     *  @note   t[1]:parseした開催スケジュール年
     *  @note   t[2]:parseした開催スケジュール月
     *  @note   t[3]:レース名リスト
     *  @param  t[4]:スケジュールデータリスト(日ごとの開催情報)
     */
    bool CaptureScheduleData(const boost::python::tuple t, const garnet::YYMM& src_yymm)
    {
        using namespace boost;
        //
        const bool b_result = python::extract<bool>(t[0]);
        if (!b_result) {
            return false; // parse失敗
        }
        const int32_t year = python::extract<int>(t[1]);
        const int32_t month = python::extract<int>(t[2]);
        if (src_yymm.m_year != year && src_yymm.m_month != month) {
            return false; // 要求した年月と異なる
        }
        const python::list mon_race_name = python::extract<python::list>(t[3]);
        const python::list mon_schedule = python::extract<python::list>(t[4]);
        const auto month_days = python::len(mon_schedule);
        const std::string src_yymm_str(std::move(src_yymm.to_string()));
        const std::string prev_yymm_str(std::move(src_yymm.prev().to_string()));

        // レースID対応表<月localレースID, 固有レースID>
        std::unordered_map<int32_t, uint32_t> race_id_map;
        // 1ヶ月分の開催スケジュール
        ScheduleAtMonth sam;
        //
        for (auto dinx = 0; dinx < month_days; dinx++) {
            // 暦日
            const int32_t mday = dinx+1;
            // 1日分の開催スケジュール
            const python::list day_schedule = python::extract<python::list>(mon_schedule[dinx]);
            // 開催中レース場数
            const auto num_rs = python::len(day_schedule);
            //
            for (auto rinx = 0; rinx < num_rs; rinx++) {
                // 開催中レースデータlist[レース場ID, レース種別, レースID(月local), 開催n日目])
                const python::list race_data = python::extract<python::list>(day_schedule[rinx]);
                //
                const int32_t st_id = python::extract<int>(race_data[0]);
                const eStadiumID est_id = static_cast<eStadiumID>(st_id);
                const eRaceType rc_type = StrToRaceType(python::extract<std::string>(race_data[1]));
                const int32_t lc_rc_id = python::extract<int>(race_data[2]);
                const int32_t lc_days = python::extract<int>(race_data[3]);
                // 固有レースID決定
                const uint32_t rc_id = (lc_days == 1)
                    /* 開催初日だったら新規発行 */
                    ? IssueRaceID(lc_rc_id, mon_race_name, race_id_map)
                    /* 二日目以降なら発行済のものを得る */
                    : SearchIssuedRaceID(src_yymm_str,
                                         prev_yymm_str,
                                         est_id,
                                         lc_rc_id,
                                         mon_race_name,
                                         race_id_map);
                // "開催n日目"補正
                // ※月跨ぎの場合htmlからは正確にn日目が取得できない
                // ※colspanをレース開催日数とみなせばいいと思っていたが
                // ※前月27日より前から続くレースは一致しない
                const int32_t days = (lc_days <= mday)
                    ? lc_days
                    : CorrectRaceDays(src_yymm_str, prev_yymm_str, lc_days, est_id, sam);
                if (IsValidRaceID(rc_id)) {
                    sam.entry(mday, est_id, rc_type, rc_id, days);
                } else {
                    // レース固有IDを確定できなかった(error)
                    PRINT_MESSAGE("fail to decide race-id : " + src_yymm_str
                                + ":" + std::to_string(mday) + ":" + StadiumIDToStr(est_id));
                }
            }
        }
        sam.output_json(src_yymm, Environment::GetDataDir());
        m_schedule.emplace(src_yymm_str, std::move(sam));
        return true;
    }

    /*!
     *  @brief  開催スケジュールデータ取得(一ヶ月分)
     */
    void RequestScheduleData()
    {
        const std::string req_ym_str(std::move(m_next_req_ym.to_string()));

        if (m_schedule.find(req_ym_str) != m_schedule.end()) {
            // もうある
            PRINT_MESSAGE("already exist schedule : " + req_ym_str);
            m_next_req_ym.inc_month();
            return;
        }

        m_wait_response = true;
        //
        std::wstring url(L"https://www.boatrace.jp/owpc/pc/race/monthlyschedule");
        const std::wstring ym_str(
            std::move(garnet::utility_string::ToWstring(req_ym_str)));
        garnet::utility_http::AddItemToURL(L"ym", ym_str, url);
        //
        web::http::http_request request(web::http::methods::GET);
        garnet::utility_http::SetHttpCommonHeaderSimple(request);
        web::http::client::http_client http_client(url);
        http_client.request(request).then([this](web::http::http_response response)
        {
            concurrency::streams::istream bodyStream = response.body();
            concurrency::streams::container_buffer<std::string> inStringBuffer;
            return bodyStream.read_to_delim(
                inStringBuffer, 0).then([this, inStringBuffer](size_t bytesRead)
            {
                std::lock_guard<std::recursive_mutex> lock(m_mtx);

                m_last_rcv_tick = garnet::utility_datetime::GetTickCountGeneral();

                const std::string& html_u8 = inStringBuffer.collection();
                try {
                    bool b_result = CaptureScheduleData(
                        boost::python::extract<boost::python::tuple>
                            (m_python.attr("parseSchedule")(html_u8)), m_next_req_ym);
                } catch(boost::python::error_already_set& e) {
                    garnet::utility_python::OutputPythonError(e);
                }

                m_wait_response = false;
                m_next_req_ym.inc_month();
            });
        });
    }

    /*!
     *  @brief  定期処理：主処理
     *  @param  tickCount   経過時間[ミリ秒]
     *  @retval true    終了した
     */
    bool Update_Main(int64_t tickCount)
    {
        if (!m_wait_response) {
            if (m_next_req_ym > m_end_ym) {
                // レース名JSON出力
                m_race_name.output_json(m_start_ym, m_end_ym, Environment::GetDataDir());
                return true;
            } else {
                // 8秒(以上)間隔でデータ取得
                const int32_t REQ_INTV_SEC = 4;
                const int64_t REQ_INTV_MS
                    = garnet::utility_datetime::ToMiliSecondsFromSecond(REQ_INTV_SEC);
                if (tickCount - m_last_rcv_tick > REQ_INTV_MS) {
                    RequestScheduleData();
                }
            }
        }    
        return false;
    }

    /*!
     *  @brief  定期処理：初期化
     */
    void Update_Initialize()
    {
        const std::string datapath(std::move(Environment::GetDataDir()));
        // 指定範囲と適合するデータをJSONから読み込む
        m_race_name.input_json(m_start_ym, datapath);
        if (!m_race_name.empty()) {
            // 適合すればレース名データが非空
            m_race_id_issue = m_race_name.get_last_id();
            // → スケジュールデータも読み込む
            for (garnet::YYMM ym = m_start_ym; ym <= m_end_ym; ym.inc_month()) {
                ScheduleAtMonth sam;
                sam.input_json(ym, datapath);
                if (!sam.empty()) {
                    m_schedule.emplace(std::move(ym.to_string()), std::move(sam));
                } else {
                    // データがない
                    PRINT_MESSAGE("not found schedule : " + ym.to_string());
                }
            }
        }
    }

public:
    /*!
     *  @param  start_ym    収集開始年月(YYYY/MM)
     *  @param  end_ym      収集終了年月(YYYY/MM)
     */
    PIMPL(const std::string& start_ym, const std::string& end_ym)
    : m_mtx()
    , m_sequence(SEQ_INITIALIZE)
    , m_wait_response(false)
    , m_last_rcv_tick(0)
    , m_python(
        std::move(
            garnet::utility_python::PreparePythonScript(
                Environment::GetPythonConfig(), "html_parser_schedule.py")))
    , m_start_ym(garnet::YYMM::Create(start_ym))
    , m_end_ym(garnet::YYMM::Create(end_ym))
    , m_next_req_ym(m_start_ym)
    , m_race_id_issue(BlankRaceID())
    , m_race_name()
    , m_schedule()
    {
    }

    /*!
     *  @param  tickCount   経過時間[ミリ秒]
     */
    bool Update(int64_t tickCount)
    {
        switch (m_sequence)
        {
        case SEQ_INITIALIZE:
            Update_Initialize();
            m_sequence = SEQ_MAIN;
            m_last_rcv_tick = tickCount;
            break;
        case SEQ_MAIN:
            if (Update_Main(tickCount)) {
                m_sequence = SEQ_COMPLETE;
                return true;
            }
            break;
        case SEQ_ERROR:
        case SEQ_COMPLETE:
            return true;
        }

        return false;
    }
};

/*!
 *  @param  start_date  収集開始年月(YYYY/MM)
 *  @param  end_ym      収集終了年月(YYYY/MM)
 */
ScheduleCollector::ScheduleCollector(const std::string& start_ym, const std::string& end_ym)
: m_pImpl(new PIMPL(start_ym, end_ym))
{
}

ScheduleCollector::~ScheduleCollector()
{
}

/*!
 *  @param  tickCount   経過時間[ミリ秒]
 */
bool ScheduleCollector::Update(int64_t tickCount)
{
    return m_pImpl->Update(tickCount);
}

} // namespace boatrace
