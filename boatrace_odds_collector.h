/*!
 *  @file   boatrace_odds_collector.h
 *  @brief  競艇データ収集：オッズデータ収集
 *  @date   2018/04/13
 */
#pragma once

#include "boatrace_data_define.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace boatrace
{
class ScheduleCollector;

class OddsCollector
{
public:
    /*!
     *  @param  start_date  収集開始年月(YYYY/MM/DD)
     *  @param  end_date    収集終了年月(YYYY/MM/DD)
     *  @param  race_type   収集レースタイプ
     */
    OddsCollector(const std::string& start_date,
                  const std::string& end_date,
                  const std::unordered_set<eRaceType>& race_type);
    /*!
     */
    ~OddsCollector();

    /*!
     *  @param  tickCount   経過時間[ミリ秒]
     *  @retval true        収集処理完了
     */
    bool Update(int64_t tickCount, const ScheduleCollector& schecule);

private:
    OddsCollector();
    OddsCollector(const OddsCollector&);
    OddsCollector(OddsCollector&&);
    OddsCollector& operator= (const OddsCollector&);

    class PIMPL;
    std::unique_ptr<PIMPL> m_pImpl;
};

} // namespace boatrace
