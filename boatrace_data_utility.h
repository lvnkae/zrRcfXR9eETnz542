/*!
 *  @file   boatrace_data_utility.h
 *  @brief  競艇データ収集：いろいろ便利
 *  @date   2018/03/17
 */
#pragma once

#include "boatrace_data_define.h"

#include <string>

#define PRINT_MESSAGE(msg) PrintMessage(msg, __func__)

namespace boatrace
{

/*!
 *  @brief  文字列(タグ)をレース種別に変換
 *  @param  tag レース種別文字列
 */
eRaceType StrToRaceType(const std::string& tag);

/*!
 *  @brief  レース場IDを文字列(レース場名)に変換
 *  @param  st_id   レース場ID
 */
std::string StadiumIDToStr(eStadiumID st_id);

/*!
 *  @brief  レース固有ID：空
 */
uint32_t BlankRaceID();
/*!
 *  @brief  レース固有ID：有効か
 *  @retval false   不正ID
 */
bool IsValidRaceID(uint32_t rc_id);

/*!
 *  @brief  レース番号(第nレース)：空
 */
int32_t BlankRaceNumber();
/*!
 *  @brief  レース番号：最初レース
 */
int32_t FirstRaceNumber();
/*!
 *  @brief  レース番号：最終レース
 */
int32_t LastRaceNumber();


/*!
 *  @brief  message出力
 *  @param  msg         message
 *  @param  func_name   発生関数名
 *  @note   標準出力
 */
void PrintMessage(const std::string& msg, const char* func_name=nullptr);

} // namespace boatrace
