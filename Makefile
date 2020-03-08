all:
	g++ -o fbdev -fopenmp -O2 -std=gnu++11 -Wall main.cpp viewer.cpp window.cpp -lncurses
