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

std::string create_odds_json_name(const garnet::YYMM& date, const std::string& path)
{
    return path + "odds_" + date.to_string() + ".json";
}

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
 *  @brief  JSON入力
 *  @param  start_date  収集レースデータ先頭年月
 *  @param  path        入力パス
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

/*!
 *  @brief  登録
 *  @param  month_day   暦日
 *  @param  st_id       レース場ID
 *  @param  rc_no       レース番号(第nレース)
 *  @param  rc_odds     オッズデータ(1レース分)
 */
void OddsAtMonth::entry(int32_t month_day, eStadiumID st_id, int32_t rc_no, sOddsAtRace&& rc_odds)
{
    m_odds[month_day][st_id].emplace(rc_no, rc_odds);
}

/*!
 *  @brief  存在確認
 *  @param  month_day   暦日
 *  @param  st_id       レース場ID
 *  @param  rc_no       レース番号(第nレース)
 */
bool OddsAtMonth::exists(int32_t month_day, eStadiumID st_id, int32_t rc_no) const
{
    const auto it_day = m_odds.find(month_day);
    if (it_day == m_odds.end()) {
        return false;
    }
    const auto it_st = it_day->second.find(st_id);
    if (it_st == it_day->second.end()) {
        return false;
    }
    return it_st->second.find(rc_no) != it_st->second.end();
}

/*!
 *  @brief  JSON入力
 *  @param  date    開催スケジュールデータの年月
 *  @param  path    入力パス
 *  @note   入力ファイル名は'br_schedule_yyyymm.json'
 */
