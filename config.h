#pragma once

//program constants
#define DELAY 1


//LED constants
#define PWM_MAX_VALUE 255
#define PWM_MIN_VALUE 0
#define LIGHT_MAX_VALUE 255
#define LIGHT_MIN_VALUE 0

/*
Note: Bekötéstõl, és LED-szalagtól függõen megcserélõdhet a G és a B

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