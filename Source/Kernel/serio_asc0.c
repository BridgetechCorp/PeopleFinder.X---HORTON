/*==============================================================================================*
 |       Filename: serio_asc0.c                                                                 |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of basic I/O functions for communication via RS-232 service   |
 |                 interface or system interface (IBIS, RS-485, J1708) using serial port ASC0.  |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>
#include "..\interrupt_defines.h"
#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#include "serio_asc0.h"
#include "time.h"
#ifdef J1708_PROTOCOL
	#include "main.h"
#endif


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#ifdef USE_T3OUT
	// Configure P3.3 as output and use alternative pin function T3OUT to output state of
	// T3CON bit T3OTL.
	#define DP3_MASK_ASC0	0x0CAA		// Relevant: P3.1, 3, 5, 7, 10, 11								
	#define DP3_VALUE_ASC0	0x048A		// Outputs: P3.1, 3, 7, 10
#else
	#define DP3_MASK_ASC0	0x0CA2		// Relevant: P3.1, 5, 7, 10, 11								
	#define DP3_VALUE_ASC0	0x0482		// Outputs: P3.1, 7, 10
#endif
#define P4_CTS			4			/* P4.4 = CTS												*/
#define P4_RTS			7			/* P4.7 = RTS												*/
#define DP4_RTS_CTS		0x0080		/* CTS = input, RTS = output								*/
#define MSK_RTS_CTS		0x0090		/* DP4.7 = RTS, DP4.4 = CTS									*/
#define S0STP			0x0008		/* Stop Bit Selection Offset, bit 3							*/
#define S0ODD			0x0100		/* Parity Selection Bit Offset, bit 12						*/
#define STXEN			1			// P3.1: signal /TXEN = activation of RS-485 driver of system interface
#define SEBELEG			5			// P3.5: signal SEBELEG = RS-232 service interface occupied
#define SYFREIGC167		7			// P3.7: signal /SYFREIGC167 = commu. via system interface forced by softw.

#define S0TIE_Mask		0x40 
#define S0TBIE_Mask		0x40 

#define ASC0_ON			0
#define ASC0_OFF		1

#ifdef J1708_PROTOCOL
	#define J1708SendCrashCnt_Random	3	// Refer to SAE J1708, appendix B.1.1.
	#define MaxJ1708SendMsgCnt			20
#endif


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear 


/*----- Global Variables -----------------------------------------------------------------------*/
char Current_ASC0_Prot_Id;	// Constant PROT_ID_UNKNOWN is equal to -1.

// Effect of pragma volatile:
// As a result of optimization variables are stored in CPU registers during function runtime
// as long as possible. This optimization is not made for variables declared with pragma volatile.
// In this case variable content is loaded from memory to register immediately before any
// variable access. Usage of pragma volatile yields greater code size and longer runtime but it
// is essential for variables changed by interrupt service routine (ISR) and polled outside ISR.
// Example using global variable index changed by ISR and polled outside ISR:
//   index = 1;
//   do
//     P7 ^= 0xFFFF;
//   while( index == 1 );
// Code generated if pragma volatile is not used for declaration of variable index:
//  0x0000:  MOVB    RL2,#0x1
//  0x0002:  MOVB    0x3d7c,RL2
//  0x0006:  XOR     P7,#0xffff
//  0x000a:  JMPR    cc_UC,0x0006
// Code generated if pragma volatile is used for declaration of variable index:
//  0x0000:  MOVB    RL2,#0x1
//  0x0002:  MOVB    0x3d7c,RL2
//  0x0006:  XOR     P7,#0xffff
//  0x000a:  MOV     DPP0,#0x20
//  0x000e:  NOP     
//  0x0010:  MOVB    RL1,0x3d7c
//  0x0014:  CMPB    RL1,#0x1
//  0x0016:  JMPR    cc_Z,0x0006
// However usage of pragma volatile does n o t eliminate necessity to ensure data consistency
// by utilization of uninterruptable program sequences ! For example simple increment of 
// variable is n o t interrupt save:
//   MOV     R12,0x3f5a
//   ADD     R12,#0x1
//   MOV     0x3f5a,R12
byte bSerialOutBuffer[SER_OUT_BUF_SIZE];
volatile word wSerialOutBuffer_read;				// Changed in ISR: Pragma volatile necessary.
word wSerialOutBuffer_write;
volatile word ASC0OutBufferCharCnt;					// Changed in ISR: Pragma volatile necessary.
volatile word LocEchoCharCnt;						// Changed in ISR: Pragma volatile necessary.
byte bSerialInBuffer[SER_IN_BUF_SIZE];
// Pointer wSerialInBuffer_read points to last byte read from ASC0 input buffer bSerialInBuffer.
volatile word wSerialInBuffer_read;					// Changed in ISR: Pragma volatile necessary.
// Pointer wSerialInBuffer_write points to last byte written to ASC0 input buffer bSerialInBuffer.
volatile word wSerialInBuffer_write;				// Changed in ISR: Pragma volatile necessary.
word wSerialInBuffer_timeoutref;

// Functions Change_ASC0_Debug_Mode and runlevel_1_jobs are used to change flags
// ASC0_Service_Interface_OccupiedB and ASC0_Debug_ModeB while keeping their consistency.
// Always query these flags rather then call functions ASC0_Service_Interface_Occupied or
// ASC0_System_Interface_Forced !
bool ASC0_Service_Interface_OccupiedB;
bool ASC0_Debug_ModeB;

Parser_type Parser;
bool RS232_7bit_ActiveB;
word WriteData_ASC0_TO_ms;			 				// Function WriteData_ASC0: Time-out in ms for
								 					// writing data into output buffer
													// bSerialOutBuffer[SER_OUT_BUF_SIZE].
bool ToUseSTXENB;
bool STXENForcedB;
volatile bool ASC0TransmChainActiveB;				// Changed in ISR: Pragma volatile necessary.
volatile bool ASC0TransmReleasedB;					// Changed in ISR: Pragma volatile necessary.
volatile bool ASC0TxDActiveB;						// Changed in ISR: Pragma volatile necessary.
bool ToUseSendDelayB;
volatile bool ASC0RecErrB;							// Changed in ISR: Pragma volatile necessary.
volatile bool LocalEchoReceivedB;					// Changed in ISR: Pragma volatile necessary.

typedef struct {
	word s0bg;
	word s0con;
} ASC0_Settings_type;

ASC0_Settings_type stASC0_settings;

RS232_Parameter_type stASC0_actual_parameter;
RS232_Parameter_type stRS232_IRMA_Standard;

ASC0Error_type far ASC0Error;

word T3Ticks_ASC0BitTime;

#ifdef J1708_PROTOCOL
typedef struct {
	word T3Ticks_J1708TimeOffs;
	byte J1708SendMsgLen;
	byte J1708SendMsgLenDCnt;
	byte J1708SendCrashCnt;
} J1708SendMsgBuff_type;

word T1_RXD1Sync;

word T3Ticks_J1708CharTime;

volatile word J1708RecMsgStartBuff;					// Changed in ISR: Pragma volatile necessary.

byte J1708SendMsgWrite;
volatile byte J1708SendMsgRead;						// Changed in ISR: Pragma volatile necessary.
volatile byte J1708SendMsgCnt;						// Changed in ISR: Pragma volatile necessary.
volatile J1708SendMsgBuff_type J1708SendMsgBuff[MaxJ1708SendMsgCnt]; 	// Changed in ISR.

volatile byte LocEchoChar;							// Changed in ISR: Pragma volatile necessary.

volatile word wSerialOutBuffer_read_Buff;			// Changed in ISR: Pragma volatile necessary.
volatile word ASC0OutBufferCharCntBuff;				// Changed in ISR: Pragma volatile necessary.

// Certain variables have alternative meaning if J1708 communication is initialized:
// - Variable "ASC0TransmChainActiveB" is permanently set to TRUE to keep function "Check_Send_Data_ASC0"
//   from starting transmission if ASC0 transmission buffer "bSerialOutBuffer" is not empty.
// - Variable "ASC0TransmReleasedB" indicates that J1708 transmission control is active. State TRUE
//   is set if dectection of bus access time for next transmission message is running or if message
//   transmission is going on. 
// - Variable "ASC0TxDActiveB" indicates ongoing message transmission. It is set to TRUE while writing
//   first character of transmission message into S0TBUF. It is reset to FALSE after receiving of local
//   echo of last character of transmission message or after transmission of last character has been
//   completed (S0TIR == TRUE) depending on whether local echo is expected or not.
void (*S0TInt_Func)(void);
void (*T3Int_Func)(void);
void (*S0RInt_Func)(void);
#endif	// end of "#ifdef J1708_PROTOCOL"


