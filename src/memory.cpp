#include "memory.hpp"

MBCBase::MBCBase(const byte *header, std::string filepath) {
	romBnkNum = 2 << header[0x48];
	if(header[0x48] == 0x52) romBnkNum = 72;
	else if(header[0x48] == 0x53) romBnkNum = 80;
	else if(header[0x48] == 0x54) romBnkNum = 96;
	ramSize = header[0x49];

	std::ifstream romFile(filepath, std::ios::binary);

	// Check file length
	romFile.seekg(0, std::ios_base::end);
	std::streamoff len = romFile.tellg();
	if(len < 0x4000 * romBnkNum) throw std::length_error("File to small");

	// Read the ROM into banks
	romFile.seekg(0, std::ios_base::beg);
	rom = new byte*[romBnkNum];
	for(int i = 0; i < romBnkNum; i++) {
		rom[i] = new byte[0x4000];
		romFile.read((char*) rom[i], 0x4000);
	}
	romFile.close();
}

MBCBase::~MBCBase() {
	if(rom != nullptr) {
		for(int i = 0; i < romBnkNum; i++)
			delete[] rom[i];
		delete[] rom;
	}
}

// --------------------------------- MBC1 member functions ---------------------------------------

MBC1::MBC1(const byte *header, std::string filepath) : MBCBase(header, filepath) {
	// Allocate external RAM
	if(ramSize != 0) {
		ramExt = new byte*[ramSize == 3 ? 4 : 1];
		for(int i = 0; i < (ramSize == 3 ? 4 : 1); i++)
			ramExt[i] = new byte[ramSize == 1 ? 0x800 : 0x2000];
	}

	romRamModeSelect = false;
	disableExtRam = true;
	romRamRegister = 0x01;
}

MBC1::~MBC1() {
	if(ramExt != nullptr) {
		for(int i = 0; i < (ramSize == 3 ? 4 : 1); i++)
			delete[] ramExt[i];
		delete[] ramExt;
	}
}

byte MBC1::getByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x3FFF) {			// ROM fixed bank
		return rom[0][addr];
	} else if(addr >= 0x4000 && addr <= 0x7FFF) {	// ROM switchable bank
		return rom[romRamModeSelect ? romRamRegister & 0x1F : romRamRegister & 0x7F][addr - 0x4000];
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		return ramExt[romRamModeSelect ? (romRamRegister & 0x60) >> 5 : 0x00][addr - 0xA000];
	}
	std::cerr << "Unknown memory acces in cartrige controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	return 0xFF;
}

void MBC1::setByte(word addr, byte data) {
	if(addr >= 0x0000 && addr <= 0x1FFF) {			// External RAM enable
		disableExtRam = !((data & 0x0A) == 0x0A);
	} else if(addr >= 0x2000 && addr <= 0x3FFF) {	// ROM bank number lower 5 bits
		if(data == 0x00) data = 0x01;	// Can't select 0th bank		
		romRamRegister &= 0xE0;			// Turn of lower 5 bits
		romRamRegister |= data & 0x1F;	// Write to lower 5 bits
	} else if(addr >= 0x4000 && addr <= 0x5FFF) {	// RAM bank or ROM bank number upper 2 bits
		romRamRegister &= 0x9F;					// Turn of bit 5 and 6 
		romRamRegister |= (data & 0x03) << 5;	// Write to bit 5 and 6
	} else if(addr >= 0x6000 && addr <= 0x7FFF) {	// ROM/RAM mode select
		romRamModeSelect = (data & 0x01) != 0;
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		ramExt[romRamModeSelect ? (romRamRegister & 0x60) >> 5 : 0x00][addr - 0xA000] = data;
	}
}

byte MBC1::readByte(word addr) { return getByte(addr); }
void MBC1::writeByte(word addr, byte data) { setByte(addr, data); }

// --------------------------------- MBC2 member functions ---------------------------------------

MBC2::MBC2(const byte *header, std::string filepath) : MBCBase(header, filepath) {
	// Allocate external RAM
	ramExt = new byte*[1];
	ramExt[0] = new byte[512];

	disableExtRam = true;
	romRegister = 0x01;
}

MBC2::~MBC2() {
	if(ramExt != nullptr) {
		delete[] ramExt[0];
		delete[] ramExt;
	}
}

byte MBC2::getByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x3FFF) {			// ROM fixed bank
		return rom[0][addr];
	} else if(addr >= 0x4000 && addr <= 0x7FFF) {	// ROM switchable bank
		return rom[romRegister & 0x0F][addr - 0x4000];
	} else if(addr >= 0xA000 && addr <= 0xA1FF) {	// External RAM
		return ramExt[0][addr - 0xA000];
	}
	std::cerr << "Unknown memory acces in cartrige controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	return 0xFF;
}

