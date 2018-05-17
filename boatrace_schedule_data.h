/*!
 *  @file   boatrace_schedule_data.h
 *  @brief  �����f�[�^���W�F�X�P�W���[���f�[�^�錾
 *  @date   2018/04/13
 */
#pragma once

#include "boatrace_data_define.h"

#include <map>
#include <deque>

namespace garnet { struct YYMM; }

namespace boatrace
{

/*!
 *  @brief  1�����̊J�ÃX�P�W���[��
 *  @note   ScheduleCollector��OddsCollector�󂯓n���p
 */
class ScheduleAtDay
{
public:
    struct sUnit
    {
        eStadiumID m_stadium;   //!< ���[�X��
        eRaceType m_type;       //!< ���[�X���

        sUnit(eStadiumID stadium, eRaceType rc_type)
        : m_stadium(stadium)
        , m_type(rc_type)
        {
        }
    };

    /*!
     *  @retval true    ��
     */
    bool empty() const
    {
        return m_schedule.empty();
    }
    /*!
     *  @brief  �o�^
     *  @param  stadium ���[�X��
     *  @param  rc_type ���[�X���
     */
    void entry(eStadiumID stadium, eRaceType rc_type)
    {
        m_schedule.emplace_back(stadium, rc_type);
    }
    /*!
     *  @brief  �擪�v�f�����o��
     *  @note   �擾&�폜
     */
    sUnit front_and_pop()
    {
        sUnit ret = m_schedule.front();
        m_schedule.pop_front();
        return ret;
    }

private:
    std::deque<sUnit> m_schedule;
};

/*!
 *  @brief  �J�Ã��[�X���(1����)
 *  @note   �ۑ��`��
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
 *  @note   �ۑ��`��
 */
class ScheduleAtMonth
{
private:
    //! 1���[�X�ꕪ�̌��J�ÃX�P�W���[��<���, �J�Ã��[�X���(1����)>
    typedef std::map<int32_t, sRaceAtDay> ScheduleInStadium;
    //! ���J�ÃX�P�W���[��<���[�X��, �J�Ã��[�X���(1����)>
    std::map<eStadiumID, ScheduleInStadium> m_schedule;

public:
    /*!
     *  @brief  �f�[�^����
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
     *  @brief  �w����̊J�ÃX�P�W���[���𓾂�
     *  @param[in]  month_day         ���
     *  @param[out] o_schedule  �J�ÃX�P�W���[��(�i�[��)
     */
    void get_at_day(int32_t month_day, ScheduleAtDay& o_schedule) const;
    /*!
     *  @brief  ���[�X��ʂ��Ƃ̐�(�J�Ó���)�𐔂���
     *  @param[out] counter     �J�E���^
     */
    void count_race_by_type(std::map<eRaceType, int32_t>& counter) const;

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



} // namespace boatrace
