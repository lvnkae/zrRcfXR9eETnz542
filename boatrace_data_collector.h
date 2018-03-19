/*!
 *  @file   boatrace_data_collector.h
 *  @brief  �����f�[�^���W�F�W������
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
     *  @param  start_date  ���W�J�n�N��(YYYY/MM)
     *  @param  end_ym      ���W�I���N��(YYYY/MM)
     */
    RaceDataCollector(const std::string& start_ym, const std::string& end_ym);
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
