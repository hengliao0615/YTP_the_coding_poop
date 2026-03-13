#include "PassengerGenerator.h"
#include <fstream>
#include <iostream>

PassengerGenerator::PassengerGenerator() : next_id(1) {
    std::random_device rd;
    gen.seed(rd());
}

void PassengerGenerator::updateNextID() {
    std::ifstream in("id_counter.txt");
    if (in >> next_id) {
        // ID 會在迴圈中逐次遞增，這裡只負責讀取當前基準
    } else {
        next_id = 0; // 若無檔案，從 0 開始
    }
    in.close();
}

bool PassengerGenerator::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    int cap, e_count;
    // 讀取 num_floors, spawn_rate (lambda), capacity, num_elevators
    if (!(file >> num_floors >> spawn_rate >> cap >> e_count)) return false;

    exit_weights.clear();
    entry_weights.clear();
    double val;
    // 讀取各樓層出現權重
    for(int i=0; i<num_floors; ++i) {
        if(file >> val) exit_weights.push_back(val);
        else exit_weights.push_back(1.0);
    }
    // 讀取各樓層目的地權重
    for(int i=0; i<num_floors; ++i) {
        if(file >> val) entry_weights.push_back(val);
        else entry_weights.push_back(1.0);
    }
    return true;
}

void PassengerGenerator::execute(int current_tick) {
    if (exit_weights.empty() || entry_weights.empty()) return;

    // 1. 使用泊松分佈決定「這一個 Tick 總共要生成幾個人」
    // spawn_rate 在這裡扮演 λ (期望值)
    std::poisson_distribution<int> poisson_dist(spawn_rate);
    int num_to_spawn = poisson_dist(gen);

    if (num_to_spawn <= 0) return;

    // 2. 準備離散分佈來選取樓層
    std::discrete_distribution<int> start_d(exit_weights.begin(), exit_weights.end());
    std::discrete_distribution<int> dest_d(entry_weights.begin(), entry_weights.end());

    // 3. 讀取並準備更新 ID 計數器
    updateNextID();

    std::ofstream out("passengers.txt", std::ios::app);
    std::ofstream jout("tmp_event.json", std::ios::app);

    for (int i = 0; i < num_to_spawn; ++i) {
        int s = start_d(gen);
        int d = dest_d(gen);

        // 確保起始樓層不等於目的地
        while (s == d) {
            d = dest_d(gen);
        }

        next_id++; // 每個乘客 ID 唯一
        
        // 寫入乘客資料
        out << next_id << " " << s << " " << d << " " << current_tick << std::endl;

        // 寫入 JSON 事件
        jout << "        { \"type\": \"PASSENGER_SPAWN\", \"passengerId\": \"P" << next_id 
             << "\", \"from\": " << s + 1 << ", \"to\": " << d + 1 
             << ", \"spawnTick\": " << current_tick << " }," << std::endl;

        // std::cout << "[Gen] P" << next_id << " @ Tick " << current_tick << ": " << s + 1 << " -> " << d + 1 << std::endl;
    }

    out.close();
    jout.close();

    // 4. 最後回寫最新的 ID 計數器
    std::ofstream id_out("id_counter.txt");
    id_out << next_id;
    id_out.close();
}