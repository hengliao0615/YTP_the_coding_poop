#include "BetterStrategy.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <current_tick>" << std::endl;
        return 1;
    }

    int current_tick = std::stoi(argv[1]);
    BetterStrategy strategy;
    strategy.execute(current_tick);

    return 0;
}