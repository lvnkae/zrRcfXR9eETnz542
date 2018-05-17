/*!
 *  @file   boatrace_schedule_data.h
 *  @brief  競艇データ収集：スケジュールデータ宣言
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
 *  @brief  1日分の開催スケジュール
 *  @note   ScheduleCollector→OddsCollector受け渡し用
 */
class ScheduleAtDay
{
public:
    struct sUnit
    {
        eStadiumID m_stadium;   //!< レース場
        eRaceType m_type;       //!< レース種別

        sUnit(eStadiumID stadium, eRaceType rc_type)
        : m_stadium(stadium)
        , m_type(rc_type)
        {
        }
    };

    /*!
     *  @retval true    空だ
     */
    bool empty() const
    {
        return m_schedule.empty();
    }
    /*!
     *  @brief  登録
     *  @param  stadium レース場
     *  @param  rc_type レース種別
     */
    void entry(eStadiumID stadium, eRaceType rc_type)
    {
        m_schedule.emplace_back(stadium, rc_type);
    }
    /*!
     *  @brief  先頭要素を取り出す
     *  @note   取得&削除
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
 *  @brief  開催レース情報(1日分)
 *  @note   保存形式
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
 *  @note   保存形式
 */
class ScheduleAtMonth
{
private:
    //! 1レース場分の月開催スケジュール<暦日, 開催レース情報(1日分)>
    typedef std::map<int32_t, sRaceAtDay> ScheduleInStadium;
    //! 月開催スケジュール<レース場, 開催レース情報(1日分)>
    std::map<eStadiumID, ScheduleInStadium> m_schedule;

public:
    /*!
     *  @brief  データが空か
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
     *  @brief  指定日の開催スケジュールを得る
     *  @param[in]  month_day         暦日
     *  @param[out] o_schedule  開催スケジュール(格納先)
     */
    void get_at_day(int32_t month_day, ScheduleAtDay& o_schedule) const;
    /*!
     *  @brief  レース種別ごとの数(開催日数)を数える
     *  @param[out] counter     カウンタ
     */
    void count_race_by_type(std::map<eRaceType, int32_t>& counter) const;

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



} // namespace boatrace
