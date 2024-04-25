#include "font.h"
#include <stddef.h>

#define GLYPH_BITMAP_LEN 128

/* 
 * Glyph bitmap representation of a character. 
 * Note the actual bitmaps were made in GIMP.
 */
struct glyph_bitmap {
	char c;
	uint8_t bitmap[GLYPH_BITMAP_LEN];
} glyph_bitmaps[] = {
{ 'A', { 0b00000000,0b00001111,0b10000000,0b00000000,
	 0b00000000,0b00001111,0b11000000,0b00000000,
	 0b00000000,0b00001111,0b11000000,0b00000000,
	 0b00000000,0b00011101,0b11000000,0b00000000,
	 0b00000000,0b00011101,0b11100000,0b00000000,
	 0b00000000,0b00011101,0b11100000,0b00000000,
	 0b00000000,0b00111100,0b11100000,0b00000000,
	 0b00000000,0b00111000,0b11110000,0b00000000,
	 0b00000000,0b00111000,0b11110000,0b00000000,
	 0b00000000,0b01111000,0b11110000,0b00000000,
	 0b00000000,0b01110000,0b01111000,0b00000000,
	 0b00000000,0b11110000,0b01111000,0b00000000,
	 0b00000000,0b11110000,0b01111000,0b00000000,
	 0b00000000,0b11110000,0b00111100,0b00000000,
	 0b00000001,0b11100000,0b00111100,0b00000000,
	 0b00000001,0b11100000,0b00111100,0b00000000,
	 0b00000001,0b11100000,0b00011110,0b00000000,
	 0b00000011,0b11000000,0b00011110,0b00000000,
	 0b00000011,0b11000000,0b00011110,0b00000000,
	 0b00000011,0b11111111,0b11111111,0b00000000,
	 0b00000111,0b11111111,0b11111111,0b00000000,
	 0b00000111,0b11111111,0b11111111,0b00000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b11000000,
	 0b00011110,0b00000000,0b00000011,0b11000000,
	 0b00011110,0b00000000,0b00000011,0b11000000,
	 0b00011110,0b00000000,0b00000011,0b11100000,
	 0b00111110,0b00000000,0b00000001,0b11100000,
	 0b00111100,0b00000000,0b00000001,0b11100000,
	 0b00111100,0b00000000,0b00000001,0b11110000 } },

{ 'B', { 0b00000111,0b11111111,0b11100000,0b00000000,
	 0b00000111,0b11111111,0b11111000,0b00000000,
	 0b00000111,0b11111111,0b11111110,0b00000000,
	 0b00000111,0b10000011,0b11111111,0b00000000,
	 0b00000111,0b10000000,0b00011111,0b00000000,
	 0b00000111,0b10000000,0b00001111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00001111,0b00000000,
	 0b00000111,0b10000000,0b00011110,0b00000000,
	 0b00000111,0b10000011,0b11111100,0b00000000,
	 0b00000111,0b11111111,0b11111000,0b00000000,
	 0b00000111,0b11111111,0b11111000,0b00000000,
	 0b00000111,0b11111111,0b11111110,0b00000000,
	 0b00000111,0b10000000,0b00011111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b11000000,
	 0b00000111,0b10000000,0b00000011,0b11000000,
	 0b00000111,0b10000000,0b00000011,0b11100000,
	 0b00000111,0b10000000,0b00000001,0b11100000,
	 0b00000111,0b10000000,0b00000001,0b11100000,
	 0b00000111,0b10000000,0b00000001,0b11100000,
	 0b00000111,0b10000000,0b00000011,0b11100000,
	 0b00000111,0b10000000,0b00000011,0b11000000,
	 0b00000111,0b10000000,0b00000111,0b11000000,
	 0b00000111,0b10000000,0b00001111,0b11000000,
	 0b00000111,0b10000001,0b11111111,0b10000000,
	 0b00000111,0b11111111,0b11111111,0b00000000,
	 0b00000111,0b11111111,0b11111100,0b00000000,
	 0b00000111,0b11111111,0b11100000,0b00000000 } },

{ 'C', { 0b00000000,0b00000011,0b11111000,0b00000000,
	 0b00000000,0b00011111,0b11111110,0b00000000,
	 0b00000000,0b01111111,0b11111111,0b10000000,
	 0b00000000,0b11111111,0b11111111,0b11000000,
	 0b00000001,0b11111000,0b00000111,0b11000000,
	 0b00000011,0b11110000,0b00000001,0b10000000,
	 0b00000011,0b11100000,0b00000000,0b00000000,
	 0b00000111,0b11000000,0b00000000,0b00000000,
	 0b00000111,0b10000000,0b00000000,0b00000000,
	 0b00001111,0b10000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b10000000,0b00000000,0b00000000,
	 0b00000111,0b10000000,0b00000000,0b00000000,
	 0b00000111,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11100000,0b00000000,0b10000000,
	 0b00000011,0b11110000,0b00000001,0b11000000,
	 0b00000001,0b11111000,0b00000011,0b11100000,
	 0b00000000,0b11111111,0b11111111,0b11000000,
	 0b00000000,0b01111111,0b11111111,0b10000000,
	 0b00000000,0b00011111,0b11111110,0b00000000,
	 0b00000000,0b00000111,0b11111000,0b00000000 } },

{ 'D', { 0b00001111,0b11111111,0b10000000,0b00000000,
	 0b00001111,0b11111111,0b11110000,0b00000000,
	 0b00001111,0b11111111,0b11111000,0b00000000,
	 0b00001111,0b00000011,0b11111110,0b00000000,
	 0b00001111,0b00000000,0b01111111,0b00000000,
	 0b00001111,0b00000000,0b00011111,0b00000000,
	 0b00001111,0b00000000,0b00001111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11100000,
	 0b00001111,0b00000000,0b00000011,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000001,0b11100000,
	 0b00001111,0b00000000,0b00000011,0b11100000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000111,0b11000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00001111,0b10000000,
	 0b00001111,0b00000000,0b00011111,0b00000000,
	 0b00001111,0b00000000,0b01111111,0b00000000,
	 0b00001111,0b00000011,0b11111110,0b00000000,
	 0b00001111,0b11111111,0b11111000,0b00000000,
	 0b00001111,0b11111111,0b11110000,0b00000000,
	 0b00001111,0b11111111,0b10000000,0b00000000 } },

{ 'E', { 0b00000011,0b11111111,0b11111111,0b10000000,
	 0b00000011,0b11111111,0b11111111,0b10000000,
	 0b00000011,0b11111111,0b11111111,0b10000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11111111,0b11111110,0b00000000,
	 0b00000011,0b11111111,0b11111110,0b00000000,
	 0b00000011,0b11111111,0b11111110,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11111111,0b11111111,0b11000000,
	 0b00000011,0b11111111,0b11111111,0b11000000,
	 0b00000011,0b11111111,0b11111111,0b11000000 } },

{ 'F', { 0b00000001,0b11111111,0b11111111,0b11000000,
	 0b00000001,0b11111111,0b11111111,0b11000000,
	 0b00000001,0b11111111,0b11111111,0b11000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11111111,0b11111111,0b00000000,
	 0b00000001,0b11111111,0b11111111,0b00000000,
	 0b00000001,0b11111111,0b11111111,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000,
	 0b00000001,0b11100000,0b00000000,0b00000000 } },

{ 'G', { 0b00000000,0b00000111,0b11110000,0b00000000,
	 0b00000000,0b00111111,0b11111110,0b00000000,
	 0b00000000,0b01111111,0b11111111,0b00000000,
	 0b00000001,0b11111111,0b11111111,0b10000000,
	 0b00000011,0b11111000,0b00000111,0b10000000,
	 0b00000011,0b11100000,0b00000011,0b00000000,
	 0b00000111,0b11000000,0b00000000,0b00000000,
	 0b00001111,0b10000000,0b00000000,0b00000000,
	 0b00001111,0b10000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b00000000,0b00000000,
	 0b00011110,0b00000000,0b11111111,0b11000000,
	 0b00011110,0b00000000,0b11111111,0b11000000,
	 0b00011110,0b00000000,0b11111111,0b11000000,
	 0b00011110,0b00000000,0b00000011,0b11000000,
	 0b00011110,0b00000000,0b00000011,0b11000000,
	 0b00011111,0b00000000,0b00000011,0b11000000,
	 0b00011111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b10000000,0b00000011,0b11000000,
	 0b00000111,0b11000000,0b00000011,0b11000000,
	 0b00000111,0b11100000,0b00000011,0b11000000,
	 0b00000011,0b11110000,0b00000111,0b11000000,
	 0b00000001,0b11111111,0b11111111,0b11000000,
	 0b00000000,0b11111111,0b11111111,0b10000000,
	 0b00000000,0b00111111,0b11111110,0b00000000,
	 0b00000000,0b00001111,0b11110000,0b00000000 } },

{ '#', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00011000,0b00001110,0b00000000,
	 0b00000000,0b00011000,0b00001100,0b00000000,
	 0b00000000,0b00011000,0b00001100,0b00000000,
	 0b00000000,0b00111000,0b00001100,0b00000000,
	 0b00000000,0b00111000,0b00011100,0b00000000,
	 0b00000000,0b00111000,0b00011100,0b00000000,
	 0b00000000,0b00111000,0b00011100,0b00000000,
	 0b00000000,0b00111000,0b00011100,0b00000000,
	 0b00000000,0b00110000,0b00011100,0b00000000,
	 0b00000111,0b11111111,0b11111111,0b10000000,
	 0b00000111,0b11111111,0b11111111,0b10000000,
	 0b00000111,0b11111111,0b11111111,0b10000000,
	 0b00000000,0b01110000,0b00111000,0b00000000,
	 0b00000000,0b01110000,0b00111000,0b00000000,
	 0b00000000,0b01110000,0b00111000,0b00000000,
	 0b00000000,0b01110000,0b00111000,0b00000000,
	 0b00000000,0b01100000,0b00111000,0b00000000,
	 0b00000000,0b01100000,0b00110000,0b00000000,
	 0b00001111,0b11111111,0b11111111,0b00000000,
	 0b00001111,0b11111111,0b11111111,0b00000000,
	 0b00001111,0b11111111,0b11111111,0b00000000,
	 0b00000000,0b11100000,0b01110000,0b00000000,
	 0b00000000,0b11100000,0b01110000,0b00000000,
	 0b00000000,0b11100000,0b01110000,0b00000000,
	 0b00000000,0b11000000,0b01110000,0b00000000,
	 0b00000000,0b11000000,0b01100000,0b00000000,
	 0b00000001,0b11000000,0b01100000,0b00000000,
	 0b00000001,0b11000000,0b11100000,0b00000000,
	 0b00000001,0b11000000,0b11100000,0b00000000,
	 0b00000001,0b11000000,0b11100000,0b00000000,
	 0b00000001,0b11000000,0b11100000,0b00000000 } },

