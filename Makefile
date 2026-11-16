CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -fno-exceptions
TARGET = acaderp
SRCS = src/main.cpp src/erp.cpp

all: $(TARGET)

$(TARGET): $(SRCS) src/erp.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
