#ifndef PACKET_H
#define PACKET_H

#include <stdbool.h>
#include <avr/io.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <util/parity.h>

struct Packet {
	uint8_t recv;
	uint8_t seen : 1;
	uint8_t type : 7;
	uint8_t length;
	uint8_t data[32];
};

#define PACKQ_MAX 8
static struct Packet packQ[8];
static uint8_t qTop = 0;
static uint8_t qBot = 0;

void Packet_init() {
	cli();
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A |= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	UCSR0B |= _BV(RXCIE0);

	for(uint8_t i = 0; i < PACKQ_MAX; i++) {
		packQ[i].seen = 1;
	}

	sei();
}

static void putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

static char getchar() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

ISR(USART_RX_vect) {
	packQ[qTop].recv = getchar();
	packQ[qTop].type = getchar() & 0b01111111;
	packQ[qTop].length = getchar();

	for(uint8_t i = 0; i < packQ[qTop].length; i++) {
		packQ[qTop].data[i] = getchar();
	}
	packQ[qTop].seen = 0; //Ready to be read
	qTop = (qTop + 1) % PACKQ_MAX;
	putchar(qTop);
}

uint8_t Packet_waiting() {
	return qTop;
}

struct Packet* Packet_get() {
	uint8_t oldBot = qBot;
	qBot = (qBot + 1) % PACKQ_MAX;
	if(packQ[oldBot].seen != 0)
		return NULL;
	
	return &packQ[oldBot];
}

void Packet_put(struct Packet* packet) {
	cli();
	putchar(packet->recv);
	putchar(packet->type);
	putchar(packet->length);

	for(uint8_t i = 0; i < packet->length; i++) {
		putchar(packet->data[i]);
	}
	sei();
}

#endif