{ '?', { 0b00000000,0b00111111,0b10000000,0b00000000,
	 0b00000000,0b11111111,0b11100000,0b00000000,
	 0b00000011,0b11111111,0b11110000,0b00000000,
	 0b00000111,0b11100001,0b11111000,0b00000000,
	 0b00000011,0b10000000,0b01111100,0b00000000,
	 0b00000001,0b00000000,0b00111100,0b00000000,               
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b01111000,0b00000000,
	 0b00000000,0b00000000,0b11110000,0b00000000,               
	 0b00000000,0b00000001,0b11110000,0b00000000,
	 0b00000000,0b00000011,0b11100000,0b00000000,
	 0b00000000,0b00000111,0b11000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,               
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b00011100,0b00000000,0b00000000,
	 0b00000000,0b00011100,0b00000000,0b00000000,               
	 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b00111111,0b00000000,0b00000000,               
	 0b00000000,0b00111111,0b00000000,0b00000000,
	 0b00000000,0b00111111,0b10000000,0b00000000,
	 0b00000000,0b00111111,0b00000000,0b00000000,
	 0b00000000,0b00111111,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000 } },

{ '0', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00011111,0b11000000,0b00000000,
	 0b00000000,0b01111111,0b11110000,0b00000000,
	 0b00000000,0b11111111,0b11111100,0b00000000,
	 0b00000001,0b11111000,0b01111110,0b00000000,
	 0b00000011,0b11100000,0b00011110,0b00000000,
	 0b00000011,0b11000000,0b00001111,0b00000000,
	 0b00000111,0b10000000,0b00001111,0b00000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000011,0b10000000,
	 0b00001110,0b00000000,0b00000011,0b11000000,
	 0b00001110,0b00000111,0b00000011,0b11000000,
	 0b00001110,0b00001111,0b10000011,0b11000000,
	 0b00011110,0b00001111,0b11000011,0b11000000,
	 0b00011110,0b00001111,0b11000011,0b11000000,
	 0b00011110,0b00001111,0b11000011,0b11000000,
	 0b00001110,0b00001111,0b10000011,0b11000000,
	 0b00001110,0b00000000,0b00000011,0b11000000,
	 0b00001110,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00001111,0b00000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00000111,0b10000000,
	 0b00000111,0b10000000,0b00001111,0b00000000,
	 0b00000011,0b11000000,0b00001111,0b00000000,
	 0b00000011,0b11100000,0b00111110,0b00000000,
	 0b00000001,0b11111000,0b01111100,0b00000000,
	 0b00000000,0b11111111,0b11111100,0b00000000,
	 0b00000000,0b01111111,0b11110000,0b00000000,
	 0b00000000,0b00011111,0b11000000,0b00000000 } },

