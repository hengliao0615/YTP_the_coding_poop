#include "BetterStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// 公式權重係數
const double ALPHA = 10.0;
const double BETA = 1.5;
const double GAMMA = 2.0;
const double C1 = 2.0;   // 即時等待人數權重
const double C2 = 0.1;   // 歷史總需求權重 (建議設小一點避免數值膨脹過快)

struct Elevator {
    int id;
    int current_floor;
    int direction;
    int box_size;
    int target_floor;
    std::vector<Passenger> box;
};

BetterStrategy::BetterStrategy() {}

// --- 輔助函式：歷史數據持久化 ---
std::vector<int> BetterStrategy::loadHistory(int num_floors) {
    std::vector<int> hist(num_floors, 0);
    std::ifstream in("history.txt");
    int val;
    for (int i = 0; i < num_floors && (in >> val); ++i) hist[i] = val;
    return hist;
}

void BetterStrategy::saveHistory(const std::vector<int>& history) {
    std::ofstream out("history.txt");
    for (int val : history) out << val << " ";
}

void BetterStrategy::loadHallQueue() {
    hall_queue.clear();
    std::ifstream in("passengers.txt");
    Passenger p;
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) hall_queue.push_back(p);
}

void BetterStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
}

void BetterStrategy::execute(int current_tick) {
    int num_floors, capacity, num_elevators;
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors)) return;
    double spawn_rate; cfg >> spawn_rate >> capacity >> num_elevators;
    cfg.close();

    loadHallQueue();
    std::vector<int> history_count = loadHistory(num_floors);

    // 更新歷史需求 (僅針對此 Tick 新生成的乘客)
    for (const auto& p : hall_queue) {
        if (p.spawn_tick == current_tick) history_count[p.start]++;
    }
    saveHistory(history_count);

    // 1. 讀取電梯狀態
    std::vector<Elevator> elevators(num_elevators);
    std::ifstream in_state("state.txt");
    for (int i = 0; i < num_elevators; ++i) {
        elevators[i].id = i;
        if (!(in_state >> elevators[i].current_floor >> elevators[i].direction >> elevators[i].box_size >> elevators[i].target_floor)) {
            elevators[i].current_floor = 0; elevators[i].direction = 0; elevators[i].target_floor = 0;
        }
        for (int j = 0; j < elevators[i].box_size; ++j) {
            Passenger p; in_state >> p.id >> p.start >> p.dest >> p.spawn_tick;
            elevators[i].box.push_back(p);
        }
    }
    in_state.close();

    std::ofstream jout("tmp_event.json", std::ios::app);

    // 2. 計算動態 Priority
    std::vector<double> dynamic_priority(num_floors, 0.0);
    for (int i = 0; i < num_floors; ++i) {
        int current_waiting = 0;
        for (const auto& p : hall_queue) if (p.start == i) current_waiting++;
        dynamic_priority[i] = (C1 * current_waiting) + (C2 * history_count[i]);
    }

    // 3. 處理每台電梯
    for (auto &e : elevators) {
        // A. 下車
        auto it = e.box.begin();
        while (it != e.box.end()) {
            if (it->dest == e.current_floor) {
                std::ofstream ev("events.txt", std::ios::app);
                ev << it->id << " " << it->start << " " << it->dest << " " << it->spawn_tick << " " << current_tick << std::endl;
                jout << "        { \"type\": \"PASSENGER_EXIT\", \"passengerId\": \"P" << it->id << "\", \"floor\": " << e.current_floor + 1 << ", \"elevatorId\": " << e.id << " }," << std::endl;
                it = e.box.erase(it);
            } else ++it;
        }

        // B. 順路就載 (方向一致性)
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            int p_dir = (hit->dest > hit->start) ? 1 : -1;
            if (hit->start == e.current_floor && (int)e.box.size() < capacity && (e.direction == 0 || e.direction == p_dir)) {
                e.box.push_back(*hit);
                jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                hit = hall_queue.erase(hit);
            } else ++hit;
        }

        // C. 決策 (廂內優先 + 排除當前樓層)
        if (!e.box.empty()) {
            // 廂內有人，目標設為最近的目的地
            int best_target = e.box[0].dest;
            int min_d = std::abs(e.current_floor - best_target);
            for (const auto& p : e.box) {
                if (std::abs(e.current_floor - p.dest) < min_d) {
                    min_d = std::abs(e.current_floor - p.dest);
                    best_target = p.dest;
                }
            }
            e.target_floor = best_target;
        } else {
            // 廂內沒人，計算 Score
            if (e.current_floor == e.target_floor || e.direction == 0) {
                double max_s = -99999.0;
                int best_f = e.current_floor;
                bool found = false;
                for (int f = 0; f < num_floors; ++f) {
                    if (f == e.current_floor) continue;
                    int max_w = -1;
                    for (auto &p : hall_queue) if (p.start == f) max_w = std::max(max_w, current_tick - p.spawn_tick);
                    
                    if (max_w != -1) {
                        double score = (ALPHA * dynamic_priority[f]) + (BETA * max_w) - (GAMMA * std::abs(e.current_floor - f));
                        if (score > max_s) { max_s = score; best_f = f; found = true; }
                    }
                }
                if (found) e.target_floor = best_f;
            }
        }

        // D. 移動執行
        int old_f = e.current_floor;
        if (e.target_floor > e.current_floor) { e.direction = 1; e.current_floor++; }
        else if (e.target_floor < e.current_floor) { e.direction = -1; e.current_floor--; }
        else e.direction = 0;

        if (old_f != e.current_floor) {
            jout << "        { \"type\": \"ELEVATOR_MOVE\", \"elevatorId\": " << e.id << ", \"from\": " << old_f + 1 << ", \"to\": " << e.current_floor + 1 << ", \"direction\": \"" << (e.direction == 1 ? "UP" : "DOWN") << "\" }," << std::endl;
        }
    }

    // 4. 儲存狀態
    std::ofstream out_state("state.txt");
    for (auto &e : elevators) {
        out_state << e.current_floor << " " << e.direction << " " << e.box.size() << " " << e.target_floor << "\n";
        for (auto &p : e.box) out_state << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
    }
    saveHallQueue();
}