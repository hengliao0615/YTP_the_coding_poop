#include "StrategyLOOK.h"
#include <fstream>
#include <thread>
#include <chrono>

int main() {
    StrategyLOOK strategy(10);
    int currentTime = 0;

    for (int t = 0; t < 1000; t++) {
        std::ifstream in("passengers.txt");
        std::string tag;
        while (in >> tag) {
            if (tag == "TIME") {
                in >> currentTime;
            } else if (tag == "P") {
                int id, start, target, dir;
                in >> id >> start >> target >> dir;
                strategy.addPassenger(id, start, target, dir);
            }
        }

        strategy.step(currentTime, "events.txt");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
