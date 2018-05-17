/*!
 *  @file   boatrace_odds_collector.cpp
 *  @brief  �����f�[�^���W�F�I�b�Y�f�[�^���W
 *  @date   2018/03/07
 */
#include "boatrace_odds_collector.h"

#include "boatrace_data.h"
#include "boatrace_data_utility.h"
#include "boatrace_schedule_collector.h"
#include "boatrace_schedule_data.h"
#include "environment.h"

#include "http/http_proxy.h"
#include "http/http_proxy_config.h"
#include "utility/utility_datetime.h"
#include "utility/utility_http.h"
#include "utility/utility_python.h"
#include "utility/utility_string.h"
#include "yymmdd.h"

#include "cpprest/http_client.h"
#include "cpprest/filestream.h"
#include <codecvt>
#include <mutex>
#include <unordered_set>

#define USE_PROXY   (0)

namespace boatrace
{

/*!
 *
 */
class OddsCollector::PIMPL
{
private:
    enum eSequence
    {
        SEQ_ERROR,      //!< �G���[��~

        SEQ_INITIALIZE, //!< ������
        SEQ_MAIN,       //!< �又��
        SEQ_COMPLETE,   //!< ����I��
    };

    std::recursive_mutex m_mtx; //!< �r������q

    eSequence m_sequence;           //!< �V�[�P���X
    bool m_decide_collection;       //!< ���W�Ώۃf�[�^����
    bool m_wait_response;           //!< �����҂��t���O
    bool m_retry;                   //!< ���g���C�t���O
    bool m_req_outputfile;          //!< �t�@�C���o�̓t���O
    int64_t m_last_rcv_tick;        //!< �ŏI������M����(tickCount)

    boost::python::api::object m_python;    //!< python�X�N���v�g�I�u�W�F�N�g
    web::http::proxy_magazine m_proxy;      //!< proxy�e�q

    const std::wstring m_url;               //!< ���WURL
    const garnet::YYMMDD m_start_date;      //!< ���W�J�n�N��
    const garnet::YYMMDD m_end_date;        //!< ���W�I���N��
    const std::unordered_set<eRaceType> m_target_race;  //!< ���W�Ώۃ��[�X�^�C�v

    garnet::YYMMDD m_collect_date;          //!< ���W���N����
    ScheduleAtDay m_collect_day_schedule;   //!< ���W����̊J�ÃX�P�W���[��
    eStadiumID m_collect_stadium;           //!< ���W�����[�X��
    int32_t m_collect_race_no;              //!< ���W�����[�X�ԍ�(��n���[�X)

    //! �I�b�Y�f�[�^<�N��������, �I�b�Y�f�[�^(�ꂩ����)
    std::unordered_map<std::string, OddsAtMonth> m_odds;

    PIMPL(const PIMPL&);
    PIMPL& operator= (const PIMPL&);


