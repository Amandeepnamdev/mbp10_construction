CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

TARGET = reconstruction_blockhouse
SRC = mbp10_construction.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o outtest.csv
