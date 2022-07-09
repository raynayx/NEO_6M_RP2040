/***
 * Middleware for UART peripheral on RP2040
 * 
 * 
 * 
 * 
 * 
***/

#ifndef _UART_MW_H_
#define _UART_MW_H_

#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <hardware/irq.h>


void uart_setup(uart_inst_t *u, uint baudrate);
void uart_irq_setup(uart_inst_t *u,uint irq_num,irq_handler_t handler,bool rx,bool tx);


#endif //_UART_MW_H_
