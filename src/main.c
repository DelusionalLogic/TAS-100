#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/eeprom.h>

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


	TWI_Slave_Initialise((0x64<<TWI_ADR_BITS) | (TRUE<<TWI_GEN_BIT));
	Packet_init();
	sei();
	eeprom_busy_wait();
	ID = eeprom_read_byte(0x00); //The ID is stored in the first byte

	TWI_Start_Transceiver();
	while(true) {
		uint8_t buff[2];
		if(!TWI_Transceiver_Busy()) {
			if(TWI_statusReg.lastTransOK) {
				if(TWI_statusReg.RxDataInBuf) {
					TWI_Get_Data_From_Transceiver(buff, 2);
					PORTB = buff[0];
					TWI_Start_Transceiver_With_Data(buff, 1);
				}
			}
		}
	}

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
