#pragma once
// serial_protocol.h

#ifndef _LED_TABLE_SERIAL_h
#define _LED_TABLE_SERIAL_h


#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif



#include <stdio.h>
#include <string.h>
#include "config.h"

#define DEBUG 0 

enum preDefined_messages_enum {
	msg_ACK = 10,
	msg_CMD_ACK = 11,
	msg_SPEC_DELAYTIME = 50,
	msg_NOT_ACK = 99,
	msg_CMD_NOT_ACK = 98,
	msg_ACTUALSTATE = 1

}pD_messages;
//template: 01_34_678_012_456_89

/* COMMAND CODES */
enum commands {
	cmd_setBlack = 0, //full dark
	cmd_setWhite = 1, //full light
	cmd_setRGB = 2, //set desired RGB value
	cmd_SPECIAL_set_rgbDELAY = 3, //template indices: 34 - cmd, 678 - delaytime
	cmd_goTo_Linear = 10,
	
	//go to desired RGB combination with specified steps in LINEAR SCALE
	cmd_goTo_Linear_WithStep_2 = 11, 
	cmd_goTo_Linear_WithStep_5 = 12,
	cmd_goTo_Linear_WithStep_10 = 13,
	cmd_goTo_Linear_WithStep_15 = 14,
	cmd_goTo_Linear_WithStep_30 = 15,
	cmd_goTo_Linear_WithStep_50 = 16,
	cmd_goTo_Linear_WithStep_65 = 17,
	cmd_goTo_Linear_WithStep_80 = 18,
	cmd_goTo_Linear_WithStep_100 = 19,

	//go to desired RGB combination with specified steps in SINE SCALE 
	cmd_goTo_Sine_WithStep_2 = 21, 
	cmd_goTo_Sine_WithStep_5 = 22,
	cmd_goTo_Sine_WithStep_10 = 23,
	cmd_goTo_Sine_WithStep_15 = 24,
	cmd_goTo_Sine_WithStep_30 = 25,
	cmd_goTo_Sine_WithStep_50 = 26,
	cmd_goTo_Sine_WithStep_65 = 27,
	cmd_goTo_Sine_WithStep_80 = 28,
	cmd_goTo_Sine_WithStep_100 = 29,

	// go to full light and go down to full dark and after that go again to original RGB combination in EXPONENTIAL SCALE
	cmd_goTo_Exp = 30,
	// go to desired RGB combination with specified steps in EXPONENTIAL SCALE 
	cmd_goTo_Exp_WithStep_2 = 31, 
	cmd_goTo_Exp_WithStep_5 = 32,
	cmd_goTo_Exp_WithStep_10 = 33,
	cmd_goTo_Exp_WithStep_15 = 34,
	cmd_goTo_Exp_WithStep_30 = 35,
	cmd_goTo_Exp_WithStep_50 = 36,
	cmd_goTo_Exp_WithStep_65 = 37,
	cmd_goTo_Exp_WithStep_80 = 38,
	cmd_goTo_Exp_WithStep_100 = 39,

	// waving on the full interval with sine wave and go back to original RGB combination
	cmd_wave_AllInterVal = 50, // a teljes intervallumon végighaladás szinuszos hullámmal, majd visszatérés az eredeti értékhez
	// waving around original RGB combination. 
	//You can specify the number of steps, RGB amplitude and number of waves
	cmd_wave_Step20_cyc1 = 51,
	cmd_wave_Step40_cyc1 = 52,
	cmd_wave_Step60_cyc1 = 53,
	cmd_wave_Step20_cyc2 = 54,
	cmd_wave_Step40_cyc2 = 55,
	cmd_wave_Step60_cyc2 = 56,
	cmd_wave_Step20_cyc3 = 57,
	cmd_wave_Step40_cyc3 = 58,
	cmd_wave_Step60_cyc3 = 59,
	/* get commands */
	cmd_get_ActualRGB = 91, //get actual RGB combination 
	cmd_get_SPEC_rgbDelayTime = 92, // get actual DELAY value (value in redLightValue field in received message) 
};

struct command_struct_t {
	int command;
	int redLightValue;
	int greenLightValue;
	int blueLightValue;
};

