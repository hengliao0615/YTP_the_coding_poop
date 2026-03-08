#include "CoreStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>

CoreStrategy::CoreStrategy() {}

void CoreStrategy::loadHallQueue() {
    hall_queue.clear();
    std::ifstream in("passengers.txt");
    Passenger p;
    // 讀取 ID, Start, Dest, SpawnTick
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) {
        hall_queue.push_back(p);
    }
}

void CoreStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) {
        out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
    }
}

struct Elevator {
    int id;
    int current_floor;
    int direction; 
    int box_size;
    std::vector<Passenger> box;
};

void CoreStrategy::execute(int current_tick) {
    int num_floors, capacity, num_elevators;
    double spawn_rate;
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors >> spawn_rate >> capacity >> num_elevators)) return;
    cfg.close();

    std::vector<Elevator> elevators(num_elevators);
    std::ifstream in_state("state.txt");
    for (int i = 0; i < num_elevators; ++i) {
        elevators[i].id = i;
        int dummy_target; // 為了相容 BetterStrategy 的 4 參數格式
        if (!(in_state >> elevators[i].current_floor >> elevators[i].direction >> elevators[i].box_size >> dummy_target)) {
            elevators[i].current_floor = 0;
            elevators[i].direction = 0;
        } else {
            for (int j = 0; j < elevators[i].box_size; ++j) {
                Passenger p;
                in_state >> p.id >> p.start >> p.dest >> p.spawn_tick;
                elevators[i].box.push_back(p);
            }
        }
    }
    in_state.close();

    loadHallQueue();
    std::ofstream jout("tmp_event.json", std::ios::app);

    for (auto &e : elevators) {
        // 1. 下車邏輯
        auto it = e.box.begin();
        while (it != e.box.end()) {
            if (it->dest == e.current_floor) {
                // 紀錄到 events.txt 用於後續統計 (包含開始與結束時間)
                std::ofstream ev("events.txt", std::ios::app);
                ev << it->id << " " << it->start << " " << it->dest << " " 
                   << it->spawn_tick << " " << current_tick << std::endl;
                
                jout << "        { \"type\": \"PASSENGER_EXIT\", \"passengerId\": \"P" << it->id 
                     << "\", \"floor\": " << e.current_floor + 1 << ", \"elevatorId\": " << e.id << " }," << std::endl;
                it = e.box.erase(it);
            } else ++it;
        }

        // 2. 上車邏輯 (順路)
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            if (hit->start == e.current_floor && (int)e.box.size() < capacity) {
                e.box.push_back(*hit);
                jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id 
                     << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                hit = hall_queue.erase(hit);
            } else ++hit;
        }

        // 3. SCAN 演算法決策
        int old_floor = e.current_floor;
        bool has_req_above = false, has_req_below = false;

        // 檢查車內目的地
        for (const auto& p : e.box) {
            if (p.dest > e.current_floor) has_req_above = true;
            if (p.dest < e.current_floor) has_req_below = true;
        }
        // 檢查大廳等待者
        for (const auto& p : hall_queue) {
            if (p.start > e.current_floor) has_req_above = true;
            if (p.start < e.current_floor) has_req_below = true;
        }

        // 方向轉換邏輯
        if (e.direction == 1) { // 向上中
            if (!has_req_above) e.direction = has_req_below ? -1 : 0;
        } else if (e.direction == -1) { // 向下中
            if (!has_req_below) e.direction = has_req_above ? 1 : 0;
        } else { // 靜止中
            if (has_req_above) e.direction = 1;
            else if (has_req_below) e.direction = -1;
        }

        // 執行移動
        if (e.direction == 1 && e.current_floor < num_floors - 1) e.current_floor++;
        else if (e.direction == -1 && e.current_floor > 0) e.current_floor--;

        if (old_floor != e.current_floor) {
            jout << "        { \"type\": \"ELEVATOR_MOVE\", \"elevatorId\": " << e.id << ", \"from\": " << old_floor + 1
                 << ", \"to\": " << e.current_floor + 1 << ", \"direction\": \"" 
                 << (e.direction == 1 ? "UP" : "DOWN") << "\" }," << std::endl;
        }
    }

    // 4. 寫回狀態 (補上 dummy 的 0 作為第四個參數)
    std::ofstream out_state("state.txt");
    for (auto &e : elevators) {
        out_state << e.current_floor << " " << e.direction << " " << e.box.size() << " 0\n";
        for (auto &p : e.box) out_state << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
    }
    out_state.close();
    saveHallQueue();
}