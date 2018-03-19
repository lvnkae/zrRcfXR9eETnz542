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
 *  @brief  開催レース情報(1日分)
 */
struct sRaceAtDay
{
    eRaceType m_type;           //!< レース種別
    uint32_t m_id;              //!< レースID
    int32_t m_days;             //!< 開催n日目

    sRaceAtDay();
    sRaceAtDay(eRaceType rc_type, uint32_t rc_id, int32_t days)
    : m_type(rc_type)
    , m_id(rc_id)
    , m_days(days)
    {
    }

    /*!
     *  @brief  空か
     */
    bool empty() const;
};

/*!
 *  @brief  開催スケジュール(1ヶ月分)
 */
class ScheduleAtMonth
{
private:
    //! 1レース場分の月開催スケジュール<暦日, 開催レース情報(1日分)>
    typedef std::map<int32_t, sRaceAtDay> ScheduleInStadium;
    //! 月開催スケジュール<暦日, 開催レース情報(1日分)>
    std::map<eStadiumID, ScheduleInStadium> m_schedule;

public:
    /*!
     *  @brief  データが空か
     *  @param  st_id   レース場ID
     */
    bool empty() const { return m_schedule.empty(); }
    /*!
     *  @brief  指定レース場のデータが空か
     *  @param  st_id   レース場ID
     */
    bool empty(eStadiumID st_id) const;
    /*!
     *  @brief  スケジュール登録(最小単位)
     *  @param  month_day   暦日
     *  @param  st_id       レース場ID
     *  @param  rc_type     レース種別
     *  @param  rc_id       レース固有ID
     *  @param  days        開催n日目
     *  @note   任意のレース場における1日分の開催情報を登録する
     */
    void entry(int32_t month_day,
               eStadiumID st_id,
               eRaceType rc_type,
               uint32_t rc_id,
               int32_t days);

    /*!
     *  @brief  当月最終レースを得る
     *  @param  st_id   レース場ID
     */
    sRaceAtDay get_last_race(eStadiumID st_id) const;

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
     *  @note   出力ファイル名は'br_schedule_yyyymm.json'
     */
    void output_json(const garnet::YYMM& date, const std::string& path) const;
};


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

} // namespace boatrace
