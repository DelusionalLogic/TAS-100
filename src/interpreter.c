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

static void neg() {
	SET_REG(REG_ACC, -GET_REG(REG_ACC));
	NEXT_INSTR();
}

static void nop() {
	NEXT_INSTR();
}

static void swp() {
	int16_t temp = GET_REG(REG_ACC);
	SET_REG(REG_ACC, GET_REG(REG_BAK));
	SET_REG(REG_BAK, temp);
	NEXT_INSTR();
}

static void sav() {
	SET_REG(REG_BAK, GET_REG(REG_ACC));
	NEXT_INSTR();
}

static void jmp() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	SET_REG(REG_PC, ins.litValue);
}

static void jez() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	if(GET_REG(REG_ACC) == 0)
		SET_REG(REG_PC, ins.litValue);
	else
		NEXT_INSTR();
}

static void jnz() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	if(GET_REG(REG_ACC) != 0)
		SET_REG(REG_PC, ins.litValue);
	else
		NEXT_INSTR();
}

static void jgz() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	if(GET_REG(REG_ACC) > 0)
		SET_REG(REG_PC, ins.litValue);
	else
		NEXT_INSTR();
}

static void jlz() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	if(GET_REG(REG_ACC) < 0)
		SET_REG(REG_PC, ins.litValue);
	else
		NEXT_INSTR();
}

instrFunc handlers[] = {
	NULL, //EMPTY OPCODE
	mov,
	add,
	sub,
	neg,
	nop,
	swp,
	sav,
	jmp,
	jez,
	jnz,
	jgz,
	jlz,
	NULL, //JRO
};

void interpret() {
	uint8_t opcode = GET_OPC();
	if(handlers[opcode] != NULL)
		handlers[opcode]();
}
