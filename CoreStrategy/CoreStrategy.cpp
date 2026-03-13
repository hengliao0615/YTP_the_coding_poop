#include "CoreStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

struct Elevator {
    int id;
    int current_floor;
    int direction; // 1: UP, -1: DOWN, 0: IDLE
    int box_size;
    int target_floor;
    std::vector<Passenger> box;
};

CoreStrategy::CoreStrategy() {}

void CoreStrategy::loadHallQueue() {
    hall_queue.clear();
    std::ifstream in("passengers.txt");
    Passenger p;
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) {
        p.enter_tick = -1;
        hall_queue.push_back(p);
    }
}

void CoreStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
}

void CoreStrategy::execute(int current_tick) {
    int num_floors, capacity, num_elevators;
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors)) return;
    double lambda; cfg >> lambda >> capacity >> num_elevators;
    cfg.close();

    loadHallQueue();

    std::vector<Elevator> elevators(num_elevators);
    std::ifstream in_state("state.txt");
    for (int i = 0; i < num_elevators; ++i) {
        elevators[i].id = i;
        in_state >> elevators[i].current_floor >> elevators[i].direction >> elevators[i].box_size >> elevators[i].target_floor;
        for (int j = 0; j < elevators[i].box_size; ++j) {
            Passenger p; in_state >> p.id >> p.start >> p.dest >> p.spawn_tick >> p.enter_tick;
            elevators[i].box.push_back(p);
        }
    }
    in_state.close();

    std::ofstream jout("tmp_event.json", std::ios::app);

    for (auto &e : elevators) {
        // --- A. 下車邏輯 ---
        auto it = e.box.begin();
        while (it != e.box.end()) {
            if (it->dest == e.current_floor) {
                std::ofstream ev("events.txt", std::ios::app);
                ev << it->id << " " << it->start << " " << it->dest << " " 
                   << it->spawn_tick << " " << it->enter_tick << " " << current_tick << std::endl;
                
                jout << "        { \"type\": \"PASSENGER_EXIT\", \"passengerId\": \"P" << it->id << "\", \"floor\": " << e.current_floor + 1 << ", \"elevatorId\": " << e.id << " }," << std::endl;
                it = e.box.erase(it);
            } else ++it;
        }

        // --- 核心修正：邊界檢查與立即轉向 ---
        // 到達頂層則轉向向下，到達底層則轉向向上，確保邊界乘客能正確判定方向上車
        if (e.direction == 1 && e.current_floor == num_floors - 1) {
            e.direction = -1;
            e.target_floor = 0;
        } else if (e.direction == -1 && e.current_floor == 0) {
            e.direction = 1;
            e.target_floor = num_floors - 1;
        }

        // --- B. 上車邏輯 ---
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            int p_dir = (hit->dest > hit->start) ? 1 : -1;
            if (hit->start == e.current_floor && (int)e.box.size() < capacity && (e.direction == 0 || e.direction == p_dir)) {
                Passenger p = *hit;
                p.enter_tick = current_tick; 
                e.box.push_back(p);
                if (e.direction == 0) e.direction = p_dir;

                jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                hit = hall_queue.erase(hit);
            } else ++hit;
        }

        // --- C. 決策邏輯 (SCAN 保持方向走到邊界) ---
        if (e.direction == 0) {
            // 只有在靜止且大廳有人時，才尋找最近請求來決定初始方向
            if (!hall_queue.empty()) {
                int min_dist = 999;
                int closest_start = e.current_floor;
                for (auto &p : hall_queue) {
                    if (std::abs(p.start - e.current_floor) < min_dist) {
                        min_dist = std::abs(p.start - e.current_floor);
                        closest_start = p.start;
                    }
                }
                if (closest_start > e.current_floor) e.direction = 1;
                else if (closest_start < e.current_floor) e.direction = -1;
            }
        }

        // 鎖定目標樓層為當前移動方向的盡頭
        if (e.direction == 1) e.target_floor = num_floors - 1;
        else if (e.direction == -1) e.target_floor = 0;

        // --- D. 執行移動 ---
        int old_f = e.current_floor;
        if (e.direction == 1) e.current_floor++;
        else if (e.direction == -1) e.current_floor--;

        // 若大廳與車內皆無任何需求，則進入 IDLE 狀態
        if (hall_queue.empty() && e.box.empty()) {
            e.direction = 0;
            e.target_floor = e.current_floor;
        }

        if (old_f != e.current_floor) {
            jout << "        { \"type\": \"ELEVATOR_MOVE\", \"elevatorId\": " << e.id << ", \"from\": " << old_f + 1 << ", \"to\": " << e.current_floor + 1 << ", \"direction\": \"" << (e.direction == 1 ? "UP" : "DOWN") << "\" }," << std::endl;
        }
    }

    std::ofstream out_state("state.txt");
    for (auto &e : elevators) {
        out_state << e.current_floor << " " << e.direction << " " << (int)e.box.size() << " " << e.target_floor << "\n";
        for (auto &p : e.box) out_state << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << " " << p.enter_tick << "\n";
    }
    saveHallQueue();
}