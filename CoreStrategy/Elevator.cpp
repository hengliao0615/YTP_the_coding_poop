#include "Elevator.h"

Elevator::Elevator(int id_, int startFloor) {
    id = id_;
    currentFloor = startFloor;
    direction = 1;
}

void Elevator::step() {
    if (direction == 1) {
        if (!upStops.empty()) {
            currentFloor++;
        } else if (!downStops.empty()) {
            direction = -1;
        }
    } else {
        if (!downStops.empty()) {
            currentFloor--;
        } else if (!upStops.empty()) {
            direction = 1;
        }
    }
}
