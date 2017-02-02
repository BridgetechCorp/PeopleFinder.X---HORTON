/*==============================================================================================*
 |       Filename: uip_kernel.c                                                                 |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of Universal IRMA Protocol (UIP).                             |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#include "..\uip_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class FC=KERNEL
#pragma class CO=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
bool IRMA_Frame_ReceivedB;

byte IRMARecBuff[UIP_FRAME_LENGTH_MAX];
int  IRMARecBuffPos;
int  IRMARecBuffEnd;

#ifdef GATEWAY
byte uip_service_lev;
bool uip_retry_flagB;
#endif

byte IrmaDesAddr;
byte IrmaSrcAddr;

#ifdef GATEWAY
bool is_uip_20_sendB;
byte uip_service_lev_send;
byte uip_sub_cmd_send;
#endif
byte IRMASendBuff[UIP_FRAME_LENGTH_MAX];
word IRMASendBuffWriteStart;
word IRMASendBuffWritePos;
word uip_send_buff_frame_len;

void (*extended_uip_link)(void);
void (*uip_10_frame_process)(int);

#ifndef GLORIA
bool Routing_To_SysInterf_ActiveB;
#endif

#ifdef GLORIA
byte GLORIASendBuff[UIP_10_FRAME_LENGTH_MAX];
byte GLORIASendBuffWritePos;
byte IRMA_ASC0_Overrun_Errors;
GLORIA_Flags_type far GLORIA_Flags;
byte GLORIA_Communication_Error_Counter;
byte GLORIA_Communication_Error_Code;
#endif

#if defined(DEVTEST) && defined(GLORIA) || defined(CAN_SENSOR) || defined(GATEWAY)
byte IrmaASrcAddr;
#endif

#ifdef DEVTEST
	#ifdef GLORIA
bool Resp_to_EmbMsg_ExpectedB;
int  EmbMsgDI;
byte EmbMsgDesAddr;
byte EmbMsgSrcAddr;
	#endif
byte IRMASectSendBuff[UIP_10_FRAME_LENGTH_MAX];
byte MaxSectionPayloadLen;
byte ResSectionPayloadLen;
byte SectionCnt;
word SectionTransmissionDelay;
bool To_Transmit_Next_Irma_Frame_SectionB;
#endif

byte IRMA_M_MemByteCnt_UppLim;

Flags_Set_Func_type Flags_Set_Func;

#ifndef NO_LOGGER				
RunLevel_Log_Func_type RunLevel_Log_Func;
Logger_Set_Time_Func_type Logger_Set_Time_Func;
#endif

#ifdef GLORIA
byte GLORIA_Software_Version[IRMA_Vv20_PayloadLen];
#endif


/*----- Function Prototypes --------------------------------------------------------------------*/
void reset_uip_frame_reading(void);
void abort_uip_frame_reading(void);
void search_first_char_of_uip_frame(void);
void read_uip_frame(void);
void read_uip_10_frame(void);
void uip_10_frame_process_a21(int IRMARecBuffEndBackup);

void init_uip_send_buff_writing_10(char DataId, byte DataVer, byte PayloadLen);
void send_buffered_uip_10_frame(byte DesAddr);

#ifdef DEVTEST
void launch_cont_multisection_irma_frame_transmission(void);
void cont_multisection_irma_frame_transmission(void);
#endif

void Send_IRMA_Error_v10(byte DesAddr, byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData);

void Set_ASC0_Debug_Mode(void);
void Clear_ASC0_Debug_Mode(void);
#ifndef GLORIA
void Set_ASC0_Route_Mode(void);
void Clear_ASC0_Route_Mode(void);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/
extern const IRMACmdTable_type IRMACmdTable['z' - '@' + 1];

/*----- Function Tables ------------------------------------------------------------------------*/
const struct
{
	char argument[16];
	void (*call_function)(void);
} execute_command[] = {
	{ "DEBUG ON"   , Set_ASC0_Debug_Mode    },
	{ "DEBUG OFF"  , Clear_ASC0_Debug_Mode  },
#ifndef GLORIA
	{ "ROUTE ON"   , Set_ASC0_Route_Mode    },
	{ "ROUTE OFF"  , Clear_ASC0_Route_Mode	},
#endif
	{ ""           , NULL					}
};


/*----- Implementation of Functions ------------------------------------------------------------*/
void Prepare_IRMA(void)
{
	return;
}	// Prepare_IRMA


#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR) || defined(OBC)
void init_uip_10_kernel(void)
{
	IRMASendBuff[0] = IRMAPrefix1;
	IRMASendBuff[1] = IRMAPrefix2;
	IRMASendBuff[2] = IRMAPrefix3;
	IRMASendBuff[3] = IRMAPrefix4;
	IRMASendBuffWriteStart = UIP_10_FRAME_DL;
	IRMASendBuff[UIP_10_FRAME_SRC] = ConfigData.irma.address;

	extended_uip_link = abort_uip_frame_reading;

	reset_uip_frame_reading();

	IRMA_Frame_ReceivedB = FALSE;

	#ifndef GLORIA
	Routing_To_SysInterf_ActiveB = FALSE;
	#endif

	#if defined(DEVTEST) && defined(GLORIA)
	Resp_to_EmbMsg_ExpectedB = FALSE;
	EmbMsgDI      = 0;
	EmbMsgDesAddr = 0;
	EmbMsgSrcAddr = 0;
	#endif

	#if defined(DEVTEST) && defined(GLORIA) || defined(CAN_SENSOR)
	IrmaASrcAddr = 0;
	#endif

	IRMA_M_MemByteCnt_UppLim = IRMA_Mv10_MaxMemByteCnt;

	Flags_Set_Func = NULL;

	#ifndef NO_LOGGER				
	RunLevel_Log_Func = NULL;
	Logger_Set_Time_Func = NULL;
	#endif
}	// init_uip_10_kernel
#endif	// end of "#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR) || defined(OBC)"


#ifdef GATEWAY
void init_uip_kernel(void)
{
	IRMASendBuff[0] = IRMAPrefix1;
	IRMASendBuff[1] = IRMAPrefix2;
	IRMASendBuff[2] = IRMAPrefix3;
	IRMASendBuff[3] = IRMAPrefix4;

	reset_uip_frame_reading();

	IRMA_Frame_ReceivedB = FALSE;

	Routing_To_SysInterf_ActiveB = FALSE;

	IrmaASrcAddr = 0;

	IRMA_M_MemByteCnt_UppLim = IRMA_Mv10_MaxMemByteCnt;

	Flags_Set_Func = NULL;

	#ifndef NO_LOGGER				
	RunLevel_Log_Func = NULL;
	Logger_Set_Time_Func = NULL;
	#endif
}	// init_uip_kernel
#endif	// end of "#ifdef GATEWAY"


void Switch_IRMA_Prot_Check(void)
{
    if( !IRMA_Frame_ReceivedB )
		Reset_System();	// Software reset.
}	// Switch_IRMA_Prot_Check


void reset_uip_frame_reading_cont(void)
{
	IRMARecBuffPos = -1;
	IRMARecBuffEnd = UIP_10_FRAME_W1;
	Set_ParserParameter(IRMAPrefix1, IRMA_FRAME_TIMEOUT, search_first_char_of_uip_frame, NULL);
	Clear_TerminalSymbol();
}	// reset_uip_frame_reading_cont


void irma_rec_timeout_job(void)
{
	if( ASC0TimeoutCond() )
		reset_uip_frame_reading_cont();
}	// irma_rec_timeout_job


void reset_uip_frame_reading(void)
{
	remove_timeout_job(irma_rec_timeout_job);
	reset_uip_frame_reading_cont();
}	// reset_uip_frame_reading


void abort_uip_frame_reading(void)
{
#if defined(GLORIA) && defined(DEVTEST)
	Resp_to_EmbMsg_ExpectedB = FALSE;
#endif
	reset_uip_frame_reading();
}	// abort_uip_frame_reading


void search_first_char_of_uip_frame(void)
{
	int c;

	// If Parser.call_terminal_function == search_first_char_of_uip_frame then ASC0 receive buffer bSerialInBuffer
	// is filled with characters as long as they are not equal to "I". As a result ASC0 receive buffer
	// may overflow:
	// - ASC0Error.In_OverflowB == TRUE
	// - Write pointer wSerialInBuffer_write is 1 byte "behind" read pointer wSerialInBuffer_read, i. e.
	//   condition wSerialInBuffer_read == (wSerialInBuffer_write + 1) or
	//   condition (wSerialInBuffer_read == 0) && (wSerialInBuffer_write == (SER_IN_BUF_SIZE - 1)) applies.
	// Typically ASC0 receive buffer overflow occurs if IBIS frames with baud rate 38400bps and character
	// format 7E2 are received because always byte 0xC9 is received for character "I" and never 0x49.
	// Runtime needed for complete read of overflowed ASC0 receive buffer on reception of character "I":
	// - A21S-2-RS485_2_DevTest_BurnIn_5.03: about 8.5ms
	do
		c = GetChar_ASC0();
	while( c != IRMAPrefix1 && c != EOF );
	if( c == IRMAPrefix1 )
	{
		IRMARecBuff[++IRMARecBuffPos] = IRMAPrefix1;
		Set_Parser_function(read_uip_frame);
		Update_ASC0TimeoutRef();
		add_timeout_job(Get_Parser_timeout(), irma_rec_timeout_job);
	}
	else
		reset_uip_frame_reading();
}	// search_first_char_of_uip_frame


