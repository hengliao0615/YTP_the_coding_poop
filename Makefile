# 編譯器與參數設定
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I.

# 執行檔目標路徑定義 (維持你原本的子目錄架構)
GEN_TARGET = PassengerGenerator/generator
CORE_STR_TARGET = CoreStrategy/core_strategy
LOOK_STR_TARGET = LookStrategy/look_strategy
BETTER_STR_TARGET = BetterStrategy/better_strategy
STA_TARGET = StatisticsTracker/stats

# 預設編譯所有目標並初始化環境
all: init_files $(GEN_TARGET) $(CORE_STR_TARGET) $(LOOK_STR_TARGET) $(BETTER_STR_TARGET) $(STA_TARGET)

# 初始化模擬所需的暫存檔與目錄
init_files:
	@mkdir -p PassengerGenerator CoreStrategy LookStrategy BetterStrategy StatisticsTracker
	@# 檢查 config.txt 是否存在，若無則產生預設 12 樓、4 電梯的設定
	@if [ ! -f config.txt ]; then \
		echo "12 0.5 10 4" > config.txt; \
		echo "1 1 1 1 1 1 1 1 1 1 1 1" >> config.txt; \
		echo "1 1 1 1 1 1 1 1 1 1 1 1" >> config.txt; \
	fi
	@# 建立必要的資料交換檔
	@touch passengers.txt events.txt stats.txt state.txt id_counter.txt history.txt

# 1. 編譯乘客生成器
$(GEN_TARGET): PassengerGenerator/main.cpp PassengerGenerator/PassengerGenerator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# 2. 編譯基準組：SCAN 策略
$(CORE_STR_TARGET): CoreStrategy/main.cpp CoreStrategy/CoreStrategy.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# 3. 編譯 LOOK 策略 (新加入)
$(LOOK_STR_TARGET): LookStrategy/main.cpp LookStrategy/LookStrategy.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# 4. 編譯實驗組：優化後 Better 策略
$(BETTER_STR_TARGET): BetterStrategy/main.cpp BetterStrategy/BetterStrategy.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# 5. 編譯統計追蹤器
$(STA_TARGET): StatisticsTracker/main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# 清理編譯產物與模擬過程中產生的所有數據檔案
clean:
	rm -f $(GEN_TARGET) $(CORE_STR_TARGET) $(LOOK_STR_TARGET) $(BETTER_STR_TARGET) $(STA_TARGET)
	rm -f passengers.txt events.txt stats.txt state.txt id_counter.txt history.txt tmp_event.json log.json