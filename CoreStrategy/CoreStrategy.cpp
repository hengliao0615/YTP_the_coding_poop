#include "CoreStrategy.h"
#include <fstream>
#include <iostream>

CoreStrategy::CoreStrategy() : current_floor(0), last_read_pos(0) {}

void CoreStrategy::readNewPassengers(const std::string& input_file) {
    std::ifstream in(input_file);
    if (!in.is_open()) return;
    in.seekg(last_read_pos);
    Passenger p;
    while (in >> p.id >> p.start >> p.dest) {
        waiting_list.push_back(p);
        last_read_pos = in.tellg();
    }
}

void CoreStrategy::simulateStep(const std::string& event_file) {
    if (waiting_list.empty()) return;
    Passenger& t = waiting_list.front();
    int goal = t.boarded ? t.dest : t.start;

    if (current_floor < goal) current_floor++;
    else if (current_floor > goal) current_floor--;
    else {
        if (!t.boarded) {
            t.boarded = true;
            std::cout << "[Strategy] Passenger " << t.id << " boarded at floor " << current_floor << std::endl;
        } else {
            std::ofstream out(event_file, std::ios::app);
            out << t.id << " " << t.start << " " << t.dest << "\n";
            std::cout << "[Strategy] Passenger " << t.id << " arrived at floor " << t.dest << std::endl;
            waiting_list.erase(waiting_list.begin());
        }
    }
    std::cout << "[Strategy] Elevator at floor: " << current_floor << std::endl;
}