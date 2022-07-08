/************************
 * 
 * 
 * 
 * 
 * 
 * 
 * *********************/

#ifndef _NEO_6M_H_
#define _NEO_6M_H_

#include <stdint.h>
#include <hardware/gpio.h>
#include <hardware/uart.h>


#define NEO_GPS_UART_BUFFER_SIZE 256
#define NEO_GPS_WORKING_BUFFER_SIZE 128

typedef struct sTimeDate_t
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint16_t year;
} sTimeDate_t;


typedef struct sLocation_t
{
	float latitude;
	char latitudeDirection;
	float longitude;
	char longitudeDirection;
	float altitude;
	char positionValid;
	double speed_km;
	double speed_knots;
	double course;
	double magnetic_declination;
}sLocation_t;

typedef struct sNEO_6M_state
{
	uart_inst_t *huart;
	uint8_t UartBufferHead;
	uint8_t UartBufferTail;
	uint8_t UartBufferLines;


	uint8_t satelite_number;
	uint8_t quality;
	uint8_t fix_mode;
	double dop; 	//dilution of precision
	double h_dop; // horizontal dilution of precision
	double v_dop; // vertiacal dilution of precision
	double altitude;
	char position_valid;
} sNEO_6M_state;

void NEO_6M_Init(sNEO_6M_state *gps,sLocation_t *l, sTimeDate_t *t, uart_inst_t *huart);
void NEO_6M_ReceiveUartChar(sNEO_6M_state *gps);
void NEO_6M_Task(sNEO_6M_state *gps, sLocation_t *l, sTimeDate_t *t);

uint8_t NEO_6M_IsFix(sNEO_6M_state *gps);
uint8_t NEO_6M_FixMode(sNEO_6M_state *gps);

void NEO6_6M_ParseLine(sNEO_6M_state *gps,sLocation_t *l,sTimeDate_t *t);

void uart_setup(uart_inst_t *u,uint baudrate, bool interrupt_enable);
void uart_rx_cb();


float NEO_6M_GpsToDecimalDegrees(float nmeaPos, char quadrant);

#endif //_NEO_6M_H_