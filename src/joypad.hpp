#ifndef JOYPAD_HPP
#define JOYPAD_HPP

#include "defs.hpp"
#include "SDL.h"

class Joypad {
private:
	byte keystates;	// From MSB to LSB: start, select, B, A, down, up, left, right; 0 means pressesed
	byte p1reg;

	bool interruptRequested;
public:
	Joypad();
	~Joypad();

	void updateKeystates(const Uint8*);
	
	void setP1reg(byte);
	byte getP1reg();

	void writeP1reg(byte);
	byte readP1reg();

	bool isInterruptRequested();
	void setInterruptRequested(bool);
};

#endif
