#include "CoreStrategy.h"
#include <thread>
#include <chrono>

int main() {
    CoreStrategy cs;
    while (true) {
        cs.readNewPassengers("passengers.txt");
        cs.simulateStep("events.txt");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}