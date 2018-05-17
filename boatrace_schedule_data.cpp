/*!
 *  @file   boatrace_data.cpp
 *  @brief  �����f�[�^���W�F�X�P�W���[���f�[�^�錾
 *  @date   2018/04/13
 */
#include "boatrace_schedule_data.h"

#include "boatrace_data_utility.h"

#include "yymmdd.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace boatrace
{

namespace
{

std::string create_schedule_json_name(const garnet::YYMM& date, const std::string& path)
{
    return path + "schedule_" + date.to_string() + ".json";
}

}


/*!
 */
sRaceAtDay::sRaceAtDay()
: m_type(RACE_NONE)
, m_id(BlankRaceID())
, m_days(0)
{
}

/*!
 *  @brief  ��
 */
bool sRaceAtDay::empty() const
{
    return !IsValidRaceID(m_id);
}

/*!
 *  @brief  �w�背�[�X��̃f�[�^����
 *  @param  st_id   ���[�X��ID
 */
bool ScheduleAtMonth::empty(eStadiumID st_id) const
{
    return m_schedule.find(st_id) == m_schedule.end();
}

/*!
 *  @brief  �X�P�W���[���o�^(�ŏ��P��)
 *  @param  month_day   ���
 *  @param  st_id       ���[�X��ID
 *  @param  rc_type     ���[�X���
 *  @param  rc_id       ���[�X�ŗLID
 *  @param  days        �J��n����
 */
void ScheduleAtMonth::entry(int32_t month_day,
                            eStadiumID st_id,
                            eRaceType rc_type,
                            uint32_t rc_id,
                            int32_t days)
{
    m_schedule[st_id].emplace(month_day, std::move(sRaceAtDay(rc_type, rc_id, days)));
}

/*!
 *  @brief  �����ŏI���[�X�𓾂�
 *  @param  st_id   ���[�X��ID
 */
sRaceAtDay ScheduleAtMonth::get_last_race(eStadiumID st_id) const
{
    const auto st_it = m_schedule.find(st_id);
    if (st_it != m_schedule.end()) {
        if (!st_it->second.empty()) {
            return st_it->second.rbegin()->second;
        }
    }
    return sRaceAtDay();
}

/*!
 *  @brief  �w����̊J�ÃX�P�W���[���𓾂�
 *  @param[in]  month_day   ���
 *  @param[out] o_schedule  �J�ÃX�P�W���[��(�i�[��)
 */
void ScheduleAtMonth::get_at_day(int32_t month_day, ScheduleAtDay& o_schedule) const
{
    for(const auto& it_sch: m_schedule) {
        const auto it = it_sch.second.find(month_day);
        if (it != it_sch.second.end()) {
            o_schedule.entry(it_sch.first, it->second.m_type);
        }
    }
}

/*!
 *  @brief  ���[�X��ʂ��Ƃ̐�(�J�Ó���)�𐔂���
 *  @param[out] counter     �J�E���^
 */
void ScheduleAtMonth::count_race_by_type(std::map<eRaceType, int32_t>& counter) const
{
    for (const auto& it_sch: m_schedule) {
        for (const auto& it_day: it_sch.second) {
            auto it = counter.find(it_day.second.m_type);
            if (it == counter.end()) {
                counter.emplace(it_day.second.m_type, 1);
            } else {
                it->second++;
            }
        }
    }
}

/*!
 *  @brief  JSON����
 *  @param  date    �J�ÃX�P�W���[���f�[�^�̔N��
 *  @param  path    ���̓p�X
 *  @note   ���̓t�@�C������'br_schedule_yyyymm.json'
 */
void ScheduleAtMonth::input_json(const garnet::YYMM& date, const std::string& path)
{
    const std::string json_file(std::move(create_schedule_json_name(date, path)));
    std::ifstream ifs(json_file);
    if (!ifs.is_open()) {
        return; // JSON�t�@�C�����Ȃ�
    }

    boost::property_tree::ptree base;
    boost::property_tree::json_parser::read_json(json_file, base);

    boost::optional<int> year = base.get_optional<int>("Year");
    boost::optional<int> month = base.get_optional<int>("Month");
    if (date.m_year != year.get() || date.m_month != month.get()) {
        return;
    }

    for (const auto& child_st: base.get_child("Schedule")) {
        ScheduleInStadium st_schedule;
        const boost::property_tree::ptree& pt_schedule = child_st.second;
        boost::optional<int> st_id = pt_schedule.get_optional<int>("StadiumID");
        for (const auto& child_race: pt_schedule.get_child("Race")) {
            const boost::property_tree::ptree& pt_race = child_race.second;
            boost::optional<int> mon_day = pt_race.get_optional<int>("date");
            boost::optional<unsigned int> rc_id = pt_race.get_optional<unsigned int>("id");
            boost::optional<int> rc_type = pt_race.get_optional<int>("type");
            boost::optional<int> days = pt_race.get_optional<int>("days");
            st_schedule.emplace(mon_day.get(),
                std::move(sRaceAtDay(static_cast<eRaceType>(rc_type.get()),
                                     rc_id.get(),
                                     days.get())));
        }
        m_schedule.emplace(static_cast<eStadiumID>(st_id.get()), std::move(st_schedule));
    }
}

/*!
 *  @brief  JSON�o��
 *  @param  date    �J�ÃX�P�W���[���f�[�^�̔N��
 *  @param  path    �o�̓p�X
 */
void ScheduleAtMonth::output_json(const garnet::YYMM& date, const std::string& path) const
{
    boost::property_tree::ptree base;

    base.put("Year", date.m_year);
    base.put("Month", date.m_month);

    boost::property_tree::ptree schedule;
    for (const auto st_data: m_schedule) {
        boost::property_tree::ptree stadium;
        stadium.put("StadiumID", st_data.first);
        boost::property_tree::ptree st_race;
        for (const auto race: st_data.second) {
            boost::property_tree::ptree race_unit;
            race_unit.put("date", race.first);
            race_unit.put("id",   race.second.m_id);
            race_unit.put("type", race.second.m_type);
            race_unit.put("days", race.second.m_days);
            st_race.push_back(std::make_pair("", race_unit));
        }
        stadium.add_child("Race", st_race);
        schedule.push_back(std::make_pair("", stadium));
    }
    base.add_child("Schedule", schedule);
    //
    const std::string filename(std::move(create_schedule_json_name(date, path)));
    boost::property_tree::json_parser::write_json(filename, base);
}

} // namespace boatrace
