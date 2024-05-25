/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 *
 * Glyph bitmaps for the Source Code Pro (monospace) font.
 */
#ifndef FONT_H
#define FONT_H

#include <stdint.h>

/* 
 * The glyph bitmap for a font is 29x32 pixels but padded to 32 bits
 * wide to make the width a multiple of a byte so that a pixel can be
 * represented by a single bit rather than a byte, in order to save space.
 */
#define FONT_PIXEL_WIDTH  29
#define FONT_PIXEL_HEIGHT 32
#define FONT_PIXEL_WIDTH_PAD 32

/*
 * Get the glyph bitmap of a character. Supported characters are 
 * note names 'A', 'B', 'C', 'D', 'E', 'F', 'G', sharp '#', question
 * mark '?', and note numbers '0', '1', '2', '3', '4', '5', '6', '7'.
 * Return NULL if the character is unsupported.
 * 
 * See the bitmap arrays in font.c:glyph_bitmaps for the format
 * of the return.
 */
uint8_t *font_get_glyph_bitmap(char c);

#endif
