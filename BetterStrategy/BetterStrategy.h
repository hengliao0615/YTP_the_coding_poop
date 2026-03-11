#ifndef BETTER_STRATEGY_H
#define BETTER_STRATEGY_H

#include <vector>
#include <string>

// 乘客結構定義
struct Passenger {
    int id;
    int start;
    int dest;
    int spawn_tick;
    int enter_tick; // 新增：用於計算精確的等待時間與標準差
};

class BetterStrategy {
public:
    BetterStrategy();
    
    // 執行一輪調度決策
    void execute(int current_tick);

private:
    std::vector<Passenger> hall_queue; // 大廳等候隊列

    // --- 私有輔助函式：處理檔案讀寫 ---
    
    // 載入與儲存當前大廳乘客狀態
    void loadHallQueue();
    void saveHallQueue();

    // 載入與儲存各樓層歷史需求數據 (用於動態 Priority)
    std::vector<int> loadHistory(int num_floors);
    void saveHistory(const std::vector<int>& history);
};

#endif