#pragma once
#include "Elevator.h"
#include <vector>
#include <unordered_map>
#include <string>

struct PassengerInfo {
    int start;
    int target;
    int direction;
};

class StrategyLOOK {
private:
    int floorCount;
    Elevator elevator;
    std::unordered_map<int, PassengerInfo> waiting;

public:
    StrategyLOOK(int floors);
    void addPassenger(int id, int start, int target, int dir);
    void step(int currentTime, const std::string& eventFile);
};
