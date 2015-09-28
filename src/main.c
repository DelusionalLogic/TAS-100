#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include "libs/packet.h"
#include "libs/TWI_slave.h"
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
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETLABEL) {
			uint8_t line = pack.data[0];
			strncpy(labels[line], (char*)pack.data + 1, 19);
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_SETREG) {
			uint8_t reg = pack.data[0];
			int16_t value = (pack.data[1] << 8) | pack.data[2];
			registers[reg] = value;
			pack.type = PT_ACK;
			pack.length = 0;
			Packet_put(&pack);
		} else if(pack.type == PT_GETID) {
			pack.type = PT_ACK;
			pack.length = 1;
			pack.data[0] = ID;
			Packet_put(&pack);
		}
	}
}

uint8_t count = 0;
ISR(TIMER2_COMPA_vect) {
	if(count < 10) {
		count++;
		return;
	}
	count = 0;
	parsePacket();
}

int main (void)
{
	cli();
	///////////////////
	////SETUP BLOCK////
	///////////////////
	MCUCR &= 0b11101111;
	DDRD  &= 0b00000011; //Set all execept the two serial pins to input
	PORTD |= 0b00000100; //Enable the pullup on pin 3
	DDRB  =  0b00100000; //Set pin 13 as output
 
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

	//Timer setup
	TCCR2A = 0;
	TCCR2B = 0;

	OCR2A = 255;

	TCCR2B |= (1 << WGM12) | (1 << CS10) | (1 << CS12);
	TIMSK2 = (1 << OCIE2A);

	eeprom_busy_wait();
	ID = eeprom_read_byte(ID_LOC); //The ID is stored in the first byte

	Packet_init(ID);
	sei();

	///////////////////
	///PROGRAM BLOCK///
	///////////////////

	updateScreen();


	while(true) {
		PORTB |= 0b00100000;
		while((PIND & 0b00000100) != 0) {}
		interpret();

		updateScreen();

		PORTB = 0;
		_delay_ms(500);
	}


	while(1) {
		;
	}

	return 1;
}