byte calculate_irma_checksum(byte *frame, word framelen)
{
	byte p;
	register word i;

	p = frame[0];
	for( i = 1; i < framelen; i++ )
		p ^= frame[i];
	p = ~p;
	return(p);
}	// calculate_irma_checksum


void route_uip_10_frame(void)
{
	WriteData_ASC0(IRMARecBuff, (word)(IRMARecBuff[UIP_10_FRAME_DL] + UIP_10_OVERHEAD));
}	// route_uip_10_frame


bool get_char_asc0_uip(void)
{
	int lb;

	lb = GetChar_ASC0();
	if( lb != EOF )		// EOF == 0xFFFF (-1): ASC0 receive buffer empty
	{
		Update_ASC0TimeoutRef();
		reload_timeout_job(irma_rec_timeout_job);
		IRMARecBuff[++IRMARecBuffPos] = (byte)lb;

		if( ASC0Error.OverrunErrorB )
		{
			ASC0Error.OverrunErrorB = FALSE;
#ifdef GLORIA
			if( IRMA_ASC0_Overrun_Errors != 0xFF )
				IRMA_ASC0_Overrun_Errors++;
#endif
			abort_uip_frame_reading();
			return(FALSE);
		}
		else
			return(TRUE);
	}	// end of "if( lb != EOF )"
	else
		return(FALSE);
}	// get_char_asc0_uip


void read_uip_frame(void)
{
	byte rec_byte;

	while( get_char_asc0_uip() )
	{
		rec_byte = IRMARecBuff[IRMARecBuffPos];

		switch( IRMARecBuffPos )
		{
			case 1 :	if( rec_byte != IRMAPrefix2 )
						{
							abort_uip_frame_reading();
							return;
						}
						break;

			case 2 :	if( rec_byte != IRMAPrefix3 )
						{
							abort_uip_frame_reading();
							return;
						}
						break;

			case 3 :	if( rec_byte != IRMAPrefix4 )
						{
							abort_uip_frame_reading();
							return;
						}
						break;

			case 4 :	IrmaDesAddr = rec_byte;
						break;

			case 5 :	IrmaSrcAddr = rec_byte;
						if( IrmaDesAddr == 0 && IrmaSrcAddr == 0 )
							Set_Parser_function(extended_uip_link);
						else
							Set_Parser_function(read_uip_10_frame);
						return;
						// Add check for error IrmaDesAddr == IrmaSrcAddr ?

			default:	abort_uip_frame_reading();
						return;
		}	// end of "switch( IRMARecBuffPos )"
	}	// end of "while( get_char_asc0_uip() )"
}	// read_uip_frame


void read_uip_10_frame(void)
{
	byte rec_byte, cs;
	int IRMARecBuffEndBackup;

	while( get_char_asc0_uip() )
	{
		rec_byte = IRMARecBuff[IRMARecBuffPos];

		switch( IRMARecBuffPos )
		{
			case 1 :
			case 2 :
			case 3 :
			case 4 :
			case 5 :	abort_uip_frame_reading();
						break;

			// message length
			case 6 :	if( rec_byte > UIP_10_MSG_LEN_MAX )
							abort_uip_frame_reading();
						// Calculate receive buffer index where checksum will be stored.
						IRMARecBuffEnd = rec_byte + UIP_10_OVERHEAD - 1;
						// Payload length is checked on processing of message.
						break;

			default:	break;
		}	// end of "switch( IRMARecBuffPos )"

		if( IRMARecBuffPos >= IRMARecBuffEnd )
		{
			cs = calculate_irma_checksum(IRMARecBuff, (word)IRMARecBuffEnd);
			if( cs != rec_byte )
			{
				abort_uip_frame_reading();
				return;
			}

#ifdef GATEWAY
			uip_service_lev = 0;
			uip_retry_flagB = FALSE;
#endif

			IRMARecBuffEndBackup = IRMARecBuffEnd;
			reset_uip_frame_reading();
			uip_10_frame_process(IRMARecBuffEndBackup);
			return;
		}	// end of "if( IRMARecBuffPos >= IRMARecBuffEnd )"
	}	// end of "while( get_char_asc0_uip() )"
}	// read_uip_10_frame


#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
void uip_10_frame_process_a21(int IRMARecBuffEndBackup)
{
	byte rec_byte;
	#if defined(GLORIA) && defined(DEVTEST)
	byte EmbPayloadLen;
	byte EmbMsgPos;
	#endif

	#ifdef GLORIA
	if( IrmaSrcAddr == EXT_NETWORK_SERVER_ADDR )
		GLORIA_Flags.ExtNetwork_Message_ReceivedB = TRUE;
	#endif
				
	if(	IrmaDesAddr != ConfigData.irma.address && IrmaDesAddr != IRMAAddrUniversalDest )
	{
	#ifdef GLORIA
		// Enable response to genuine IRMA messages transmitted by GLORIA module software
		// (source addresses 0xFD, 0xFC) also in case A21 device address is not equal to 1.
		if(	IrmaSrcAddr != GLORIA_MODULE_ADDR && IrmaSrcAddr != EXT_NETWORK_SERVER_ADDR || 
		    IrmaDesAddr != GLORIA_MASTER_ADDR )
		{
	#endif
	#if defined(GLORIA) && defined(DEVTEST)
			Resp_to_EmbMsg_ExpectedB = FALSE;
	#endif
			// Route IRMA message from service interface to system interface if source address
			// is equal to reserved value 0xFE (service device). 
			if( ASC0_Debug_ModeB )
			{
	#ifdef GLORIA
				if( IrmaSrcAddr == IRMAAddrServiceDev && 
				    (IrmaDesAddr == GLORIA_MODULE_ADDR || IrmaDesAddr == EXT_NETWORK_SERVER_ADDR) )
	#else
				if( IrmaSrcAddr == IRMAAddrServiceDev && Routing_To_SysInterf_ActiveB )
	#endif
					route_uip_10_frame(); 
			}
			return;
	#ifdef GLORIA
		}
	#endif
	}

	rec_byte = IRMARecBuff[UIP_10_FRAME_DK];
	if( rec_byte < '@' || rec_byte > 'z' )
	{
	#if defined(GLORIA) && defined(DEVTEST)
		Resp_to_EmbMsg_ExpectedB = FALSE;
	#endif
		Send_IRMA_Error_v10(IrmaSrcAddr, rec_byte, IRMAErr_UnkTypeIdent, 0, NULL);
		return;
	}

	if( Recovery_Mode && rec_byte != 'M' && rec_byte != 'P' && rec_byte != 'R' && rec_byte != 'U' && rec_byte != 'I' )
		return;

	if(	IrmaSrcAddr == IRMAAddrUniversalDest )
	{
		Send_IRMA_Error_v10(IrmaSrcAddr, rec_byte, IRMAErr_SrcAddrNotAllowed, 1, &IrmaSrcAddr);
		return;
	}

	rec_byte -= '@';
	if( IRMACmdTable[rec_byte].call_function == NULL )
	{
	#if defined(GLORIA) && defined(DEVTEST)
		Resp_to_EmbMsg_ExpectedB = FALSE;
	#endif
		Send_IRMA_Error_v10(IrmaSrcAddr, rec_byte + '@', IRMAErr_UnkTypeIdent, 0, NULL);
		return;
	}

	if( !IRMA_Frame_ReceivedB )
	{
		remove_timeout_job(Switch_IRMA_Prot_Check);
		IRMA_Frame_ReceivedB = TRUE;
	}

	// Payload may be now handled as zero terminated string. 
	IRMARecBuff[IRMARecBuffEndBackup] = '\0';

	#if defined(GLORIA) && defined(DEVTEST)
	if( Resp_to_EmbMsg_ExpectedB )
	{
		Resp_to_EmbMsg_ExpectedB = FALSE;
		if( rec_byte == (EmbMsgDI - '@') && IrmaSrcAddr == EmbMsgDesAddr && IrmaDesAddr == EmbMsgSrcAddr )
		{
			// Local variable IRMARecBuffEndBackup contains length of received IRMA frame excluding checksum.
			EmbPayloadLen = IRMARecBuffEndBackup - UIP_10_FRAME_LENGTH_MIN + 1;
			init_uip_send_buff_writing_10('A', 0x10, EmbPayloadLen + IRMA_Av10_EmbLen);
			IRMASendBuff[IRMASendBuffWritePos++] = EmbMsgSrcAddr;
			IRMASendBuff[IRMASendBuffWritePos++] = EmbMsgDesAddr;
			IRMASendBuff[IRMASendBuffWritePos++] = EmbMsgDI;
			IRMASendBuff[IRMASendBuffWritePos++] = IRMARecBuff[UIP_10_FRAME_VER];
			// Copy received IRMA frame excluding checksum to transmit buffer.
			for( EmbMsgPos = UIP_10_FRAME_W1; EmbMsgPos < IRMARecBuffEndBackup; EmbMsgPos++ )
				IRMASendBuff[IRMASendBuffWritePos++] = IRMARecBuff[EmbMsgPos];
			send_buffered_uip_10_frame(IrmaASrcAddr);
			return;
		}
	}
	#endif

	IRMACmdTable[rec_byte].call_function(IrmaSrcAddr,
                                         IRMARecBuff[UIP_10_FRAME_VER],
                                         IRMARecBuff[UIP_10_FRAME_DL] - UIP_10_MESSAGE_DATAOFFSET,
                                         &IRMARecBuff[UIP_10_FRAME_W1]);
}	// uip_10_frame_process_a21
#endif	// end of "#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)"


