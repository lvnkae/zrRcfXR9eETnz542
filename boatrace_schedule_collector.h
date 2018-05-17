/*!
 *  @file   boatrace_schedule_collector.h
 *  @brief  競艇データ収集：開催スケジュールデータ収集
 *  @date   2018/03/07
 */
#pragma once

#include <memory>
#include <string>

namespace garnet { struct YYMMDD; }

namespace boatrace
{
class ScheduleAtDay;

class ScheduleCollector
{
public:
    /*!
     *  @param  start_date  収集開始年月(YYYY/MM)
     *  @param  end_ym      収集終了年月(YYYY/MM)
     */
    ScheduleCollector(const std::string& start_ym, const std::string& end_ym);
    /*!
     */
    ~ScheduleCollector();

    /*!
     *  @param  tickCount   経過時間[ミリ秒]
     *  @retval true        収集処理完了
     */
    bool Update(int64_t tickCount);

    /*!
     *  @brief  1日分の開催スケジュールを得る
     *  @param[in]  date        開催年月日
     *  @param[out] o_schedule  開催スケジュール
     */
    void GetScheduleAtDay(const garnet::YYMMDD& date, ScheduleAtDay& o_schedule) const;


private:
    ScheduleCollector();
    ScheduleCollector(const ScheduleCollector&);
    ScheduleCollector(ScheduleCollector&&);
    ScheduleCollector& operator= (const ScheduleCollector&);

    class PIMPL;
    std::unique_ptr<PIMPL> m_pImpl;
};

} // namespace boatrace
