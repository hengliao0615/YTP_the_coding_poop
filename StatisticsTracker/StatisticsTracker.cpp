#include "StatisticsTracker.h"
#include <fstream>
#include <iostream>

StatisticsTracker::StatisticsTracker() : total_served(0), last_read_pos(0) {}

void StatisticsTracker::processEvents(const std::string& event_file) {
    std::ifstream in(event_file);
    if (!in.is_open()) return;
    in.seekg(last_read_pos);
    int id, s, d;
    while (in >> id >> s >> d) {
        total_served++;
        last_read_pos = in.tellg();
        std::cout << "[Stats] Processed event for passenger " << id << std::endl;
    }
}

void StatisticsTracker::updateReport(const std::string& report_file) {
    std::ofstream out(report_file);
    out << "Total_Served: " << total_served << "\n";
}