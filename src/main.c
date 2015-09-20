#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <util/setbaud.h>

#include "libs/packet.h"
#include "libs/TWI_slave.h"
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

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */


	eeprom_busy_wait();
	eeprom_write_byte(0x00, 0x64);
	ID = eeprom_read_byte(0x00); //The ID is stored in the first byte
	Packet_init(ID);
	sei();

	struct Packet pack;
	while(true) {
		if(Packet_get(&pack) == 0) {
			if(pack.type == PT_PING) {
				pack.type = PT_PONG;
				pack.length = 0;
				Packet_put(&pack);
			}
		}
		_delay_ms(100);
	}

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