/*----- Function Prototypes --------------------------------------------------------------------*/
#ifdef J1708_PROTOCOL
void S0TINT_handler_J1708(void);
void S0TINT_handler_def(void);
void T3INT_handler_J1708_Idle(void);
void T3INT_handler_J1708_Send(void);
void T3INT_handler_J1708_Echo(void);
void T3INT_handler_def(void);
void S0RINT_handler_J1708(void);
void S0RINT_handler_def(void);
#endif	// end of "#ifdef J1708_PROTOCOL"

word WriteData_ASC0(const byte *data, word size);


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
// Initialize global variables not properly initialized by function "clear_kernel_RAM".
void init_global_var_serio_asc0(void)
{
	// Communication protocol used on interface ASC0 undefined.
	Current_ASC0_Prot_Id = PROT_ID_UNKNOWN;

	WriteData_ASC0_TO_ms = 1000;

	stRS232_IRMA_Standard.baudrate     = 38400;
	stRS232_IRMA_Standard.data_bits    = 8;
	stRS232_IRMA_Standard.parity       = 'N';
	stRS232_IRMA_Standard.stop_bits    = 1;
	stRS232_IRMA_Standard.flow_control = 0;
}	// init_global_var_serio_asc0


void InitP3P4ForASC0(void)
{
	Save_ILVL_and_Disable_All_INTs();
	_bfld( P3, DP3_MASK_ASC0, DP3_MASK_ASC0);  	// Set P3 outputs = 1.
	_bfld(DP3, DP3_MASK_ASC0, DP3_VALUE_ASC0); 	// Set direction bits for P3.
	_putbit(ASC0_ON, P4, P4_RTS);				// Set RTS output = 0.
	_bfld(DP4, MSK_RTS_CTS, DP4_RTS_CTS);		// Set direction bits for P4.
	S0BG  = 0;									// Deactivate ASC0.
	S0CON = 0;									//
	A21_Status.ASC0.Out_OverflowB = FALSE;		// Initialize A21_Status.ASC0. In contrast to
	A21_Status.ASC0.In_OverflowB  = FALSE;		// ASC0Error this initialization is done only once
	A21_Status.ASC0.OverrunErrorB = FALSE;		// during software start-up.
	A21_Status.ASC0.FrameErrorB   = FALSE;		//
	A21_Status.ASC0.ParityErrorB  = FALSE;		//
	Restore_ILVL();

#ifdef DEBUG_START_RS232_DEBUG_MODE
	Change_ASC0_Debug_Mode(TRUE);
#endif
}	// InitP3P4ForASC0


void ActivateASC0SysTransm(void)
{
    // Signal /TXEN is used for activation of RS-485 driver of system interface if RS-232 service 
    // interface is not occupied or if RS-232 service interface is occupied and ASC0 communication
    // via system interface was forced by software.
	if( !_getbit(P3, SEBELEG) || !_getbit(P3, SYFREIGC167) )
	{
		_putbit(FALSE, P3, STXEN);
	    // Signal /TXEN to be continously activated instead of automatic control during transmission.
		ToUseSTXENB = FALSE;
		STXENForcedB = TRUE;
	}
}	// ActivateASC0SysTransm


void DeactivateASC0SysTransm(void)
{
	if( STXENForcedB )
	{
	    // Signal /TXEN is disabled immediately only if no transmission is running on ASC0.
		// Otherwise it will be disabled by ISR function T3INT_handler on end of transmission.
		Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
		if( !ASC0TxDActiveB )
			_putbit(TRUE, P3, STXEN);
		else
			ToUseSTXENB = TRUE;
		Restore_ILVL();
		STXENForcedB = FALSE;
	}
}	// DeactivateASC0SysTransm


void ActivateASC0SendDelay(void)
{
	ToUseSendDelayB = ConfigData.ASC0_control.half_duplexB;
}	// ActivateASC0SendDelay


void DeactivateASC0SendDelay(void)
{
	ToUseSendDelayB = FALSE;
}	// DeactivateASC0SendDelay


dword ASC0_baudrate(dword desired_baudrate, word *bg_ptr)
{
	word bg;
	dword ModuloVar;
	dword actual_baudrate;

	if( fCPU_div_32 >= desired_baudrate )
	{
		// Baudrate = fCPU / 32 / (bg + 1)
		bg = fCPU_div_32 / desired_baudrate;

		ModuloVar = fCPU_div_32 - bg * desired_baudrate;
		if( ModuloVar < desired_baudrate / 2 )
			bg--;

		actual_baudrate = fCPU_div_32 / (bg + 1);
		if( fCPU_div_32 - actual_baudrate * (bg + 1) > bg / 2 )
			actual_baudrate++;

		*bg_ptr = bg;
		return(actual_baudrate);
	}
	else
	{
		*bg_ptr = 0;
		return(0);
	}
}	// ASC0_baudrate


signed long ASC0_baudrate_dev(dword actual_baudrate, dword desired_baudrate)
{
	signed long dev;

	// Calculate baud rate deviation in ppm to avoid usage of real numbers.
	dev = 1000000 * ((signed long)actual_baudrate - (signed long)desired_baudrate);
	// Explicit type cast needed for compilation of 32-bit signed division.
	dev /= (signed long)desired_baudrate;
	return(dev);
}	// ASC0_baudrate_dev


bool ASC0_baudrate_valid(dword desired_baudrate)
{
	dword actual_baudrate;
	word bg;
	signed long dev;

	actual_baudrate = ASC0_baudrate(desired_baudrate, &bg);
	if( actual_baudrate > 0 )
	{
		// Calculate baud rate deviation in ppm to avoid usage of real numbers.
		dev = ASC0_baudrate_dev(actual_baudrate, desired_baudrate);
		// Deviation from desired baud rate has to be < 1%.
		if( dev >= 0 )
			return(dev < 10000);
		else
			return(dev > -10000);
	}
	else
		return(FALSE);
}	// ASC0_baudrate_valid


// Function compute_ASC0_settings always returns with the following S0CON settings:
// S0R		= 1
// S0LB		= 0
// S0BRS	= 0
// S0ODD	= 0
// Reserved	= 0
// S0OE		= 0
// S0FE		= 0
// S0PE		= 0
// S0OEN	= 1
// S0FEN	= 1
// S0PEN	= variable
// S0REN	= 1
// S0STP	= variable
// S0M		= variable
// Return value of function compute_ASC0_settings ( = actual baudrate) is equal to 0 under the
// following conditions:
// - parameter->baudrate == 0 or
// - ConfigSystem.fCPU is invalid.
dword compute_ASC0_settings(RS232_Parameter_type *parameter, ASC0_Settings_type *settings)
{
	word con;
	word bg;
	dword actual_baudrate = 0;

	con = 0x80D1;	// Default:	S0R = 1, S0LB = 0, S0BRS = 0, S0ODD	= 0,
					//			Res. = 0, S0OE = 0, S0FE = 0, S0PE = 0,
					//			S0OEN = 1, S0FEN	= 1, S0PEN = 0, S0REN = 1,
					//			S0STP = 0, S0M = 001 (8-bit data, no parity)
	if( !fCPU_div_32_validB || parameter->baudrate == 0 )
		bg = 52;	// Default baud rate if no other found: 9600 bps at 16 MHz
	else
	{
		actual_baudrate = ASC0_baudrate(parameter->baudrate, &bg);

		if( parameter->data_bits == 7 )
			con = 0x80F3;	// 7-bit data always with parity !
		else 
			if( parameter->data_bits == 8 && (parameter->parity == 'E' || parameter->parity == 'O') )
				con = 0x80F7;

		if( parameter->stop_bits == 2 )
			con |= S0STP;
		if( parameter->parity == 'O' )
			con |= S0ODD;
	}

	settings->s0bg = bg;
	settings->s0con = con;
	return(actual_baudrate);
}	// compute_ASC0_settings