struct message_struct_t {
	int message;
	int redLightValue;
	int greenLightValue;
	int blueLightValue;
}message_struct;

struct rgbLights {
	int red;
	int green;
	int blue;
};

/*
*Protocol description
*  >> 00 Msg RedLight GreenLight BlueLight 00 <<
*/

class Serial_protocol {
private:

	char readvalue;
	unsigned int valueLength;
	//char readString[];
	String value;
	String startValue;
	String stopValue;
	command_struct_t cmd;
	message_struct_t msg;

	/* 
	* function for encoding  a 2 digit num to 2 character, which are transferable on serial port 
	*/
	void encode_2digits(uint8_t num, char &c_0, char &c_1) {
		uint8_t tens = num / 10;
		uint8_t ones = num - tens * 10;
		c_0 = char(tens + 48);
		c_1 = char(ones + 48);
			#if DEBUG
			Serial.print("te: "); Serial.print(tens); Serial.print(" on: "); Serial.print(ones); Serial.println();
			#endif
	}
	/*
	* function for encoding a 3 digits number to 3 character, which are transferable on serial port
	*/
	void encode_3digits(uint8_t num, char &c_0, char &c_1, char &c_2) {
		num = THRESHOLD(num, 0, 255);
		uint8_t hundreds = num / 100;
		uint8_t tens = (num - hundreds * 100) / 10;
		uint8_t ones = (num - hundreds * 100 - tens * 10);
		c_0 = char(hundreds + 48);
		c_1 = char(tens + 48);
		c_2 = char(ones + 48);
			#if DEBUG
			Serial.print("hu: "); Serial.print(hundreds); Serial.print("  te: "); Serial.print(tens); Serial.print("  on: "); Serial.print(ones); Serial.println();
			#endif
	}
	/*
	* function for encoding a 4 digits number to 4 character, which are transferable on serial port
	*/
	void encode_4digits(uint16_t num, char &c_0, char &c_1,
		char &c_2, char &c_3) {

		uint16_t thousands = num / 1000;
		uint16_t hundreds = (num - thousands * 1000) / 100;
		uint16_t tens = (num - thousands * 1000 - hundreds * 100) / 10;
		uint16_t ones = (num - thousands * 1000 - hundreds * 100 - tens * 10);
		c_0 = char(thousands + 48);
		c_1 = char(hundreds + 48);
		c_2 = char(tens + 48);
		c_3 = char(ones + 48);

	}




public:
	/*
	* INIT FUNCTION
	* I use it, becaouse serial port initialization must be in .ino file Setup() func. (because Setup() func. (in .ino file)  sets which part of 
	* uC will be on or off --> we need a global manager instance of this class, but serial port initialization must be in Setup() func. ) 
	*/
	void init() {
		Serial.begin(9600);
		delay(100);
		Serial.println("##### Good morning sir! #####");
	}
	/*
	* method for sending strings. NOT RECOMMENDED FOR USING, ONLY DEBUGGING. ANYBODY EAT THIS ON OTHER SIDE !!
	*/
	void sendString(char Msg[]) {
		Serial.println(Msg);
	}
	/*
	* Method for sending ACK.
	* In message field we can send back the RGB code, thus we can sign, which comand received and processed
	*/
	void sendACKwithValues(message_struct_t message) {
		message.message = msg_ACK;
		encodeAndSendMsg(message);
	}
	/*
	* Method for sending NOT ACK, if something nasty happened :-) 
	*/
	void sendNOTACK() {
		msg.message = msg_NOT_ACK;
		msg.redLightValue = 999;
		msg.greenLightValue = 999;
		msg.blueLightValue = 999;
		encodeAndSendMsg(msg);
	}
	/*
	* Method for sending predefined message 
	*/
	void sendMsg(int Msg) {
		message_struct_t msg_struct;
		msg_struct.message = Msg;
		msg_struct.redLightValue = 0;
		msg_struct.greenLightValue = 0;
		msg_struct.blueLightValue = 0;
		encodeAndSendMsg(msg_struct);
	}
	/*
	* Metódus üezenet és RGB értékek küldéséhez
	*/
	void send(message_struct_t message) {
		encodeAndSendMsg(message);
	}




