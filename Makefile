CXX = g++
CXXFLAGS = -std=c++11 -Wall -I.

GEN_TARGET = PassengerGenerator/generator
STR_TARGET = CoreStrategy/strategy
STA_TARGET = StatisticsTracker/stats

GEN_SRCS = PassengerGenerator/main.cpp PassengerGenerator/PassengerGenerator.cpp
STR_SRCS = CoreStrategy/main.cpp CoreStrategy/CoreStrategy.cpp
STA_SRCS = StatisticsTracker/main.cpp StatisticsTracker/StatisticsTracker.cpp

GEN_OBJS = $(GEN_SRCS:.cpp=.o)
STR_OBJS = $(STR_SRCS:.cpp=.o)
STA_OBJS = $(STA_SRCS:.cpp=.o)

all: init $(GEN_TARGET) $(STR_TARGET) $(STA_TARGET)

init:
	@touch config.txt passengers.txt events.txt stats.txt
	@if [ ! -s config.txt ]; then \
		echo "10 0.5" > config.txt; \
		echo "0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1" >> config.txt; \
		echo "0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1" >> config.txt; \
	fi

$(GEN_TARGET): $(GEN_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(GEN_OBJS)

$(STR_TARGET): $(STR_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STR_OBJS)

$(STA_TARGET): $(STA_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(STA_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(GEN_TARGET) $(STR_TARGET) $(STA_TARGET) $(GEN_OBJS) $(STR_OBJS) $(STA_OBJS)
	rm -f passengers.txt events.txt stats.txt