CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
SRC = src/main.cpp src/utils.cpp src/symbols.cpp src/lexer.cpp src/encoder.cpp src/directives.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = assembler

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) $(OBJ)
