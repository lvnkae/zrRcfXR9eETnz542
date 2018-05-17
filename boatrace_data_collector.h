/*!
 *  @file   boatrace_data_collector.h
 *  @brief  競艇データ収集：集権処理
 *  @date   2018/03/06
 */
#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include "boatrace_data_define.h"

namespace boatrace
{

class RaceDataCollector
{
public:
    /*!
     *  @param  sch_start_ym    開催スケジュール収集開始年月(YYYY/MM)
     *  @param  sch_end_ym      開催スケジュール収集終了年月(YYYY/MM)
     *  @param  dat_start_ym    レースデータ収集開始年月(YYYY/MM)
     *  @param  dat_end_ym      レースデータ収集開始年月(YYYY/MM)
     *  @param  race_type       データ収集対象レースタイプ
     */
    RaceDataCollector(const std::string& sch_start_ym,
                      const std::string& sch_end_ym,
                      const std::string& dat_start_ym,
                      const std::string& dat_end_ym,
                      const std::unordered_set<eRaceType>& race_type);
    ~RaceDataCollector();

    /*!
     *  @retval true    終了してよい
     */
    bool Update(int64_t tickCount);

private:
    RaceDataCollector();
    RaceDataCollector(const RaceDataCollector&);
    RaceDataCollector(RaceDataCollector&&);
    RaceDataCollector& operator= (const RaceDataCollector&);

    class PIMPL;
    std::unique_ptr<PIMPL> m_pImpl;
};

} // namespace boatrace
