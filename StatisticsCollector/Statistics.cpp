#include "Statistics.h"
#include <fstream>

using namespace std;

void Statistics::processEventFile(const string& filename) {
    ifstream in(filename);
    string tag;
    int currentTime = 0;

    while (in >> tag) {
        if (tag == "TIME") {
            in >> currentTime;
        } else if (tag == "P_PICK") {
            int id, e;
            in >> id >> e;
            records[id].pick = currentTime;
        } else if (tag == "P_DROP") {
            int id, e;
            in >> id >> e;
            records[id].drop = currentTime;
            totalWait += (records[id].pick - records[id].spawn);
            totalCount++;
        }
    }
}

void Statistics::outputStats(const string& filename) {
    ofstream out(filename);
    if (totalCount == 0) return;
    out << "TOTAL_AVG_WAIT "
        << (double)totalWait / totalCount << "\n";
}
