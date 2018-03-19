/*!
 *  @file   boatrace_data_define.h
 *  @brief  競艇データ収集：定義
 *  @date   2018/03/13
 */
#pragma once

#include <memory>

namespace boatrace
{

enum eStadiumID
{
    STADIUM_NONE,

    STADIUM_KIRYU,      // 01:桐生(群馬)
    STADIUM_TODA,       // 02:戸田(埼玉)
    STADIUM_EDOGAWA,    // 03:江戸川(東京)
    STADIUM_HEIWAJIMA,  // 04:平和島(東京)
    STADIUM_TAMAGAWA,   // 05:多摩川(東京)

    STADIUM_HAMANAKO,   // 06:浜名湖(静岡)
    STADIUM_GAMAGOORI,  // 07:蒲郡(愛知)
    STADIUM_TOKONAME,   // 08:常滑(愛知)
    STADIUM_TSU,        // 09:津(三重)

    STADIUM_MIKUNI,     // 10:三国(福井)
    STADIUM_BIWAKO,     // 11:びわこ(滋賀)
    STADIUM_SUMINOE,    // 12:住之江(大阪)
    STADIUM_AMAGASAKI,  // 13:尼崎(兵庫)

    STADIUM_NARUTO,     // 14:鳴門(徳島)
    STADIUM_MARUGAME,   // 15:丸亀(香川)
    STADIUM_KOJIMA,     // 16:児島(岡山)
    STADIUM_MIYAJIMA,   // 17:宮島(広島)
    STADIUM_TOKUYAMA,   // 18:徳山(山口)
    STADIUM_SHIMONOSEKI,// 19:下関(山口)

    STADIUM_WAKAMATSU,  // 20:若松(福岡)
    STADIUM_ASHIYA,     // 21:芦屋(福岡)
    STADIUM_FUKUOKA,    // 22:福岡(福岡)
    STADIUM_KARATSU,    // 23:唐津(佐賀)
    STADIUM_OOMURA,     // 24:大村(長崎)

    NUM_STADIUM = 24,
};

enum eRaceGrade
{
    GRADE_NONE,

    GRADE_SG,       // SG
    GRADE_G1,       // G1
    GRADE_G2,       // G2
    GRADE_G3,       // G3
    GRADE_NORMAL,   // 一般
};

enum eRaceType
{
    RACE_NONE,

    RACE_SG,        //!< SG
    RACE_G1,        //!< G1
    RACE_G2,        //!< G2
    RACE_G3,        //!< G3
    RACE_ALLLADYS,  //!< オールレディース
    RACE_VENUS,     //!< ヴィーナスシリーズ
    RACE_ROOKIE,    //!< ルーキーシリーズ
    RACE_MASTERS,   //!< マスターズリーグ 
    RACE_NORMAL,    //!< 一般
};

} // namespace boatrace
