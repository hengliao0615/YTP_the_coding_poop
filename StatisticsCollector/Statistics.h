#pragma once
#include <unordered_map>
#include <string>

struct Record {
    int spawn = 0;
    int pick = 0;
    int drop = 0;
};

class Statistics {
private:
    std::unordered_map<int, Record> records;
    int totalWait = 0;
    int totalCount = 0;

public:
    void processEventFile(const std::string& filename);
    void outputStats(const std::string& filename);
};
