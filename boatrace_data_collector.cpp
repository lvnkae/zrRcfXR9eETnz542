/*!
 *  @file   boatrace_data_collector.coo
 *  @brief  �����f�[�^���W�F�W������
 *  @date   2018/03/06
 */
#include "boatrace_data_collector.h"

#include "boatrace_schedule_collector.h"
#include "boatrace_odds_collector.h"

namespace boatrace
{

class RaceDataCollector::PIMPL
{
private:
    enum eSequence
    {
        SEQ_ERROR,              //!< �G���[��~

        SEQ_INITIALIZE,         //!< ������
        SEQ_COLLECT_SCHEDULE,   //!< �X�P�W���[���f�[�^���W
        SEQ_COLLECT_ODDS,       //!< �I�b�Y�f�[�^���W
        SEQ_COLLECT_RACE,       //!< ���[�X�f�[�^���W
    };

    eSequence m_sequence;   //! �V�[�P���X

    ScheduleCollector m_schedule_col;   //! �J�ÃX�P�W���[���f�[�^���W��
    OddsCollector m_odds_col;           //! �I�b�Y�f�[�^���W��

public:
    /*!
     *  @param  sch_start_ym    �J�ÃX�P�W���[�����W�J�n�N��(YYYY/MM)
     *  @param  sch_end_ym      �J�ÃX�P�W���[�����W�I���N��(YYYY/MM)
     *  @param  dat_start_ym    ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
     *  @param  dat_end_ym      ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
     *  @param  race_type       �f�[�^���W�Ώۃ��[�X�^�C�v
     */
    PIMPL(const std::string& sch_start_ym,
          const std::string& sch_end_ym,
          const std::string& dat_start_ym,
          const std::string& dat_end_ym,
          const std::unordered_set<eRaceType>& race_type)
    : m_sequence(SEQ_INITIALIZE)
    , m_schedule_col(sch_start_ym, sch_end_ym)
    , m_odds_col(dat_start_ym, dat_end_ym, race_type)
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
            // ���ɂȂ�
            m_sequence = SEQ_COLLECT_SCHEDULE;
            break;
        case SEQ_COLLECT_SCHEDULE:
            if (m_schedule_col.Update(tickCount)) {
                m_sequence = SEQ_COLLECT_ODDS;
            }
            break;
        case SEQ_COLLECT_ODDS:
            if (m_odds_col.Update(tickCount, m_schedule_col)) {
                m_sequence = SEQ_COLLECT_RACE;
            }
            break;
        case SEQ_COLLECT_RACE:
            return true;
            break;
        case SEQ_ERROR:
            return true;
        }
        return false;
    }
};

/*!
 *  @param  sch_start_ym    �J�ÃX�P�W���[�����W�J�n�N��(YYYY/MM)
 *  @param  sch_end_ym      �J�ÃX�P�W���[�����W�I���N��(YYYY/MM)
 *  @param  dat_start_ym    ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
 *  @param  dat_end_ym      ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
 *  @param  race_type       �f�[�^���W�Ώۃ��[�X�^�C�v
 */
RaceDataCollector::RaceDataCollector(const std::string& sch_start_ym,
                                     const std::string& sch_end_ym,
                                     const std::string& dat_start_ym,
                                     const std::string& dat_end_ym,
                                     const std::unordered_set<eRaceType>& race_type)
: m_pImpl(new PIMPL(sch_start_ym, sch_end_ym, dat_start_ym, dat_end_ym, race_type))
{
}
RaceDataCollector::~RaceDataCollector()
{
}

/*!
 *  @param  tickCount   �o�ߎ���[�~���b]
 */
bool RaceDataCollector::Update(int64_t tickCount)
{
    return m_pImpl->Update(tickCount);
}

} // namespace boatrace
