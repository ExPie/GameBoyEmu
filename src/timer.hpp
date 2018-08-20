#ifndef TIMER_HPP
#define TIMER_HPP

#include "defs.hpp"

class Timer {
private:
	word divReg;	// Mapped to 0xFF04;		upper part of is the register content
	byte timaReg;	// Mapped to 0xFF05
	byte tmaReg;	// Mapped to 0xFF06
	byte tacReg;	// Mapped to 0xFF07

	bool interruptRequested;
public:
	Timer();
	~Timer();

	void run(int);

	void setByte(word, byte);	
	byte getByte(word);
	void writeByte(word, byte);
	byte readByte(word);

	bool isInterruptRequested();
	void setInterruptRequested(bool);
};

#endif
