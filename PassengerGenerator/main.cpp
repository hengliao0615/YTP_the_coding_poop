#include "PassengerGenerator.h"
#include <thread>
#include <chrono>

int main() {
    PassengerGenerator gen;
    gen.loadConfig("config.txt");

    for (int t = 0; t < 1000; t++) {
        auto passengers = gen.generate();
        gen.outputPassengers(passengers, "passengers.txt");
        gen.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
