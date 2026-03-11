#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <ctime>

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    int current_tick = std::stoi(argv[1]);

    // 1. 讀取設定
    int num_floors, capacity, num_elevators;
    double lambda; 
    std::ifstream cfg("config.txt");
    if (!(cfg >> num_floors >> lambda >> capacity >> num_elevators)) return 1;
    cfg.close();

    // 2. 準備隨機數引擎
    static std::default_random_engine generator(time(0) + current_tick);
    std::poisson_distribution<int> distribution(lambda);
    std::uniform_int_distribution<int> floor_dist(0, num_floors - 1);

    // 3. 決定這一個 Tick 要生成多少人
    int num_new_passengers = distribution(generator);

    if (num_new_passengers > 0) {
        // 讀取 ID 計數器
        int last_id = 0;
        std::ifstream id_in("id_counter.txt");
        id_in >> last_id;
        id_in.close();

        std::ofstream out("passengers.txt", std::ios::app);
        // 新增：開啟 JSON 暫存檔以記錄生成事件
        std::ofstream jout("tmp_event.json", std::ios::app);

        for (int i = 0; i < num_new_passengers; ++i) {
            int start = floor_dist(generator);
            int dest = floor_dist(generator);
            while (dest == start) dest = floor_dist(generator);

            last_id++;
            out << last_id << " " << start << " " << dest << " " << current_tick << "\n";
            
            // 寫入 JSON 事件：讓視覺化工具知道哪層樓有人出現了
            jout << "        { \"type\": \"PASSENGER_SPAWN\", \"passengerId\": \"P" << last_id 
                 << "\", \"from\": " << start + 1 << ", \"to\": " << dest + 1 << " }," << std::endl;
        }
        out.close();
        jout.close();

        std::ofstream id_out("id_counter.txt");
        id_out << last_id;
        id_out.close();
    }

    return 0;
}