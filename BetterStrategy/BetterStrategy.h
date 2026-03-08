#ifndef BETTER_STRATEGY_H
#define BETTER_STRATEGY_H

#include <vector>
#include <string>

struct Passenger {
    int id;
    int start;
    int dest;
    int spawn_tick;
};

class BetterStrategy {
public:
    BetterStrategy();
    void execute(int current_tick);

private:
    std::vector<Passenger> hall_queue;
    void loadHallQueue();
    void saveHallQueue();
};

#endif