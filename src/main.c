
#include <hardware/uart.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "neo_6m.h"



bool gps_run_r_cb(struct repeating_timer *timer);
bool gps_task_r_cb(struct repeating_timer *timer_1);

extern sNEO_6M_state g;



//This struct is used to keep track of the timer by the ```add_repeating_timer_ms``` function
struct repeating_timer timer;
struct repeating_timer timer_1;


int main()
{
	stdio_init_all();
	
	NEO_6M_Init(&g,uart0);
	add_repeating_timer_ms(5000,gps_run_r_cb,NULL,&timer);
	add_repeating_timer_us(0,gps_task_r_cb,NULL,&timer_1);

	while (1)
	{
		for(uint64_t i = 0; i < UINT64_MAX; i++);
	}
	return 0;
}

bool gps_run_r_cb(struct repeating_timer *timer)
{
	if(NEO_6M_IsFix(&g) && NEO_6M_FixMode(&g))
	{
		printf("Time: %02d:%02d:%02d\t",g.hour,g.minute,g.second);
		printf("Location: %.6f %.6f\n",NEO_6M_GpsToDecimalDegrees(g.latitude,g.latitudeDirection),
		NEO_6M_GpsToDecimalDegrees(g.longitude,g.longitudeDirection));
	}
	else
	{
		printf("No fix\n");
	}

	return true;
}

bool gps_task_r_cb(struct repeating_timer *timer_1)
{
	NEO_6M_Task(&g);
	return true;
}