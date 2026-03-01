#include "PassengerGenerator.h"
#include <thread>
#include <chrono>

int main() {
    PassengerGenerator pg;
    while (true) {
        if (pg.loadConfig("config.txt")) {
            pg.generateAndSave("passengers.txt");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}