void MBC2::setByte(word addr, byte data) {
	if(addr >= 0x0000 && addr <= 0x1FFF && ((addr >> 4) & 1) == 0) {	// External RAM enable
		disableExtRam = !((data & 0x0A) == 0x0A);
	} else if(addr >= 0x2000 && addr <= 0x3FFF && ((addr >> 4) & 1) == 1) {	// ROM bank number 
		if(data == 0x00) data = 0x01;	// Can't select 0th bank		
		romRegister = data & 0x0F;
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		ramExt[0][addr - 0xA000] = data;
	}
}

byte MBC2::readByte(word addr) { return getByte(addr); }
void MBC2::writeByte(word addr, byte data) { setByte(addr, data); }

// --------------------------------- MBCROM member functions ---------------------------------------

MBCROM::MBCROM(const byte *header, std::string filepath) : MBCBase(header, filepath) {
	// Allocate external RAM
	ramExt = new byte*[1];
	ramExt[0] = new byte[0x2000];
}

MBCROM::~MBCROM() {
	if(ramExt != nullptr) {
		delete[] ramExt[0];
		delete[] ramExt;
	}
}

byte MBCROM::getByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x3FFF) {			// ROM fixed bank
		return rom[0][addr];
	} else if(addr >= 0x4000 && addr <= 0x7FFF) {	// ROM "second" bank
		return rom[1][addr - 0x4000];
	} else if(addr >= 0xA000 && addr <= 0xA1FF) {	// External RAM
		return ramExt[0][addr - 0xA000];
	}
	std::cerr << "Unknown memory acces in cartrige controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	return 0xFF;
}

void MBCROM::setByte(word addr, byte data) {
	if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		ramExt[0][addr - 0xA000] = data;
	}
}

byte MBCROM::readByte(word addr) { return getByte(addr); }
void MBCROM::writeByte(word addr, byte data) { setByte(addr, data); }

// --------------------------------- MBC3 member functions ---------------------------------------

MBC3::MBC3(const byte *header, std::string filepath) : MBCBase(header, filepath) {
	// Allocate external RAM
	if(ramSize != 0) {
		ramExt = new byte*[ramSize == 3 ? 4 : 1];
		for(int i = 0; i < (ramSize == 3 ? 4 : 1); i++)
			ramExt[i] = new byte[ramSize == 1 ? 0x800 : 0x2000];
	}

	rtcRamRegister = 0; 
	rtcRamModeSelect = 0;
	disableExtRamAndTimer = true;
	romBankSelect = 1;
	latchDataRegister = 1;
	for(int i = 0; i < 5; i++) rtcRegisters[i] = 0;
}

MBC3::~MBC3() {
	if(ramExt != nullptr) {
		for(int i = 0; i < (ramSize == 3 ? 4 : 1); i++)
			delete[] ramExt[i];
		delete[] ramExt;
	}
}

byte MBC3::getByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x3FFF) {			// ROM fixed bank
		return rom[0][addr];
	} else if(addr >= 0x4000 && addr <= 0x7FFF) {	// ROM switchable bank
		return rom[romBankSelect & 0x7F][addr - 0x4000];
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		if(rtcRamModeSelect >= 0 && rtcRamModeSelect <= 3) {
			return ramExt[rtcRamModeSelect][addr - 0xA000];
		} else if(rtcRamModeSelect >= 0x8 && rtcRamModeSelect <= 0xC) {
			return rtcRegisters[rtcRamModeSelect - 0x8];
		}
	}
	std::cerr << "Unknown memory acces in cartrige controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	return 0xFF;
}

void MBC3::setByte(word addr, byte data) {
	if(addr >= 0x0000 && addr <= 0x1FFF) {			// External RAM enable
		disableExtRamAndTimer = !((data & 0x0A) == 0x0A);
	} else if(addr >= 0x2000 && addr <= 0x3FFF) {	// ROM bank number 
		if(data == 0x00 || data > romBnkNum) data = 0x01;	// Can't select 0th bank	
		romBankSelect = data & 0x7F;
	} else if(addr >= 0x4000 && addr <= 0x5FFF) {	// RTC and RAM register select
		rtcRamModeSelect = data & 0x0F;
	} else if(addr >= 0x6000 && addr <= 0x7FFF) {	// ROM/RAM mode select
		if(latchDataRegister == 0 && data == 1) int a = 1; // TODO latch data
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		if(rtcRamModeSelect >= 0 && rtcRamModeSelect <= 3) {
			ramExt[rtcRamModeSelect][addr - 0xA000] = data;
		} else if(rtcRamModeSelect >= 0x8 && rtcRamModeSelect <= 0xC) {
			rtcRegisters[rtcRamModeSelect - 0x8] = data;
		}
	}
}

