
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude
FUENTES = $(wildcard src/*.cpp)

ifeq ($(OS),Windows_NT)
    PROGRAMA = corrector.exe
else
    PROGRAMA = corrector
endif

all:
	$(CXX) $(CXXFLAGS) $(FUENTES) -o $(PROGRAMA)

.PHONY: all
