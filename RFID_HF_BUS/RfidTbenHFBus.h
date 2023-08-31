/*******************************************************************
RFID Modbus code Version 1
Ver1: Initial commit by Low Chang Hong (29 Nov 2021)
Ver2: HF bus application by Low Chang Hong (29 Nov 2021)

Note for usage, unless explicitly mentioned, all length are in bytes, not word
******************************************************************/
#ifndef TRFID_HF_H
#define TRFID_HF_H

#include <chrono>
#include <thread>
#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <string>

#define SLEEP_TIME 50ms
#define SLEEP(a) std::this_thread::sleep_for(a)

#define MAX_BYTELENTH 500
#define DATA_BYTELENTH 128
#define EPC_BYTELENTH 16

extern "C" {
#include <modbus.h>
}

using namespace std::chrono_literals;
using std::string;

class RfidTben_hf
{
	modbus_t * modbusHandler;
	
public:
#define TBEN_S2_2RFID_4DXP
#ifdef TBEN_S2_2RFID_4DXP
	typedef enum {
		Idle = 0x0000,
		Inventory = 0x0001,
		Read = 0x0002,
		Write = 0x0004,
		StartContinousMode = 0x0010,
		StopContinousMode = 0x0012,
		GetData = 0x0011,
		ChangeEPCLength = 0x0007,
		Reset = 0x8000
	}RfidCommand;

	typedef enum {
		Killpassword = 0,
		EPC = 1,
		TID = 2,
		userArea = 3,
		accessPassword = 4,
		epcSize = 5
	}RfidMemoryArea;

	typedef enum {
		ch0_commandCode = 0x800,
		ch0_memoryArea = 0x801, //MSB 8bit, LSB 8bit is loopcounter
		ch0_startAddr = 0x802,
		ch0_length = 0x804,
		ch0_EPClength = 0x805,
		ch0_AntennaNo = 0x80a,
		ch0_outputData = 0x80c,

		ch0_responseCode = 0x00,
		ch0_statusCode = 0x02,
		ch0_rwlength = 0x03, //Length of the read or written data
		ch0_errorCode = 0x04,
		ch0_tagCounter = 0x05,
		ch0_byteAvailable = 0x06,
		ch0_readFrag = 0x07,
		ch0_writeFrag = 0x07,
		ch0_tagPresentAt = 0xa,
		ch0_inputTag = 0x0c,

		ch1_commandCode = 0x84c,
		ch1_memoryArea = 0x84d, //MSB 8bit, LSB 8bit is loopcounter
		ch1_startAddr = 0x84e,
		ch1_length = 0x850,
		ch1_EPClength = 0x851,
		ch1_AntennaNo = 0x856,
		ch1_outputData = 0x0858,

		ch1_responseCode = 0x4c,
		ch1_statusCode = 0x4E,
		ch1_rwlength = 0x4F, //Length of the read or written data
		ch1_errorCode = 0x50,
		ch1_tagCounter = 0x51,
		ch1_byteAvailable = 0x52,
		ch1_readFrag = 0x53,
		ch1_writeFrag = 0x53,
		ch1_tagPresentAt = 0x56,
		ch1_inputTag = 0x58,
	}ModbusAddress;


#endif

	uint8_t databuffer[MAX_BYTELENTH];

	uint16_t awRFID_input[DATA_BYTELENTH /2];
	uint16_t awRFID_output[DATA_BYTELENTH /2];
	uint16_t tagPresentwhere;
	uint16_t rwlength;
	uint16_t uidlength;

	string uidDetected;
	string asRFIDname[50];


	int  connectModbus(modbus_t * mb, const char * ipAddress);
	int  connectModbus(const char * ipAddress);
	void disconnectModbus();
	~RfidTben_hf();

	int hfbus_readTagPresent(ModbusAddress MBaddr);
	int readrwLength(ModbusAddress MBaddr);
	int hfbus_readTagInput(ModbusAddress MBaddr);
	int hfbus_readUserData(uint16_t wordLen, ModbusAddress MBaddr);

	int writeModbus(uint16_t startAddr, int valueToWrite);
	int Rfid_changeMode(RfidCommand command, ModbusAddress MBaddr);
	int Rfid_changeStartAddr(uint32_t addr, ModbusAddress MBaddr);
	int Rfid_changeAntenna(uint16_t anNum, ModbusAddress MBaddr);
	int Rfid_changeByteLength(uint16_t len, ModbusAddress MBaddr);
	int Rfid_changeEpcLength(uint16_t epclen, ModbusAddress MBaddr);
	int Rfid_changeOutputData(int wordLen, ModbusAddress MBaddr);

	uint16_t wErrorCode;
	bool xEC_Error; //true if error occurred
	bool xEC_Busy; //true if still executing command
	bool xTagPresent; //true if tag present at read/write head
	bool xRWHeadSwitchON; //true if read/write head switched on
	bool xContinuousModeActive; //true if continuous mode active
	bool xAntennaDetune;//true if read/write head detuned
	bool xParNoSupported;//true if parameter not supported by read/write head
	bool xError;//true if error message of the read/write head
	bool xNotConnected;//true if 

	uint16_t Rfid_readStatusCode(ModbusAddress MBaddr);
	uint16_t Rfid_readResponseCode(ModbusAddress MBaddr);
	uint16_t Rfid_readErrorCode(ModbusAddress MBaddr);

	//helpful functions
	string wordToByteString(uint16_t wordSrc);
	string convertExtendedAscii(uint8_t hex);
	string wordToAscii(uint16_t wordSrc);
	

	//derived functions
	string parseHFuid(int channel);
	string parseHFuserdata(uint16_t readLen);
	uint32_t Rfid_ReadData(int channel, int antennaNumber, uint16_t byteLen, uint16_t startaddr);
	int Rfid_WriteData(int channel, int antennaNumber, uint16_t byteLen, uint16_t startaddr);

	//User-defined functions
	int Rfid_MultiWrite(int channel, int antennaNumber, uint16_t byteLen);
	int Rfid_MultiRead(int channel, int antennaNumber, uint16_t byteLen);
	int incrementCount(int channel);
	int initCount(int channel);



};

#endif
