#include "PassengerGenerator.h"
#include <fstream>
#include <iostream>

PassengerGenerator::PassengerGenerator() : next_id(1) {
    std::random_device rd;
    gen.seed(rd());
}

void PassengerGenerator::updateNextID() {
    std::ifstream in("id_counter.txt");
    if (in >> next_id) next_id++;
    else next_id = 1;
    in.close();

    std::ofstream out("id_counter.txt");
    out << next_id;
    out.close();
}

bool PassengerGenerator::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    int cap, e_count;
    if (!(file >> num_floors >> spawn_rate >> cap >> e_count)) return false;

    exit_weights.clear();
    entry_weights.clear();
    double val;
    for(int i=0; i<num_floors; ++i) {
        if(file >> val) exit_weights.push_back(val);
        else exit_weights.push_back(1.0);
    }
    for(int i=0; i<num_floors; ++i) {
        if(file >> val) entry_weights.push_back(val);
        else entry_weights.push_back(1.0);
    }
    return true;
}

// 接收 current_tick 並紀錄
void PassengerGenerator::execute(int current_tick) {
    std::uniform_real_distribution<double> dist(0, 1);
    if (dist(gen) < spawn_rate) {
        if (exit_weights.empty() || entry_weights.empty()) return;

        std::discrete_distribution<int> start_d(exit_weights.begin(), exit_weights.end());
        std::discrete_distribution<int> dest_d(entry_weights.begin(), entry_weights.end());
        
        int s = start_d(gen);
        int d = dest_d(gen);

        if (s != d) {
            updateNextID();
            // 寫入乘客隊列，增加 spawn_tick
            std::ofstream out("passengers.txt", std::ios::app);
            out << next_id << " " << s << " " << d << " " << current_tick << std::endl;

            // JSON 輸出也包含 spawnTick 方便 debug 與視覺化
            std::ofstream jout("tmp_event.json", std::ios::app);
            jout << "        { \"type\": \"PASSENGER_SPAWN\", \"passengerId\": \"P" << next_id 
                 << "\", \"from\": " << s + 1 << ", \"to\": " << d + 1 
                 << ", \"spawnTick\": " << current_tick << " }," << std::endl;

            std::cout << "[Gen] P" << next_id << " @ Tick " << current_tick << ": " << s + 1 << " -> " << d + 1 << std::endl;
        }
    }
}