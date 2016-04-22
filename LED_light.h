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
Note: Mivel PWM jelet küldünk ki, ezért már 1-es értéknél bekapcsolnak a LED-ek
*/

class LED_light {
private:
	uint8_t s_led_pin;
	
	float temp_linearization;
	uint8_t duty;
	/*
	A LED egy nemlineáris eszköz. Ergo a világosság nem arányosan növekszik a kiadott értékkel, hanem köbgyökösen.
	Ezért jó öreg köb függvényünket segítségül hívva kompenzáljuk ezt.
	*/
	uint16_t linearizator(uint16_t inputvalue) {
		temp_linearization = (float)inputvalue / 100; //leosztjuk 100-al, így már a [0,1] intervallumon dolgozunk
		return (uint8_t)(pow(temp_linearization, 3)*255); //köbgyök, és a teljes tartomány annyiszorosát vesszük
	}
	uint8_t linearizator_in255interval(uint16_t inputvalue) {
		inputvalue = 
		return (uint8_t)(pow((float)inputvalue / 255, 3) * 255);
	}

public:
	LED_light(byte ledpin) {
		s_led_pin = ledpin;
		pinMode(s_led_pin, OUTPUT);
	}
	LED_light() {} //csak akkor szabad így inicializálni, ha valamikor valahogy mégis beállítjuk a LED PINjét kimenetként
	/*
	Ezzel a fügvénnyel küldjük ki a megfelelõ világosságértékhez tartozó PWM kitöltési tényezõt.
	A kívánt világosság értéket 0-100 intervallumon várjuk
	*/
	void setlight(uint16_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, 100);
		analogWrite(s_led_pin, linearizator(lightvalue));
	}
	/*
	* [0,255] intervallumon való világosság átadás
	*/
	void setlight_255(uint8_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, LIGHT_MAX_VALUE);
		analogWrite(s_led_pin, linearizator_in255interval(lightvalue));
	}
	/*
	A hagyományos úton történõ kitöltési tényezõ átadás (csak átadjuk a kitöltési tényezõt)
	[0,255] közötti értéket esz meg
	*/
	void setlight_old(uint8_t lightvalue) {
		lightvalue = THRESHOLD(lightvalue, 0, 255);
		analogWrite(s_led_pin, lightvalue);
	}
	void on() {
		setlight(LIGHT_MAX_VALUE);
	}

	void off() {
		setlight(LIGHT_MIN_VALUE);
	}

};



#endif

