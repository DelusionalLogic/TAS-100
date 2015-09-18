#ifndef PACKET_H
#define PACKET_H

#include <stdbool.h>
#include <avr/io.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <util/parity.h>

struct Packet {
	uint8_t type;
	uint8_t length;
	uint8_t data[32];
	uint16_t checksum;
};

void Packet_initu(uint8_t id);

struct Packet* Packet_get();
void Packet_put(struct Packet* packet);
#endif
