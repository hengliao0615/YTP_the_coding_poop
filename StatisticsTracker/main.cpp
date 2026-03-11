#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <algorithm> // 用於 std::max_element

int main() {
    std::ifstream in("events.txt");
    int id, start, dest, spawn, enter, exit;
    std::vector<double> wait_times;
    std::vector<double> service_times;
    
    while (in >> id >> start >> dest >> spawn >> enter >> exit) {
        wait_times.push_back((double)enter - spawn);
        service_times.push_back((double)exit - spawn);
    }
    in.close();

    std::ofstream out("stats.txt");
    int count = wait_times.size();

    if (count > 0) {
        // 1. 計算平均等待時間
        double sum_wait = std::accumulate(wait_times.begin(), wait_times.end(), 0.0);
        double avg_wait = sum_wait / count;
        
        // 2. 計算最大等待時間
        double max_wait = *std::max_element(wait_times.begin(), wait_times.end());

        // 3. 計算平均服務總耗時
        double sum_service = std::accumulate(service_times.begin(), service_times.end(), 0.0);
        double avg_service = sum_service / count;

        // 4. 計算等待時間標準差
        double sq_sum = 0;
        for (double w : wait_times) sq_sum += std::pow(w - avg_wait, 2);
        double std_dev = std::sqrt(sq_sum / count);

        out << "=== 模擬統計結果 ===" << std::endl;
        out << "已完成載客總數: " << count << std::endl;
        out << "平均等待時間: " << std::fixed << std::setprecision(2) << avg_wait << " Ticks" << std::endl;
        out << "等待時間標準差: " << std_dev << std::endl;
        out << "最大等待時間: " << (int)max_wait << " Ticks" << std::endl; // 新增輸出
        out << "平均服務總耗時: " << avg_service << " Ticks" << std::endl;
    } else {
        out << "尚無完成紀錄。" << std::endl;
    }
    out.close();
    return 0;
}