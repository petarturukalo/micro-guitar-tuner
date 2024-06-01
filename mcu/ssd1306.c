/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 *
 * This source implements the Solomon Systech SSD1306 datasheet,
 * linked to in the "Resources" section of the top-level README.
 */
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "ssd1306.h"
#include "font.h"
#include "2d_bit_array.h"

static uint32_t ssd1306_i2c_controller = I2C1;
static enum ssd1306_i2c_slave_address ssd1306_addr;

void ssd1306_init_i2c(enum ssd1306_i2c_slave_address addr)
{
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Configure pin PB8 as I2C1_SCL and pin PB9 as I2C1_SDA. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO8|GPIO9);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO8|GPIO9);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO8|GPIO9);
	gpio_set(GPIOB, GPIO8|GPIO9);
	
	i2c_set_speed(ssd1306_i2c_controller, i2c_speed_fm_400k, rcc_apb1_frequency/1e6);
	i2c_peripheral_enable(ssd1306_i2c_controller);

	ssd1306_addr = addr;
}

/*
 * The non-I2C related payload specific to the SSD1306 is typically an interleaving 
 * of control byte and then data/command byte. 
 */
struct control_byte {
	uint8_t unused : 6;  /* This shall be zeroed. */
	enum {
		CTL_NEXT_BYTE_CMD = 0,  /* Next byte in the payload is a command byte. */
		CTL_NEXT_BYTE_DATA = 1  /* Next byte in the payload is a data byte that will be stored at the GDDRAM. */
	} next_byte : 1;  
	enum {
		CTL_CONTINUATION_DATA = 0,        /* All following bytes until the stop signal are data bytes. */
		CTL_CONTINUATION_INTERLEAVED = 1  /* Interleaving of control byte and data/command byte. */
	} continuation : 1;
} __attribute__((packed));

/*
 * Issue a command byte to the SSD1306 controller. 
 */
static void _ssd1306_send_cmd(uint8_t cmd)
{
	struct {
		struct control_byte ctl;
		uint8_t cmd;
	} payload;

	payload.ctl.unused = 0;
	payload.ctl.next_byte = CTL_NEXT_BYTE_CMD;
	payload.ctl.continuation = CTL_CONTINUATION_INTERLEAVED;
	payload.cmd = cmd;

	i2c_transfer7(ssd1306_i2c_controller, ssd1306_addr, (uint8_t *)&payload, sizeof(payload), NULL, 0);
}

enum ssd1306_cmd {
	SSD1306_CMD_SET_MEM_ADDR_MODE           = 0x20,
	SSD1306_CMD_SET_CONTRAST                = 0x81,
	SSD1306_CMD_CHARGE_PUMP_SETTING         = 0x8D,
	SSD1306_CMD_REVERSE_SEGMENTS            = 0xA1,
	SSD1306_CMD_SET_DISPLAY_ON_OFF          = 0xAE,
	SSD1306_CMD_REVERSE_COM_OUTPUT_SCAN_DIR = 0xC8,
	SSD1306_CMD_SET_DISPLAY_CLOCK           = 0xD5
};

/*
 * Issue a command to the SSD1306 controller. A command can optionally have arguments,
 * either encoded in unused bits of the command identifier itself (id_encoded_args), or 
 * encoded in their own command bytes sent after the initial command identifying byte (extra_args).
 */
static void ssd1306_send_cmd(enum ssd1306_cmd cmd, uint8_t id_encoded_args, 
			     uint8_t *extra_args, int extra_args_len)
{
	_ssd1306_send_cmd(cmd|id_encoded_args);

	for (int i = 0; i < extra_args_len; ++i) 
		_ssd1306_send_cmd(extra_args[i]);
}

enum ssd1306_memory_addressing_mode {
	SSD1306_MEM_ADDR_MODE_HORIZONTAL = 0b00,
	SSD1306_MEM_ADDR_MODE_VERTICAL   = 0b01,
	SSD1306_MEM_ADDR_MODE_PAGE       = 0b10
};

