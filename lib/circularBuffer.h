/**
 * Circular Buffer
 *  Implemented as understood from the MES book section on Circular Buffers
 * page 177
*/

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include "stdint.h"
#include "stdlib.h"

typedef enum cb_status_t
{
	cb_status_t_OKAY,
	cb_status_BUFFER_FULL,
	cb_status_BUFFER_EMPTY
} cb_status_t;

typedef struct cb_t
{
	char *buffer;	//block of memory for data
	uint16_t size;	//must be power of 2
	uint16_t read;	// holds current read position (0 to (size-1))
	uint16_t write;	// holds current write postion (0 to (size-1))
} cb_t;

uint16_t cb_init(cb_t *cb,uint16_t size);
uint16_t cb_used(cb_t *cb);
cb_status_t cb_write(cb_t *cb, char *data);
cb_status_t cb_read(cb_t *cb,char *data);








#endif //_CIRCULAR_BUFFER_H_