void Put_word_IRMASendBuff(word value)
{
	byte *p = IRMASendBuff + IRMASendBuffWritePos;

	*p       = value;
	*(p + 1) = value >> 8;
	IRMASendBuffWritePos += sizeof(word);
}	// Put_word_IRMASendBuff


void Put_dword_IRMASendBuff(dword value)
{
	byte *p = IRMASendBuff + IRMASendBuffWritePos;

	*p       = value;
	*(p + 1) = value >> 8;
	*(p + 2) = value >> 16;
	*(p + 3) = value >> 24;
	IRMASendBuffWritePos += sizeof(dword);
}	// Put_dword_IRMASendBuff


void init_uip_send_buff_writing_10(char DataId, byte DataVer, byte PayloadLen)
{
#ifdef GATEWAY
	byte service_level;

	if( is_uip_20_sendB )
	{
		service_level = uip_service_lev_send & 0x0F;

		IRMASendBuffWriteStart = UIP_20_FRAME_DL;
		IRMASendBuff[UIP_20_FRAME_LEV] = 0x20 | service_level;
		IRMASendBuff[UIP_20_FRAME_SRC] = ConfigData.irma.address;

		IRMASendBuffWritePos = IRMASendBuffWriteStart;
		IRMASendBuff[IRMASendBuffWritePos++] = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;
		IRMASendBuff[IRMASendBuffWritePos++] = 0;

		IRMASendBuff[IRMASendBuffWritePos++] = (byte)DataId;
		IRMASendBuff[IRMASendBuffWritePos++] = DataVer;
		IRMASendBuff[IRMASendBuffWritePos++] = uip_sub_cmd_send;
	}
	else
	{
	#ifdef J1708_PROTOCOL
		if( Current_ASC0_Prot_Id == PROT_ID_J1708 )
			IRMASendBuffWriteStart = J1708EmbIRMAOffsLen - 1;
		else
			IRMASendBuffWriteStart = UIP_10_FRAME_DL;
	#else
		IRMASendBuffWriteStart = UIP_10_FRAME_DL;
	#endif
		IRMASendBuff[UIP_10_FRAME_SRC] = ConfigData.irma.address;

		IRMASendBuffWritePos = IRMASendBuffWriteStart;
		IRMASendBuff[IRMASendBuffWritePos++] = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;

		IRMASendBuff[IRMASendBuffWritePos++] = (byte)DataId;
		IRMASendBuff[IRMASendBuffWritePos++] = DataVer;
	}
#else
	IRMASendBuffWritePos = IRMASendBuffWriteStart;
	IRMASendBuff[IRMASendBuffWritePos++] = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)DataId;
	IRMASendBuff[IRMASendBuffWritePos++] = DataVer;
#endif
}	// init_uip_send_buff_writing_10


void send_uip_10_frame(byte DesAddr, char DataId, byte DataVer, byte PayloadLen, byte *Payload)
{
	byte PayloadPos, FrameLen;
	
#ifdef GATEWAY
	if( is_uip_20_sendB )
	{
		IRMASendBuff[UIP_20_FRAME_DES] = DesAddr;

		if( DataId > 0 )
		{
			if( PayloadLen > UIP_10_PAYLOADLEN_MAX )
				PayloadLen = UIP_10_PAYLOADLEN_MAX;
			init_uip_send_buff_writing_10(DataId, DataVer, PayloadLen);
			for( PayloadPos = 0; PayloadPos < PayloadLen; PayloadPos++ )
				IRMASendBuff[IRMASendBuffWritePos++] = Payload[PayloadPos];
		}
		else
			PayloadLen = IRMASendBuff[UIP_20_FRAME_DL] - UIP_20_MESSAGE_DATAOFFSET;

		FrameLen = PayloadLen + UIP_20_FRAME_W1;
		IRMASendBuff[FrameLen] = calculate_irma_checksum(IRMASendBuff, (word)FrameLen);
		FrameLen++;

		// Store UIP frame length for possible transmission retry.
		uip_send_buff_frame_len = (word)FrameLen;

		WriteData_ASC0(IRMASendBuff, (word)FrameLen);
	}
	else
	{
#endif
		IRMASendBuff[UIP_10_FRAME_DES] = DesAddr;

		if( DataId > 0 )
		{
			if( PayloadLen > UIP_10_PAYLOADLEN_MAX )
				PayloadLen = UIP_10_PAYLOADLEN_MAX;
			init_uip_send_buff_writing_10(DataId, DataVer, PayloadLen);
			for( PayloadPos = 0; PayloadPos < PayloadLen; PayloadPos++ )
				IRMASendBuff[IRMASendBuffWritePos++] = Payload[PayloadPos];
		}
		else
			PayloadLen = IRMASendBuff[UIP_10_FRAME_DL] - UIP_10_MESSAGE_DATAOFFSET;

		FrameLen = PayloadLen + UIP_10_FRAME_W1;
		IRMASendBuff[FrameLen] = calculate_irma_checksum(IRMASendBuff, (word)FrameLen);
		FrameLen++;

		// Store UIP frame length for possible transmission retry.
		uip_send_buff_frame_len = (word)FrameLen;

		WriteData_ASC0(IRMASendBuff, (word)FrameLen);
#ifdef GATEWAY
	}
#endif
}	// send_uip_10_frame


void send_buffered_uip_10_frame(byte DesAddr)
{
	send_uip_10_frame(DesAddr, 0, 0, 0, NULL);
}	// send_buffered_uip_10_frame


#ifdef GATEWAY
void init_uip_send_buff_writing_20(byte service_level, char DataId, byte DataVer, char sub_cmd, word PayloadLen)
{
	word dl;

	service_level &= 0x0F;
	dl = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;

	IRMASendBuffWriteStart = UIP_20_FRAME_DL;
	IRMASendBuff[UIP_10_FRAME_DES] = 0;
	IRMASendBuff[UIP_10_FRAME_SRC] = 0;
	IRMASendBuff[UIP_20_FRAME_LEV] = 0x20 | service_level;
	IRMASendBuff[UIP_20_FRAME_SRC] = ConfigData.irma.address;

	IRMASendBuffWritePos = IRMASendBuffWriteStart;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dl;			// little endian
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)(dl >> 8);
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)DataId;
	IRMASendBuff[IRMASendBuffWritePos++] = DataVer;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)sub_cmd;
}	// init_uip_send_buff_writing_20


void send_uip_20_frame(byte service_level, byte DesAddr, char DataId, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	word PayloadPos, dl, FrameLen;
	
	IRMASendBuff[UIP_20_FRAME_DES] = DesAddr;
	if( DataId > 0 )
	{
		if( PayloadLen > UIP_20_PAYLOADLEN_MAX )
			PayloadLen = UIP_20_PAYLOADLEN_MAX;
		init_uip_send_buff_writing_20(service_level, DataId, DataVer, sub_cmd, PayloadLen);
		for( PayloadPos = 0; PayloadPos < PayloadLen; PayloadPos++ )
			IRMASendBuff[IRMASendBuffWritePos++] = Payload[PayloadPos];
	}
	else
	{
		dl = ((word)IRMASendBuff[UIP_20_FRAME_DL + 1] << 8) + (word)IRMASendBuff[UIP_20_FRAME_DL];
		PayloadLen = dl - UIP_20_MESSAGE_DATAOFFSET;
	}

	FrameLen = PayloadLen + UIP_20_FRAME_W1;
	IRMASendBuff[FrameLen] = calculate_irma_checksum(IRMASendBuff, FrameLen);
	FrameLen++;

	// Store UIP frame length for possible transmission retry.
	uip_send_buff_frame_len = FrameLen;

	WriteData_ASC0(IRMASendBuff, FrameLen);
}	// send_uip_20_frame


