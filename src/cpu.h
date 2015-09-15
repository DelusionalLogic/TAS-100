#ifndef CPU_H
#define CPU_H

#include <avr/io.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>

enum Upcode {
	UPC_MOV = 0x01,
	UPC_ADD,
	UPC_SUB,
	UPC_NEG,
	UPC_SWP,
	UPC_SAV,
	UPC_JMP,
	UPC_JEZ,
	UPC_JNZ,
	UPC_JGZ,
	UPC_JLZ,
	UPC_JRO,
};

//Lets just pretend that NIL is actually a register.
//As long as we never read from it that should work
enum Register { REG_ACC, REG_BAK, REG_NIL, REG_PC };
int16_t registers[4] = { 0xABAB, 0, 0, 0 };
#define GET_REG(reg) (registers[reg])
#define SET_REG(reg, val) do{ registers[reg] = val; } while(0)

uint16_t instr[30] = {
	0x0011, 0x0140,
	0x0002, 0x0000,
	0x0002, 0x0000,
	0x0002, 0x0000,
	0x0007, 0x00C0,
};

#define GET_OPC() (instr[GET_REG(REG_PC)] & 0xF)
#define GET_A() ((instr[GET_REG(REG_PC)] >> 4) & 0xFFF)
#define GET_B() ((instr[GET_REG(REG_PC)+1] >> 4) & 0xFFF)

#define NEXT_INSTR()  do{ \
	registers[REG_PC]+=2;\
	if(registers[REG_PC] >= 30 || GET_OPC() == 0x00)\
		registers[REG_PC] = 0;\
}while(0)

enum Port { PORT_LEFT, PORT_RIGHT, PORT_UP, PORT_DOWN, PORT_ANY, PORT_LAST };

struct Operand {
	enum { OP_REGISTER, OP_PORT, OP_LITERAL } type;
	union {
		int16_t litValue;
		enum Register reg;
		enum Port port;
	};
};

#endif

static inline void DECODE_OP(uint16_t val, struct Operand* op) {
	if(val < 0x0A) {
		if(val <= 0x02) {
			op->type = OP_REGISTER;
			op->reg = val == 0x01 ? REG_ACC : REG_NIL;
		} else {
			op->type = OP_PORT;
			op->port = val - 0x03; //They happen to align correctly.
		}
	} else {
		op->type = OP_LITERAL;
		val -= 0x0A; //THIS CHANGES VAL CAREFUL
		//Put a small word into a larger word
		op->litValue = ((val & 0x800) << 4) | (val & 0x7FF);
	}
}
