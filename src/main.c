#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "cpu.h"
#include "libs/packet.h"



uint8_t ID = 0;
struct Packet pack;
int16_t lit;

static inline void mov() {
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
	} else if(src.type == OP_LITERAL) {
		//This doesn't make any sense. Assign value to lit?
	}
	NEXT_INSTR();
	return;
}

static inline void jmp() {
	struct Operand ins;
	DECODE_OP(GET_B(), &ins);

	struct Packet newPack;
	newPack.recv = 0xFF;
	newPack.type = 0x11;
	newPack.length = 0x02;
	newPack.data[0] = (ins.litValue >> 8) & 0xFF;
	newPack.data[1] = (ins.litValue) & 0xFF;
	Packet_put(&newPack);

	SET_REG(REG_PC, ins.litValue);
}

int main (void)
{
	MCUCR &= 0b11101111;
	/* set PORTB for output*/
	DDRD  =  0b00000000;
	PORTD |= 0b00000100;
	DDRB  =  0b00100000;


	Packet_init();
	eeprom_busy_wait();
	ID = eeprom_read_byte(0x00); //The ID is stored in the first byte

	struct Packet* pack = NULL;
	while(pack == NULL) pack = Packet_get();
	Packet_put(pack);
	_delay_ms(1000);

	struct Packet newPack;
	newPack.recv = 0xFF;
	newPack.type = 0x11;
	newPack.length = 0x02;
	newPack.data[0] = (GET_REG(REG_ACC) >> 8) & 0xFF;
	newPack.data[1] = (GET_REG(REG_ACC)) & 0xFF;
	Packet_put(&newPack);

	while(true) {
		PORTB |= 0b00100000;
		while((PIND & 0b00000100) != 0) {}
		uint8_t opcode = GET_OPC();
		switch(opcode) {
			case UPC_MOV:
				mov();
				break;
			case UPC_ADD:
				SET_REG(REG_ACC, GET_REG(REG_ACC)+1);
				NEXT_INSTR();
				break;
			case UPC_SUB:
				SET_REG(REG_ACC, GET_REG(REG_ACC)-1);
				NEXT_INSTR();
				break;
			case UPC_NEG:
				SET_REG(REG_ACC, -GET_REG(REG_ACC));
				NEXT_INSTR();
				break;
			case UPC_SWP: {
				int16_t im = GET_REG(REG_ACC);
				SET_REG(REG_ACC, GET_REG(REG_BAK));
				SET_REG(REG_BAK, im);
				NEXT_INSTR();
				break;
		    }
			case UPC_SAV:
				SET_REG(REG_BAK, GET_REG(REG_ACC));
				NEXT_INSTR();
				break;
			case UPC_JMP:
				jmp();
				break;
		}

		//Write ACC to packet
		newPack.recv = 0xFF;
		newPack.type = 0x11;
		newPack.length = 0x02;
		newPack.data[0] = (GET_REG(REG_ACC) >> 8) & 0xFF;
		newPack.data[1] = (GET_REG(REG_ACC)) & 0xFF;
		Packet_put(&newPack);

		PORTB = 0;
		_delay_ms(500);
	}


	while(1) {
		;
	}

	return 1;
}
