#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>
#include <util/setbaud.h>
#include <stdbool.h>

void Serial_init() {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A |= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void Serial_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

char Serial_getchar() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

bool Serial_ischar() {
	return bit_is_set(UCSR0A, RXC0);
}

#endif