void OddsAtMonth::input_json(const garnet::YYMM& date, const std::string& path)
{
    const std::string json_file(std::move(create_odds_json_name(date, path)));
    std::ifstream ifs(json_file);
    if (!ifs.is_open()) {
        return; // JSONファイルがない
    }

    boost::property_tree::ptree base;
    boost::property_tree::json_parser::read_json(json_file, base);

    const boost::optional<int> year = base.get_optional<int>("Year");
    const boost::optional<int> month = base.get_optional<int>("Month");
    if (date.m_year != year.get() || date.m_month != month.get()) {
        return;
    }

    for (const auto& pr_date: base.get_child("OddsData")) {
        const boost::property_tree::ptree& tr_date = pr_date.second;
        const boost::optional<int32_t> day = tr_date.get_optional<int32_t>("date");
        for (const auto& pr_stadium: tr_date.get_child("StadiumOdds")) {
            const boost::property_tree::ptree& tr_stadium = pr_stadium.second;
            const boost::optional<int32_t> i_stadium_id
                = tr_stadium.get_optional<int32_t>("StadiumID");
            for (const auto& pr_race: tr_stadium.get_child("RaceOdds")) {
                sOddsAtRace rc_odds;
                //
                const boost::property_tree::ptree& tr_race = pr_race.second;
                const boost::optional<int32_t> race_number
                    = tr_race.get_optional<int32_t>("RaceNumber");
                //
                for (const auto& pr_wins3: tr_race.get_child("Wins3")) {
                    const boost::property_tree::ptree& tr_wins3 = pr_wins3.second;
                    const boost::optional<float32> odds = tr_wins3.get_optional<float32>("Odds");
                    //
                    const auto tr_combination_all = tr_wins3.get_child("Combination");
                    const size_t WINS3_NUM = 3;
                    if (tr_combination_all.size() == WINS3_NUM) {
                        int32_t c[WINS3_NUM];
                        int32_t c_inx = 0;
                        for (const auto& pr_combination: tr_combination_all) {
                            c[c_inx++] = pr_combination.second.get_value<int32_t>();
                        }
                        rc_odds.m_data[c[0]].m_wins3[c[1]].emplace(c[2], odds.get());
                    } else {
                        PRINT_MESSAGE("illegal wins3 combination (" + date.to_string()
                                       + "/" + std::to_string(day.get())
                                       + " st:" + std::to_string(i_stadium_id.get()) 
                                       + " rc:" + std::to_string(race_number.get())
                                       + " odds:" + std::to_string(odds.get())
                                       + ")");
                    }
                }
                for (const auto& pr_wins2: tr_race.get_child("Wins2")) {
                    const boost::property_tree::ptree& tr_wins2 = pr_wins2.second;
                    const boost::optional<float32> odds = tr_wins2.get_optional<float32>("Odds");
                    //
                    const auto tr_combination_all = tr_wins2.get_child("Combination");
                    const size_t WINS2_NUM = 2;
                    if (tr_combination_all.size() == WINS2_NUM) {
                        int32_t c[WINS2_NUM];
                        int32_t c_inx = 0;
                        for (const auto& pr_combination: tr_combination_all) {
                            c[c_inx++] = pr_combination.second.get_value<int32_t>();
                        }
                        rc_odds.m_data[c[0]].m_wins2.emplace(c[1], odds.get());
                    } else {
                        PRINT_MESSAGE("illegal wins2 combination (" + date.to_string()
                                       + "/" + std::to_string(day.get())
                                       + " st:" + std::to_string(i_stadium_id.get()) 
                                       + " rc:" + std::to_string(race_number.get())
                                       + " odds:" + std::to_string(odds.get())
                                       + ")");
                    }
                }
                for (const auto& pr_place3: tr_race.get_child("Place3")) {
                    const boost::property_tree::ptree& tr_place3 = pr_place3.second;
                    const boost::optional<float32> odds = tr_place3.get_optional<float32>("Odds");
                    //
                    const auto tr_combination_all = tr_place3.get_child("Combination");
                    const size_t PLACE3_NUM = 3;
                    if (tr_combination_all.size() == PLACE3_NUM) {
                        int32_t c[PLACE3_NUM];
                        int32_t c_inx = 0;
                        for (const auto& pr_combination: tr_combination_all) {
                            c[c_inx++] = pr_combination.second.get_value<int32_t>();
                        }
                        rc_odds.m_data[c[0]].m_place3[c[1]].emplace(c[2], odds.get());
                    } else {
                        PRINT_MESSAGE("illegal place3 combination (" + date.to_string()
                                       + "/" + std::to_string(day.get())
                                       + " st:" + std::to_string(i_stadium_id.get()) 
                                       + " rc:" + std::to_string(race_number.get())
                                       + " odds:" + std::to_string(odds.get())
                                       + ")");
                    }
                }
                for (const auto& pr_place2: tr_race.get_child("Place2")) {
                    const boost::property_tree::ptree& tr_place2 = pr_place2.second;
                    const boost::optional<float32> odds = tr_place2.get_optional<float32>("Odds");
                    //
                    const auto tr_combination_all = tr_place2.get_child("Combination");
                    const size_t PLACE2_NUM = 2;
                    if (tr_combination_all.size() == PLACE2_NUM) {
                        int32_t c[PLACE2_NUM];
                        int32_t c_inx = 0;
                        for (const auto& pr_combination: tr_combination_all) {
                            c[c_inx++] = pr_combination.second.get_value<int32_t>();
                        }
                        rc_odds.m_data[c[0]].m_place2.emplace(c[1], odds.get());
                    } else {
                        PRINT_MESSAGE("illegal place2 combination (" + date.to_string()
                                       + "/" + std::to_string(day.get())
                                       + " st:" + std::to_string(i_stadium_id.get()) 
                                       + " rc:" + std::to_string(race_number.get())
                                       + " odds:" + std::to_string(odds.get())
                                       + ")");
                    }
                }
                //
                const eStadiumID stadium_id = static_cast<eStadiumID>(i_stadium_id.get());
                entry(day.get(), stadium_id, race_number.get(), std::move(rc_odds));
            }
        }
    }
}

/*!
 *  @brief  JSON出力
 *  @param  date    開催スケジュールデータの年月
 *  @param  path    出力パス
 */
