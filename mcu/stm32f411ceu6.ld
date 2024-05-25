/*
 * This was copied from the example at top of file libopencm3/lib/cortex-m-generic.ld, 
 * but with the length of the memory regions changed to match the flash and RAM sizes of 
 * the STM32F411CEU6 MCU.
 */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 512K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

INCLUDE libopencm3/lib/cortex-m-generic.ld
