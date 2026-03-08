#include <iostream>
#include <fstream>

int main() {
    int count = 0;
    std::ifstream ev("events.txt");
    int id, s, d;
    while (ev >> id >> s >> d) count++;
    
    std::ofstream out("stats.txt");
    out << "Total_Arrivals: " << count << std::endl;
    std::cout << "[Stats] Total served: " << count << std::endl;
    return 0;
}