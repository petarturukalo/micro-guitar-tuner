/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 *
 * Operations on 2D bit arrays. 2D bit arrays are used rather than 2D byte
 * arrays in order to save space.
 */
#ifndef BIT_ARRAY_2D_H
#define BIT_ARRAY_2D_H

#include <stdint.h>
#include <stdbool.h>

#define BITS_IN_BYTE 8

/*
 * Get an 8-bit bitmask with only the bit at index bit_index high (0-indexed).
 * E.g. the bitmask for bit index 0 is 0b1000,0000, and the bitmask for bit 
 * index 7 is 0b0000,0001.
 */
uint8_t bit_index_to_8bit_bitmask(int bit_index);

/*
 * Set the bit at row i, column j of the 2D bit array to value.
 * Note ncols must be a multiple of BITS_IN_BYTE.
 */
void bit_array_2d_set(uint8_t *bit_array, int ncols, int nrows, int i, int j, bool value);

/* 
 * Coordinate in the destination 2D bit array that the top left corner of the source
 * 2D bit array will get written to.
 */
struct write_coord {
	int row;
	int col;
};

/*
 * Copy a ncols by nrows source 2D bit array to a destination 2D bit array.
 * Note ncols must be a multiple of BITS_IN_BYTE.
 */
void bit_array_2d_copy(uint8_t *dest_bit_array, int dest_ncols, int dest_nrows,
		       uint8_t *src_bit_array, int src_ncols, int src_nrows,
		       struct write_coord coord);

#endif
