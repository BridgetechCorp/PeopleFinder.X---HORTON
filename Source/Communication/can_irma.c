/*==============================================================================================*
 |       Filename: can_irma.c                                                                   |
 | Project/Module: A21 or GATEWAY/module group Communication                                    |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Basic functions for CAN IRMA communication.                                  |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifdef SERIAL_SENSOR
	#error "File can_irma.c included but symbol SERIAL_SENSOR defined"
#endif
#ifdef OBC
	#error "File can_irma.c included but symbol OBC defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167.h>
#include <stddef.h>
#include <string.h>

#include "..\interrupt_defines.h"
#include "..\kernel_interface.h"
#include "communication.h"
#include "can.h"
#include "can_irma.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
//#pragma class PR=COMM
//#pragma class CO=COMM
//#pragma class FB=COMMRAM
//#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
can_addr_id_type analyzer_addr_id;


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/
const can_addr_id_type service_dev_addr_id = { 0, 0, 0 };


/*----- Implementation of Functions ------------------------------------------------------------*/
can_addr_id_type addr_to_can_addr_id(dword addr)
{
	byte high, mid, low;
	can_addr_id_type can_addr_id;

	high = (byte)(addr >> 16);
	mid  = (byte)(addr >>  8);
	low  = (byte)addr;
	
	can_addr_id.high = (mid >> 5) + (high << 3);
	can_addr_id.mid  = (low >> 5) + (mid  << 3);
	can_addr_id.low  =  low << 3;
	
	return(can_addr_id);
}	// addr_to_can_addr_id


dword can_addr_id_to_addr(can_addr_id_type can_addr_id)
{
	dword addr;

	addr  = (dword)can_addr_id.high << 13;
	addr |= (dword)can_addr_id.mid  << 5;
	addr |= (dword)can_addr_id.low  >> 3;
	addr &= 0x0007FFFFUL;

	return(addr);
}	// can_addr_id_to_addr


void determine_analyzer_addr_id(void)
{
	/* Determine CAN IRMA device address of IRMA Analyzer.	*/
	analyzer_addr_id.high = 0x20 +	// bit 18 of CAN identifier == 1 for IRMA Analyzer
	                        ((Comm_ConfigSystem->device_id.year << 1) & 0x1E) +
	                        (byte)(Comm_ConfigSystem->device_id.serial_number >> 13);
	analyzer_addr_id.mid  = (byte)(Comm_ConfigSystem->device_id.serial_number >> 5);
	analyzer_addr_id.low  = (byte)(Comm_ConfigSystem->device_id.serial_number << 3) & 0xF8;
}	// determine_analyzer_addr_id


//|-------------------------------------------------------------------------------------|
//| Function: conf_can_transmit_msg_obj													|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Configure CAN1 transmit message object before first usage.							|
//| Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, pages 18-24 and 18-26.	|
//| To be called during initialization of CAN1 only.									|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void conf_can_transmit_msg_obj(void)
{
	word *can_mo_ctrl;

	// Create 16-bit pointer to transmit message control register.
	can_mo_ctrl = (word *)canreg[(TRA_MSG * MSG_OFFSET) + MSG_CTRL0];

	// Handling of transmit message object flags after initialization:
	// RMTPND: Changed by CAN1 controller only.
	// TXRD:   Set to request transmission of updated message object by CPU,
	//         reset by CAN1 controller.
	// CPUUPD: Reset to enable transmission of updated message object,
	//         set else. As a result no automatic transmission due to
	//         reception of appropriate remote frame possible.
	// NEWDAT: Set on update of message object by CPU,
	//         reset by CAN1 controller before start of transmission.
	// MSGVAL: Not changed to indicate usage of message object.
	// TXIE:   Set by CPU if CAN transmit buffer is not empty,
	//         reset by CPU if CAN transmit buffer is empty.
	// RXIE:   Reset by CPU because there is no need to handle received
	//         remote frames.
	// INTPND: Set by CAN1 controller, reset by CAN interrupt handler.
	*can_mo_ctrl = WFLG_RESET_RMTPND |	// Service of remote frame not pending.
				   WFLG_RESET_TXRQ   |	// Transmission not requested.
				   WFLG_SET_CPUUPD   |	// Update of message object by CPU running.
				   WFLG_RESET_NEWDAT |	// New message data does not exist.
				   FLAG_RESET_MSGVAL |	// Message is invalid.
				   FLAG_RESET_TXIE   |	// Disable transmitter interrupt.
				   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
				   FLAG_RESET_INTPND;	// No interrupt pending.

	// DLC (Data Length Code) = 8,
	// DIR = 1 (transmit message object), XTD = 1 (extended 29-bit identifier)
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_CONFIG] = 0x8C;

	*can_mo_ctrl = WFLG_RMTPND       |
				   WFLG_TXRQ         |
				   WFLG_CPUUPD       |
				   WFLG_NEWDAT       |
				   FLAG_SET_MSGVAL   |	// Message is valid.
				   FLAG_TXIE         |
				   FLAG_RXIE         |
				   FLAG_INTPND;
}	// end of conf_can_transmit_msg_obj