void send_buffered_uip_20_frame(byte DesAddr)
{
	send_uip_20_frame(0, DesAddr, 0, 0, 0, 0, NULL);
}	// send_buffered_uip_20_frame
#endif


void repeat_last_uip_frame_transmission(void)
{
	WriteData_ASC0(IRMASendBuff, uip_send_buff_frame_len);
}	// repeat_last_uip_frame_transmission


#ifdef GLORIA
void Init_GLORIA_SendBuff(void)
{
	GLORIASendBuff[0] = IRMAPrefix1;
	GLORIASendBuff[1] = IRMAPrefix2;
	GLORIASendBuff[2] = IRMAPrefix3;
	GLORIASendBuff[3] = IRMAPrefix4;
	GLORIASendBuff[UIP_10_FRAME_SRC] = GLORIA_MASTER_ADDR;
	GLORIASendBuff[UIP_10_FRAME_DES] = GLORIA_MODULE_ADDR;
}	// Init_GLORIA_SendBuff


void Init_GLORIA_SendBuff_Writing(byte DesAddr, char DataId, byte DataVer, byte PayloadLen)
{
	GLORIASendBuff[UIP_10_FRAME_DES] = DesAddr;
	GLORIASendBuffWritePos = UIP_10_FRAME_DL;
	GLORIASendBuff[GLORIASendBuffWritePos++] = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	GLORIASendBuff[GLORIASendBuffWritePos++] = DataId;
	GLORIASendBuff[GLORIASendBuffWritePos++] = DataVer;
}	// Init_GLORIA_SendBuff_Writing


void Send_GLORIA_Frame(void)
{
	byte FrameLen;
	
	FrameLen = GLORIASendBuff[UIP_10_FRAME_DL] + UIP_10_FRAME_DK;
	GLORIASendBuff[FrameLen] = calculate_irma_checksum(GLORIASendBuff, (word)FrameLen);
	FrameLen++;
	WriteData_ASC0(GLORIASendBuff, (word)FrameLen);
}	// Send_GLORIA_Frame


void Send_GLORIA_Query(byte DataId, byte DataVer)
{
	Init_GLORIA_SendBuff_Writing(GLORIA_MODULE_ADDR, DataId, DataVer, 0);
	Send_GLORIA_Frame();
}	// Send_GLORIA_Query
#endif	// end of "#ifdef GLORIA"


byte Init_SendBuff_Writing(byte DesAddr, char DataId, byte DataVer, byte PayloadLen, byte **SendBuff)
{
#ifdef GLORIA
	if( DesAddr == EXT_NETWORK_SERVER_ADDR ||
		DesAddr == GLORIA_MODULE_ADDR      || 
		DesAddr == GLORIA_SLAVE1_ADDR      ||
		DesAddr == GLORIA_SLAVE2_ADDR      ||
		DesAddr == GLORIA_SLAVE3_ADDR )
	{		
		Init_GLORIA_SendBuff_Writing(DesAddr, DataId, DataVer, PayloadLen);
		*SendBuff = GLORIASendBuff;
		return(GLORIASendBuffWritePos);
	}
#endif

	DesAddr;
	init_uip_send_buff_writing_10(DataId, DataVer, PayloadLen);
	*SendBuff = IRMASendBuff;
	return(IRMASendBuffWritePos);
}	// Init_SendBuff_Writing


void Send_Frame(byte DesAddr, byte *SendBuff)
{
#ifdef GLORIA
	if( SendBuff == GLORIASendBuff )
	{
		Send_GLORIA_Frame();
		return;
	}
#endif

	SendBuff;
	send_buffered_uip_10_frame(DesAddr);
}	// Send_Frame


#ifdef DEVTEST
bool irma_frame_section_transmission(void)
{
	byte PayloadLen, MsgSectLen, Pos;
	bool Not_Last_SectionB;

	if (SectionCnt > 0)
	{
		PayloadLen = MaxSectionPayloadLen;
		SectionCnt--;
		Not_Last_SectionB = TRUE;
	}
	else
	{
		PayloadLen = ResSectionPayloadLen;
		Not_Last_SectionB = FALSE;
	}
	IRMASectSendBuff[UIP_10_FRAME_W1]++;
	for (Pos = 1; Pos <= PayloadLen; Pos++)
		IRMASectSendBuff[UIP_10_FRAME_W1 + Pos] = IRMASendBuff[IRMASendBuffWritePos++];
	PayloadLen++;
	IRMASectSendBuff[UIP_10_FRAME_DL] = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	MsgSectLen = PayloadLen + UIP_10_FRAME_W1;
	IRMASectSendBuff[MsgSectLen] = calculate_irma_checksum(IRMASectSendBuff, (word)MsgSectLen);
	MsgSectLen++;
	WriteData_ASC0(IRMASectSendBuff, (word)MsgSectLen);
	return (Not_Last_SectionB);
}	// irma_frame_section_transmission


void init_multisection_irma_frame_transmission(byte DesAddr)
{
	byte PayloadLen, MsgLen, Pos;

	IRMASendBuff[UIP_10_FRAME_DES] = DesAddr;
	PayloadLen = IRMASendBuff[UIP_10_FRAME_DL] - UIP_10_MESSAGE_DATAOFFSET;
	SectionCnt = 0;
	if (MaxSectionPayloadLen > 0)
	{
		SectionCnt = PayloadLen / MaxSectionPayloadLen;
		ResSectionPayloadLen = PayloadLen % MaxSectionPayloadLen;
	}
	if (SectionCnt == 0)
	{
		MsgLen = PayloadLen + UIP_10_FRAME_W1;
		IRMASendBuff[MsgLen] = calculate_irma_checksum(IRMASendBuff, (word)MsgLen);
		MsgLen++;
		WriteData_ASC0(IRMASendBuff, (word)MsgLen);
	}
	else
	{
		for (Pos = 0; Pos < UIP_10_FRAME_W1; Pos++)
			IRMASectSendBuff[Pos] = IRMASendBuff[Pos];
		IRMASectSendBuff[UIP_10_FRAME_W1] = (SectionCnt + 1) << 4;
		// Global var. IRMASendBuffWritePos used as read pointer for section transmission.
		IRMASendBuffWritePos = UIP_10_FRAME_W1;
		irma_frame_section_transmission();
		To_Transmit_Next_Irma_Frame_SectionB = FALSE;
		// Following runlevel job will not be deleted during software runtime.
		Add_Runlevel_Job(cont_multisection_irma_frame_transmission);
		add_timeout_job(SectionTransmissionDelay, launch_cont_multisection_irma_frame_transmission);
	}
}	// init_multisection_irma_frame_transmission


void launch_cont_multisection_irma_frame_transmission(void)
{
	To_Transmit_Next_Irma_Frame_SectionB = TRUE;
}	// launch_cont_multisection_irma_frame_transmission


void cont_multisection_irma_frame_transmission(void)
{
	if (To_Transmit_Next_Irma_Frame_SectionB)
	{
		if (irma_frame_section_transmission())
			add_timeout_job(SectionTransmissionDelay, launch_cont_multisection_irma_frame_transmission);
		To_Transmit_Next_Irma_Frame_SectionB = FALSE;
	}
}	// cont_multisection_irma_frame_transmission
#endif	// end of "#ifdef DEVTEST"


void Check_ErrDataLen(byte *ErrDataLen, byte *ErrData, byte ErrDataLenMax)
{
	if( *ErrDataLen == 0xFF && ErrData != NULL )
		*ErrDataLen = strlen((char *)ErrData);

	if( *ErrDataLen > ErrDataLenMax)
		*ErrDataLen = ErrDataLenMax;
}	// Check_ErrDataLen


void Build_IRMA_Error_v10(byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData)
{
	byte ErrDataPos;

	Check_ErrDataLen(&ErrDataLen, ErrData, UIP_10_PAYLOADLEN_MAX - 2);
	init_uip_send_buff_writing_10('E', 0x10, 2 + ErrDataLen);
	IRMASendBuff[IRMASendBuffWritePos++] = ErrDataId;
	IRMASendBuff[IRMASendBuffWritePos++] = ErrNo;
	for( ErrDataPos = 0; ErrDataPos < ErrDataLen; ErrDataPos++ )
		IRMASendBuff[IRMASendBuffWritePos++] = ErrData[ErrDataPos];
}	// Build_IRMA_Error_v10


#ifdef GLORIA
void Build_GLORIA_Error_v10(byte DesAddr, byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData)
{
	byte ErrDataPos;

	Check_ErrDataLen(&ErrDataLen, ErrData, UIP_10_PAYLOADLEN_MAX - 2);
	Init_GLORIA_SendBuff_Writing(DesAddr, 'E', 0x10, 2 + ErrDataLen);
	GLORIASendBuff[GLORIASendBuffWritePos++] = ErrDataId;
	GLORIASendBuff[GLORIASendBuffWritePos++] = ErrNo;
	for( ErrDataPos = 0; ErrDataPos < ErrDataLen; ErrDataPos++ )
		GLORIASendBuff[GLORIASendBuffWritePos++] = ErrData[ErrDataPos];
}	// Build_GLORIA_Error_v10
#endif	// end of "#ifdef GLORIA"


