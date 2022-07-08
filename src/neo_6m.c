
#include "neo_6m.h"
#include "string.h"
#include <stdlib.h>
#include <hardware/irq.h>

char neo_gps_UART_buffer[NEO_GPS_UART_BUFFER_SIZE];
char neo_gps_working_buffer[NEO_GPS_WORKING_BUFFER_SIZE];\
sNEO_6M_state g;
volatile char UartReceivedChar;

/*
 * Behaves like strtok() except that it returns empty tokens also.
 * Found on https://stackoverflow.com/questions/42315585/split-string-into-tokens-in-c-when-there-are-2-delimiters-in-a-row
 */
static char* strtoke(char *str, const char *delim)
{
  static char *start = NULL; /* stores string str for consecutive calls */
  char *token = NULL; /* found token */
  /* assign new start in case */
  if (str) start = str;
  /* check whether text to parse left */
  if (!start) return NULL;
  /* remember current start as found token */
  token = start;
  /* find next occurrence of delim */
  start = strpbrk(start, delim);
  /* replace delim with terminator and move start to follower */
  if (start) *start++ = '\0';
  /* done */
  return token;
}

void NEO_6M_ReceiveUartChar(sNEO_6M_state *g)
{
	uint8_t TempHead;

	TempHead = (g->UartBufferHead + 1) % NEO_GPS_UART_BUFFER_SIZE;

	if(TempHead == g->UartBufferTail)
	{
		//handle error
	}
	else
	{
		if(UartReceivedChar == 13)
		{
			g->UartBufferLines++;
			g->UartBufferHead = TempHead;
			neo_gps_UART_buffer[TempHead] = UartReceivedChar;
		}
		else if((UartReceivedChar == 0) || (UartReceivedChar == 10))
		{
			//ignore byte 0 and 10
		}
		else
		{
			g->UartBufferHead = TempHead;
			neo_gps_UART_buffer[TempHead] = UartReceivedChar;
		}
	}
}


int NEO_6M_GetCharFromBuffer(sNEO_6M_state *g)
{
	if(g->UartBufferHead == g->UartBufferTail)
	{
		return -1; //error no char to return
	}
	g->UartBufferTail = (g->UartBufferTail + 1) % NEO_GPS_UART_BUFFER_SIZE;
	return neo_gps_UART_buffer[g->UartBufferTail];
}


int NEO_6M_GetLineFromBuffer(sNEO_6M_state *g)
{
	char TempChar;
	char* LinePointer = (char*)neo_gps_working_buffer;

	if(g->UartBufferLines)
	{
		while((TempChar = NEO_6M_GetCharFromBuffer(g)))
		{
			if(TempChar == 13)
			{
				break;
			}
			*LinePointer = TempChar;
			LinePointer++;
		}
		*LinePointer = 0;
		g->UartBufferLines--;
	}
	return 0;
}


void NEO_6M_ParseGPRMC(sTimeDate_t *t,sLocation_t *l)
{
	// eg1. $GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62
	// eg2. $GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68

	char *parsePointer;
	uint32_t Temp;

	// Time of FIX
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		Temp = atoi(parsePointer);
		t->second = Temp % 100;
		t->minute = (Temp / 100) % 100;
		t->hour = (Temp / 10000) % 100;
	}
	// Navigation receiver warning A = OK, V = warning
	parsePointer = strtoke(NULL, ",");
	// Latitude
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->latitude = atof(parsePointer)/100.;
	}
	// Latitude Direction
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->latitudeDirection = *parsePointer;
	}
	// Longnitude
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->longitude = atof(parsePointer)/100.;
	}
	// Longnitude Direction
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->longitudeDirection = *parsePointer;
	}
	// Speed over ground, Knots
	parsePointer = strtoke(NULL, ",");
	// Course Made Good, True
	parsePointer = strtoke(NULL, ",");
	// Date of fix
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		Temp = atoi(parsePointer);
		t->year = Temp % 100;
		t->month = (Temp / 100) % 100;
		t->day = (Temp / 10000) % 100;
	}
}

