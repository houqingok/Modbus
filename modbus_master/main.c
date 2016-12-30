#include "ModbusMaster.h"
#include "DSP2833x_GlobalPrototypes.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include<math.h>
#include"lookup_table.h"
float32 sine_ref=0;
int i=0;


__interrupt void cpu_timer1_isr(void);
ModbusMaster mb;

int main(){


	InitSysCtrl();
	DINT;


	// Initialize the PIE control registers to their default state.
	// The default state is all PIE interrupts disabled and flags
	// are cleared.
	// This function is found in the DSP2833x_PieCtrl.c file.
	   InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	   IER = 0x0000;
	   IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
	// This function is found in DSP2833x_PieVect.c.
	   InitPieVectTable();

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	   EALLOW;  // This is needed to write to EALLOW protected registers
	   //PieVectTable.TINT0 = &cpu_timer0_isr;
	   PieVectTable.XINT13 = &cpu_timer1_isr;
	   //PieVectTable.TINT2 = &cpu_timer2_isr;
	   EDIS;    // This is needed to disable write to EALLOW protected registers

	// Step 4. Initialize the Device Peripheral. This function can be
	//         found in DSP2833x_CpuTimers.c
	   InitCpuTimers();   // For this example, only initialize the Cpu Timers

	#if (CPU_FRQ_150MHZ)
	// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
	// 150MHz CPU Freq, 1 second Period (in uSeconds)

	   ConfigCpuTimer(&CpuTimer0, 150, 1000000);
	   ConfigCpuTimer(&CpuTimer1, 150, 1000000);
	   ConfigCpuTimer(&CpuTimer2, 150, 1000000);
	#endif

	#if (CPU_FRQ_100MHZ)
	// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
	// 100MHz CPU Freq, 1 second Period (in uSeconds)

	   //ConfigCpuTimer(&CpuTimer0, 100, 1000000);
	   ConfigCpuTimer(&CpuTimer1, 100, 1000000);
	   //ConfigCpuTimer(&CpuTimer2, 100, 1000000);
	#endif
	// To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
	// of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in DSP2833x_CpuTimers.h), the
	// below settings must also be updated.

	   //Enable timer interrupt
	   //CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	   // Enable timer interrupt and start timer!
	   CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0


	   //CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	// Step 5. User specific code, enable interrupts:

	// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
	// which is connected to CPU-Timer 1, and CPU int14, which is connected
	// to CPU-Timer 2:
	   //IER |= M_INT1;

	   IER |= M_INT13;// this is timer 1 interrupt

	   //IER |= M_INT14;

	// Enable TINT0 in the PIE: Group 1 interrupt 7
	   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	// Enable global Interrupts and higher priority real-time debug events:
	   EINT;   // Enable Global interrupt INTM
	   ERTM;   // Enable Global real-time interrupt DBGM

//TODO when did the timer start?

	mb = construct_ModbusMaster();     //initialize the data member and function pointers
//what is this?
//
//
	mb.holdingRegisters.dummy0 = 20.1;
	mb.holdingRegisters.dummy1 = 23.13;
	mb.holdingRegisters.dummy2 = 100.1;
	mb.holdingRegisters.dummy3 = 21.2;
	mb.holdingRegisters.dummy4 = 100.5;
	mb.holdingRegisters.dummy5 = 100.555;


	mb.coils.dummy0=1;
	mb.coils.dummy1=1;
	mb.coils.dummy2=1;
	mb.coils.dummy3=0;
	mb.coils.dummy4=1;
	mb.coils.dummy5=0;
	mb.coils.dummy6=1;
	mb.coils.dummy7=0;
	mb.coils.dummy8=1;
	mb.coils.dummy9=1;



	mb.requester.slaveAddress = 0;
	mb.requester.functionCode = MB_FUNC_READ_COIL;
	mb.requester.addr	      = 1;                        //starting address
	mb.requester.totalData    = 9;                           //how many registers we wish to read
	mb.requester.generate(&mb);
	while(1) {
		//calculate six leg length, put them in holding registers and write slave registers.

		mb.requester.generate(&mb);
		mb.loopStates(&mb);

	}

}

__interrupt void cpu_timer1_isr(void)
{
   CpuTimer1.InterruptCount++;
   i++;
   sine_ref = sine_table[i];
   if(i==499)
       i=0;

   // The CPU acknowledges the interrupt.
   EDIS;
}
