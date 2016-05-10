#pragma once
#pragma once
#ifndef RGB_LEDS_MANAGER_H
#define RGB_LEDS_MANAGER_H
#include <Arduino.h>
#include <assert.h>
#include "led_light.h"
#include "config.h"
#include "serial_protocol.h"


#define DEBUG 0
#define DEBUG_SET_RGB 0
#define PI 3.1416
#define LN2 log(2)
/*
* RGB_Leds osztály
*	- RGB LED szalag fényerejének beállítása
*	- Serial porton keresztül történû parancsok értelmezése, és végrehajtása
*	- Különbözõ függvények szerinti átmenetek definiálása két RGB kombináció között
*	Jelenleg:
*		- lineáris
*		- szinuszos
*		- exponenciális
*		- impulzus

* IMPORTANT:   INIT SEQUENCE:
*	1. create an instance with activated RGB pins
*	2. call init() func. in .ino file's setup() func. after initialized Serial port
*/
class RGB_Leds {
private:
	LED_light RedLed;
	LED_light GreenLed;
	LED_light BlueLed;
	rgbLights rgb;			//this value contains only the actual setted light values. Only set function should using this.
	rgbLights rgb_future;
	byte delaytime;
	Serial_protocol* m_console;
	
	#if LIGHT_MAX_VALUE == 100
	/*
	* SET function for sending RGB combination to LED-strip
	* You should only this function for activate RGB combination on LED strip (for consistence and easy using)
	* Working only on [0,100] intervallum
	*/
	void set_100() {
		delay(delaytime);
		rgb.red = THRESHOLD(rgb.red, 0, LIGHT_MAX_VALUE);
		rgb.green = THRESHOLD(rgb.green, 0, LIGHT_MAX_VALUE);
		rgb.blue = THRESHOLD(rgb.blue, 0, LIGHT_MAX_VALUE);
		RedLed.setlight(rgb.red);
		GreenLed.setlight(rgb.green);
		BlueLed.setlight(rgb.blue);
			#if DEBUG_SET_RGB
			Serial.print("\t\t\t\tr:"); Serial.print(rgb.red);
			Serial.print(" g: "); Serial.print(rgb.green);
			Serial.print(" b: "); Serial.print(rgb.blue); Serial.println();
			#endif	
	}
	
	#else if LIGHT_MAX_VALUE == 255
	/*
	* SET function for sending RGB combination to LED-strip
	* You should only this function for activate RGB combination on LED strip (for consistence and easy using)
	* Working only on [0,255] intervallum
	* (example for usage: setOfAll() and setMaxLightAll() functions)
	*/
	void set() {
		delay(delaytime);
		rgb.red = THRESHOLD(rgb.red, 0, LIGHT_MAX_VALUE);
		rgb.green = THRESHOLD(rgb.green, 0, LIGHT_MAX_VALUE);
		rgb.blue = THRESHOLD(rgb.blue, 0, LIGHT_MAX_VALUE);
		RedLed.setlight_255(rgb.red);
		GreenLed.setlight_255(rgb.green);
		BlueLed.setlight_255(rgb.blue);
			#if DEBUG_SET_RGB
			Serial.print("\t\t\t\tr:"); Serial.print(rgb.red);
			Serial.print(" g: "); Serial.print(rgb.green);
			Serial.print(" b: "); Serial.print(rgb.blue); Serial.println();
			#endif	
	}
	#endif


	 /* GO TO FUNCTIONS */

	/*
	* go to desired RGB combination on LINEAR scale with desired step 
	*/
	void goToLinear(rgbLights rgb_new, byte step) {

		rgbLights rgb_original = rgb;
		rgbLights stepSizes;
		stepSizes.red = (float)(rgb_new.red - rgb_original.red) / (float)step;
		stepSizes.green = (float)(rgb_new.green - rgb_original.green) / (float)step;
		stepSizes.blue = (float)(rgb_new.blue - rgb_original.blue) / (float)step;
		for (byte i = 0; i <= step; i++) {
			rgb.red += stepSizes.red;
			rgb.green += stepSizes.green;
			rgb.blue += stepSizes.blue;
			set();
		}
		//rgb.red = rgb_new.red;
		//rgb.green = rgb_new.green;
		//rgb.blue = rgb_new.blue;
		//set();
	}
	
