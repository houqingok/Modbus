#include "DSP2833x_Device.h"
#include "ModbusMaster.h"
#include "ModbusSettings.h"
#include "Log.h"



#if DEBUG_UTILS_PROFILING
#include "Profiling.h"
ProfilingTool profiling;
#endif

Uint64 ticks = 0;
Uint64 abcd=0;

void master_loopStates(ModbusMaster *self){
	MB_MASTER_DEBUG();
	switch (self->state) {
	case MB_CREATE:
		MB_MASTER_DEBUG("State: MB_MASTER_CREATE\n");
		self->create(self);
		break;
	case MB_START:
		MB_MASTER_DEBUG("State: MB_MASTER_START\n");
		self->start(self);
	case MB_END:
		MB_MASTER_DEBUG("State: MB_MASTER_START\n");
		self->start(self);
		break;
	case MB_WAIT:
		self->wait(self);
		break;
	case MB_REQUEST:
		MB_MASTER_DEBUG("State: MB_MASTER_REQUEST\n");
		self->request(self);
		break;
	case MB_RECEIVE:
		MB_MASTER_DEBUG("State: MB_MASTER_RECEIVE\n");
		self->receive(self);
		break;
	case MB_PROCESS:
		MB_MASTER_DEBUG("State: MB_MASTER_PROCESS\n");
		self->process(self);
		break;
	case MB_DESTROY:
		MB_MASTER_DEBUG("State: MB_MASTER_DESTROY\n");
		self->destroy(self);
		break;
	}
}

void master_create(ModbusMaster *self){
	MB_MASTER_DEBUG();

	// Configure Serial Port A
	self->serial.baudrate = SERIAL_BAUDRATE;
	self->serial.parityType = SERIAL_PARITY;
	self->serial.bitsNumber = SERIAL_BITS_NUMBER;
	self->serial.init(&self->serial);

	self->timer.init(&self->timer, MB_REQ_TIMEOUT);

#if DEBUG_UTILS_PROFILING
	profiling = construct_ProfilingTool();
#endif
	self->state = MB_START;
}
//TODO why wait?
void master_start(ModbusMaster *self){
	MB_MASTER_DEBUG();
	
	self->dataRequest.clear(&self->dataRequest);    //clear data
	self->dataResponse.clear(&self->dataResponse);  //clear data

	self->serial.clear();
	self->timeout = false;

	self->timer.resetTimer();
	self->timer.setTimerReloadPeriod(&self->timer, MB_REQ_INTERVAL);
	self->timer.start();

	self->state = MB_WAIT;
}
// 3.5
void master_wait(ModbusMaster *self) {
	if (self->timer.expiredTimer(&self->timer)) {
		self->timer.stop();
		self->state = MB_REQUEST;
	}
}

