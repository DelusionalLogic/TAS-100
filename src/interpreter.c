#include "interpreter.h"
#include <avr/io.h>
#include "cpu.h"


static void mov() {
	struct Operand src;
	struct Operand dst;
	DECODE_OP(GET_B(), &src);
	DECODE_OP(GET_A(), &dst);
	int16_t im = 0;
	if(src.type == OP_REGISTER) {
		im = GET_REG(src.reg);
	} else if(src.type == OP_LITERAL) {
		im = src.litValue;
	}

	if(dst.type == OP_REGISTER) {
		SET_REG(dst.reg, im);
	} else if(dst.type == OP_LITERAL) {
		//This doesn't make any sense. Assign value to lit?
	}
	NEXT_INSTR();
	return;
}

static void add() {
	struct Operand b;
	DECODE_OP(GET_B(), &b);

	if(b.type == OP_REGISTER) {
		SET_REG(REG_ACC, GET_REG(REG_ACC) + GET_REG(b.reg));
	} else if(b.type == OP_LITERAL) {
		SET_REG(REG_ACC, GET_REG(REG_ACC) + b.litValue);
	}
	NEXT_INSTR();
}

static void sub() {
	struct Operand b;
	DECODE_OP(GET_B(), &b);

	if(b.type == OP_REGISTER) {
		SET_REG(REG_ACC, GET_REG(REG_ACC) - GET_REG(b.reg));
	} else if(b.type == OP_LITERAL) {
		SET_REG(REG_ACC, GET_REG(REG_ACC) - b.litValue);
	}
	NEXT_INSTR();
}

static void jmp() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	SET_REG(REG_PC, ins.litValue);
}

instrFunc handlers[] = {
	NULL, //EMPTY OPCODE
	mov,
	add,
	sub,
	NULL, //NEG
	NULL, //NOP
	NULL, //SWP
	NULL, //SAV
	jmp,
	NULL, //JEZ
	NULL, //JNZ
	NULL, //JGZ
	NULL, //JLZ
	NULL, //JRO
};

void interpret() {
	uint8_t opcode = GET_OPC();
	if(handlers[opcode] != NULL)
		handlers[opcode]();
}
