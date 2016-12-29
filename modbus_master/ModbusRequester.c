#include <ModbusRequester.h>
#include "ModbusDefinitions.h"
#include "ModbusSettings.h"
#include "ModbusMaster.h"
#include "Log.h"
#include "Crc.h"

int requester_request(ModbusMaster *mb, Uint16 slaveAddress, ModbusFunctionCode functionCode,
		Uint16 addr, Uint16 totalData) {
	mb->requester.slaveAddress = slaveAddress;
	mb->requester.functionCode = functionCode;
	mb->requester.addr	       = addr;
	mb->requester.totalData    = totalData;
	mb->requester.generate(mb);

	// Set the ModbusMaster to the start state
	mb->state = MB_START;

	// Wait until the MobbusMaster finish all the state flow
	while(mb->state != MB_END) {
		mb->loopStates(mb);
	}

	return 1;
}

int requester_readInputs(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_READ_INPUT, addr, totalData);
}

int requester_readCoils(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_READ_COIL, addr, totalData);
}

int requester_readHolding(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_READ_HOLDINGREGISTERS, addr, totalData);
}

int requester_readInputRegs(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_READ_INPUTREGISTERS, addr, totalData);
}

int requester_forceCoils(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_FORCE_NCOILS, addr, totalData);
}

int requester_writeHolding(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData) {
	return mb->requester.request(mb, slaveAddress, MB_FUNC_WRITE_NREGISTERS, addr, totalData);
}
// according to the function code, generate the data needed after the function code
// question is how?
void requester_generate(ModbusMaster *master) {
	ModbusRequester requester = master->requester; //check
	Uint16 i, sizeWithoutCRC;                      //check
	Uint16 * transmitStringWithoutCRC;             //check

	// Reference to MODBUS Data Map
	// what is this data map? used for salve?
	char * dataPtr;                                //check
//	Uint16 sizeOfMap = 0;
//TODO  why this, requester.slaveAddress was just copied from master->dataRequest.slaveAddress, they should be the same
	master->dataRequest.slaveAddress = requester.slaveAddress;  //check
	master->dataRequest.functionCode = requester.functionCode;  //check

	// First, get the right data map based on the function code
	// why this? what is this data map?
	if (requester.functionCode == MB_FUNC_READ_COIL ||             //check
			requester.functionCode == MB_FUNC_FORCE_COIL ||        //check
			requester.functionCode == MB_FUNC_FORCE_NCOILS)        //check
	{
		dataPtr = (char *)&(master->coils);  //TODO why char *     //check
//		sizeOfMap = sizeof(master->coils);
	}
	else if (requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS ||// check
   			requester.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER || // check
			requester.functionCode == MB_FUNC_WRITE_NREGISTERS)         //check
	{
		dataPtr = (char *)&(master->holdingRegisters);                 //check
//		sizeOfMap = sizeof(master->holdingRegisters);
	}


	//
	// For read function, we send the first address and the number of reads we need.
	// Second: prepare the dataRequest content array.
	if (requester.functionCode == MB_FUNC_READ_COIL || requester.functionCode == MB_FUNC_READ_INPUT ||                         //check
			requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS || requester.functionCode == MB_FUNC_READ_INPUTREGISTERS)  //check
	{
		master->dataRequest.content[MB_READ_ADDRESS_HIGH]   = (requester.addr & 0xFF00) >> 8;                                  //check
		master->dataRequest.content[MB_READ_ADDRESS_LOW]    = (requester.addr & 0x00FF);                                       //check
		master->dataRequest.content[MB_READ_TOTALDATA_HIGH] = (requester.totalData & 0xFF00) >> 8;                             //check
		master->dataRequest.content[MB_READ_TOTALDATA_LOW]  = (requester.totalData & 0x00FF);                                  //check
		//because in serial transmitting, we transmit 8 bit as a group, and the address/totalData is 16 bit, we need to break them down to
		//high 8 bit and low 8 bit.
		master->dataRequest.contentIdx = MB_READ_TOTALDATA_LOW + 1;   //TODO??
	}
	// write single coil or single register
	else if (requester.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER || requester.functionCode == MB_FUNC_FORCE_COIL) {         //check
		master->dataRequest.content[MB_WRITE_ADDRESS_HIGH]  = (requester.addr & 0xFF00) >> 8;                                   //check
		master->dataRequest.content[MB_WRITE_ADDRESS_LOW]   = (requester.addr & 0x00FF);

		master->dataRequest.contentIdx = MB_WRITE_ADDRESS_LOW+1;

//changed into MB_WRITE_ADDRESS_LOW for the second
//so the address is 16bit and we organize the high address and low address
		// Get the data at the specified address (one byte only)

		//single coil
		if(requester.functionCode == MB_FUNC_FORCE_COIL)
		{
			master->dataRequest.content[master->dataRequest.contentIdx++]|=(*(dataPtr+requester.addr) & 0x0001);
		}
		else
		//single register
		{


		#if MB_32_BITS_REGISTERS == true
		//TODO what is the difference, dsp uses 32 bit register, we need access a 16 bit data, one address is for 16 bit
		// so a 32 bit register have two addresses
		master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + requester.addr*2 + 1) & 0xFF00) >> 8;
		master->dataRequest.content[master->dataRequest.contentIdx++]  = (*(dataPtr + requester.addr*2 + 1) & 0x00FF);

		master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + requester.addr*2 + 0) & 0xFF00) >> 8;
		master->dataRequest.content[master->dataRequest.contentIdx++]  = (*(dataPtr + requester.addr*2 + 0) & 0x00FF);
		#else
		//16 bit reg
		master->dataRequest.content[MB_WRITE_VALUE_HIGH] = (*(dataPtr + requester.addr) & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_VALUE_LOW]  = (*(dataPtr + requester.addr) & 0x00FF);
		master->dataRequest.contentIdx = MB_WRITE_VALUE_LOW + 1;
		#endif
		}
	}
	//To write multiple registers or coils, we need the starting address and the number of coils of registers and the data that we need to write to them
	//
	else if (requester.functionCode == MB_FUNC_WRITE_NREGISTERS ||	requester.functionCode == MB_FUNC_FORCE_NCOILS) {                //check
		master->dataRequest.content[MB_WRITE_N_ADDRESS_HIGH]  = (requester.addr & 0xFF00) >> 8;                                      //check
		master->dataRequest.content[MB_WRITE_N_ADDRESS_LOW]  = (requester.addr & 0x00FF);                                            //check

		master->dataRequest.content[MB_WRITE_N_QUANTITY_HIGH] = (requester.totalData & 0xFF00) >> 8;                                 //check
		master->dataRequest.content[MB_WRITE_N_QUANTITY_LOW]  = (requester.totalData & 0x00FF);  //check
		//number of bytes for forcing NCOILS
		if(requester.functionCode == MB_FUNC_FORCE_NCOILS)

			{
			if(requester.totalData%8==0)
				master->dataRequest.content[MB_WRITE_N_BYTES]         = (requester.totalData)/8;
			else
			master->dataRequest.content[MB_WRITE_N_BYTES]         = (requester.totalData)/8+1;

			master->dataRequest.contentIdx = MB_WRITE_N_BYTES+1;//TODO??
			for(i =0; i < requester.totalData; i++){
				Uint16 padding = i + requester.addr;
				master->dataRequest.content[master->dataRequest.contentIdx]|=(*(dataPtr + padding) & 0x0001) << (i%8);
	            if((i+1)%8==0)
	            	master->dataRequest.contentIdx++;
			}
            	if(requester.totalData%8!=0)
			    master->dataRequest.contentIdx++;

			}
		else
		// number of bytes for writing N registers
		{
            #if MB_32_BITS_REGISTERS == true
			master->dataRequest.content[MB_WRITE_N_BYTES]         = (requester.totalData) * 4;//check
            #else
			//16 bit
			master->dataRequest.content[MB_WRITE_N_BYTES]         = (requester.totalData) * 2;
			#endif

			master->dataRequest.contentIdx = MB_WRITE_N_BYTES+1;

			for(i=0; i < requester.totalData*2; i++) {                                                                                      //check
				Uint16 padding = i + requester.addr*2; //data is here
				#if MB_32_BITS_REGISTERS == true                                                                                       //TODO why do we need four assignment?
				//write 32 bit
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 1) & 0xFF00) >> 8;
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 1) & 0x00FF);
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 0) & 0xFF00) >> 8;
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 0) & 0x00FF);
				i++;                                                                                                                  //TODO?
				#else
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding) & 0xFF00) >> 8;
				master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding) & 0x00FF);
				#endif
			}


		}



		// Loop through the selected data map, different for writing N regs and coils
        // For writing N coils, we need to combine the bits data into bytes.






	}


	// Prepares data request string
	// Slave address (1 byte) + Function Code (1 byte) + CRC (2 bytes) ->  MB_SIZE_COMMON_DATA
    //TODO How is dataRequest generated??
	master->dataRequest.size = MB_SIZE_COMMON_DATA + master->dataRequest.contentIdx;            //TODO? what is contentIdx?

	sizeWithoutCRC = master->dataRequest.size - 2;

	transmitStringWithoutCRC = master->dataResponse.getTransmitStringWithoutCRC(&master->dataRequest);
	//pointer that points to the to transmit data
	//First get the data without CRC and then use the data to calculate CRC
	master->dataRequest.crc = generateCrc( transmitStringWithoutCRC, sizeWithoutCRC, true );

	master->requester.isReady = true;
}