	/*
	* go to desired RGB combination on SINE scale with desired (K) step 
	* calculation base:
	*		light = light_original + (light_desired - light_original) * sin((k/K) * (pi/2))
	*/
	void goToSine(rgbLights rgb_new, float K) {
			#if DEBUG
			Serial.println("gotoSine");
			#endif
		float red_original = (float)rgb.red;
		float green_original = (float)rgb.green;
		float blue_original = (float)rgb.blue;
		float red_diff = (float)rgb_new.red - red_original;
		float green_diff = (float)rgb_new.green - green_original;
		float blue_diff = (float)rgb_new.blue - blue_original;
		float red;
		float green;
		float blue;
		float factor;

		for (float k = 0; k <= K; k++) {
			factor = sin((k / K) * (PI / 2));
			red = red_original + red_diff * factor;
			green = green_original + green_diff * factor;
			blue = blue_original + blue_diff * factor;
			rgb.red = (byte)red;
			rgb.green = (byte)green;
			rgb.blue = (byte)blue;
			set();
		}
	}
	/*
	* go to desired RGB combination on EXP  scale with desired (K) step 
	* calculation base: y = exp(x)-1
	*		x represents every step on [0, ln(2)] interval
	*		(-1) shifts down the exponential curve, thus the results will be on [0,1] interval
	* calculation equation:
	*		light = light_original + (light_desired - light_original) * (exp(x) - 1)
	*/

	void goToExp(rgbLights rgb_new, float K) {
#if DEBUG
		Serial.println("gotoExp");
#endif
		float red_original = (float)rgb.red;
		float green_original = (float)rgb.green;
		float blue_original = (float)rgb.blue;
		float red_diff = (float)rgb_new.red - red_original;
		float green_diff = (float)rgb_new.green - green_original;
		float blue_diff = (float)rgb_new.blue - blue_original;
		float red;
		float green;
		float blue;
		float factor;
		float delta = 0;

		for (float k = 0; k <= K; k++) {
			factor = exp(delta) - 1;
			delta += LN2 / K;
			red = red_original + red_diff * factor;
			green = green_original + green_diff * factor;
			blue = blue_original + blue_diff * factor;
			rgb.red = (byte)red;
			rgb.green = (byte)green;
			rgb.blue = (byte)blue;
			set();
		}
	}
	/*
	* waving around actual RGB configuration with specified amplitude, steps and cycle WITH SINE WAVE (exactly with goToSine function)
	*/
	void wave(rgbLights ampl, byte step, byte cyc) {
		rgbLights ampl_negative, ampl_positive, ampl_original;
		//calculate limits
		//original "0" value
		ampl_original.red = rgb.red;
		ampl_original.green = rgb.green;
		ampl_original.blue = rgb.blue;
		//negative peak
		ampl_negative.red = THRESHOLD(rgb.red - ampl.red / 2, 0, LIGHT_MAX_VALUE);
		ampl_negative.green = THRESHOLD(rgb.green - ampl.green / 2, 0, LIGHT_MAX_VALUE);
		ampl_negative.blue = THRESHOLD(rgb.blue - ampl.blue / 2, 0, LIGHT_MAX_VALUE);
		//positive peak
		ampl_positive.red = THRESHOLD(rgb.red + ampl.red / 2, 0, LIGHT_MAX_VALUE);
		ampl_positive.green = THRESHOLD(rgb.green + ampl.green / 2, 0, LIGHT_MAX_VALUE);
		ampl_positive.blue = THRESHOLD(rgb.blue + ampl.blue / 2, 0, LIGHT_MAX_VALUE);
		
		//create wave
		for (byte i = 0; i < cyc; i++) {
			//first quarter of period which direction is positive
			goToSine(ampl_positive, step / 4);
			//second and third quarter of period wich direction are negative 
			goToSine(ampl_negative, step / 2);
			//last quarter of period with positive direction again
			goToSine(ampl_original, step / 4);
		}

	}
public:

	/* 
	* MAIN INIT
	* initialization with activated RGB pins 
	*/
	RGB_Leds(LED_light R, LED_light G, LED_light B) {
		RedLed = R;
		GreenLed = G;
		BlueLed = B;
	}
	/*
	* SETUP INIT with initialized serial port
	*/
	void init(Serial_protocol* console) {
		m_console = console;

		delaytime = 25;
		rgb.red = 0;
		rgb.green = 0;
		rgb.blue = 0;
		//test after init
		rgb_future.red = LIGHT_MAX_VALUE / 4; rgb_future.green = 0; rgb_future.blue = 0;
		goToSine(rgb_future, 20);

		rgb_future.red = 0; rgb_future.green = LIGHT_MAX_VALUE / 4;
		goToSine(rgb_future, 20);

		rgb_future.green = 0; rgb_future.blue = LIGHT_MAX_VALUE / 4;
		goToSine(rgb_future, 20);

		rgb_future.blue = 0;
		goToSine(rgb_future, 20);

	}
	/*
	* Switch off all LED
	*/
	void setOffAll() {
		rgb.red = 0;
		rgb.green = 0;
		rgb.blue = 0;
		set();
	}
	/*
	* Shwitch on all LED with max light value 
	*/
	void setMaxLightAll() {
		rgb.red = LIGHT_MAX_VALUE;
		rgb.green = LIGHT_MAX_VALUE;
		rgb.blue = LIGHT_MAX_VALUE;
		set();
	}
	/*
	* get function
	*/
	rgbLights get_actualRGB() {
		return rgb;
	}


