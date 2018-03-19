/*!
 *  @file   boatrace_data_collector.coo
 *  @brief  �����f�[�^���W�F�W������
 *  @date   2018/03/06
 */
#include "boatrace_data_collector.h"

#include "boatrace_schedule_collector.h"

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


public:
    /*!
     *  @param  start_date  ���W�J�n�N��(YYYY/MM)
     *  @param  end_ym      ���W�I���N��(YYYY/MM)
     */
    PIMPL(const std::string& start_ym, const std::string& end_ym)
    : m_sequence(SEQ_INITIALIZE)
    , m_schedule_col(start_ym, end_ym)
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
            return true;
            break;
        case SEQ_ERROR:
            return true;
        }
        return false;
    }
};

/*!
 *  @param  start_date  ���W�J�n�N��(YYYY/MM)
 *  @param  end_ym      ���W�I���N��(YYYY/MM)
 */
RaceDataCollector::RaceDataCollector(const std::string& start_ym, const std::string& end_ym)
: m_pImpl(new PIMPL(start_ym, end_ym))
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
