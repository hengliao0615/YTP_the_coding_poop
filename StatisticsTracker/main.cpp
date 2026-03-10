#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

struct FinishedPassenger {
    int id;
    int start;
    int dest;
    int spawn_tick;
    int end_tick;
};

int main(int argc, char* argv[]) {
    // 1. 讀取目前 Tick (選填，用於顯示進度)
    int current_tick = 0;
    if (argc > 1) {
        current_tick = std::stoi(argv[1]);
    }

    std::vector<FinishedPassenger> finished;
    std::ifstream in("events.txt");
    FinishedPassenger p;

    // 2. 讀取 events.txt 中的所有完成記錄
    // 格式：[ID] [Start] [Dest] [Spawn] [End]
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick >> p.end_tick) {
        finished.push_back(p);
    }
    in.close();

    // 3. 計算統計數據
    int total_passengers = finished.size();
    double total_wait_time = 0;
    int max_wait_time = 0;
    int min_wait_time = 999999;

    for (const auto& fp : finished) {
        int wait = fp.end_tick - fp.spawn_tick;
        total_wait_time += wait;
        if (wait > max_wait_time) max_wait_time = wait;
        if (wait < min_wait_time) min_wait_time = wait;
    }

    double avg_wait_time = (total_passengers > 0) ? (total_wait_time / total_passengers) : 0;

    // 4. 輸出摘要到 stats.txt (這會被 controller.sh 每秒更新)
    std::ofstream out("stats.txt");
    out << "======== 模擬統計摘要 ========" << std::endl;
    out << "目前模擬時間 (Tick): " << current_tick << std::endl;
    out << "已完成載客總數: " << total_passengers << std::endl;
    
    if (total_passengers > 0) {
        out << "平均服務總耗時: " << std::fixed << std::setprecision(2) << avg_wait_time << " Ticks" << std::endl;
        out << "最大服務耗時: " << max_wait_time << " Ticks" << std::endl;
        out << "最小服務耗時: " << min_wait_time << " Ticks" << std::endl;
    } else {
        out << "目前尚無乘客抵達目的地。" << std::endl;
    }
    out.close();

    // 5. 可選：在終端機輸出簡易訊息 (Debug 用)
    // std::cout << "[Stats] Total Finished: " << total_passengers << " | Avg Wait: " << avg_wait_time << std::endl;

    return 0;
}