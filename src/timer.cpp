#include "timer.hpp"
#include <iostream>

Timer::Timer() : divReg(0xABCC), timaReg(0), tmaReg(0), tacReg(0), interruptRequested(false) {}

Timer::~Timer() {}

void Timer::run(int clocks) { 
	byte shiftValue;
	switch(tacReg & 0x03) {
		case 0: shiftValue = 9; break;
		case 1: shiftValue = 3; break;
		case 2: shiftValue = 5; break;
		case 3:	shiftValue = 7;
	}

	for(int i = 0; i < clocks; i++) {
		byte oldRelaventDivBit = (divReg & shiftValue) >> shiftValue;
		divReg++;
		byte newRelaventDivBit = (divReg & shiftValue) >> shiftValue;
		byte oldTima = timaReg;

		// TIMA is increased
		if(oldRelaventDivBit == 1 && newRelaventDivBit == 0 && (tacReg & 0x4) != 0) {	
			timaReg++;
			if(timaReg < oldTima) {	// Interrupt triggered
				interruptRequested = true;
				timaReg = tmaReg;
			}
		}
	}
}

void Timer::setByte(word addr, byte data) {
	switch(addr) {
		case 0xFF04:
			divReg = data << 8;
			break;
		case 0xFF05:
			timaReg = data & 0xFF;
			break;
		case 0xFF06:
			tmaReg = data & 0xFF;
			break;
		case 0xFF07:
			tacReg = data & 0xFF;
			break;
		default:
			std::cerr << "Unknown memory acces in Timer: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	}
}

byte Timer::getByte(word addr) {
	switch(addr) {
		case 0xFF04:
			return divReg >> 8;
		case 0xFF05:
			return timaReg;
		case 0xFF06:
			return tmaReg;
		case 0xFF07:
			return tacReg;
		default:
			std::cerr << "Unknown memory acces in Timer: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
			return -1;
	}
}

void Timer::writeByte(word addr, byte data) {
	switch(addr) {
		case 0xFF04:
			divReg = 0;
			break;
		case 0xFF05:
			timaReg = data;
			break;
		case 0xFF06:
			tmaReg = data;
			break;
		case 0xFF07:
			tacReg = data & 0x07;
			break;
		default:
			std::cerr << "Unknown memory acces in Timer: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	}
}

byte Timer::readByte(word addr) {
	switch(addr) {
		case 0xFF04:
			return divReg >> 8;
		case 0xFF05:
			return timaReg;
		case 0xFF06:
			return tmaReg;
		case 0xFF07:
			return tacReg;
		default:
			std::cerr << "Unknown memory acces in Timer: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
			return -1;
	}
}

bool Timer::isInterruptRequested() { return interruptRequested; }
void Timer::setInterruptRequested(bool data) { interruptRequested = data; }