//|-------------------------------------------------------------------------------------|
//| Function: conf_can_receive_msg_objs													|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Configure CAN1 receive message objects before first usage.							|
//| Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, pages 18-25 and 18-27.	|
//| To be called during initialization of CAN1 only.									|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void conf_can_receive_msg_objs(void)
{
	byte rec_mo_cnt, rec_mo_idx, id_h, id_lh, id_ll;
	word *can_mo_ctrl;
	dword sensor_adr;

	// Count of used receive message objects is equal to configured sensor count.
	// Check this number to avoid unintended writing to other memory locations.
	rec_mo_cnt = Comm_ConfigData->sensors;
	if( rec_mo_cnt > 8 )
		rec_mo_cnt = 8;

	for( rec_mo_idx = 0; rec_mo_idx < rec_mo_cnt; rec_mo_idx++ )
	{
		// Create 16-bit pointer to receive message control register.
		can_mo_ctrl = (word *)canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_CTRL0];

		// CAN IRMA device address: bits 18-0 of extended CAN identifier
		sensor_adr = Comm_ConfigData->sensor_config[rec_mo_idx].address;
		id_h  = (byte)((sensor_adr & 0x0007E000) >> 13);
		id_lh = (byte)((sensor_adr & 0x00001FE0) >> 5);
		id_ll = (byte)((sensor_adr & 0x0000001F) << 3);

		// CAN IRMA receiver class = 010 (certain IRMA Analyzer)
		*canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_ARBIT0] = 0x00;			// ID28...21
		*canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_ARBIT1] = 0x80 + id_h;	// ID20...13	
		*canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_ARBIT2] = id_lh;			// ID12...5
		*canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_ARBIT3] = id_ll;			// (ID4...0)<<3

		// DLC (Data Length Code) = 8,
		// DIR = 0 (receive message object), XTD = 1 (extended 29-bit identifier)
		*canreg[((RECS1_MSG + rec_mo_idx) * MSG_OFFSET) + MSG_CONFIG] = 0x84;

		*can_mo_ctrl = WFLG_RMTPND       |
					   WFLG_TXRQ         |
					   WFLG_MSGLST       |
					   WFLG_NEWDAT       |
					   FLAG_SET_MSGVAL   |	// Message is valid.
					   FLAG_TXIE         |
					   FLAG_SET_RXIE     |	// Enable receiver interrupt.
					   FLAG_INTPND;
	}	// end of for( rec_mo_idx = 0; rec_mo_idx < rec_mo_cnt; rec_mo_idx++ )

	id_h  = analyzer_addr_id.high;
	id_lh = analyzer_addr_id.mid;
	id_ll = analyzer_addr_id.low;

	// CAN IRMA receiver class = 010 (certain IRMA Analyzer)
	*canreg[(RECIW_MSG * MSG_OFFSET) + MSG_ARBIT0] = 0x00;			// ID28...21
	*canreg[(RECIW_MSG * MSG_OFFSET) + MSG_ARBIT1] = 0x80 + id_h;	// ID20...13	
	*canreg[(RECIW_MSG * MSG_OFFSET) + MSG_ARBIT2] = id_lh;			// ID12...5
	*canreg[(RECIW_MSG * MSG_OFFSET) + MSG_ARBIT3] = id_ll;			// (ID4...0)<<3	

	// DLC (Data Length Code) = 8,
	// DIR = 0 (receive message object), XTD = 1 (extended 29-bit identifier)
	*canreg[(RECIW_MSG * MSG_OFFSET) + MSG_CONFIG] = 0x84;

	// Create 16-bit pointer to receive message control register.
	can_mo_ctrl = (word *)canreg[(RECIW_MSG * MSG_OFFSET) + MSG_CTRL0];
  	// Flags RMTPND, TXRQ, MSGLST, NEWDAT, TXIE, INTPND unchanged.
	*can_mo_ctrl = WFLG_RMTPND       |
				   WFLG_TXRQ         |
				   WFLG_MSGLST       |
  				   WFLG_NEWDAT       |
  				   FLAG_SET_MSGVAL   |	// Message is valid.
  				   FLAG_TXIE         |
  				   FLAG_SET_RXIE     |	// Enable receiver interrupt.
  				   FLAG_INTPND;

	// CAN IRMA receiver class = xxx (all CAN IRMA messages)
	*canreg[(MSG_15 * MSG_OFFSET) + MSG_ARBIT0] = 0x00;			// ID28...21
	*canreg[(MSG_15 * MSG_OFFSET) + MSG_ARBIT1] = 0x00;  		// ID20...13
	*canreg[(MSG_15 * MSG_OFFSET) + MSG_ARBIT2] = 0x00; 		// ID12...5
	*canreg[(MSG_15 * MSG_OFFSET) + MSG_ARBIT3] = 0x00;			// (ID4...0)<<3

	// DLC (Data Length Code) = 8,
	// DIR = 0 (receive message object), XTD = 1 (extended 29-bit identifier)
	*canreg[(MSG_15 * MSG_OFFSET) + MSG_CONFIG] = 0x84;

	// Create 16-bit pointer to receive message control register.
	can_mo_ctrl = (word *)canreg[(MSG_15 * MSG_OFFSET) + MSG_CTRL0];
	// Flags RMTPND, TXRQ, MSGLST, NEWDAT, TXIE, INTPND unchanged.
	*can_mo_ctrl = WFLG_RMTPND       |
				   WFLG_TXRQ         |
				   WFLG_MSGLST       |
				   WFLG_NEWDAT       |
				   FLAG_SET_MSGVAL   |	// Message is valid.
				   FLAG_TXIE         |
				   FLAG_SET_RXIE     |	// Enable receiver interrupt.
				   FLAG_INTPND;
}	// end of conf_can_receive_msg_objs


