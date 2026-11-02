CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Iinclude

SRCS = src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rosec

TEST_SRCS = src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp tests/test_lexer.cpp tests/test_parser.cpp tests/test_main.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = rosec_test
TEST_CXXFLAGS = $(CXXFLAGS) -Itests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(TEST_TARGET)

tests/%.o: tests/%.cpp
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TARGET).exe $(TEST_TARGET) $(TEST_TARGET).exe

.PHONY: all test clean
