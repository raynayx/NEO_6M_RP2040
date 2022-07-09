#include "uart_mw.h"

void uart_setup(uart_inst_t *u, uint baudrate)
{
	#ifdef _BOARDS_SEEED_XIAO_RP2040_H
		uart_init(u,115200);  //init UART 0
		gpio_set_function(0,GPIO_FUNC_UART); //set GPIO 0 to UART TX found on D6
		gpio_set_function(1,GPIO_FUNC_UART); //set GPIO 1 to UART RX found on D7
	#else //Pico and Pico compatible
		uart_init(u,baudrate);  //init UART 0
		gpio_set_function(0,GPIO_FUNC_UART); //set GPIO 0 to UART TX
		gpio_set_function(1,GPIO_FUNC_UART); //set GPIO 1 to UART RX
	#endif
}

void uart_irq_setup(uart_inst_t *u,uint irq_num,irq_handler_t handler,bool rx,bool tx)
{
	irq_set_exclusive_handler(irq_num,handler);
	irq_set_enabled(irq_num,true);
	uart_set_irq_enables(u,true,false);
}