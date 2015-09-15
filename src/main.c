#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/eeprom.h>
#include "libs/packet.h"
 
short instr[] = {
	0x28, 0x11
};

uint8_t ID = 0;
struct Packet pack;
 
int main (void)
{
  /* set PORTB for output*/
  DDRB = 0xFF;
  Packet_init();
  eeprom_busy_wait();
  ID = eeprom_read_byte(0x00); //The ID is stored in the first byte

  while (1)
  {
	  struct Packet* pack = Packet_get();
	  if(pack != NULL) {
		  Packet_put(pack);
	  }
	  _delay_ms(10);
  }
 
  return 1;
}
