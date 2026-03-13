#include "PassengerGenerator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // 檢查是否有傳入 current_tick 參數
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <current_tick>" << std::endl;
        return 1;
    }

    int current_tick = std::stoi(argv[1]);

    // 1. 初始化生成器
    PassengerGenerator generator;

    // 2. 載入設定檔 (config.txt)
    if (!generator.loadConfig("config.txt")) {
        std::cerr << "Error: Could not load config.txt" << std::endl;
        return 1;
    }

    // 3. 執行該 Tick 的生成邏輯
    generator.execute(current_tick);

    return 0;
}