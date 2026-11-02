CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Iinclude

SRCS = src/Token.cpp src/Lexer.cpp src/AST.cpp src/Parser.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rosec

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all clean