void update_can_transmit_message_object(void)
{
	byte len;
	byte n;

	len = can_transmit_buf[can_transmit_buf_read].length & 0xF;

	// Update identifier of message object.
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_ARBIT0] = can_transmit_buf[can_transmit_buf_read].id_hh;
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_ARBIT1] = can_transmit_buf[can_transmit_buf_read].id_hl;
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_ARBIT2] = can_transmit_buf[can_transmit_buf_read].id_lh;
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_ARBIT3] = can_transmit_buf[can_transmit_buf_read].id_ll;

	// DLC (Data Length Code) = len,
	// DIR = 1 (transmit message object), XTD = 1 (extended 29-bit identifier)
	*canreg[(TRA_MSG * MSG_OFFSET) + MSG_CONFIG] = (len << 4) + 0xC;

	// Copy message data bytes to message object.
	for( n = 0; n < len; n++ )
		*canreg[(TRA_MSG * MSG_OFFSET) + MSG_DATA + n] = can_transmit_buf[can_transmit_buf_read].data[n];

	NEXT(can_transmit_buf_read, CAN_TRANSMIT_BUF_MSG_CNT);
}	// update_can_transmit_message_object


//|-------------------------------------------------------------------------------------|
//| Function: can_transmit																|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Update transmit message object if no transmission running and request transmission	|
//| afterwards.																			|
//| Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, pages 18-24 and 18-26.	|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void can_transmit(void)
{
	word *can_mo_ctrl;

	// Create 16-bit pointer to transmit message control register.
	can_mo_ctrl = (word *)canreg[(TRA_MSG * MSG_OFFSET) + MSG_CTRL0];

	// Hints:
	// - Consistency of CAN transmit message object on update is guaranteed by temporary
	//   setting of CAN transmit message object flag CPUUPD (during call of function
	//   update_can_transmit_message_object). 
	// - Flag TXIE must n o t be resetted as long as CAN transmission is running because
	//   flag INTPND of CAN transmit message object is not set if TXIE is reset (01).
	//   Otherwise transmission finish (TXRQ 1->0) interrupts could be missing.
	// - There is no need to temporarily disable CAN1 interrupts.
	//   * Flag CPUUPD is not set (01) on query:
	//     It does not matter if transmission finish (TXRQ 1->0) interrupt is launched
	//     sooner or later after query of flag CPUUPD.
	//   * Flag CPUUPD is set (10) on query:
	//     Transmission finish (TXRQ 1->0) interrupt cannot be launched during runtime
	//     of function can_transmit because CAN transmission is finished.
	//   During development of IRMA Opera version 6.00 temporary disabling of CAN1
	//   interrupts was used due to insufficient consideration of facts mentioned above.
	//   Bit IE (Interrupt Enable) of CAN1 control register was reset before CPUUPD
	//   query and set again immediately before return from function. However function
	//   can_transmit may be interrupted by timer and time-out jobs. Hence CAN1 
	//   interrupt disabling time could as long as several hundreds of microseconds
	//   if e.g. timer job check_doors_RL4 was processed. As a result CAN receive
	//   messages could be lost.

	// Check if CAN transmit message object flag CPUUPD is set.
    if( (*canreg[(TRA_MSG * MSG_OFFSET) + MSG_CTRL1] & FLAG_SET_CPUUPD) == FLAG_SET_CPUUPD )
	{
		// Flag CPUUPD is set (10), transmission not running but started now.
		*can_mo_ctrl = WFLG_RMTPND       |
					   WFLG_TXRQ         |
					   WFLG_SET_CPUUPD   |	// Update of message object by CPU running.
					   WFLG_SET_NEWDAT   |	// New message data exist.
					   FLAG_MSGVAL	     |
					   FLAG_TXIE         |
					   FLAG_RXIE         |
					   FLAG_INTPND;

		update_can_transmit_message_object();

		*can_mo_ctrl = WFLG_RMTPND       |
					   WFLG_SET_TXRQ     |	// Transmission requested.
					   WFLG_RESET_CPUUPD |	// Update of message object by CPU finished.
					   WFLG_NEWDAT       |
					   FLAG_MSGVAL       |
					   FLAG_SET_TXIE     |	// Enable transmitter interrupt.
					   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
					   FLAG_INTPND;
	}
	else
		// Flag CPUUPD is not set (01), transmission running. Transmission of buffered
		// CAN messages will be continued by CAN interrupt handler.
		*can_mo_ctrl = WFLG_RMTPND       |
					   WFLG_TXRQ         |
					   WFLG_CPUUPD       |
					   WFLG_NEWDAT       |
					   FLAG_MSGVAL       |
					   FLAG_SET_TXIE     |	// Enable transmitter interrupt.
					   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
					   FLAG_INTPND;
}	//can_transmit


