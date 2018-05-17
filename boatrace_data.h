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

/*!
 *  @brief  �I�b�Y�f�[�^(1�o����(�g)��)
 */
struct sOdds
{
    //! 3�A�P<�g��, <�g��, �I�b�Y>>
    std::map<int32_t, std::map<int32_t, float32>> m_wins3;
    //! 2�A�P<�g��, �I�b�Y>
    std::map<int32_t, float32> m_wins2;
    //! 3�A��<�g��, <�g��, �I�b�Y>>
    std::map<int32_t, std::map<int32_t, float32>> m_place3;
    //! 2�A��<�g��, �I�b�Y>
    std::map<int32_t, float32> m_place2;
};

/*!
 *  @brief  �I�b�Y�f�[�^(1���[�X��)
 */
struct sOddsAtRace
{
    //! <�g��, 1�o����(�g)���I�b�Y>
    std::map<int32_t, sOdds> m_data;    
};

/*!
 *  @brief  �I�b�Y�f�[�^(�ꂩ����)
 */
class OddsAtMonth
{
private:
    //! 1�J�ꕪ�̃I�b�Y�f�[�^<���[�X�ԍ�(��n���[�X), �I�b�Y�f�[�^(1���[�X��)>
    typedef std::map<int32_t, sOddsAtRace> OddsAtStadium;
    //! 1�����̃I�b�Y�f�[�^<���[�X��ID, �I�b�Y�f�[�^(1�J�ꕪ)>
    typedef std::map<eStadiumID, OddsAtStadium> OddsAtDay;
    //! 1�������̃I�b�Y�f�[�^<���, �I�b�Y�f�[�^(1����)>
    std::map<int32_t, OddsAtDay> m_odds;

public:
    /*!
     *  @brief  �f�[�^����
     */
    bool empty() const { return m_odds.empty(); }
    /*!
     *  @brief  ���݊m�F
     *  @param  month_day   ���
     *  @param  st_id       ���[�X��ID
     *  @param  rc_no       ���[�X�ԍ�(��n���[�X)
     *  @retval false       ���݂��Ȃ�
     */
    bool exists(int32_t month_day, eStadiumID st_id, int32_t rc_no) const;

    /*!
     *  @brief  �o�^
     *  @param  month_day   ���
     *  @param  st_id       ���[�X��ID
     *  @param  rc_no       ���[�X�ԍ�(��n���[�X)
     *  @param  rc_odds     �I�b�Y�f�[�^(1���[�X��)
     */
    void entry(int32_t month_day, eStadiumID st_id, int32_t rc_no, sOddsAtRace&& rc_odds);

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
     *  @note   �o�̓t�@�C������'br_odds_yyyymm.json'
     */
    void output_json(const garnet::YYMM& date, const std::string& path) const;
};

} // namespace boatrace
