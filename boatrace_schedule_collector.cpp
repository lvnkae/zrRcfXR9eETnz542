/*!
 *  @file   boatrace_schedule_collector.cpp
 *  @brief  �����f�[�^���W�F�J�ÃX�P�W���[���f�[�^���W
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
        SEQ_ERROR,      //!< �G���[��~

        SEQ_INITIALIZE, //!< ������
        SEQ_MAIN,       //!< �又��
        SEQ_COMPLETE,   //!< ����I��
    };

    std::recursive_mutex m_mtx; //!< �r������q
    eSequence m_sequence;       //!< �V�[�P���X
    bool m_wait_response;       //!< �����҂��t���O
    int64_t m_last_rcv_tick;    //!< �ŏI������M����(tickCount)

    boost::python::api::object m_python;    //!< python�X�N���v�g�I�u�W�F�N�g

    const garnet::YYMM m_start_ym;  //!< ���W�J�n�N��
    const garnet::YYMM m_end_ym;    //!< ���W�I���N��
    garnet::YYMM m_next_req_ym;     //!< ���ɗv������N��

    //!< ���[�X�ŗLID���s��
    uint32_t m_race_id_issue;
    //!< ���[�X���e�[�u��
    BoatraceNames m_race_name;
    //!< �J�ÃX�P�W���[���e�[�u��<�N��������, �ꂩ�����̃X�P�W���[��>
    std::unordered_map<std::string, ScheduleAtMonth> m_schedule;

    PIMPL(const PIMPL&);
    PIMPL& operator= (const PIMPL&);

    /*!
     *  @brief  ���[�X�ŗLID���s
     *  @param[in]  mon_rc_id       ���[�XID(�����[�J��)
     *  @param[in]  mon_race_name   ���[�X���e�[�u��(�����[�J��)
     *  @param[out] mon_race_id_map ���[�XID�Ή��\(�����[�J��)
     *  @return �ŗL���[�XID
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
     *  @brief  (�K������)���s�σ��[�X�ŗLID��T��
     *  @param  yymm_str        ����(������)
     *  @param  prev_yymm_str   �O��(������j
     *  @param  st_id           ���[�X��ID
     *  @param  mon_rc_id       ���[�XID(�����[�J��)
     *  @param  mon_race_name   ���[�X���e�[�u��(�����[�J��)
     *  @param  mon_race_id_map ���[�XID�Ή��\(�����[�J��)
     *  @return �ŗL���[�XID
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
            // �����Ή��\�ɂ���΂��̂܂܎g�p
            return it_id->second;
        } else {
            const auto it_prev_mon = m_schedule.find(prev_yymm_str);
            if (it_prev_mon != m_schedule.end()) {
                // ���ׂ� �� �O���ŏI���[�X��ID���g�p
                const sRaceAtDay last_race(std::move(it_prev_mon->second.get_last_race(st_id)));
                if (!last_race.empty()) {
                    return last_race.m_id;
                } else {
                    PRINT_MESSAGE("[error] fail to get last race : " + yymm_str
                                + " : " + StadiumIDToStr(st_id));
                }
            } else if (m_schedule.empty()) {
                // ���ׂ� �� �����ł͂Ȃ����V�K���s(���W�������菈��)
                PRINT_MESSAGE("first month : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
                return IssueRaceID(mon_rc_id, mon_race_name, mon_race_id_map);
            } else {
                // �O���f�[�^���Ȃ�(error)
                PRINT_MESSAGE("not found prev month data : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            }
        }
        return BlankRaceID();
    }

    /*!
     *  @brief  �J��n���ڕ␳
     *  @param  yymm_str        ����(������)
     *  @param  prev_yymm_str   �O��(������j
     *  @param  lc_days         �␳�O�f�[�^
     *  @param  st_id           ���[�X��ID
     *  @param  sam             �����X�P�W���[���f�[�^
     */
    int32_t CorrectRaceDays(const std::string& yymm_str,
                            const std::string& prev_yymm_str,
                            int32_t lc_days,
                            eStadiumID st_id,
                            const ScheduleAtMonth& sam)
    {
        if (sam.empty(st_id)) {
            // �Ώۃ��[�X��̓����f�[�^����Ȃ�ΑO����+1��Ԃ�
            const auto it_prev_mon = m_schedule.find(prev_yymm_str);
            if (it_prev_mon != m_schedule.end()) {
                const sRaceAtDay pm_last_race(std::move(it_prev_mon->second.get_last_race(st_id)));
                if (!pm_last_race.empty()) {
                    return pm_last_race.m_days + 1;
                } else {
                    // �Ȃ����O���f�[�^����(error)
                    PRINT_MESSAGE("[error]fail to get prev month last race : " + yymm_str
                                + " : " + StadiumIDToStr(st_id));
                }
            } else if (m_schedule.empty()) {
                // �������菈�� �� �␳�Ȃ�
                // �������͎̂ăf�[�^�Ȃ̂ł���ĂĂ�OK
                PRINT_MESSAGE("first month : " + yymm_str
                            + " : " + StadiumIDToStr(st_id));
            } else {
                // �O���f�[�^���Ȃ�(error)
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
     *  @brief  �J�ÃX�P�W���[���f�[�^��荞��(�ꃖ����)
     *  @param  t           python�X�N���v�g(html-parse)�߂�l(tuple)
     *  @param  src_yymm    �X�N���v�g�ɓn����html�̊J�ÔN��
     *  @return ����
     *  @note   t[0]:parse����
     *  @note   t[1]:parse�����J�ÃX�P�W���[���N
     *  @note   t[2]:parse�����J�ÃX�P�W���[����
     *  @note   t[3]:���[�X�����X�g
     *  @param  t[4]:�X�P�W���[���f�[�^���X�g(�����Ƃ̊J�Ï��)
     */
    bool CaptureScheduleData(const boost::python::tuple t, const garnet::YYMM& src_yymm)
    {
        using namespace boost;
        //
        const bool b_result = python::extract<bool>(t[0]);
        if (!b_result) {
            return false; // parse���s
        }
        const int32_t year = python::extract<int>(t[1]);
        const int32_t month = python::extract<int>(t[2]);
        if (src_yymm.m_year != year && src_yymm.m_month != month) {
            return false; // �v�������N���ƈقȂ�
        }
        const python::list mon_race_name = python::extract<python::list>(t[3]);
        const python::list mon_schedule = python::extract<python::list>(t[4]);
        const auto month_days = python::len(mon_schedule);
        const std::string src_yymm_str(std::move(src_yymm.to_string()));
        const std::string prev_yymm_str(std::move(src_yymm.prev().to_string()));

        // ���[�XID�Ή��\<��local���[�XID, �ŗL���[�XID>
        std::unordered_map<int32_t, uint32_t> race_id_map;
        // 1�������̊J�ÃX�P�W���[��
        ScheduleAtMonth sam;
        //
        for (auto dinx = 0; dinx < month_days; dinx++) {
            // ���
            const int32_t mday = dinx+1;
            // 1�����̊J�ÃX�P�W���[��
            const python::list day_schedule = python::extract<python::list>(mon_schedule[dinx]);
            // �J�Ò����[�X�ꐔ
            const auto num_rs = python::len(day_schedule);
            //
            for (auto rinx = 0; rinx < num_rs; rinx++) {
                // �J�Ò����[�X�f�[�^list[���[�X��ID, ���[�X���, ���[�XID(��local), �J��n����])
                const python::list race_data = python::extract<python::list>(day_schedule[rinx]);
                //
                const int32_t st_id = python::extract<int>(race_data[0]);
                const eStadiumID est_id = static_cast<eStadiumID>(st_id);
                const eRaceType rc_type = StrToRaceType(python::extract<std::string>(race_data[1]));
                const int32_t lc_rc_id = python::extract<int>(race_data[2]);
                const int32_t lc_days = python::extract<int>(race_data[3]);
                // �ŗL���[�XID����
                const uint32_t rc_id = (lc_days == 1)
                    /* �J�Ï�����������V�K���s */
                    ? IssueRaceID(lc_rc_id, mon_race_name, race_id_map)
                    /* ����ڈȍ~�Ȃ甭�s�ς̂��̂𓾂� */
                    : SearchIssuedRaceID(src_yymm_str,
                                         prev_yymm_str,
                                         est_id,
                                         lc_rc_id,
                                         mon_race_name,
                                         race_id_map);
                // "�J��n����"�␳
                // �����ׂ��̏ꍇhtml����͐��m��n���ڂ��擾�ł��Ȃ�
                // ��colspan�����[�X�J�Ó����Ƃ݂Ȃ��΂����Ǝv���Ă�����
                // ���O��27�����O���瑱�����[�X�͈�v���Ȃ�
                const int32_t days = (lc_days <= mday)
                    ? lc_days
                    : CorrectRaceDays(src_yymm_str, prev_yymm_str, lc_days, est_id, sam);
                if (IsValidRaceID(rc_id)) {
                    sam.entry(mday, est_id, rc_type, rc_id, days);
                } else {
                    // ���[�X�ŗLID���m��ł��Ȃ�����(error)
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
     *  @brief  �J�ÃX�P�W���[���f�[�^�擾(�ꃖ����)
     */
    void RequestScheduleData()
    {
        const std::string req_ym_str(std::move(m_next_req_ym.to_string()));

        if (m_schedule.find(req_ym_str) != m_schedule.end()) {
            // ��������
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
     *  @brief  ��������F�又��
     *  @param  tickCount   �o�ߎ���[�~���b]
     *  @retval true    �I������
     */
    bool Update_Main(int64_t tickCount)
    {
        if (!m_wait_response) {
            if (m_next_req_ym > m_end_ym) {
                // ���[�X��JSON�o��
                m_race_name.output_json(m_start_ym, m_end_ym, Environment::GetDataDir());
                return true;
            } else {
                // 8�b(�ȏ�)�Ԋu�Ńf�[�^�擾
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
     *  @brief  ��������F������
     */
    void Update_Initialize()
    {
        const std::string datapath(std::move(Environment::GetDataDir()));
        // �w��͈͂ƓK������f�[�^��JSON����ǂݍ���
        m_race_name.input_json(m_start_ym, datapath);
        if (!m_race_name.empty()) {
            // �K������΃��[�X���f�[�^�����
            m_race_id_issue = m_race_name.get_last_id();
            // �� �X�P�W���[���f�[�^���ǂݍ���
            for (garnet::YYMM ym = m_start_ym; ym <= m_end_ym; ym.inc_month()) {
                ScheduleAtMonth sam;
                sam.input_json(ym, datapath);
                if (!sam.empty()) {
                    m_schedule.emplace(std::move(ym.to_string()), std::move(sam));
                } else {
                    // �f�[�^���Ȃ�
                    PRINT_MESSAGE("not found schedule : " + ym.to_string());
                }
            }
        }
    }

public:
    /*!
     *  @param  start_ym    ���W�J�n�N��(YYYY/MM)
     *  @param  end_ym      ���W�I���N��(YYYY/MM)
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
     *  @param  tickCount   �o�ߎ���[�~���b]
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
 *  @param  start_date  ���W�J�n�N��(YYYY/MM)
 *  @param  end_ym      ���W�I���N��(YYYY/MM)
 */
ScheduleCollector::ScheduleCollector(const std::string& start_ym, const std::string& end_ym)
: m_pImpl(new PIMPL(start_ym, end_ym))
{
}

ScheduleCollector::~ScheduleCollector()
{
}

/*!
 *  @param  tickCount   �o�ߎ���[�~���b]
 */
bool ScheduleCollector::Update(int64_t tickCount)
{
    return m_pImpl->Update(tickCount);
}

} // namespace boatrace