word InitASC0(RS232_Parameter_type *parameter, byte prot_id)
{
	word *ASC0StatusPtr;

	ConfigData.ASC0_real.baudrate = compute_ASC0_settings(parameter, &stASC0_settings);
	ConfigData.ASC0_real.s0bg = stASC0_settings.s0bg;
	// Calculate baud rate deviation in ppm to avoid usage of real numbers.
	ConfigData.ASC0_real.deviation = 
		ASC0_baudrate_dev(ConfigData.ASC0_real.baudrate, parameter->baudrate);
	RS232_7bit_ActiveB = parameter->data_bits == 7;
	stASC0_actual_parameter.baudrate = parameter->baudrate;
	stASC0_actual_parameter.data_bits = parameter->data_bits;
	stASC0_actual_parameter.parity = parameter->parity;
	stASC0_actual_parameter.stop_bits = parameter->stop_bits;
	stASC0_actual_parameter.flow_control = parameter->flow_control;
	Current_ASC0_Prot_Id = (char)prot_id;
	Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
	T3IE = DISABLE;
	#ifdef USE_T3OUT
	T3CON = 0x02C2;						// T3 counts down with clock = fCPU / 32
	#else
	T3CON = 0x00C2;						// T3 counts down with clock = fCPU / 32
	#endif
	S0R = DISABLE;						// Disable ASC0 baud generator.
	S0BG  = stASC0_settings.s0bg;		// Set ASC0 baud rate.
	S0CON = stASC0_settings.s0con;		// Set ASC0 control register.
	S0RIR = FALSE;
	// S0RIC: Receive interrupt control register is configured in configuration.c
	S0RIE = DISABLE;
	// S0EIC: Error interrupt control register is configured in configuration.c
	S0EIE = DISABLE;
	// S0TBIC: Transmit interrupt control register is configured in configuration.c
	S0TBIE = DISABLE;
	// S0TIC: Transmit interrupt control register is configured in configuration.c
	S0TIE = DISABLE;
	Restore_ILVL();
	ToUseSTXENB = FALSE;				// /TXEN output is not used by default.
	STXENForcedB = FALSE;
	ToUseSendDelayB = ConfigData.ASC0_control.half_duplexB;
	ASC0TransmReleasedB    = FALSE; 	// Transmission not released, send delay to be considered.
	ASC0TransmChainActiveB = FALSE; 	// No transmission going on.
	ASC0TxDActiveB         = FALSE;		//
	ASC0RecErrB            = FALSE;
	LocalEchoReceivedB     = FALSE; 	// Local echo receivable only during transmission.
	LocEchoCharCnt         = 0;
	//
	ASC0Error.Out_OverflowB = FALSE;
	ASC0Error.In_OverflowB  = FALSE;
	ASC0Error.OverrunErrorB = FALSE;
	ASC0Error.FrameErrorB   = FALSE;
	ASC0Error.ParityErrorB  = FALSE;
	//
	wSerialOutBuffer_read  = SER_OUT_BUF_SIZE - 1;
	wSerialOutBuffer_write = SER_OUT_BUF_SIZE - 1;
	ASC0OutBufferCharCnt   = 0;
	wSerialInBuffer_read       = SER_IN_BUF_SIZE - 1;
	wSerialInBuffer_write      = SER_IN_BUF_SIZE - 1;
	wSerialInBuffer_timeoutref = SER_IN_BUF_SIZE - 1;
	//
	Parser.terminal_symbol        = '\0';
	Parser.call_terminalB         = FALSE;
	Parser.call_checksumB         = FALSE;
	Parser.call_terminal_function = NULL;
	Parser.call_checksum_function = NULL;
	Parser.timeout                = 0;
	//
	T3Ticks_ASC0BitTime = stASC0_settings.s0bg + 1;

#ifdef J1708_PROTOCOL
	// Signal /TXEN (P3.1) has to be = 0 to open gate U301 for signal TXDTTLSY on LPBG-A21-J1xx.
	_putbit(FALSE, P3, STXEN);
	T3Ticks_J1708CharTime = T3Ticks_ASC0BitTime * 10;
	if( prot_id == PROT_ID_J1708 )
	{
	    S0TInt_Func = No_Job_Kernel;
		T3Int_Func = No_Job_Kernel;
		S0RInt_Func = S0RINT_handler_J1708;
		J1708RecMsgStartBuff = SER_IN_BUF_SIZE - 1;
		J1708SendMsgWrite =	0;
		J1708SendMsgRead = 0;
		J1708SendMsgCnt = 0;
		// Avoid activity of function "Check_Send_Data_ASC0" not needed for J1708 communication.
		ASC0TransmChainActiveB = TRUE;
	}
	else
	{
	    S0TInt_Func = S0TINT_handler_def;
		T3Int_Func = T3INT_handler_def;
		S0RInt_Func = S0RINT_handler_def;
	}
#endif

	ASC0StatusPtr = (word *)&A21_Status.ASC0;
	*ASC0StatusPtr = 0;

	S0RIE = ENABLE;
	return(stASC0_settings.s0bg);
}	// InitASC0


bool j1708_prot_active(void)
{
	return(Current_ASC0_Prot_Id == PROT_ID_J1708);
}	//j1708_prot_active


void Check_Send_Data_ASC0(void)
{
    // Signal /TXEN is used for activation of RS-485 driver of system interface if RS-232 service 
    // interface is not occupied or if RS-232 service interface is occupied and ASC0 communication
    // via system interface was forced by software (signal /SYFREIGC167 = 0).
	ToUseSTXENB = (!_getbit(P3, SEBELEG) || !_getbit(P3, SYFREIGC167)) && !STXENForcedB;
	// Return immediately if transmission is already running, i.e. if at least one character was written into
	// ASC0 transmitter buffer and ASC0 transmitter buffer or transmitter interrupt is enabled.
	if( ASC0TransmChainActiveB )
		return;
	if( ConfigData.ASC0.flow_control && _getbit(P4, P4_CTS) )	// Do not send any data if flow control is used
		return;													// and CTS is not set.
	// Activate transmitter chain if at least one character is in output buffer and transmission is released,
	// i.e. no send delay is pending.
	if( ASC0OutBufferCharCnt > 0 && ASC0TransmReleasedB )
	{								
		// Disable ASC0 related interrupts to ensure consistency of all variables used for ASC0 control.
		Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
		T3IE = DISABLE;
		S0TIE = DISABLE;									// Disable ASC0 transmit interrupts.
		S0TBIE = DISABLE;									// Disable ASC0 transmit buffer interrupts.
		//
		if( ToUseSTXENB )
			_putbit(FALSE, P3, STXEN);						// Enable /TXEN.
		//
		if( ++wSerialOutBuffer_read >= SER_OUT_BUF_SIZE )	// Get character from ASC0 output buffer.
			wSerialOutBuffer_read = 0;						//
		ASC0OutBufferCharCnt--;								//
		// S0TBIR is set immediately with the write access to S0TBUF presumed no ASC0 transmission 
		// is running. This condition is ensured by leaving Check_Send_Data_ASC0 if
		// ASC0TransmChainActiveB == TRUE.
		S0TBUF = bSerialOutBuffer[wSerialOutBuffer_read];	// Write first character into ASC0 transmit shift
															// register. Start of transmission may be delayed
							   								// up to 1 bit time (refer to C167CR user manual,
							   								// page 11-7.
		ASC0TransmChainActiveB  = TRUE;
		ASC0TxDActiveB          = TRUE;
		//
		if( ASC0OutBufferCharCnt == 0 )						// Activate ASC0 transmitter chain. Transmitter
		{													// buffer interrupt S0TBINT is enabled if more than
			S0TIR = FALSE;									// one character was written to bSerialOutBuffer
			S0TIE = ENABLE;									// by function WriteData_ASC0. Otherwise transmitter
		}													// interrupt S0TINT is enabled. ASC0 transmitter
		else												// chain will be continued by calling of 
			S0TBIE = ENABLE;								// S0TBIR_handler or S0TIR_handler.
		Restore_ILVL();
	}	// end of if( ASC0OutBufferCharCnt > 0 && ASC0TransmReleasedB )
}	// Check_Send_Data_ASC0


void ReleaseTransmission(void)
{
	ASC0TransmReleasedB = TRUE;
}	// ReleaseTransmission


_inline void
Enable_T3INT(void)
{
	T3IR = FALSE;
	// Without additional "nop" statement execution of "T3IE = ENABLE" (bset T3IC,0x6)
	// could not be found in TRACE32 trace memory. After execution of "T3IR = FALSE"
	// (bclr T3IC,0x7, 2 byte) IP was increased by 4. This effect may result from
	// opcode pipelining or may be a TRACE32 artefact. But the cause of this behaviour
	// isn큧 known so the "nop" was introduced for the sake of reliability.
	_nop();
	T3IE = ENABLE;
}	// Enable_T3INT


