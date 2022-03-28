#include "RfidTbenHFBus.h"

int RfidTben_hf::connectModbus(const char * ipAddress) {

	modbusHandler = modbus_new_tcp(ipAddress, 502);
	if (modbusHandler == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	if (modbus_connect(modbusHandler) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(modbusHandler);
		return -1;
	}
	return 0;
}

int RfidTben_hf::connectModbus(modbus_t * mb, const char * ipAddress) {

	mb = modbus_new_tcp(ipAddress, 502);
	if (mb == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	if (modbus_connect(mb) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(mb);
		return -1;
	}
	return 0;	
}


void RfidTben_hf::disconnectModbus() {
	modbus_close(modbusHandler);
	modbus_free(modbusHandler);
	printf("Closing modbus connection\n");
	return;
}

RfidTben_hf::~RfidTben_hf() {
	modbus_close(modbusHandler);
	modbus_free(modbusHandler);
	printf("Closing modbus connection\n");
}


int RfidTben_hf::hfbus_readTagPresent(ModbusAddress MBaddr) {
	int searchBit=1;
	int32_t tagLocation;
	int tc = modbus_read_registers(modbusHandler, MBaddr, 2, (uint16_t *)&tagLocation);
	if (tc == -1) {
		fprintf(stderr, "device count read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	if (tagLocation == 0) {
		tagPresentwhere = 0;
		return 0;
	}
	else {
		for (tagPresentwhere = 1; tagPresentwhere < 33; tagPresentwhere++) {
			if (tagLocation & searchBit) {
				//fprintf(stdout, "tagPresentwhere: %d\n", tagPresentwhere);
				return tagPresentwhere;
			}
			searchBit = searchBit << 1;
		}
		return 0;
	}
		
	
}

int RfidTben_hf::readrwLength(ModbusAddress MBaddr) {
	int tc = modbus_read_registers(modbusHandler, MBaddr, 1, &rwlength);
	if (tc == -1) {
		fprintf(stderr, "device count read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	fprintf(stdout, "chX_rwlength: %d\n", rwlength);
	return rwlength;
}

int RfidTben_hf::hfbus_readTagInput(ModbusAddress MBaddr) {
	int tc = modbus_read_registers(modbusHandler, MBaddr, rwlength/2, awRFID_input);
	if (tc == -1) {
		fprintf(stderr, "device count read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return 0;
}

int RfidTben_hf::hfbus_readUserData(uint16_t wordLen, ModbusAddress MBaddr) {
	int rc = 0;
	rc = modbus_read_registers(modbusHandler, MBaddr, wordLen, awRFID_input);
	if (rc == -1) {
		fprintf(stderr, "read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return rc;
}

int  RfidTben_hf::writeModbus(uint16_t startAddr, int valueToWrite) {
	int wc = 0;
	wc = modbus_write_register(modbusHandler, startAddr, valueToWrite);
	if (wc == -1) {
		fprintf(stderr, "write failed: %s\n", modbus_strerror(errno));
		return -1;
	}

	return wc;
}

int RfidTben_hf::Rfid_changeMode(RfidCommand command, ModbusAddress MBaddr) {
	if (command == Reset) {
		memset(awRFID_input, 0, sizeof(awRFID_input));
	}
	return writeModbus(MBaddr, command);
}

int RfidTben_hf::Rfid_changeStartAddr(uint32_t addr, ModbusAddress MBaddr) { //to be tested
	int wc;
	uint16_t temp[2];
	memcpy(temp, &addr, 2);
	wc = writeModbus(MBaddr, temp[0]);
	wc = writeModbus((MBaddr + 1), temp[1]);
	return wc;
}

int RfidTben_hf::Rfid_changeAntenna(uint16_t anNum, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, anNum);
}

int RfidTben_hf::Rfid_changeByteLength(uint16_t len, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, len);
}

int RfidTben_hf::Rfid_changeEpcLength(uint16_t epclen, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, epclen);
}

int RfidTben_hf::Rfid_changeOutputData(int wordLen, ModbusAddress MBaddr) {
	//lenth of output data to write in word
	int wc = modbus_write_registers(modbusHandler, MBaddr, wordLen, awRFID_output);
	if (wc == -1) {
		fprintf(stderr, "write failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wc;
}

uint16_t RfidTben_hf::Rfid_readResponseCode(ModbusAddress MBaddr) {
	uint16_t responseCode;
	int resc = modbus_read_registers(modbusHandler, MBaddr, 1, &responseCode);
	if (resc == -1) {
		fprintf(stderr, "responseCode read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	xEC_Error = (responseCode & 0x4000) >> 14;
	xEC_Busy = (responseCode & 0x8000) >> 15;
	return responseCode;
}

uint16_t RfidTben_hf::Rfid_readStatusCode(ModbusAddress MBaddr) {
	uint16_t statusCode;
	int resc = modbus_read_registers(modbusHandler, MBaddr, 1, &statusCode);
	if (resc == -1) {
		fprintf(stderr, "responseCode read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	xTagPresent = (statusCode & 0x1);
	xAntennaDetune = (statusCode & 0x10) >> 4;
	xParNoSupported = (statusCode & 0x20) >> 5;
	xError = (statusCode & 0x40) >> 6;
	xNotConnected = (statusCode & 0x80) >> 7;
	xRWHeadSwitchON = (statusCode & 0x100) >> 8;
	xContinuousModeActive = (statusCode & 0x200) >> 9;
	return statusCode;
}

uint16_t RfidTben_hf::Rfid_readErrorCode(ModbusAddress MBaddr) {
	int ec = modbus_read_registers(modbusHandler, MBaddr, 1, &wErrorCode);
	if (ec == -1) {
		fprintf(stderr, "error code read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wErrorCode;
}

/**************************************************************************/
/******************			Helpful functions		***********************/
/**************************************************************************/

string RfidTben_hf::convertExtendedAscii(uint8_t hexInput) {
	string exStr = "";
	int temp[2];
	temp[0] = hexInput & 0xf;
	temp[1] = hexInput >> 4;

	if (temp[1] < 10)
		exStr.push_back(('0' + temp[1]));
	else
		exStr.push_back(('0' + temp[1] + 7));

	if (temp[0] < 10)
		exStr.push_back(('0' + temp[0]));
	else
		exStr.push_back(('0' + temp[0] + 7));

	return exStr;
}

string RfidTben_hf::wordToAscii(uint16_t wordSrc) {
	uint8_t hex[2];
	string str = "";
	hex[0] = wordSrc & 0xff;
	hex[1] = (wordSrc >> 8) & 0xff;
	if (hex[0] < 127)
		str.push_back((char)hex[0]);
	else
		str += convertExtendedAscii(hex[0]);

	if (hex[1] < 127)
		str.push_back((char)hex[1]);
	else
		str += convertExtendedAscii(hex[1]);

	return str;
}

//parse hex to string

string RfidTben_hf::wordToByteString(uint16_t wordSrc) {
	uint8_t hex;
	char temp[4];
	char swap[5] = { '0','0','0','0','\0' };
	for (int i = 0; i < 4; i++) {
		hex = (wordSrc >> (i * 4)) & 0xf;
		if (hex < 10) {
			temp[i] = ('0' + hex);
		}
		else {
			temp[i] = ('0' + hex + 7);
		}
	}
	swap[0] = temp[1];
	swap[1] = temp[0];
	swap[2] = temp[3];
	swap[3] = temp[2];
	string str(swap);
	return str;
}
/**************************************************************************/
/******************			Derived functions		***********************/
/**************************************************************************/

string RfidTben_hf::parseHFuid(int channel) {
	//note that readheadDetected will not change if the same tag is only movesd from 1 antenna to another 
	//note time is needed to switch between read and 
	uidDetected ="";
	uint16_t readheadDetected;
	int i;
	switch (channel) {
		case 0:
			Rfid_changeMode(Idle, ch0_commandCode);
			SLEEP(500ms);
			readrwLength(ch0_rwlength);
			SLEEP(500ms);
			hfbus_readTagInput(ch0_inputTag);
			SLEEP(SLEEP_TIME);
			break;
		case 1:
			Rfid_changeMode(Idle, ch1_commandCode);
			SLEEP(500ms);
			readrwLength(ch1_rwlength);
			SLEEP(500ms);
			hfbus_readTagInput(ch1_inputTag);
			SLEEP(SLEEP_TIME);
			break;
		default:
			return "-1";
	}

	uidlength = rwlength - 2; 
	for (i = 0; i < uidlength/2; i++) {
		uidDetected = uidDetected + wordToByteString(awRFID_input[i]);
	}
	readheadDetected = (awRFID_input[i] >> 8);
	std::cout << "UID:" << uidDetected << " was recorded at read head:" << readheadDetected <<"\n";
	return uidDetected;
}

string RfidTben_hf::parseHFuserdata(int channel) {
	string userdataDetected = "";
	int i;
	switch (channel) {
	case 0:
		hfbus_readTagPresent(ch0_tagPresentAt);
		readrwLength(ch0_rwlength);
		break;
	case 1:
		hfbus_readTagPresent(ch1_tagPresentAt);
		readrwLength(ch1_rwlength);
		break;
	default:
		return "-1";
	}

	for (i = 0; i < rwlength/2; i++) {
		userdataDetected = userdataDetected + wordToAscii(awRFID_input[i]);
	}
	std::cout << "User Data:" << userdataDetected << " at read head:" << tagPresentwhere <<"\n";
	return userdataDetected;
}

uint32_t RfidTben_hf::Rfid_ReadData(int channel, int antennaNumber, uint16_t byteLen) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set antenna number to read
	//step 2: Create 'Length' to length of data to read. Don't set starting address!!!
	//step 3: Send command
	//step 4: Read 'inpput process buffer' for response
	uint32_t result;
	uint16_t wordlength = byteLen / 2 + (byteLen & 1);

	switch (channel) {
	case 0:
		Rfid_changeMode(Idle, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(antennaNumber, ch0_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch0_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Read, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch0_responseCode);
		} while (xEC_Busy);
		hfbus_readUserData(wordlength, ch0_inputTag);
		SLEEP(SLEEP_TIME);
		return 0;
	case 1:
		Rfid_changeMode(Idle, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(antennaNumber, ch1_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch1_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Read, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch1_responseCode);
		} while (xEC_Busy);
		hfbus_readUserData(wordlength, ch1_inputTag);
		SLEEP(SLEEP_TIME);
		return 0;
	default:
		return -1;
	}
}

uint32_t RfidTben_hf::Rfid_ReadData(int channel, uint16_t byteLen) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set antenna number to read
	//step 2: Create 'Length' to length of data to read. Don't set starting address!!!
	//step 3: Send command
	//step 4: Read 'inpput process buffer' for response
	uint32_t result;
	uint16_t wordlength = byteLen / 2 + (byteLen & 1);

	switch (channel) {
	case 0:
		hfbus_readTagPresent(ch0_tagPresentAt);
		SLEEP(SLEEP_TIME); 
		Rfid_changeMode(Idle, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(tagPresentwhere, ch0_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch0_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Read, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch0_responseCode);
		} while (xEC_Busy);
		hfbus_readUserData(wordlength, ch0_inputTag);
		SLEEP(SLEEP_TIME);
		return 0;
	case 1:
		hfbus_readTagPresent(ch1_tagPresentAt);
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Idle, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(tagPresentwhere, ch1_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch1_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Read, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch1_responseCode);
		} while (xEC_Busy);
		hfbus_readUserData(wordlength, ch1_inputTag);
		SLEEP(SLEEP_TIME);
		return 0;
	default:
		return -1;
	}
}

int RfidTben_hf::Rfid_WriteData(int channel, int antennaNumber, uint16_t byteLen) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set antenna number to read
	//step 2: Create 'Length' to length of data to read. Don't set starting address!!!
	//step 4: write 'output process buffer' for writing into tag
	//step 3: Send command
	
	uint16_t wordlength = byteLen / 2 + (byteLen & 1);

	switch (channel) {
	case 0:
		Rfid_changeMode(Idle, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(antennaNumber, ch0_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch0_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeOutputData(wordlength, ch0_outputData);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Write, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch0_responseCode);
		} while (xEC_Busy);
		return 0;
	case 1:
		Rfid_changeMode(Idle, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(antennaNumber, ch1_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch1_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeOutputData(wordlength, ch1_outputData);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Write, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch1_responseCode);
		} while (xEC_Busy);
		return 0;
	default:
		return -1;
	}
}

int RfidTben_hf::Rfid_WriteData(int channel, uint16_t byteLen) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set antenna number to read
	//step 2: Create 'Length' to length of data to read. Don't set starting address!!!
	//step 4: write 'output process buffer' for writing into tag
	//step 3: Send command

	uint16_t wordlength = byteLen / 2 + (byteLen & 1);

	switch (channel) {
	case 0:
		hfbus_readTagPresent(ch0_tagPresentAt);
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Idle, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(tagPresentwhere, ch0_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch0_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeOutputData(wordlength, ch0_outputData);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Write, ch0_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch0_responseCode);
		} while (xEC_Busy);
		return 0;
	case 1:
		hfbus_readTagPresent(ch1_tagPresentAt);
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Idle, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		Rfid_changeAntenna(tagPresentwhere, ch1_AntennaNo);
		SLEEP(SLEEP_TIME);
		Rfid_changeByteLength(byteLen, ch1_length);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeOutputData(wordlength, ch1_outputData);//read doubleword
		SLEEP(SLEEP_TIME);
		Rfid_changeMode(Write, ch1_commandCode);
		SLEEP(SLEEP_TIME);
		do {
			Rfid_readResponseCode(ch1_responseCode);
		} while (xEC_Busy);
		return 0;
	default:
		return -1;
	}
}

/**************************************************************************/
/******************		User-defined functions		***********************/
/**************************************************************************/

//assume the tag follows the following format: LQ14x14ONL000001**(32bitinteger)
//assume the tag is already in position

int RfidTben_hf::incrementCount(int channel) {
	uint32_t count;
	
	Rfid_ReadData(channel, tagPresentwhere, DATA_BYTELENTH);
	memcpy(&count, awRFID_input + (EPC_BYTELENTH / 2) +2, 4); //extract count
	count++;
	std::cout << "count :" << count << std::endl;
	memcpy(awRFID_output, awRFID_input, DATA_BYTELENTH);
	memcpy(awRFID_output + (EPC_BYTELENTH / 2)+2, &count, 4);

	Rfid_WriteData(channel, tagPresentwhere, DATA_BYTELENTH);


	return 0;
}

int RfidTben_hf::initCount(int channel) {
	uint32_t count;

	Rfid_ReadData(channel, tagPresentwhere, DATA_BYTELENTH);
	count=0;

	memcpy(awRFID_output, awRFID_input, DATA_BYTELENTH);
	memcpy(awRFID_output + (EPC_BYTELENTH / 2)+2, &count, 4);

	Rfid_WriteData(channel, tagPresentwhere, DATA_BYTELENTH);

	return 0;
}