	/*
	* Function for encoding and sending message to Serial Port
	* data stream protocol:
	* if indices: 01_34_678_012_456_89
	* then:
	*		"_"		separator character
	*		01  -	Start characters. Only the "0" and "1" characters are allowed in previous sequence.
	*		34  -	Message characters. These are deifine a message, which is understandable in other side if the protocol's header similar than this.
	*		678 -	RED light value
	*		012	-	GREEN light value
	*		456 -	BLUE light value
	*		89  -	Stop characters. Only the "1" and "0" characters are allowed in previous sequence.
	* A datastream's lenght must be 20 with every character. Otherwise it is not understandable in other side of Serial Port !
	*/

	void encodeAndSendMsg(message_struct_t msg_struct) {
		//msg indices:    01_34_678_012_456_89
		char message[] = "01_00_000_000_000_10";
		//encode msg 
		encode_2digits(msg_struct.message, message[3], message[4]);
		encode_3digits(msg_struct.redLightValue, message[6], message[7], message[8]);
		encode_3digits(msg_struct.greenLightValue, message[10], message[11], message[12]);
		encode_3digits(msg_struct.blueLightValue, message[14], message[15], message[16]);
		//send msg
		Serial.println(message);
	}



	/*
	* Method for receiving and decoding messages from Serial Port
	* Catch messsages only wich are corresponds the template
	* data stream template:
	* ha a cmd indices: 01_34_678_012_456_89
	* akkor:
	*		"_" -	separator character
	*		01  -	Start characters. Only the "0" and "1" characters are allowed in previous sequence.
	*		34  -	commands characters. These tell us, what is the message of computer, what is the command
	*		678 -	RED light value
	*		012	-	GREEN light value
	*		456 -	BLUE light value
	*		89  -	Stop characters. Only the "1" and "0" characters are allowed in previous sequence.
	* A datastream's lenght must be 20 with every character. Otherwise it is not understandable in other side of Serial Port !
	*
	*
	* return: decoded message
	* after we received and decoded the message, we send an ACK message with catched RGB code
	*/
	command_struct_t readAndDecodeCmd() {

		//cmd indices:    01_34_678_012_456_89
		if (Serial.available()) {
			value = Serial.readString();
			valueLength = value.length();
			Serial.println(value);

			//Check
			if (valueLength == 20) {
				startValue = (String)value[0] + (String)value[1];
				stopValue = (String)(value[valueLength - 2]) + (String)(value[valueLength - 1]);
				if ((startValue = "01") && (stopValue == "10")) {
					//Check OK

					cmd.command = ((uint8_t)(value[3]) - 48) * 10 + ((uint8_t)(value[4]) - 48);
					cmd.redLightValue = ((uint8_t)(value[6]) - 48) * 100 + ((uint8_t)(value[7]) - 48) * 10 + ((uint8_t)(value[8]) - 48);
					cmd.greenLightValue = ((uint8_t)(value[10]) - 48) * 100 + ((uint8_t)(value[11]) - 48) * 10 + ((uint8_t)(value[12]) - 48);
					cmd.blueLightValue = ((uint8_t)(value[14]) - 48) * 100 + ((uint8_t)(value[15]) - 48) * 10 + ((uint8_t)(value[16]) - 48);
				}


			}
			//Give parameters to ACK sender in msg structure, and send ACK, because we are happy :-)
			msg.redLightValue = cmd.redLightValue;
			msg.greenLightValue = cmd.greenLightValue;
			msg.blueLightValue = cmd.blueLightValue;
				#if DEBUG
				Serial.println(value);
				Serial.print("cmd_state: "); Serial.print(cmd.command); Serial.println();
				Serial.print("r: "); Serial.print(cmd.redLightValue); Serial.print(" g: "); Serial.print(cmd.greenLightValue); Serial.print(" b: "); Serial.print(cmd.blueLightValue);
				Serial.println();
				#endif	
			sendACKwithValues(msg);
		}
		return cmd;
	}
};

#endif