#ifndef PASSENGER_GENERATOR_H
#define PASSENGER_GENERATOR_H
#include <vector>
#include <random>
#include <string>

class PassengerGenerator {
public:
    PassengerGenerator();
    bool loadConfig(const std::string& filename);
    void generateAndSave(const std::string& output_file);
private:
    int next_id;
    int num_floors;
    double spawn_rate;
    std::vector<double> exit_weights;
    std::vector<double> entry_weights;
    std::default_random_engine gen;
};
#endif