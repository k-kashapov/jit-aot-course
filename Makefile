all:
	mkdir -p build
	$(CXX) fibonacci.cpp -Wall -Wextra --std=c++20 -o build/testIR.elf
