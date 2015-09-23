#include "cpu.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

int16_t registers[4] = { 0, 0, 0, 0 };

uint16_t instr[30] = {
	/* TEST PROGRAM */
	/* 0x0011, 0x0140, */
	/* 0x0002, 0x0010, */
	/* 0x0002, 0x0010, */
	/* 0x0002, 0x00B0, */
	/* 0x0008, 0x00B0, */
	0
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