void OddsAtMonth::output_json(const garnet::YYMM& date, const std::string& path) const
{
    boost::property_tree::ptree base;

    base.put("Year", date.m_year);
    base.put("Month", date.m_month);

    boost::property_tree::ptree tr_odds;
    for (const auto day_odds: m_odds) {
        boost::property_tree::ptree tr_day;
        tr_day.put("date", day_odds.first);
        boost::property_tree::ptree tr_stadium_all;
        for (const auto stadium_odds: day_odds.second) {
            boost::property_tree::ptree tr_stadium;
            tr_stadium.put("StadiumID", stadium_odds.first);
            boost::property_tree::ptree tr_race_all;
            for (const auto race_odds: stadium_odds.second) {
                boost::property_tree::ptree tr_race;
                tr_race.put("RaceNumber", race_odds.first);
                boost::property_tree::ptree tr_wins3;
                boost::property_tree::ptree tr_wins2;
                boost::property_tree::ptree tr_place3;
                boost::property_tree::ptree tr_place2;
                for (const auto waku_odds: race_odds.second.m_data) {
                    const int32_t combination_first = waku_odds.first;
                    for (const auto w3_second: waku_odds.second.m_wins3) {
                        for (const auto w3_third: w3_second.second) {
                            boost::property_tree::ptree tr_combination;
                            {
                                boost::property_tree::ptree tr_cmb_val;
                                tr_cmb_val.put("", combination_first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                                tr_cmb_val.put("", w3_second.first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                                tr_cmb_val.put("", w3_third.first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                            }
                            boost::property_tree::ptree tr_odds_unit;
                            tr_odds_unit.add_child("Combination", tr_combination);
                            tr_odds_unit.put("Odds", w3_third.second);
                            tr_wins3.push_back(std::make_pair("", tr_odds_unit));
                        }
                    }
                    for (const auto w2_second: waku_odds.second.m_wins2) {
                        boost::property_tree::ptree tr_combination;
                        {
                            boost::property_tree::ptree tr_cmb_val;
                            tr_cmb_val.put("", combination_first);
                            tr_combination.push_back(std::make_pair("", tr_cmb_val));
                            tr_cmb_val.put("", w2_second.first);
                            tr_combination.push_back(std::make_pair("", tr_cmb_val));
                        }
                        boost::property_tree::ptree tr_odds_unit;
                        tr_odds_unit.add_child("Combination", tr_combination);
                        tr_odds_unit.put("Odds", w2_second.second);
                        tr_wins2.push_back(std::make_pair("", tr_odds_unit));
                    }
                    for (const auto p3_second: waku_odds.second.m_place3) {
                        for (const auto p3_third: p3_second.second) {
                            boost::property_tree::ptree tr_combination;
                            {
                                boost::property_tree::ptree tr_cmb_val;
                                tr_cmb_val.put("", combination_first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                                tr_cmb_val.put("", p3_second.first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                                tr_cmb_val.put("", p3_third.first);
                                tr_combination.push_back(std::make_pair("", tr_cmb_val));
                            }
                            boost::property_tree::ptree tr_odds_unit;
                            tr_odds_unit.add_child("Combination", tr_combination);
                            tr_odds_unit.put("Odds", p3_third.second);
                            tr_place3.push_back(std::make_pair("", tr_odds_unit));
                        }
                    }
                    for (const auto p2_second: waku_odds.second.m_place2) {
                        boost::property_tree::ptree tr_combination;
                        {
                            boost::property_tree::ptree tr_cmb_val;
                            tr_cmb_val.put("", combination_first);
                            tr_combination.push_back(std::make_pair("", tr_cmb_val));
                            tr_cmb_val.put("", p2_second.first);
                            tr_combination.push_back(std::make_pair("", tr_cmb_val));
                        }
                        boost::property_tree::ptree tr_odds_unit;
                        tr_odds_unit.add_child("Combination", tr_combination);
                        tr_odds_unit.put("Odds", p2_second.second);
                        tr_place2.push_back(std::make_pair("", tr_odds_unit));
                    }
                }
                tr_race.add_child("Wins3", tr_wins3);
                tr_race.add_child("Wins2", tr_wins2);
                tr_race.add_child("Place3", tr_place3);
                tr_race.add_child("Place2", tr_place2);
                tr_race_all.push_back(std::make_pair("", tr_race));
            }
            tr_stadium.add_child("RaceOdds", tr_race_all);
            tr_stadium_all.push_back(std::make_pair("", tr_stadium));
        }
        tr_day.add_child("StadiumOdds", tr_stadium_all);
        tr_odds.push_back(std::make_pair("", tr_day));
    }
    base.add_child("OddsData", tr_odds);
    //
    const std::string filename(std::move(create_odds_json_name(date, path)));
    boost::property_tree::json_parser::write_json(filename, base);
}

} // namespace boatrace