byte MBC3::readByte(word addr) { return getByte(addr); }
void MBC3::writeByte(word addr, byte data) { setByte(addr, data); }

// --------------------------------- Memory member functions -------------------------------------

Memory::Memory(std::string filepath) { 
	this->filepath = filepath; 
	std::ifstream romFile(filepath, std::ios::binary);

	// Check file length
	romFile.seekg(0, std::ios_base::end);
	std::streamoff len = romFile.tellg();
	if(len < 0x150) throw std::length_error("File to small");

	// Read the header
	romFile.seekg(0x100, std::ios_base::beg);
	romFile.read((char*) cartrigeHeader, 0x50);
	romFile.close();

	// Validate nintendo logo
	for(int i = 0; i < 0x30; i++) {
		if(cartrigeHeader[i + 4] != nintendoLogo[i]) {
			std::cerr << "Not a Game Boy ROM file" << std::endl;
			throw std::invalid_argument("Not a Game Boy ROM file");
		}
	}

	// Construct the right MBC
	if(cartrigeHeader[0x47] >= 0x01 && cartrigeHeader[0x47] <= 0x03)
		mbc = new MBC1(cartrigeHeader, filepath);
	else if (cartrigeHeader[0x47] == 0x00 || cartrigeHeader[0x47] == 0x05 || cartrigeHeader[0x47] == 0x06)
		mbc = new MBC2(cartrigeHeader, filepath);
	else if(cartrigeHeader[0x47] >= 0x08 && cartrigeHeader[0x47] <= 0x09)
		mbc = new MBCROM(cartrigeHeader, filepath);	
	else if(cartrigeHeader[0x47] >= 0x0F && cartrigeHeader[0x47] <= 0x13)
		mbc = new MBC3(cartrigeHeader, filepath);
	else 
		throw std::invalid_argument("Not a supported MBC chip");
}

Memory::~Memory() {
	if(mbc != nullptr) { 
		delete mbc; 
		mbc = nullptr;
	}
}

byte Memory::getByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x7FFF) {			// Cartrige
		return mbc->getByte(addr);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {	// VRAM
		return lcd->getByte(addr);
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		return mbc->getByte(addr);
	} else if(addr >= 0xC000 && addr <= 0xDFFF) {	// WRAM
		return workRam[addr - 0xC000];
	} else if(addr >= 0xE000 && addr <= 0xFDFF) {	// Echo WRAM
		return workRam[addr - 0xE000];
	} else if(addr >= 0xFE00 && addr < 0xFEA0) {	// OAM
		return lcd->getByte(addr);
	} else if(addr >= 0xFF00 && addr < 0xFF80) {	// IO ports
		if(addr == 0xFF00)								// Joypad
			return joypad.getP1reg();
		if(addr >= 0xFF04 && addr <= 0xFF07)			// Timer
			return timer.getByte(addr);
		if(addr >= 0xFF40 && addr <= 0xFF4B)			// LCD registers
			return lcd->getByte(addr);
		return IOPorts[addr - 0xFF00];
	} else if(addr >= 0xFF80 && addr < 0x10000) {	// High RAM
		return highRam[addr - 0xFF80];
	}
	return 0xFF;
}