//
//	Track Made Good and Ground Speed.
//
void NEO_6M_ParseGPVTG(sLocation_t *l)
{
	// eg1. $GPVTG,360.0,T,348.7,M,000.0,N,000.0,K*43
	// eg2. $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K

	char *parsePointer;

	// True track made good
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
		{
			l->course = atof(parsePointer);
		}
	parsePointer = strtoke(NULL, ",");
	// Magnetic track made good
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
			{
				l->magnetic_declination = atof(parsePointer);
			}
	parsePointer = strtoke(NULL, ",");
	// Ground speed, knots
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->speed_knots = atof(parsePointer);
	}
	parsePointer = strtoke(NULL, ",");
	// Ground speed, Kilometers per hour
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		l->speed_km = atof(parsePointer);
	}
}

//
//	Global Positioning System Fix Data
//
void NEO_6M_ParseGPGGA(sNEO_6M_state *gps)
{
	// eg. $GPGGA,212846.00,5025.81511,N,01639.92090,E,1,04,4.72,281.1,M,42.0,M,,*5F

	char *parsePointer;

	// UTC of Position
	parsePointer = strtoke(NULL, ",");
	// Latitude
	parsePointer = strtoke(NULL, ",");
	// N or S
	parsePointer = strtoke(NULL, ",");
	// Longitude
	parsePointer = strtoke(NULL, ",");
	// E or W
	parsePointer = strtoke(NULL, ",");
	// GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->quality = atoi(parsePointer);
	}
	// Number of satellites in use [not those in view]
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->satelite_number = atoi(parsePointer);
	}
	// Horizontal dilution of position
	parsePointer = strtoke(NULL, ",");
	// Antenna altitude above/below mean sea level (geoid)
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->altitude = atof(parsePointer);
	}
}

//
//	GPS DOP and active satellites
//
void NEO_6M_ParseGPGSA(sNEO_6M_state *gps)
{
	// eg1. $GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C
	// eg2. $GPGSA,A,3,19,28,14,18,27,22,31,39,,,,,1.7,1.0,1.3*35

	char *parsePointer;

	// Mode
	parsePointer = strtoke(NULL, ",");
	// 2D/3D Fix
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->fix_mode = atoi(parsePointer);
	}
	// IDs of SVs used in position fix (null for unused fields)
	for(uint8_t i=0; i < 12; i++)
	{
		parsePointer = strtoke(NULL, ",");
	}
	// PDOP
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->dop = atof(parsePointer);
	}
	// HDOP
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->h_dop = atof(parsePointer);
	}
	// VDOP
	parsePointer = strtoke(NULL, ",");
	if(strlen(parsePointer) > 0)
	{
		gps->v_dop = atof(parsePointer);
	}
}

//
// GPS Satellites in view
//
void NEO_6M_ParseGPGSV(sNEO_6M_state *gps)
{
	// eg. $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
	//     $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
	//     $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D

	// Todo
	// Do I need to check satellites data?
}

//
// Geographic Position, Latitude / Longitude and time.
//
void NEO_6M_ParseGPGLL(sNEO_6M_state *gps)
{
	//$GPGLL, 1111.11, a, yyyyy.yy, a, hhmmss.ss, A*hh < CR><LF>
	char *parsePointer;

		// UTC of Position
		parsePointer = strtoke(NULL, ",");
		// Latitude
		parsePointer = strtoke(NULL, ",");
		// N or S
		parsePointer = strtoke(NULL, ",");
		// Longitude
		parsePointer = strtoke(NULL, ",");
		// E or W
		parsePointer = strtoke(NULL, ",");
		// GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
		parsePointer = strtoke(NULL, ",");
		if(strlen(parsePointer) > 0)
		{
			gps->position_valid = *parsePointer;
		}

}