{ '1', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00011111,0b11000000,0b00000000,
	 0b00000001,0b11111111,0b11000000,0b00000000,
	 0b00000001,0b11111111,0b11000000,0b00000000,
	 0b00000001,0b11111111,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000111,0b11111111,0b11111111,0b11000000,
	 0b00000111,0b11111111,0b11111111,0b11000000,
	 0b00000111,0b11111111,0b11111111,0b11000000 } },

{ '2', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00111111,0b10000000,0b00000000,
	 0b00000001,0b11111111,0b11110000,0b00000000,
	 0b00000011,0b11111111,0b11111000,0b00000000,
	 0b00000111,0b11110001,0b11111100,0b00000000,
	 0b00001111,0b10000000,0b00111110,0b00000000,
	 0b00001110,0b00000000,0b00011110,0b00000000,
	 0b00000100,0b00000000,0b00011111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001110,0b00000000,
	 0b00000000,0b00000000,0b00011110,0b00000000,
	 0b00000000,0b00000000,0b00011110,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b01111100,0b00000000,
	 0b00000000,0b00000000,0b11111000,0b00000000,
	 0b00000000,0b00000000,0b11110000,0b00000000,
	 0b00000000,0b00000001,0b11100000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000111,0b10000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b01111100,0b00000000,0b00000000,
	 0b00000000,0b11111000,0b00000000,0b00000000,
	 0b00000001,0b11110000,0b00000000,0b00000000,
	 0b00000011,0b11100000,0b00000000,0b00000000,
	 0b00000111,0b11000000,0b00000000,0b00000000,
	 0b00001111,0b11111111,0b11111111,0b11000000,
	 0b00001111,0b11111111,0b11111111,0b11000000,
	 0b00001111,0b11111111,0b11111111,0b11000000 } },