void can_transmit_buf_write_update(void)
{
    NEXT(can_transmit_buf_write, CAN_TRANSMIT_BUF_MSG_CNT);
    can_transmit();	// Transmit CAN message.
}	// can_transmit_buf_write_update


// Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, pages 18-9 and 18-10 and to
// Infineon AP292401 "Interrupt Register behavior of the CAN module in Siemens 16-bit
// Microcontrollers".
// Delay from reset of bit INTPND by CPU to update of byte INTID by CAN1 controller may
// be as long as 26 CAN clock cycles under worst case conditions. If INTID is queried
// to early same CAN interrupt request is serviced again accidentally. Furthermore
// service waiting time is lenthened for other interrupt requests pending. Because
// A21 firmware always uses setting CPS == 0 26 CAN clock cycles are equal to 52 CPU
// clock cycles.
//   Worst case condition does not apply to A21 firmware because there are no consecutive
// CPU accesses to CAN1 memory between INTPND reset and following INTID query. However
// storage of newly received CAN message may occur within period in question.
// - fCPU == 16.0000MHz, TCPU == 62,5ns, maximum INTID update delay time == 3,25탎
// - 06/06/2008:
//   Minimum runtime between INTPND reset and INTID update is applicable for transmission
//   finish (TXRQ 1->0) interrupts. For IRMA Opera version 6.00 this time was determined
//   using TRACE32 emulator: about 3탎. It should be sufficiently high because worst
//   case does not apply.
// - 07/03/2008:
//   Code to check TXOK and to set A21_Status.CAN1.Tra_Msg_InvalidB added. Runtime
//   increased.
// 
// Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, pages 18-24 to 18-29 and to
// Infineon AP292201 "'C' CAN Driver Routines for the C166 Family", source files
// "CISR16X1.C", "RDM1516X.C", "RDMOD16X.C" and "SNDMO16X.C".
// It is basic concept of A21 firmware to minimize time between reception of CAN message
// indicated by transition NEWDAT 0 -> 1 and following complete readout of according
// CAN receive message object. This CAN receive message service time is short enough to
// handle worst case of message reception rate:
// 1Mbps, messages without data bytes, no bit stuffing, minimum pause between messages
// (intermission of 3 recessive bits), resulting time between two subsequent message
// receptions = 64탎 + 3탎 = 67탎
// Sum of CANINT_handler runtime (while loop for INTID query passed only once) and
// maximum CAN interrupt disabling time has to be lower than 67탎.
// So in contrast to implementation of function "rd_modata_16x" bits NEWDAT and INTPND
// are resetted a f t e r reading of respective CAN receive message object data bytes.
//   Steps of readout of CAN receive message object specified by Infineon AP292201
// "'C' CAN Driver Routines for the C166 Family":
// 1. Evaluate CAN identifier if necessary.
// 2. Readout DLC.
// 3. Reset flags NEWDAT and INTPND.
// 4. Readout message data bytes.
// 5. Check if flag NEWDAT is set.
// Motivation for early reset of flags NEWDAT and INTPND:
// - INTPND:
//	 Avoidance of impact of worst case INTID update delay time as mentioned above.
// - NEWDAT:
//   Enabling of message data bytes consistency check. Refer to Infineon C167CR
//   Derivatives User큦 Manual, V3.2, pages 18-20, remark 2).
// Message data bytes consistency check is not performed by A21 firmware because
// it would increase CANINT_handler runtime substantially. Furthermore minimized
// runtime prevents data inconsistency.
//
// 06/01/2008:
// Questions not answered by flow chart at page 18-25 of Infineon C167CR Derivatives
// User큦 Manual, V3.2:
// - At what time check NEWDAT == 1 ? is made by CAN state machine ?
// - At what time DLC and data bytes are updated by CAN state machine ?
// - At what time transition NEWDAT 0 -> 1 is made by CAN state machine ?
// Best case:
// All actions are executed immediately one after the other.
// Worst case:
// Check NEWDAT == 1 ? (and possible set of MSGLST) is already made if CAN identifier is
// received and transition NEWDAT 0 -> 1 is made as late as message reception is finished,
// i.e. end of frame checked for Form Error.