    /*!
     *  @brief  �I�b�Y�f�[�^��荞��(1���[�X��)
     *  @param  t   python�X�N���v�g(html-parse)�߂�l(tuple)
     *  @return ����
     *  @note   t[0]:parse����
     *  @note   t[1]:parse�����I�b�Y�f�[�^�N
     *  @note   t[2]:parse�����I�b�Y�f�[�^��
     *  @note   t[3]:parse�����I�b�Y�f�[�^��
     *  @note   t[4]:3�A�P�I�b�Y�f�[�^
     *  @note   t[5]:2�A�P�I�b�Y�f�[�^
     *  @note   t[6]:3�A���I�b�Y�f�[�^
     *  @note   t[7]:2�A���I�b�Y�f�[�^
     */
    bool CaptureOddsData(const boost::python::tuple t)
    {
        using namespace boost;
        //
        const bool b_result = python::extract<bool>(t[0]);
        if (!b_result) {
            //
            PRINT_MESSAGE("fail to parse odds-data (date:" + m_collect_date.to_string()
                          + " stadium:" + std::to_string(m_collect_stadium)
                          + "race:" + std::to_string(m_collect_race_no)
                          + ")");
            return false; // [error]parse���s
        }
        const garnet::YYMMDD parse_date = garnet::YYMMDD(python::extract<int32_t>(t[1]),
                                                         python::extract<int32_t>(t[2]),
                                                         python::extract<int32_t>(t[3]));
        if (parse_date.m_year == 0 && parse_date.m_month == 0 && parse_date.m_day == 0) {
            PRINT_MESSAGE("no odds-data(date:" + m_collect_date.to_string()
                          + " stadium:" + std::to_string(m_collect_stadium)
                          + "race:" + std::to_string(m_collect_race_no)
                          + ")");
            return true; // �������������f�[�^�����݂��Ȃ��̂ŃX���[
                         // (���Y�J�Ó��S���[�X�����E���~�̏ꍇ����)
        }
        if (m_collect_date != parse_date) {
            PRINT_MESSAGE("not equal parse odds-data (date:" + m_collect_date.to_string()
                          + " stadium:" + std::to_string(m_collect_stadium)
                          + "race:" + std::to_string(m_collect_race_no)
                          + ")");
            return false; // [error]�v�������N�����ƈقȂ�
        }

        sOddsAtRace rc_odds;
        {
            const python::list odds_wins3 = python::extract<python::list>(t[4]);
            const auto num_data = python::len(odds_wins3);
            for (auto inx = 0; inx < num_data; inx++) {
                const python::list dat = python::extract<python::list>(odds_wins3[inx]);
                const python::list combination = python::extract<python::list>(dat[0]);
                const int32_t c1 = python::extract<int32_t>(combination[0]);
                const int32_t c2 = python::extract<int32_t>(combination[1]);
                const int32_t c3 = python::extract<int32_t>(combination[2]);
                const float32 odds = python::extract<float32>(dat[1]);
                rc_odds.m_data[c1].m_wins3[c2].emplace(c3, odds);
            }
        }
        {
            const python::list odds_wins2 = python::extract<python::list>(t[5]);
            const auto num_data = python::len(odds_wins2);
            for (auto inx = 0; inx < num_data; inx++) {
                const python::list dat = python::extract<python::list>(odds_wins2[inx]);
                const python::list combination = python::extract<python::list>(dat[0]);
                const int32_t c1 = python::extract<int32_t>(combination[0]);
                const int32_t c2 = python::extract<int32_t>(combination[1]);
                const float32 odds = python::extract<float32>(dat[1]);
                rc_odds.m_data[c1].m_wins2.emplace(c2, odds);
            }
        }
        {
            const python::list odds_place3 = python::extract<python::list>(t[6]);
            const auto num_data = python::len(odds_place3);
            for (auto inx = 0; inx < num_data; inx++) {
                const python::list dat = python::extract<python::list>(odds_place3[inx]);
                const python::list combination = python::extract<python::list>(dat[0]);
                const int32_t c1 = python::extract<int32_t>(combination[0]);
                const int32_t c2 = python::extract<int32_t>(combination[1]);
                const int32_t c3 = python::extract<int32_t>(combination[2]);
                const float32 odds = python::extract<float32>(dat[1]);
                rc_odds.m_data[c1].m_place3[c2].emplace(c3, odds);
            }
        }
        {
            const python::list odds_place2 = python::extract<python::list>(t[7]);
            const auto num_data = python::len(odds_place2);
            for (auto inx = 0; inx < num_data; inx++) {
                const python::list dat = python::extract<python::list>(odds_place2[inx]);
                const python::list combination = python::extract<python::list>(dat[0]);
                const int32_t c1 = python::extract<int32_t>(combination[0]);
                const int32_t c2 = python::extract<int32_t>(combination[1]);
                const float32 odds = python::extract<float32>(dat[1]);
                rc_odds.m_data[c1].m_place2.emplace(c2, odds);
            }
        }

        m_odds[m_collect_date.to_yymm().to_string()].entry(m_collect_date.m_day,
                                                           m_collect_stadium,
                                                           m_collect_race_no,
                                                           std::move(rc_odds));
        return true;
    }