	/*
	* Method for managging set commands
	* simple commands:
	*	cmd_setBlack :				full dark
	*	cmd_setWhite :				full light
	*	cmd_setRGB	 :				set a desired RGB combination
	*	cmd_SPECIAL_set_rgbDELAY :	set DELAY value, which determine a base delay in set() func. (this will be determine the dynamics of all system)
	*/
	bool manage_set_commands(command_struct_t cmd) {
		#if DEBUG
		Serial.print("manage_set_commands"); Serial.print(cmd.command); Serial.println();
		#endif
		switch (cmd.command)
		{
		case cmd_setBlack:
			rgb.red = 0;
			rgb.green = 0;
			rgb.blue = 0;
			set();
			return true;
		case cmd_setRGB:
			rgb.red = cmd.redLightValue;
			rgb.blue = cmd.blueLightValue;
			rgb.green = cmd.greenLightValue;
			set();
			return true;
		case cmd_setWhite:
			rgb.red = 100;
			rgb.green = 100;
			rgb.blue = 100;
			set();
			return true;

		case cmd_SPECIAL_set_rgbDELAY:
			delaytime = cmd.redLightValue;
			// TODO: implementation of special command set delay
			return true;
		default:
			return false;
		}
	}
	/*
	* managger of goTO commands on LINEAR SCALE
	*/
	bool manage_goTo_lin_commands(command_struct_t cmd) {
		rgbLights rgb_new;
		switch (cmd.command)
		{
		case cmd_goTo_Linear_WithStep_2:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 2);
			return true;
		case cmd_goTo_Linear_WithStep_5:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 5);
			return true;
		case cmd_goTo_Linear_WithStep_10:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 10);
			return true;
		case cmd_goTo_Linear_WithStep_15:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 15);
			return true;
		case cmd_goTo_Linear_WithStep_30:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 30);
			return true;
		case cmd_goTo_Linear_WithStep_50:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 50);
			return true;
		case cmd_goTo_Linear_WithStep_65:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 65);
			return true;
		case cmd_goTo_Linear_WithStep_80:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 80);
			return true;
		case cmd_goTo_Linear_WithStep_100:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToLinear(rgb_new, 100);
			return true;
		default:
			return false;
		}
		return false;
	}
	/*
	* managger of GOTO commands on SINE SCALE
	*/
	bool manage_goto_sin_commands(command_struct_t cmd) {
		rgbLights rgb_new;
		switch (cmd.command)
		{
		case cmd_goTo_Sine_WithStep_2:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 2);
			return true;
		case cmd_goTo_Sine_WithStep_5:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 5);
			return true;
		case cmd_goTo_Sine_WithStep_10:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 10);
			return true;
		case cmd_goTo_Sine_WithStep_15:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 15);
			return true;
		case cmd_goTo_Sine_WithStep_30:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 30);
			return true;
		case cmd_goTo_Sine_WithStep_50:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 50);
			return true;
		case cmd_goTo_Sine_WithStep_65:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 65);
			return true;
		case cmd_goTo_Sine_WithStep_80:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 80);
			return true;
		case cmd_goTo_Sine_WithStep_100:
			rgb_new.red = cmd.redLightValue;
			rgb_new.green = cmd.greenLightValue;
			rgb_new.blue = cmd.blueLightValue;
			goToSine(rgb_new, 100);
			return true;
		default:
			return false;
		}
	}
	/*
	* managger of GOTO commands on EXP SCALE
	*/
	bool manage_goto_exp_commands(command_struct_t cmd) {
		byte step = 10;
		switch (cmd.command)
		{
		case cmd_goTo_Exp:
			rgbLights original;
			original.red = rgb.red; original.green = rgb.green; original.blue = rgb.blue;
			rgb_future.red = 0; rgb_future.green = 0; rgb_future.blue = 0;
			goToExp(rgb_future, 10);

			rgb_future.red = LIGHT_MAX_VALUE; rgb_future.green = LIGHT_MAX_VALUE; rgb_future.blue = LIGHT_MAX_VALUE;
			goToExp(rgb_future, 40);

			rgb_future.red = 0; rgb_future.green = 0; rgb_future.blue = 0;
			goToExp(rgb_future, 40);

			goToExp(original, 10);

			return true;
		case cmd_goTo_Exp_WithStep_2:	step = 2; break;
		case cmd_goTo_Exp_WithStep_5:	step = 5; break;
		case cmd_goTo_Exp_WithStep_10:	step = 10; break;
		case cmd_goTo_Exp_WithStep_15:	step = 15; break;
		case cmd_goTo_Exp_WithStep_30:	step = 30; break;
		case cmd_goTo_Exp_WithStep_50:	step = 50; break;
		case cmd_goTo_Exp_WithStep_65:	step = 65; break;
		case cmd_goTo_Exp_WithStep_80:	step = 80; break;
		case cmd_goTo_Exp_WithStep_100:	step = 100; break;
		default:
			return false;
		}
		rgb_future.red = cmd.redLightValue;
		rgb_future.green = cmd.greenLightValue;
		rgb_future.blue = cmd.blueLightValue;
		goToExp(rgb_future, step);
		return true;
	}


	/*
	* managger of PULSATION or WAVE commands
	*/
	bool manage_pulsation_commands(command_struct_t cmd) {
		byte step = 40;
		byte cyc = 1;
		switch (cmd.command)
		{
		case cmd_wave_AllInterVal:
			rgbLights original;
			original.red = rgb.red; original.green = rgb.green; original.blue = rgb.blue;
			rgb_future.red = 0; rgb_future.green = 0; rgb_future.blue = 0;
			goToSine(rgb_future, 10);

			rgb_future.red = LIGHT_MAX_VALUE; rgb_future.green = LIGHT_MAX_VALUE; rgb_future.blue = LIGHT_MAX_VALUE;
			goToSine(rgb_future, 40);

			rgb_future.red = 0; rgb_future.green = 0; rgb_future.blue = 0;
			goToSine(rgb_future, 40);

			goToSine(original, 10);

			return true;
		case cmd_wave_Step20_cyc1: step = 20; cyc = 1; break;
		case cmd_wave_Step40_cyc1: step = 40; cyc = 1; break;
		case cmd_wave_Step60_cyc1: step = 60; cyc = 1; break;
		case cmd_wave_Step20_cyc2: step = 20; cyc = 2; break;
		case cmd_wave_Step40_cyc2: step = 40; cyc = 2; break;
		case cmd_wave_Step60_cyc2: step = 60; cyc = 2; break;
		case cmd_wave_Step20_cyc3: step = 20; cyc = 3; break;
		case cmd_wave_Step40_cyc3: step = 40; cyc = 3; break;
		case cmd_wave_Step60_cyc3: step = 60; cyc = 3; break;
		default:
			return false;
		}
		rgb_future.red = cmd.redLightValue;
		rgb_future.green = cmd.greenLightValue;
		rgb_future.blue = cmd.blueLightValue;
		wave(rgb_future, step, cyc);
		return true;
	}
	/*
	* managger of getState commands
	*	cmd_get_ActualRGB:			responds ACTUALSTATE message and RGB combination
	*	cmd_get_SPEC_rgbDelayTime:	responds SPEC_DELAYTIME message and actual DELAY value in milisec on redLightValue field. Other fields will be zeros 
	*/
	bool manage_getState_commands(command_struct_t cmd) {
		#if DEBUG
		Serial.println("manage_getState_commands");
		#endif	
		switch (cmd.command)
		{
		case cmd_get_ActualRGB:
			#if  DEBUG
			Serial.println("cmd_get_ActualRGB");
			#endif
			message_struct_t msg;
			msg.message = msg_ACTUALSTATE;
			msg.redLightValue = rgb.red;
			msg.greenLightValue = rgb.green;
			msg.blueLightValue = rgb.blue;
			m_console->send(msg);
			return true;
		case cmd_get_SPEC_rgbDelayTime:
			msg.message = msg_SPEC_DELAYTIME;
			msg.redLightValue = delaytime;
			msg.greenLightValue = 0;
			msg.blueLightValue = 0;
			m_console->send(msg);
			return true;
		default:
			return false;
		}
		return false;
	}
	/*
	* method for managging cmds. Shorting and call the perfect managger method
	*/
	bool manage_Commands(command_struct_t cmd) {
		#if DEBUG
		Serial.print("manage_Commands"); Serial.print(cmd.command); Serial.println();
		#endif
		if (cmd.command < 10) {
			return manage_set_commands(cmd);
		}
		if (cmd.command >= 10 && cmd.command < 20) {
			return manage_goTo_lin_commands(cmd);
		}
		if (cmd.command >= 20 && cmd.command < 30) {
			return manage_goto_sin_commands(cmd);
		}
		if (cmd.command >= 30 && cmd.command < 40) {
			return manage_goto_exp_commands(cmd);
		}
		if (cmd.command >= 50 && cmd.command < 60) {
			return manage_pulsation_commands(cmd);
		}
		if (cmd.command >= 90 && cmd.command < 100) {
			return manage_getState_commands(cmd);
		}
		if (cmd.command >= 100) {
			return false;
		}
		return false;
	}
};
#endif