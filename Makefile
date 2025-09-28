all:
	mkdir -p build
	$(CXX) ir.cpp -Wall -Wextra --std=c++20 -o build/testIR.elf
