#ifndef CPU_H
#define CPU_H

#include <avr/io.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>

enum Opcode {
	OPC_MOV = 0x01,
	OPC_ADD,
	OPC_SUB,
	OPC_NEG,
	OPC_NOP,
	OPC_SWP,
	OPC_SAV,
	OPC_JMP,
	OPC_JEZ,
	OPC_JNZ,
	OPC_JGZ,
	OPC_JLZ,
	OPC_JRO,
};

//Lets just pretend that NIL is actually a register.
//As long as we never read from it that should work
enum Register { REG_ACC, REG_BAK, REG_NIL, REG_PC };
int16_t registers[4] = { 0, 0, 0, 0 };
#define GET_REG(reg) (registers[reg])
#define SET_REG(reg, val) do{ \
	if(val >= 999)\
		registers[reg] = 999;\
	else if(val <= -999)\
		registers[reg] = -999;\
	else\
		registers[reg] = val;\
} while(0)\

uint16_t instr[30] = {
	0x0011, 0x0140,
	0x0002, 0x0010,
	0x0002, 0x0010,
	0x0002, 0x00B0,
	0x0008, 0x00B0,
};

#define GET_OPC() GET_OPC_AT(GET_REG(REG_PC))
#define GET_A() GET_A_AT(GET_REG(REG_PC))
#define GET_B() GET_B_AT(GET_REG(REG_PC))

#define GET_OPC_AT(LINE) (instr[LINE*2] & 0xF)
#define GET_A_AT(LINE) ((instr[LINE*2] >> 4) & 0xFFF)
#define GET_B_AT(LINE) ((instr[(LINE*2)+1] >> 4) & 0xFFF)

#define NEXT_INSTR()  do{ \
	registers[REG_PC]++;\
	if(registers[REG_PC] >= 15 || GET_OPC() == 0x00)\
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

void DECODE_OP(uint16_t val, struct Operand* op) {
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
#endif

