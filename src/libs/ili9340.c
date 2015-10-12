/**
	This file is part of FORTMAX.

	FORTMAX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	FORTMAX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FORTMAX.  If not, see <http://www.gnu.org/licenses/>.

	Copyright: Martin K. Schröder (info@fortmax.se) 2014
*/

#include "ili9340.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <limits.h>

#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_MISO PB4
#define SPI_MOSI PB3
#define SPI_SCK PB5
#define SPI_SS PB2

#define SET_BIT(port, bitMask) *(port) |= (bitMask)
#define CLEAR_BIT(port, bitMask) *(port) &= ~(bitMask)

#define ILI_PORT PORTB
#define ILI_DDR DDRB
#define CS_PIN SPI_SS
#define RST_PIN PB1
#define DC_PIN PB0

#define _SB(port, pin) {port |= _BV(pin);}
#define _RB(port, pin) {port &= ~_BV(pin);}
#define CS_HI _SB(ILI_PORT, CS_PIN)
#define CS_LO _RB(ILI_PORT, CS_PIN)
#define RST_HI _SB(ILI_PORT, RST_PIN)
#define RST_LO _RB(ILI_PORT, RST_PIN)
#define DC_HI _SB(ILI_PORT, DC_PIN)
#define DC_LO _RB(ILI_PORT, DC_PIN)


//static uint16_t _width = ILI9340_TFTWIDTH, _height  = ILI9340_TFTHEIGHT;

static struct ili9340 {
	uint16_t screen_width, screen_height; 
	int16_t cursor_x, cursor_y;
	int8_t char_width, char_height;
	uint16_t back_color, front_color;
	uint16_t scroll_start; 
} term;


