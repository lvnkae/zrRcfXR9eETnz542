// boatrace_data_collector.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "environment.h"
#include "boatrace_data_collector.h"
#include "boatrace_data_define.h"
#include "boatrace_data_utility.h"

#include "utility/utility_datetime.h"
#include "utility/utility_string.h"

#include <memory>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc != 6) {
        return 0;
    }
    /*!
        argv[0]
        argv[1] �X�P�W���[�����W�ŏ��N��
        argv[2] �X�P�W���[�����W�ŏI�N��
        argv[3] �f�[�^���W�ŏ��N����
        argv[4] �f�[�^���W�ŏ��N����
        argv[5] �f�[�^���W�Ώۃ��[�X���(eRaceType) delim"/"
     */

    std::shared_ptr<Environment> environment(Environment::Create());

    std::unordered_set<boatrace::eRaceType> rc_type;
    {
        std::unordered_set<std::string> rc_type_str;
        garnet::utility_string::Split(argv[5], "/", rc_type_str);
        for (const auto& tag: rc_type_str) {
            rc_type.emplace(boatrace::StrToRaceType(tag));
        }
    }
    boatrace::RaceDataCollector collector(argv[1], argv[2], argv[3], argv[4], rc_type);

    while(1) {
        const int64_t tickCount = garnet::utility_datetime::GetTickCountGeneral();
        if (collector.Update(tickCount)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