// Disabling of T6 interrupt used in A21S software is necessary to avoid "hanging" of signal /TXEN.
// Error description for case of missing frame "Save_ILVL_and_Disable_All_INTs();" ... "Restore_ILVL();":
// - T6INT_handler is called just after assignment "T3 = T3Ticks_ASC0BitTime;".
// - Runtime of T6INT_handler is about 30탎 if T6Int_Func == SSC_Data_io. 
// - If ASC0 baud rate is greater than about 33000bps, T3 is already overflowed on return from
//   T6INT_handler.
// - T3IR is cleared although T3 contains value close to 65536.
// - T3INT_handler is called after period much longer than an ASC0 bit time. Example:
// - For ASC0 baud rate 38400bps T3INT_handler is called after about 113ms (26탎 expected !!!)
#ifdef J1708_PROTOCOL
void Load_T3_and_Enable_T3INT(word T3Ticks)
{
	Save_ILVL_and_Disable_All_INTs();
	T3 = T3Ticks;
	#ifdef USE_T3OUT
	T3OTL = 1;
	#endif
	Enable_T3INT();
	Restore_ILVL();
}	// Load_T3_and_Enable_T3INT
#else
void Load_T3_with_J1708BitTime_and_Enable_T3INT(void)
{
	Save_ILVL_and_Disable_All_INTs();
	T3 = T3Ticks_ASC0BitTime;			// Wait for transmission of one bit (last stop bit).
	#ifdef USE_T3OUT
	T3OTL = 1;
	#endif
	Enable_T3INT();
	Restore_ILVL();
}	// Load_T3_with_J1708BitTime_and_Enable_T3INT
#endif	// end of "#ifdef J1708_PROTOCOL"


// Default register bank 0 is replaced by register bank SERIO_ASC0_RB for ISR runtime.																			|
// Register banks 1 ... 3 are assigned to ISR register banks in the order as source files
// are listed in Tasking EDE project file.
// Typically register bank 2 is used as SERIO_ASC0_RB.

