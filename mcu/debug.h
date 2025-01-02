/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/* 
 * Whether to intialise debugging facilities (the UART and counter). 
 * Turn this off when not debugging because their initialised clocks waste power.
 */
#define ENABLE_DEBUG 0

/*
 * Initialise 115,200 8N1 UART transfer on pin PB6, USART1_TX.
 * After which calls to printf() will log its output over this UART transmit line.
 */
void uart_init(void);

/* Initialise a counter that counts up from 0 at a 1 MHz rate. */
void counter_init(void);
/* Get the current value of the counter. */
uint32_t counter_count(void);

#endif