static void ssd1306_set_memory_addressing_mode(enum ssd1306_memory_addressing_mode mem_addr_mode)
{
	uint8_t arg = mem_addr_mode;
	ssd1306_send_cmd(SSD1306_CMD_SET_MEM_ADDR_MODE, 0, &arg, 1);
}

static void ssd1306_reverse_com_output_scan_direction(void)
{
	ssd1306_send_cmd(SSD1306_CMD_REVERSE_COM_OUTPUT_SCAN_DIR, 0, NULL, 0);
}

static void ssd1306_reverse_segments(void)
{
	ssd1306_send_cmd(SSD1306_CMD_REVERSE_SEGMENTS, 0, NULL, 0);
}

/* Contrast increases as the contrast param increases. */
static void ssd1306_set_contrast(uint8_t contrast)
{
	ssd1306_send_cmd(SSD1306_CMD_SET_CONTRAST, 0, &contrast, 1);
}

/* 
 * Set display clock divider and oscillator (clock source) frequency. 
 * Both params are 4-bit numbers, not 8-bit.
 */
static void ssd1306_set_display_clock(uint8_t clock_divider, uint8_t osc_freq)
{
	struct {
		uint8_t clock_divider : 4;
		uint8_t osc_freq : 4;
	} __attribute__((packed)) args;

	args.clock_divider = clock_divider;
	args.osc_freq = osc_freq;

	ssd1306_send_cmd(SSD1306_CMD_SET_DISPLAY_CLOCK, 0, (uint8_t *)&args, 1);
}

/* False param means disable. */
static void ssd1306_enable_charge_pump(bool enable)
{
	struct {
		uint8_t unused1 : 2;
		uint8_t enable_charge_pump : 1;
		uint8_t unused2 : 1;
		uint8_t high_bit : 1;  /* Must be high. */
		uint8_t unused3 : 3;
	} __attribute__((packed)) arg;

	memset(&arg, 0, sizeof(arg));
	arg.enable_charge_pump = enable;
	arg.high_bit = 1;

	ssd1306_send_cmd(SSD1306_CMD_CHARGE_PUMP_SETTING, 0, (uint8_t *)&arg, 1);
}

/* False param means off. */
static void ssd1306_set_display_on(bool on)
{
	ssd1306_send_cmd(SSD1306_CMD_SET_DISPLAY_ON_OFF, on, NULL, 0);
}

void ssd1306_init(void)
{
	ssd1306_set_memory_addressing_mode(SSD1306_MEM_ADDR_MODE_HORIZONTAL);
	/* 
	 * The below two commands together flip the screen as if you had physically turned the 
	 * display 180 degrees. 
	 */
	ssd1306_reverse_com_output_scan_direction();
	ssd1306_reverse_segments();

	/* Lower contrast and clock to save power (use less current). */
	ssd1306_set_contrast(1);  /* Although this is the lowest visible contrast setting it's still quite bright. */
	ssd1306_set_display_clock(0, 0);  /* Slowest clock which doesn't produce screen flickering. */

	/* Clear screen before it gets turned on. */
	gddram_mcu_buf_zero();
	ssd1306_fill_gddram();
	
	ssd1306_enable_charge_pump(true);
	ssd1306_set_display_on(true);
}


#define GDDRAM_MCU_BUF_LEN 1024  /* Bytes in 128x64 bits. */
#define GDDRAM_MCU_BUF_NCOLS 16  /* Bytes in 128 bits. */
#define GDDRAM_MCU_BUF_NROWS  8  /* Bytes in 64 bits. */

#define GDDRAM_NPAGES 8
#define GDDRAM_NROWS_IN_PAGE 8

static uint8_t gddram_mcu_buf[GDDRAM_MCU_BUF_LEN];

/*
 * Transpose the gddram_mcu_buf 2D bit array into a data stream gddram_mcu_buf_transposed that
 * when sent to the SSD1306 GDDRAM configured for horizontal addressing mode will show up on 
 * the display as the original 2D bit array.
 *
 * See SSD1306 datasheet section 8.7 for info on the GDDRAM and section 10.1.3 for info 
 * on horizontal addressing mode.
 */
