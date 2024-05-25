/*
 * Driver for the SSD1306 OLED display controller.
 */
#ifndef SSD1306_H	
#define SSD1306_H	

#include "2d_bit_array.h"

#define GDDRAM_PIXEL_WIDTH 128
#define GDDRAM_PIXEL_HEIGHT 64

/*
 * The slave address is of format 0b011110<SA0> where <SA0> is
 * the slave address bit and gives SSD1306_I2C_SLAVE_ADDR_HIGH (0b0111101)
 * if high, and SSD1306_I2C_SLAVE_ADDR_LOW (0b0111100) if low.
 */
enum ssd1306_i2c_slave_address {
	SSD1306_I2C_SLAVE_ADDR_LOW  = 0x3C,
	SSD1306_I2C_SLAVE_ADDR_HIGH = 0x3D
};

/*
 * The SSD1306 controller on my display interfaces with the MCU via I2C. 
 * Initialise I2C controller I2C1 to use pins PB8 as I2C1_SCL and PB9 as 
 * I2C1_SDA for I2C communication with the SSD1306.
 *
 * @addr: which address to use will typically be detailed on the retailer's product page for the 
 *	  display bought
 */
void ssd1306_init_i2c(enum ssd1306_i2c_slave_address addr);
/* 
 * Initialise the SSD1306 controller and power on the display proper. 
 * Ensure to call ssd1306_init_i2c() before this.
 */
void ssd1306_init(void);

/*
 * Fill the whole 128 bits wide by 64 bits high Graphic Display Data RAM (GDDRAM) backing 
 * the display with the data in the GDDRAM MCU side buffer.
 *
 * Ensure to fill the GDDRAM MCU side buffer with the gddram_mcu_buf_*() functions below
 * with what you want displayed before calling this.
 */
void ssd1306_fill_gddram(void);

/* 
 * Functions to write to the GDDRAM MCU side 2D bit array buffer below. 
 *
 * Param write_coord is the coordinate of the MCU side buffer that the top left corner
 * of the 2D bit array will get written to
 */

/* Zero the entire GDDRAM MCU side buffer. */
void gddram_mcu_buf_zero(void);
/* Write a ncols by nrows 2D bit array to the GDDRAM MCU side buffer. */
void gddram_mcu_buf_write_2d_bit_array(uint8_t *bit_array, int ncols, int nrows,
				       struct write_coord coord);
/* Text font is the font implemented in file 'font.h'. */
void gddram_mcu_buf_write_text(const char *text, struct write_coord coord);
/* Lines are 1 pixel thick. */
void gddram_mcu_buf_write_horizontal_line(struct write_coord coord, int length);
void gddram_mcu_buf_write_vertical_line(struct write_coord coord, int height);

#endif
