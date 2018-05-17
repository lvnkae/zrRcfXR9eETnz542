/*!
 *  @file   boatrace_data_collector.h
 *  @brief  �����f�[�^���W�F�W������
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
     *  @param  sch_start_ym    �J�ÃX�P�W���[�����W�J�n�N��(YYYY/MM)
     *  @param  sch_end_ym      �J�ÃX�P�W���[�����W�I���N��(YYYY/MM)
     *  @param  dat_start_ym    ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
     *  @param  dat_end_ym      ���[�X�f�[�^���W�J�n�N��(YYYY/MM)
     *  @param  race_type       �f�[�^���W�Ώۃ��[�X�^�C�v
     */
    RaceDataCollector(const std::string& sch_start_ym,
                      const std::string& sch_end_ym,
                      const std::string& dat_start_ym,
                      const std::string& dat_end_ym,
                      const std::unordered_set<eRaceType>& race_type);
    ~RaceDataCollector();

    /*!
     *  @retval true    �I�����Ă悢
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
