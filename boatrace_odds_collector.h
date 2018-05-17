/*!
 *  @file   boatrace_odds_collector.h
 *  @brief  �����f�[�^���W�F�I�b�Y�f�[�^���W
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
     *  @param  start_date  ���W�J�n�N��(YYYY/MM/DD)
     *  @param  end_date    ���W�I���N��(YYYY/MM/DD)
     *  @param  race_type   ���W���[�X�^�C�v
     */
    OddsCollector(const std::string& start_date,
                  const std::string& end_date,
                  const std::unordered_set<eRaceType>& race_type);
    /*!
     */
    ~OddsCollector();

    /*!
     *  @param  tickCount   �o�ߎ���[�~���b]
     *  @retval true        ���W��������
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
