#include "BetterStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// 權重參數
const double ALPHA = 10.0;  // 樓層權重 (Priority)
const double BETA = 1.5;    // 等待時間權重 (Wait Time)
const double GAMMA = 2.0;   // 距離懲罰 (Distance)

struct Elevator {
    int id;
    int current_floor;
    int direction; // 1: UP, -1: DOWN, 0: IDLE
    int box_size;
    int target_floor;
    std::vector<Passenger> box;
};

BetterStrategy::BetterStrategy() {}

void BetterStrategy::loadHallQueue() {
    hall_queue.clear();
    std::ifstream in("passengers.txt");
    Passenger p;
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) {
        hall_queue.push_back(p);
    }
}

void BetterStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) {
        out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
    }
}

void BetterStrategy::execute(int current_tick) {
    int num_floors, capacity, num_elevators;
    double spawn_rate;
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors >> spawn_rate >> capacity >> num_elevators)) return;
    cfg.close();

    // 1. 讀取狀態
    std::vector<Elevator> elevators(num_elevators);
    std::ifstream in_state("state.txt");
    for (int i = 0; i < num_elevators; ++i) {
        elevators[i].id = i;
        if (!(in_state >> elevators[i].current_floor >> elevators[i].direction >> elevators[i].box_size >> elevators[i].target_floor)) {
            elevators[i].current_floor = 0;
            elevators[i].direction = 0;
            elevators[i].target_floor = 0;
        }
        for (int j = 0; j < elevators[i].box_size; ++j) {
            Passenger p;
            in_state >> p.id >> p.start >> p.dest >> p.spawn_tick;
            elevators[i].box.push_back(p);
        }
    }
    in_state.close();

    loadHallQueue();
    std::ofstream jout("tmp_event.json", std::ios::app);
    std::vector<double> priority(num_floors, 1.0); // 預設權重

    for (auto &e : elevators) {
        // --- A. 下車邏輯 ---
        auto it = e.box.begin();
        while (it != e.box.end()) {
            if (it->dest == e.current_floor) {
                std::ofstream ev("events.txt", std::ios::app);
                ev << it->id << " " << it->start << " " << it->dest << " " << it->spawn_tick << " " << current_tick << std::endl;
                jout << "        { \"type\": \"PASSENGER_EXIT\", \"passengerId\": \"P" << it->id 
                     << "\", \"floor\": " << e.current_floor + 1 << ", \"elevatorId\": " << e.id << " }," << std::endl;
                it = e.box.erase(it);
            } else ++it;
        }

        // --- B. 上車邏輯 (需求 1: 順路就載) ---
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            if (hit->start == e.current_floor && (int)e.box.size() < capacity) {
                int p_dir = (hit->dest > hit->start) ? 1 : -1;
                // 條件：電梯靜止 OR 乘客要去的地方與電梯移動方向一致
                if (e.direction == 0 || e.direction == p_dir) {
                    e.box.push_back(*hit);
                    jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id 
                         << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                    hit = hall_queue.erase(hit);
                    continue;
                }
            }
            ++hit;
        }

        // --- C. 決策邏輯 (需求 2: 廂內優先 & 需求 3: 排除當前樓層) ---
        if (!e.box.empty()) {
            // 電梯內有人：優先送客。目標設為最近的目的地 (類似 SCAN)
            int best_target = e.box[0].dest;
            int min_dist = std::abs(e.current_floor - best_target);
            for (const auto& p : e.box) {
                if (std::abs(e.current_floor - p.dest) < min_dist) {
                    min_dist = std::abs(e.current_floor - p.dest);
                    best_target = p.dest;
                }
            }
            e.target_floor = best_target;
        } else {
            // 電梯沒人：計算 Score 尋找大廳需求
            if (e.current_floor == e.target_floor || e.direction == 0) {
                double max_score = -99999.0;
                int best_f = e.current_floor;
                bool found_request = false;

                for (int f = 0; f < num_floors; ++f) {
                    if (f == e.current_floor) continue; // 排除目前樓層

                    int max_w = -1;
                    for (auto &p : hall_queue) {
                        if (p.start == f) {
                            int w = current_tick - p.spawn_tick;
                            if (w > max_w) max_w = w;
                        }
                    }

                    if (max_w != -1) { // 該樓層有人在等
                        double dist = std::abs(e.current_floor - f);
                        double score = (ALPHA * priority[f]) + (BETA * max_w) - (GAMMA * dist);
                        if (score > max_score) {
                            max_score = score;
                            best_f = f;
                            found_request = true;
                        }
                    }
                }
                if (found_request) e.target_floor = best_f;
                else e.target_floor = e.current_floor; // 沒事做，待在原地
            }
        }

        // --- D. 執行移動 ---
        int old_floor = e.current_floor;
        if (e.target_floor > e.current_floor) {
            e.direction = 1;
            e.current_floor++;
        } else if (e.target_floor < e.current_floor) {
            e.direction = -1;
            e.current_floor--;
        } else {
            e.direction = 0;
        }

        if (old_floor != e.current_floor) {
            jout << "        { \"type\": \"ELEVATOR_MOVE\", \"elevatorId\": " << e.id << ", \"from\": " << old_floor + 1
                 << ", \"to\": " << e.current_floor + 1 << ", \"direction\": \"" 
                 << (e.direction == 1 ? "UP" : "DOWN") << "\" }," << std::endl;
        }
    }

    // 3. 儲存狀態
    std::ofstream out_state("state.txt");
    for (auto &e : elevators) {
        out_state << e.current_floor << " " << e.direction << " " << e.box.size() << " " << e.target_floor << "\n";
        for (auto &p : e.box) out_state << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
    }
    out_state.close();
    saveHallQueue();
}