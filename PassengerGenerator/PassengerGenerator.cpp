#include "PassengerGenerator.h"
#include <fstream>
#include <random>

using namespace std;

PassengerGenerator::PassengerGenerator() {
    currentTime = 0;
    nextPassengerId = 1;
}

void PassengerGenerator::loadConfig(const string& filename) {
    ifstream in(filename);
    string tag;

    in >> tag >> floorCount;
    in >> tag >> generationRate;

    enterProb.resize(floorCount);
    exitProb.resize(floorCount);

    in >> tag;
    for (int i = 0; i < floorCount; i++)
        in >> enterProb[i];

    in >> tag;
    for (int i = 0; i < floorCount; i++)
        in >> exitProb[i];
}

vector<Passenger> PassengerGenerator::generate() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(0,1);

    vector<Passenger> result;

    if (dis(gen) < generationRate) {
        discrete_distribution<> startDist(exitProb.begin(), exitProb.end());
        discrete_distribution<> endDist(enterProb.begin(), enterProb.end());

        int start = startDist(gen);
        int target = endDist(gen);

        if (start != target) {
            Passenger p;
            p.id = nextPassengerId++;
            p.startFloor = start;
            p.targetFloor = target;
            p.direction = (target > start) ? 1 : -1;
            p.spawnTime = currentTime;
            result.push_back(p);
        }
    }

    return result;
}

void PassengerGenerator::outputPassengers(const vector<Passenger>& passengers, const string& filename) {
    ofstream out(filename, ios::app);

    if (!passengers.empty()) {
        out << "TIME " << currentTime << "\n";
        for (auto& p : passengers) {
            out << "P " << p.id << " "
                << p.startFloor << " "
                << p.targetFloor << " "
                << p.direction << "\n";
        }
    }
}

void PassengerGenerator::tick() {
    currentTime++;
}
