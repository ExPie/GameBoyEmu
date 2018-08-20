#include "lcd.hpp"
#include "memory.hpp"

LCD::LCD() {
	init();
}

LCD::~LCD() {

}

void LCD::init() {
	LYreg = 0;
	columnRendering = 0;
	clocksSpentInLine = 0;
	setStatMode(2);
	dmaClocksLeft = 0;
	frameCount = 0;
	screenRedrawn = true;
}

void LCD::turnOff() {
	setStatMode(1);
}

void LCD::run(int clocks) {
	for(int i = 0; i < clocks; i += 4) {
		if(dmaClocksLeft > 0) dmaTransfer(DMAreg);

		if((LCDCreg & 0x80) != 0) {
			if(clocksSpentInLine == 4 && LYreg != 0) {
				if(LYreg == LYCreg) STATreg |= 0x04;
			} else STATreg &= 0xFB;
			if(clocksSpentInLine == 80 && (STATreg & 0x03) != 1) {
				setStatMode(3); 
			}
			if(clocksSpentInLine == 172 && (STATreg & 0x03) != 1) {
				setStatMode(0);
				renderBackgroundLine();
				renderWindowLine();
				renderSpritesLine();
			}
			if(clocksSpentInLine == 456) {
				if((STATreg & 0x03) != 1 || LYreg == 143)setStatMode(2);
				clocksSpentInLine = 0;
				LYreg++;
			}
			if(LYreg == 144 && clocksSpentInLine == 4) {
				setStatMode(1);
				mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) | 0x01); // Request VBLANK interupt
			}
			if(LYreg == 154) {
				setStatMode(2);
				clocksSpentInLine = 0;
				LYreg = 0;
				frameCount++;
				screenRedrawn = true;
			}
			
			clocksSpentInLine += 4;
		}
	}
}

void LCD::dmaTransfer(byte addr) {
	word startAddr = 0x0000 | (addr << 8);
	if(dmaClocksLeft > 160 * 4) { dmaClocksLeft -= 4; return; } // Write first byte on second tick
	byte byteIndex = 160 - (dmaClocksLeft / 4);
	mem -> setByte(0xFE00 + byteIndex, mem -> getByte(startAddr + byteIndex));
	dmaClocksLeft -= 4;
}

