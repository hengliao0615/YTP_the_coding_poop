#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <string>

int main(int argc, char* argv[]) {
    int current_tick = 0;
    if (argc >= 2) {
        current_tick = std::stoi(argv[1]);
    }

    std::vector<double> all_wait_times;
    int served_count = 0;

    // 1. 讀取已完成服務的乘客 (由 events.txt 取得)
    std::ifstream ev_in("events.txt");
    int id, start, dest, spawn, enter, exit_t;
    while (ev_in >> id >> start >> dest >> spawn >> enter >> exit_t) {
        if (enter != -1) {
            all_wait_times.push_back((double)enter - spawn);
            served_count++;
        }
    }
    ev_in.close();

    // 2. 讀取滯留乘客 (由 passengers.txt 取得)
    std::ifstream ps_in("passengers.txt");
    int stranded_count = 0;
    while (ps_in >> id >> start >> dest >> spawn) {
        double stranded_wait = (double)current_tick - spawn;
        if (stranded_wait < 0) stranded_wait = 0;
        all_wait_times.push_back(stranded_wait);
        stranded_count++;
    }
    ps_in.close();

    // 3. 輸出完整的統計報表到 stats.txt
    std::ofstream out("stats.txt");
    if (!all_wait_times.empty()) {
        double sum = std::accumulate(all_wait_times.begin(), all_wait_times.end(), 0.0);
        double avg = sum / all_wait_times.size();
        double max_w = *std::max_element(all_wait_times.begin(), all_wait_times.end());
        
        double sq_sum = 0;
        for (double w : all_wait_times) sq_sum += std::pow(w - avg, 2);
        double std_dev = std::sqrt(sq_sum / all_wait_times.size());

        out << "=== 模擬完整統計報告 ===" << std::endl;
        out << "實際載運量 (已送達): " << served_count << " 人" << std::endl;
        out << "滯留乘客人數: " << stranded_count << " 人" << std::endl;
        out << "總生成乘客數: " << (served_count + stranded_count) << " 人" << std::endl;
        out << "------------------------------------" << std::endl;
        out << "平均等待時間: " << std::fixed << std::setprecision(2) << avg << " Ticks" << std::endl;
        out << "等待時間標準差: " << std_dev << " Ticks" << std::endl;
        out << "最大等待時間: " << (int)max_w << " Ticks" << std::endl;
        out << "------------------------------------" << std::endl;
        out << "註：包含滯留乘客計算至 Tick " << current_tick << std::endl;
    } else {
        out << "尚無乘客數據可供統計。" << std::endl;
    }
    out.close();

    return 0;
}