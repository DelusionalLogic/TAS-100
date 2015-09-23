#include "packet.h"
#include <avr/io.h>
#include <util/crc16.h>
#include <string.h>
#include "TWI_slave.h"

static uint8_t sendBuff[sizeof(struct Packet)];
static uint8_t recvBuff[sizeof(struct Packet)];

static inline uint16_t checksum(struct Packet* packet) {
	uint16_t crc = 0xFFFF;

	crc = _crc16_update(crc, packet->type);
	crc = _crc16_update(crc, packet->length);
	
	for(int i = 0; i < packet->length; i++) {
		crc = _crc16_update(crc, packet->data[i]);
	}
	return crc;
}

void Packet_init(uint8_t id) {
	TWI_Slave_Initialise((id << TWI_ADR_BITS) | (TRUE << TWI_GEN_BIT));
	TWI_Start_Transceiver();
}

struct Packet ackPack = {
	.type = PT_ACK,
	.length = 0
};

//packet param is in an undefined state when return value is != 0
uint8_t Packet_get(struct Packet* packet) {
	if(!TWI_Transceiver_Busy()) {
		if(TWI_statusReg.lastTransOK) {
			if(TWI_statusReg.RxDataInBuf){
				packet->broadcast = TWI_statusReg.genAddressCall ? true : false;
				TWI_Get_Data_From_Transceiver(recvBuff, sizeof(uint8_t) * 2); //READ HEADER
				packet->type = recvBuff[0];
				packet->length = recvBuff[1];

				TWI_Get_Data_From_Transceiver(recvBuff, sizeof(uint8_t) * (packet->length + 4)); //READ DATA AND FOOTER

				memcpy(packet->data, recvBuff + 2, packet->length);
				
				packet->checksum = (recvBuff[packet->length + 2] << 8) | recvBuff[packet->length + 3];

				if(checksum(packet) != packet->checksum) {
					packet->type = PT_NACK;
					packet->length = 0;
					
					Packet_put(packet);
					return 10;
				}
				if(packet->type == PT_NACK){
					TWI_Start_Transceiver_With_Data(sendBuff, 4 + sendBuff[1]);
				}
				return 0;
			} else {
				return 1;
			}
		} else {
			if(TWI_statusReg.RxDataInBuf) {
				packet->type = PT_NACK;
				packet->length = 0;

				Packet_put(packet);
				return 10;
			}
		}
	}
	return 1;
}

void Packet_noput() {
	TWI_Start_Transceiver();
}

void Packet_put(struct Packet* packet) {
	packet->checksum = checksum(packet);

	cli();

	sendBuff[0] = packet->type;
	sendBuff[1] = packet->length;

	memcpy(sendBuff + 2, packet->data, packet->length);

	sendBuff[2 + packet->length] = (packet->checksum >> 8) & 0x00FF;
	sendBuff[3 + packet->length] = packet->checksum;

	TWI_Start_Transceiver_With_Data(sendBuff, 4 + packet->length);

	sei();
}

