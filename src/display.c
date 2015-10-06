#include "display.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "cpu.h"
#include "libs/ili9340.h"

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

volatile char labels[15][19] = {
	{0},
};

static inline uint8_t otoa(uint16_t val, char* buff) {
	struct Operand op;
	DECODE_OP(val, &op);
	if(op.type == OP_REGISTER) {
		strncpy_P(buff, Register[op.reg], REG_LEN);
	} else if(op.type == OP_PORT) {
		strncpy_P(buff, Port[op.port], PORT_LEN);
	} else if(op.type == OP_LITERAL) {
		itoa(op.litValue, buff, 10);
	}
	return strlen(buff);
}

char buff[19]; //Update screen shares this
uint8_t buffLen = 0;
uint16_t bg;
uint16_t fg;
void drawInstr(uint8_t i) {
	buffLen = 0;
	buff[0] = '\0';
	uint8_t y = (i * 8) + BOX_TOP + 2;
	uint8_t x = BOX_LEFT + 2;
	if(i == GET_REG(REG_PC)) {
		fg = 0x0000;
		bg = 0xFFFF;
	} else {
		fg = 0xFFFF;
		bg = 0x0000;
	}
	ili9340_fillRect(x-1, y-1, 109, 9, bg);
	ili9340_setBackColor(bg);
	ili9340_setFrontColor(fg);

	if(labels[i][0] != '\0') {
		buffLen += strlen(strcpy(buff + buffLen, labels[i]));
		buff[buffLen++] = ':';
	}

	if(GET_OPC_AT(i) == 0) {
		buff[buffLen++] = '\0';
		ili9340_drawString(x, y, buff); //Just draw the label and continue
		return;
	}

	strncpy_P(buff + buffLen, Instruction[GET_OPC_AT(i)-1], OPC_LEN);
	buffLen += 3; //All instrctions are 3 bytes long;

	switch(GET_OPC_AT(i)) {
		case OPC_MOV:
			buff[buffLen++] = ' ';
			buffLen += otoa(GET_B_AT(i), buff + buffLen);
			buff[buffLen++] = ' ';
			buffLen += otoa(GET_A_AT(i), buff + buffLen);
			break;
		case OPC_ADD:
		case OPC_SUB:
		case OPC_JRO:
			buff[buffLen++] = ' ';
			buffLen += otoa(GET_A_AT(i), buff + buffLen);
			break;
		case OPC_JMP:
		case OPC_JEZ:
		case OPC_JNZ:
		case OPC_JGZ:
		case OPC_JLZ: {
						  buff[buffLen++] = ' ';
						  struct Operand dst;
						  DECODE_OP(GET_A_AT(i), &dst);
						  buffLen += strlen(strcpy(buff + buffLen, labels[dst.litValue]));
						  break;
					  }
		default:
					  break;
	}
	ili9340_drawString(x, y, buff);
}

void redrawScreen();

void initScreen() {
	ili9340_init();
	ili9340_setRotation(1);
	redrawScreen();
}

void redrawScreen() {
	ili9340_setBackColor(0x0000);
	ili9340_setFrontColor(0xFFFF);
	ili9340_fillRect(0, 0, SCR_WIDTH, SCR_HEIGHT, 0x0000);

	ili9340_drawFastHLine(BOX_LEFT,  BOX_TOP,    BOX_WIDTH,  0xFFFF);
	ili9340_drawFastHLine(BOX_LEFT,  BOX_BOTTOM, BOX_WIDTH,  0xFFFF);
	ili9340_drawFastVLine(BOX_LEFT,  BOX_TOP,    BOX_HEIGHT, 0xFFFF);
	ili9340_drawFastVLine(BOX_RIGHT, BOX_TOP,    BOX_HEIGHT, 0xFFFF);

	ili9340_drawFastVLine(BOX_SPLIT, BOX_TOP,    BOX_HEIGHT, 0xFFFF);

	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*0 + 3, "ACC");
	ili9340_drawFastHLine(BOX_SPLIT,     BOX_TOP + INFO_HEIGHT*1,     INFO_WIDTH, 0xFFFF);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*1 + 3, "BAK");
	ili9340_drawFastHLine(BOX_SPLIT,     BOX_TOP + INFO_HEIGHT*2,     INFO_WIDTH, 0xFFFF);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*2 + 3, "LAST");
	ili9340_drawFastHLine(BOX_SPLIT,     BOX_TOP + INFO_HEIGHT*3,     INFO_WIDTH, 0xFFFF);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*3 + 3, "MODE");
	ili9340_drawFastHLine(BOX_SPLIT,     BOX_TOP + INFO_HEIGHT*4,     INFO_WIDTH, 0xFFFF);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*4 + 3, "IDLE");

	itoa(GET_REG(REG_ACC), buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*0 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*0 + 11, buff);
	itoa(GET_REG(REG_BAK), buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*1 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*1 + 11, buff);
	//TODO: LAST: Last side used when ANY
	//TODO: MODE: Current Running mode (IDLE, RUN, WRITING, READING)
	//IDLE PERCENTAGE
	itoa(0, buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*4 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*4 + 11, buff);

	for(uint8_t i = 0; i < 15; i++) {
		drawInstr(i);
	}
}

uint8_t lastPC = 0;
void updateScreen() {
	ili9340_setBackColor(0x0000);
	ili9340_setFrontColor(0xFFFF);
	//Draw info
	itoa(GET_REG(REG_ACC), buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*0 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*0 + 11, buff);
	itoa(GET_REG(REG_BAK), buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*1 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*1 + 11, buff);
	//TODO: LAST: Last side used when ANY
	//TODO: MODE: Current Running mode (IDLE, RUN, WRITING, READING)
	//IDLE PERCENTAGE
	itoa(0, buff, 10);
	ili9340_fillRect(     BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*4 + 11, 6*4, 8, 0x0000);
	ili9340_drawString(   BOX_SPLIT + 6, BOX_TOP + INFO_HEIGHT*4 + 11, buff);

	//Draw instruction
	for(uint8_t i = 0; i < 15; i++) {
		if(i != lastPC && i != GET_REG(REG_PC))
			continue;
		drawInstr(i);
	}
	lastPC = GET_REG(REG_PC);
}
