#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <avr/cpufunc.h>

#include "libs/packet.h"
#include "libs/TWI_slave.h"
#include "libs/ili9340.h"
#include "display.h"
#include "cpu.h"
#include "interpreter.h"

#define ID_LOC 0x00

uint8_t ID = 0;

struct Packet pack;
void parsePacket() {
	if(Packet_get(&pack) == 0) {
		if(pack.type == PT_PING) {
			pack.type = PT_PONG;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETID) {
			if(pack.length != 1)
				return; //Not ID? Ignore
			cli();
			ID = pack.data[0];
			eeprom_write_byte(ID_LOC, pack.data[0]);
			_MemoryBarrier(); //Lets just make sure that we write that
			Packet_init(ID);
			sei();
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETPROG) {
			uint8_t line = pack.data[0] * 2;
			uint16_t fw = (pack.data[1] << 8) | pack.data[2];
			uint16_t sw = (pack.data[3] << 8) | pack.data[4];
			instr[line] = fw;
			instr[line+1] = sw;
			_MemoryBarrier(); //Lets just make sure that we write that
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETLABEL) {
			uint8_t line = pack.data[0];
			strncpy(labels[line], (char*)pack.data + 1, pack.length - 1);
			labels[line][pack.length] = '\0';
			_MemoryBarrier(); //Lets just make sure that we write that
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETREG) {
			uint8_t reg = pack.data[0];
			int16_t value = (pack.data[1] << 8) | pack.data[2];
			registers[reg] = value;
			_MemoryBarrier(); //Lets just make sure that we write that
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_GETID) {
			pack.type = PT_ACK;
			pack.length = 1;
			pack.data[0] = ID;
			Packet_put(&pack);
		} else if(pack.type == PT_GETPROG) {
			uint8_t line =  pack.data[0] * 2;
			pack.type = PT_ACK;
			pack.data[0] = (instr[line]     >> 8) & 0xFF;
			pack.data[1] =  instr[line]           & 0xFF;
			pack.data[2] = (instr[line + 1] >> 8) & 0xFF;
			pack.data[3] =  instr[line + 1]       & 0xFF;
			pack.length = 4;
			Packet_put(&pack);
		} else if(pack.type == PT_REDRAW) {
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
			redrawScreen();
		}
	}
}

int main (void)
{
	cli();
	///////////////////
	////SETUP BLOCK////
	///////////////////
	MCUCR &= 0b11101111;
	DDRD  &= 0b00000011; //Set all execept the two serial pins to input
	PORTD |= 0b00000000; //Don't Enable the pullup on pin 3
 
	//Serial
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	//Serial end

	eeprom_busy_wait();
	ID = eeprom_read_byte(ID_LOC); //The ID is stored in the first byte

	Packet_init(ID);
	sei();

	///////////////////
	///PROGRAM BLOCK///
	///////////////////

	initScreen();
	updateScreen();


	while(true) {
		PORTB |= 0b00100000;
		while((PIND & 0b00000100) == 0) {parsePacket();}
		_delay_ms(10);
		while((PIND & 0b00000100) != 0) {parsePacket();}
		interpret();

		updateScreen();

		PORTB = 0;
	}


	while(1) {
		;
	}

	return 1;
}
