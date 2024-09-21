CXX = g++

LLVM_LIB_FLAGS := $(shell llvm-config --ldflags --system-libs --libs core)
LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)
CXXFLAGS := -Wall -Wextra $(LLVM_CXXFLAGS) -std=c++20 -Wno-unused-parameter


SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc, %.o, $(SOURCES))

BIN := pl0

.PHONY: clean debug

all: $(BIN)

debug: CXXFLAGS += -g
debug: all

$(BIN): $(OBJECTS)
	$(CXX) $^ $(LLVM_LIB_FLAGS) -o $@

%.o: %.cc %.hpp
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<


clean:
	@rm -rf $(OBJECTS) $(BIN)
