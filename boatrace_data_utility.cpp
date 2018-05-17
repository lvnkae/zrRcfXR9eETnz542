/*!
 *  @file   boatrace_data_utility.cpp
 *  @brief  競艇データ収集：いろいろ便利
 *  @date   2018/03/17
 */
#include "boatrace_data_utility.h"

#include <iostream>

namespace boatrace
{

/*!
 *  @brief  文字列(タグ)をレース種別に変換
 *  @param  tag レース種別文字列
 */
eRaceType StrToRaceType(const std::string& tag)
{
    if (tag ==  "SG") {
        return RACE_SG;
    } else if (tag ==  "G1") {
        return RACE_G1;
    } else if (tag ==  "G2") {
        return RACE_G2;
    } else if (tag ==  "G3") {
        return RACE_G3;
    } else if (tag ==  "NORMAL") {
        return RACE_NORMAL;
    } else if (tag ==  "LADYS") {
        return RACE_ALLLADYS;
    } else if (tag ==  "ROOKIE") {
        return RACE_ROOKIE;
    } else if (tag ==  "VENUS") {
        return RACE_VENUS;
    } else if (tag ==  "MASTERS") {
        return RACE_MASTERS;
    } else {
        PRINT_MESSAGE(": illegal race type(" + tag + ")");
        return RACE_NONE;
    }
}

/*!
 *  @brief  レース場IDを文字列(レース場名)に変換
 *  @param  st_id   レース場ID
 */
std::string StadiumIDToStr(eStadiumID st_id)
{
    switch (st_id) {
    case STADIUM_KIRYU:      // 01:桐生(群馬)
        return "KIRYU";
    case STADIUM_TODA:       // 02:戸田(埼玉)
        return "TODA";
    case STADIUM_EDOGAWA:    // 03:江戸川(東京)
        return "EDOGAWA";
    case STADIUM_HEIWAJIMA:  // 04:平和島(東京)
        return "HEIWAJIMA";
    case STADIUM_TAMAGAWA:   // 05:多摩川(東京)
        return "TAMAGAWA";

    case STADIUM_HAMANAKO:   // 06:浜名湖(静岡)
        return "HAMANAKO";
    case STADIUM_GAMAGOORI:  // 07:蒲郡(愛知)
        return "GAMAGOORI";
    case STADIUM_TOKONAME:   // 08:常滑(愛知)
        return "TOKONAME";
    case STADIUM_TSU:        // 09:津(三重)
        return "TSU";

    case STADIUM_MIKUNI:     // 10:三国(福井)
        return "MIKUNI";
    case STADIUM_BIWAKO:     // 11:びわこ(滋賀)
        return "BIWAKO";
    case STADIUM_SUMINOE:    // 12:住之江(大阪)
        return "SUMINOE";
    case STADIUM_AMAGASAKI:  // 13:尼崎(兵庫)
        return "AMAGASAKI";

    case STADIUM_NARUTO:     // 14:鳴門(徳島)
        return "NARUTO";
    case STADIUM_MARUGAME:   // 15:丸亀(香川)
        return "MARUGAME";
    case STADIUM_KOJIMA:     // 16:児島(岡山)
        return "KOJIMA";
    case STADIUM_MIYAJIMA:   // 17:宮島(広島)
        return "MIYAJIMA";
    case STADIUM_TOKUYAMA:   // 18:徳山(山口)
        return "TOKUYAMA";
    case STADIUM_SHIMONOSEKI:// 19:下関(山口)
        return "SHIMONOSEKI";

    case STADIUM_WAKAMATSU:  // 20:若松(福岡)
        return "WAKAMATSU";
    case STADIUM_ASHIYA:     // 21:芦屋(福岡)
        return "ASHIYA";
    case STADIUM_FUKUOKA:    // 22:福岡(福岡)
        return "FUKUOKA";
    case STADIUM_KARATSU:    // 23:唐津(佐賀)
        return "KARATSU";
    case STADIUM_OOMURA:     // 24:大村(長崎)
        return "OOMURA";

    default:
        PRINT_MESSAGE(": illegal stadium id(" + std::to_string(st_id) + ")");
        return "STADIUM_ERR";
    }    
}

/*!
 *  @brief  レース固有ID：空
 */
uint32_t BlankRaceID() { return 0; }

/*!
 *  @brief  レース固有ID：有効か
 */
bool IsValidRaceID(uint32_t rc_id) { return rc_id > 0; }

/*!
 *  @brief  レース番号(第nレース)：空
 */
int32_t BlankRaceNumber() { return 0; }
/*!
 *  @brief  レース番号：最初レース
 */
int32_t FirstRaceNumber() { return 1; }
/*!
 *  @brief  レース番号：最終レース
 */
int32_t LastRaceNumber() { return 12; }

/*!
 *  @brief  message出力
 *  @param  msg         message
 *  @param  func_name   発生関数名
 */
void PrintMessage(const std::string& msg, const char* func_name)
{
    if (nullptr != func_name) {
        std::cout << func_name << ">> " << msg.c_str() << std::endl;
    } else {
        std::cout << msg.c_str() << std::endl;
    }
}

} // namespace boatrace
