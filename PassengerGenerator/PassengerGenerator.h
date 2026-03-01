#pragma once
#include <vector>
#include <string>

struct Passenger {
    int id;
    int startFloor;
    int targetFloor;
    int direction;
    int spawnTime;
};

class PassengerGenerator {
private:
    int floorCount;
    double generationRate;
    std::vector<double> enterProb;
    std::vector<double> exitProb;
    int currentTime;
    int nextPassengerId;

public:
    PassengerGenerator();
    void loadConfig(const std::string& filename);
    std::vector<Passenger> generate();
    void outputPassengers(const std::vector<Passenger>& passengers, const std::string& filename);
    void tick();
};
