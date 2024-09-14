CXX = g++
CXXFLAGS := -Wall -Wextra -std=c++20

SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc, %.o, $(SOURCES))

BIN := pl0

.PHONY: clean debug

all: $(BIN)

debug: CXXFLAGS += -ggdb
debug: all

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cc %.hpp
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<


clean:
	@rm -rf $(OBJECTS) $(BIN)
