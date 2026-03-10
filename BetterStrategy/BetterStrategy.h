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
    // 增加從檔案讀寫歷史數據的函式
    void loadHallQueue();
    void saveHallQueue();
    std::vector<int> loadHistory(int num_floors);
    void saveHistory(const std::vector<int>& history);
};

#endif