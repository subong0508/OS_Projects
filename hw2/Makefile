CC = g++
CXXFLAGS = -std=c++11

all: miniShell program1 program2 program3
miniShell: miniShell.h miniShell.cpp
			$(CC) $(CXXFLAGS) miniShell.cpp -o miniShell
program1: programs.h program1.cpp
			$(CC) $(CXXFLAGS) program1.cpp -o program1
program2: programs.h program2.cpp
			$(CC) $(CXXFLAGS) program2.cpp -o program2
program3: programs.h program3.cpp
			$(CC) $(CXXFLAGS) program3.cpp -o program3 -lpthread
clean:
	rm -rf miniShell
	rm -rf program1
	rm -rf program2
	rm -rf program3

