#include <hardware/clocks.h>
#include <hardware/regs/rosc.h>
#include <hardware/regs/clocks.h>
#include <hardware/regs/m0plus.h>
#include <hardware/structs/rosc.h>
#include <hardware/structs/scb.h>
#include <hardware/structs/clocks.h>
#include "power.h"

/* 
 * Whether to keep enabled the clocks needed for serial logging over USB.
 * Only use this when debugging, and especially ensure to turn this off 
 * when running the pico off a battery in order to save power.
 * TODO guard all prints and init stdio in the program off this? if so move
 * this define to power.h or its own header file?
 */
#define ENABLE_USB_DEBUG 1

/* From pico-extras. */
static void rosc_disable(void) 
{
	uint32_t ctrl = rosc_hw->ctrl;
	ctrl &= ~ROSC_CTRL_ENABLE_BITS;
	ctrl |= (ROSC_CTRL_ENABLE_VALUE_DISABLE << ROSC_CTRL_ENABLE_LSB);
	rosc_hw->ctrl = ctrl;
	while (rosc_hw->status & ROSC_STATUS_STABLE_BITS)
		;
}

void power_save_configure_clocks(void)
{
	/*
	 * Assume all clock generators except the clk_gpout[0-3] generators are already on and
	 * are being run off the XOSC clock source (either directly or indirectly via a PLL), 
	 * and that all clock sources are on.
	 */

	/* Turn off all unused clock generators. */
	clock_stop(clk_rtc);
	clock_stop(clk_peri);
#if !ENABLE_USB_DEBUG
	clock_stop(clk_usb);
#endif

	/* 
	 * Can't turn off the PLL clocks because pll_usb is required by clk_adc and pll_sys by 
	 * clk_sys. 
	 */

	/* Turn off unused clock sources. */
	rosc_disable();

	/* Only allow ADC clock while sleeping. */
	clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_SYS_ADC_BITS|CLOCKS_SLEEP_EN0_CLK_ADC_ADC_BITS;
	clocks_hw->sleep_en1 = CLOCKS_SLEEP_EN1_CLK_SYS_TIMER_BITS;  /* Needed for multicore synchronisation. */
#if ENABLE_USB_DEBUG
	clocks_hw->sleep_en1 |= CLOCKS_SLEEP_EN1_CLK_SYS_USBCTRL_BITS|CLOCKS_SLEEP_EN1_CLK_USB_USBCTRL_BITS;
#endif
}

void power_save_enable_core_deep_sleep(void)
{
	scb_hw->scr |= M0PLUS_SCR_SLEEPDEEP_BITS;
}

