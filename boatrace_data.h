/*!
 *  @file   boatrace_data.h
 *  @brief  競艇データ収集：データ宣言
 *  @date   2018/03/17
 */
#pragma once

#include "boatrace_data_define.h"

#include <map>

namespace garnet { struct YYMM; }

namespace boatrace
{

/*!
 *  @brief  ボートレース名群
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
     *  @brief  データが空か
     */
    bool empty() const { return m_race_name.empty(); }
    /*!
     *  @brief  レース名登録
     *  @param  rc_id   レース固有ID
     *  @param  br_name レース名
     */
    void entry(uint32_t rc_id, const std::string& br_name);

    /*!
     *  @brief  最終データのレースIDを得る
     */
    uint32_t get_last_id() const;

    /*!
     *  @brief  JSON入力
     *  @param  start_date  収集レースデータ先頭年月
     *  @param  path        入力パス
     *  @note   入力ファイル名は'br_name.json'
     */
    void input_json(const garnet::YYMM& start_date, const std::string& path);
    /*!
     *  @brief  JSON出力
     *  @param  start_date  収集レースデータ先頭年月
     *  @param  end_date    収集レースデータ末尾年月
     *  @param  path        出力パス
     *  @note   出力ファイル名は'br_name.json'
     */
    void output_json(const garnet::YYMM& start_date,
                     const garnet::YYMM& end_date,
                     const std::string& path) const;
};

/*!
 *  @brief  オッズデータ(1出走者(枠)分)
 */
struct sOdds
{
    //! 3連単<枠番, <枠番, オッズ>>
    std::map<int32_t, std::map<int32_t, float32>> m_wins3;
    //! 2連単<枠番, オッズ>
    std::map<int32_t, float32> m_wins2;
    //! 3連複<枠番, <枠番, オッズ>>
    std::map<int32_t, std::map<int32_t, float32>> m_place3;
    //! 2連複<枠番, オッズ>
    std::map<int32_t, float32> m_place2;
};

/*!
 *  @brief  オッズデータ(1レース分)
 */
struct sOddsAtRace
{
    //! <枠番, 1出走者(枠)分オッズ>
    std::map<int32_t, sOdds> m_data;    
};

/*!
 *  @brief  オッズデータ(一か月分)
 */
class OddsAtMonth
{
private:
    //! 1開場分のオッズデータ<レース番号(第nレース), オッズデータ(1レース分)>
    typedef std::map<int32_t, sOddsAtRace> OddsAtStadium;
    //! 1日分のオッズデータ<レース場ID, オッズデータ(1開場分)>
    typedef std::map<eStadiumID, OddsAtStadium> OddsAtDay;
    //! 1ヶ月分のオッズデータ<暦日, オッズデータ(1日分)>
    std::map<int32_t, OddsAtDay> m_odds;

public:
    /*!
     *  @brief  データが空か
     */
    bool empty() const { return m_odds.empty(); }
    /*!
     *  @brief  存在確認
     *  @param  month_day   暦日
     *  @param  st_id       レース場ID
     *  @param  rc_no       レース番号(第nレース)
     *  @retval false       存在しない
     */
    bool exists(int32_t month_day, eStadiumID st_id, int32_t rc_no) const;

    /*!
     *  @brief  登録
     *  @param  month_day   暦日
     *  @param  st_id       レース場ID
     *  @param  rc_no       レース番号(第nレース)
     *  @param  rc_odds     オッズデータ(1レース分)
     */
    void entry(int32_t month_day, eStadiumID st_id, int32_t rc_no, sOddsAtRace&& rc_odds);

    /*!
     *  @brief  JSON入力
     *  @param  date    開催スケジュールデータの年月
     *  @param  path    入力パス
     *  @note   入力ファイル名は'br_schedule_yyyymm.json'
     */
    void input_json(const garnet::YYMM& date, const std::string& path);
    /*!
     *  @brief  JSON出力
     *  @param  date    開催スケジュールデータの年月
     *  @param  path    出力パス
     *  @note   出力ファイル名は'br_odds_yyyymm.json'
     */
    void output_json(const garnet::YYMM& date, const std::string& path) const;
};

} // namespace boatrace