void NEO_6M_ParseLine(sNEO_6M_state *gps, sLocation_t *l, sTimeDate_t *t)
{
	//
	// Nice website with NMEA commuincates description
	//	http://aprs.gids.nl/nmea
	//

	// Header
	char* parsePointer = strtoke((char*)neo_gps_working_buffer, ",");

	if(strcmp(parsePointer, "$GPRMC") == 0) NEO_6M_ParseGPRMC(t,l);
	// else if(strcmp(parsePointer, "$GPVTG") == 0) NEO_6M_ParseGPVTG(l);
	else if(strcmp(parsePointer, "$GPGGA") == 0) NEO_6M_ParseGPGGA(gps);
	else if(strcmp(parsePointer, "$GPGSA") == 0) NEO_6M_ParseGPGSA(gps);
//	else if(strcmp(parsePointer, "$GPGSV") == 0) NEO6_ParseGPGSV(gps);		//not implemented
 	// else if(strcmp(parsePointer, "$GPGLL") == 0) NEO_6M_ParseGPGLL(gps);
}

uint8_t NEO_6M_IsFix(sNEO_6M_state *gps)
{
	return gps->quality;
}

uint8_t NEO_6M_FixMode(sNEO_6M_state *gps)
{
	return gps->fix_mode;
}
void NEO_6M_Task(sNEO_6M_state *gps, sLocation_t *l, sTimeDate_t *t)
{
	if(gps->UartBufferLines)
	{
		NEO_6M_GetLineFromBuffer(gps);
		NEO_6M_ParseLine(gps,l, t);
	}
}


void NEO_6M_Init(sNEO_6M_state *gps,sLocation_t *l,sTimeDate_t *t,uart_inst_t *huart)
{
	gps->huart = huart;
	gps->UartBufferHead = 0;
	gps->UartBufferTail = 0;
	gps->UartBufferLines = 0;
	uart_setup(gps->huart,9600,true);


	t->second = 0;
	t->minute = 0;
	t->hour = 0;
	t->day = 0;
	t->month = 0;
	t->year = 0;

	l->latitude = 0.f;
	l->latitudeDirection = '0';
	l->longitude = 0.f;
	l->longitudeDirection = '0';
	l->positionValid = 'V';

	l->speed_km = 0;
	l->speed_knots = 0;

	l->course = 0;
	l->magnetic_declination = 0;

	gps->satelite_number = 0;
	gps->quality = 0;
	gps-> dop = 0;
	gps->h_dop = 0;
	gps->v_dop = 0;

	NEO_6M_ReceiveUartChar(gps);
}


void uart_setup(uart_inst_t *u,uint baudrate, bool interrupt_enable)
{	
	#ifdef _BOARDS_SEEED_XIAO_RP2040_H
		uart_init(u,115200);  //init UART 0
		gpio_set_function(0,GPIO_FUNC_UART); //set GPIO 0 to UART TX found on D6
		gpio_set_function(1,GPIO_FUNC_UART); //set GPIO 1 to UART RX found on D7
	#else
		uart_init(u,baudrate);  //init UART 0
		gpio_set_function(0,GPIO_FUNC_UART); //set GPIO 0 to UART TX
		gpio_set_function(1,GPIO_FUNC_UART); //set GPIO 1 to UART RX
	#endif

	if(interrupt_enable)
	{
		irq_set_exclusive_handler(UART0_IRQ,uart_rx_cb);
		irq_set_enabled(UART0_IRQ,true);
		uart_set_irq_enables(uart0,true,false);
	}
}

void uart_rx_cb()
{
	while(uart_is_readable(uart0))
	{
		UartReceivedChar = uart_getc(uart0);
		NEO_6M_ReceiveUartChar(&g);
	}
}


/**
 * Convert NMEA absolute position to decimal degrees
 * "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
 * then negated if quadrant is 'W' or 'S'
 * https://stackoverflow.com/a/63363808
 * 
 * The above solution is normal
 * This receiver however seems to do the conversion but for the quadrant
 * hence just negate the W and S quadrant values
 */
float NEO_6M_GpsToDecimalDegrees(float nmeaPos, char quadrant)
{
  	float v = nmeaPos;
 	// uint temp =(uint)nmeaPos;

    // v = (float)temp + (nmeaPos - (float)temp)/60.f;
    if(quadrant =='W' || quadrant =='S')
	{
		v = -v;
	}

  return v;
}