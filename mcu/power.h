#ifndef POWER_H
#define POWER_H

/*
 * Configure clocks to save power: switch off unused clock sources 
 * and clock generators, and only allow the ADC clock while sleeping 
 * as its interrupt is needed to wake up the sampling core.
 *
 * Ensure to also call enable_core_deep_sleep() on each core.
 */
void power_save_configure_clocks(void);
void power_save_enable_core_deep_sleep(void);

#endif
