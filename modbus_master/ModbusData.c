#include "ModbusDefinitions.h"
#include "ModbusData.h"
#include "Log.h"

// Clears all data of Modbus Data
void data_clear(ModbusData *self){
	MB_DATA_RESPONSE_DEBUG();
	self->slaveAddress = 0;
	self->functionCode = 0;
	self->size = 0;
	self->contentIdx = 0;
	self->crc = 0;
}

// Return a string with the data that will be transmitted
// Mostly used for the last step of process: MB_TRANSMIT (serial TX)
// TODO how the transmitted data is generated??
 Uint16 * data_getTransmitString(ModbusData *self){
	static Uint16 string[MB_BUFFER_SIZE] = {0};
	Uint16 stringIndex = 0;
	Uint16 contentIterator;

	string[stringIndex++] = self->slaveAddress;
	string[stringIndex++] = self->functionCode;
	
	// Loop through the "general content" part of data frame, address, function code and function related data included
	for(contentIterator = 0; contentIterator < (self->size - MB_SIZE_COMMON_DATA); contentIterator++){
		string[stringIndex++] = (self->content[contentIterator]) & 0x00FF;
	}
	// Add crc data

	string[stringIndex++] = (self->crc & 0xFF00) >> 8;
	string[stringIndex++] = self->crc & 0x00FF;
    // So we have all the data, ready to transmit!
	MB_DATA_RESPONSE_DEBUG();
	return string;
}

// Return a string with the data that will be transmitted (but without CRC)
// Mostly used to create the CRC (get this string and the size of it and then generate the CRC)
 Uint16 * data_getTransmitStringWithoutCRC(ModbusData *self){
	static Uint16 string[MB_BUFFER_SIZE] = {0}; //Uint16 is unsigned int, integer is 16 bit!!!
	Uint16 stringIndex = 0;                     //ok this is not true..

	Uint16 contentIterator;

	string[stringIndex++] = self->slaveAddress;
	string[stringIndex++] = self->functionCode;

	// Loop through the "general content" part of data frame
	for(contentIterator = 0; contentIterator < (self->size - MB_SIZE_COMMON_DATA); contentIterator++){
		string[stringIndex++] = (self->content[contentIterator]);
	}

	MB_DATA_RESPONSE_DEBUG();
	return string;// return the address of the to-be sent data
}

ModbusData construct_ModbusData(){
	ModbusData modbusData;

	modbusData.slaveAddress = 0;
	modbusData.functionCode = 0;
	modbusData.contentIdx = 0;
	modbusData.crc = 0;
	modbusData.size = 0;

	modbusData.clear = data_clear;
	modbusData.getTransmitString = data_getTransmitString;
	modbusData.getTransmitStringWithoutCRC = data_getTransmitStringWithoutCRC;

	MB_DATA_RESPONSE_DEBUG();

	return modbusData;
}
