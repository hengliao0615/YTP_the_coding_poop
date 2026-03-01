#ifndef CORE_STRATEGY_H
#define CORE_STRATEGY_H
#include <vector>
#include <string>

struct Passenger {
    int id, start, dest;
    bool boarded = false;
};

class CoreStrategy {
public:
    CoreStrategy();
    void readNewPassengers(const std::string& input_file);
    void simulateStep(const std::string& event_file);
private:
    int current_floor;
    long last_read_pos;
    std::vector<Passenger> waiting_list;
};
#endif