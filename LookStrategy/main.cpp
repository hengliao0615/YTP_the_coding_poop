#include "LookStrategy.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    int current_tick = 0;
    if (argc > 1) {
        try {
            current_tick = std::stoi(argv[1]);
        } catch (...) {
            current_tick = 0;
        }
    }

    LookStrategy strategy;
    strategy.execute(current_tick);

    return 0;
}