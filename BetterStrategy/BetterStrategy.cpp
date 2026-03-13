#include "BetterStrategy.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

using namespace std;

// --- 調度參數定義 ---
const double ALPHA = 100.0; 
const double BETA  = 15.0;  
const double GAMMA = 10;  
const double C1    = 20;  // 即時等待人數權重
const double C2    = 3;  // 歷史需求權重

std::random_device rd;
std::mt19937 gen_rand(rd());

struct Elevator {
    int id;
    int current_floor;
    int direction; 
    int box_size;
    int target_floor;
    std::vector<Passenger> box;
};

BetterStrategy::BetterStrategy() {}

// --- 資料持久化 ---
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
    // 增加讀取 EnterTick 欄位 (預設為 -1)
    while (in >> p.id >> p.start >> p.dest >> p.spawn_tick) {
        p.enter_tick = -1; 
        hall_queue.push_back(p);
    }
}

void BetterStrategy::saveHallQueue() {
    std::ofstream out("passengers.txt");
    for (const auto& p : hall_queue) out << p.id << " " << p.start << " " << p.dest << " " << p.spawn_tick << "\n";
}

void BetterStrategy::execute(int current_tick) {
    int num_floors, capacity, num_elevators;
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors)) return;
    double lambda; cfg >> lambda >> capacity >> num_elevators;
    cfg.close();

    loadHallQueue();
    std::vector<int> history_count = loadHistory(num_floors);

    for (const auto& p : hall_queue) {
        if (p.spawn_tick == current_tick) history_count[p.start]++;
    }
    saveHistory(history_count);

    std::vector<Elevator> elevators(num_elevators);
    std::ifstream in_state("state.txt");
    for (int i = 0; i < num_elevators; ++i) {
        elevators[i].id = i;
        if (!(in_state >> elevators[i].current_floor >> elevators[i].direction >> elevators[i].box_size >> elevators[i].target_floor)) {
            elevators[i].current_floor = 0; elevators[i].direction = 0; elevators[i].target_floor = 0;
        }
        for (int j = 0; j < elevators[i].box_size; ++j) {
            Passenger p; in_state >> p.id >> p.start >> p.dest >> p.spawn_tick >> p.enter_tick;
            elevators[i].box.push_back(p);
        }
    }
    in_state.close();

    std::ofstream jout("tmp_event.json", std::ios::app);

    std::vector<double> dynamic_priority(num_floors, 0.0);
    for (int i = 0; i < num_floors; ++i) {
        int wait_count = 0;
        for (const auto& p : hall_queue) if (p.start == i) wait_count++;
        dynamic_priority[i] = (C1 * wait_count) + (C2 * history_count[i]);
    }

    for (auto &e : elevators) {
        // --- A. 下車邏輯 (含 EnterTick 紀錄輸出) ---
        auto it = e.box.begin();
        while (it != e.box.end()) {
            if (it->dest == e.current_floor) {
                std::ofstream ev("events.txt", std::ios::app);
                // 格式：ID Start Dest Spawn Enter Exit
                ev << it->id << " " << it->start << " " << it->dest << " " 
                   << it->spawn_tick << " " << it->enter_tick << " " << current_tick << std::endl;
                
                jout << "        { \"type\": \"PASSENGER_EXIT\", \"passengerId\": \"P" << it->id << "\", \"floor\": " << e.current_floor + 1 << ", \"elevatorId\": " << e.id << " }," << std::endl;
                it = e.box.erase(it);
            } else ++it;
        }

        if (e.box.empty()) e.direction = 0;

        // --- B. 上車邏輯 (寫入 EnterTick) ---
        auto hit = hall_queue.begin();
        while (hit != hall_queue.end()) {
            int p_dir = (hit->dest > hit->start) ? 1 : -1;
            if (hit->start == e.current_floor && (int)e.box.size() < capacity && (e.direction == 0 || e.direction == p_dir)) {
                Passenger p = *hit;
                p.enter_tick = current_tick; // 乘客上車的瞬間
                e.box.push_back(p);
                if (e.direction == 0) e.direction = p_dir;
                jout << "        { \"type\": \"PASSENGER_ENTER\", \"passengerId\": \"P" << hit->id << "\", \"elevatorId\": " << e.id << ", \"floor\": " << e.current_floor + 1 << " }," << std::endl;
                hit = hall_queue.erase(hit);
            } else ++hit;
        }

        // --- C. 決策邏輯 (穩定模式 + 機率選取) ---
        if (!e.box.empty()) {
            int best_target = e.box[0].dest;
            int min_d = std::abs(e.current_floor - best_target);
            for (const auto& p : e.box) {
                if (std::abs(e.current_floor - p.dest) < min_d) {
                    min_d = std::abs(e.current_floor - p.dest);
                    best_target = p.dest;
                }
            }
            e.target_floor = best_target;
        } 
        else if (e.current_floor == e.target_floor || e.direction == 0) {
            struct Candidate { int floor; double score; };
            std::vector<Candidate> candidates;
            double min_score = 1e9;
            // cout<<"calculating elevator "<<e.id<<" at "<<e.current_floor<<endl;
            for (int f = 0; f < num_floors; ++f) {
                if (f == e.current_floor) continue;
                int max_w = -1;
                for (auto &p : hall_queue) if (p.start == f) max_w = std::max(max_w, current_tick - p.spawn_tick);
                if (max_w != -1) {
                    double s = (ALPHA * dynamic_priority[f]) + (BETA * max_w) - (GAMMA * std::abs(e.current_floor - f));
                    candidates.push_back({f, s});
                    // cout<<"floor: "<<f+1<<' '<<s<<endl;
                    if (s < min_score) min_score = s;
                }
            }

            if (!candidates.empty()) {
                double total_w = 0;
                std::vector<double> ws;
                for (auto &c : candidates) {
                    double w = c.score - min_score + 1.0;
                    ws.push_back(w);
                    total_w += w;
                }
                std::uniform_real_distribution<> dis(0, total_w);
                double pick = dis(gen_rand);
                double cur_s = 0;
                for (size_t i = 0; i < candidates.size(); ++i) {
                    cur_s += ws[i];
                    if (pick <= cur_s) { e.target_floor = candidates[i].floor; break; }
                }
                // cout<<"chose: "<<e.target_floor<<endl;
            } else {
                e.target_floor = e.current_floor; e.direction = 0;
            }
        }

        // --- D. 移動 ---
        int old_f = e.current_floor;
        if (e.target_floor > e.current_floor) { e.direction = 1; e.current_floor++; }
        else if (e.target_floor < e.current_floor) { e.direction = -1; e.current_floor--; }
        else e.direction = 0;

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