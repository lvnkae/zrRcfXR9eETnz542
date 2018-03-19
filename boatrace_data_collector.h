/*!
 *  @file   boatrace_data_collector.h
 *  @brief  競艇データ収集：集権処理
 *  @date   2018/03/06
 */
#pragma once

#include <memory>
#include <string>

namespace boatrace
{

class RaceDataCollector
{
public:
    /*!
     *  @param  start_date  収集開始年月(YYYY/MM)
     *  @param  end_ym      収集終了年月(YYYY/MM)
     */
    RaceDataCollector(const std::string& start_ym, const std::string& end_ym);
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
