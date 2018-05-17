/*!
 *  @file   boatrace_data_collector.coo
 *  @brief  競艇データ収集：集権処理
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
        SEQ_ERROR,              //!< エラー停止

        SEQ_INITIALIZE,         //!< 初期化
        SEQ_COLLECT_SCHEDULE,   //!< スケジュールデータ収集
        SEQ_COLLECT_ODDS,       //!< オッズデータ収集
        SEQ_COLLECT_RACE,       //!< レースデータ収集
    };

    eSequence m_sequence;   //! シーケンス

    ScheduleCollector m_schedule_col;   //! 開催スケジュールデータ収集者
    OddsCollector m_odds_col;           //! オッズデータ収集者

public:
    /*!
     *  @param  sch_start_ym    開催スケジュール収集開始年月(YYYY/MM)
     *  @param  sch_end_ym      開催スケジュール収集終了年月(YYYY/MM)
     *  @param  dat_start_ym    レースデータ収集開始年月(YYYY/MM)
     *  @param  dat_end_ym      レースデータ収集開始年月(YYYY/MM)
     *  @param  race_type       データ収集対象レースタイプ
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
 *  @param  sch_start_ym    開催スケジュール収集開始年月(YYYY/MM)
 *  @param  sch_end_ym      開催スケジュール収集終了年月(YYYY/MM)
 *  @param  dat_start_ym    レースデータ収集開始年月(YYYY/MM)
 *  @param  dat_end_ym      レースデータ収集開始年月(YYYY/MM)
 *  @param  race_type       データ収集対象レースタイプ
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
 *  @param  tickCount   経過時間[ミリ秒]
 */
bool RaceDataCollector::Update(int64_t tickCount)
{
    return m_pImpl->Update(tickCount);
}

} // namespace boatrace
