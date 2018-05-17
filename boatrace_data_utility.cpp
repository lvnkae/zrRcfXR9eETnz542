/*!
 *  @file   boatrace_data_utility.cpp
 *  @brief  �����f�[�^���W�F���낢��֗�
 *  @date   2018/03/17
 */
#include "boatrace_data_utility.h"

#include <iostream>

namespace boatrace
{

/*!
 *  @brief  ������(�^�O)�����[�X��ʂɕϊ�
 *  @param  tag ���[�X��ʕ�����
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
 *  @brief  ���[�X��ID�𕶎���(���[�X�ꖼ)�ɕϊ�
 *  @param  st_id   ���[�X��ID
 */
std::string StadiumIDToStr(eStadiumID st_id)
{
    switch (st_id) {
    case STADIUM_KIRYU:      // 01:�ː�(�Q�n)
        return "KIRYU";
    case STADIUM_TODA:       // 02:�˓c(���)
        return "TODA";
    case STADIUM_EDOGAWA:    // 03:�]�ː�(����)
        return "EDOGAWA";
    case STADIUM_HEIWAJIMA:  // 04:���a��(����)
        return "HEIWAJIMA";
    case STADIUM_TAMAGAWA:   // 05:������(����)
        return "TAMAGAWA";

    case STADIUM_HAMANAKO:   // 06:�l����(�É�)
        return "HAMANAKO";
    case STADIUM_GAMAGOORI:  // 07:���S(���m)
        return "GAMAGOORI";
    case STADIUM_TOKONAME:   // 08:�튊(���m)
        return "TOKONAME";
    case STADIUM_TSU:        // 09:��(�O�d)
        return "TSU";

    case STADIUM_MIKUNI:     // 10:�O��(����)
        return "MIKUNI";
    case STADIUM_BIWAKO:     // 11:�т킱(����)
        return "BIWAKO";
    case STADIUM_SUMINOE:    // 12:�Z�V�](���)
        return "SUMINOE";
    case STADIUM_AMAGASAKI:  // 13:���(����)
        return "AMAGASAKI";

    case STADIUM_NARUTO:     // 14:��(����)
        return "NARUTO";
    case STADIUM_MARUGAME:   // 15:�ۋT(����)
        return "MARUGAME";
    case STADIUM_KOJIMA:     // 16:����(���R)
        return "KOJIMA";
    case STADIUM_MIYAJIMA:   // 17:�{��(�L��)
        return "MIYAJIMA";
    case STADIUM_TOKUYAMA:   // 18:���R(�R��)
        return "TOKUYAMA";
    case STADIUM_SHIMONOSEKI:// 19:����(�R��)
        return "SHIMONOSEKI";

    case STADIUM_WAKAMATSU:  // 20:�ᏼ(����)
        return "WAKAMATSU";
    case STADIUM_ASHIYA:     // 21:����(����)
        return "ASHIYA";
    case STADIUM_FUKUOKA:    // 22:����(����)
        return "FUKUOKA";
    case STADIUM_KARATSU:    // 23:����(����)
        return "KARATSU";
    case STADIUM_OOMURA:     // 24:�呺(����)
        return "OOMURA";

    default:
        PRINT_MESSAGE(": illegal stadium id(" + std::to_string(st_id) + ")");
        return "STADIUM_ERR";
    }    
}

/*!
 *  @brief  ���[�X�ŗLID�F��
 */
uint32_t BlankRaceID() { return 0; }

/*!
 *  @brief  ���[�X�ŗLID�F�L����
 */
bool IsValidRaceID(uint32_t rc_id) { return rc_id > 0; }

/*!
 *  @brief  ���[�X�ԍ�(��n���[�X)�F��
 */
int32_t BlankRaceNumber() { return 0; }
/*!
 *  @brief  ���[�X�ԍ��F�ŏ����[�X
 */
int32_t FirstRaceNumber() { return 1; }
/*!
 *  @brief  ���[�X�ԍ��F�ŏI���[�X
 */
int32_t LastRaceNumber() { return 12; }

/*!
 *  @brief  message�o��
 *  @param  msg         message
 *  @param  func_name   �����֐���
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
