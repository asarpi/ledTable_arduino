#include "config.h"
#include "LED_table_Serial.h"
#include "LED_light.h"
#include "RGB_Leds_manager.h"
#include <HardwareSerial.h>


LED_light LED_red(PIN_R);
LED_light LED_green(PIN_G);
LED_light LED_blue(PIN_B);
RGB_Leds Leds(LED_red, LED_green, LED_blue);


char CheckArray[3] = { 0,0,0 };
uint8_t CheckArrayIndex = 0;
uint8_t CheckingFlag = 0;
Serial_protocol SerialConsole;
uint8_t command;
command_struct_t command_struct;

void setup() {
	Leds.setOffAll();
	//LED_red.off();
	//LED_green.off();
	//LED_blue.off();
	SerialConsole.init();
	Leds.init(&SerialConsole);

	
}

void loop() {

	/*message_struct_t msg;
	msg.message = 10;
	msg.redLightValue = 50;
	msg.greenLightValue = 0;
	msg.blueLightValue = 100;
	SerialConsole.encodeAndSendMsg(msg);*/
	
	//Leds.manage_setCommands(command_struct);
	if (Serial.available()) {
		command_struct = SerialConsole.readAndDecodeCmd();
	//Serial.print("main"); Serial.print(command_struct.command); Serial.println();
		if (command_struct.command < 100) {
			if (Leds.manage_Commands(command_struct))
				SerialConsole.sendMsg(msg_CMD_ACK);
			else
				SerialConsole.sendMsg(msg_CMD_NOT_ACK);
		}
		else
			SerialConsole.sendMsg(msg_CMD_NOT_ACK);
	}

}



void loop_() { // run over and over

		/*
		if (readvalue == 'r')
			LED_red.on();
		if (readvalue == 'g')
			LED_green.on();
		if (readvalue == 'b')
			LED_blue.on();
		Serial.write(readvalue);*/
		//LED_red.on();
		//delay(500);
		/*
		for (int i = 0; i < 10; i++) {
			Serial.write('a');
			delay(500);
		}*/

	/*readvalue = Serial.read();
	Serial.write(readvalue);
	
	
		if (readvalue == 5) {
			Serial.write(readvalue);
			Serial.println("lofasz");
			LED_red.on();
			delay(100);
		}
		else
			LED_red.off();*/
	//}
	/*
	LED_red.off();
	LED_green.off();
	LED_blue.off();*/
}