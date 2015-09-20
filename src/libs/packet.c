#include "packet.h"
#include <avr/io.h>
#include <util/crc16.h>
#include <string.h>
#include "TWI_slave.h"

static uint8_t sendBuff[sizeof(struct Packet)];
static uint8_t recvBuff[sizeof(struct Packet)];

static inline uint8_t checksum(uint8_t data[], uint8_t length) {
	//TODO: write checksum
}

void Packet_init(uint8_t id) {
	TWI_Slave_Initialise(id);
	TWI_Start_Transceiver();
}

//Packet is destroyed when this method returns 1
uint8_t Packet_get(struct Packet* packet) {
	if(!TWI_Transceiver_Busy()) {
		if(TWI_statusReg.lastTransOK) {
			if(TWI_statusReg.RxDataInBuf){
				TWI_Get_Data_From_Transceiver(recvBuff, sizeof(recvBuff));
				packet->type = recvBuff[0];
				packet->length = recvBuff[1];

				memcpy(packet->data, recvBuff + 2, packet->length);
				
				packet->checksum = (recvBuff[2+packet->length] << 8) | recvBuff[3+packet->length];
				
				if(checksum != 0) {
					packet->type = PT_NACK;
					packet->length = 0;
				}
			}
		}
	}
}

void Packet_noput() {
	TWI_Start_Transceiver();
}

void Packet_put(struct Packet* packet) {
	cli();

	sendBuff[0] = packet->type;
	sendBuff[1] = packet->length;

	memcpy(sendBuff + 2, packet->data, packet->length);

	sendBuff[2 + packet->length] = (packet->checksum >> 8) & 0x00FF;
	sendBuff[3 + packet->length] = packet->checksum;

	TWI_Start_Transceiver_With_Data(sendBuff, 4 + packet->length);

	sei();
}

