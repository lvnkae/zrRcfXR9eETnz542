/*!
 *  @file   boatrace_data_utility.cpp
 *  @brief  £øf[^ûWF¢ë¢ëÖ
 *  @date   2018/03/17
 */
#include "boatrace_data_utility.h"

#include <iostream>

namespace boatrace
{

/*!
 *  @brief  ¶ñ(^O)ð[XíÊÉÏ·
 *  @param  tag [XíÊ¶ñ
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
 *  @brief  [XêIDð¶ñ([Xê¼)ÉÏ·
 *  @param  st_id   [XêID
 */
std::string StadiumIDToStr(eStadiumID st_id)
{
    switch (st_id) {
    case STADIUM_KIRYU:      // 01:Ë¶(Qn)
        return "KIRYU";
    case STADIUM_TODA:       // 02:Ëc(éÊ)
        return "TODA";
    case STADIUM_EDOGAWA:    // 03:]Ëì()
        return "EDOGAWA";
    case STADIUM_HEIWAJIMA:  // 04:½a()
        return "HEIWAJIMA";
    case STADIUM_TAMAGAWA:   // 05:½ì()
        return "TAMAGAWA";

    case STADIUM_HAMANAKO:   // 06:l¼Î(Ãª)
        return "HAMANAKO";
    case STADIUM_GAMAGOORI:  // 07:S(¤m)
        return "GAMAGOORI";
    case STADIUM_TOKONAME:   // 08:í(¤m)
        return "TOKONAME";
    case STADIUM_TSU:        // 09:Ã(Od)
        return "TSU";

    case STADIUM_MIKUNI:     // 10:O(ä)
        return "MIKUNI";
    case STADIUM_BIWAKO:     // 11:Ñí±( ê)
        return "BIWAKO";
    case STADIUM_SUMINOE:    // 12:ZV](åã)
        return "SUMINOE";
    case STADIUM_AMAGASAKI:  // 13:òè(ºÉ)
        return "AMAGASAKI";

    case STADIUM_NARUTO:     // 14:Âå(¿)
        return "NARUTO";
    case STADIUM_MARUGAME:   // 15:ÛT(ì)
        return "MARUGAME";
    case STADIUM_KOJIMA:     // 16:(ªR)
        return "KOJIMA";
    case STADIUM_MIYAJIMA:   // 17:{(L)
        return "MIYAJIMA";
    case STADIUM_TOKUYAMA:   // 18:¿R(Rû)
        return "TOKUYAMA";
    case STADIUM_SHIMONOSEKI:// 19:ºÖ(Rû)
        return "SHIMONOSEKI";

    case STADIUM_WAKAMATSU:  // 20:á¼(ª)
        return "WAKAMATSU";
    case STADIUM_ASHIYA:     // 21:°®(ª)
        return "ASHIYA";
    case STADIUM_FUKUOKA:    // 22:ª(ª)
        return "FUKUOKA";
    case STADIUM_KARATSU:    // 23:Ã(²ê)
        return "KARATSU";
    case STADIUM_OOMURA:     // 24:åº(·è)
        return "OOMURA";

    default:
        PRINT_MESSAGE(": illegal stadium id(" + std::to_string(st_id) + ")");
        return "STADIUM_ERR";
    }    
}

/*!
 *  @brief  [XÅLIDFó
 */
uint32_t BlankRaceID() { return 0; }

/*!
 *  @brief  [XÅLIDFLø©
 */
bool IsValidRaceID(uint32_t rc_id) { return rc_id > 0; }

/*!
 *  @brief  [XÔ(æn[X)Fó
 */
int32_t BlankRaceNumber() { return 0; }
/*!
 *  @brief  [XÔFÅ[X
 */
int32_t FirstRaceNumber() { return 1; }
/*!
 *  @brief  [XÔFÅI[X
 */
int32_t LastRaceNumber() { return 12; }

/*!
 *  @brief  messageoÍ
 *  @param  msg         message
 *  @param  func_name   ­¶Ö¼
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
