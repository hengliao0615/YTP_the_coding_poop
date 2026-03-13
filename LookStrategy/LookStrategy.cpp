#include "LookStrategy.h"
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

LookStrategy::LookStrategy() {}

void LookStrategy::loadHallQueue() {
    hall_queue.clear();
    std::ifstream in("passengers.txt");
    Passenger p;
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) {
        p.enter_tick = -1;
        hall_queue.push_back(p);
    }
}

void LookStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
}

void LookStrategy::execute(int current_tick) {
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

        // 新增：下車完如目前車廂是空的，先把 e.direction 設為 0
        if (e.box.empty()) {
            e.direction = 0;
        }

        // std::cout<<"check"<<std::endl;
        // std::cout<<e.id<<' '<<e.direction<<std::endl;

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

        // if(!e.box.empty()){
        //     std::cout<<"dumb"<<std::endl;
        // }

        // --- C. 決策邏輯 (LOOK) ---
        bool has_call_ahead = false;

        if (e.direction == 1) { // 向上移動中
            // 檢查更高樓層是否有人等車，或車內乘客要去更高樓層
            for (const auto& p : hall_queue) if (p.start > e.current_floor) has_call_ahead = true;
            for (const auto& p : e.box) if (p.dest > e.current_floor) has_call_ahead = true;
            
            if (!has_call_ahead) e.direction = -1; // 前方沒人，轉向
        } 
        else if (e.direction == -1) { // 向下移動中
            // 檢查更低樓層是否有人等車，或車內乘客要去更低樓層
            for (const auto& p : hall_queue) if (p.start < e.current_floor) has_call_ahead = true;
            for (const auto& p : e.box) if (p.dest < e.current_floor) has_call_ahead = true;

            if (!has_call_ahead) e.direction = 1; // 前方沒人，轉向
        }

        // 若電梯原本靜止或剛才轉向後發現全棟都沒請求
        bool any_call = !hall_queue.empty() || !e.box.empty();
        if (!any_call) {
            e.direction = 0;
            e.target_floor = e.current_floor;
        } else if (e.direction == 0) {
            // 尋找最近的請求來決定初始方向
            int min_dist = 999;
            for (const auto& p : hall_queue) {
                if (std::abs(p.start - e.current_floor) < min_dist) {
                    min_dist = std::abs(p.start - e.current_floor);
                    e.direction = (p.start >= e.current_floor) ? 1 : -1;
                }
            }
        }

        // --- D. 執行移動 ---
        int old_f = e.current_floor;
        if (e.direction == 1) e.current_floor++;
        else if (e.direction == -1) e.current_floor--;

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