    /*!
     *  @brief  1���[�X���̃I�b�Y�f�[�^�擾
     *  @param  schedule    �J�ÃX�P�W���[��
     */
    void RequestOddsData(const ScheduleCollector& schedule)
    {
        m_wait_response = true;
        m_decide_collection = false;
        //
        std::wstring url(m_url);
        {
            using garnet::utility_http::AddItemToURL;
            AddItemToURL(L"day", std::to_wstring(m_collect_date.m_day), url);
            AddItemToURL(L"month", std::to_wstring(m_collect_date.m_month), url);
            AddItemToURL(L"year", std::to_wstring(m_collect_date.m_year), url);
            const int32_t stadium_id = static_cast<int32_t>(m_collect_stadium);
            AddItemToURL(L"jyo", std::to_wstring(stadium_id), url);
            AddItemToURL(L"r", std::to_wstring(m_collect_race_no), url);
            const int32_t mode = 99; // �{����،�I�b�Y�̂ݓ���
            AddItemToURL(L"mode", std::to_wstring(mode), url);
        }
        //
        web::http::http_request request(web::http::methods::GET);
        garnet::utility_http::SetHttpCommonHeaderSimple(request);
        const std::wstring current_proxy(std::move(m_proxy.get_current()));
#if USE_PROXY
        web::http::client::http_client_config config;
        config.set_proxy(web::web_proxy(L"http://" + current_proxy));
        web::http::client::http_client http_client(url, config);
#else
        web::http::client::http_client http_client(url);
#endif/* USE_PROXY */
        try {
            http_client.request(request).then([this,
                                               url,
                                               current_proxy](web::http::http_response response)
            {
                concurrency::streams::istream bodyStream = response.body();
                concurrency::streams::container_buffer<std::string> inStringBuffer;

                m_last_rcv_tick = garnet::utility_datetime::GetTickCountGeneral();

                const web::http::status_code status = response.status_code();
                if (status == web::http::status_codes::OK) {
                    return bodyStream.read_to_delim(
                        inStringBuffer, 0).then([this,
                                                 inStringBuffer,
                                                 url,
                                                 current_proxy](size_t bytesRead)
                    {
                        //
                        std::lock_guard<std::recursive_mutex> lock(m_mtx);
                        //
                        m_proxy.update_tick(current_proxy, m_last_rcv_tick);
                        m_proxy.reset_error_count(current_proxy);
                        //
                        const std::string& html_u8 = inStringBuffer.collection();
                        try {
                            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utfconv;
                            std::string outfilename(m_collect_date.to_string());
                            outfilename += "_st" + std::to_string(m_collect_stadium);
                            outfilename += "_rc" + std::to_string(m_collect_race_no);
                            outfilename += ".html";
                            //
                            const size_t LOCK_MESSAGE_SIZE = 200;
                            if (bytesRead <= LOCK_MESSAGE_SIZE) {
                                // �A�N�Z�X�ߑ��ɂ�郍�b�N
                                PRINT_MESSAGE("lock!! url=" + utfconv.to_bytes(url));
                                m_python.attr("debugOutputHTMLToFile")(html_u8, outfilename);
                                m_retry = true;
                                // proxy���x�܂���
                                m_proxy.update_tick(current_proxy, m_last_rcv_tick);
                                m_proxy.release_current();
                            } else {
                                bool b_result = CaptureOddsData(
                                    boost::python::extract<boost::python::tuple>
                                        (m_python.attr("parseOddstable")(html_u8)));
                                if (!b_result) {
                                    // [error]parse���s(�擾url�o��) 
                                    PRINT_MESSAGE("parse error! url=" + utfconv.to_bytes(url));
                                    m_python.attr("debugOutputHTMLToFile")(html_u8, outfilename);
                                    // ���g���C�����X�L�b�v
                                }
                            }
                        } catch(boost::python::error_already_set& e) {
                            garnet::utility_python::OutputPythonError(e);
                        }

                        m_wait_response = false;
                    });
                } else if (status == web::http::status_codes::InternalError ||
                           status == web::http::status_codes::BadGateway ||
                           status == web::http::status_codes::ServiceUnavailable ||
                           status == web::http::status_codes::GatewayTimeout) {
                        //
                        std::lock_guard<std::recursive_mutex> lock(m_mtx);
                        //
                        m_proxy.update_tick(current_proxy, m_last_rcv_tick);
                        m_proxy.inc_error_count(current_proxy);
                        //
                        m_wait_response = false;
                        m_retry = true;
                }
                return bodyStream.read_to_delim(inStringBuffer, 0).then([](size_t bytesRead){});
            }).wait();
        } catch (const::std::exception& e) {
            // http_request��O
            std::lock_guard<std::recursive_mutex> lock(m_mtx);
            // ���b�Z�[�W�̓J�b�g���ăR�[�h�̂ݏo��
            std::string error_u8(e.what());
            auto last_colon_pos = error_u8.find_last_of(":");
            if (last_colon_pos > 0) {
                error_u8[last_colon_pos] = '\0';
                std::cout << error_u8.c_str() << std::endl;
            } else {
                std::cout << "exception" << std::endl;
            }
            //
            m_proxy.update_tick(current_proxy, m_last_rcv_tick);
            m_proxy.inc_error_count(current_proxy);
            //
            m_last_rcv_tick = garnet::utility_datetime::GetTickCountGeneral();
            m_wait_response = false;
            m_retry = true;
        }
    }

