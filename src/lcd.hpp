#ifndef LCD_HPP
#define LCD_HPP

#include <iostream>
#include "defs.hpp"

class Memory;

class LCD {
	private:
	public:
		byte screen[0x90][0xA0];
		byte screenSourceData[0x90][0xA0]; // 0 - 3 for bg/win colors, 4 - 43 are spirtes
		byte VRAM[0x2000];
		byte OAM[0x100];

		byte vramTiles[24 * 8][16 * 8];
		byte backgorundTiles[32 * 8][32 * 8];

		// Registers
		byte LCDCreg; // Mapped to 0xFF40
		byte STATreg; // Mapped to 0xFF41
		byte SCYreg; // Mapped to 0xFF42
		byte SCXreg; // Mapped to 0xFF43
		byte LYreg; // Mapped to 0xFF44
		byte LYCreg; // Mapped to 0xFF45
		byte DMAreg; // Mapped to 0xFF46
		byte WYreg; // Mapped to 0xFF4A
		byte WXreg; // Mapped to 0xFF4B
		byte BGPreg; // Mapped to 0xFF47
		byte OBP0reg; // Mapped to 0xFF48
		byte OBP1reg; // Mapped to 0xFF49

		byte columnRendering;
		word clocksSpentInLine;

		word dmaClocksLeft;
		int frameCount;
		bool screenRedrawn;

		Memory* mem;

		LCD();
		~LCD();

		void init();
		void turnOff();
		void run(int);

		void dmaTransfer(byte);

		void displayBGLineTest();
		void dumpVramTiles();
		void dumpBackgorundTiles();

		void renderBackgroundLine();
		void renderWindowLine();
		void renderSpritesLine();
		void findSpritesOnLine(int*, int, int);

		void setStatMode(byte);

		void setByte(word, byte);
		byte getByte(word);
		void writeByte(word, byte);
		byte readByte(word);
};

#endif