void _spi_init(void) {
    SPI_DDR &= ~((1<<SPI_MISO)); //input
    SPI_DDR |= ((1<<SPI_MOSI) | (1<<SPI_SS) | (1<<SPI_SCK)); //output

		// pullup! 
		SPI_PORT |= (1<<SPI_MISO);
		
    SPCR = ((1<<SPE)|               // SPI Enable
            (0<<SPIE)|              // SPI Interupt Enable
            (0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
            (1<<MSTR)|              // Master/Slave select
            (0<<SPR1)|(0<<SPR0)|    // SPI Clock Rate
            (0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    SPSR = (1<<SPI2X); // Double SPI Speed Bit
}

void _spi_write(uint8_t c) {
	SPDR = c;
	while(!(SPSR & _BV(SPIF)));
}


void _wr_command(uint8_t c) {
	DC_LO;
	CS_LO; 
  //CLEAR_BIT(dcport, dcpinmask);
  //digitalWrite(_dc, LOW);
  //CLEAR_BIT(clkport, clkpinmask);
  //digitalWrite(_sclk, LOW);
  //CLEAR_BIT(csport, cspinmask);
  //digitalWrite(_cs, LOW);

  _spi_write(c);

	CS_HI; 
  //SET_BIT(csport, cspinmask);
  //digitalWrite(_cs, HIGH);
}


void _wr_data(uint8_t c) {
	DC_HI; 
  CS_LO; 
  _spi_write(c);
	CS_HI; 
} 

void _wr_data16(uint16_t c){
	DC_HI; 
  CS_LO; 
  _spi_write(c >> 8);
  _spi_write(c & 0xff); 
	CS_HI;
}
// Rather than a bazillion _wr_command() and _wr_data() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80

void ili9340_init(void) {
	ILI_DDR |= _BV(RST_PIN);
	ILI_DDR |= _BV(DC_PIN);
	ILI_DDR |= _BV(CS_PIN);
	
	RST_LO; 

	_spi_init();
	
  RST_HI; 
  _delay_ms(5); 
  RST_LO; 
  _delay_ms(20);
  RST_HI; 
  _delay_ms(150);

  _wr_command(0xEF);
  _wr_data(0x03);
  _wr_data(0x80);
  _wr_data(0x02);

  _wr_command(0xCF);  
  _wr_data(0x00); 
  _wr_data(0XC1); 
  _wr_data(0X30); 

  _wr_command(0xED);  
  _wr_data(0x64); 
  _wr_data(0x03); 
  _wr_data(0X12); 
  _wr_data(0X81); 
 
  _wr_command(0xE8);  
  _wr_data(0x85); 
  _wr_data(0x00); 
  _wr_data(0x78); 

  _wr_command(0xCB);  
  _wr_data(0x39); 
  _wr_data(0x2C); 
  _wr_data(0x00); 
  _wr_data(0x34); 
  _wr_data(0x02); 
 
  _wr_command(0xF7);  
  _wr_data(0x20); 

  _wr_command(0xEA);  
  _wr_data(0x00); 
  _wr_data(0x00); 
 
  _wr_command(ILI9340_PWCTR1);    //Power control 
  _wr_data(0x23);   //VRH[5:0] 
 
  _wr_command(ILI9340_PWCTR2);    //Power control 
  _wr_data(0x10);   //SAP[2:0];BT[3:0] 
 
  _wr_command(ILI9340_VMCTR1);    //VCM control 
  _wr_data(0x3e); //�Աȶȵ���
  _wr_data(0x28); 
  
  _wr_command(ILI9340_VMCTR2);    //VCM control2 
  _wr_data(0x86);  //--
 
  _wr_command(ILI9340_MADCTL);    // Memory Access Control 
  _wr_data(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

  _wr_command(ILI9340_PIXFMT);    
  _wr_data(0x55); 
  
  _wr_command(ILI9340_FRMCTR1);    
  _wr_data(0x00);  
  _wr_data(0x18); 
 
  _wr_command(ILI9340_DFUNCTR);    // Display Function Control 
  _wr_data(0x08); 
  _wr_data(0x82);
  _wr_data(0x27);  
 
  _wr_command(0xF2);    // 3Gamma Function Disable 
  _wr_data(0x00); 
 
  _wr_command(ILI9340_GAMMASET);    //Gamma curve selected 
  _wr_data(0x01); 
 
  _wr_command(ILI9340_GMCTRP1);    //Set Gamma 
  _wr_data(0x0F); 
  _wr_data(0x31); 
  _wr_data(0x2B); 
  _wr_data(0x0C); 
  _wr_data(0x0E); 
  _wr_data(0x08); 
  _wr_data(0x4E); 
  _wr_data(0xF1); 
  _wr_data(0x37); 
  _wr_data(0x07); 
  _wr_data(0x10); 
  _wr_data(0x03); 
  _wr_data(0x0E); 
  _wr_data(0x09); 
  _wr_data(0x00); 
  
  _wr_command(ILI9340_GMCTRN1);    //Set Gamma 
  _wr_data(0x00); 
  _wr_data(0x0E); 
  _wr_data(0x14); 
  _wr_data(0x03); 
  _wr_data(0x11); 
  _wr_data(0x07); 
  _wr_data(0x31); 
  _wr_data(0xC1); 
  _wr_data(0x48); 
  _wr_data(0x08); 
  _wr_data(0x0F); 
  _wr_data(0x0C); 
  _wr_data(0x31); 
  _wr_data(0x36); 
  _wr_data(0x0F); 

  _wr_command(ILI9340_SLPOUT);    //Exit Sleep 
  _delay_ms(120); 		
  _wr_command(ILI9340_DISPON);    //Display on

  term.screen_width = ILI9340_TFTWIDTH;
  term.screen_height = ILI9340_TFTHEIGHT;
  term.char_height = 13;
  term.char_width = 8;
  term.back_color = 0x0000;
  term.front_color = 0xffff;
  term.cursor_x = term.cursor_y = 0;
  term.scroll_start = 0; 
}

void ili9340_setScrollStart(uint16_t start){
  _wr_command(0x37); // Vertical Scroll definition.
  _wr_data16(start);
  term.scroll_start = start; 
}


void ili9340_setScrollMargins(uint16_t top, uint16_t bottom) {
  // Did not pass in VSA as TFA+VSA=BFA must equal 320
	_wr_command(0x33); // Vertical Scroll definition.
  _wr_data16(top);
  _wr_data16(ili9340_height()-(top+bottom));
  _wr_data16(bottom); 
}

void ili9340_setAddrWindow(int16_t x0, int16_t y0, int16_t x1,
 int16_t y1) {
	/*y0 = (y0 - term.scroll_start);
	y1 = (y1 - term.scroll_start);
	if(y0 < 0) y0 = term.screen_height - y0;
	if(y1 < 0) y1 = term.screen_height - y0; */
	//y0 = (y0 + term.scroll_start) % term.screen_height;
	//y1 = (y1 + term.scroll_start) % term.screen_height;
	
  _wr_command(ILI9340_CASET); // Column addr set
  _wr_data(x0 >> 8);
  _wr_data(x0 & 0xFF);     // XSTART 
  _wr_data(x1 >> 8);
  _wr_data(x1 & 0xFF);     // XEND

  _wr_command(ILI9340_PASET); // Row addr set
  _wr_data(y0>>8);
  _wr_data(y0);     // YSTART
  _wr_data(y1>>8);
  _wr_data(y1);     // YEND

  _wr_command(ILI9340_RAMWR); // write to RAM
}


void ili9340_pushColor(uint16_t color) {
  DC_HI;
  CS_LO; 

  _spi_write(color >> 8);
  _spi_write(color);

	CS_HI; 
}
uint16_t ili9340_width(void){
	return term.screen_width;
}

uint16_t ili9340_height(void){
	return term.screen_height;
}

// fill a rectangle
void ili9340_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color) {
	struct ili9340 *t = &term;

	//y = (y + term.scroll_start) % term.screen_height;
	
  // rudimentary clipping (drawChar w/big text requires this)
  //if((x >= t->screen_width) || (y >= t->screen_height)) return;
  if((x + w - 1) >= t->screen_width)  w = t->screen_width  - x;
  if((y + h - 1) >= t->screen_height) h = t->screen_height - y;

  ili9340_setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  DC_HI; 
  CS_LO; 
  
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      _spi_write(hi);
      _spi_write(lo);
    }
  }
  CS_HI; 
}

void ili9340_setBackColor(uint16_t col){
	//uint8_t r, uint8_t g, uint8_t b
	struct ili9340 *t = &term;
	t->back_color = col; 
	//t->back_color = (uint16_t)r << 8 | (uint16_t)g << 4 | b; 
}

void ili9340_setFrontColor(uint16_t col){
	struct ili9340 *t = &term;
	t->front_color = col; 
	//t->front_color = (uint16_t)r << 8 | (uint16_t)g << 4 | b; 
}

uint8_t ili9340_remapChar(uint8_t ch) {
	if(ch >= 97 && ch <= 122)
		return ch - 97;
	if(ch >= 65 && ch <= 90)
		return ch - 39;
	if(ch >= 48 && ch <= 57)
		return ch + 4;
	if(ch == 58)
		return 62;
	if(ch == 33)
		return 63;
	if(ch == 46)
		return 64;
	if(ch == 32)
		return 0xFF;
	return 0;
}

void ili9340_drawChar(uint16_t x, uint16_t y, uint8_t ch){
	struct ili9340 *t = &term;

	ch = ili9340_remapChar(ch);
	
	ili9340_setAddrWindow(x, y, x+t->char_width-1, y + t->char_height);

	DC_HI;
	CS_LO;

	for(int i = 0; i < t->char_height; i++) {
		uint8_t line = ch == 0xFF ? 0 : pgm_read_byte(font + ch * t->char_height + i);
		for(int j = 0; j < t->char_width; j++) {
			int pix = line & _BV(j) ? t->front_color : t->back_color;
			_spi_write(pix >> 8);
			_spi_write(pix);
		}
	}

	CS_HI;
}

void ili9340_drawString(uint16_t x, uint16_t y, const char *text){
	static char _buffer[128]; // buffer for 1 char
	int len = strlen(text);
	struct ili9340 *t = &term;
	
	for(const char *_ch = text; *_ch; _ch++){
		DDRD |= _BV(5);
		PORTD |= _BV(5); 
		if(!*_ch) break;
		
		ili9340_drawChar(x, y, *_ch);
		x += t->char_width; 
		PORTD &= ~_BV(5); 
	}

}

void ili9340_drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {
	struct ili9340 *t = &term; 
  // Rudimentary clipping
  
	//y = (y + term.scroll_start) % term.screen_height;
	
  if((x >= t->screen_width) || (y >= t->screen_height)) return;
  if((x+w-1) >= t->screen_width)  w = t->screen_width-x;
  
  ili9340_setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;
  DC_HI;
  CS_LO; 
  while (w--) {
    _spi_write(hi);
    _spi_write(lo);
  }
  CS_HI; 
}

void ili9340_drawFastVLine(int16_t x, int16_t y, int16_t h,
  uint16_t color) {
	struct ili9340 *t = &term; 
  // Rudimentary clipping
  
	//y = (y + term.scroll_start) % term.screen_height;
	
  if((x >= t->screen_width) || (y >= t->screen_height)) return;
  if((y+h-1) >= t->screen_height)  h = t->screen_height-y;
  
  ili9340_setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;
  DC_HI;
  CS_LO; 
  while (h--) {
    _spi_write(hi);
    _spi_write(lo);
  }
  CS_HI; 
}

void ili9340_setRotation(uint8_t m) {
	struct ili9340 *t = &term; 
  _wr_command(ILI9340_MADCTL);
  int rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     _wr_data(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     t->screen_width  = ILI9340_TFTWIDTH;
     t->screen_height = ILI9340_TFTHEIGHT;
     break;
   case 1:
     _wr_data(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
     t->screen_width  = ILI9340_TFTHEIGHT;
     t->screen_height = ILI9340_TFTWIDTH;
     break;
  case 2:
    _wr_data(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
     t->screen_width  = ILI9340_TFTWIDTH;
     t->screen_height = ILI9340_TFTHEIGHT;
    break;
   case 3:
     _wr_data(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     t->screen_width  = ILI9340_TFTHEIGHT;
     t->screen_height = ILI9340_TFTWIDTH;
     break;
  }
}