void Send_IRMA_Error_v10(byte DesAddr, byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData)
{
#ifdef GLORIA
	byte *SendBuff, SendBuffWritePos;
	byte ErrDataPos;

	Check_ErrDataLen(&ErrDataLen, ErrData, UIP_10_PAYLOADLEN_MAX - 2);
	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'E', 0x10, 2 + ErrDataLen, &SendBuff);
	SendBuff[SendBuffWritePos++] = ErrDataId;
	SendBuff[SendBuffWritePos++] = ErrNo;
	for( ErrDataPos = 0; ErrDataPos < ErrDataLen; ErrDataPos++ )
		SendBuff[SendBuffWritePos++] = ErrData[ErrDataPos];
	Send_Frame(DesAddr, SendBuff);
#else
	Build_IRMA_Error_v10(ErrDataId, ErrNo, ErrDataLen, ErrData);
	send_buffered_uip_10_frame(DesAddr);
#endif	// end of "#ifdef GLORIA"
}	// Send_IRMA_Error_v10


void Send_Error_InvL7Length(byte DesAddr, byte ErrDataId, byte PayloadLen)
{
	byte ErrVal;

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Send_IRMA_Error_v10(DesAddr, ErrDataId, IRMAErr_InvMsgLen, 1, &ErrVal);
}	// Send_Error_InvL7Length


void Send_IRMA_Error_v11(byte DesAddr, byte ErrDataId, byte ErrDataVer, byte ErrNo, byte ErrDataLen, byte *ErrData)
{
	byte *SendBuff, SendBuffWritePos;
	byte ErrDataPos;

	Check_ErrDataLen(&ErrDataLen, ErrData, UIP_10_PAYLOADLEN_MAX - 3);
	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'E', 0x11, 3 + ErrDataLen, &SendBuff);
	SendBuff[SendBuffWritePos++] = ErrNo;
	SendBuff[SendBuffWritePos++] = ErrDataId;
	SendBuff[SendBuffWritePos++] = ErrDataVer;
	if( ErrData != NULL )
	{
		for(ErrDataPos = 0; ErrDataPos < ErrDataLen; ErrDataPos++)
			SendBuff[SendBuffWritePos++] = ErrData[ErrDataPos];
	}
	Send_Frame(DesAddr, SendBuff);
}	// Send_IRMA_Error_v11


/*---------------------------------------------------------------------------------------- C ---*/
bool Query_FAIdx_ValidB(byte PayloadLen, byte *Payload, byte *FAIdx)
{
	byte FAAddr;

	*FAIdx = 0xFF;
	if( PayloadLen == 1 )
	{
		FAAddr = Payload[0];
		*FAIdx = Get_FA_Index(FAAddr);
		if( *FAIdx == 0xFF && FAAddr != 0xFF )
			return(FALSE);
	}
	return(TRUE);
}	// Query_FAIdx_ValidB


#ifndef DEVTEST
bool Query_HCLims_ValidB(byte *Payload, byte *HCIdx)
{
	byte HC_LowLim, HC_UppLim, HCCnt;

	HC_LowLim = Payload[0];
	HC_UppLim = Payload[1];

	HCCnt = 0;
	if( Height_Classification_SupportedB )
		HCCnt = ConfigData.Height_Class_No;

	// Brackets are necessary to give dereferencing priority over increment.
	// As a result height class index is incremented, not pointer to height class index.
	for( *HCIdx = 0; *HCIdx < HCCnt; (*HCIdx)++ )
		if( HC_LowLim == ConfigData.Height_Classes_LowLim[*HCIdx] &&
			HC_UppLim == ConfigData.Height_Classes_UppLim[*HCIdx] )
			break;

	return(*HCIdx < HCCnt);
}	// Query_HCLims_ValidB
#endif	// end of "#ifndef DEVTEST"


