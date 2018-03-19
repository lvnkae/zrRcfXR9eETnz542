/*!
 *  @file   boatrace_data_collector.coo
 *  @brief  競艇データ収集：集権処理
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
        SEQ_ERROR,              //!< エラー停止

        SEQ_INITIALIZE,         //!< 初期化
        SEQ_COLLECT_SCHEDULE,   //!< スケジュールデータ収集
        SEQ_COLLECT_ODDS,       //!< オッズデータ収集
        SEQ_COLLECT_RACE,       //!< レースデータ収集
    };

    eSequence m_sequence;   //! シーケンス

    ScheduleCollector m_schedule_col;   //! 開催スケジュールデータ収集者


public:
    /*!
     *  @param  start_date  収集開始年月(YYYY/MM)
     *  @param  end_ym      収集終了年月(YYYY/MM)
     */
    PIMPL(const std::string& start_ym, const std::string& end_ym)
    : m_sequence(SEQ_INITIALIZE)
    , m_schedule_col(start_ym, end_ym)
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
            // 特になし
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
 *  @param  start_date  収集開始年月(YYYY/MM)
 *  @param  end_ym      収集終了年月(YYYY/MM)
 */
RaceDataCollector::RaceDataCollector(const std::string& start_ym, const std::string& end_ym)
: m_pImpl(new PIMPL(start_ym, end_ym))
{
}
RaceDataCollector::~RaceDataCollector()
{
}

/*!
 *  @param  tickCount   経過時間[ミリ秒]
 */
bool RaceDataCollector::Update(int64_t tickCount)
{
    return m_pImpl->Update(tickCount);
}

} // namespace boatrace
