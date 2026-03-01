#include "PassengerGenerator.h"
#include <fstream>
#include <iostream>

PassengerGenerator::PassengerGenerator() : next_id(1) {
    std::random_device rd;
    gen.seed(rd());
}

bool PassengerGenerator::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!(file >> num_floors >> spawn_rate)) return false;
    exit_weights.assign(num_floors, 0);
    entry_weights.assign(num_floors, 0);
    for(int i=0; i<num_floors; ++i) file >> exit_weights[i];
    for(int i=0; i<num_floors; ++i) file >> entry_weights[i];
    return true;
}

void PassengerGenerator::generateAndSave(const std::string& output_file) {
    std::uniform_real_distribution<double> dist(0, 1);
    if (dist(gen) < spawn_rate) {
        std::discrete_distribution<int> start_d(exit_weights.begin(), exit_weights.end());
        std::discrete_distribution<int> dest_d(entry_weights.begin(), entry_weights.end());
        int s = start_d(gen), d = dest_d(gen);
        if (s != d) {
            std::ofstream out(output_file, std::ios::app);
            out << next_id++ << " " << s << " " << d << "\n";
            std::cout << "[Gen] Created Passenger " << next_id-1 << " (" << s << "->" << d << ")" << std::endl;
        }
    }
}