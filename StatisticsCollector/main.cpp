#include "Statistics.h"

int main() {
    Statistics stats;
    stats.processEventFile("events.txt");
    stats.outputStats("stats.txt");
}
