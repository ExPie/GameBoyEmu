#include "board.hpp"

Board::Board(std::string filepath) {
	try {
		memory = new Memory(filepath);
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	if(memory != nullptr) {
		memory->connectLCD(&lcd);
		cpu.connectMemory(memory);
		cpu.init();
	}
}

Board::~Board() {
	if(memory != nullptr) delete memory;
}

void Board::run() {
	bool flag = false;
	unsigned int addressToStop = 0x28A0000;
	long long totalClocks = 0;
	for(long long count = 0;; count++) {
		cpu.handleInterrupts();
		byte opcode = memory->readByte(cpu.regs.pc);
		if(flag || cpu.regs.pc == addressToStop) { // 0x028A confirmed // LCDC BG gets turned off after this
			flag = true;
			std::cout << std::hex << std::uppercase << "Executing opcode: 0x" << +opcode << " at address 0x" << cpu.regs.pc << std::nouppercase << std::dec << std::endl;
			byte b = memory->readByte(0xFF44);
			std::cout << std::hex << std::uppercase << "REG AF: 0x" << (int)cpu.regs.af << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "REG BC: 0x" << (int)cpu.regs.bc << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "REG DE: 0x" << (int)cpu.regs.de << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "REG HL: 0x" << (int)cpu.regs.hl << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "REG SP: 0x" << (int)cpu.regs.sp << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "TOP STACK WORD: 0x" << memory->readWord(cpu.regs.sp) << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "LCDC: 0x" << (int)lcd.LCDCreg << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "STAT: 0x" << (int)lcd.STATreg << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "LY: 0x" << (int)lcd.LYreg << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "IE: 0x" << (int) memory->readByte(IE) << std::nouppercase << std::dec << std::endl;
			std::cout << std::hex << std::uppercase << "IF: 0x" << (int) memory->readByte(0xFF0F) << std::nouppercase << std::dec << std::endl;

			
			std::string line;
			std::getline(std::cin, line);
			if(!line.empty()) {
				std::stringstream streamedLine(line);
				streamedLine >> std::hex >> addressToStop;
				flag = false;
			}
			std::cout << std::hex << std::uppercase << "Stopping address: 0x" << addressToStop << std::nouppercase << std::dec << std::endl << std::endl;

		}
		totalClocks += cpu.clocks;
		cpu.clocks = 0;
		cpu.exec(opcode);
		lcd.run(cpu.clocks);
	}
}

void Board::step() {
	cpu.handleInterrupts();
	byte opcode = memory->readByte(cpu.regs.pc);
	cpu.clocks = 0;
	cpu.exec(opcode);
	lcd.run(cpu.clocks);
	memory->runTimer(cpu.clocks);
	
	// Check for timer interupt
	if(memory->isTimerInterruptRequested()) {
		memory->setByte(0xFF0F, memory->getByte(0xFF0F) | 1 << 2);
		memory->setTimerInterruptRequested(false);
	}

	// Check for joypad interupt
	if(memory->isJoypadInterruptRequested()) {
		memory->setByte(0xFF0F, memory->getByte(0xFF0F) | 1 << 4);
		memory->setJoypadInterruptRequested(false);
	}
}
