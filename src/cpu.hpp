#ifndef CPU_HPP
#define CPU_HPP

#include "defs.hpp"
#include "memory.hpp"

class CPU {
	public:
		// Registers
		struct {
			union {
				struct { byte f; byte a; };
				word af;
			};
			union {
				struct { byte c; byte b; };
				word bc;
			};
			union {
				struct { byte e; byte d; };
				word de;
			};
			union {
				struct { byte l; byte h; };
				word hl;
			};
			word pc, sp;
		}regs;

		// Interrupt Master Enable
		bool ime;
		bool delayIme;

		// Memory managment
		Memory* mem;

		// Time
		int clocks;

		// States
		bool halt;
		bool stop;
		bool skipNext;
		

		CPU();
		~CPU();

		void connectMemory(Memory*);

		void init();

		void exec(byte);
		void execExt(byte);

		void handleInterrupts();

	private:
		byte incByte(byte);
		byte decByte(byte);
		byte rlc(byte);
		byte rrc(byte);
		byte rl(byte);
		byte rr(byte);
		word addWords(word, word);
		void jumpRelative(sbyte);
		void daa();
		void add(byte);
		void adc(byte);
		void sub(byte);
		void sbc(byte);
		void and(byte);
		void xor(byte);
		void or(byte);
		void cp(byte);
		word addWordSbyte(word, sbyte);
		byte sla(byte);
		byte sra(byte);
		byte srl(byte);
		byte swap(byte);
		void bit(byte, byte);
		byte res(byte, byte);
		byte set(byte, byte);
};

#endif
