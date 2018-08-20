#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <iostream>
#include <fstream>
#include "defs.hpp"
#include "lcd.hpp"
#include "joypad.hpp"
#include "timer.hpp"

class MBCBase {
protected:
	byte romBnkNum;	// Number of ROM banks on chip
	byte ramSize;	// Number of RAM banks on chip

	byte **rom;		// ROM banks
	byte **ramExt;	// External RAM banks
public:
	virtual byte getByte(word addr) = 0;
	virtual void setByte(word addr, byte data) = 0;
	virtual byte readByte(word addr) = 0;
	virtual void writeByte(word addr, byte data) = 0;

	MBCBase(const byte *header, std::string filepath);
	~MBCBase();
};

class MBC1 : public MBCBase {
private:
	byte romRamRegister; // Select ROM or RAM banks depending on mode
	bool romRamModeSelect;
	bool disableExtRam;
public:
	MBC1(const byte *header, std::string filepath);
	MBC1(const MBC1&) = delete;
	~MBC1();

	byte getByte(word addr);
	void setByte(word addr, byte data);
	byte readByte(word addr);
	void writeByte(word addr, byte data);
};

class MBC2 : public MBCBase {
private:
	byte romRegister; // Select ROM bank
	bool disableExtRam;
public:
	MBC2(const byte *header, std::string filepath);
	MBC2(const MBC2&) = delete;
	~MBC2();

	byte getByte(word addr);
	void setByte(word addr, byte data);
	byte readByte(word addr);
	void writeByte(word addr, byte data);
};

class MBCROM : public MBCBase {
public:
	MBCROM(const byte *header, std::string filepath);
	MBCROM(const MBCROM&) = delete;
	~MBCROM();

	byte getByte(word addr);
	void setByte(word addr, byte data);
	byte readByte(word addr);
	void writeByte(word addr, byte data);
};

class MBC3 : public MBCBase {
private:
	byte rtcRamRegister; // Select RTC or RAM banks depending on mode
	byte rtcRamModeSelect;
	bool disableExtRamAndTimer;
	byte romBankSelect;
	byte latchDataRegister;
	byte rtcRegisters[5];
public:
	MBC3(const byte *header, std::string filepath);
	MBC3(const MBC3&) = delete;
	~MBC3();

	byte getByte(word addr);
	void setByte(word addr, byte data);
	byte readByte(word addr);
	void writeByte(word addr, byte data);
};

class Memory {
private:
	std::string filepath = "";
	byte cartrigeHeader[0x50];	// Address 0x100 - 0x14F

	// controller class with rom and ram
	MBCBase* mbc = nullptr;
		
	// VRAM, OAM and LCD registers
	LCD* lcd = nullptr;

	Joypad joypad;
	Timer timer;

	byte workRam[0x2000];

	byte IOPorts[0x80];
	byte highRam[0x80];

public:
	// Reads header and instantiates the correct mbc class which reads the complete rom
	Memory(std::string filepath);
	~Memory();

	byte getByte(word addr);
	void setByte(word addr, byte data);
	byte readByte(word addr);
	void writeByte(word addr, byte data);

	word getWord(word addr);
	void setWord(word addr, word data);
	word readWord(word addr);
	void writeWord(word addr, word data);

	void connectLCD(LCD* l);
	bool isDmaInProgress();

	// joypad 
	void updateJoypad(const Uint8*);
	bool isJoypadInterruptRequested();
	void setJoypadInterruptRequested(byte);
	bool isTimerInterruptRequested();
	void setTimerInterruptRequested(byte);
	void runTimer(int);
};

#endif
