#include "BetterStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// 公式參數調整
const double ALPHA = 10.0;  
const double BETA = 1.2;    
const double GAMMA = 1.5;   

struct Elevator {
    int id;
    int current_floor;
    int direction; 
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

    // 1. 讀取電梯狀態 (state.txt: floor dir box_size target)
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

    // 2. 樓層基礎優先權 (P) - 這裡可以根據歷史資料調整，目前設為 1.0
    std::vector<double> priority(num_floors, 1.0);

    for (auto &e : elevators) {
        // A. 下車邏輯
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

        // B. 上車邏輯 (順路就載)
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            if (hit->start == e.current_floor && (int)e.box.size() < capacity) {
                e.box.push_back(*hit);
                jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id 
                     << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                hit = hall_queue.erase(hit);
            } else ++hit;
        }

        // C. 計算得分並決定 Target (每當電梯空閒或到達目標時重算)
        if (e.direction == 0 || e.current_floor == e.target_floor) {
            double max_score = -99999.0;
            int best_f = e.current_floor;

            for (int f = 0; f < num_floors; ++f) {
                // 檢查該樓層有無需求
                bool has_demand = false;
                int max_w = 0;
                for (auto &p : hall_queue) {
                    if (p.start == f) {
                        has_demand = true;
                        int w = current_tick - p.spawn_tick;
                        if (w > max_w) max_w = w;
                    }
                }
                for (auto &p : e.box) if (p.dest == f) has_demand = true;

                if (!has_demand) continue;

                // 公式: Score = αP + βW - γD
                double dist = std::abs(e.current_floor - f);
                double score = (ALPHA * priority[f]) + (BETA * max_w) - (GAMMA * dist);

                if (score > max_score) {
                    max_score = score;
                    best_f = f;
                }
            }
            e.target_floor = best_f;
        }

        // D. 執行移動
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