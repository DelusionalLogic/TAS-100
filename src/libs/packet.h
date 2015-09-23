#ifndef PACKET_H
#define PACKET_H

#include <stdbool.h>
#include <avr/io.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <util/parity.h>

struct Packet {
	enum { 
		PT_ACK          = 0x01,
		PT_NACK,
		PT_PING,
		PT_PONG,
		PT_SETID        = 0x10,
		PT_SETPROG,
		PT_SETLABEL,
		PT_SETREG,
		PT_GETID        = 0x20,
		PT_GETPROG,
		PT_GETLABEL,
		PT_GETREG,
	} type;
	uint8_t length;
	uint8_t data[32];
	uint16_t checksum;


	//No transmit:
	bool broadcast;
};

extern struct Packet ackPack; //Ack packet template

void Packet_init(uint8_t id);

uint8_t Packet_get(struct Packet* packet);
void Packet_noput();
void Packet_put(struct Packet* packet);
#endif
