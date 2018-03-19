/*!
 *  @file   boatrace_schedule_collector.h
 *  @brief  �����f�[�^���W�F�J�ÃX�P�W���[���f�[�^���W
 *  @date   2018/03/07
 */
#pragma once

#include <memory>
#include <string>

namespace boatrace
{

class ScheduleCollector
{
public:
    /*!
     *  @param  start_date  ���W�J�n�N��(YYYY/MM)
     *  @param  end_ym      ���W�I���N��(YYYY/MM)
     */
    ScheduleCollector(const std::string& start_ym, const std::string& end_ym);
    /*!
     */
    ~ScheduleCollector();

    /*!
     *  @param  tickCount   �o�ߎ���[�~���b]
     *  @retval true        ���W��������
     */
    bool Update(int64_t tickCount);

private:
    ScheduleCollector();
    ScheduleCollector(const ScheduleCollector&);
    ScheduleCollector(ScheduleCollector&&);
    ScheduleCollector& operator= (const ScheduleCollector&);

    class PIMPL;
    std::unique_ptr<PIMPL> m_pImpl;
};

} // namespace boatrace