#include "debug.h"
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <unistd.h>

void uart_init(void)
{
#if ENABLE_DEBUG
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART1);

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	/* USART1_TX. */
	gpio_set_af(GPIOB, GPIO_AF7, GPIO6);

	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_enable(USART1);
#endif
}

/*
 * Write buffers destined for stdout to the UART initialised in uart_init(). 
 * This will forward the output of calls to printf() over the UART.
 */
int _write(int fd, const char *buf, int len)
{
#if ENABLE_DEBUG
	if (fd == STDOUT_FILENO) {
		int i = 0;
		for (; i < len; ++i) {
			if (buf[i] == '\n')
				usart_send_blocking(USART1, '\r');
			usart_send_blocking(USART1, buf[i]);
		}
		return i;
	}
#endif
	return -1;
}

void counter_init(void)
{
#if ENABLE_DEBUG
	rcc_periph_clock_enable(RCC_TIM5);

	/* See timer2_set_sampling_rate() for explanation of below 3 lines. */
	timer_set_prescaler(TIM5, ((rcc_apb1_frequency*2)/1e6)-1);
	timer_generate_event(TIM5, TIM_EGR_UG);
	timer_clear_flag(TIM5, TIM_EGR_UG);
	timer_enable_counter(TIM5);
#endif
}

uint32_t counter_count(void)
{
#if ENABLE_DEBUG
	return timer_get_counter(TIM5);
#else
	return 0;
#endif
}

