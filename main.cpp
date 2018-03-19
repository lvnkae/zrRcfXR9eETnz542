// boatrace_data_collector.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "environment.h"
#include "boatrace_data_collector.h"

#include "utility/utility_datetime.h"

#include <memory>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        return 0;
    }

    std::shared_ptr<Environment> environment(Environment::Create());

    boatrace::RaceDataCollector collector(argv[1], argv[2]);

    while(1) {
        const int64_t tickCount = garnet::utility_datetime::GetTickCountGeneral();
        if (collector.Update(tickCount)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}

