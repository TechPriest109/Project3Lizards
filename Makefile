CXX = g++
CXXFLAGS = -g -Wall -std=c++11 -pthread

all: lizard lizardUni

lizard: lizard.cpp
	$(CXX) $(CXXFLAGS) lizard.cpp -o lizard

lizardUni: lizardUni.cpp lizardUni.hpp
	$(CXX) $(CXXFLAGS) lizardUni.cpp -o lizardUni

clean: 
	rm -f lizard lizardUni