// RFID_HF_BUS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <csignal>
#include "RfidTbenHFBus.h"
extern "C"{
	#include <modbus.h>
}

#define SLEEP_TIME 500ms
#define SLEEP(a) std::this_thread::sleep_for(a)

#define DLENTH 22


using namespace std::chrono_literals;

int display_options();

void signalHandler(int signum) {
	std::cout << "Interrupt signal (" << signum << ") received.\n";
	exit(signum);
}

int main()
{
	signal(SIGINT, signalHandler);
	int ch = 0;
	char charInput = 0;
	uint16_t tagPresentwhere;

	RfidTben_hf mock2;

	std::cout << "This is sample test program for TBEN_S2_2RFID_4DXP\n";
	std::cout << "Connecting via modbus to TBEN_S2_2RFID_4DXP\n";
	if (mock2.connectModbus("192.168.1.35") != 0) {
		std::cout << "Connection failed. Please check hardware and resolve error. Exiting program\n";
	}
	else {
		std::cout << "Connected.\n"; 
		
		do
		{
			std::cout << "What's the channel connected to rfid reader?\n";
			if (!(std::cin >> ch)) {//error occurred
				std::cout << "invalid input" << std::endl;
				std::cin.clear();//Clear the error
				std::cin.ignore(); //discard input
			}
		} while (ch != 0 && ch != 1);

		RfidTben_hf::ModbusAddress chX_commandCode;
		RfidTben_hf::ModbusAddress chX_startAddr;
		RfidTben_hf::ModbusAddress chX_length;
		RfidTben_hf::ModbusAddress chX_tagCounter;
		RfidTben_hf::ModbusAddress chX_byteAvailable;
		RfidTben_hf::ModbusAddress chX_inputTag;
		RfidTben_hf::ModbusAddress chX_rwLength;
		RfidTben_hf::ModbusAddress chX_tagPresentAt;

		if (ch == 0) {
			chX_commandCode = RfidTben_hf::ch0_commandCode;
			chX_startAddr = RfidTben_hf::ch0_startAddr;
			chX_length = RfidTben_hf::ch0_length;
			chX_tagCounter = RfidTben_hf::ch0_tagCounter;
			chX_byteAvailable = RfidTben_hf::ch0_byteAvailable;
			chX_inputTag = RfidTben_hf::ch0_inputTag;
			chX_rwLength = RfidTben_hf::ch0_rwlength;
			chX_tagPresentAt = RfidTben_hf::ch0_tagPresentAt;
		}
		else {
			chX_commandCode = RfidTben_hf::ch1_commandCode;
			chX_startAddr = RfidTben_hf::ch1_startAddr;
			chX_length = RfidTben_hf::ch1_length;
			chX_tagCounter = RfidTben_hf::ch1_tagCounter;
			chX_byteAvailable = RfidTben_hf::ch1_byteAvailable;
			chX_inputTag = RfidTben_hf::ch1_inputTag;
			chX_rwLength = RfidTben_hf::ch1_rwlength;
			chX_tagPresentAt = RfidTben_hf::ch1_tagPresentAt;
		}

		while (true) {
			int option = display_options();
			int loopCount = 0;
			char input2 = '0';
			uint32_t input3;
			int input4;
			uint32_t input5;
			uint8_t epcChange[DLENTH] = { 0x4c,0x51,0x31,0x34,0x78,0x31,0x34,0x4f,
								0x4e,0x4c,0x30,0x30,0x30,0x30,0x30,(uint8_t)'1',
								(uint8_t)'*',(uint8_t)'*',0,0,0,0};
			uint16_t * intptr = mock2.awRFID_input;
			switch (option) {
			case 1:
				mock2.Rfid_changeMode(RfidTben_hf::Idle, chX_commandCode);
				break;
			case 2:
				mock2.hfbus_readTagPresent(chX_tagPresentAt);
				std::cout << "tag detected at readhead: " << mock2.tagPresentwhere << std::endl;
				break;
			case 3:
				mock2.parseHFuid(ch);
				break;
			case 4:
				//mock2.hfbus_readTagPresent(chX_tagPresentAt);
				mock2.Rfid_ReadData(ch, DLENTH);
				mock2.parseHFuserdata(ch);
				break;
			case 5:
				//mock2.hfbus_readTagPresent(chX_tagPresentAt);
				memcpy(mock2.awRFID_output, epcChange, DLENTH);
				mock2.Rfid_WriteData(ch, DLENTH);
				break;
			case 6:
				mock2.incrementCount(ch);
				break;
			case 7:
				mock2.initCount(ch);
				break;
			case 9:
				mock2.Rfid_changeMode(RfidTben_hf::Reset, chX_commandCode);
				break;
			case 0:
				std::cout << "ENDing PROGRAM.....\n";
				return 0;
			default:
				break;
			}
		}
		/*
		// simple application
		int prevhead=0;
		std::cout << "\ncPress crtl + c to end program\n";
		while (1)
		{
			tagPresentwhere = mock2.hfbus_readTagPresent();
			if (prevhead != tagPresentwhere) {
				std::cout << "Tag detected at " << tagPresentwhere << "\n"; 
				mock2.parseHFuid();
				prevhead = tagPresentwhere;
			}
			SLEEP(SLEEP_TIME);
		} 
		*/

	}

	return 0;
}


int display_options() {
	int input = 1;
	std::cout << "Please choose your options\n";
	std::cout << "1: idle\n";
	std::cout << "2: detect readhead activated\n";
	std::cout << "3: read UID\n";
	std::cout << "4: read userdata\n";
	std::cout << "5: write userdata\n";
	std::cout << "6: increment Userdata\n";
	std::cout << "7: init all userdata\n";
	std::cout << "9: reset\n";
	std::cout << "0: END PROGRAM\n";
	std::cout << "Please choose your options....>   ";
	do
	{
		if (!(std::cin >> input)) {//error occurred
			std::cout << "invalid input" << std::endl;
			std::cin.clear();//Clear the error
			std::cin.ignore(); //discard input
		}
	} while (input < 0 || input>9);

	return (int)input;
}