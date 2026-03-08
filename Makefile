CXX = g++
CXXFLAGS = -std=c++11 -Wall -I.

# 執行檔路徑
GEN_TARGET = PassengerGenerator/generator
CORE_STR_TARGET = CoreStrategy/core_strategy
BETTER_STR_TARGET = BetterStrategy/better_strategy
STA_TARGET = StatisticsTracker/stats

# 編譯所有目標
all: init_files $(GEN_TARGET) $(CORE_STR_TARGET) $(BETTER_STR_TARGET) $(STA_TARGET)

init_files:
	@mkdir -p PassengerGenerator CoreStrategy BetterStrategy StatisticsTracker
	@touch passengers.txt events.txt stats.txt state.txt id_counter.txt

$(GEN_TARGET): PassengerGenerator/main.cpp PassengerGenerator/PassengerGenerator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CORE_STR_TARGET): CoreStrategy/main.cpp CoreStrategy/CoreStrategy.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BETTER_STR_TARGET): BetterStrategy/main.cpp BetterStrategy/BetterStrategy.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(STA_TARGET): StatisticsTracker/main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(GEN_TARGET) $(CORE_STR_TARGET) $(BETTER_STR_TARGET) $(STA_TARGET)
	rm -f passengers.txt events.txt stats.txt state.txt id_counter.txt tmp_event.json log.json