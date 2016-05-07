CC=g++

CFLAGS=-std=c++11 -Werror -Wall -Wextra -lpthread -lboost_system -lboost_thread

TARGET=benchmark/main

EXECUTABLE=run_benchmark

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(TARGET).cpp

clean:
	$(RM) $(EXECUTABLE)

