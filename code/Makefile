CC=g++

CFLAGS=-std=c++11 -lpthread

TARGET=benchmark/main

EXECUTABLE=run_benchmark

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(TARGET).cpp

clean:
	$(RM) $(EXECUTABLE)

