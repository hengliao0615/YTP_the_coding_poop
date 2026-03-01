#pragma once
#include <vector>
#include <set>

class Elevator {
public:
    int id;
    int currentFloor;
    int direction;
    std::set<int> upStops;
    std::set<int> downStops;
    std::vector<int> passengers;

    Elevator(int id, int startFloor);
    void step();
};
