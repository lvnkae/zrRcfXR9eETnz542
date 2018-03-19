/*!
 *  @file   boatrace_data.cpp
 *  @brief  競艇データ収集：データ宣言
 *  @date   2018/03/17
 */
#include "boatrace_data.h"

#include "boatrace_data_utility.h"

#include "utility/utility_datetime.h"
#include "yymmdd.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace boatrace
{

namespace
{
const char RACENAME_JSON_FILE[] = "br_name.json";

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
 *  @brief  空か
 */
bool sRaceAtDay::empty() const
{
    return !IsValidRaceID(m_id);
}

/*!
 *  @brief  指定レース場のデータが空か
 *  @param  st_id   レース場ID
 */
bool ScheduleAtMonth::empty(eStadiumID st_id) const
{
    return m_schedule.find(st_id) == m_schedule.end();
}

/*!
 *  @brief  スケジュール登録(最小単位)
 *  @param  month_day   暦日
 *  @param  st_id       レース場ID
 *  @param  rc_type     レース種別
 *  @param  rc_id       レース固有ID
 *  @param  days        開催n日目
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
 *  @brief  当月最終レースを得る
 *  @param  st_id   レース場ID
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
 *  @brief  JSON入力
 *  @param  date    開催スケジュールデータの年月
 *  @param  path    入力パス
 *  @note   入力ファイル名は'br_schedule_yyyymm.json'
 */
void ScheduleAtMonth::input_json(const garnet::YYMM& date, const std::string& path)
{
    const std::string json_file(std::move(create_schedule_json_name(date, path)));
    std::ifstream ifs(json_file);
    if (!ifs.is_open()) {
        return; // JSONファイルがない
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
 *  @brief  JSON出力
 *  @param  date    開催スケジュールデータの年月
 *  @param  path    出力パス
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

/*!
 *  @brief  レース名登録
 *  @param  rc_id   レース固有ID
 *  @param  br_name レース名
 */
void BoatraceNames::entry(uint32_t rc_id, const std::string& br_name)
{
    m_race_name.emplace(rc_id, br_name);
}

/*!
 *  @brief  最終データのレースIDを得る
 */
uint32_t BoatraceNames::get_last_id() const
{
    if (m_race_name.empty()) {
        return BlankRaceID();
    } else {
        return m_race_name.rbegin()->first;
    }
}

/*!
 *  @brief  JSON出力
 *  @param  start_date  収集レースデータ先頭年月
 *  @param  path        出力パス
 */
void BoatraceNames::input_json(const garnet::YYMM& start_date, const std::string& path)
{
    const std::string json_file(path + RACENAME_JSON_FILE);
    std::ifstream ifs(json_file);
    if (!ifs.is_open()) {
        return; // JSONファイルがない
    }

    boost::property_tree::ptree base;
    boost::property_tree::json_parser::read_json(json_file, base);

    boost::optional<std::string> j_start_date_str = base.get_optional<std::string>("Start");
    boost::optional<std::string> j_end_date_str = base.get_optional<std::string>("End");
    const garnet::YYMM j_start_date(std::move(garnet::YYMM::Create(j_start_date_str.get())));
    const garnet::YYMM j_end_date(std::move(garnet::YYMM::Create(j_end_date_str.get())));

    if (j_start_date > start_date || start_date.prev() > j_end_date) {
        // 収集要求先頭年月(A)について
        // ・Aが既存データ始端より過去
        // ・Aの前月が既存データ終端より未来(不連続)
        // になる場合は使い回し不可(再収集)
        return; 
    }

    for (const auto& child: base.get_child("Boatrace")) {
        const boost::property_tree::ptree& names = child.second;
        boost::optional<unsigned int> id = names.get_optional<unsigned int>("id");
        boost::optional<std::string> name = names.get_optional<std::string>("name");
        m_race_name.emplace(id.get(), name.get());
    }
}

/*!
 *  @brief  JSON出力
 *  @param  start_date  収集レースデータ先頭年月
 *  @param  end_date    収集レースデータ末尾年月
 *  @param  path        出力パス
 */
void BoatraceNames::output_json(const garnet::YYMM& start_date,
                                const garnet::YYMM& end_date,
                                const std::string& path) const
{
    boost::property_tree::ptree base;

    base.put("Start", start_date.to_delim_string());
    base.put("End", end_date.to_delim_string());

    boost::property_tree::ptree names;
    for (const auto name: m_race_name) {
        boost::property_tree::ptree name_unit;
        name_unit.put("id", name.first);
        name_unit.put("name", name.second);
        names.push_back(std::make_pair("", name_unit));
    }
    base.add_child("Boatrace", names);
    //
    const std::string filename(path + RACENAME_JSON_FILE);
    boost::property_tree::json_parser::write_json(filename, base);
}

} // namespace boatrace
