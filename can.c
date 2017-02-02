/*==============================================================================================*
 |       Filename: can.c                                                                        |
 | Project/Module: A21 or GATEWAY/module group Communication                                    |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Basic functions for CAN communication.                                       |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifdef SERIAL_SENSOR
	#error "File can.c included but symbol SERIAL_SENSOR defined"
#endif
#ifdef OBC
	#error "File can.c included but symbol OBC defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167.h>
#include <stddef.h>
#include <string.h>

#include "kernel_interface.h"
#include "communication.h"
#include "can.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
//#pragma class PR=KERNEL
//#pragma class CO=KERNEL
//#pragma class FB=KERNELRAM
//#pragma class PR=COMM
//#pragma class CO=COMM
//#pragma class FB=COMMRAM
//#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
byte *canreg[256];					// pointer to register array of CAN1 controller

// CAN receive buffer, ring buffer for received CAN messages:
byte can_receive_buf[CAN_RECEIVE_BUF_LEN];
word can_receive_buf_read;	 		// index of position where the next message is read from
word can_receive_buf_write;			// index of position where the next message is written to
// Usage of global variables can_receive_buf_write_pag and can_receive_buf_write_pof enables
// minimization of time needed by interrupt service routine CANINT_handler for complete readout
// of CAN receive message objects. Variables are initialized by function init_can_interf and
// updated immediately after update of buffer pointer can_receive_buf_write by interrupt service
// routine CANINT_handler.
word can_receive_buf_write_pag;
word can_receive_buf_write_pof;

// CAN transmit buffer, ring buffer for CAN messages to be transmitted:
can_transmit_buf_type can_transmit_buf[CAN_TRANSMIT_BUF_MSG_CNT];
byte can_transmit_buf_read;
byte can_transmit_buf_write;

boolean can_bus_off_state_detectedB;


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
void clear_can_receive_buffer(void)
{
	// Initialize read and write pointers.
	// Ensure consistency of pointers evaluated by CANINT_handler.
//	_atomic(2);
      // CMB
	//Save_ILVL_and_Disable_All_INTs();
	//can_receive_buf_read = can_receive_buf_write = 0;
	//can_receive_buf_write_pag =	_pag(&can_receive_buf[0]);
	//can_receive_buf_write_pof = _pof(&can_receive_buf[0]);
	//Restore_ILVL();
}	// clear_can_receive_buffer


void clear_can_transmit_buffer(void)
{
	// Ensure consistency of pointers evaluated by CANINT_handler.
	//_atomic(2);
	can_transmit_buf_read =	can_transmit_buf_write = 0;
}	// clear_can_transmit_buffer


void begin_can1_init(void)
{
	byte i, BTR0_Value, BTR1_Value, mo_no;
	word *can_mo_ctrl;

        // CMB
	//XP0IE = DISABLE;					//CAN1 controller interrupt disabled

	// Initialize register array of CAN1 controller. Avoid endless for loop.
	for( i = 0; i < 255; i++ )
		canreg[i] = (byte *)(CAN1_BASIC_ADDR + i);
	canreg[255] = (byte *)(CAN1_BASIC_ADDR + 255);

	if( (Comm_ConfigSystem->fCPU == 24000000) )
	    BTR1_Value = BTR1_VAL_24MHz;
	else
	    BTR1_Value = BTR1_VAL_16MHz;

	switch( Comm_ConfigData->CAN_Baud_Rate )
	{
		case  125 :	BTR0_Value = BTR0_VAL_0125kbps; break;
		case  250 :	BTR0_Value = BTR0_VAL_0250kbps; break;
		case  500 :	BTR0_Value = BTR0_VAL_0500kbps; break;
		case 1000 :
		default   :	BTR0_Value = BTR0_VAL_1000kbps; break;
	}

	// Bit CCE (Configuration Change Enable) of CAN1 control register is temporarily set to enabled baud rate setting.
	*canreg[CONTROL] = 0x41;			//TM=0, CCE=1, CPS=0, EIE=0, SIE=0, IE=0, INIT=1
	*canreg[BIT_TIMING0] = BTR0_Value;
	*canreg[BIT_TIMING1] = BTR1_Value;
	*canreg[CONTROL] = 0x01;  			//TM=0, CCE=0, CPS=0, EIE=0, SIE=0, IE=0, INIT=1

	/*Global Masks Configuration: 1=must match 0=don't care*/
	/*1. Global Mask Short (not used here)*/
	*canreg[GLOBAL_MASK1] = 0xFF;		//1111 1111
	*canreg[GLOBAL_MASK2] = 0xFF;		//1111 1111

	/*2. Global Mask Long*/
	// CAN IRMA message identifier, bits 28-22: don't care
	// CAN IRMA receiver class,     bits 21-19: must match
	// CAN IRMA device address,     bits 18- 0: must match
	*canreg[GLOBAL_MASK3] = 0x01;		//0000 0001		ID28...21
	*canreg[GLOBAL_MASK4] = 0xFF;		//1111 1111		ID20...13
	*canreg[GLOBAL_MASK5] = 0xFF;		//1111 1111		ID12...5
	*canreg[GLOBAL_MASK6] = 0xF8;		//1111 1000		(ID4...0)<<3

	/*3. Last Message Mask*/
	// CAN IRMA message identifier, bits 28-22: don't care
	// CAN IRMA receiver class,     bits 21-20: don't care
	// CAN IRMA receiver class,     bit     19: don't care
	// CAN IRMA device address,     bits 18- 0: don't care
	// Any incoming message ID will be accepted by message object 15. However, only those
	// messages that are not accepted by message objects 1, 2 ... 9 will arrive to message
	// object 15. Infineon C167CR Derivatives User´s Manual, V3.2, page 18-20:
	// "If a received message (data frame or remote frame) matches with more than one valid
	// message object, it is associated with the object with the lowest message number."
	*canreg[MSG15_MASK1] = 0x00;
	*canreg[MSG15_MASK2] = 0x00;
	*canreg[MSG15_MASK3] = 0x00;
	*canreg[MSG15_MASK4] = 0x00;

	for( mo_no = 1; mo_no <= 15; mo_no++ )
	{
		can_mo_ctrl = (word *)canreg[(mo_no * MSG_OFFSET) + MSG_CTRL0];

		*can_mo_ctrl = WFLG_RESET_RMTPND |	// Service of remote frame not pending.
					   WFLG_RESET_TXRQ   |	// Transmission not requested.
					   WFLG_RESET_MSGLST |	// No receive message lost.
					   WFLG_RESET_NEWDAT |	// New message data does not exist.
					   FLAG_RESET_MSGVAL |	// Message is invalid.
					   FLAG_RESET_TXIE   |	// Disable transmitter interrupt.
					   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
					   FLAG_RESET_INTPND;	// No interrupt pending.
	}
}	// begin_can1_init


