#ifndef LOOK_STRATEGY_H
#define LOOK_STRATEGY_H

#include <vector>

// 保持與 CoreStrategy 相同的乘客結構
struct Passenger {
    int id;
    int start;
    int dest;
    int spawn_tick;
    int enter_tick;
};

class LookStrategy {
public:
    LookStrategy();
    void execute(int current_tick);

private:
    std::vector<Passenger> hall_queue;
    void loadHallQueue();
    void saveHallQueue();
};

#endif