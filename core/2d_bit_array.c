#include "2d_bit_array.h"

uint8_t bit_index_to_8bit_bitmask(int bit_index)
{
	return 1<<(BITS_IN_BYTE-bit_index-1);
}

/*
 * Get a pointer to the byte that the bit at row i, column j resides in, within the 2D bit array.
 */
static uint8_t *bit_array_2d_get_byte(uint8_t *bit_array, int ncols, int i, int j)
{
	int column_byte_index = j/BITS_IN_BYTE;
	int nbyte_cols = ncols/BITS_IN_BYTE;
	return bit_array+(nbyte_cols*i)+column_byte_index;
}

/*
 * Get an 8-bit bitmask of the byte the bit at column j resides in, but with only the column j bit high.
 */
static uint8_t bit_array_2d_get_col_8bit_bitmask(int j)
{
	return bit_index_to_8bit_bitmask(j%BITS_IN_BYTE);
}

/*
 * Get the bit at row i, column j of the 2D bit array.
 */
static bool bit_array_2d_get(uint8_t *bit_array, int ncols, int nrows, int i, int j)
{
	uint8_t *byte = bit_array_2d_get_byte(bit_array, ncols, i, j);
	uint8_t bitmask = bit_array_2d_get_col_8bit_bitmask(j);
	return *byte & bitmask;
}

void bit_array_2d_set(uint8_t *bit_array, int ncols, int nrows, int i, int j, bool value)
{
	if ((i >= 0 && i < nrows) && (j >= 0 && j < ncols)) {
		uint8_t *byte = bit_array_2d_get_byte(bit_array, ncols, i, j);
		uint8_t bitmask = bit_array_2d_get_col_8bit_bitmask(j);

		if (value)
			*byte |= bitmask;
		else
			*byte &= ~bitmask;
	}
}

void bit_array_2d_copy(uint8_t *dest_bit_array, int dest_ncols, int dest_nrows,
		       uint8_t *src_bit_array, int src_ncols, int src_nrows,
		       struct write_coord coord)
{
	for (int i = 0; i < src_nrows; ++i) {
		for (int j = 0; j < src_ncols; ++j) {
			bool bit = bit_array_2d_get(src_bit_array, src_ncols, src_nrows, i, j);
			bit_array_2d_set(dest_bit_array, dest_ncols, dest_nrows,
					 coord.row+i, coord.col+j, bit);
		}
	}
}