    /*!
     *  @brief  �t�@�C���o��
     *  @param  yymm    �o�̓f�[�^�N��
     */
    void OutputFile(const garnet::YYMM& yymm)
    {
        const auto it = m_odds.find(yymm.to_string());
        if (m_odds.end() == it) {
            PRINT_MESSAGE("not found odds_data (" + yymm.to_string() + ")");
            return;
        }
        it->second.output_json(yymm, Environment::GetDataDir());
    }

    /*!
     *  @brief  ���W�������肷��
     *  @param  schedule    �J�ÃX�P�W���[��
     *  @retval true        ����ł���
     */
    bool DecideCollectDate(const ScheduleCollector& schedule)
    {
        while (1) {
            if (m_collect_date > m_end_date) {
                return false;
            }
            if (m_collect_day_schedule.empty()) {
                schedule.GetScheduleAtDay(m_collect_date, m_collect_day_schedule);
            }
            if (m_collect_day_schedule.empty()) {
                m_collect_date++;
            } else {
                return true;
            }
        }
    }

    /*!
     *  @brief  ���W���[�X������肷��
     *  @param  schedule    �J�ÃX�P�W���[��
     *  @retval true        ����ł���
     */
    bool DecideCollectStadium()
    {
        while (!m_collect_day_schedule.empty()) {
            const ScheduleAtDay::sUnit unit(std::move(m_collect_day_schedule.front_and_pop()));
            if (m_target_race.find(unit.m_type) != m_target_race.end()) {
                m_collect_stadium = unit.m_stadium;
                m_collect_race_no = FirstRaceNumber();
                return true;
            }
        }
        return false;
    }

    /*!
     *  @brief  ���W���[�X�����肷��
     *  @param  schedule    �J�ÃX�P�W���[��
     *  @retval true        ����ł���
     */
    bool DecideCollectRace(const ScheduleCollector& schedule)
    {
        while (1) {
            if (STADIUM_NONE == m_collect_stadium) {
                if (!DecideCollectDate(schedule)) {
                    return false;
                }
                if (DecideCollectStadium()) {
                    return true;
                }
                m_collect_date++;
            } else {
                m_collect_race_no++;
                if (m_collect_race_no <= LastRaceNumber()) {
                    return true;
                }
                m_collect_stadium = STADIUM_NONE;
                if (m_collect_day_schedule.empty()) {
                    m_collect_date++;
                }
            }
        }
    }

    /*!
     *  @brief  proxy����
     *  @param  tickCount   �o�ߎ���[�~���b]
     */
    bool DecideProxy(int64_t tickCount)
    {
#if USE_PROXY
        const std::wstring prev(m_proxy.get_current());
        m_proxy.update(tickCount);
        const std::wstring next(m_proxy.get_current());
        if (prev != next && !next.empty()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utfconv;
            std::wstring lc_wstr(
                std::move(garnet::utility_datetime::GetLocalMachineTime(L"%H:%M:%S")));
            PRINT_MESSAGE("change proxy (" + utfconv.to_bytes(next)
                        + ") @ " + utfconv.to_bytes(lc_wstr));
        }
        return !next.empty();
#else
        return true;
#endif/* USE_PROXY */
    }

