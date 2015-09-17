#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "cpu.h"
#include "libs/packet.h"
#include "display.h"
#include "interpreter.h"

uint8_t ID = 0;




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
	updateScreen();

	while(true) {
		PORTB |= 0b00100000;
		while((PIND & 0b00000100) != 0) {}
		interpret();

		//Write ACC to packet
		/*
		newPack.recv = 0xFF;
		newPack.type = 0x11;
		newPack.length = 0x02;
		newPack.data[0] = (GET_REG(REG_ACC) >> 8) & 0xFF;
		newPack.data[1] = (GET_REG(REG_ACC)) & 0xFF;
		Packet_put(&newPack);
		*/

		updateScreen();

		PORTB = 0;
		_delay_ms(500);
	}


	while(1) {
		;
	}

	return 1;
}