void requester_save(ModbusMaster *master) {
	ModbusRequester requester = master->requester;

	// Reference to MODBUS Data Map
	char * dataPtr;
//	Uint16 sizeOfMap;

	// First, get the right data map based on the function code
	if (requester.functionCode == MB_FUNC_READ_COIL ||
			requester.functionCode == MB_FUNC_FORCE_COIL ||
			requester.functionCode == MB_FUNC_FORCE_NCOILS)
	{
		dataPtr = (char *)&(master->coils);
//		sizeOfMap = sizeof(master->coils);
	}
	else if (requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS ||
			requester.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER ||
			requester.functionCode == MB_FUNC_WRITE_NREGISTERS)
	{
		dataPtr = (char *)&(master->holdingRegisters);
//		sizeOfMap = sizeof(master->holdingRegisters);
	}

	if (requester.functionCode == MB_FUNC_READ_COIL || requester.functionCode == MB_FUNC_READ_INPUT ||
				requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS || requester.functionCode == MB_FUNC_READ_INPUTREGISTERS)
	{
		Uint16 * memAddr;
		Uint16 i;

		for(i=0; i < (requester.totalData); i++) {
			Uint16 padding = i + requester.addr;
			memAddr = (Uint16 *) (dataPtr + padding);
			*(memAddr) = (master->dataResponse.content[1+i*2] << 8) | (master->dataResponse.content[(1+1)+i*2]);
		}
	}
}

ModbusRequester construct_ModbusRequestHandler(){
	ModbusRequester requester;

	requester.slaveAddress = 1;
	requester.functionCode = MB_FUNC_READ_HOLDINGREGISTERS;
	requester.addr = 0;
	requester.totalData = 0;

	requester.isReady = false;

	requester.generate   = requester_generate;
	requester.save       = requester_save;
	requester.request    = requester_request;

	requester.readCoils     = requester_readCoils;
	requester.readInputs    = requester_readInputs;
	requester.readHolding   = requester_readHolding;
	requester.readInputRegs = requester_readInputRegs;
	requester.writeHolding  = requester_writeHolding;
	requester.forceCoils    = requester_forceCoils;

	MB_DATA_HANDLER_DEBUG();

	return requester;
}