    /*!
     *  @brief  ��������F�又��
     *  @param  tickCount   �o�ߎ���[�~���b]
     *  @param  schedule    �J�ÃX�P�W���[��
     *  @retval true    �I������
     */
    bool Update_Main(int64_t tickCount, const ScheduleCollector& schedule)
    {
        if (m_wait_response) {
            return false;
        }
        if (!DecideProxy(tickCount)) {
            return false;
        }
        if (m_retry) {
            const int32_t RETRY_INTV_SEC = 16;
            const int64_t RETRY_INTV_MS
                = garnet::utility_datetime::ToMiliSecondsFromSecond(RETRY_INTV_SEC);
            if (tickCount - m_last_rcv_tick > RETRY_INTV_MS) {
                m_retry = false;
                RequestOddsData(schedule);
            }
        } else {
            if (m_decide_collection) {
                // �w��b��(�ȏ�)�Ԋu�Ńf�[�^�擾
                const int32_t REQ_INTV_SEC = 4;
                const int64_t REQ_INTV_MS
                    = garnet::utility_datetime::ToMiliSecondsFromSecond(REQ_INTV_SEC);
                if (tickCount - m_last_rcv_tick > REQ_INTV_MS) {
                    RequestOddsData(schedule);
                }
            } else {
                const garnet::YYMM prev_yymm(std::move(m_collect_date.to_yymm()));
                if (!DecideCollectRace(schedule)) {
                    // ���W�����i�̂͂��j
                    PRINT_MESSAGE("end odds-data collection (last-day:"
                                    + m_collect_date.to_string() + ")");
                    return true;
                } else {
                    // �����ς������ꂩ�����̃f�[�^���t�@�C���o��
                    if (prev_yymm != m_collect_date.to_yymm() && m_req_outputfile) {
                        OutputFile(prev_yymm);
                        m_req_outputfile = false;
                    }
                    const auto it = m_odds.find(m_collect_date.to_yymm().to_string());
                    if (it != m_odds.end()) {
                        if (it->second.exists(m_collect_date.m_day,
                                              m_collect_stadium,
                                              m_collect_race_no)) {
                            // ��������
                            return false;
                        }
                    }
                    m_req_outputfile = true;
                    m_decide_collection = true;
                }
            }
        } 
        return false;
    }

    /*!
     *  @brief  ��������F������
     */
    void Update_Initialize()
    {
        const std::string datapath(std::move(Environment::GetDataDir()));
        //
        const garnet::YYMM end_ym(std::move(m_end_date.to_yymm()));
        for (garnet::YYMM ym(std::move(m_start_date.to_yymm())); ym <= end_ym; ym.inc_month()) {
            OddsAtMonth oam;
            oam.input_json(ym, datapath);
            if (!oam.empty()) {
                m_odds.emplace(std::move(ym.to_string()), std::move(oam));
            } else {
                PRINT_MESSAGE("not found odds : " + ym.to_string());
            }
        }
    }

public:
    /*!
     *  @param  start_date  ���W�J�n�N��(YYYY/MM/DD)
     *  @param  end_date    ���W�I���N��(YYYY/MM/DD)
     *  @param  race_type   ���W���[�X�^�C�v
     */
    PIMPL(const std::string& start_date,
          const std::string& end_date,
          const std::unordered_set<eRaceType>& race_type)
    : m_mtx()
    , m_sequence(SEQ_INITIALIZE)
    , m_decide_collection(false)
    , m_wait_response(false)
    , m_retry(false)
    , m_req_outputfile(false)
    , m_last_rcv_tick(0)
    , m_python(
        std::move(
            garnet::utility_python::PreparePythonScript(
                Environment::GetPythonConfig(), "html_parser_odds.py")))
    , m_proxy(Environment::GetProxyConfig())
    , m_url(std::move(Environment::GetOddsSiteURL()))
    , m_start_date(garnet::YYMMDD::Create(start_date))
    , m_end_date(garnet::YYMMDD::Create(end_date))
    , m_target_race(race_type)
    , m_collect_date(m_start_date)
    , m_collect_day_schedule()
    , m_collect_stadium(STADIUM_NONE)
    , m_collect_race_no(BlankRaceNumber())
    , m_odds()
    {
    }

    /*!
     *  @param  tickCount   �o�ߎ���[�~���b]
     *  @param  schedule    �J�ÃX�P�W���[��
     */
    bool Update(int64_t tickCount, const ScheduleCollector& schedule)
    {
        switch (m_sequence)
        {
        case SEQ_INITIALIZE:
            Update_Initialize();
            m_sequence = SEQ_MAIN;
            m_last_rcv_tick = tickCount;
            break;
        case SEQ_MAIN:
            if (Update_Main(tickCount, schedule)) {
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
 *  @param  start_date  ���W�J�n�N��(YYYY/MM/DD)
 *  @param  end_date    ���W�I���N��(YYYY/MM/DD)
 *  @param  race_type   ���W���[�X�^�C�v
 */
OddsCollector::OddsCollector(const std::string& start_date,
                             const std::string& end_date,
                             const std::unordered_set<eRaceType>& race_type)
: m_pImpl(new PIMPL(start_date, end_date, race_type))
{
}

OddsCollector::~OddsCollector()
{
}

/*!
 *  @param  tickCount   �o�ߎ���[�~���b]
 *  @param  schedule    �J�ÃX�P�W���[��
 */
bool OddsCollector::Update(int64_t tickCount, const ScheduleCollector& schedule)
{
    return m_pImpl->Update(tickCount, schedule);
}

} // namespace boatrace
