#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "cpu.h"

#define OPC_LEN 4
const char Instruction[][OPC_LEN] PROGMEM = {
	"MOV",
	"ADD",
	"SUB",
	"NEG",
	"NOP",
	"SWP",
	"SAV",
	"JMP",
	"JEZ",
	"JNZ",
	"JGZ",
	"JLZ",
	"JRO",
};

#define PORT_LEN 6
const char Port[][PORT_LEN] PROGMEM = {
	"LEFT\0",
	"RIGHT",
	"UP\0\0\0",
	"DOWN\0",
	"ANY\0\0",
	"LAST\0",
};

#define REG_LEN 4
const char Register[][REG_LEN] PROGMEM = {
	"ACC",
	"BAK",
	"NIL",
	"PC\0",
};

static void d_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

static void d_putstr(char* str, uint8_t len) {
	for (int i  = 0; i < len; i++) {
		if(str[i] == '\0')
			break;
		d_putchar(str[i]);
	}
}

static inline void otoa(uint16_t val, char* buff) {
	struct Operand op;
	DECODE_OP(val, &op);
	if(op.type == OP_REGISTER) {
		strncpy_P(buff, Register[op.reg], REG_LEN);
	} else if(op.type == OP_PORT) {
		strncpy_P(buff, Port[op.port], PORT_LEN);
	} else if(op.type == OP_LITERAL) {
		itoa(op.litValue, buff, 10);
	}
}

char buff[19];
void updateScreen() {
	d_putstr("ACC = ", 6);
	itoa(GET_REG(REG_ACC), buff, 10);
	d_putstr(buff, strlen(buff));
	d_putstr(", PC = ", 7);
	itoa(GET_REG(REG_PC), buff, 10);
	d_putstr(buff, strlen(buff));
	d_putstr(", OPC = 0x", 10);
	itoa(GET_OPC(), buff, 16);
	d_putstr(buff, strlen(buff));
	d_putstr(", A = 0x", 8);
	itoa(GET_B(), buff, 16);
	d_putstr(buff, strlen(buff));

	d_putchar('\n');

	strncpy_P(buff, Instruction[GET_OPC()-1], OPC_LEN);
	d_putstr(buff, OPC_LEN);

	switch(GET_OPC()) {
		case OPC_MOV:
			d_putchar(' ');
			otoa(GET_B(), buff);
			d_putstr(buff, 18);
			d_putchar(' ');
			otoa(GET_A(), buff);
			d_putstr(buff, 18);
			break;
		case OPC_JMP:
		case OPC_JEZ:
		case OPC_JNZ:
		case OPC_JGZ:
		case OPC_JLZ:
		case OPC_JRO:
			d_putstr("VALUE", 5);
			d_putchar(' ');
			otoa(GET_B(), buff);
			d_putstr(buff, 18);
			break;
	}

	d_putchar('\n');

	for(uint8_t i = 0; i < 15; i++) {
		if(GET_OPC_AT(i) == 0)
			continue;

		if(i == GET_REG(REG_PC))
			d_putchar('#');
		else
			d_putchar(' ');

		strncpy_P(buff, Instruction[GET_OPC_AT(i)-1], OPC_LEN);
		d_putstr(buff, OPC_LEN);

		switch(GET_OPC_AT(i)) {
			case OPC_MOV:
				d_putchar(' ');
				otoa(GET_B_AT(i), buff);
				d_putstr(buff, 18);
				d_putchar(' ');
				otoa(GET_A_AT(i), buff);
				d_putstr(buff, 18);
				break;
			case OPC_JMP:
			case OPC_JEZ:
			case OPC_JNZ:
			case OPC_JGZ:
			case OPC_JLZ:
			case OPC_JRO:
				d_putchar(' ');
				otoa(GET_B_AT(i), buff);
				d_putstr(buff, 18);
				break;
		}

		d_putchar('\n');

	}

	d_putchar('\n');
}

#endif
