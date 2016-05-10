#pragma once

//program constants
#define DELAY 1


//LED constants
#define PWM_MAX_VALUE 255	//maximum PWM value 
#define PWM_MIN_VALUE 0		// minimum PWM value
#define LIGHT_MAX_VALUE 255 //maximum allowed light value (sometimes we can set an other value, if we want to control the algortihm with 0-100 interval (percent) )
#define LIGHT_MIN_VALUE 0	//minimum allowed light value

/*
Note: R and G pin depends on wiring or type of LED strip

*/
#define PIN_R 6
#define PIN_G 9 
#define PIN_B 5
#define PIN_BUILTIN 13


//MACROS

//Tresholds
#define THRESHOLD_MAX(A,MAX) (A>MAX) ? (MAX) : (A)
#define THRESHOLD_MIN(A,MIN) (A<MIN) ? (MIN) : (A)
#define THRESHOLD(A,MIN,MAX) (A>MAX) ? (MAX) : ( (A<MIN) ? (MIN) : (A) )