/*---------------------------------------------------------------------------------------- F ---*/
void Build_IRMA_F_v10_Payload(void)
{
	byte PayloadLen;
	byte *FlagBytePtr = (byte *)&Operation_Flags;

	PayloadLen = sizeof(Operation_Flags);

	init_uip_send_buff_writing_10('F', 0x10, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = *FlagBytePtr;
	IRMASendBuff[IRMASendBuffWritePos++] = *(FlagBytePtr + 1);
	IRMASendBuff[IRMASendBuffWritePos++] = *(FlagBytePtr + 2);
	IRMASendBuff[IRMASendBuffWritePos++] = *(FlagBytePtr + 3);
}	// Build_IRMA_F_v10_Payload


byte IRMA_F_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte *FlagBytePtr = (byte *)&Operation_Flags;
	byte ErrVal;

	if( PayloadLen == 0 )						// Query of operation flags
	{
		Build_IRMA_F_v10_Payload();
		return(0);
	}

	if( PayloadLen == sizeof(Operation_Flags) )	// Setting of operation flags
	{
		disable_timer_and_timeout_jobs();
		*FlagBytePtr       = Payload[0];
		*(FlagBytePtr + 1) = Payload[1];
		*(FlagBytePtr + 2) = Payload[2];
		*(FlagBytePtr + 3) = Payload[3];
		if( Flags_Set_Func != NULL )
			Flags_Set_Func();
		enable_timer_and_timeout_jobs();

		Build_IRMA_F_v10_Payload();
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('F', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_F_v10_Resp


byte IRMA_F_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	if( SrcAddr == IRMAAddrServiceDev )
	{
		Result = IRMA_F_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'F', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_F_v10


void IRMA_Message_Flags(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_F_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'F', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Flags


/*---------------------------------------------------------------------------------------- I ---*/
void Add_IRMA_I_FAInfo(const byte FACnt)
{
	byte FAIdx, FA_Application;

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
	{
	 	IRMASendBuff[IRMASendBuffWritePos++] = ConfigData.function_area[FAIdx].address;
		if(  ConfigData.function_area[FAIdx].function.countingB    && 
		    !ConfigData.function_area[FAIdx].function.observationB &&
		    !ConfigData.function_area[FAIdx].function.controlB )
		{
#ifndef DEVTEST
			if( ConfigData.function_area[FAIdx].function.heightclassB && Height_Classification_SupportedB )
				FA_Application = 'H';
			else
#endif
				FA_Application = 'C';
		}
		else if( ConfigData.function_area[FAIdx].function.observationB )
			FA_Application = 'S';
		else
			FA_Application = '?';

		IRMASendBuff[IRMASendBuffWritePos++] = FA_Application;
	}
}	// Add_IRMA_I_FAInfo


void Build_IRMA_I_v10_v11_Payload(byte DataVer)
{
	byte FACnt, PayloadLen, ProdIdLen;

    FACnt = ConfigData.function_areas;
	PayloadLen = FACnt * 2;

	if( DataVer == 0x10 )
	{
		ProdIdLen   = IRMA_Iv10_ProdIdLen;
		PayloadLen += IRMA_Iv10_PayloadLenMin;
	}
	else
	{
		ProdIdLen   = IRMA_Iv11_ProdIdLen;
		PayloadLen += IRMA_Iv11_PayloadLenMin;
	}

	init_uip_send_buff_writing_10('I', DataVer, PayloadLen);
	memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)ConfigSystem.product_id, ProdIdLen - 1);
	IRMASendBuffWritePos += (ProdIdLen - 1);
	IRMASendBuff[IRMASendBuffWritePos++] = 0;
	IRMASendBuff[IRMASendBuffWritePos++] = ConfigSystem.device_id.year;
	IRMASendBuff[IRMASendBuffWritePos++] = ConfigSystem.device_id.serial_number;
	IRMASendBuff[IRMASendBuffWritePos++] = ConfigSystem.device_id.serial_number >> 8;
	Put_dword_IRMASendBuff(ADDR_CONFIG_DATA);
	IRMASendBuff[IRMASendBuffWritePos++] = FACnt;
	Add_IRMA_I_FAInfo(FACnt);
}	// Build_IRMA_I_v10_v11_Payload

	  
/*---------------------------------------------------------------------------------------- J ---*/
byte IRMA_J_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte subcmd_id, ErrVal;

	if( PayloadLen == 1 )
	{
		subcmd_id = Payload[0];
		switch( subcmd_id )
		{						
#ifdef SERIAL_SENSOR
			case  0  :
			case 'E' :	disable_timer_and_timeout_jobs();
						PWM_Stop();
						enable_timer_and_timeout_jobs();
						Build_IRMA_Error_v10('J', 0, 0, NULL);
						return(0);
#endif
#ifdef CAN_SENSOR
			case  0  :
			case 'E' :	Build_IRMA_Error_v10('J', IRMAErr_ExecutionErr, 0, NULL);
						return(IRMAErr_ExecutionErr);
#endif

#ifdef SERIAL_SENSOR
			case  1  :
			case 'S' :	disable_timer_and_timeout_jobs();
						PWM_Start();
						enable_timer_and_timeout_jobs();
						Build_IRMA_Error_v10('J', 0, 0, NULL);
						return(0);
#endif
#ifdef CAN_SENSOR
			case  1  :
			case 'S' :	Build_IRMA_Error_v10('J', IRMAErr_ExecutionErr, 0, NULL);
						return(IRMAErr_ExecutionErr);
#endif

			default  :	Build_IRMA_Error_v10('J', IRMAErr_UnkSubCmd, 1, &subcmd_id);
						return(IRMAErr_UnkSubCmd);
		}	//switch( subcmd_id )
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('J', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_J_v10_Resp


byte IRMA_J_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_J_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'J', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_J_v10


void IRMA_Message_IRed(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_J_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'J', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_IRed


/*---------------------------------------------------------------------------------------- M ---*/
byte IRMA_M_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal;
	dword MemAddr, MemAddrEnd;
	byte MemByteCnt, MemByteIdx;
#ifndef NO_LOGGER
	bool Logger_AddressB;
#endif

	if( PayloadLen != IRMA_Mv10_HeadLen )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('M', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr    = get_dword(Payload);
	MemByteCnt = Payload[4];
	MemAddrEnd = MemAddr + (dword)(MemByteCnt - 1);

	if( MemAddr > MaxMemAddrA21 )
	{
		Build_IRMA_Error_v10('M', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr/*Payload*/);
		return(IRMAErr_InvMemAddr);
	}

	if( MemByteCnt == 0 || MemByteCnt > IRMA_M_MemByteCnt_UppLim || 
	    MemAddrEnd > MaxMemAddrA21 )
	{
		Build_IRMA_Error_v10('M', IRMAErr_InvMemByteCnt, 1, &MemByteCnt);
		return(IRMAErr_InvMemByteCnt);
	}

#ifndef NO_LOGGER
	Logger_AddressB = FALSE;
	if( Valid_Logger_Address(MemAddr) )
	{
		if( Valid_Logger_Range(MemAddr, MemAddrEnd) == FALSE )
		{
			Build_IRMA_Error_v10('M', IRMAErr_InvMemByteCnt, 1, &MemByteCnt);
			return(IRMAErr_InvMemByteCnt);
		}

		reload_timeout_job(Unlock_Logger);
		Logger_AddressB = TRUE;
	}
#endif

	PayloadLen = IRMA_Mv10_HeadLen + MemByteCnt;
	init_uip_send_buff_writing_10('M', 0x10, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[0];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[1];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[2];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[3];
	IRMASendBuff[IRMASendBuffWritePos++] = MemByteCnt;

#ifndef NO_LOGGER
	if( Logger_AddressB )
	{
		Logger_Memory_Read(MemAddr, IRMASendBuff + IRMASendBuffWritePos, MemByteCnt);
		IRMASendBuffWritePos += MemByteCnt;		
	}
	else
#endif
		for( MemByteIdx = 0; MemByteIdx < MemByteCnt; MemByteIdx++ )
			IRMASendBuff[IRMASendBuffWritePos++] = *(byte huge *)(MemAddr++);

	return(0);
}	// IRMA_M_v10_Resp


#ifdef CAN_SENSOR
byte IRMA_M_v11_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal;
	dword MemAddr, MemAddrEnd;
	byte MemByteCnt, MemByteIdx;

	if( PayloadLen != IRMA_Mv11_HeadLen )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('M', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr  = (dword)Payload[2] << 16;
	MemAddr |= (dword)Payload[1] << 8;
	MemAddr |= (dword)Payload[0];
	MemByteCnt = Payload[3];
	MemAddrEnd = MemAddr + (dword)(MemByteCnt - 1);

	// 24-bit address always <= 0x00FFFFFF
//	if( MemAddr > MaxMemAddrA21 )
//	{
//		Build_IRMA_Error_v10('M', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr);
//		return(IRMAErr_InvMemAddr);
//	}

	if( MemByteCnt == 0 || MemByteCnt > IRMA_Mv11_MemByteCnt_UppLim || 
	    MemAddrEnd > MaxMemAddrA21 )
	{
		Build_IRMA_Error_v10('M', IRMAErr_InvMemByteCnt, 1, &MemByteCnt);
		return(IRMAErr_InvMemByteCnt);
	}

	PayloadLen = IRMA_Mv11_HeadLen + MemByteCnt;
	init_uip_send_buff_writing_10('M', 0x11, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[0];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[1];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[2];
	IRMASendBuff[IRMASendBuffWritePos++] = MemByteCnt;

	for( MemByteIdx = 0; MemByteIdx < MemByteCnt; MemByteIdx++ )
		IRMASendBuff[IRMASendBuffWritePos++] = *(byte huge *)(MemAddr++);

	return(0);
}	// IRMA_M_v11_Resp
#endif	// end of "#ifdef CAN_SENSOR"


byte IRMA_M_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_M_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'M', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_M_v10


void IRMA_Message_Memory(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_M_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'M', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Memory


/*---------------------------------------------------------------------------------------- N ---*/
byte IRMA_N_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal;
	dword MemAddr;
	byte _huge *MemPtr;
	int ErrNo;
	byte RunLevel;

	if( PayloadLen != IRMA_Nv10_HeadLen )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('N', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr = get_dword(Payload);
	if( !FlashProg_Allowed(MemAddr) )
	{
		Build_IRMA_Error_v10('N', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr/*Payload*/);
		return(IRMAErr_InvMemAddr);
	}

	RunLevel = Get_Runlevel();
	if( RunLevel == RUNLEVEL_SERV )
	{
		MemPtr = (byte *)MemAddr;
		ErrNo = Erase_Sector(MemPtr);
		if( ErrNo == 0 )
		{
			Build_IRMA_Error_v10('N', 0, 0, NULL);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('N', IRMAErr_MemWrite, 4, (byte *)&MemAddr/*Payload*/);
			return(IRMAErr_MemWrite);
		}
	}
	else
	{
		Build_IRMA_Error_v10('N', IRMAErr_InvRunLevel, 1, &RunLevel);
		return(IRMAErr_InvRunLevel);
	}
}	// IRMA_N_v10_Resp


byte IRMA_N_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_N_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'N', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_N_v10


void IRMA_Message_New(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_N_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'N', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_New


/*---------------------------------------------------------------------------------------- O ---*/
void Build_IRMA_O_v10_Payload(byte PortNo)
{
	byte PayloadLen;

	PayloadLen = sizeof(byte) + 3 * sizeof(word);
	init_uip_send_buff_writing_10('O', 0x10, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = PortNo;

	switch( PortNo )
	{
		case 2 :	Put_word_IRMASendBuff(DP2);
					Put_word_IRMASendBuff(ODP2);
					Put_word_IRMASendBuff(P2);
					break;

		case 3 :	Put_word_IRMASendBuff(DP3);
					Put_word_IRMASendBuff(ODP3);
					Put_word_IRMASendBuff(P3);
					break;

		case 4 :	Put_word_IRMASendBuff(DP4);
					Put_word_IRMASendBuff(0);
					Put_word_IRMASendBuff(P4);
					break;

		case 5 :	Put_word_IRMASendBuff(P5DIDIS);
					Put_word_IRMASendBuff(0);
					Put_word_IRMASendBuff(P5);
					break;

		case 6 :	Put_word_IRMASendBuff(DP6);
					Put_word_IRMASendBuff(ODP6);
					Put_word_IRMASendBuff(P6);
					break;

		case 7 :	Put_word_IRMASendBuff(DP7);
					Put_word_IRMASendBuff(ODP7);
					Put_word_IRMASendBuff(P7);
					break;

		case 8 :	Put_word_IRMASendBuff(DP8);
					Put_word_IRMASendBuff(ODP8);
					Put_word_IRMASendBuff(P8);
					break;
	}
}	// Build_IRMA_O_v10_Payload


byte IRMA_O_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte PortNo, ErrVal;

	if( PayloadLen == 1 )	// Incoming IRMA-O v1.0 request message
	{
	    PortNo = Payload[0];
		if( PortNo >= 2 && PortNo <= 8 )
		{
			Build_IRMA_O_v10_Payload(PortNo);
			return(0);
		}

		Build_IRMA_Error_v10('O', IRMAErr_InvL7Parameter, 1, &PortNo);
		return(IRMAErr_InvL7Parameter);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('O', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_O_v10_Resp


byte IRMA_O_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_O_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'O', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_O_v10


#if !defined(SW4) && !defined(SW2)
void IRMA_Message_Port(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_O_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'O', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Port
#endif	// end of "#if !defined(SW4) && !defined(SW2)"


/*---------------------------------------------------------------------------------------- P ---*/
bool RAM_Write_Allowed(dword MemAddr)
{
	return(MemAddr >= ADDR_APPLICRAM && MemAddr <= ADDR_APPLICRAM_END);
}	// RAM_Write_Allowed


byte Get_Writing_Permission(dword MemAddr)
{
	if( RAM_Write_Allowed(MemAddr) )
		return(RAM_ALLOWED);

	if( FlashProg_Allowed(MemAddr) )
		return(ROM_ALLOWED);

#ifndef NO_LOGGER
	if( Logger_Write_Allowed(MemAddr) )
		return(LOG_ALLOWED);
#endif

	return(NOT_ALLOWED);
}	// Get_Writing_Permission


byte IRMA_P_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal, MemByteCnt, RunLevel, Writing_Permission, Writing_Permission_End;
	dword MemAddr, MemAddrEnd;
	int ErrNo = 0;

	if( PayloadLen < IRMA_Pv10_HeadLen )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('P', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr = get_dword(Payload);
	MemByteCnt = Payload[4];

	Writing_Permission = Get_Writing_Permission(MemAddr);
	if( Writing_Permission == NOT_ALLOWED )
	{
		Build_IRMA_Error_v10('P', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr/*Payload*/);
		return(IRMAErr_InvMemAddr);
	}

	MemAddrEnd = MemAddr + (dword)(MemByteCnt - 1);
#ifndef NO_LOGGER
	if( Writing_Permission == LOG_ALLOWED )
	{
		if( Valid_Logger_Range(MemAddr, MemAddrEnd) )
			Writing_Permission_End = LOG_ALLOWED;
		else
			Writing_Permission_End = NOT_ALLOWED;
	}
	else
#endif
		Writing_Permission_End = Get_Writing_Permission(MemAddrEnd);

	if( MemByteCnt == 0 || MemByteCnt > IRMA_M_MemByteCnt_UppLim ||
		Writing_Permission_End == NOT_ALLOWED || Writing_Permission_End != Writing_Permission )
	{
		Build_IRMA_Error_v10('P', IRMAErr_InvMemByteCnt, 1, &MemByteCnt);
		return(IRMAErr_InvMemByteCnt);
	}

	RunLevel = Get_Runlevel();
#ifndef NO_LOGGER
	if( RunLevel == RUNLEVEL_SERV || Writing_Permission == LOG_ALLOWED )
#else
	if( RunLevel == RUNLEVEL_SERV )
#endif
	{
		switch( Writing_Permission )
		{
			case RAM_ALLOWED :	memcpy((void *)MemAddr, (void *)&Payload[5], MemByteCnt);
								ErrNo = memcmp((void *)MemAddr, (void *)&Payload[5], MemByteCnt);
								break;

			case ROM_ALLOWED :	ErrNo = Program_Flash(MemByteCnt, (byte huge *)&Payload[5], (byte huge *)MemAddr);
								break;

#ifndef NO_LOGGER
			case LOG_ALLOWED :	reload_timeout_job(Unlock_Logger);
								ErrNo = Logger_Memory_Write(MemAddr, &Payload[5], MemByteCnt);
								break;
#endif
		}

		if( ErrNo == 0 )
		{
			Build_IRMA_Error_v10('P', 0, 0, NULL);
			return(0);
		}											 
		else
		{
			Build_IRMA_Error_v10('P', IRMAErr_MemWrite, 4, (byte *)(&MemAddr));
			return(IRMAErr_MemWrite);
		}
	}
	else
	{
		Build_IRMA_Error_v10('P', IRMAErr_InvRunLevel, 1, &RunLevel);
		return(IRMAErr_InvRunLevel);
	}
}	// IRMA_P_v10_Resp


byte IRMA_P_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_P_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'P', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_P_v10


void IRMA_Message_Program(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_P_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'P', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Program


/*---------------------------------------------------------------------------------------- R ---*/
byte IRMA_R_v10_Resp(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte RunLevel, ErrVal, subcmd_id;
	int (*service_routine)(byte);
	int ErrNo;

	if( PayloadLen < IRMA_Rv10_PayloadLen )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('R', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	RunLevel = Get_Runlevel();
	if( RunLevel == RUNLEVEL_SERV )
	{
		subcmd_id = Payload[0];
		switch( subcmd_id )
		{						
			//case 'L' :	/* Copy reload routine from analyzer ROM to RAM
			//			and start execution in RAM (not implemented) */
			//case 0x0 :	/* Stop reload routine in RAM (not implemented) */
			//			if(PayloadLen != IRMA_Rv10_PayloadLen)
			//				break;
			//			Build_IRMA_Error_v10('R', IRMAErr_ExecutionErr, 0, NULL);
			//			return(IRMAErr_ExecutionErr);

			case '?' :	if( PayloadLen != IRMA_Rv10_PayloadLen )
							break;
						init_uip_send_buff_writing_10('R', 0x10, 9);
						IRMASendBuff[IRMASendBuffWritePos++] = '?';
						Put_dword_IRMASendBuff(ADDR_APPLICRAM);
						Put_dword_IRMASendBuff(ADDR_APPLICRAM_END - ADDR_APPLICRAM + 1);
						return(0);

			case 'S' :	if( PayloadLen != IRMA_Rv10_S_PayloadLen )
							break;
						service_routine = (void *)get_dword(Payload + 1);
						ErrNo = service_routine(SrcAddr);

						if( ErrNo == 0 )
						{
							Build_IRMA_Error_v10('R', 0, 0, NULL);
							return(0);
						}											 
						else
						{
							Build_IRMA_Error_v10('R', IRMAErr_ExecutionErr, 0, NULL);
							return(IRMAErr_ExecutionErr);
						}

			default :	Build_IRMA_Error_v10('R', IRMAErr_UnkSubCmd, 1, &subcmd_id);
						return(IRMAErr_UnkSubCmd);
		}	//switch( subcmd_id )

		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('R', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}
	else
	{
		Build_IRMA_Error_v10('R', IRMAErr_InvRunLevel, 1, &RunLevel);
		return(IRMAErr_InvRunLevel);
	}
}	// IRMA_R_v10_Resp


byte IRMA_R_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_R_v10_Resp(SrcAddr, PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'R', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_R_v10


void IRMA_Message_Run(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_R_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'R', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Run


/*---------------------------------------------------------------------------------------- T ---*/
#ifndef NO_LOGGER
void Build_IRMA_T_v10_Payload(void)
{
	Time_type Time;
	Date_type Date;
	byte PayloadLen;

	Get_Date_Time(&Date, &Time);
	PayloadLen = 8;
	init_uip_send_buff_writing_10('T', 0x10, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = Date.Year;
	IRMASendBuff[IRMASendBuffWritePos++] = Date.Month;
	IRMASendBuff[IRMASendBuffWritePos++] = Date.Day;
	IRMASendBuff[IRMASendBuffWritePos++] = Time.Hour;
	IRMASendBuff[IRMASendBuffWritePos++] = Time.Minutes;
	IRMASendBuff[IRMASendBuffWritePos++] = Time.Seconds;
	IRMASendBuff[IRMASendBuffWritePos++] = Logger_Interval.Minutes;
	IRMASendBuff[IRMASendBuffWritePos++] = Logger_Interval.Seconds;
}	// Build_IRMA_T_v10_Payload


byte IRMA_T_v10_Resp(byte PayloadLen, byte *Payload)
{
	Date_type Old_Date, Date;
	Time_type Old_Time, Time;
	byte ErrVal;

	if( PayloadLen == 6 )
	{
		if( Payload[0] != 0xFF && Payload[1] != 0xFF && Payload[2] != 0xFF &&
			Payload[3] != 0xFF && Payload[4] != 0xFF && Payload[5] != 0xFF )
		{
			Date.Year    = Payload[0];
			Date.Month   = Payload[1];
			Date.Day     = Payload[2];
			Time.Hour    = Payload[3];
			Time.Minutes = Payload[4];
			Time.Seconds = Payload[5];
			Set_Date_Time(&Old_Date, &Old_Time, &Date, &Time);
		}

	#ifdef DEVTEST
		if( Logger_Set_Time_Func != NULL )
			Logger_Set_Time_Func();
	#endif

		Build_IRMA_T_v10_Payload();
		return(0);
	}

	if( PayloadLen == 0 )
	{
		Build_IRMA_T_v10_Payload();
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('T', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_T_v10_Resp


byte IRMA_T_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_T_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'T', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_T_v10


void IRMA_Message_Time(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_T_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'T', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Time
#endif	// end of "#ifndef NO_LOGGER"


/*---------------------------------------------------------------------------------------- U ---*/
#ifdef GLORIA
void Set_GMS_RunLevel(byte RunLevel)
{
	if( GLORIA_Flags.Compatibility_ModeB )
	{
		// Former GMS accepts reset request (IRMA message U[0]) only if address of
		// transmitter is equal to address of service device (PC).
		GLORIASendBuff[UIP_10_FRAME_SRC] = IRMAAddrServiceDev;
	}
	Init_GLORIA_SendBuff_Writing(GLORIA_MODULE_ADDR, 'U', 0x10, 1);
	GLORIASendBuff[GLORIASendBuffWritePos++] = RunLevel;
	Send_GLORIA_Frame();
	GLORIASendBuff[UIP_10_FRAME_SRC] = GLORIA_MASTER_ADDR;
}	// Set_GMS_RunLevel


void Send_GMS_RunLevel_Request(void)
{
	Send_GLORIA_Query('U', 0x10);
}
#endif


byte IRMA_U_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte Requested_RunLevel, Current_RunLevel;
	bool Runlevel_Change_PermittedB = FALSE;
	byte ErrVal;

	Current_RunLevel = Get_Runlevel();

	if( PayloadLen == 1 )
	{
		Requested_RunLevel = Payload[0];
		if( Requested_RunLevel <= RUNLEVEL_APPL )
		{
			switch( Requested_RunLevel )
			{
				// Switching between run levels 4 and 5 allowed anyway.
				// Switching to run level 3 is an "one-way street".
				// Software reset needed to return to normal operation from run level 3 or 2.
				// This reset is caused by setting run level to 0.
				case 4 :	Runlevel_Change_PermittedB = RunLevel3Main != NULL && Runlevel_4_or_5();
							break;
				case 5 :	Runlevel_Change_PermittedB = RunLevel5Main != NULL && Runlevel_4_or_5();
							break;
				case 0 :	
				case 3 :	Runlevel_Change_PermittedB = TRUE;
							break;
			}

			if( Runlevel_Change_PermittedB )
			{
				if( Requested_RunLevel == RUNLEVEL_RESET )
					Reset_Request_ReceivedB = TRUE;
				else 
					if( Requested_RunLevel != Current_RunLevel )
					{
#ifdef CAN_SENSOR				
						if( Requested_RunLevel == RUNLEVEL_APPL && can_sensor_init_status < 8 )
							Requested_RunLevel = RUNLEVEL_COMM;
#endif
						Set_Runlevel(Requested_RunLevel);
#ifndef NO_LOGGER				
						if( RunLevel_Log_Func != NULL )
							RunLevel_Log_Func(Current_RunLevel, Requested_RunLevel);
#endif
					}

				Build_IRMA_Error_v10('U', 0, 1, &Requested_RunLevel);
				return(0);
			}
			Build_IRMA_Error_v10('U', IRMAErr_SetNotAllowed, 1, &Requested_RunLevel);
			return(IRMAErr_InvRunLevel);
		}	// end of if( Requested_RunLevel <= RUNLEVEL_APPL )

		Build_IRMA_Error_v10('U', IRMAErr_InvRunLevel, 1, &Requested_RunLevel);
		return(IRMAErr_InvRunLevel);
	}

	if( PayloadLen == 0 )
	{
		Build_IRMA_Error_v10('U', 0, 1, &Current_RunLevel);
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('U', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_U_v10_Resp


byte IRMA_U_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_U_v10_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'U', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_U_v10


void IRMA_Message_RunLevel(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_U_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'U', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_RunLevel


/*---------------------------------------------------------------------------------------- V ---*/
#ifdef GLORIA
void Software_Version_Request_Timeout(void)
{
	GLORIA_Flags.Software_Version_ExpectedB = FALSE;
	GLORIA_Flags.Communication_ErrorB       = TRUE;
	GLORIA_Communication_Error_Code         = GMS_VERSION_FAILED;
}	// Software_Version_Request_Timeout


void Send_IRMA_Version_v20(void)
{
	Send_GLORIA_Query('V', 0x20);
}	// Send_IRMA_Version_v20
#endif


byte IRMA_V_v2x_Resp(byte SrcAddr, byte DataVer, byte PayloadLen, byte **SendBuff)
{
	bool get_inherent_firmware_nameB;
	byte SendBuffWritePos;
	byte ErrVal;

	get_inherent_firmware_nameB = DataVer == 0x20;

	*SendBuff = IRMASendBuff;
	if( PayloadLen == 0 )	// Incoming IRMA-V v2.0 or v2.1 request message
	{
		// Pointer reference SendBuff is modified by function Init_SendBuff_Writing as needed.
		SendBuffWritePos = Init_SendBuff_Writing(SrcAddr, 'V', DataVer, IRMA_Vv20_PayloadLen, SendBuff);

		get_firmware_name(get_inherent_firmware_nameB, *SendBuff, &SendBuffWritePos);

	    return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	// Pointer reference SendBuff is modified by function Init_SendBuff_Writing as needed.
	SendBuffWritePos = Init_SendBuff_Writing(SrcAddr, 'E', 0x10, 3, SendBuff);
	// Brackets are necessary to give dereferencing priority over array access.
	(*SendBuff)[SendBuffWritePos++] = 'V';
	(*SendBuff)[SendBuffWritePos++] = IRMAErr_InvMsgLen;
	(*SendBuff)[SendBuffWritePos++] = ErrVal;
	return(IRMAErr_InvMsgLen);
}	// IRMA_V_v2x_Resp


byte IRMA_V_v20_Resp(byte SrcAddr, byte PayloadLen, byte *Payload, byte **SendBuff)
{
    byte Result;

	Payload;
	Result = IRMA_V_v2x_Resp(SrcAddr, 0x20, PayloadLen, SendBuff);
	return(Result);
}	// IRMA_V_v20_Resp


byte IRMA_V_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;
	byte *SendBuff;

#ifdef GLORIA
	if( PayloadLen == IRMA_Vv20_PayloadLen )
	{
		if( GLORIA_Flags.Software_Version_ExpectedB && SrcAddr == GLORIA_MODULE_ADDR )
		{
			remove_timeout_job(Software_Version_Request_Timeout);
			GLORIA_Flags.Software_Version_ExpectedB = FALSE;
			memcpy((void *)GLORIA_Software_Version, (void *)Payload, IRMA_Vv20_PayloadLen);
			GLORIA_Software_Version[IRMA_Vv20_PayloadLen - 1] = 0;
			GLORIA_Flags.GMS_RunLevel_SettingB = TRUE;
		}
	    return(0);
	}
#endif

	Result = IRMA_V_v20_Resp(SrcAddr, PayloadLen, Payload, &SendBuff);
	Send_Frame(SrcAddr, SendBuff);
	return(Result);
}	// IRMA_V_v20


byte IRMA_V_v21_Resp(byte SrcAddr, byte PayloadLen, byte *Payload, byte **SendBuff)
{
    byte Result;

	Payload;
	Result = IRMA_V_v2x_Resp(SrcAddr, 0x21, PayloadLen, SendBuff);
	return(Result);
}	// IRMA_V_v21_Resp


byte IRMA_V_v21(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;
	byte *SendBuff;

	Result = IRMA_V_v21_Resp(SrcAddr, PayloadLen, Payload, &SendBuff);
	Send_Frame(SrcAddr, SendBuff);
	return(Result);
}	// IRMA_V_v21


void IRMA_Message_Version(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x20 :	IRMA_V_v20(SrcAddr, PayloadLen, Payload);
					break;

		case 0x21 :	IRMA_V_v21(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'V', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Version


/*---------------------------------------------------------------------------------------- X ---*/
void Set_ASC0_Debug_Mode(void)
{
	Change_ASC0_Debug_Mode(TRUE);
}	// Set_ASC0_Debug_Mode


void Clear_ASC0_Debug_Mode(void)
{
	Change_ASC0_Debug_Mode(FALSE);
}	// Clear_ASC0_Debug_Mode


#ifndef GLORIA
void Set_ASC0_Route_Mode(void)
{
	Routing_To_SysInterf_ActiveB = TRUE;
}	// Set_ASC0_Route_Mode


void Clear_ASC0_Route_Mode(void)
{
	Routing_To_SysInterf_ActiveB = FALSE;
}	// Clear_ASC0_Route_Mode
#endif	// end of "#ifdef GLORIA"


byte IRMA_X_v10_Resp(byte PayloadLen, byte *Payload)
{
	byte n = 0;

	while( execute_command[n].argument[0] != '\0' )
	{
		if( strcmp((char *)Payload, execute_command[n].argument) == 0 )
		{
			if( execute_command[n].call_function != NULL )
				execute_command[n].call_function();
			break;
		}
		n++;
	}
	if( execute_command[n].argument[0] == '\0' )
	{
		Build_IRMA_Error_v10('X', IRMAErr_UnkCommand, PayloadLen, Payload);
		return(IRMAErr_UnkCommand);
	}
	else
	{
//		Build_IRMA_Error_v10('X', 0, 0, NULL);
		return(0);
	}
}	// IRMA_X_v10_Resp


byte IRMA_X_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	if( SrcAddr == IRMAAddrServiceDev )
	{
		Result = IRMA_X_v10_Resp(PayloadLen, Payload);
		if( Result > 0 )
			send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'X', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_X_v10


void IRMA_Message_Execute(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_X_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'X', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Execute
