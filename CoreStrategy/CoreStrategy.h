#ifndef CORE_STRATEGY_H
#define CORE_STRATEGY_H

#include <vector>

struct Passenger {
    int id;
    int start;
    int dest;
    int spawn_tick;
    int enter_tick; // 必須同步，否則編譯或讀取 state.txt 會出錯
};

class CoreStrategy {
public:
    CoreStrategy();
    void execute(int current_tick);

private:
    std::vector<Passenger> hall_queue;
    void loadHallQueue();
    void saveHallQueue();
};

#endif