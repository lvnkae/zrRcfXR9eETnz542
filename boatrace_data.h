/*!
 *  @file   boatrace_data.h
 *  @brief  �����f�[�^���W�F�f�[�^�錾
 *  @date   2018/03/17
 */
#pragma once

#include "boatrace_data_define.h"

#include <map>

namespace garnet { struct YYMM; }

namespace boatrace
{

/*!
 *  @brief  �J�Ã��[�X���(1����)
 */
struct sRaceAtDay
{
    eRaceType m_type;           //!< ���[�X���
    uint32_t m_id;              //!< ���[�XID
    int32_t m_days;             //!< �J��n����

    sRaceAtDay();
    sRaceAtDay(eRaceType rc_type, uint32_t rc_id, int32_t days)
    : m_type(rc_type)
    , m_id(rc_id)
    , m_days(days)
    {
    }

    /*!
     *  @brief  ��
     */
    bool empty() const;
};

/*!
 *  @brief  �J�ÃX�P�W���[��(1������)
 */
class ScheduleAtMonth
{
private:
    //! 1���[�X�ꕪ�̌��J�ÃX�P�W���[��<���, �J�Ã��[�X���(1����)>
    typedef std::map<int32_t, sRaceAtDay> ScheduleInStadium;
    //! ���J�ÃX�P�W���[��<���, �J�Ã��[�X���(1����)>
    std::map<eStadiumID, ScheduleInStadium> m_schedule;

public:
    /*!
     *  @brief  �f�[�^����
     *  @param  st_id   ���[�X��ID
     */
    bool empty() const { return m_schedule.empty(); }
    /*!
     *  @brief  �w�背�[�X��̃f�[�^����
     *  @param  st_id   ���[�X��ID
     */
    bool empty(eStadiumID st_id) const;
    /*!
     *  @brief  �X�P�W���[���o�^(�ŏ��P��)
     *  @param  month_day   ���
     *  @param  st_id       ���[�X��ID
     *  @param  rc_type     ���[�X���
     *  @param  rc_id       ���[�X�ŗLID
     *  @param  days        �J��n����
     *  @note   �C�ӂ̃��[�X��ɂ�����1�����̊J�Ï���o�^����
     */
    void entry(int32_t month_day,
               eStadiumID st_id,
               eRaceType rc_type,
               uint32_t rc_id,
               int32_t days);

    /*!
     *  @brief  �����ŏI���[�X�𓾂�
     *  @param  st_id   ���[�X��ID
     */
    sRaceAtDay get_last_race(eStadiumID st_id) const;

    /*!
     *  @brief  JSON����
     *  @param  date    �J�ÃX�P�W���[���f�[�^�̔N��
     *  @param  path    ���̓p�X
     *  @note   ���̓t�@�C������'br_schedule_yyyymm.json'
     */
    void input_json(const garnet::YYMM& date, const std::string& path);
    /*!
     *  @brief  JSON�o��
     *  @param  date    �J�ÃX�P�W���[���f�[�^�̔N��
     *  @param  path    �o�̓p�X
     *  @note   �o�̓t�@�C������'br_schedule_yyyymm.json'
     */
    void output_json(const garnet::YYMM& date, const std::string& path) const;
};


/*!
 *  @brief  �{�[�g���[�X���Q
 */
class BoatraceNames
{
private:
    BoatraceNames(const BoatraceNames&);
    BoatraceNames(BoatraceNames&&);
    BoatraceNames& operator= (const BoatraceNames&);

    std::map<uint32_t, std::string> m_race_name;

public:
    BoatraceNames()
    : m_race_name()
    {
    }

    /*!
     *  @brief  �f�[�^����
     */
    bool empty() const { return m_race_name.empty(); }
    /*!
     *  @brief  ���[�X���o�^
     *  @param  rc_id   ���[�X�ŗLID
     *  @param  br_name ���[�X��
     */
    void entry(uint32_t rc_id, const std::string& br_name);

    /*!
     *  @brief  �ŏI�f�[�^�̃��[�XID�𓾂�
     */
    uint32_t get_last_id() const;

    /*!
     *  @brief  JSON����
     *  @param  start_date  ���W���[�X�f�[�^�擪�N��
     *  @param  path        ���̓p�X
     *  @note   ���̓t�@�C������'br_name.json'
     */
    void input_json(const garnet::YYMM& start_date, const std::string& path);
    /*!
     *  @brief  JSON�o��
     *  @param  start_date  ���W���[�X�f�[�^�擪�N��
     *  @param  end_date    ���W���[�X�f�[�^�����N��
     *  @param  path        �o�̓p�X
     *  @note   �o�̓t�@�C������'br_name.json'
     */
    void output_json(const garnet::YYMM& start_date,
                     const garnet::YYMM& end_date,
                     const std::string& path) const;
};

} // namespace boatrace
