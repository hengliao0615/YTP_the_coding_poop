#ifndef PASSENGER_GENERATOR_H
#define PASSENGER_GENERATOR_H

#include <vector>
#include <random>
#include <string>

class PassengerGenerator {
public:
    PassengerGenerator();
    bool loadConfig(const std::string& filename);
    // 修正點：確保這裡有 int 參數
    void execute(int current_tick); 

private:
    int num_floors;
    double spawn_rate;
    std::vector<double> exit_weights;
    std::vector<double> entry_weights;
    int next_id;
    std::mt19937 gen;

    void updateNextID();
};

#endif