// Flag S0TBIR is set if a character is transfered from the ASC0 transmitter buffer to ASC0 transmit shift
// register.
interrupt (S0TBINT) void S0TBINT_handler(void)
{
	S0TBIE = DISABLE;
	if( ASC0OutBufferCharCnt > 0 )
	{
		if( ++wSerialOutBuffer_read >= SER_OUT_BUF_SIZE )	// Get character from ASC0 output buffer.
			wSerialOutBuffer_read = 0;						//
		ASC0OutBufferCharCnt--;								//
		S0TBUF = bSerialOutBuffer[wSerialOutBuffer_read];	// Write next character into ASC0 transmitter buffer.
		S0TBIE = ENABLE;									// ASC0 transmitter buffer is occupied now.
	}
	else
	{
		// ASC0 transmitter buffer is empty now. Transmission of last character
		// written by function WriteData_ASC0 to bSerialOutBuffer is running.
		S0TIR = FALSE;
		S0TIE = ENABLE;
	}
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// S0TBINT_handler


#ifdef J1708_PROTOCOL
byte RXD1Sync(void)
{
	T1_RXD1Sync = T1;			// Update RXD1Sync call time.
	CC7IR = FALSE;				// Initialize detection of falling RXD0 edge.
	if( !_getbit(P3, 11) )		// RXD0 == 0 ?
	{
		CC7IR = TRUE; 			// CC7IR == TRUE indicates that RXD0 == 1 synchronization
								// failed.
		return(1);				// RXD0 == 0: Character is being received at ASC0.
	}
	if( CC7IR )					// Falling RXD0 edge detected ?
		return(2);				// Falling RXD0 edge detected: Character is being
	//							// received at ASC0.
	return(0);
}	// RXD1Sync					// J1708 bus idle condition detected.


// To be called in ASC0 related interrupt service routines only.
void Init_J1708MsgTransmission(void)
{
	wSerialOutBuffer_write = SER_OUT_BUF_SIZE - 1;
	wSerialOutBuffer_read = SER_OUT_BUF_SIZE - 1;
	ASC0OutBufferCharCnt = 0;
	J1708SendMsgWrite = 0;
	J1708SendMsgRead = 0;
	J1708SendMsgCnt = 0;
	LocEchoCharCnt = 0;
	ASC0TxDActiveB = FALSE;
	LocalEchoReceivedB = FALSE;
}	// Init_J1708MsgTransmission


bool Continue_J1708MsgTransmission(void)
{
	word T3Ticks;
	bool ToWaitJ1708BusAccTimeB = FALSE;

	// Transmission of J1708 message completed ?
	if( J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt == 0 )
	{
		if( ++J1708SendMsgRead >= MaxJ1708SendMsgCnt )
			J1708SendMsgRead = 0;
		// Redundancy: underflow protection
		if( J1708SendMsgCnt > 0 )
			J1708SendMsgCnt--;
		ToWaitJ1708BusAccTimeB = TRUE;
	}
	// Redundancy:
	// Consistency check for transmission buffers. Reinitialize buffers if data are corrupted.
	if( J1708SendMsgCnt > 0 && ASC0OutBufferCharCnt > 0 &&
	    ASC0OutBufferCharCnt >= J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt )
//	if( ASC0OutBufferCharCnt > 0 )
	{
		if( ToWaitJ1708BusAccTimeB )
		{
			ASC0TxDActiveB = FALSE;
			if( RXD1Sync() == 0 )
			{
				T3Int_Func = T3INT_handler_J1708_Send;
				T3Ticks = T3Ticks_J1708CharTime + J1708SendMsgBuff[J1708SendMsgRead].T3Ticks_J1708TimeOffs;
			}
			else
			{
				T3Int_Func = T3INT_handler_J1708_Idle;				// Check J1708 bus state after potential bus idle
				T3Ticks = T3Ticks_J1708CharTime;					// has been elapsed.
			}
			Load_T3_and_Enable_T3INT(T3Ticks);
			return(FALSE);
		}
		else
		{
			if( ++wSerialOutBuffer_read >= SER_OUT_BUF_SIZE )		// Get character from ASC0 output buffer.
				wSerialOutBuffer_read = 0;							//
			ASC0OutBufferCharCnt--;									//
			J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt--;
			LocEchoChar = bSerialOutBuffer[wSerialOutBuffer_read];
			S0TBUF = LocEchoChar;									// Write next character into ASC0 transmit shift
																	// register. Start of transmission may be delayed
							   										// up to 1 bit time (refer to C167CR user manual,
							   										// page 11-7.
			return(TRUE);
		}
	}
	else
	{
		// Redundancy:
		// Ensure that there are no inconsistencies among transmission buffers pending.
		Init_J1708MsgTransmission();
		ASC0TxDActiveB = FALSE;
		ASC0TransmReleasedB = FALSE;
		return(FALSE);
	}
}	// Continue_J1708MsgTransmission


interrupt (S0TINT) void S0TINT_handler(void)
{
	S0TInt_Func();
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// S0TINT_handler


void S0TINT_handler_J1708(void)
{
	S0TIE = DISABLE;
	if( Continue_J1708MsgTransmission() )
		S0TIE = ENABLE;
	//
	LocEchoCharCnt = 0;						// Redundancy for maximum software reliability.
	ASC0TxDActiveB = FALSE;					//
	LocalEchoReceivedB = FALSE;	 			//
	S0RInt_Func = S0RINT_handler_J1708;		//
	S0RIE = ENABLE;							//
}	// S0TINT_handler_J1708


// Flag S0TIR is set if transmission of a character on ASC0 excluding last stop bit is finished.
// Transmission interrupt S0TINT is enabled only if ASC0 transmission buffer is empty.
void S0TINT_handler_def(void)
#else
interrupt (S0TINT) void S0TINT_handler(void)
#endif
{
	S0TIE = DISABLE;
	if( ASC0OutBufferCharCnt > 0 )
	{
		if( ++wSerialOutBuffer_read >= SER_OUT_BUF_SIZE )	// Get character from ASC0 output buffer.
			wSerialOutBuffer_read = 0;						//
		ASC0OutBufferCharCnt--;								//
		// S0TBIR is set immediately with the write access to S0TBUF presumed no ASC0 transmission 
		// is running.
		S0TBUF = bSerialOutBuffer[wSerialOutBuffer_read];	// Write next character into ASC0 transmit shift
															// register. Transmission starts after shift-out 
															// of pending stop bit.
		if( ASC0OutBufferCharCnt == 0 )						// Continue ASC0 transmitter chain. Transmitter
			S0TIE = ENABLE;									// buffer interrupt S0TBINT is enabled if more than
		else												// one character is stored in bSerialOutBuffer.
			S0TBIE = ENABLE;								// Otherwise transmitter interrupt S0TINT is enabled.
															// ASC0 transmitter chain will be continued by
															// calling of S0TBINT_handler or again S0TINT_handler.
	}
	else
	{
		// Transmission of last character written by function WriteData_ASC0 to
		// bSerialOutBuffer is finished.
#ifdef J1708_PROTOCOL
		Load_T3_and_Enable_T3INT(T3Ticks_ASC0BitTime);
#else
		Load_T3_with_J1708BitTime_and_Enable_T3INT();	// Wait for transmission of one bit (last stop bit).
#endif													// T3 ISR disables /TXEN.
		ASC0TransmChainActiveB = FALSE;					// Transmitter chain at end.
		ASC0TransmReleasedB = FALSE;					//
	}
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// S0TINT_handler(_def)


#ifdef J1708_PROTOCOL
interrupt (T3INT) void T3INT_handler(void)
{
	T3Int_Func();
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// T3INT_handler


// Function to detect J1708 bus idle state and to configure J1708 bus control accordingly.
void T3INT_handler_J1708_Idle(void)
{
	T3IE = DISABLE;
	if( S0RIR )
	{
		S0RIR = FALSE;
		S0RINT_handler_J1708();
	}
	else
	{
		// Check if J1708 bus idle state was encountered, i.e. if J1708 bus idle time elapsed
		// without any bus activity.
		if( CC7IR )
		{
			// Bus activity found. Ensure capability to receive J1708 messages and restart bus
			// idle check.
			RXD1Sync();
			Load_T3_and_Enable_T3INT(T3Ticks_J1708CharTime);
		}
		else
		{
			/*
			if( J1708RecInitFunc != NULL )
				J1708RecInitFunc();
			*/
			// J1708 bus idle state encountered. Start bus access check if there are J1708 messages
			// to be transmited.
			if( J1708SendMsgCnt > 0 )
			{
				T3Int_Func = T3INT_handler_J1708_Send;
				Load_T3_and_Enable_T3INT(J1708SendMsgBuff[J1708SendMsgRead].T3Ticks_J1708TimeOffs);
			}
			else
				ASC0TransmReleasedB = FALSE;
			//
			// J1708 bus idle state is specified as end of message indicator.
			J1708RecMsgStartBuff = wSerialInBuffer_write;
			// Request call-up of function "Receive_J1708Msg" from function "runlevel_1_jobs".
			Parser.call_terminalB = TRUE;
		}
	}
	LocEchoCharCnt = 0;						// Redundancy for maximum software reliability.
	ASC0TxDActiveB = FALSE;					//
	LocalEchoReceivedB = FALSE;	 			//
	S0RInt_Func = S0RINT_handler_J1708;		//
	S0RIE = ENABLE;							//
}	// T3INT_handler_J1708_Idle


// Function to detect J1708 bus availability and to initialize transmission of next J1708 message
// if necessary. According to SAE J1708 time elapsing between bus activity check and start of
// transmission is minimized.
void T3INT_handler_J1708_Send(void)
{
	T3IE = DISABLE;
	// Prepare character from ASC0 output buffer for transmission. Store "read pointer" for
	// possible restorage.
	wSerialOutBuffer_read_Buff = wSerialOutBuffer_read;
	if( ++wSerialOutBuffer_read >= SER_OUT_BUF_SIZE )		// Get character from ASC0 output buffer.
		wSerialOutBuffer_read = 0;							//
	LocEchoChar = bSerialOutBuffer[wSerialOutBuffer_read];	//
	// Check if J1708 bus is available for transmission, i.e. if J1708 bus access time elapsed
	// without any bus activity.
	if( CC7IR )
	{
		// Bus activity found. Ensure capability to receive J1708 messages and restart bus
		// idle check.
		wSerialOutBuffer_read =	wSerialOutBuffer_read_Buff;
		T3INT_handler_J1708_Idle();
	}
	else
	{
		// Consistency check for transmission buffers. Reinitialize buffers if data are corrupted.
		if( J1708SendMsgCnt > 0 && ASC0OutBufferCharCnt > 0 &&
		    ASC0OutBufferCharCnt >= J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt )
		{
			// Start transmission of next J1708 message. Store data needed for send crash recovery
			// in buffer variables.
			S0TBUF = LocEchoChar;  				// Write next character into ASC0 transmit shift
								   				// register. Start of transmission may be delayed
								   				// up to 1 bit time (refer to C167CR user manual,
								   				// page 11-7.
			LocalEchoReceivedB = FALSE;		 	// Redundancy for maximum software reliability.
			ASC0OutBufferCharCntBuff = ASC0OutBufferCharCnt;
		    // Local echo of transmission is expected if RS-232 service interface is not occupied or
		    // if RS-232 service interface is occupied and ASC0 communication via system interface 
		    // (J1708 bus in this case) was forced by software.
			if( !_getbit(P3, SEBELEG) || !_getbit(P3, SYFREIGC167) )
			{
				T3Int_Func = T3INT_handler_J1708_Echo;
				// Consider delay up to 1 bit time.
				Load_T3_and_Enable_T3INT(T3Ticks_J1708CharTime + T3Ticks_ASC0BitTime);
				LocEchoCharCnt = J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLen;
				ASC0TxDActiveB = TRUE;
			}
			else
			{
			    S0TInt_Func = S0TINT_handler_J1708;
				S0TIR = FALSE;
				S0TIE = ENABLE;
				LocEchoCharCnt = 0;
				ASC0TxDActiveB = FALSE;
			}
			ASC0OutBufferCharCnt--;									
			J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt--;
		}
		else
			Init_J1708MsgTransmission();
		//
		S0RInt_Func = S0RINT_handler_J1708;	// Redundancy for maximum software reliability.
		S0RIE = ENABLE;						//
	}
}	// T3INT_handler_J1708_Send


// Echo checking is relevant for operation of single A21 base module with signal
// SYFREIGC167 == 0 only.
void T3INT_handler_J1708_Echo(void)
{
	T3IE = DISABLE;
	if( S0RIR )
	{
		S0RIR = FALSE;
		S0RINT_handler_J1708();
	}
	else
		// No echo received for first char of transmission message. Continue
		// transmission without echo expectation. Transmission will be now
		// S0TIE controlled.
		if( LocEchoCharCnt == J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLen )
		{
		    S0TInt_Func = S0TINT_handler_J1708;
			S0TIR = FALSE;
			S0TIE = ENABLE;
			LocEchoCharCnt = 0;
			ASC0TxDActiveB = FALSE;
			LocalEchoReceivedB = FALSE;
		}
	//
	S0RInt_Func = S0RINT_handler_J1708;	// Redundancy for maximum software reliability.
	S0RIE = ENABLE;						//
}	// T3INT_handler_J1708_Echo


void T3INT_handler_def(void)
#else
// Software flow for case of 2-wire RS-485 bus:
// Provided that local echo is undisturbed T3INT_handler is always called after that call of S0RINT_handler
// where LocEchoCharCnt is decremented to zero and LocalEchoReceivedB is set to FALSE due to this result.
interrupt (T3INT) void T3INT_handler(void)
#endif
{
	T3IE = DISABLE;
	if( ToUseSTXENB && !ASC0TransmChainActiveB )
		_putbit(TRUE, P3, STXEN);					// Disable /TXEN.
	//
	ASC0TxDActiveB = FALSE;
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// T3INT_handler(_def)


byte Check_Send_Delay(void)
{
	if( ToUseSendDelayB && (!_getbit(P3, SEBELEG) || !_getbit(P3, SYFREIGC167)) )
		return(ConfigData.ASC0_control.send_delay);
	else
		return(0);
}	// Check_Send_Delay


#ifdef J1708_PROTOCOL
interrupt (S0RINT) void S0RINT_handler(void)
{
	S0RInt_Func();
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// S0RINT_handler


word T3Ticks_J1708BusAccTimeOffs(byte priority)
{
    word T3Ticks;

	// Default bus access time Ta according to J1708 paragraph 5.2.2.2:
	// Ta = Ti + (2 * Tb) * P
	// Tb = ASC0 bit time Tb.
	// Ti = bus idle time = 10 * Tb, P = message priority
	T3Ticks = T3Ticks_ASC0BitTime * 2 * priority;
	return(T3Ticks);
}	// T3Ticks_J1708BusAccTimeOffs


void S0RINT_handler_J1708(void)
{
	byte c;
	byte random_priority;

	// Upon entry into S0RINT interrupt service routine bit S0RIR is cleared automatically.
	// Refer to page 5-7 of Infineon C167CR Derivatives User큦 Manual, V3.2.
	c = S0RBUF;
	// Query bit S0EIR and clear it afterwards.
	if( _testclear(S0EIR) )
		ASC0RecErrB = TRUE;
	//
	// Bits S0OE, S0FE and S0PE of ASC0 control register S0CON have to be cleared by software.
	// Refer to page 11-15 of Infineon C167CR Derivatives User큦 Manual, V3.2.
	if( S0OE )											// Check for ASC0 overrun error.
	{
		S0OE = FALSE;									// Clear error flag.
		A21_Status.ASC0.OverrunErrorB = TRUE;
		ASC0Error.OverrunErrorB       = TRUE;
	}
	if( S0FE )											// Check for ASC0 frame error.
	{
		S0FE = FALSE;									// Clear error flag.
		A21_Status.ASC0.FrameErrorB = TRUE;
		ASC0Error.FrameErrorB       = TRUE;
	}
	if( S0PE )											// Check for ASC0 parity error.
	{
		S0PE = FALSE;									// Clear error flag.
		bSerialInBuffer[wSerialInBuffer_write] = 0xFF; 	// Force checksum error.
		A21_Status.ASC0.ParityErrorB = TRUE;
		ASC0Error.ParityErrorB       = TRUE;
	}
	//
    if( ASC0TxDActiveB )		 					// Local echo will be received during
		LocalEchoReceivedB = LocEchoCharCnt > 0;	// transmission.
	//
	if( LocalEchoReceivedB )  						// Ignore local echo resulting from
	{												// 2-wire communication.
		// Was there a J1708 bus contention during transmission ? 
		if( c == LocEchoChar && !ASC0RecErrB )
		{
			// Transmission message undisturbed. No J1708 bus contention encountered.
			LocEchoCharCnt--;
			LocalEchoReceivedB = LocEchoCharCnt != 0;
			Continue_J1708MsgTransmission();
			return;										
		}
		else
		{
			ASC0RecErrB = FALSE;
			if( J1708SendMsgBuff[J1708SendMsgRead].J1708SendCrashCnt < 255 )
				J1708SendMsgBuff[J1708SendMsgRead].J1708SendCrashCnt++;
			//
			// Refer to SAE J1708, appendix B.1.1.
			if( J1708SendMsgBuff[J1708SendMsgRead].J1708SendCrashCnt >= J1708SendCrashCnt_Random )
			{
				// Pseudo random variation of J1708 bus access time needed. Content of timer T1
				// is used to generate pseudo random number because this timer is running
				// continously and asynchronously with respect to J1708 message transmission.
				random_priority = (byte)(T1 & 0x0007) + 1;
				J1708SendMsgBuff[J1708SendMsgRead].T3Ticks_J1708TimeOffs = 
					T3Ticks_J1708BusAccTimeOffs(random_priority);
			}
			J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLenDCnt =
				J1708SendMsgBuff[J1708SendMsgRead].J1708SendMsgLen;
			wSerialOutBuffer_read =	wSerialOutBuffer_read_Buff;
			// Variable ASC0OutBufferCharCnt possibly changed by function "Write_J1708SendMsgBuff". 
			ASC0OutBufferCharCnt = ASC0OutBufferCharCntBuff;
    		S0TInt_Func = No_Job_Kernel;
			S0TIE = DISABLE;
    		ASC0TxDActiveB = FALSE;
			LocalEchoReceivedB = FALSE;
		}
	}
	//
	// ASC0 input buffer write pointer may be adjusted b e f o r e writing of character to buffer
	// because ASC0 interrupt service routine cannot be interrupted be any program sequence
	// accessing ASC0 input buffer. Usage of usual buffer writing (pointer adjustment is last step)
	// would lead to greater program code.
	// Usage of macro NEXT yields greater program code than usage of equivalent C statements.
	// Refer to comments regarding macro NEXT in header file "general.h".
	if( ++wSerialInBuffer_write >= SER_IN_BUF_SIZE )
		wSerialInBuffer_write = 0;
//	NEXT(wSerialInBuffer_write, SER_IN_BUF_SIZE);
	if( wSerialInBuffer_write == wSerialInBuffer_read )	// Check ASC0 input buffer for overflow.
	{
		// Overflow occured. Character stored in buffer at position wSerialInBuffer_read will be
		// lost. Shift ASC0 input buffer read pointer to limit loss to this one character. All
		// characters already stored in buffer would be lost if write pointer would "overrun"
		// read pointer.
		// Usage of macro NEXT yields greater program code than usage of equivalent C statements.
		// Refer to comments regarding macro NEXT in header file "general.h".
		if( ++wSerialInBuffer_read >= SER_IN_BUF_SIZE )
			wSerialInBuffer_read = 0;
//		NEXT(wSerialInBuffer_read, SER_IN_BUF_SIZE);
		// Set ASC0 input buffer overflow flags.
		A21_Status.ASC0.In_OverflowB = TRUE;
		ASC0Error.In_OverflowB       = TRUE;
	}
	bSerialInBuffer[wSerialInBuffer_write] = c;
	// To be used in later version of firmware.
	ASC0RecErrB = FALSE;
	// To be cancelled if Parser.call_terminalB = TRUE in function "T3INT_handler_J1708_Idle".
//	if( c == Parser.terminal_symbol || Parser.terminal_symbol == 0xFF )
//		Parser.call_terminalB = TRUE;
	//
	RXD1Sync();
	T3Int_Func = T3INT_handler_J1708_Idle;
	Load_T3_and_Enable_T3INT(T3Ticks_J1708CharTime);
	ASC0TransmReleasedB = TRUE;
}	// S0RINT_handler_J1708


void S0RINT_handler_def(void)
#else
interrupt (S0RINT) void S0RINT_handler(void)
#endif
{
	byte c;

	// Upon entry into S0RINT interrupt service routine bit S0RIR is cleared automatically.
	// Refer to page 5-7 of Infineon C167CR Derivatives User큦 Manual, V3.2.
	c = S0RBUF;
	// Query bit S0EIR and clear it afterwards.
	if( _testclear(S0EIR) )
		ASC0RecErrB = TRUE;
	//
	// Bits S0OE, S0FE and S0PE of ASC0 control register S0CON have to be cleared by software.
	// Refer to page 11-15 of Infineon C167CR Derivatives User큦 Manual, V3.2.
	if( S0OE )											// Check for ASC0 overrun error.
	{
		S0OE = FALSE;									// Clear error flag.
		A21_Status.ASC0.OverrunErrorB = TRUE;
		ASC0Error.OverrunErrorB       = TRUE;
	}
	if( S0FE )											// Check for ASC0 frame error.
	{
		S0FE = FALSE;									// Clear error flag.
		A21_Status.ASC0.FrameErrorB = TRUE;
		ASC0Error.FrameErrorB       = TRUE;
	}
	if( S0PE )											// Check for ASC0 parity error.
	{
		S0PE = FALSE;									// Clear error flag.
		A21_Status.ASC0.ParityErrorB = TRUE;
		ASC0Error.ParityErrorB       = TRUE;
	}
	if( ToUseSendDelayB && ToUseSTXENB )				// Consider local echo only for
	{													// 2-wire communication on system interface.
	    if( ASC0TransmChainActiveB )					// Local echo may be received during
			LocalEchoReceivedB = LocEchoCharCnt > 0;	// transmission.
		//
		if( LocalEchoReceivedB )  						// Ignore local echo resulting from
		{												// 2-wire communication.
			LocEchoCharCnt--;
			// Local echo on 2-wire bus may be highly disturbed due to bus contention. As a result
			// countdown of characters of expected local echo may fail, i.e. countdown may not be
			// finished at end of corresponding ASC0 transmission. To avoid "damage" of receive frame
			// following suppression of local echo is terminated if 2-wire bus transmitter is not
			// longer active.
			LocalEchoReceivedB = LocEchoCharCnt != 0 && ASC0TxDActiveB;
			if( ASC0TxDActiveB )
				return;										
		}
	}
	//
	// ASC0 input buffer write pointer may be adjusted b e f o r e writing of character to buffer
	// because ASC0 interrupt service routine cannot be interrupted be any program sequence
	// accessing ASC0 input buffer. Usage of usual buffer writing (pointer adjustment is last step)
	// would lead to greater program code.
	// Usage of macro NEXT yields greater program code than usage of equivalent C statements.
	// Refer to comments regarding macro NEXT in header file "general.h".
	if( ++wSerialInBuffer_write >= SER_IN_BUF_SIZE )
		wSerialInBuffer_write = 0;
//	NEXT(wSerialInBuffer_write, SER_IN_BUF_SIZE);
	if( wSerialInBuffer_write == wSerialInBuffer_read )	// Check ASC0 input buffer for overflow.
	{
		// Overflow occured. Character stored in buffer at position wSerialInBuffer_read will be
		// lost. Shift ASC0 input buffer read pointer to limit loss to this one character. All
		// characters already stored in buffer would be lost if write pointer would "overrun"
		// read pointer.
		// Usage of macro NEXT yields greater program code than usage of equivalent C statements.
		// Refer to comments regarding macro NEXT in header file "general.h".
		if( ++wSerialInBuffer_read >= SER_IN_BUF_SIZE )
			wSerialInBuffer_read = 0;
//		NEXT(wSerialInBuffer_read, SER_IN_BUF_SIZE);
		// Set ASC0 input buffer overflow flags.
		A21_Status.ASC0.In_OverflowB = TRUE;
		ASC0Error.In_OverflowB       = TRUE;
		if( ConfigData.ASC0.flow_control )
			_putbit(ASC0_OFF, P4, P4_RTS);				// Clear RTS signal.
	}
	// ASC0 UART is not capable of handling 7-bit characters. As a consequence most significant
	// bit has to be masked if 7 data bits are used.
	if( RS232_7bit_ActiveB )
		c &= 0x7F;
	bSerialInBuffer[wSerialInBuffer_write] = c;
	//
	if( Parser.call_terminalB )
		Parser.call_checksumB = TRUE;
	if( c == Parser.terminal_symbol || Parser.terminal_symbol == 0xFF )
		Parser.call_terminalB = TRUE;
	//
	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// S0RINT_handler(_def)


// In case of ASC0 input buffer overflow ASC0 input buffer read pointer is shifted by S0RINT
// interrupt service routine. As a consequence buffer read result returned by function
// CheckChar_ASC0 may be other than result returned by function GetChar_ASC0 called afterwards.
int CheckChar_ASC0(void)
{
	word InBuffer_read;

	// Buffer read result is equal to -1 if ASC0 input buffer is empty.
	if( wSerialInBuffer_read == wSerialInBuffer_write )
		return(EOF);
	//
	InBuffer_read = wSerialInBuffer_read;
	NEXT(InBuffer_read, SER_IN_BUF_SIZE);
	return(bSerialInBuffer[InBuffer_read]);
}	// CheckChar_ASC0


int GetChar_ASC0(void)
{
	int data;

	// Buffer read result is equal to -1 if ASC0 input buffer is empty.
	if( wSerialInBuffer_read == wSerialInBuffer_write )
		return(EOF);
	// 
	// In case of ASC0 input buffer overflow ASC0 input buffer read pointer is shifted by S0RINT
	// interrupt service routine. As a consequence data consistency is ensured by usage of
	// uninterruptable program sequence.
	Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
	// Usage of macro NEXT yields greater program code than usage of equivalent C statements.
	// Refer to comments regarding macro NEXT in header file "general.h".
	if( ++wSerialInBuffer_read >= SER_IN_BUF_SIZE )
		wSerialInBuffer_read = 0;
//	NEXT(wSerialInBuffer_read, SER_IN_BUF_SIZE);
	data = bSerialInBuffer[wSerialInBuffer_read];
	Restore_ILVL();
	if( ConfigData.ASC0.flow_control )
		_putbit(ASC0_ON, P4, P4_RTS);
	//
	return(data);
}	// GetChar_ASC0


#ifdef J1708_PROTOCOL
void NextJ1708RecMsgStart(void)
{
	wSerialInBuffer_read = J1708RecMsgStartBuff;
}	// NextJ1708RecMsgStart
#endif


word PutByte_ASC0(byte c)
{
	byte data[sizeof(byte)];
	word WriteRes;

	data[0] = c;
	WriteRes = WriteData_ASC0(data, sizeof(byte));
	return(WriteRes);
}	// PutByte_ASC0


word PutChar_ASC0(char c)
{
	byte data[sizeof(char)];
	word WriteRes;

	data[0] = (byte)c;
	WriteRes = WriteData_ASC0(data, sizeof(char));
	return(WriteRes);
}	// PutChar_ASC0


word PutWord_ASC0(word c)
{
	byte data[sizeof(word)];
	word WriteRes;

	data[0] = (byte)c;
	data[1] = c >> 8;
	WriteRes = WriteData_ASC0(data, sizeof(word));
	return(WriteRes);
}	// PutWord_ASC0


word PutString_ASC0(const char *data)
{
	word size;
	word WriteRes;

	size = strlen(data);
	if( size > 255 )
		size = 255;
	WriteRes = WriteData_ASC0((byte *)data, (word)size);
	return(WriteRes);
}	// PutString_ASC0


// Function "WriteData_ASC0" may not be called within timer or time-out job because there is no protection
// implemented against interruption of "WriteData_ASC0" by "WriteData_ASC0".
word WriteData_ASC0(const byte *data, word size)
{
    word lASC0OutBufferCharCnt;
	bool OutBuffOverflowExpectedB;
	word ActualTO_ms;
	bool TimeOutIntvNotElapsedB;
	word size_copy;
    word S0TICBuff, S0TBICBuff;
    byte SendDelay;

	if( Current_ASC0_Prot_Id == PROT_ID_J1708 )
		return(0xFFFF);

	/*
	// Calculation of ASC0OutBufferCharCnt from wSerialOutBuffer_write and wSerialOutBuffer_read.
	// Always ASC0OutBufferCharCnt = 0 for wSerialOutBuffer_write == wSerialOutBuffer_read.
	if( wSerialOutBuffer_write >= wSerialOutBuffer_read )
		ASC0OutBufferCharCnt = wSerialOutBuffer_write - wSerialOutBuffer_read;
	else
		ASC0OutBufferCharCnt = SER_OUT_BUF_SIZE - wSerialOutBuffer_read + wSerialOutBuffer_write;
	*/
	lASC0OutBufferCharCnt = ASC0OutBufferCharCnt;
	OutBuffOverflowExpectedB = (lASC0OutBufferCharCnt + (word)size) > SER_OUT_BUF_SIZE;
	if( OutBuffOverflowExpectedB )
	{
		if( WriteData_ASC0_TO_ms > 0 )
		{
			ActualTO_ms = StartTimeOutIntv(WriteData_ASC0_TO_ms);
			if( ActualTO_ms >= WriteData_ASC0_TO_ms )
			{
				do
				{
					TimeOutIntvNotElapsedB = !TimeOutIntvElapsed();
					lASC0OutBufferCharCnt = ASC0OutBufferCharCnt;
					OutBuffOverflowExpectedB = lASC0OutBufferCharCnt + (word)size > SER_OUT_BUF_SIZE;
				} while( OutBuffOverflowExpectedB && TimeOutIntvNotElapsedB );
				if( TimeOutIntvNotElapsedB )
					CancelTimeOutIntv();
			}
		}
		if( OutBuffOverflowExpectedB )
		{
			A21_Status.ASC0.Out_OverflowB = TRUE;
			ASC0Error.Out_OverflowB       = TRUE;
			return(lASC0OutBufferCharCnt + (word)size - SER_OUT_BUF_SIZE);
		}
	}
	// Parameter size is used for update of variable ASC0OutBufferCharCnt inside uninterruptable sequence.
	size_copy = size;
	// Variable wSerialOutBuffer_write is used as "write pointer" only and never referenced
	// for buffer state query. It is not read within interrupt service routines.
	// That큦 why writing to buffer bSerialOutBuffer may be done outside uninterruptable sequence.
	// Global variable ASC0OutBufferCharCnt is used on reading of ASC0 output buffer.
	while( size_copy-- != 0 )
	{
		if( ++wSerialOutBuffer_write >= SER_OUT_BUF_SIZE )
			wSerialOutBuffer_write = 0;
		bSerialOutBuffer[wSerialOutBuffer_write] = *(data++);
	}
	// State of bits S0TIE and S0TBIE may be changed also by according ISRs. Global disabling of interrupt requests is needed
	// to avoid contention regarding state of these bits.
	Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
	S0TICBuff = S0TIC;	  		 						// Save current state of bit S0TIE.
	S0TBICBuff = S0TBIC; 		 						// Save current state of bit S0TBIE.
	S0TIE = DISABLE;   									// Disable ASC0 transmit interrupts.
	S0TBIE = DISABLE;  									// Disable ASC0 transmit buffer interrupts.
	//
	ASC0OutBufferCharCnt += size;
	// LocEchoCharCnt > ASC0OutBufferCharCnt in time interval between writing to S0TBUF and decrement
	// of LocEchoCharCnt in interrupt service routine "S0RINT_handler(_def)".
//	LocEchoCharCnt += size;
	LocEchoCharCnt = ASC0OutBufferCharCnt;
	// No write access to S0TIR or S0TBIR allowed because these bits may be setted by ASC0 hardware.
	// C compiler uses bmov statements for restoring of S0TIE and S0TBIE to avoid unintended resetting of S0TIR or S0TBIR.
    S0TIE = S0TICBuff & S0TIE_Mask;		  				// Restore S0TIE state. 
    S0TBIE = S0TBICBuff & S0TBIE_Mask;	  				// Restore S0TBIE state. 
	Restore_ILVL();
	//
	if( ASC0TransmChainActiveB )
		return(0);
	//
    SendDelay = Check_Send_Delay();
	if( SendDelay > 0 )
		add_timeout_job(SendDelay, ReleaseTransmission);
	else
	{
		ASC0TransmReleasedB = TRUE;
		Check_Send_Data_ASC0();
	}
	return(0);
}	// WriteData_ASC0


#ifdef J1708_PROTOCOL
bool Write_J1708SendMsgBuff(byte priority, byte MsgLen, byte *Msg)
{
	byte CSMsgLen;
	byte CS;
	byte b;
	word T3Ticks;

	if( Current_ASC0_Prot_Id != PROT_ID_J1708 )
		return(FALSE);

	Save_ILVL_and_Disable_T1_and_lower_priority_INTs();
	if( MsgLen <= MaxJ1708MsgLen && J1708SendMsgCnt < MaxJ1708SendMsgCnt )
	{
		// Both variables wSerialOutBuffer_write and J1708SendMsgWrite are used as
		// "write pointers" only and never referenced for buffer state query. They
		// are not read within interrupt service routines.
		// Global variables ASC0OutBufferCharCnt and J1708SendMsgCnt are used on
		// reading of ASC0 output buffer bSerialOutBuffer.
		CSMsgLen = MsgLen + 1;
		CS = 0;
		while( MsgLen-- != 0 )
		{
			b = *(Msg++);
			if( ++wSerialOutBuffer_write >= SER_OUT_BUF_SIZE )
				wSerialOutBuffer_write = 0;
			bSerialOutBuffer[wSerialOutBuffer_write] = b;
			CS += b;
		}
		CS = -CS;
		if( ++wSerialOutBuffer_write >= SER_OUT_BUF_SIZE )
			wSerialOutBuffer_write = 0;
		bSerialOutBuffer[wSerialOutBuffer_write] = CS;
		J1708SendMsgBuff[J1708SendMsgWrite].T3Ticks_J1708TimeOffs = T3Ticks_J1708BusAccTimeOffs(priority);
		J1708SendMsgBuff[J1708SendMsgWrite].J1708SendMsgLen = CSMsgLen;
		J1708SendMsgBuff[J1708SendMsgWrite].J1708SendMsgLenDCnt = CSMsgLen;
		J1708SendMsgBuff[J1708SendMsgWrite].J1708SendCrashCnt = 0;
		if( ++J1708SendMsgWrite >= MaxJ1708SendMsgCnt )
			J1708SendMsgWrite = 0;
		Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
		ASC0OutBufferCharCnt += CSMsgLen;
		J1708SendMsgCnt++;
		// Update buffer variable for send crash recovery too because function
		// "Write_J1708SendMsgBuff" may be called between start of message transmission
		// and according send crash detection.
		ASC0OutBufferCharCntBuff += CSMsgLen;
		if( !ASC0TransmReleasedB )
		{
			if( RXD1Sync() == 0 )
			{
				T3Int_Func = T3INT_handler_J1708_Send;
				T3Ticks = T3Ticks_J1708CharTime + J1708SendMsgBuff[J1708SendMsgRead].T3Ticks_J1708TimeOffs;
			}
			else
			{
				S0RInt_Func = S0RINT_handler_J1708;					// Redundancy for maximum software reliability.
				S0RIE = ENABLE;										//
				T3Int_Func = T3INT_handler_J1708_Idle;				// Check J1708 bus state after potential bus idle
				T3Ticks = T3Ticks_J1708CharTime;			   		// has been elapsed.
			}
			Load_T3_and_Enable_T3INT(T3Ticks);
			ASC0TransmReleasedB = TRUE;
		}
		Restore_ILVL();	// ASC0
		Restore_ILVL();	// T1
		return(TRUE);
	}
	else
	{
		A21_Status.ASC0.Out_OverflowB = TRUE;
		ASC0Error.Out_OverflowB       = TRUE;
		Restore_ILVL();	// T1
		return(FALSE);
	}
}	// Write_J1708SendMsgBuff	
#endif


bool FlushOutBufferASC0(word TO_ms)
{
	word ActualTO_ms;
	bool TimeOutIntvNotElapsedB;

	if( TO_ms > 0 )
	{
		ASC0TransmReleasedB = ASC0OutBufferCharCnt > 0;
		if( ASC0TransmReleasedB )
		{
			ActualTO_ms = StartTimeOutIntv(TO_ms);
			if( ActualTO_ms < TO_ms )
				return(FALSE);

			Check_Send_Data_ASC0();
			do
				TimeOutIntvNotElapsedB = !TimeOutIntvElapsed();
			while( ASC0TxDActiveB && TimeOutIntvNotElapsedB );

			if( TimeOutIntvNotElapsedB )
				CancelTimeOutIntv();
			return(TimeOutIntvNotElapsedB);
		}
		else
			return(TRUE);
	}
	else
		return(FALSE);
}	// FlushOutBufferASC0


bool WaitForEndOfTransmission(word Release_TO_ms, word Flush_TO_ms)
{
	word ActualTO_ms;
	bool TimeOutIntvNotElapsedB;

	if( !ASC0TransmReleasedB )
	{
		ActualTO_ms = StartTimeOutIntv(Release_TO_ms);
		if( ActualTO_ms < Release_TO_ms )
			return(FALSE);

		do
			TimeOutIntvNotElapsedB = !TimeOutIntvElapsed();
		while( !ASC0TransmReleasedB && TimeOutIntvNotElapsedB );

		if( TimeOutIntvNotElapsedB )
			CancelTimeOutIntv();
		else
			return(FALSE);
	}
	return(FlushOutBufferASC0(Flush_TO_ms));
}	// WaitForEndOfTransmission


void Clear_TerminalSymbol(void)
{
	word a;

	Parser.call_terminalB = FALSE;
	Parser.call_checksumB = FALSE;
	if( Parser.terminal_symbol == 0xFF )
		Parser.call_terminalB = wSerialInBuffer_read != wSerialInBuffer_write;
	else
	{
		a = wSerialInBuffer_read;
		// More terminal symbols ?
		while( a != wSerialInBuffer_write && !Parser.call_terminalB )
		{
			NEXT(a, SER_IN_BUF_SIZE);
			if( bSerialInBuffer[a] == Parser.terminal_symbol )
			{
				Parser.call_terminalB = TRUE;
				// More bytes in buffer ?
				if( a != wSerialInBuffer_write )
					Parser.call_checksumB = TRUE;
			}
		}
	}
}	// Clear_TerminalSymbol


bool ASC0_Service_Interface_Occupied(void)
{
	return(_getbit(P3, SEBELEG));
}	// ASC0_Service_Interface_Occupied


bool ASC0_System_Interface_Forced(void)
{
	return(!_getbit(P3, SYFREIGC167));
}	// ASC0_System_Interface_Forced


// Flags ASC0_Service_Interface_OccupiedB and ASC0_Debug_ModeB are updated by function
// runlevel_1_jobs if necessary.

// Logical 0 is outputted by IBIS receiver circuit on A21 system interface module if receiver
// input is left open. This logical 0 would block ASC0 receiver input if ASC0 debug mode is active.
void Change_ASC0_Debug_Mode(bool valueB)
{
	_putbit(!valueB, P3, SYFREIGC167);

	Save_ILVL_and_Disable_All_INTs();
	ASC0_Service_Interface_OccupiedB = ASC0_Service_Interface_Occupied();
	if( ASC0_Service_Interface_OccupiedB )
		ASC0_Debug_ModeB = valueB;
	else
		ASC0_Debug_ModeB = FALSE;
	Restore_ILVL();
}	// Change_ASC0_Debug_Mode


void Update_ASC0TimeoutRef(void)
{
	wSerialInBuffer_timeoutref = wSerialInBuffer_write;
}	// Update_ASC0TimeoutRef


bool ASC0TimeoutCond(void)
{
	return(wSerialInBuffer_write == wSerialInBuffer_timeoutref);
}	// ASC0TimeoutCond


bool ASC0_Transmission_Finished(void)
{
#ifdef J1708_PROTOCOL
	return( ASC0OutBufferCharCnt == 0 && !ASC0TxDActiveB  && !S0TIE );
#else
	return( ASC0OutBufferCharCnt == 0 && !ASC0TxDActiveB/*!ASC0TransmChainActiveB*/ );
#endif
}	// ASC0_Transmission_Finished
