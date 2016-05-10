#pragma once
// LED_light.h

#include "config.h"

#ifndef _LED_LIGHT_h
#define _LED_LIGHT_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/*
* LED_ligth Class:
*	interface class for base operations on a LED
*		- initialization of LED's pin as PWM output
*		- switch on (full light) or off the light
*		- set a lightvalue
*			- on [0,100] interval with linearization
*			- on [0,255] interval with linearization
*			- on [0,255] interval without any operation
*/
class LED_light {
private:
	uint8_t s_led_pin;

	float temp_linearization;
	uint8_t duty;
	/*
	* LED is a non-linear device --> light is increasing with duty's third root --> we need the desired value's third power for compensate it
	* We work on [0,1] interval
	*/
	uint16_t linearizator(uint16_t inputvalue) {
		temp_linearization = (float)inputvalue / 100; //dividy by 100 --> we working on [0,1] interval 
		return (uint8_t)(pow(temp_linearization, 3) * 255); //we get the value's third power and multiply with 255 (becaouse duty is on [0,255] interval)
	}
	uint8_t linearizator_in255interval(uint16_t inputvalue) {
	//	inputvalue =
			return (uint8_t)(pow((float)inputvalue / 255, 3) * 255);
	}

public:
	/* INIT FUNCTION
	* switch on LED pin as PWM output 
	*/
	LED_light(byte ledpin) {
		s_led_pin = ledpin;
		pinMode(s_led_pin, OUTPUT);
	}
	/*
	* init an abstarct LED instance. This isn't contain any real pin's config while we don't set it
	*/
	LED_light() {}

	/*
	* function for setting light value on [0,100] interval with linearization
	*/
	void setlight(uint16_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, 100);
		analogWrite(s_led_pin, linearizator(lightvalue));
	}
	/*
	* function for setting light value on [0,255] interval 
	*/
	void setlight_255(uint8_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, LIGHT_MAX_VALUE);
		analogWrite(s_led_pin, linearizator_in255interval(lightvalue));
	}
	/*
	* function forsetting light on original way (on [0,255] interval without linearization
	*/
	void setlight_old(uint8_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, 255);
		analogWrite(s_led_pin, lightvalue);
	}

	/*
	* whitch on the LED with full light
	*/
	void on() {
		setlight(LIGHT_MAX_VALUE);
	}

	/*
	* whitch off the LED
	*/
	void off() {
		setlight(LIGHT_MIN_VALUE);
	}

};



#endif

