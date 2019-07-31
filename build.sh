#!/bin/bash
g++ -std=c++17 -I./src/lib -O3 -o matrix_calc  ./src/matrix_calc.cpp
g++ -std=c++17 -I./src/lib -I./src/test -O3 -o test  ./src/test/test.cpp
