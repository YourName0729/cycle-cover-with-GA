all:
	g++ -std=c++17 -O3 -g -Wall -fmessage-length=0 -o cycle_cover cycle_cover.cpp
clean:
	rm -r cycle_cover