void master_request(ModbusMaster *self){

	GpioDataRegs.GPASET.bit.GPIO20 = 1; // set 485 chip enable

	Uint16 * transmitString;
	// Wait until the code signals that the request is ready to transmit
	while (self->requester.isReady == false ) { }
	// Reset request ready signal
	self->requester.isReady = false;
//TODO ????why dataResponse not request??
// What is the difference bet. dataResponse and dataRequest??
	transmitString = self->dataRequest.getTransmitString(&self->dataRequest);
#if DEBUG_UTILS_PROFILING
	profiling.start(&profiling);
#endif
	//Transmit here!!!

//	self->serial.transmitData(transmitString, self->dataRequest.size);

	//send ABCDEFGHIJ and change line


	 //Uint16 ptr[10]={0x01,0x0f,0x00,0x00,0x00,0x03,0x01,0x01,0x4e,0x97};
	 //Uint16 ptr1[10]={0x01,0x0f,0x00,0x00,0x00,0x03,0x01,0x00,0x8F,0x57};

	 self->serial.transmitData(transmitString,self->dataRequest.size);

   /* if(abcd == 0)
    {
	self->serial.transmitData(ptr, 10);
    abcd = 1;
    }
    else
    {
    	self->serial.transmitData(ptr1, 10);

        abcd = 0;
    }*/

	MB_MASTER_DEBUG();

	self->state = MB_RECEIVE;
}
// data transmitted and wait for response
void master_receive(ModbusMaster *self){

	GpioDataRegs.GPACLEAR.bit.GPIO20 = 1;//set 485 chip receive enable

	self->timer.resetTimer();
	self->timer.setTimerReloadPeriod(&self->timer, MB_REQ_TIMEOUT);
	self->timer.start();

	self->requestProcessed = false;

	// Wait to have some date at the RX Buffer
	// If removed it can give errors on address and function code receiving
	while (self->serial.rxBufferStatus() < 2
			&& ( self->serial.getRxError() == false )
			&& ( self->timer.expiredTimer(&self->timer) == false )
	) { ; }

	// Get basic data from response
	self->dataResponse.slaveAddress = self->serial.getRxBufferedWord();//8 bit
	self->dataResponse.functionCode = self->serial.getRxBufferedWord();
    //TODO?
	// Prepare the buffer size
	if (self->dataRequest.functionCode == MB_FUNC_READ_HOLDINGREGISTERS) {
		self->serial.fifoWaitBuffer = MB_SIZE_COMMON_DATA_WITHOUTCRC;
		self->serial.fifoWaitBuffer += self->dataRequest.content[3] * 2;
		self->serial.fifoWaitBuffer += 1; //the number of incoming bytes itself is received as well
	}//other function codes?
	else {
		self->serial.fifoWaitBuffer = MB_SIZE_RESP_WRITE;
	}

	// Receive the data contents, based on the buffer size
	while ( ( self->serial.fifoWaitBuffer > 0 )
		&& ( self->serial.getRxError() == false )
		&& ( self->timer.expiredTimer(&self->timer) == false )
	) {
		if(self->serial.rxBufferStatus() > 0) {
			self->dataResponse.content[self->dataResponse.contentIdx++] = self->serial.getRxBufferedWord();
			self->serial.fifoWaitBuffer--;
		}
	}

	// Receive the CRC
	self->dataResponse.crc = (self->serial.getRxBufferedWord() << 8) | self->serial.getRxBufferedWord();

	self->timer.stop();
#if DEBUG_UTILS_PROFILING
	profiling.stop(&profiling);
#endif

	// Jump to START if there is any problem with the basic info
	// This means mistakes happened!
	if (self->dataResponse.slaveAddress != self->dataRequest.slaveAddress ||
			self->dataResponse.functionCode != self->dataRequest.functionCode ) {
		self->state = MB_END;
		return ;
	}

	// If there is any error on Reception, it will go to the START state
	if (self->serial.getRxError() == true || self->timer.expiredTimer(&self->timer)){
		self->state = MB_END;
	} else {
		self->state = MB_PROCESS;
	}
}

void master_process (ModbusMaster *self){
	self->requester.save(self);

	self->requestProcessed = true;

	self->state = MB_END;
}

void master_destroy(ModbusMaster *self){
	MB_MASTER_DEBUG();
}

ModbusMaster construct_ModbusMaster(){
	ModbusMaster modbusMaster;

	MB_MASTER_DEBUG();

	modbusMaster.state = MB_CREATE;
	//checked
	modbusMaster.dataRequest = construct_ModbusData();
	//checked													      // this is mainly for pointing function pointers to corresponding functions
	modbusMaster.dataResponse = construct_ModbusData();
	//checked link functions														  // of structure types
	modbusMaster.requester = construct_ModbusRequestHandler();//
	//checked link functions
	modbusMaster.serial = construct_Serial();
	//checked
	modbusMaster.timer = construct_Timer();                   //

	modbusMaster.timeoutCounter = 0;
	modbusMaster.successfulRequests = 0;
	modbusMaster.requestReady = false;
	modbusMaster.requestProcessed = false;
// initialize data
#if MB_COILS_ENABLED
	modbusMaster.coils = construct_ModbusCoilsMap();
#endif
#if MB_INPUTS_ENABLED
	modbusMaster.inputs = construct_ModbusInputsMap();
#endif
#if MB_HOLDING_REGISTERS_ENABLED
	modbusMaster.holdingRegisters = construct_ModbusHoldingRegistersMap();
#endif
#if MB_INPUT_REGISTERS_ENABLED
	modbusMaster.inputRegisters = construct_ModbusInputRegistersMap();
#endif
// point function pointers to corresponding functions
	modbusMaster.loopStates = master_loopStates;
	modbusMaster.create = master_create;
	modbusMaster.start = master_start;
	modbusMaster.wait = master_wait;
	modbusMaster.request = master_request;
	modbusMaster.receive = master_receive;
	modbusMaster.process = master_process;
	modbusMaster.destroy = master_destroy;

	return modbusMaster;
}