// 06/17/2008:
// - CANINT_handler runtime (while loop for INTID query passed only once) measured using
//   TRACE32 emulator:
//   * IRMA Opera version 5.11:
//     about 32탎
//     for receive message objects 1 and 15, DLC == 0 
//   * IRMA Opera version 5.11:
//     26.6탎 ... 26.8탎 
//     for receive message objects 2 and 3, independent of DLC
//   * IRMA Opera version 6.00:
//     27.7탎 ... 28.0탎
//     for receive message objects 1, 2 ... 9 and 15, independent of DLC
// - Assignment NEWDAT := 0 performed b e f o r e reading CAN identifier and DLC from CAN
//   receive message object will corrupt these parameters:
//   CAN identifier := 0 and DLC := 8.

//|-------------------------------------------------------------------------------------|
//| Function: CANINT_handler					 										|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Interrupt service routine (ISR) for CAN1 controller.								|
//| Default register bank 0 is replaced by register bank CAN_BASE_RB for ISR runtime.	|
//| Register banks 1 ... 3 are assigned to ISR register banks in the order as source 	|
//| files are listed in Tasking EDE project file.										|
//| Typically register bank 3 (start address 0xF630) is used as CAN_BASE_RB.			|
//| Received CAN messages are transferred from CAN receive message object 1, 2 ... 9	|
//| or 15 to CAN receive buffer.														|
//| CAN transmit message object 14 is updated if message is pending in CAN transmit		|
//| buffer.																				|
//|-------------------------------------------------------------------------------------|
/*interrupt (CANINT) void CANINT_handler(void)
{
	byte intreg;
	byte stareg;
	byte *write_ptr;
	word *can_mo_ctrl;

	#pragma asm
	WFLG_RMTPND			EQU	0C000h
	WFLG_TXRQ			EQU	03000h
	WFLG_MSGLST			EQU	00C00h
	WFLG_RESET_NEWDAT	EQU	00100h
	FLAG_MSGVAL			EQU	0C0h
	FLAG_RESET_TXIE		EQU	010h
	FLAG_SET_RXIE		EQU	008h
	FLAG_RESET_INTPND	EQU	001h
	QUIT_RECINT	EQU	(WFLG_RMTPND | WFLG_TXRQ | WFLG_MSGLST | WFLG_RESET_NEWDAT | FLAG_MSGVAL | FLAG_RESET_TXIE | FLAG_SET_RXIE | FLAG_RESET_INTPND)
	#pragma endasm

	while( (intreg = *canreg[INT_REG]) != 0 )
	{
		// Read BOFF, EWRN, RXOK, TKOK and LEC.
		// Refer to Infineon C167CR Derivatives User큦 Manual, V3.2, page 18-8.
		stareg = *canreg[STATUS];
		// Reset RXOK, TXOK and LEC. State of bits BOFF and EWRN not influenced.
		*canreg[STATUS] = 0;
		switch( intreg )
		{
			// Comments regarding all CAN receive message objects:
			// Memory block copy (16-bit) used for runtime minimization. Masking of
			// bits DIR and XTD as well as DLC nibble shift done in function
			// process_can_irma_message.
			// Variable @i1 uses DPP2 == C166_DGROUP == 0x03, set by interrupt frame.
			// Variable @i2 uses DPP0 == 0x23 (memory class COMMRAM at 0x08C000-0x08EFFF).

			// message object 15 interrupt, any CAN identifier
			// Handling of alternating buffer by CPU was not considered. Refer to Infineon
			// C167CR Derivatives User큦 Manual, V3.2, pages 18-28 and 18-29, to Infineon
			// AP292201 "'C' CAN Driver Routines for the C166 Family", source file
			// "RDM1516X.C" and to Infineon AP16021 "CAN Interrupt Structure", chapter 6.
			case 2:
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AFF2h
					Loop15:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AFFEh
					JMPR cc_ULT,Loop15
					MOV @i1,#0AFF0h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 1 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address of IRMA Analyzer
			case (RECIW_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF12h
					Loop0:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF1Eh
					JMPR cc_ULT,Loop0
					MOV @i1,#0AF10h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 2 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 1
			case (RECS1_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF22h
					Loop1:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF2Eh
					JMPR cc_ULT,Loop1
					MOV @i1,#0AF20h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 3 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 2
			case (RECS2_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF32h
					Loop2:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF3Eh
					JMPR cc_ULT,Loop2
					MOV @i1,#0AF30h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 4 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 3
			case (RECS3_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF42h
					Loop3:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF4Eh
					JMPR cc_ULT,Loop3
					MOV @i1,#0AF40h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 5 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 4
			case (RECS4_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF52h
					Loop4:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF5Eh
					JMPR cc_ULT,Loop4
					MOV @i1,#0AF50h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 6 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 5
			case (RECS5_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF62h
					Loop5:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF6Eh
					JMPR cc_ULT,Loop5
					MOV @i1,#0AF60h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 7 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 6
			case (RECS6_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF72h
					Loop6:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF7Eh
					JMPR cc_ULT,Loop6
					MOV @i1,#0AF70h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 8 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 7
			case (RECS7_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF82h
					Loop7:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF8Eh
					JMPR cc_ULT,Loop7
					MOV @i1,#0AF80h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			// message object 9 interrupt,
			// CAN IRMA receiver class = 010 (certain IRMA Analyzer),
			// CAN IRMA device address: logical address or device address of IRMA Sensor 8
			case (RECS8_MSG + 2):
				#pragma asm( @i1,@i2=can_receive_buf_write_pof,@w3=can_receive_buf_write_pag,@w4,@b5=stareg )
					PUSH @w3
					POP DPP0
					MOV @i1,#0AF92h
					Loop8:
					MOV [@i2+],[@i1]
					CMPI2 @i1,#0AF9Eh
					JMPR cc_ULT,Loop8
					MOV @i1,#0AF90h
					MOV [@i2],[@i1]
					MOV @w4,#QUIT_RECINT
					MOV [@i1],@w4
					SUB @i2,#1
					MOVB [@i2],@b5
				#pragma endasm

				break;

			case (TRA_MSG + 2):	// Transmit Interrupt
				// Create 16-bit pointer to transmit message control register.
				can_mo_ctrl = (word *)canreg[(TRA_MSG * MSG_OFFSET) + MSG_CTRL0];

				// Something else to send ?
				if( can_transmit_buf_write != can_transmit_buf_read )
				{
					*can_mo_ctrl = WFLG_RMTPND       |
								   WFLG_TXRQ         |
								   WFLG_SET_CPUUPD   |	// Update of message object by CPU running.
								   WFLG_SET_NEWDAT   |	// New message data exist.
								   FLAG_MSGVAL       |
								   FLAG_TXIE         |
								   FLAG_RXIE         |
								   FLAG_INTPND;

					update_can_transmit_message_object();

					*can_mo_ctrl = WFLG_RMTPND       |
								   WFLG_SET_TXRQ     |	// Transmission requested.
								   WFLG_RESET_CPUUPD |	// Update of message object by CPU finished.
								   WFLG_NEWDAT       |
								   FLAG_MSGVAL       |
								   FLAG_SET_TXIE     |	// Enable transmitter interrupt.
								   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
								   FLAG_RESET_INTPND;	// No interrupt pending.
				}
				else
					*can_mo_ctrl = WFLG_RMTPND       |
								   WFLG_TXRQ         |
								   WFLG_SET_CPUUPD   |	// Update of message object by CPU running.
								   WFLG_NEWDAT       |
								   FLAG_MSGVAL       |
								   FLAG_RESET_TXIE   |	// Disable transmitter interrupt.
								   FLAG_RESET_RXIE   |	// Disable receiver interrupt.
								   FLAG_RESET_INTPND;	// No interrupt pending.

				// Flag TXOK == 0 for transmitted CAN message ?
				if( (stareg & 0x08) != 0x08 )
					// Yes, set according A21 CAN1 status flag.
					A21_Status.CAN1.Tra_Msg_InvalidB = TRUE;

				break;

			default:
				break;
		}	//	end of "switch( intreg )"

		// CAN message received by one of the receive message objects. Update
		// CAN receive buffer write pointer and related variables.
		if( intreg >= 2	&& intreg <= RECS8_MSG + 2 )
		{
			can_receive_buf_write += CAN_MSG_OBJ_LEN;
			if( can_receive_buf_write >= CAN_RECEIVE_BUF_LEN )
				can_receive_buf_write = 0;
			if( can_receive_buf_write == can_receive_buf_read )
				A21_Status.CAN1.Buffer_OverflowB = TRUE;
			// Calculation of receive buffer address for next message to be received:
			write_ptr = &can_receive_buf[0];
			write_ptr += can_receive_buf_write;
			can_receive_buf_write_pag =	_pag(write_ptr);
			can_receive_buf_write_pof = _pof(write_ptr);
		}	//	end of "if( intreg >= 2	&& intreg <= RECS8_MSG + 2 )"
	}	// end of "while( (intreg = *canreg[INT_REG]) != 0 )"

	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// CANINT_handler
*/