void LCD::displayBGLineTest() {

	byte bgY = SCYreg;
	byte bgX = SCXreg;
	byte wnY = WYreg;
	byte wnX = WXreg - 7;

	byte scY = LYreg;
	
	word wnTileMapSelect = (LCDCreg & 0x40) == 0 ? 0x9800 : 0x9C00;
	bool wnEnable = (LCDCreg & 0x20) != 0;
	word bgwnTileDataSelect = (LCDCreg & 0x10) == 0 ? 0x8800 : 0x8000;
	word bgTileMapSelect = (LCDCreg & 0x08) == 0 ? 0x9800 : 0x9C00;
	bool bgEnable = (LCDCreg & 0x01) != 0;

	for(int i = 0; i < 0xA0; i++) {
		byte scX = i;

		byte mapX = (bgX + scX) % 256;
		byte mapY = (bgY + scY) % 256;
		byte tileNumX = mapX / 8;
		byte tileNumY = mapY / 8;
		byte pxInTileX = mapX % 8;
		byte pxInTileY = mapY % 8;

		word tilePositionInMapMem = tileNumY * 32 + tileNumX;
		word tileAddrInMapMem = bgTileMapSelect + tilePositionInMapMem;

		byte unsignTileNum = mem -> readByte(tileAddrInMapMem);
		sbyte signTileNum = (sbyte)mem -> readByte(tileAddrInMapMem);

		word tileAddr;
		if((LCDCreg & 0x10) == 0) {
			tileAddr = bgwnTileDataSelect + 0x0800 + (signTileNum * 16);
		} else {
			tileAddr = bgwnTileDataSelect + (unsignTileNum * 16);
		}

		byte upperTileByte = readByte(tileAddr + 2 * pxInTileY + 0);
		byte lowerTileByte = readByte(tileAddr + 2 * pxInTileY + 1);


		byte upperColorBit = (upperTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte lowerColorBit = (lowerTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte colorNum = upperColorBit << 1 | lowerColorBit;

		byte color = (BGPreg & 3 << colorNum * 2) >> colorNum * 2;

		if(bgEnable) 
			screen[scY][scX] = color;
		else
			screen[scX][scY] = 0x00;
	}
}

void LCD::dumpVramTiles() {
	for(int i = 0x00; i < 24; i++) { 
		for(int j = 0x00; j < 16; j++) { 
			word tileAddr = 0x8000 + 16 * (i * 16 + j);

			for(int k = 0; k < 8; k++) {
				byte upperBitsRow = getByte(tileAddr + 2 * k);
				byte lowerBitsRow = getByte(tileAddr + 2 * k + 1);
				
				for(int l = 0; l < 8; l++) {
					byte upperColorBit = (upperBitsRow & (1 << (7 - l))) >> (7 - l);
					byte lowerColorBit = (lowerBitsRow & (1 << (7 - l))) >> (7 - l);
					byte colorNum = upperColorBit << 1 | lowerColorBit;

					vramTiles[i * 8 + k][j * 8 + l] = 3 - colorNum;
				}
			}
		}
	}
}

void LCD::dumpBackgorundTiles() {
	word tileMapAddr = (LCDCreg & 0x08) == 0 ? 0x9800 : 0x9C00;
	word tileDataAddr = (LCDCreg & 0x10) == 0 ? 0x9000 : 0x8000;
	for(int i = 0; i < 32; i++) {
		for(int j = 0; j < 32; j++) {
			word tileAddrOffset = getByte(tileMapAddr + i * 32 + j);
			word tileAddr = tileDataAddr + (tileDataAddr == 0x8000 ? tileAddrOffset : (sbyte) tileAddrOffset) * 16;

			for(int k = 0; k < 8; k++) {
				byte upperBitsRow = getByte(tileAddr + 2 * k);
				byte lowerBitsRow = getByte(tileAddr + 2 * k + 1);

				for(int l = 0; l < 8; l++) {
					byte upperColorBit = (upperBitsRow & (1 << (7 - l))) >> (7 - l);
					byte lowerColorBit = (lowerBitsRow & (1 << (7 - l))) >> (7 - l);
					byte colorNum = upperColorBit << 1 | lowerColorBit;

					backgorundTiles[i * 8 + k][j * 8 + l] = 3 - colorNum;
				}
			}
		}
	}
}

void LCD::renderBackgroundLine() {
	byte bgY = SCYreg;
	byte bgX = SCXreg;
	byte screenY = LYreg;
	if(screenY >= 0x90) return;
	byte screenX;

	word tileDataSelect = (LCDCreg & 0x10) == 0 ? 0x9000 : 0x8000;
	word tileMapSelect = (LCDCreg & 0x08) == 0 ? 0x9800 : 0x9C00;
	bool bgEnable = (LCDCreg & 0x01) != 0;

	for(int i = 0; i < 0xA0; i++) {
		screenX = i;

		byte mapX = (bgX + screenX) % 256;
		byte mapY = (bgY + screenY) % 256;
		byte tileNumX = mapX / 8;
		byte tileNumY = mapY / 8;
		byte pxInTileX = mapX % 8;
		byte pxInTileY = mapY % 8;

		word tilePositionInMapMem = tileNumY * 32 + tileNumX;
		word tileAddrInMapMem = tileMapSelect + tilePositionInMapMem;
		byte tileNum = mem->readByte(tileAddrInMapMem);
		word tileAddr = tileDataSelect + 16 * (tileDataSelect == 0x9000 ? (sbyte)tileNum : tileNum);

		byte upperTileByte = readByte(tileAddr + 2 * pxInTileY + 0);
		byte lowerTileByte = readByte(tileAddr + 2 * pxInTileY + 1);

		byte upperColorBit = (upperTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte lowerColorBit = (lowerTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte colorNum = upperColorBit << 1 | lowerColorBit;
		byte color = (BGPreg & 3 << colorNum * 2) >> colorNum * 2;

		screen[screenY][screenX] = bgEnable ? 3-color: 0x00;
		screenSourceData[screenY][screenX] = bgEnable ? colorNum : 0x00;
	}
}

void LCD::renderWindowLine() {
	byte wY = WYreg;
	byte wX = WXreg - 7;
	if(wX < 0) wX = 0; // Watch out: weird behavior
	byte screenY = LYreg;
	if(screenY >= 0x90) return;
	byte screenX;

	if(screenY < wY) return;

	word tileDataSelect = (LCDCreg & 0x10) == 0 ? 0x9000 : 0x8000;
	word tileMapSelect = (LCDCreg & 0x40) == 0 ? 0x9800 : 0x9C00;
	bool wEnable = (LCDCreg & 0x20) != 0;
	if(!wEnable) return;

	for(int i = wX; i < 0xA0; i++) {
		screenX = i;

		byte mapX = (screenX - wX) % 256;
		byte mapY = (screenY - wY) % 256;
		byte tileNumX = mapX / 8;
		byte tileNumY = mapY / 8;
		byte pxInTileX = mapX % 8;
		byte pxInTileY = mapY % 8;

		word tilePositionInMapMem = tileNumY * 32 + tileNumX;
		word tileAddrInMapMem = tileMapSelect + tilePositionInMapMem;
		byte tileNum = mem->readByte(tileAddrInMapMem);
		word tileAddr = tileDataSelect + 16 * (tileDataSelect == 0x9000 ? (sbyte) tileNum : tileNum);

		byte upperTileByte = readByte(tileAddr + 2 * pxInTileY + 0);
		byte lowerTileByte = readByte(tileAddr + 2 * pxInTileY + 1);

		byte upperColorBit = (upperTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte lowerColorBit = (lowerTileByte & (1 << (7 - pxInTileX))) >> (7 - pxInTileX);
		byte colorNum = upperColorBit << 1 | lowerColorBit;
		byte color = (BGPreg & 3 << colorNum * 2) >> colorNum * 2;

		screen[screenY][screenX] = 3 - color;
		screenSourceData[screenY][screenX] = colorNum;
	}
}

void LCD::renderSpritesLine() {
	bool spriteEnable = (LCDCreg & 0x02) != 0;
	if(!spriteEnable) return;
	int spriteSize = (LCDCreg & 0x04) >> 2;

	byte screenY = LYreg;
	if(screenY >= 0x90) return;
	byte screenX;

	int spritesOnLine[10];
	for(int i = 0; i < 10; i++) spritesOnLine[i] = -1;
	findSpritesOnLine(spritesOnLine, screenY, spriteSize);

	for(int i = 0; i < 10; i++) {
		if(spritesOnLine[i] == -1) break;

		byte spriteNum = spritesOnLine[i];

		byte spriteY = OAM[spriteNum * 4] - 16;
		byte spriteX = OAM[spriteNum * 4 + 1] - 8;
		byte tileNum = OAM[spriteNum * 4 + 2];
		byte attribs = OAM[spriteNum * 4 + 3];

		bool behindBG = (attribs & 0x80) != 0;
		bool flipY = (attribs & 0x40) != 0;
		bool flipX = (attribs & 0x20) != 0;
		byte paletteNum = (attribs & 0x10) >> 4;

		byte lineDiff = screenY - spriteY;

		word tileAddr;
		if(spriteSize == 0) {
			tileAddr = 0x8000 | tileNum << 4;
		} else {
			tileAddr = 0x8000 | (lineDiff > 7 && !flipY ? tileNum | 0x01 : tileNum & 0xFE) << 4;
		}

		word tileLineNum;

		if(flipY) {
			if(lineDiff > 7) tileLineNum = 15 - lineDiff;
			else tileLineNum = 7 - lineDiff;
		} else {
			if(lineDiff > 7) tileLineNum = lineDiff - 8;
			else tileLineNum = lineDiff;
		}

		byte upperTileByte = readByte(tileAddr + 2 * tileLineNum + 0);
		byte lowerTileByte = readByte(tileAddr + 2 * tileLineNum + 1);
		byte paletteReg = paletteNum == 0 ? OBP0reg : OBP1reg;

		for(int j = 0; j < 8; j++) {
			byte posX = flipX ? 7 - j : j;
			screenX = spriteX + j;
			if(screenX < 0 || screenX > 160) continue;

			byte upperColorBit = (upperTileByte & (1 << (7 - posX))) >> (7 - posX);
			byte lowerColorBit = (lowerTileByte & (1 << (7 - posX))) >> (7 - posX);
			byte colorNum = upperColorBit << 1 | lowerColorBit;
			byte color = (paletteReg & 3 << colorNum * 2) >> colorNum * 2;

			byte screenSourceDataNum = screenSourceData[screenY][screenX];

			// Sort the priority
			if(screenSourceDataNum < 4) {
				if((!behindBG || screenSourceDataNum == 0) && colorNum != 0) {
					screen[screenY][screenX] = 3 - color;
					screenSourceData[screenY][screenX] = spriteNum + 4;
				}
			} else if(spriteNum + 4 < screenSourceDataNum && colorNum != 0) {
				screen[screenY][screenX] = 3 - color;
				screenSourceData[screenY][screenX] = spriteNum + 4;
			}
		}
	}
}

void LCD::findSpritesOnLine(int* spritesOnLine, int line, int spriteSize) {
	int currSpriteInTable = 0;
	for(int i = 0; i < 0xA0 && currSpriteInTable < 10; i+=4) 
		if(line >= OAM[i] - 16 && line < OAM[i] - 16 + 8 + 8 * spriteSize)
			spritesOnLine[currSpriteInTable++] = i / 4;
}

void LCD::setStatMode(byte mode) {
	STATreg = STATreg & 0xFC | (mode & 0x03);
}

void LCD::setByte(word addr, byte data) {
	if(addr >= 0x8000 && addr < 0xA000) {
		VRAM[addr - 0x8000] = data;
		//std::cout << "Memory acces in LCD controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
		//
		//int a;
		//if(VRAM[0x240] == 0) {
		//	a = 1;
		//}
		//if(addr == 0x8fff) {
		//	int a = 2;
		//}
	}
	else if(addr >= 0xFE00 && addr < 0xFEA0)
		OAM[addr - 0xFE00] = data;
	else if(addr == 0xFF40)
		LCDCreg = data;
	else if(addr == 0xFF41)
		STATreg = data;
	else if(addr == 0xFF42)
		SCYreg = data;
	else if(addr == 0xFF43)
		SCXreg = data;
	else if(addr == 0xFF44)
		LYreg = data;
	else if(addr == 0xFF45)
		LYCreg = data;
	else if(addr == 0xFF46)
		DMAreg = data;
	else if(addr == 0xFF4A)
		WYreg = data;
	else if(addr == 0xFF4B)
		WXreg = data;
	else if(addr == 0xFF47)
		BGPreg = data;
	else if(addr == 0xFF48)
		OBP0reg = data & 0xFC;
	else if(addr == 0xFF49)
		OBP1reg = data & 0xFC;
	else
		std::cerr << "Unknown memory acces in LCD controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
}

byte LCD::getByte(word addr) {
	if(addr >= 0x8000 && addr < 0xA000)
		return VRAM[addr - 0x8000];
	else if(addr >= 0xFE00 && addr < 0xFEA0)
		return OAM[addr - 0xFE00];
	else if(addr == 0xFF40)
		return LCDCreg;
	else if(addr == 0xFF41)
		return STATreg;
	else if(addr == 0xFF42)
		return SCYreg;
	else if(addr == 0xFF43)
		return SCXreg;
	else if(addr == 0xFF44)
		return LYreg;
	else if(addr == 0xFF45)
		return LYCreg;
	else if(addr == 0xFF46)
		return DMAreg;
	else if(addr == 0xFF4A)
		return WYreg;
	else if(addr == 0xFF4B)
		return WXreg;
	else if(addr == 0xFF47)
		return BGPreg;
	else if(addr == 0xFF48)
		return OBP0reg;
	else if(addr == 0xFF49)
		return OBP1reg;
	else
		std::cerr << "Unknown memory acces in LCD controller: 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << std::endl;
	return -1;
}

void LCD::writeByte(word addr, byte data) {
	byte mode = STATreg & 0x03;
	if(addr >= 0x8000 && addr < 0xA000) {
		if(mode >= 0 && mode <= 2) setByte(addr, data);
		//else std::cerr << "Illegal LCD write at address 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << " in mode: " << +mode << std::endl;
	} else if(addr >= 0xFE00 && addr < 0xFE9F) {
		if(mode >= 0 && mode <= 1) setByte(addr, data);
		//else std::cerr << "Illegal LCD write at address 0x" << std::hex << std::uppercase << addr << std::nouppercase << std::dec << " in mode: " << +mode << std::endl;
	} else if(addr == 0xFF40) {
		byte currLCDC = getByte(0xFF40);
		if(((currLCDC ^ data) & 0x80) != 0x00)	// chaged on/off
			(data & 0x80) != 0 ? init() : turnOff();
		setByte(0xFF40, data);
	} else if(addr == 0xFF41) {
		setByte(addr, data & 0xFC);
	} else if(addr == 0xFF46) {
		dmaClocksLeft = 4 + 4 * 160;
		setByte(addr, data);
	} else {
		setByte(addr, data);
	}
}

byte LCD::readByte(word addr) {
	byte mode = STATreg & 0x03;
	if(addr >= 0x8000 && addr < 0xA000) {
		return mode >= 0 && mode <= 2 ? getByte(addr) : 0xFF;
	} else if(addr >= 0xFE00 && addr < 0xFE9F) {
		return mode >= 0 && mode <= 1 ? getByte(addr) : 0xFF;
	} else {
		return getByte(addr);
	}
}