void end_can1_init(void)
{
	*canreg[STATUS]  = 0x00;			//Clear RXOK, TXOK and LEC (Last Error Code).

	*canreg[CONTROL] = 0x02;			//TM=0, CCE=0, CPS=0, EIE=0, SIE=0, IE=1, INIT=0
//CMB
	//XP0IE = ENABLE;						//CAN1 controller interrupt enabled
}	// end_can1_init


void Disable_CAN_Interrupts(void)
{
	//CMB
    //XP0IE = DISABLE;					//CAN1 controller interrupt disabled

	*canreg[CONTROL] = 0x00;			//TM=0, CCE=0, CPS=0, EIE=0, SIE=0, IE=0, INIT=0
}	// Disable_CAN_Interrupts


// Entering of CAN bus off state occurs only if CAN transmit error count is greater than
// or equal to 256. Usually this situation arises if CAN IRMA device(s) using wrong baud
// rate is(are) connected to CAN IRMA bus.

// If no CAN IRMA Sensor a n d no CAN IRMA service device is connected to CAN IRMA bus
// acknowledgement error occurs for each CAN message transmission performed by A21
// firmware. However CAN bus off state will n o t be entered because of changing from
// error active to error passive state on exceeding transmit error count of 127. In
// error passive state transmit error count is n o t incremented on detection of
// acknowledgement error.

//|-------------------------------------------------------------------------------------|
//| Function: can_busoff																|
//|-------------------------------------------------------------------------------------|
//| Input: non																			|
//| Output:	state of bit BOFF of CAN1 controller										|
//|-------------------------------------------------------------------------------------|
boolean can_busoff(void)
{
	byte statusreg;

	if( (statusreg = *canreg[STATUS]) & 0x40 )
		// Bit EWRN is set, if at least one of the error counters equals or exceeds the
		// error warning limit of 96. EWRN is reset, if both error counters are less than
		// the error warning limit.
		A21_Status.CAN1.CAN_error_warnB = TRUE;

	if( statusreg & 0x80 )
	{
		// CAN controller enters bus off state when the TRANSMIT ERROR COUNT is greater
		// than or equal to 256.
		A21_Status.CAN1.CAN_bus_offB = TRUE;
		// BOFF == 1: Bit INIT of CAN control register was set (CAN1 initialization is
		// running) on change BOFF 0 -> 1.
		return(TRUE);
	}
	else
		return(FALSE);
}	// can_busoff


//|-------------------------------------------------------------------------------------|
//| Function: start_can_busoff_recovery_sequence										|
//|-------------------------------------------------------------------------------------|
//| Start CAN bus off recovery sequence by resetting bit INIT of CAN1 control register.	|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void start_can_busoff_recovery_sequence(void)
{
   *canreg[CONTROL] &= 0xFE;
}	// start_can_busoff_recovery_sequence
