#ifndef BOARD_HPP
#define BOARD_HPP

#include "cpu.hpp"
#include "memory.hpp"
#include "lcd.hpp"

#include <iostream>
#include <sstream>
#include <string>

class Board {
private:
public:
	LCD lcd;
	Memory* memory = nullptr;
	CPU cpu;

	Board(std::string filepath);
	Board(const Board&) = delete;
	~Board();

	void run();
	void step();
};

#endif