{ '3', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00111111,0b11000000,0b00000000,
	 0b00000000,0b11111111,0b11110000,0b00000000,
	 0b00000011,0b11111111,0b11111100,0b00000000,
	 0b00001111,0b11110000,0b11111110,0b00000000,
	 0b00000111,0b10000000,0b00111111,0b00000000,
	 0b00000110,0b00000000,0b00011111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00011110,0b00000000,
	 0b00000000,0b00000000,0b00111110,0b00000000,
	 0b00000000,0b00000001,0b11111000,0b00000000,
	 0b00000000,0b00111111,0b11110000,0b00000000,
	 0b00000000,0b00111111,0b11000000,0b00000000,
	 0b00000000,0b00111111,0b11111000,0b00000000,
	 0b00000000,0b00000001,0b11111100,0b00000000,
	 0b00000000,0b00000000,0b00111110,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00001000,0b00000000,0b00000111,0b10000000,
	 0b00001110,0b00000000,0b00001111,0b10000000,
	 0b00011111,0b00000000,0b00011111,0b00000000,
	 0b00001111,0b11110001,0b11111110,0b00000000,
	 0b00000111,0b11111111,0b11111100,0b00000000,
	 0b00000001,0b11111111,0b11111000,0b00000000,
	 0b00000000,0b00111111,0b11000000,0b00000000 } },

