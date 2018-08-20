#include "cpu.hpp"
#include <iostream>

CPU::CPU() {

}

CPU::~CPU() { }

void CPU::connectMemory(Memory* memory) {
	mem = memory;
}

void CPU::init() {
	// Init CPU registers
	regs.pc = 0x0100;
	regs.sp = 0xFFFE;
	regs.af = 0x01B0;
	regs.bc = 0x0013;
	regs.de = 0x00D8;
	regs.hl = 0x014D;

	// Init control registers
	mem -> writeByte(TIMA, 0x00);
	mem -> writeByte(TMA, 0x00);
	mem -> writeByte(TAC, 0x00);
	mem -> writeByte(NR10, 0x80);
	mem -> writeByte(NR11, 0xBF);
	mem -> writeByte(NR12, 0xF3);
	mem -> writeByte(NR14, 0xBF);
	mem -> writeByte(NR21, 0x3F);
	mem -> writeByte(NR22, 0x00);
	mem -> writeByte(NR24, 0xBF);
	mem -> writeByte(NR30, 0x7F);
	mem -> writeByte(NR31, 0xFF);
	mem -> writeByte(NR32, 0x9F);
	mem -> writeByte(NR33, 0xBF);
	mem -> writeByte(NR41, 0xFF);
	mem -> writeByte(NR42, 0x00);
	mem -> writeByte(NR43, 0x00);
	mem -> writeByte(NR44, 0xBF);
	mem -> writeByte(NR50, 0x77);
	mem -> writeByte(NR51, 0xF3);
	mem -> writeByte(NR52, 0xF1);
	mem -> writeByte(LCDC, 0x91);
	mem -> writeByte(SCY, 0x00);
	mem -> writeByte(SCX, 0x00);
	mem -> writeByte(LYC, 0x00);
	mem -> writeByte(BGP, 0xFC);
	mem -> writeByte(OBP0, 0xFF);
	mem -> writeByte(OBP1, 0xFF);
	mem -> writeByte(WY, 0x00);
	mem -> writeByte(WX, 0x00);
	mem -> writeByte(IE, 0x00);

	// Init internal clock
	clocks = 0;

	// Interrupts
	ime = false;
	delayIme = false;

	skipNext = false;
}

