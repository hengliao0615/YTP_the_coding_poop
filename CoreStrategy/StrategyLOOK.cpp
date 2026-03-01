#include "StrategyLOOK.h"
#include <fstream>

using namespace std;

StrategyLOOK::StrategyLOOK(int floors)
    : floorCount(floors), elevator(1, 0) {}

void StrategyLOOK::addPassenger(int id, int start, int target, int dir) {
    waiting[id] = {start, target, dir};
    if (dir == 1)
        elevator.upStops.insert(start);
    else
        elevator.downStops.insert(start);
}

void StrategyLOOK::step(int currentTime, const string& eventFile) {
    ofstream out(eventFile, ios::app);
    out << "TIME " << currentTime << "\n";

    for (auto it = waiting.begin(); it != waiting.end();) {
        if (it->second.start == elevator.currentFloor) {
            out << "P_PICK " << it->first << " 1\n";

            if (it->second.direction == 1)
                elevator.upStops.insert(it->second.target);
            else
                elevator.downStops.insert(it->second.target);

            it = waiting.erase(it);
        } else {
            ++it;
        }
    }

    if (elevator.direction == 1 &&
        elevator.upStops.count(elevator.currentFloor)) {
        out << "E_OPEN 1 " << elevator.currentFloor << "\n";
        elevator.upStops.erase(elevator.currentFloor);
    }

    if (elevator.direction == -1 &&
        elevator.downStops.count(elevator.currentFloor)) {
        out << "E_OPEN 1 " << elevator.currentFloor << "\n";
        elevator.downStops.erase(elevator.currentFloor);
    }

    elevator.step();
    out << "E_MOVE 1 " << elevator.currentFloor << "\n";
}