{ '4', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000000,0b01111100,0b00000000,
	 0b00000000,0b00000000,0b11111100,0b00000000,
	 0b00000000,0b00000000,0b11111100,0b00000000,
	 0b00000000,0b00000001,0b11111100,0b00000000,
	 0b00000000,0b00000011,0b11111100,0b00000000,
	 0b00000000,0b00000111,0b10111100,0b00000000,
	 0b00000000,0b00001111,0b00111100,0b00000000,
	 0b00000000,0b00001111,0b00111100,0b00000000,
	 0b00000000,0b00011110,0b00111100,0b00000000,
	 0b00000000,0b00111100,0b00111100,0b00000000,
	 0b00000000,0b01111000,0b00111100,0b00000000,
	 0b00000000,0b11110000,0b00111100,0b00000000,
	 0b00000000,0b11110000,0b00111100,0b00000000,
	 0b00000001,0b11100000,0b00111100,0b00000000,
	 0b00000011,0b11000000,0b00111100,0b00000000,
	 0b00000111,0b10000000,0b00111100,0b00000000,
	 0b00001111,0b00000000,0b00111100,0b00000000,
	 0b00001111,0b00000000,0b00111100,0b00000000,
	 0b00011110,0b00000000,0b00111100,0b00000000,
	 0b00111111,0b11111111,0b11111111,0b11100000,
	 0b00111111,0b11111111,0b11111111,0b11100000,
	 0b00111111,0b11111111,0b11111111,0b11100000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000 } },

{ '5', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000011,0b11111111,0b11111111,0b00000000,
	 0b00000011,0b11111111,0b11111111,0b00000000,
	 0b00000011,0b11111111,0b11111111,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000011,0b10000000,0b00000000,0b00000000,
	 0b00000011,0b10000000,0b00000000,0b00000000,
	 0b00000011,0b10000000,0b00000000,0b00000000,
	 0b00000011,0b10011111,0b11100000,0b00000000,
	 0b00000111,0b11111111,0b11111000,0b00000000,
	 0b00000111,0b11111111,0b11111110,0b00000000,
	 0b00000111,0b11100000,0b01111111,0b00000000,
	 0b00000001,0b00000000,0b00011111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b11000000,
	 0b00000000,0b00000000,0b00000111,0b11000000,
	 0b00000000,0b00000000,0b00000111,0b11000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00001111,0b10000000,
	 0b00001100,0b00000000,0b00011111,0b00000000,
	 0b00011111,0b00000000,0b00111111,0b00000000,
	 0b00001111,0b11110001,0b11111110,0b00000000,
	 0b00000111,0b11111111,0b11111100,0b00000000,
	 0b00000001,0b11111111,0b11110000,0b00000000,
	 0b00000000,0b01111111,0b11000000,0b00000000 } },