void CPU::exec(byte opcode) {
	// System halted/stopped
	if(halt || stop) {
		clocks += 4;
		return;
	}

	// Skip opcode
	regs.pc++;

	if(skipNext) {
		skipNext = false;
		return;
	}

	// Execute 
	switch (opcode) {
		case 0x00: // NOP
			clocks += 4;
			break;
		case 0x01: // LD BC, d16
			regs.bc = mem->readWord(regs.pc);
			regs.pc += 2;
			clocks += 12;
			break;
		case 0x02: // LD (BC), A
			mem -> writeByte(regs.bc, regs.a);
			clocks += 8;
			break;
		case 0x03: // INC BC
			regs.bc++;
			clocks += 8;
			break;
		case 0x04: // INC B
			regs.b = incByte(regs.b);
			clocks += 4;
			break;
		case 0x05: // DEC B
			regs.b = decByte(regs.b);
			clocks += 4;
			break;
		case 0x06: // LD B, d8
			regs.b = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x07: // RLCA
			regs.a = rlc(regs.a);
			regs.f &= 0x70;
			clocks += 4;
			break;
		case 0x08: // LD (a16), SP
			mem -> writeWord(mem -> readWord(regs.pc), regs.sp);
			regs.pc += 2;
			clocks += 20;
			break;
		case 0x09: // ADD HL, BC
			regs.hl = addWords(regs.hl, regs.bc);
			clocks += 8;
			break;
		case 0x0A: // LD A, (BC)
			regs.a = mem -> readByte(regs.bc);
			clocks += 8;
			break;
		case 0x0B: // DEC BC
			regs.bc--;
			clocks += 8;
			break;
		case 0x0C: // INC C
			regs.c = incByte(regs.c);
			clocks += 4;
			break;
		case 0x0D: // DEC C
			regs.c = decByte(regs.c);
			clocks += 4;
			break;
		case 0x0E: // LD C, d8
			regs.c = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x0F: // RRCA
			regs.a = rrc(regs.a);
			regs.f &= 0x70;
			clocks += 4;
			break;
		case 0x10: // STOP
			stop = true;
			clocks += 4;
			break;
		case 0x11: // LD DE, d16
			regs.de = mem -> readWord(regs.pc);
			regs.pc += 2;
			clocks += 12;
			break;
		case 0x12: // LD (DE), A
			mem -> writeByte(regs.de, regs.a);
			clocks += 8;
			break;
		case 0x13: // INC DE
			regs.de++;
			clocks += 8;
			break;
		case 0x14: // INC D
			regs.d = incByte(regs.d);
			clocks += 4;
			break;
		case 0x15: // DEC D
			regs.d = decByte(regs.d);
			break;
		case 0x16: // LD D, d8
			regs.d = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x17: // RLA
			regs.a = rl(regs.a);
			regs.f &= 0x70;
			clocks += 4;
			break;
		case 0x18: // JR r8
			jumpRelative((sbyte)mem -> readByte(regs.pc));
			clocks += 12;
			break;
		case 0x19: // ADD HL, DE
			regs.hl = addWords(regs.hl, regs.de);
			clocks += 8;
			break;
		case 0x1A: // LD A, (DE)
			regs.a = mem -> readByte(regs.de);
			clocks += 8;
			break;
		case 0x1B: // DEC DE
			regs.de--;
			clocks += 8;
			break;
		case 0x1C: // INC E
			regs.e = incByte(regs.e);
			clocks += 4;
			break;
		case 0x1D: // DEC E
			regs.e = decByte(regs.e);
			clocks += 4;
			break;
		case 0x1E: // LD E, d8
			regs.e = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x1F: // RRA
			regs.a = rr(regs.a);
			regs.f &= 0x70;
			clocks += 4;
			break;
		case 0x20: // JR NZ, r8
			if((regs.f & 0x80) == 0) { jumpRelative((sbyte)mem -> readByte(regs.pc)); clocks += 12; } 
			else { regs.pc++; clocks += 8; }
			break;
		case 0x21: // LD HL, d16
			regs.hl = mem -> readWord(regs.pc);
			regs.pc += 2;
			clocks += 12;
			break;
		case 0x22: // LD (HL+), A
			mem -> writeByte(regs.hl++, regs.a);
			clocks += 8;
			break;
		case 0x23: // INC HL
			regs.hl++;
			clocks += 8;
			break;
		case 0x24: // INC H
			regs.h = incByte(regs.h);
			clocks += 4;
			break;
		case 0x25: // DEC H
			regs.h = decByte(regs.h);
			clocks += 4;
			break;
		case 0x26: // LD H, d8
			regs.h = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x27: // DAA
			daa();
			clocks += 4;
			break;
		case 0x28: // JR Z, r8
			if((regs.f & 0x80) != 0) { jumpRelative((sbyte)mem -> readByte(regs.pc)); clocks += 12; } 
			else { regs.pc++; clocks += 8; }
			break;
		case 0x29: // ADD HL, HL
			regs.hl = addWords(regs.hl, regs.hl);
			clocks += 8;
			break;
		case 0x2A: // LD A, (HL+)
			regs.a = mem -> readByte(regs.hl++);
			clocks += 8;
			break;
		case 0x2B: // DEC HL
			regs.hl--;
			clocks += 8;
			break;
		case 0x2C: // INC L
			regs.l = incByte(regs.l);
			clocks += 4;
			break;
		case 0x2D: // DEC L
			regs.l = decByte(regs.l);
			clocks += 4;
			break;
		case 0x2E: // LD L, d8
			regs.l = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x2F: // CPL
			regs.a = ~regs.a;
			regs.f |= 0x60;
			clocks += 4;
			break;
		case 0x30: // JR NC, r8
			if((regs.f & 0x10) == 0) { jumpRelative((sbyte)mem -> readByte(regs.pc)); clocks += 12; } 
			else { regs.pc++; clocks += 8; }
			break;
		case 0x31: // LD SP, d16
			regs.sp = mem -> readWord(regs.pc);
			regs.pc += 2;
			clocks += 12;
			break;
		case 0x32: // LD (HL-), A
			mem -> writeByte(regs.hl--, regs.a);
			clocks += 8;
			break;
		case 0x33: // INC SP
			regs.sp++;
			clocks += 8;
			break;
		case 0x34: // INC (HL)
			mem -> writeByte(regs.hl, incByte(mem -> readByte(regs.hl)));
			clocks += 12;
			break;
		case 0x35: // DEC (HL)
			mem -> writeByte(regs.hl, decByte(mem -> readByte(regs.hl)));
			clocks += 12;
			break;
		case 0x36: // LD (HL), d8
			mem -> writeByte(regs.hl, mem -> readByte(regs.pc++));
			clocks += 12;
			break;
		case 0x37: // SCF
			regs.f |= 0x10;
			regs.f &= 0x90;
			clocks += 4;
			break;
		case 0x38: // JR C, r8
			if((regs.f & 0x10) != 0) { jumpRelative((sbyte)mem -> readByte(regs.pc)); clocks += 12; } 
			else { regs.pc++; clocks += 8; }
			break;
		case 0x39: // ADD HL, SP
			regs.hl = addWords(regs.hl, regs.sp);
			clocks += 8;
			break;
		case 0x3A: // LD A, (HL-)
			regs.a = mem -> readByte(regs.hl--);
			clocks += 8;
			break;
		case 0x3B: // DEC SP
			regs.sp--;
			clocks += 8;
			break;
		case 0x3C: // INC A
			regs.a = incByte(regs.a);
			clocks += 4;
			break;
		case 0x3D: // DEC A
			regs.a = decByte(regs.a);
			clocks += 4;
			break;
		case 0x3E: // LD A, d8
			regs.a = mem -> readByte(regs.pc++);
			clocks += 8;
			break;
		case 0x3F: // CCF
			regs.f ^= 0x10;
			regs.f &= 0x90;
			clocks += 4;
			break;
		case 0x40: // LD B, B
			regs.b = regs.b;
			clocks += 4;
			break;
		case 0x41: // LD B, C
			regs.b = regs.c;
			clocks += 4;
			break;
		case 0x42: // LD B, D
			regs.b = regs.d;
			clocks += 4;
			break;
		case 0x43: // LD B, E
			regs.b = regs.e;
			clocks += 4;
			break;
		case 0x44: // LD B, H
			regs.b = regs.h;
			clocks += 4;
			break;
		case 0x45: // LD B, L
			regs.b = regs.l;
			clocks += 4;
			break;
		case 0x46: // LD B, (HL)
			regs.b = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x47: // LD B, A
			regs.b = regs.a;
			clocks += 4;
			break;
		case 0x48: // LD C, B
			regs.c = regs.b;
			clocks += 4;
			break;
		case 0x49: // LD C, C 
			regs.c = regs.c;
			clocks += 4;
			break;
		case 0x4A: // LD C, D
			regs.c = regs.d;
			clocks += 4;
			break;
		case 0x4B: // LD C, E
			regs.c = regs.e;
			clocks += 4;
			break;
		case 0x4C: // LD C, H
			regs.c = regs.h;
			clocks += 4;
			break;
		case 0x4D: // LD C, L
			regs.c = regs.l;
			clocks += 4;
			break;
		case 0x4E: // LD C, (HL)
			regs.c = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x4F: // LD C, A
			regs.c = regs.a;
			clocks += 4;
			break;
		case 0x50: // LD D, B
			regs.d = regs.b;
			clocks += 4;
			break;
		case 0x51: // LD D, C
			regs.d = regs.c;
			clocks += 4;
			break;
		case 0x52: // LD D, D
			regs.d = regs.d;
			clocks += 4;
			break;
		case 0x53: // LD D, E
			regs.d = regs.e;
			clocks += 4;
			break;
		case 0x54: // LD D, H
			regs.d = regs.h;
			clocks += 4;
			break;
		case 0x55: // LD D, L
			regs.d = regs.l;
			clocks += 4;
			break;
		case 0x56: // LD D, (HL) 
			regs.d = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x57: // LD D, A
			regs.d = regs.a;
			clocks += 4;
			break;
		case 0x58: // LD E, B
			regs.e = regs.b;
			clocks += 4;
			break;
		case 0x59: // LD E, C
			regs.e = regs.c;
			clocks += 4;
			break;
		case 0x5A: // LD E, D
			regs.e = regs.d;
			clocks += 4;
			break;
		case 0x5B: // LD E, E
			regs.e = regs.e;
			clocks += 4;
			break;
		case 0x5C: // LD E, H
			regs.e = regs.h;
			clocks += 4;
			break;
		case 0x5D: // LD E, L
			regs.e = regs.l;
			clocks += 4;
			break;
		case 0x5E: // LD E, (HL)
			regs.e = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x5F: // LD E, A
			regs.e = regs.a;
			clocks += 4;
			break;
		case 0x60: // LD H, B
			regs.h = regs.b;
			clocks += 4;
			break;
		case 0x61: // LD H, C
			regs.h = regs.c;
			clocks += 4;
			break;
		case 0x62: // LD H, D
			regs.h = regs.d;
			clocks += 4;
			break;
		case 0x63: // LD H, E
			regs.h = regs.e;
			clocks += 4;
			break;
		case 0x64: // LD H, H
			regs.h = regs.h;
			clocks += 4;
			break;
		case 0x65: // LD H, L
			regs.h = regs.l;
			clocks += 4;
			break;
		case 0x66: // LD H, (HL)
			regs.h = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x67: // LD H, A
			regs.h = regs.a;
			clocks += 4;
			break;
		case 0x68: // LD L, B
			regs.l = regs.b;
			clocks += 4;
			break;
		case 0x69: // LD L, C
			regs.l = regs.c;
			clocks += 4;
			break;
		case 0x6A: // LD L, D
			regs.l = regs.d;
			clocks += 4;
			break;
		case 0x6B: // LD L, E
			regs.l = regs.e;
			clocks += 4;
			break;
		case 0x6C: // LD L, H
			regs.l = regs.h;
			clocks += 4;
			break;
		case 0x6D: // LD L, L
			regs.l = regs.l;
			clocks += 4;
			break;
		case 0x6E: // LD L, (HL)
			regs.l = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x6F: // LD L, A 
			regs.l = regs.a;
			clocks += 4;
			break;
		case 0x70: // LD (HL), B
			mem -> writeByte(regs.hl, regs.b);
			clocks += 8;
			break;
		case 0x71: // LD (HL), C
			mem -> writeByte(regs.hl, regs.c);
			clocks += 8;
			break;
		case 0x72: // LD (HL), D
			mem -> writeByte(regs.hl, regs.d);
			clocks += 8;
			break;
		case 0x73: // LD (HL), E
			mem -> writeByte(regs.hl, regs.e);
			clocks += 8;
			break;
		case 0x74: // LD (HL), H
			mem -> writeByte(regs.hl, regs.h);
			clocks += 8;
			break;
		case 0x75: // LD (HL), L
			mem -> writeByte(regs.hl, regs.l);
			clocks += 8;
			break;
		case 0x76: // HALT
			halt = true;
			clocks += 4;
			break;
		case 0x77: // LD (HL), A
			mem -> writeByte(regs.hl, regs.a);
			clocks += 8;
			break;
		case 0x78: // LD A, B
			regs.a = regs.b;
			clocks += 4;
			break;
		case 0x79: // LD A, C
			regs.a = regs.c;
			clocks += 4;
			break;
		case 0x7A: // LD A, D
			regs.a = regs.d;
			clocks += 4;
			break;
		case 0x7B: // LD A, E
			regs.a = regs.e;
			clocks += 4;
			break;
		case 0x7C: // LD A, H
			regs.a = regs.h;
			clocks += 4;
			break;
		case 0x7D: // LD A, L
			regs.a = regs.l;
			clocks += 4;
			break;
		case 0x7E: // LD A, (HL)
			regs.a = mem -> readByte(regs.hl);
			clocks += 8;
			break;
		case 0x7F: // LD A, A 
			regs.a = regs.a;
			clocks += 4;
			break;
		case 0x80: // ADD A, B
			add(regs.b);
			clocks += 4;
			break;
		case 0x81: // ADD A, C
			add(regs.c);
			clocks += 4;
			break;
		case 0x82: // ADD A, D
			add(regs.d);
			clocks += 4;
			break;
		case 0x83: // ADD A, E
			add(regs.e);
			clocks += 4;
			break;
		case 0x84: // ADD A, H
			add(regs.h);
			clocks += 4;
			break;
		case 0x85: // ADD A, L
			add(regs.l);
			clocks += 4;
			break;
		case 0x86: // ADD A, (HL)
			add(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0x87: // ADD A, A
			add(regs.a);
			clocks += 4;
			break;
		case 0x88: // ADC A, B
			adc(regs.b);
			clocks += 4;
			break;
		case 0x89: // ADC A, C
			adc(regs.c);
			clocks += 4;
			break;
		case 0x8A: // ADC A, D
			adc(regs.d);
			clocks += 4;
			break;
		case 0x8B: // ADC A, E
			adc(regs.e);
			clocks += 4;
			break;
		case 0x8C: // ADC A, H
			adc(regs.h);
			clocks += 4;
			break;
		case 0x8D: // ADC A, L
			adc(regs.l);
			clocks += 4;
			break;
		case 0x8E: // ADC A, (HL)
			adc(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0x8F: // ADC A, A
			adc(regs.a);
			clocks += 4;
			break;
		case 0x90: // SUB B
			sub(regs.b);
			clocks += 4;
			break;
		case 0x91: // SUB C
			sub(regs.c);
			clocks += 4;
			break;
		case 0x92: // SUB D
			sub(regs.d);
			clocks += 4;
			break;
		case 0x93: // SUB E
			sub(regs.e);
			clocks += 4;
			break;
		case 0x94: // SUB H
			sub(regs.h);
			clocks += 4;
			break;
		case 0x95: // SUB L
			sub(regs.l);
			clocks += 4;
			break;
		case 0x96: // SUB (HL)
			sub(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0x97: // SUB A
			sub(regs.a);
			clocks += 4;
			break;
		case 0x98: // SBC A, B
			sbc(regs.b);
			clocks += 4;
			break;
		case 0x99: // SBC A, C
			sbc(regs.c);
			clocks += 4;
			break;
		case 0x9A: // SBC A, D
			sbc(regs.d);
			clocks += 4;
			break;
		case 0x9B: // SBC A, E
			sbc(regs.e);
			clocks += 4;
			break;
		case 0x9C: // SBC A, H
			sbc(regs.h);
			clocks += 4;
			break;
		case 0x9D: // SBC A, L
			sbc(regs.l);
			clocks += 4;
			break;
		case 0x9E: // SBC A, (HL)
			sbc(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0x9F: // SBC A, A
			sbc(regs.a);
			clocks += 4;
			break;
		case 0xA0: // AND B
			and(regs.b);
			clocks += 4;
			break;
		case 0xA1: // AND C
			and(regs.c);
			clocks += 4;
			break;
		case 0xA2: // AND D
			and(regs.d);
			clocks += 4;
			break;
		case 0xA3: // AND E
			and(regs.e);
			clocks += 4;
			break;
		case 0xA4: // AND H
			and(regs.h);
			clocks += 4;
			break;
		case 0xA5: // AND L
			and(regs.l);
			clocks += 4;
			break;
		case 0xA6: // AND (HL)
			and(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0xA7: // AND A
			and(regs.a);
			clocks += 4;
			break;
		case 0xA8: // XOR B
			xor(regs.b);
			clocks += 4;
			break;
		case 0xA9: // XOR C
			xor(regs.c);
			clocks += 4;
			break;
		case 0xAA: // XOR D
			xor(regs.d);
			clocks += 4;
			break;
		case 0xAB: // XOR E
			xor(regs.e);
			clocks += 4;
			break;
		case 0xAC: // XOR H
			xor(regs.h);
			clocks += 4;
			break;
		case 0xAD: // XOR L
			xor(regs.l);
			clocks += 4;
			break;
		case 0xAE: // XOR (HL)
			xor(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0xAF: // XOR A
			xor(regs.a);
			clocks += 4;
			break;
		case 0xB0: // OR B
			or(regs.b);
			clocks += 4;
			break;
		case 0xB1: // OR C
			or(regs.c);
			clocks += 4;
			break;
		case 0xB2: // OR D
			or(regs.d);
			clocks += 4;
			break;
		case 0xB3: // OR E
			or(regs.e);
			clocks += 4;
			break;
		case 0xB4: // OR H
			or(regs.h);
			clocks += 4;
			break;
		case 0xB5: // OR L
			or(regs.l);
			clocks += 4;
			break;
		case 0xB6: // OR (HL)
			or(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0xB7: // OR A
			or(regs.a);
			clocks += 4;
			break;
		case 0xB8: // CP B
			cp(regs.b);
			clocks += 4;
			break;
		case 0xB9: // CP C
			cp(regs.c);
			clocks += 4;
			break;
		case 0xBA: // CP D
			cp(regs.d);
			clocks += 4;
			break;
		case 0xBB: // CP E
			cp(regs.e);
			clocks += 4;
			break;
		case 0xBC: // CP H
			cp(regs.h);
			clocks += 4;
			break;
		case 0xBD: // CP L
			cp(regs.l);
			clocks += 4;
			break;
		case 0xBE: // CP (HL)
			cp(mem -> readByte(regs.hl));
			clocks += 8;
			break;
		case 0xBF: // CP A
			cp(regs.a);
			clocks += 4;
			break;
		case 0xC0: // RET NZ
			if((regs.f & 0x80) == 0) { regs.pc = mem -> readWord(regs.sp); regs.sp += 2; clocks += 20; }
			else clocks += 8;
			break;
		case 0xC1: // POP BC
			regs.bc = mem -> readWord(regs.sp);
			regs.sp += 2;
			clocks += 12;
			break;
		case 0xC2: // JP NZ, a16
			if((regs.f & 0x80) == 0) { regs.pc = mem -> readWord(regs.pc); clocks += 16; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xC3: // JP a16
			regs.pc = mem -> readWord(regs.pc); 
			clocks += 16; 
			break;
		case 0xC4: // CALL NZ, a16
			if((regs.f & 0x80) == 0) { regs.sp -= 2; mem -> writeWord(regs.sp, regs.pc + 2); regs.pc = mem -> readWord(regs.pc); clocks += 24; }
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xC5: // PUSH BC
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.bc);
			clocks += 16;
			break;
		case 0xC6: // ADD A, d8
			add(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xC7: // RST 00H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x00;
			clocks += 16;
			break;
		case 0xC8: // RET Z
			if((regs.f & 0x80) != 0) { regs.pc = mem -> readWord(regs.sp); regs.sp += 2; clocks += 20; } 
			else clocks += 8;
			break;
		case 0xC9: // RET
			regs.pc = mem -> readWord(regs.sp); 
			regs.sp += 2; 
			clocks += 16;
			break;
		case 0xCA: // JP Z, a16
			if((regs.f & 0x80) != 0) { regs.pc = mem -> readWord(regs.pc); clocks += 16; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xCB: // PREFIX CB
			execExt(mem -> readByte(regs.pc));
			break;
		case 0xCC: // CALL Z, a16
			if((regs.f & 0x80) != 0) { regs.sp -= 2; mem -> writeWord(regs.sp, regs.pc + 2); regs.pc = mem -> readWord(regs.pc); clocks += 24; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xCD: // CALL a16
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc + 2); 
			regs.pc = mem -> readWord(regs.pc);
			clocks += 24;
			break;
		case 0xCE: // ADC A, d8
			adc(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xCF: // RST 08H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x08;
			clocks += 16;
			break;
		case 0xD0: // RET NC
			if((regs.f & 0x10) == 0) { regs.pc = mem -> readWord(regs.sp); regs.sp += 2; clocks += 20; } 
			else clocks += 8;
			break;
		case 0xD1: // POP DE
			regs.de = mem -> readWord(regs.sp);
			regs.sp += 2;
			clocks += 12;
			break;
		case 0xD2: // JP NC, a16
			if((regs.f & 0x10) == 0) { regs.pc = mem -> readWord(regs.pc); clocks += 16; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xD4: // CALL NC, a16
			if((regs.f & 0x10) == 0) { regs.sp -= 2; mem -> writeWord(regs.sp, regs.pc + 2); regs.pc = mem -> readWord(regs.pc); clocks += 24; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xD5: // PUSH DE
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.de);
			clocks += 16;
			break;
		case 0xD6: // SUB d8
			sub(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xD7: // RST 10H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x10;
			clocks += 16;
			break;
		case 0xD8: // RET C
			if((regs.f & 0x10) != 0) { regs.pc = mem -> readWord(regs.sp); regs.sp += 2; clocks += 20; } 
			else clocks += 8;
			break;
		case 0xD9: // RETI
			regs.pc = mem -> readWord(regs.sp);
			regs.sp += 2;
			ime = true;
			clocks += 16;
			break;
		case 0xDA: // JP C, a16
			if((regs.f & 0x10) != 0) { regs.pc = mem -> readWord(regs.pc); clocks += 16; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xDC: // CALL C, a16
			if((regs.f & 0x10) != 0) { regs.sp -= 2; mem -> writeWord(regs.sp, regs.pc + 2); regs.pc = mem -> readWord(regs.pc); clocks += 24; } 
			else { regs.pc += 2; clocks += 12; }
			break;
		case 0xDE: // SBC A, d8
			sbc(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xDF: // RST 18H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x18;
			clocks += 16;
			break;
		case 0xE0: // LDH (a8), A
			mem -> writeByte(0xFF00 + mem -> readByte(regs.pc++), regs.a);
			clocks += 12;
			break;
		case 0xE1: // POP HL
			regs.hl = mem -> readWord(regs.sp);
			regs.sp += 2;
			clocks += 12;
			break;
		case 0xE2: // LD (C), A
			mem -> writeByte(0xFF00 + regs.c, regs.a);
			clocks += 8;
			break;
		case 0xE5: // PUSH HL
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.hl);
			clocks += 16;
			break;
		case 0xE6: // AND d8
			and(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xE7: // RST 20H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x20;
			clocks += 16;
			break;
		case 0xE8: // ADD SP, r8
			regs.sp = addWordSbyte(regs.sp, (sbyte)mem -> readByte(regs.pc++));
			clocks += 16;
			break;
		case 0xE9: // JP (HL)
			regs.pc = regs.hl;
			clocks += 4;
			break;
		case 0xEA: // LD (a16), A
			mem -> writeByte(mem -> readWord(regs.pc), regs.a);
			regs.pc += 2;
			clocks += 16;
			break;
		case 0xEE: // XOR d8
			xor(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xEF: // RST 28H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x28;
			clocks += 16;
			break;
		case 0xF0: // LDH A, (a8)
			regs.a = mem -> readByte(0xFF00 + mem -> readByte(regs.pc++));
			clocks += 12;
			break;
		case 0xF1: // POP AF
			regs.af = mem -> readWord(regs.sp);
			regs.f &= 0xF0;
			regs.sp += 2;
			clocks += 12;
			break;
		case 0xF2: // LD A, (C)
			regs.a = mem -> readByte(0xFF00 + regs.c);
			clocks += 8;
			break;
		case 0xF3: // DI
			ime = false;
			clocks += 4;
			break;
		case 0xF5: // PUSH AF
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.af);
			clocks += 16;
			break;
		case 0xF6: // OR d8
			or(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xF7: // RST 30H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x30;
			clocks += 16;
			break;
		case 0xF8: // LD HL, SP+r8
			regs.hl = addWordSbyte(regs.sp, (sbyte)mem -> readByte(regs.pc++));
			clocks += 12;
			break;
		case 0xF9: // LD SP, HL
			regs.sp = regs.hl;
			clocks += 8;
			break;
		case 0xFA: // LD A, (a16)
			regs.a = mem -> readByte(mem -> readWord(regs.pc));
			regs.pc += 2;
			clocks += 16;
			break;
		case 0xFB: // EI
			if(delayIme) break;
			delayIme = true;
			clocks += 4;
			return;
		case 0xFE: // CP d8
			cp(mem -> readByte(regs.pc++));
			clocks += 8;
			break;
		case 0xFF: // RST 38H
			regs.sp -= 2;
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x38;
			clocks += 16;
			break;
		default:
			std::cerr << std::hex << std::uppercase << "Unknown opcode: 0x" << +opcode << " at address 0x" << regs.pc - 1 << std::nouppercase << std::dec << std::endl;
	}

	// Delay IE 1 instruction
	if(delayIme) {
		delayIme = false;
		ime = true;
	}
}

void CPU::execExt(byte opcode) {
	// Skip a byte
	regs.pc++;

	switch(opcode) {
		case 0x00: // RLC B
			regs.b = rlc(regs.b);
			clocks += 8;
			break;
		case 0x01: // RLC C
			regs.c = rlc(regs.c);
			clocks += 8;
			break;
		case 0x02: // RLC D
			regs.d = rlc(regs.d);
			clocks += 8;
			break;
		case 0x03: // RLC E
			regs.e = rlc(regs.e);
			clocks += 8;
			break;
		case 0x04: // RLC H
			regs.h = rlc(regs.h);
			clocks += 8;
			break;
		case 0x05: // RLC L
			regs.l = rlc(regs.l);
			clocks += 8;
			break;
		case 0x06: // RLC (HL)
			mem -> writeByte(regs.hl, rlc(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x07: // RLC A
			regs.a = rlc(regs.a);
			clocks += 8;
			break;
		case 0x08: // RRC B
			regs.b = rrc(regs.b);
			clocks += 8;
			break;
		case 0x09: // RRC C
			regs.c = rrc(regs.c);
			clocks += 8;
			break;
		case 0x0A: // RRC D
			regs.d = rrc(regs.d);
			clocks += 8;
			break;
		case 0x0B: // RRC E
			regs.e = rrc(regs.e);
			clocks += 8;
			break;
		case 0x0C: // RRC H
			regs.h = rrc(regs.h);
			clocks += 8;
			break;
		case 0x0D: // RRC L
			regs.l = rrc(regs.l);
			clocks += 8;
			break;
		case 0x0E: // RRC (HL)
			mem -> writeByte(regs.hl, rrc(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x0F: // RRC A
			regs.a = rrc(regs.a);
			clocks += 8;
			break;
		case 0x10: // RL B
			regs.b = rl(regs.b);
			clocks += 8;
			break;
		case 0x11: // RL C
			regs.c = rl(regs.c);
			clocks += 8;
			break;
		case 0x12: // RL D
			regs.d = rl(regs.d);
			clocks += 8;
			break;
		case 0x13: // RL E
			regs.e = rl(regs.e);
			clocks += 8;
			break;
		case 0x14: // RL H
			regs.h = rl(regs.h);
			clocks += 8;
			break;
		case 0x15: // RL L
			regs.l = rl(regs.l);
			clocks += 8;
			break;
		case 0x16: // RL (HL)
			mem -> writeByte(regs.hl, rl(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x17: // RL A
			regs.a = rl(regs.a);
			clocks += 8;
			break;
		case 0x18: // RR B
			regs.b = rr(regs.b);
			clocks += 8;
			break;
		case 0x19: // RR C
			regs.c = rr(regs.c);
			clocks += 8;
			break;
		case 0x1A: // RR D
			regs.d = rr(regs.d);
			clocks += 8;
			break;
		case 0x1B: // RR E
			regs.e = rr(regs.e);
			clocks += 8;
			break;
		case 0x1C: // RR H
			regs.h = rr(regs.h);
			clocks += 8;
			break;
		case 0x1D: // RR L
			regs.l = rr(regs.l);
			clocks += 8;
			break;
		case 0x1E: // RR (HL)
			mem -> writeByte(regs.hl, rr(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x1F: // RR A
			regs.a = rr(regs.a);
			clocks += 8;
			break;
		case 0x20: // SLA B
			regs.b = sla(regs.b);
			clocks += 8;
			break;
		case 0x21: // SLA C
			regs.c = sla(regs.c);
			clocks += 8;
			break;
		case 0x22: // SLA D
			regs.d = sla(regs.d);
			clocks += 8;
			break;
		case 0x23: // SLA E
			regs.e = sla(regs.e);
			clocks += 8;
			break;
		case 0x24: // SLA H
			regs.h = sla(regs.h);
			clocks += 8;
			break;
		case 0x25: // SLA L
			regs.l = sla(regs.l);
			clocks += 8;
			break;
		case 0x26: // SLA (HL)
			mem -> writeByte(regs.hl, sla(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x27: // SLA A
			regs.a = sla(regs.a);
			clocks += 8;
			break;
		case 0x28: // SRA B
			regs.b = sra(regs.b);
			clocks += 8;
			break;
		case 0x29: // SRA C
			regs.c = sra(regs.c);
			clocks += 8;
			break;
		case 0x2A: // SRA D
			regs.d = sra(regs.d);
			clocks += 8;
			break;
		case 0x2B: // SRA E
			regs.e = sra(regs.e);
			clocks += 8;
			break;
		case 0x2C: // SRA H
			regs.h = sra(regs.h);
			clocks += 8;
			break;
		case 0x2D: // SRA L
			regs.l = sra(regs.l);
			clocks += 8;
			break;
		case 0x2E: // SRA (HL)
			mem -> writeByte(regs.hl, sra(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x2F: // SRA A
			regs.a = sra(regs.a);
			clocks += 8;
			break;
		case 0x30: // SWAP B
			regs.b = swap(regs.b);
			clocks += 8;
			break;
		case 0x31: // SWAP C
			regs.c = swap(regs.c);
			clocks += 8;
			break;
		case 0x32: // SWAP D
			regs.d = swap(regs.d);
			clocks += 8;
			break;
		case 0x33: // SWAP E
			regs.e = swap(regs.e);
			clocks += 8;
			break;
		case 0x34: // SWAP H
			regs.h = swap(regs.h);
			clocks += 8;
			break;
		case 0x35: // SWAP L
			regs.l = swap(regs.l);
			clocks += 8;
			break;
		case 0x36: // SWAP (HL)
			mem -> writeByte(regs.hl, swap(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x37: // SWAP A
			regs.a = swap(regs.a);
			clocks += 8;
			break;
		case 0x38: // SRL B
			regs.b = srl(regs.b);
			clocks += 8;
			break;
		case 0x39: // SRL C
			regs.c = srl(regs.c);
			clocks += 8;
			break;
		case 0x3A: // SRL D
			regs.d = srl(regs.d);
			clocks += 8;
			break;
		case 0x3B: // SRL E
			regs.e = srl(regs.e);
			clocks += 8;
			break;
		case 0x3C: // SRL H
			regs.h = srl(regs.h);
			clocks += 8;
			break;
		case 0x3D: // SRL L
			regs.l = srl(regs.l);
			clocks += 8;
			break;
		case 0x3E: // SRL (HL)
			mem -> writeByte(regs.hl, srl(mem -> readByte(regs.hl)));
			clocks += 16;
			break;
		case 0x3F: // SRL A
			regs.a = srl(regs.a);
			clocks += 8;
			break;
		case 0x40: // BIT 0, B
			bit(regs.b, 0);
			clocks += 8;
			break;
		case 0x41: // BIT 0, C
			bit(regs.c, 0);
			clocks += 8;
			break;
		case 0x42: // BIT 0, D
			bit(regs.d, 0);
			clocks += 8;
			break;
		case 0x43: // BIT 0, E
			bit(regs.e, 0);
			clocks += 8;
			break;
		case 0x44: // BIT 0, H
			bit(regs.h, 0);
			clocks += 8;
			break;
		case 0x45: // BIT 0, L
			bit(regs.l, 0);
			clocks += 8;
			break;
		case 0x46: // BIT 0, HL
			bit(mem -> readByte(regs.hl), 0);
			clocks += 16;
			break;
		case 0x47: // BIT 0, A
			bit(regs.a, 0);
			clocks += 8;
			break;
		case 0x48: // BIT 1, B
			bit(regs.b, 1);
			clocks += 8;
			break;
		case 0x49: // BIT 1, C
			bit(regs.c, 1);
			clocks += 8;
			break;
		case 0x4A: // BIT 1, D
			bit(regs.d, 1);
			clocks += 8;
			break;
		case 0x4B: // BIT 1, E
			bit(regs.e, 1);
			clocks += 8;
			break;
		case 0x4C: // BIT 1, H
			bit(regs.h, 1);
			clocks += 8;
			break;
		case 0x4D: // BIT 1, L
			bit(regs.l, 1);
			clocks += 8;
			break;
		case 0x4E: // BIT 1, HL
			bit(mem -> readByte(regs.hl), 1);
			clocks += 16;
			break;
		case 0x4F: // BIT 1, A
			bit(regs.a, 1);
			clocks += 8;
			break;
		case 0x50: // BIT 2, B
			bit(regs.b, 2);
			clocks += 8;
			break;
		case 0x51: // BIT 2, C
			bit(regs.c, 2);
			clocks += 8;
			break;
		case 0x52: // BIT 2, D
			bit(regs.d, 2);
			clocks += 8;
			break;
		case 0x53: // BIT 2, E
			bit(regs.e, 2);
			clocks += 8;
			break;
		case 0x54: // BIT 2, H
			bit(regs.h, 2);
			clocks += 8;
			break;
		case 0x55: // BIT 2, L
			bit(regs.l, 2);
			clocks += 8;
			break;
		case 0x56: // BIT 2, HL
			bit(mem -> readByte(regs.hl), 2);
			clocks += 16;
			break;
		case 0x57: // BIT 2, A
			bit(regs.a, 2);
			clocks += 8;
			break;
		case 0x58: // BIT 3, B
			bit(regs.b, 3);
			clocks += 8;
			break;
		case 0x59: // BIT 3, C
			bit(regs.c, 3);
			clocks += 8;
			break;
		case 0x5A: // BIT 3, D
			bit(regs.d, 3);
			clocks += 8;
			break;
		case 0x5B: // BIT 3, E
			bit(regs.e, 3);
			clocks += 8;
			break;
		case 0x5C: // BIT 3, H
			bit(regs.h, 3);
			clocks += 8;
			break;
		case 0x5D: // BIT 3, L
			bit(regs.l, 3);
			clocks += 8;
			break;
		case 0x5E: // BIT 3, HL
			bit(mem -> readByte(regs.hl), 3);
			clocks += 16;
			break;
		case 0x5F: // BIT 3, A
			bit(regs.a, 3);
			clocks += 8;
			break;
		case 0x60: // BIT 4, B
			bit(regs.b, 4);
			clocks += 8;
			break;
		case 0x61: // BIT 4, C
			bit(regs.c, 4);
			clocks += 8;
			break;
		case 0x62: // BIT 4, D
			bit(regs.d, 4);
			clocks += 8;
			break;
		case 0x63: // BIT 4, E
			bit(regs.e, 4);
			clocks += 8;
			break;
		case 0x64: // BIT 4, H
			bit(regs.h, 4);
			clocks += 8;
			break;
		case 0x65: // BIT 4, L
			bit(regs.l, 4);
			clocks += 8;
			break;
		case 0x66: // BIT 4, HL
			bit(mem -> readByte(regs.hl), 4);
			clocks += 16;
			break;
		case 0x67: // BIT 4, A
			bit(regs.a, 4);
			clocks += 8;
			break;
		case 0x68: // BIT 5, B
			bit(regs.b, 5);
			clocks += 8;
			break;
		case 0x69: // BIT 5, C
			bit(regs.c, 5);
			clocks += 8;
			break;
		case 0x6A: // BIT 5, D
			bit(regs.d, 5);
			clocks += 8;
			break;
		case 0x6B: // BIT 5, E
			bit(regs.e, 5);
			clocks += 8;
			break;
		case 0x6C: // BIT 5, H
			bit(regs.h, 5);
			clocks += 8;
			break;
		case 0x6D: // BIT 5, L
			bit(regs.l, 5);
			clocks += 8;
			break;
		case 0x6E: // BIT 5, HL
			bit(mem -> readByte(regs.hl), 5);
			clocks += 16;
			break;
		case 0x6F: // BIT 5, A
			bit(regs.a, 5);
			clocks += 8;
			break;
		case 0x70: // BIT 6, B
			bit(regs.b, 6);
			clocks += 8;
			break;
		case 0x71: // BIT 6, C
			bit(regs.c, 6);
			clocks += 8;
			break;
		case 0x72: // BIT 6, D
			bit(regs.d, 6);
			clocks += 8;
			break;
		case 0x73: // BIT 6, E
			bit(regs.e, 6);
			clocks += 8;
			break;
		case 0x74: // BIT 6, H
			bit(regs.h, 6);
			clocks += 8;
			break;
		case 0x75: // BIT 6, L
			bit(regs.l, 6);
			clocks += 8;
			break;
		case 0x76: // BIT 6, HL
			bit(mem -> readByte(regs.hl), 6);
			clocks += 16;
			break;
		case 0x77: // BIT 6, A
			bit(regs.a, 6);
			clocks += 8;
			break;
		case 0x78: // BIT 7, B
			bit(regs.b, 7);
			clocks += 8;
			break;
		case 0x79: // BIT 7, C
			bit(regs.c, 7);
			clocks += 8;
			break;
		case 0x7A: // BIT 7, D
			bit(regs.d, 7);
			clocks += 8;
			break;
		case 0x7B: // BIT 7, E
			bit(regs.e, 7);
			clocks += 8;
			break;
		case 0x7C: // BIT 7, H
			bit(regs.h, 7);
			clocks += 8;
			break;
		case 0x7D: // BIT 7, L
			bit(regs.l, 7);
			clocks += 8;
			break;
		case 0x7E: // BIT 7, HL
			bit(mem -> readByte(regs.hl), 7);
			clocks += 16;
			break;
		case 0x7F: // BIT 7, A
			bit(regs.a, 7);
			clocks += 8;
			break;
		case 0x80: // RES 0, B
			regs.b = res(regs.b, 0);
			clocks += 8;
			break;
		case 0x81: // RES 0, C
			regs.c = res(regs.c, 0);
			clocks += 8;
			break;
		case 0x82: // RES 0, D
			regs.d = res(regs.d, 0);
			clocks += 8;
			break;
		case 0x83: // RES 0, E
			regs.e = res(regs.e, 0);
			clocks += 8;
			break;
		case 0x84: // RES 0, H
			regs.h = res(regs.h, 0);
			clocks += 8;
			break;
		case 0x85: // RES 0, L
			regs.l = res(regs.l, 0);
			clocks += 8;
			break;
		case 0x86: // RES 0, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 0));
			clocks += 16;
			break;
		case 0x87: // RES 0, A
			regs.a = res(regs.a, 0);
			clocks += 8;
			break;
		case 0x88: // RES 1, B
			regs.b = res(regs.b, 1);
			clocks += 8;
			break;
		case 0x89: // RES 1, C
			regs.c = res(regs.c, 1);
			clocks += 8;
			break;
		case 0x8A: // RES 1, D
			regs.d = res(regs.d, 1);
			clocks += 8;
			break;
		case 0x8B: // RES 1, E
			regs.e = res(regs.e, 1);
			clocks += 8;
			break;
		case 0x8C: // RES 1, H
			regs.h = res(regs.h, 1);
			clocks += 8;
			break;
		case 0x8D: // RES 1, L
			regs.l = res(regs.l, 1);
			clocks += 8;
			break;
		case 0x8E: // RES 1, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 1));
			clocks += 16;
			break;
		case 0x8F: // RES 1, A
			regs.a = res(regs.a, 1);
			clocks += 8;
			break;
		case 0x90: // RES 2, B
			regs.b = res(regs.b, 2);
			clocks += 8;
			break;
		case 0x91: // RES 2, C
			regs.c = res(regs.c, 2);
			clocks += 8;
			break;
		case 0x92: // RES 2, D
			regs.d = res(regs.d, 2);
			clocks += 8;
			break;
		case 0x93: // RES 2, E
			regs.e = res(regs.e, 2);
			clocks += 8;
			break;
		case 0x94: // RES 2, H
			regs.h = res(regs.h, 2);
			clocks += 8;
			break;
		case 0x95: // RES 2, L
			regs.l = res(regs.l, 2);
			clocks += 8;
			break;
		case 0x96: // RES 2, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 2));
			clocks += 16;
			break;
		case 0x97: // RES 2, A
			regs.a = res(regs.a, 2);
			clocks += 8;
			break;
		case 0x98: // RES 3, B
			regs.b = res(regs.b, 3);
			clocks += 8;
			break;
		case 0x99: // RES 3, C
			regs.c = res(regs.c, 3);
			clocks += 8;
			break;
		case 0x9A: // RES 3, D
			regs.d = res(regs.d, 3);
			clocks += 8;
			break;
		case 0x9B: // RES 3, E
			regs.e = res(regs.e, 3);
			clocks += 8;
			break;
		case 0x9C: // RES 3, H
			regs.h = res(regs.h, 3);
			clocks += 8;
			break;
		case 0x9D: // RES 3, L
			regs.l = res(regs.l, 3);
			clocks += 8;
			break;
		case 0x9E: // RES 3, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 3));
			clocks += 16;
			break;
		case 0x9F: // RES 3, A
			regs.a = res(regs.a, 3);
			clocks += 8;
			break;
		case 0xA0: // RES 4, B
			regs.b = res(regs.b, 4);
			clocks += 8;
			break;
		case 0xA1: // RES 4, C
			regs.c = res(regs.c, 4);
			clocks += 8;
			break;
		case 0xA2: // RES 4, D
			regs.d = res(regs.d, 4);
			clocks += 8;
			break;
		case 0xA3: // RES 4, E
			regs.e = res(regs.e, 4);
			clocks += 8;
			break;
		case 0xA4: // RES 4, H
			regs.h = res(regs.h, 4);
			clocks += 8;
			break;
		case 0xA5: // RES 4, L
			regs.l = res(regs.l, 4);
			clocks += 8;
			break;
		case 0xA6: // RES 4, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 4));
			clocks += 16;
			break;
		case 0xA7: // RES 4, A
			regs.a = res(regs.a, 4);
			clocks += 8;
			break;
		case 0xA8: // RES 5, B
			regs.b = res(regs.b, 5);
			clocks += 8;
			break;
		case 0xA9: // RES 5, C
			regs.c = res(regs.c, 5);
			clocks += 8;
			break;
		case 0xAA: // RES 5, D
			regs.d = res(regs.d, 5);
			clocks += 8;
			break;
		case 0xAB: // RES 5, E
			regs.e = res(regs.e, 5);
			clocks += 8;
			break;
		case 0xAC: // RES 5, H
			regs.h = res(regs.h, 5);
			clocks += 8;
			break;
		case 0xAD: // RES 5, L
			regs.l = res(regs.l, 5);
			clocks += 8;
			break;
		case 0xAE: // RES 5, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 5));
			clocks += 16;
			break;
		case 0xAF: // RES 5, A
			regs.a = res(regs.a, 5);
			clocks += 8;
			break;
		case 0xB0: // RES 6, B
			regs.b = res(regs.b, 6);
			clocks += 8;
			break;
		case 0xB1: // RES 6, C
			regs.c = res(regs.c, 6);
			clocks += 8;
			break;
		case 0xB2: // RES 6, D
			regs.d = res(regs.d, 6);
			clocks += 8;
			break;
		case 0xB3: // RES 6, E
			regs.e = res(regs.e, 6);
			clocks += 8;
			break;
		case 0xB4: // RES 6, H
			regs.h = res(regs.h, 6);
			clocks += 8;
			break;
		case 0xB5: // RES 6, L
			regs.l = res(regs.l, 6);
			clocks += 8;
			break;
		case 0xB6: // RES 6, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 6));
			clocks += 16;
			break;
		case 0xB7: // RES 6, A
			regs.a = res(regs.a, 6);
			clocks += 8;
			break;
		case 0xB8: // RES 7, B
			regs.b = res(regs.b, 7);
			clocks += 8;
			break;
		case 0xB9: // RES 7, C
			regs.c = res(regs.c, 7);
			clocks += 8;
			break;
		case 0xBA: // RES 7, D
			regs.d = res(regs.d, 7);
			clocks += 8;
			break;
		case 0xBB: // RES 7, E
			regs.e = res(regs.e, 7);
			clocks += 8;
			break;
		case 0xBC: // RES 7, H
			regs.h = res(regs.h, 7);
			clocks += 8;
			break;
		case 0xBD: // RES 7, L
			regs.l = res(regs.l, 7);
			clocks += 8;
			break;
		case 0xBE: // RES 7, (HL)
			mem -> writeByte(regs.hl, res(mem -> readByte(regs.hl), 7));
			clocks += 16;
			break;
		case 0xBF: // RES 7, A
			regs.a = res(regs.a, 7);
			clocks += 8;
			break;
		case 0xC0: // SET 0, B
			regs.b = set(regs.b, 0);
			clocks += 8;
			break;
		case 0xC1: // SET 0, C
			regs.c = set(regs.c, 0);
			clocks += 8;
			break;
		case 0xC2: // SET 0, D
			regs.d = set(regs.d, 0);
			clocks += 8;
			break;
		case 0xC3: // SET 0, E
			regs.e = set(regs.e, 0);
			clocks += 8;
			break;
		case 0xC4: // SET 0, H
			regs.h = set(regs.h, 0);
			clocks += 8;
			break;
		case 0xC5: // SET 0, L
			regs.l = set(regs.l, 0);
			clocks += 8;
			break;
		case 0xC6: // SET 0, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 0));
			clocks += 16;
			break;
		case 0xC7: // SET 0, A
			regs.a = set(regs.a, 0);
			clocks += 8;
			break;
		case 0xC8: // SET 1, B
			regs.b = set(regs.b, 1);
			clocks += 8;
			break;
		case 0xC9: // SET 1, C
			regs.c = set(regs.c, 1);
			clocks += 8;
			break;
		case 0xCA: // SET 1, D
			regs.d = set(regs.d, 1);
			clocks += 8;
			break;
		case 0xCB: // SET 1, E
			regs.e = set(regs.e, 1);
			clocks += 8;
			break;
		case 0xCC: // SET 1, H
			regs.h = set(regs.h, 1);
			clocks += 8;
			break;
		case 0xCD: // SET 1, L
			regs.l = set(regs.l, 1);
			clocks += 8;
			break;
		case 0xCE: // SET 1, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 1));
			clocks += 16;
			break;
		case 0xCF: // SET 1, A
			regs.a = set(regs.a, 1);
			clocks += 8;
			break;
		case 0xD0: // SET 2, B
			regs.b = set(regs.b, 2);
			clocks += 8;
			break;
		case 0xD1: // SET 2, C
			regs.c = set(regs.c, 2);
			clocks += 8;
			break;
		case 0xD2: // SET 2, D
			regs.d = set(regs.d, 2);
			clocks += 8;
			break;
		case 0xD3: // SET 2, E
			regs.e = set(regs.e, 2);
			clocks += 8;
			break;
		case 0xD4: // SET 2, H
			regs.h = set(regs.h, 2);
			clocks += 8;
			break;
		case 0xD5: // SET 2, L
			regs.l = set(regs.l, 2);
			clocks += 8;
			break;
		case 0xD6: // SET 2, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 2));
			clocks += 16;
			break;
		case 0xD7: // SET 2, A
			regs.a = set(regs.a, 2);
			clocks += 8;
			break;
		case 0xD8: // SET 3, B
			regs.b = set(regs.b, 3);
			clocks += 8;
			break;
		case 0xD9: // SET 3, C
			regs.c = set(regs.c, 3);
			clocks += 8;
			break;
		case 0xDA: // SET 3, D
			regs.d = set(regs.d, 3);
			clocks += 8;
			break;
		case 0xDB: // SET 3, E
			regs.e = set(regs.e, 3);
			clocks += 8;
			break;
		case 0xDC: // SET 3, H
			regs.h = set(regs.h, 3);
			clocks += 8;
			break;
		case 0xDD: // SET 3, L
			regs.l = set(regs.l, 3);
			clocks += 8;
			break;
		case 0xDE: // SET 3, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 3));
			clocks += 16;
			break;
		case 0xDF: // SET 3, A
			regs.a = set(regs.a, 3);
			clocks += 8;
			break;
		case 0xE0: // SET 4, B
			regs.b = set(regs.b, 4);
			clocks += 8;
			break;
		case 0xE1: // SET 4, C
			regs.c = set(regs.c, 4);
			clocks += 8;
			break;
		case 0xE2: // SET 4, D
			regs.d = set(regs.d, 4);
			clocks += 8;
			break;
		case 0xE3: // SET 4, E
			regs.e = set(regs.e, 4);
			clocks += 8;
			break;
		case 0xE4: // SET 4, H
			regs.h = set(regs.h, 4);
			clocks += 8;
			break;
		case 0xE5: // SET 4, L
			regs.l = set(regs.l, 4);
			clocks += 8;
			break;
		case 0xE6: // SET 4, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 4));
			clocks += 16;
			break;
		case 0xE7: // SET 4, A
			regs.a = set(regs.a, 4);
			clocks += 8;
			break;
		case 0xE8: // SET 5, B
			regs.b = set(regs.b, 5);
			clocks += 8;
			break;
		case 0xE9: // SET 5, C
			regs.c = set(regs.c, 5);
			clocks += 8;
			break;
		case 0xEA: // SET 5, D
			regs.d = set(regs.d, 5);
			clocks += 8;
			break;
		case 0xEB: // SET 5, E
			regs.e = set(regs.e, 5);
			clocks += 8;
			break;
		case 0xEC: // SET 5, H
			regs.h = set(regs.h, 5);
			clocks += 8;
			break;
		case 0xED: // SET 5, L
			regs.l = set(regs.l, 5);
			clocks += 8;
			break;
		case 0xEE: // SET 5, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 5));
			clocks += 16;
			break;
		case 0xEF: // SET 5, A
			regs.a = set(regs.a, 5);
			clocks += 8;
			break;
		case 0xF0: // SET 6, B
			regs.b = set(regs.b, 6);
			clocks += 8;
			break;
		case 0xF1: // SET 6, C
			regs.c = set(regs.c, 6);
			clocks += 8;
			break;
		case 0xF2: // SET 6, D
			regs.d = set(regs.d, 6);
			clocks += 8;
			break;
		case 0xF3: // SET 6, E
			regs.e = set(regs.e, 6);
			clocks += 8;
			break;
		case 0xF4: // SET 6, H
			regs.h = set(regs.h, 6);
			clocks += 8;
			break;
		case 0xF5: // SET 6, L
			regs.l = set(regs.l, 6);
			clocks += 8;
			break;
		case 0xF6: // SET 6, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 6));
			clocks += 16;
			break;
		case 0xF7: // SET 6, A
			regs.a = set(regs.a, 6);
			clocks += 8;
			break;
		case 0xF8: // SET 7, B
			regs.b = set(regs.b, 7);
			clocks += 8;
			break;
		case 0xF9: // SET 7, C
			regs.c = set(regs.c, 7);
			clocks += 8;
			break;
		case 0xFA: // SET 7, D
			regs.d = set(regs.d, 7);
			clocks += 8;
			break;
		case 0xFB: // SET 7, E
			regs.e = set(regs.e, 7);
			clocks += 8;
			break;
		case 0xFC: // SET 7, H
			regs.h = set(regs.h, 7);
			clocks += 8;
			break;
		case 0xFD: // SET 7, L
			regs.l = set(regs.l, 7);
			clocks += 8;
			break;
		case 0xFE: // SET 7, (HL)
			mem -> writeByte(regs.hl, set(mem -> readByte(regs.hl), 7));
			clocks += 16;
			break;
		case 0xFF: // SET 7, A
			regs.a = set(regs.a, 7);
			clocks += 8;
	}
}

void CPU::handleInterrupts() {
	if(ime) {
		if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x01) != 0) {	// VBLANK interrupt
			//std::cout << "VBLANK interupt triggered" << std::endl;
			mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) & 0xFE);	// Clear coresponfing IF flag
			regs.sp -= 2;												// Write PC to stack
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x40;												// Jump to interupt vector
			if(halt) {
				halt = false;											// Exit halt mode
				clocks += 4;											// Extra time	
			}
			ime = false;												// Disable IME
			clocks += 20;							
		}
		else if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x02) != 0) {	// LCD STAT interrput
			std::cout << "STAT interupt triggered" << std::endl;
			mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) & 0xFD);	// Clear coresponfing IF flag
			regs.sp -= 2;												// Write PC to stack
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x48;												// Jump to interupt vector
			if(halt) {
				halt = false;											// Exit halt mode
				clocks += 4;											// Extra time	
			}
			ime = false;												// Disable IME
			clocks += 20;
		}
		else if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x04) != 0) {	// Timer interrupt
			std::cout << "TIMER interupt triggered" << std::endl;
			mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) & 0xFB);	// Clear coresponfing IF flag
			regs.sp -= 2;												// Write PC to stack
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x50;												// Jump to interupt vector
			if(halt) {
				halt = false;											// Exit halt mode
				clocks += 4;											// Extra time	
			}
			ime = false;												// Disable IME
			clocks += 20;
		}
		else if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x08) != 0) {	// Serial interrupt
			std::cout << "SERIAL interupt triggered" << std::endl;
			mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) & 0xF7);	// Clear coresponfing IF flag
			regs.sp -= 2;												// Write PC to stack
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x58;												// Jump to interupt vector
			if(halt) {
				halt = false;											// Exit halt mode
				clocks += 4;											// Extra time	
			}
			ime = false;												// Disable IME
			clocks += 20;
		}
		else if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x10) != 0) {	// Joypad interrupt
			std::cout << "JOYPAD interupt triggered" << std::endl;
			mem -> writeByte(0xFF0F, mem -> readByte(0xFF0F) & 0xFE);	// Clear coresponfing IF flag
			regs.sp -= 2;												// Write PC to stack
			mem -> writeWord(regs.sp, regs.pc);
			regs.pc = 0x60;												// Jump to interupt vector
			if(halt) {
				halt = false;											// Exit halt mode
				clocks += 4;											// Extra time	
			}
			ime = false;												// Disable IME
			clocks += 20;
		}
	}
	
	if((mem -> readByte(IE) & mem -> readByte(0xFF0F) & 0x1F) != 0 && halt) {	// Exit halt regardles of IME
		halt = false;
		clocks += 4;
	}
}

byte CPU::incByte(byte op1) {
	op1++;									// Increment operand
	regs.f &= 0x10;							// Clear affected flags
	if(op1 == 0x00) regs.f |= 0x80;			// Set Z
	regs.f &= 0xB0;							// Clear N
	if((op1 & 0x0F) == 0x00) regs.f |= 0x20;// Set H
	return op1;
}

byte CPU::decByte(byte op1) {
	op1--;									// Decrement operand
	regs.f &= 0x10;							// Clear affected flags
	if(op1 == 0x00) regs.f |= 0x80;			// Set Z
	regs.f |= 0x40;							// Set N
	if((op1 & 0x0F) == 0x0F) regs.f |= 0x20;// Set H
	return op1;
}

byte CPU::rlc(byte op1) {
	byte bit7 = (op1 & 0x80) >> 7;	// Remember bit7
	op1 <<= 1;						// Shift left
	op1 |= bit7;					// Transfer rotated bit7
	regs.f = 0x00;					// Clear flags
	if(op1 == 0x00) regs.f |= 0x80;	// Set Z
	regs.f |= bit7 << 4;			// Set carry to rotated bit
	return op1;
}

byte CPU::rrc(byte op1) {
	byte bit1 = op1 & 0x01;			// Remember bit1
	op1 >>= 1;						// Shift right
	op1 |= bit1 << 7;				// Transfer rotated bit1
	regs.f = 0x00;					// Clear flags
	if(op1 == 0x00) regs.f |= 0x80;	// Set Z
	regs.f |= bit1 << 4;			// Set carry to rotated bit
	return op1;
}

byte CPU::rl(byte op1) {
	byte bit7 = op1 & 0x80;			// Remember bit7
	op1 <<= 1;						// Shift left
	op1 |= (regs.f & 0x10) >> 4;	// Carry to bit0
	regs.f = 0x00;					// Clear flags
	if(op1 == 0x00) regs.f |= 0x80;	// Set Z
	regs.f |= bit7 >> 3;			// Set carry to old bit7
	return op1;
}

byte CPU::rr(byte op1) {
	byte bit1 = op1 & 0x01;			// Remember bit1
	op1 >>= 1;						// Shift right
	op1 |= (regs.f & 0x10) << 3;	// Carry to bit7
	regs.f = 0x00;					// Clear flags
	if(op1 == 0x00) regs.f |= 0x80;	// Set Z
	regs.f |= bit1 << 4;			// Set carry to rotated bit
	return op1;
}

word CPU::addWords(word op1, word op2) {
	regs.f &= 0x80;												// Clear affected flags
	if((op1 & 0x0FFF) + (op2 & 0x0FFF) > 0x0FFF) regs.f |= 0x20;// Set H
	if(op1 + op2 > 0xFFFF) regs.f |= 0x10;						// Set C
	return op1 + op2;
}

void CPU::jumpRelative(sbyte data) {
	regs.pc += data + 1;
}

void CPU::daa() {
	byte N = regs.f & 0x40;				// N flag
	byte H = regs.f & 0x20;				// H flag
	byte C = regs.f & 0x10;				// C flag
	regs.f &= 0x40;						// Clear affected flags
	if((regs.a & 0x0F) > 9 || H != 0) {	// Lower nibble > 9 or H set
		regs.a += N == 0 ? 6 : -6;		// Add or subtract 6 based on N
	}
	if(regs.a >> 4 > 9 || C != 0) {		// Upper nibble > 9 or C set
		regs.a += N == 0 ? 0x60 : -0x60;// Add or subtract 0x60 based on N
		regs.f |= 0x10;					// Set C
	}
	if(regs.a == 0) regs.f |= 0x80;		// Set Z
}

void CPU::add(byte op1) {
	regs.f = 0x00;											// Reset flags
	if((regs.a & 0xF) + (op1 & 0xF) > 0xF) regs.f |= 0x20;	// Set H
	if(regs.a + op1 > 0xFF) regs.f |= 0x10;					// Set C
	regs.a += op1;											// Add to accumulator
	if(regs.a == 0x00) regs.f |= 0x80;						// Set Z
}

void CPU::adc(byte op1) {
	byte C = (regs.f & 0x10) >> 4;								// Remember carry
	regs.f = 0x00;												// Reset flags
	if((regs.a & 0xF) + (op1 & 0xF) + C > 0xF) regs.f |= 0x20;	// Set H
	if(regs.a + op1 + C > 0xFF) regs.f |= 0x10;					// Set C
	regs.a += op1 + C;											// Add to accumulator
	if(regs.a == 0x00) regs.f |= 0x80;							// Set Z
	regs.f &= 0xB0;												// Clear N
}

void CPU::sub(byte op1) {
	regs.f = 0x00;									// Reset flags
	if(regs.a == op1) regs.f |= 0x80;				// Set Z
	regs.f |= 0x40;									// Set N
	if((regs.a & 0xF) < (op1 & 0xF)) regs.f |= 0x20;// Set H
	if(regs.a < op1) regs.f |= 0x10;				// Set C
	regs.a -= op1;									// Subtract from accumulator
}

void CPU::sbc(byte op1) {
	byte C = (regs.f & 0x10) >> 4;						// Remember carry
	regs.f = 0x00;										// Reset flags
	if(regs.a == op1 + C) regs.f |= 0x80;				// Set Z
	regs.f |= 0x40;										// Set N
	if((regs.a & 0xF) < (op1 + C & 0xF)) regs.f |= 0x20;// Set H
	if(regs.a < op1 + C) regs.f |= 0x10;				// Set C
	regs.a -= op1 + C;									// Subtract from accumulator
}

void CPU::and(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.a &= op1;					// And accumulator with op1
	regs.f |= 0x20;					// Set H
	if(regs.a == 0) regs.f |= 0x80;	// Set Z
}

void CPU::xor(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.a ^= op1;					// Xor accumulator with op1
	if(regs.a == 0) regs.f |= 0x80;	// Set Z
}

void CPU::or(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.a |= op1;					// Or accumulator with op1
	if(regs.a == 0) regs.f |= 0x80;	// Set Z
}

void CPU::cp(byte op1) {
	regs.f = 0x00;									// Reset flags
	if(regs.a == op1) regs.f |= 0x80;				// Set Z
	regs.f |= 0x40;									// Set N
	if((regs.a & 0xF) < (op1 & 0xF)) regs.f |= 0x20;// Set H
	if(regs.a < op1) regs.f |= 0x10;				// Set C
}

word CPU::addWordSbyte(word op1, sbyte op2) {
	regs.f = 0x00;										// Reset flags
	if((op1 & 0xF) + (op2 & 0xF) > 0xF) regs.f |= 0x20;	// Set H
	if((op1 & 0xFF) + op2 > 0xFF) regs.f |= 0x10;		// Set C
	return op1 + (sword)op2;							// Add extended op2 and return sum
}

byte CPU::sla(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.f |= (op1 & 0x80) >> 3;	// Set C
	op1 <<= 1;						// Shift left
	if(op1 == 0) regs.f |= 0x80;	// Set Z
	return op1;
}

byte CPU::sra(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.f |= (op1 & 0x01) << 4;	// Set C
	op1 >>= 1;						// Shift right
	op1 |= ((op1 & 0x40) << 1);		// Restore bit7
	if(op1 == 0) regs.f |= 0x80;	// Set Z
	return op1;
}

byte CPU::srl(byte op1) {
	regs.f = 0x00;					// Reset flags
	regs.f |= (op1 & 0x01) << 4;	// Set C
	op1 >>= 1;						// Shift right
	if(op1 == 0) regs.f |= 0x80;	// Set Z
	return op1;
}

byte CPU::swap(byte op1) {
	regs.f = 0x00;					// Reset flags
	byte low = op1 & 0x0F;			// Remember low nibble
	op1 >>= 4;						// Shift high nibble to low nibble
	op1 |= low << 4;				// Restore low nibble to high nibble
	if(op1 == 0) regs.f |= 0x80;	// Set Z	
	return op1;
}

void CPU::bit(byte op1, byte bit) {
	regs.f &= 0x10;									// Clear affected flags
	if((op1 & 0x01 << bit) == 0) regs.f |= 0x80;	// Set Z
	regs.f |= 0x20;									// Set H
}

byte CPU::res(byte op1, byte bit) {
	return op1 &= ~(0x01 << bit);
}

byte CPU::set(byte op1, byte bit) {
	return op1 |= 0x01 << bit;
}
