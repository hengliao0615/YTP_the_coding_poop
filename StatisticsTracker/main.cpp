#include "StatisticsTracker.h"
#include <thread>
#include <chrono>

int main() {
    StatisticsTracker st;
    while (true) {
        st.processEvents("events.txt");
        st.updateReport("stats.txt");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}