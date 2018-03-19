/*!
 *  @file   boatrace_data_define.h
 *  @brief  �����f�[�^���W�F��`
 *  @date   2018/03/13
 */
#pragma once

#include <memory>

namespace boatrace
{

enum eStadiumID
{
    STADIUM_NONE,

    STADIUM_KIRYU,      // 01:�ː�(�Q�n)
    STADIUM_TODA,       // 02:�˓c(���)
    STADIUM_EDOGAWA,    // 03:�]�ː�(����)
    STADIUM_HEIWAJIMA,  // 04:���a��(����)
    STADIUM_TAMAGAWA,   // 05:������(����)

    STADIUM_HAMANAKO,   // 06:�l����(�É�)
    STADIUM_GAMAGOORI,  // 07:���S(���m)
    STADIUM_TOKONAME,   // 08:�튊(���m)
    STADIUM_TSU,        // 09:��(�O�d)

    STADIUM_MIKUNI,     // 10:�O��(����)
    STADIUM_BIWAKO,     // 11:�т킱(����)
    STADIUM_SUMINOE,    // 12:�Z�V�](���)
    STADIUM_AMAGASAKI,  // 13:���(����)

    STADIUM_NARUTO,     // 14:��(����)
    STADIUM_MARUGAME,   // 15:�ۋT(����)
    STADIUM_KOJIMA,     // 16:����(���R)
    STADIUM_MIYAJIMA,   // 17:�{��(�L��)
    STADIUM_TOKUYAMA,   // 18:���R(�R��)
    STADIUM_SHIMONOSEKI,// 19:����(�R��)

    STADIUM_WAKAMATSU,  // 20:�ᏼ(����)
    STADIUM_ASHIYA,     // 21:����(����)
    STADIUM_FUKUOKA,    // 22:����(����)
    STADIUM_KARATSU,    // 23:����(����)
    STADIUM_OOMURA,     // 24:�呺(����)

    NUM_STADIUM = 24,
};

enum eRaceGrade
{
    GRADE_NONE,

    GRADE_SG,       // SG
    GRADE_G1,       // G1
    GRADE_G2,       // G2
    GRADE_G3,       // G3
    GRADE_NORMAL,   // ���
};

enum eRaceType
{
    RACE_NONE,

    RACE_SG,        //!< SG
    RACE_G1,        //!< G1
    RACE_G2,        //!< G2
    RACE_G3,        //!< G3
    RACE_ALLLADYS,  //!< �I�[�����f�B�[�X
    RACE_VENUS,     //!< ���B�[�i�X�V���[�Y
    RACE_ROOKIE,    //!< ���[�L�[�V���[�Y
    RACE_MASTERS,   //!< �}�X�^�[�Y���[�O 
    RACE_NORMAL,    //!< ���
};

} // namespace boatrace