{ '6', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00000000,0b00000111,0b11111000,0b00000000,
	 0b00000000,0b00011111,0b11111110,0b00000000,
	 0b00000000,0b01111111,0b11111111,0b10000000,
	 0b00000000,0b11111111,0b00111111,0b10000000,
	 0b00000001,0b11111000,0b00000011,0b00000000,
	 0b00000011,0b11100000,0b00000000,0b00000000,
	 0b00000011,0b11000000,0b00000000,0b00000000,
	 0b00000111,0b11000000,0b00000000,0b00000000,
	 0b00000111,0b10000000,0b00000000,0b00000000,
	 0b00000111,0b10000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b00000000,0b00000000,0b00000000,
	 0b00001111,0b00000111,0b11110000,0b00000000,
	 0b00001111,0b00011111,0b11111100,0b00000000,
	 0b00001111,0b01111111,0b11111111,0b00000000,
	 0b00001111,0b11111000,0b00011111,0b00000000,
	 0b00001111,0b11100000,0b00001111,0b10000000,
	 0b00001111,0b10000000,0b00000111,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00001111,0b00000000,0b00000011,0b11000000,
	 0b00000111,0b00000000,0b00000011,0b11000000,
	 0b00000111,0b10000000,0b00000011,0b11000000,
	 0b00000111,0b10000000,0b00000011,0b11000000,
	 0b00000011,0b11000000,0b00000111,0b10000000,
	 0b00000011,0b11100000,0b00001111,0b10000000,
	 0b00000001,0b11111000,0b01111111,0b00000000,
	 0b00000000,0b11111111,0b11111110,0b00000000,
	 0b00000000,0b00111111,0b11111000,0b00000000,
	 0b00000000,0b00001111,0b11100000,0b00000000 } },

{ '7', { 0b00000000,0b00000000,0b00000000,0b00000000,
	 0b00011111,0b11111111,0b11111111,0b11000000,
	 0b00011111,0b11111111,0b11111111,0b11000000,
	 0b00011111,0b11111111,0b11111111,0b11000000,
	 0b00000000,0b00000000,0b00000111,0b10000000,
	 0b00000000,0b00000000,0b00000111,0b00000000,
	 0b00000000,0b00000000,0b00001111,0b00000000,
	 0b00000000,0b00000000,0b00011110,0b00000000,
	 0b00000000,0b00000000,0b00111100,0b00000000,
	 0b00000000,0b00000000,0b00111000,0b00000000,
	 0b00000000,0b00000000,0b01111000,0b00000000,
	 0b00000000,0b00000000,0b11110000,0b00000000,
	 0b00000000,0b00000000,0b11110000,0b00000000,
	 0b00000000,0b00000001,0b11100000,0b00000000,
	 0b00000000,0b00000001,0b11100000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b11000000,0b00000000,
	 0b00000000,0b00000011,0b10000000,0b00000000,
	 0b00000000,0b00000111,0b10000000,0b00000000,
	 0b00000000,0b00000111,0b10000000,0b00000000,
	 0b00000000,0b00000111,0b10000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00001111,0b00000000,0b00000000,
	 0b00000000,0b00011111,0b00000000,0b00000000,
	 0b00000000,0b00011111,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000,
	 0b00000000,0b00011110,0b00000000,0b00000000 } },

{ 0 }
};

uint8_t *font_get_glyph_bitmap(char c)
{
	struct glyph_bitmap *gb = glyph_bitmaps;

	for (; gb->c; ++gb) {
		if (gb->c == c)
			return gb->bitmap;
	}
	return NULL;
}