static void gddram_mcu_buf_transpose(uint8_t *gddram_mcu_buf, uint8_t *gddram_mcu_buf_transposed) 
{
	/* Pointers to bytes vertically adjacent in a page. */
	uint8_t *rows[GDDRAM_NROWS_IN_PAGE];
	int w = 0;  /* Write index. */
	
	for (int i = 0; i < GDDRAM_NPAGES; ++i) {
		for (int j = 0; j < GDDRAM_MCU_BUF_NCOLS; ++j) {
			if (j == 0) {
				/* Initialise to point to the start byte of each row in the page. */
				rows[0] = gddram_mcu_buf;
				for (int k = 1; k < GDDRAM_NROWS_IN_PAGE; ++k)
					rows[k] = rows[k-1]+GDDRAM_MCU_BUF_NCOLS;
			}
			/* 
			 * Each column of bits in the column byte becomes a transposed byte with
			 * top row bit the LSB and bottom row bit the MSB.
			 */
			for (int bit_index = 0; bit_index < BITS_IN_BYTE; ++bit_index) {
				uint8_t transposed = 0;
				uint8_t bitmask = bit_index_to_8bit_bitmask(bit_index);
				for (int k = 0; k < GDDRAM_NROWS_IN_PAGE; ++k)  {
					if (*rows[k]&bitmask)
						transposed |= 1<<k;
				}
				gddram_mcu_buf_transposed[w++] = transposed;
			}
			/* Shift to the next byte column. */
			for (int k = 0; k < GDDRAM_NROWS_IN_PAGE; ++k)
				rows[k] += 1;
		}
		/* Move to start of next page. */
		gddram_mcu_buf += GDDRAM_MCU_BUF_NCOLS*GDDRAM_NROWS_IN_PAGE;
	}
}

void ssd1306_fill_gddram(void)
{
	static struct {
		struct control_byte ctl;
		uint8_t gddram_mcu_buf_transposed[GDDRAM_MCU_BUF_LEN];
	} payload;

	payload.ctl.unused = 0; 
	payload.ctl.next_byte = CTL_NEXT_BYTE_DATA; 
	payload.ctl.continuation = CTL_CONTINUATION_DATA;
	gddram_mcu_buf_transpose(gddram_mcu_buf, payload.gddram_mcu_buf_transposed);

	i2c_transfer7(ssd1306_i2c_controller, ssd1306_addr, (uint8_t *)&payload, sizeof(payload), NULL, 0);
}

void gddram_mcu_buf_zero(void)
{
	memset(gddram_mcu_buf, 0, GDDRAM_MCU_BUF_LEN);
}

void gddram_mcu_buf_write_2d_bit_array(uint8_t *bit_array, int ncols, int nrows,
				       struct write_coord coord)
{
	bit_array_2d_copy(gddram_mcu_buf, GDDRAM_PIXEL_WIDTH, GDDRAM_PIXEL_HEIGHT,
			  bit_array, ncols, nrows, coord);
}

void gddram_mcu_buf_write_text(const char *text, struct write_coord coord)
{
	char c;

	while (c = *text++) {
		uint8_t *bitmap = font_get_glyph_bitmap(c);
		if (bitmap) {
			gddram_mcu_buf_write_2d_bit_array(bitmap, FONT_PIXEL_WIDTH_PAD, FONT_PIXEL_HEIGHT,
							  coord);
		}
		coord.col += FONT_PIXEL_WIDTH;
	}
}

void gddram_mcu_buf_write_horizontal_line(struct write_coord coord, int length)
{
	for (int i = 0; i < length; ++i) {
		bit_array_2d_set(gddram_mcu_buf, GDDRAM_PIXEL_WIDTH, GDDRAM_PIXEL_HEIGHT,
				 coord.row, coord.col+i, 1);
	}
}

void gddram_mcu_buf_write_vertical_line(struct write_coord coord, int height)
{
	for (int i = 0; i < height; ++i) {
		bit_array_2d_set(gddram_mcu_buf, GDDRAM_PIXEL_WIDTH, GDDRAM_PIXEL_HEIGHT,
				 coord.row+i, coord.col, 1);
	}
}

