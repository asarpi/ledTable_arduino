#pragma once
#ifndef RGB_LEDS_MANAGER_H
#define RGB_LEDS_MANAGER_H
#include <Arduino.h>
#include <assert.h>
#include "LED_light.h"
#include "config.h"
#include "LED_table_Serial.h"


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
*/
class RGB_Leds {
private:
	LED_light RedLed;
	LED_light GreenLed;
	LED_light BlueLed;
	rgbLights rgb;
	rgbLights rgb_future;
	byte delaytime;
	Serial_protocol* m_console;
	#if LIGHT_MAX_VALUE == 100
	/*
	* SET függvény a [0,100] intervallum használatához
	* MINDIG ezen keresztül történik a LED-szalag aktuális fényerejének beállítása. Ennek használatával konzisztens marad mindig az osztály,
	* és egyszerûbb a használata. 
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
	* SET függvény a [0,255]-ös intervallum használatához
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
	/*
	* goToLinear a lineáris sákálán való eléréséhez a kívánt RGB kombinációnak (K) lépésben
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
	* goToSine a kívánt RGB kombináció eléréséhez a kívánt (K) lépésben a szinuszos skálán
	* a számítás a következõképpen történik:
	*		light = light_original + (light_desired - light_original) * sing((k/K) * (pi/2))
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
			factor = sin((k/K) * (PI/2));
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
	 * goToExp függvény a kívánt RGB kombináció eléréséhez a kívánt lépésszámban (K) az exponenciális skálán
	 * az exponenciális számítás alapja: y = exp(x)-1
	 *		x-et a [0, ln(2)] intervallumon való lineáris lépkedés adja. 
	 *		a (-1) -es taggal eltoljuk lefelé az exponenciális görbét, így az exp(x) érték pont a [0,1] intervallumra fog kerülni
	 * a számítás képlete:
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
			factor = exp(delta)-1;
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
	* Az ingadozás megvalóítása az aktuális RGB konfiguráció körül a meghatározott amplítudóval a meghatározott lépésszámmal, és ciklusszámban
	* Az ingadozás a szinuszos skálán történik, így azt a függvényt is hívjuk
	*/
	void wave(rgbLights ampl, byte step, byte cyc) {
		rgbLights ampl_negative, ampl_positive, ampl_original;
		//eredeti "0" érték
		ampl_original.red = rgb.red;
		ampl_original.green = rgb.green;
		ampl_original.blue = rgb.blue;
		//negatív csúcsérték
		ampl_negative.red = THRESHOLD(rgb.red - ampl.red / 2, 0, LIGHT_MAX_VALUE);
		ampl_negative.green = THRESHOLD(rgb.green - ampl.green / 2, 0, LIGHT_MAX_VALUE);
		ampl_negative.blue = THRESHOLD(rgb.blue - ampl.blue / 2, 0, LIGHT_MAX_VALUE);
		//pozitív csúcsérték
		ampl_positive.red = THRESHOLD(rgb.red + ampl.red / 2, 0, LIGHT_MAX_VALUE);
		ampl_positive.green = THRESHOLD(rgb.green + ampl.green / 2, 0, LIGHT_MAX_VALUE);
		ampl_positive.blue = THRESHOLD(rgb.blue + ampl.blue / 2, 0, LIGHT_MAX_VALUE);

		for (byte i = 0; i < cyc; i++) {
			//pozitív negyedperiódus
			goToSine(ampl_positive, step / 4);
			//negatív félperiódus
			goToSine(ampl_negative, step / 2);
			//pozitív negyedperiódus
			goToSine(ampl_original, step / 4);
		}

	}
public:
	//MAIN INIT
	RGB_Leds(LED_light R, LED_light G, LED_light B) {
		RedLed = R;
		GreenLed = G;
		BlueLed = B;
	}
	/*SETUP INIT
	* !!! FONTOS !!! meg kell adni a setup() fv-ben a már elõre inicializált Serial_Protokol példányát, aminek segítségével tudunk
	* a definiált protkoll szerint szabványosan komunikálni
	*/
	void init(Serial_protocol* console){
		m_console = console;

		delaytime = 25;
		rgb.red = 0;
		rgb.green = 0;
		rgb.blue = 0;
		
		rgb_future.red = LIGHT_MAX_VALUE/4; rgb_future.green = 0; rgb_future.blue = 0;
		goToSine(rgb_future, 20);
		
		rgb_future.red = 0; rgb_future.green = LIGHT_MAX_VALUE/4;
		goToSine(rgb_future, 20);
		
		rgb_future.green = 0; rgb_future.blue = LIGHT_MAX_VALUE/4;
		goToSine(rgb_future, 20);
		
		rgb_future.blue = 0;
		goToSine(rgb_future, 20);

	}
	/*
	* Összes LED kikapcsolása
	*/
	void setOffAll() {
		rgb.red = 0;
		rgb.green = 0;
		rgb.blue = 0;
		set();
	}
	/*
	* Összes LED bekapcsolása maximum fényerõvel
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
	* SET commandok managelésére szolgáló metódus
	* ezek csak egyszerû szín beállítások
	* cmd_setBlack :			teljes sötétség
	* cmd_setWhite :			maximum világosság
	* cmd_setRGB :				egy egyedi RGB szín
	* cmd_SPECIAL_set_rgbDELAY :az RGB jelek LED-szallagra való kikerülésének idejét meghatározó érték beállítása 
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
			return true;
		default:
			return false;
		}
	}
	/*
	* goTo_lin commandok lekezelése: ezeknek a parancsoknak a parancs nevében meghatározott lépésszámban eléri a kívánt RGB színt
	* lineáris skálán haladva
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
	* goTo_sin parancsok lekezelése: a kívánt színt a kívánt lépésszámmal érjük el szinusos skálán haladva lépésenként
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
	* goTo_sin parancsok lekezelése: a kívánt színt a kívánt lépésszámmal érjük el exponenciális skálán haladva lépésenként
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
	* pulsation parancsok lekezelése. Ezeknek a parancsoknak hatására a kívánt RGB amplitúdó kombinációnak megfelelõen szinuszosan ingadozik a 
	* fényerõ az éppen aktuális érték körül. A kívánt lépésszám, illetve az ingadozások száma elõre definiálva van a commandokban
	* a cmd_pulsation_AllInterval parancs hatására a LED-szalag elsötétedik, majd szinuszosan kivilágosodik a maximum értékekig, majd teljesen
	*elsötétedik, ezután visszaáll arra az RGB kombinációra, ahol a mûvelet elõtt volt.
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
	* getState parancsok kezelése
	* cmd_get_ActualRGB:			a válaszban ACTUALSTATE üzenettel együtt az aktuális RGB értékek fognak érkezni
	* cmd_get_SPEC_rgbDelayTime:	a válaszban SPEC_DELAYTIME üzenetel együtt az aktuálisan meghatározott DELAY ideje
	*								fog érkezni ms-ben a redLLightValue mezõben, a többi mezõ 0 lesz
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
	* Metódus a cmd-k managelésére. A beérkezett commandokat szétválogatjuk, és a különbözõ csoportokba tartozókat
	* különbözõ metódusok kezelik
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