#include "joypad.hpp"

Joypad::Joypad() {
	keystates = 0xFF;
	p1reg = 0xFF;
	interruptRequested = false;
}

Joypad::~Joypad() {}

void Joypad::updateKeystates(const Uint8 *keys) {
	byte oldKeystates = keystates;
	keystates = 0xFF;
	if(keys[SDL_SCANCODE_RETURN])		keystates &= 0x7F;
	if(keys[SDL_SCANCODE_BACKSPACE])	keystates &= 0xBF;
	if(keys[SDL_SCANCODE_X])			keystates &= 0xDF;
	if(keys[SDL_SCANCODE_Z])			keystates &= 0xEF;
	if(keys[SDL_SCANCODE_DOWN])			keystates &= 0xF7;
	if(keys[SDL_SCANCODE_UP])			keystates &= 0xFB;
	if(keys[SDL_SCANCODE_LEFT])			keystates &= 0xFD;
	if(keys[SDL_SCANCODE_RIGHT])		keystates &= 0xFE;

	// Check for interrupts
	if((p1reg & 0x20) == 0) {
		for(int i = 0; i < 4; i++)
			if((oldKeystates & 0x10 << i) == (keystates & 0x10 << i)) interruptRequested = true;
	}
	if((p1reg & 0x10) == 0) {
		for(int i = 0; i < 4; i++)
			if((oldKeystates & 0x01 << i) == (keystates & 0x01 << i)) interruptRequested = true;
	}
}

void Joypad::setP1reg(byte data) { p1reg = data; }
byte Joypad::getP1reg() { return p1reg; }
void Joypad::writeP1reg(byte data) { p1reg = p1reg & 0xCF | data & 0x30; }
byte Joypad::readP1reg() { 
	byte pressedkeys = 0xFF;
	if((p1reg & 0x20) == 0) pressedkeys &= keystates >> 4;
	if((p1reg & 0x10) == 0) pressedkeys &= keystates;
	return p1reg & 0xF0 | pressedkeys & 0x0F; 
}

bool Joypad::isInterruptRequested() { return interruptRequested; }
void Joypad::setInterruptRequested(bool data) { interruptRequested = data; }
