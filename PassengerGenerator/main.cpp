#include "PassengerGenerator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    PassengerGenerator gen;
    if (!gen.loadConfig("config.txt")) {
        std::cerr << "Failed to load config.txt" << std::endl;
        return 1;
    }

    // 從 controller.sh 接收目前的 tick 參數
    int current_tick = 0;
    if (argc > 1) {
        try {
            current_tick = std::stoi(argv[1]);
        } catch (...) {
            current_tick = 0;
        }
    }

    // 呼叫更新後的 execute
    gen.execute(current_tick);
    
    return 0;
}