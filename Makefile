CXX = g++
CXXFLAGS = -g -Wall -std=c++11 -pthread

all: lizard oneWay

lizard: lizard.cpp
	$(CXX) $(CXXFLAGS) lizard.cpp -o lizard

oneWay: oneWay.cpp oneWay.hpp
	$(CXX) $(CXXFLAGS) oneWay.cpp -o oneWay

clean: 
	rm -f lizard oneWay