void Memory::setByte(word addr, byte data) {
	if(addr >= 0x0000 && addr <= 0x7FFF) {			// Cartrige
		mbc->setByte(addr, data);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {	// VRAM
		lcd->setByte(addr, data);
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		mbc->setByte(addr, data);
	} else if(addr >= 0xC000 && addr <= 0xDFFF) {	// WRAM
		workRam[addr - 0xC000] = data;
	} else if(addr >= 0xE000 && addr <= 0xFDFF) {	// Echo WRAM
		workRam[addr - 0xE000] = data;
	} else if(addr >= 0xFE00 && addr < 0xFEA0) {	// OAM
		lcd->setByte(addr, data);
	} else if(addr >= 0xFF00 && addr < 0xFF80) {	// IO ports
		if(addr == 0xFF00)								// Joypad
			joypad.setP1reg(data);
		if(addr >= 0xFF04 && addr <= 0xFF07)			// Timer
			timer.setByte(addr, data);
		if(addr >= 0xFF40 && addr <= 0xFF4B)			// LCD registers
			lcd->setByte(addr, data);
		IOPorts[addr - 0xFF00] = data;
	} else if(addr >= 0xFF80 && addr < 0x10000) {	// High RAM
		highRam[addr - 0xFF80] = data;
	}
}


byte Memory::readByte(word addr) {
	if(addr >= 0x0000 && addr <= 0x7FFF) {			// Cartrige
		return mbc->readByte(addr);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {	// VRAM
		return lcd->readByte(addr);
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		return mbc->readByte(addr);
	} else if(addr >= 0xC000 && addr <= 0xDFFF) {	// WRAM
		return isDmaInProgress() ? 0xFF : workRam[addr - 0xC000];
	} else if(addr >= 0xE000 && addr <= 0xFDFF) {	// Echo WRAM
		return isDmaInProgress() ? 0xFF : workRam[addr - 0xE000];
	} else if(addr >= 0xFE00 && addr < 0xFEA0) {	// OAM
		return lcd->getByte(addr);
	} else if(addr >= 0xFF00 && addr < 0xFF80) {	// IO ports
		if(addr == 0xFF00)								// Joypad
			return joypad.readP1reg();
		if(addr >= 0xFF04 && addr <= 0xFF07)			// Timer
			return timer.readByte(addr);
		if(addr >= 0xFF40 && addr <= 0xFF4B)			// LCD registers
			return lcd->readByte(addr);
		return IOPorts[addr - 0xFF00];
	} else if(addr >= 0xFF80 && addr < 0x10000) {	// High RAM
		return highRam[addr - 0xFF80];
	}
	return 0xFF;
}

void Memory::writeByte(word addr, byte data) {
	if(addr >= 0x0000 && addr <= 0x7FFF) {			// Cartrige
		mbc->writeByte(addr, data);
	} else if(addr >= 0x8000 && addr <= 0x9FFF) {	// VRAM
		lcd->writeByte(addr, data);
	} else if(addr >= 0xA000 && addr <= 0xBFFF) {	// External RAM
		mbc->writeByte(addr, data);
	} else if(addr >= 0xC000 && addr <= 0xDFFF) {	// WRAM
		if(!isDmaInProgress()) workRam[addr - 0xC000] = data;
	} else if(addr >= 0xE000 && addr <= 0xFDFF) {	// Echo WRAM
		if(!isDmaInProgress()) workRam[addr - 0xE000] = data;
	} else if(addr >= 0xFE00 && addr < 0xFEA0) {	// OAM
		lcd->writeByte(addr, data);
	} else if(addr >= 0xFF00 && addr < 0xFF80) {	// IO ports
		if(addr == 0xFF00)								// Joypad
			joypad.writeP1reg(data);
		if(addr >= 0xFF04 && addr <= 0xFF07)			// Timer
			timer.writeByte(addr, data);
		if(addr >= 0xFF40 && addr <= 0xFF4B)			// LCD registers
			lcd->writeByte(addr, data);
		IOPorts[addr - 0xFF00] = data;
	} else if(addr >= 0xFF80 && addr < 0x10000) {	// High RAM
		highRam[addr - 0xFF80] = data;
	}
}

word Memory::getWord(word addr) {
	word data = getByte(addr++);
	data |= getByte(addr) << 8;
	return data;
}

void Memory::setWord(word addr, word data) {
	setByte(addr++, data & 0xFF);
	setByte(addr, data >> 8);
}

word Memory::readWord(word addr) {
	word data = readByte(addr++);
	data |= readByte(addr) << 8;
	return data;;
}

void Memory::writeWord(word addr, word data) {
	writeByte(addr++, data & 0xFF);
	writeByte(addr, data >> 8);
}

void Memory::connectLCD(LCD * l) {
	lcd = l;
	l->mem = this;
}

bool Memory::isDmaInProgress() { return lcd->dmaClocksLeft > 0; }

void Memory::updateJoypad(const Uint8* keystates) { joypad.updateKeystates(keystates); }
bool Memory::isJoypadInterruptRequested() { return joypad.isInterruptRequested(); }
void Memory::setJoypadInterruptRequested(byte data) { joypad.setInterruptRequested(data); }
bool Memory::isTimerInterruptRequested() { return timer.isInterruptRequested();  }
void Memory::setTimerInterruptRequested(byte data) { timer.setInterruptRequested(data); }
void Memory::runTimer(int clocks) { timer.run(clocks); }
