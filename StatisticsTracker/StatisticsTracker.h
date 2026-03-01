#ifndef STATISTICS_TRACKER_H
#define STATISTICS_TRACKER_H
#include <string>

class StatisticsTracker {
public:
    StatisticsTracker();
    void processEvents(const std::string& event_file);
    void updateReport(const std::string& report_file);
private:
    int total_served;
    long last_read_pos;
};
#endif