/*==============================================================================================*
 |       Filename: uip.c                                                                        |
 | Project/Module: A21, GATEWAY or OBC/module group Communication                               |
 |           Date: 02/11/2011 (to be updated on any change)                                     |
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "..\kernel_interface.h"
#include "communication.h"
#if defined(SW2) || defined(SW4)
	#include "switches.h"
#endif
#include "uip.h"
#ifndef NO_LOGGER
	#include "logger.h"
#endif


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=COMM
#pragma class FC=COMM
#pragma class CO=COMM
#pragma class FB=COMMRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
void (*IRMA_Init_Prot_Function)(void);
RS232_Parameter_type IRMA_Init_RS232;
byte IRMA_Init_Prot_Id;

//byte IRMA_Status_Address;


/*----- Function Prototypes --------------------------------------------------------------------*/
void Init_IRMA(void);					// prototype of external function
#ifdef IBIS_PROTOCOL
void init_ibisip_cmplt(void);			// prototype of external function
#endif
#ifdef UJ1708IP
void Init_UJ1708IP_Cmplt(void);			// prototype of external function
#endif
#ifdef J1708_ORBITAL
void Init_J1708_Orbital_Cmplt(void);	// prototype of external function
#endif

void IRMA_Message_BaudRate(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
void IRMA_Message_Error(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#ifndef NO_LOGGER
void IRMA_Message_Logger(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif
#if defined(SW4) || defined(SW2)
void IRMA_Message_Port(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif

#ifndef NO_LOGGER
void Confirmation_Timeout(void);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/
const struct
{
	char name[5];
	byte prot_id;
	byte data_bits;
	char parity;
	byte stop_bits;
	void (*init_function)(void);
} ASC0ProtList[] = {
	{ "IRMA", PROT_ID_IRMA , 8, 'N', 1, Init_IRMA                },
#ifdef IBIS_PROTOCOL
	#ifdef IBIS_INEO
	{ "IBIS", PROT_ID_IBIS , 8, 'N', 1, init_ibisip_cmplt        },
	#else
	{ "IBIS", PROT_ID_IBIS , 7, 'E', 2, init_ibisip_cmplt        },
	#endif
#endif
#ifdef UJ1708IP
	{ "1708", PROT_ID_J1708, 8, 'N', 1, Init_UJ1708IP_Cmplt      },
#endif
#ifdef J1708_ORBITAL
	{ "1708", PROT_ID_J1708, 8, 'N', 1, Init_J1708_Orbital_Cmplt },
#endif
	{ ""    , 0            , 0, 0  , 0, NULL                     }
};

#if !defined(IBIS_PROTOCOL) && !defined(J1708_PROTOCOL)
const char CustomProtIdStr[CustomProtIdStrLen + 1] = { '_', '_', 0 };
#endif


/*----- Implementation of Functions ------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------- B ---*/
byte IRMA_B(byte DataVer, byte PayloadLen, byte *Payload)
{
	byte ErrVal = 0;
	byte idx = 0;
	bool validB;

	IRMA_Init_Prot_Function = NULL;
	IRMA_Init_RS232 = stRS232_IRMA_Standard;
	IRMA_Init_Prot_Id = PROT_ID_IRMA;
	if( DataVer == 0x10 )
	{
		if( PayloadLen == IRMA_Bv10_PayloadLen )
			IRMA_Init_Prot_Function = reset_uip_frame_reading;
		else
		{
			ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
			Build_IRMA_Error_v10('B', IRMAErr_InvMsgLen, 1, &ErrVal);
		}
	}
	else
	{
		if( DataVer == 0x11 )
		{
			if( PayloadLen == IRMA_Bv11_PayloadLen )
			{
				while( ASC0ProtList[idx].name[0] != '\0' )
				{
					if( strcmp((char *)&Payload[IRMA_Bv10_PayloadLen], ASC0ProtList[idx].name) == 0 )
					{
						IRMA_Init_Prot_Function = ASC0ProtList[idx].init_function;
						IRMA_Init_Prot_Id = ASC0ProtList[idx].prot_id;
						IRMA_Init_RS232.data_bits = ASC0ProtList[idx].data_bits;
						IRMA_Init_RS232.parity    = ASC0ProtList[idx].parity;
						IRMA_Init_RS232.stop_bits = ASC0ProtList[idx].stop_bits;
						break;
					}
					idx++;
				}
				if( ASC0ProtList[idx].name[0] == '\0' )
					// Unknown communication protocol.
					Build_IRMA_Error_v10('B', IRMAErr_UnkCommProt, PayloadLen - IRMA_Bv10_PayloadLen, &Payload[IRMA_Bv10_PayloadLen]);
			}
			else
			{
				ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
				Build_IRMA_Error_v10('B', IRMAErr_InvMsgLen, 1, &ErrVal);
			}
		}
		else
			Build_IRMA_Error_v10('B', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
	}
	if( IRMA_Init_Prot_Function == NULL )
		return(IRMASendBuff[IRMASendBuffWriteStart + 4]);

	IRMA_Init_RS232.baudrate = get_dword(Payload);

	validB = ASC0_baudrate_valid(IRMA_Init_RS232.baudrate);
	if( validB )
		validB = Payload[4] == IRMA_Init_RS232.data_bits &&
				 Payload[5] == IRMA_Init_RS232.parity    &&
				 Payload[6] >= IRMA_Init_RS232.stop_bits &&
				 Payload[7] == 0;

	if( validB )
		Build_IRMA_Error_v10('B', 0, PayloadLen, Payload);
	else
		// Invalid baud rate or invalid communication parameter.
		Build_IRMA_Error_v10('B', IRMAErr_InvASC0Param, IRMA_Bv10_PayloadLen, Payload);
	return(IRMASendBuff[IRMASendBuffWriteStart + 4]);
}	// IRMA_B


void IRMA_Message_BaudRate(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	byte ErrNo;

	ErrNo = IRMA_B(DataVer, PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	if( IRMA_Init_Prot_Function != NULL && ErrNo == 0 )
	{
		if( WaitForEndOfTransmission(1000, 1000) )
		{
			InitASC0(&IRMA_Init_RS232, IRMA_Init_Prot_Id);
			IRMA_Init_Prot_Function();
		}
		else
			Init_IRMA();
	}
}	// IRMA_Message_BaudRate


/*---------------------------------------------------------------------------------------- D ---*/
void Get_D_10_Data(byte FAIdx, D_10_type *D_10_Data)
{
	D_10_Data->FA_Addr = Comm_ConfigData->function_area[FAIdx].address;
	if( Comm_ConfigData->function_area[FAIdx].door.position.exactB )
	{
		D_10_Data->Left   = DeviceStatus.door[FAIdx].left.motionB << 7;
		D_10_Data->Left  += DeviceStatus.door[FAIdx].left.opening;
		D_10_Data->Right  = DeviceStatus.door[FAIdx].right.motionB << 7;
		D_10_Data->Right += DeviceStatus.door[FAIdx].right.opening;
	}
	else if( DeviceStatus.door[FAIdx].closedB )
	{
		D_10_Data->Left  = 0;
		D_10_Data->Right = 0;
	}
	else
	{
		D_10_Data->Left  = 100;
		D_10_Data->Right = 100;
	}
}	// Get_D_10_Data


void Write_D_10_Data_To_IRMASendBuff(byte FAIdx)
{
	D_10_type Buffer_D_10;

	Get_D_10_Data(FAIdx, &Buffer_D_10);
	memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)&Buffer_D_10, D_10_TYPE_SIZE);		
	IRMASendBuffWritePos += D_10_TYPE_SIZE;
}	// Write_D_10_Data_To_IRMASendBuff


void Build_IRMA_D_v10_Payload(byte FAIdx)
{
	byte FACnt;

	if( FAIdx != 0xFF )
	{
		init_uip_send_buff_writing_10('D', 0x10, 3);
		Write_D_10_Data_To_IRMASendBuff(FAIdx);
	}
	else
	{
		FACnt = Comm_ConfigData->function_areas;
		init_uip_send_buff_writing_10('D', 0x10, 3 * FACnt);
		for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
			Write_D_10_Data_To_IRMASendBuff(FAIdx);
	}
}	// Build_IRMA_D_v10_Payload


void Write_All_D_10_Data_To_IRMASendBuff(void)
{
	byte FACnt, FAIdx;

	FACnt = Comm_ConfigData->function_areas;
	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
		Write_D_10_Data_To_IRMASendBuff(FAIdx);
}	// Write_All_D_10_Data_To_IRMASendBuff


byte irma_D_v10_door_state_setting(byte PayloadLen, byte *Payload)
{
	byte FACnt, FAIdx, PayIdx;
	bool Setting_PermittedB;

	FACnt = Comm_ConfigData->function_areas;
    if( PayloadLen != D_10_TYPE_SIZE * FACnt )
	{
		// Number of function areas included in payload doesn't match with the number
		// of function areas configured in this IRMA Analyzer.
		Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
		return(IRMAErr_SetNotAllowed);
	}

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
		if( Payload[D_10_TYPE_SIZE * FAIdx] != Comm_ConfigData->function_area[FAIdx].address )
		{
			// At least one parameter FAi doesn't match the corresponding function area
			// address configured in this IRMA Analyzer.
			Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
			return(IRMAErr_SetNotAllowed);
		}

	init_uip_send_buff_writing_10('D', 0x10, 2 * D_10_TYPE_SIZE * FACnt);
	// Old door states:
	Write_All_D_10_Data_To_IRMASendBuff();

	for( PayIdx = 0, FAIdx = 0; PayIdx < PayloadLen; PayIdx += D_10_TYPE_SIZE, FAIdx++ )
	{
		if( Comm_ConfigData->function_area[FAIdx].door.position.exactB )
		{
			Save_ILVL_and_Disable_All_INTs();
			DeviceStatus.door[FAIdx].left.motionB  = (Payload[PayIdx + 1] & 0x80) == 0x80;
			DeviceStatus.door[FAIdx].left.opening  = (Payload[PayIdx + 1] & 0x7F);
			DeviceStatus.door[FAIdx].right.motionB = (Payload[PayIdx + 2] & 0x80) == 0x80;
			DeviceStatus.door[FAIdx].right.opening = (Payload[PayIdx + 2] & 0x7F);
			Restore_ILVL();
		}
		else
		{
			Setting_PermittedB = FALSE;
			if( Payload[PayIdx + 1] == 0 && Payload[PayIdx + 2] == 0 )
                Setting_PermittedB = Set_Door_Status(FAIdx, TRUE);
			else if( Payload[PayIdx + 1] == 100 && Payload[PayIdx + 2] == 100 )
                Setting_PermittedB = Set_Door_Status(FAIdx, FALSE);

			if( !Setting_PermittedB )
			{
				// Clear old door states: 
				IRMASendBuffWritePos -= PayloadLen;
				Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
				return(IRMAErr_SetNotAllowed);
			}
		}
	}
	Write_All_D_10_Data_To_IRMASendBuff();

	A21_Status.Sensor_Interface.Door_States_Ever_Set_By_MsgB = TRUE;
	return(0);
}	// irma_D_v10_door_state_setting


#ifndef SERIAL_SENSOR
byte IRMA_D_v10_Resp(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte FAIdx, Result, ErrVal;

	if( PayloadLen < 2 )					// query of door status
	{
		if( Query_FAIdx_ValidB(PayloadLen, Payload, &FAIdx) )
		{
			Build_IRMA_D_v10_Payload(FAIdx);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('D', IRMAErr_InvFAAddr, 1, Payload);
			return(IRMAErr_InvFAAddr);
		}
	}

	if( PayloadLen % D_10_TYPE_SIZE == 0 )	// setting of door status
	{
		// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
		if( SrcAddr >= IRMAAddrReserveFirst )
		{
			Result = irma_D_v10_door_state_setting(PayloadLen, Payload);
			return(Result);
		}
		else
		{
			Build_IRMA_Error_v10('D', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
			return(IRMAErr_SrcAddrNotAllowed);
		}
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('D', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_D_v10_Resp
#endif	// end of "#ifndef SERIAL_SENSOR"


void Send_IRMA_Door_v10(byte DesAddr, byte FAIdx)
{
	Build_IRMA_D_v10_Payload(FAIdx);
	send_buffered_uip_10_frame(DesAddr);
}	// Send_IRMA_Door_v10


byte IRMA_D_v12_Resp(byte PayloadLen, byte *Payload)
{
	byte FACnt, FAIdx, ErrVal;

	Payload;
	if( PayloadLen == 0 )			// query of door event counters
	{
	    FACnt = Comm_ConfigData->function_areas;
		PayloadLen = 1 + FACnt * 2 * sizeof(word);

		init_uip_send_buff_writing_10('D', 0x12, PayloadLen);
		IRMASendBuff[IRMASendBuffWritePos++] = FACnt;
		for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
		{
			Put_word_IRMASendBuff(door_opened_event_cnt[FAIdx]);
			Put_word_IRMASendBuff(door_closed_event_cnt[FAIdx]);
		}

		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('D', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_D_v12_Resp


byte IRMA_D_v12(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_D_v12_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_D_v12


/*---------------------------------------------------------------------------------------- E ---*/
void irma_E_v10_communication_test(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	if( PayloadLen > IRMA_Ev10_PayloadLenMin && Payload[1] == 0xFF )
		Send_IRMA_Error_v10(SrcAddr, 'E', 0, PayloadLen - 2, &Payload[2]);
}	// irma_E_v10_communication_test


#ifndef NO_LOGGER
void irma_E_v10_logger_confirmation(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	if( Payload[1] == 0 )
	{
		if( PayloadLen < 3 )
			Send_IRMA_Error_v10(SrcAddr, 'E', IRMAErr_InvRecConf, 0xFF, (byte *)"inadmissible length");
		else
			if( Confirm_Data_Record(PayloadLen - 2, &Payload[2]) != 0 )
				Send_IRMA_Error_v10(SrcAddr, 'E', IRMAErr_InvRecConf, 0xFF, (byte *)"not found");
	}
}	// irma_E_v10_logger_confirmation
#endif	// end of "#ifndef NO_LOGGER"


#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)
void IRMA_E_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte DataId;
	
	if( PayloadLen < IRMA_Ev10_PayloadLenMin )
		return;

	DataId = Payload[0];
	switch( DataId )
	{
		case 'E' :	irma_E_v10_communication_test(SrcAddr, PayloadLen, Payload);
					break;

	#ifndef NO_LOGGER
		case 'L' :	irma_E_v10_logger_confirmation(SrcAddr, PayloadLen, Payload);
					break;
	#endif	// end of "#ifndef NO_LOGGER"

		default  :	break;
	}
}	// IRMA_E_v10
#endif	// end of "#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)"


#ifndef NO_LOGGER
void irma_E_v11_logger_confirmation(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte ErrNo, ErrDataVer;
	
	SrcAddr;
	ErrNo      = Payload[0];
	ErrDataVer = Payload[2];

	if( ErrNo == 0 && ErrDataVer == 0x20 && PayloadLen == 4 && Payload[3] == 'D' &&
		Logger_Info.Confirmation_ExpectedB )
	{
			remove_timeout_job(Confirmation_Timeout);
			Logger_Info.Confirmation_ExpectedB = FALSE;
			Confirm_Data_Records_Range();
	}
}	// irma_E_v11_logger_confirmation
#endif	// end of "#ifndef NO_LOGGER"


#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)
void IRMA_E_v11(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte ErrNo, ErrDataId, ErrDataVer;
	
	if( PayloadLen < IRMA_Ev11_PayloadLenMin )
		return;

	SrcAddr;
	ErrNo      = Payload[0];
	ErrDataId  = Payload[1];
	ErrDataVer = Payload[2];

	switch( ErrDataId )
	{
	#ifndef NO_LOGGER
		case 'L' :	irma_E_v11_logger_confirmation(SrcAddr, PayloadLen, Payload);
					break;
	#endif	// end of "#ifndef NO_LOGGER"

		default  :	break;
	}
}	// IRMA_E_v11


void IRMA_Message_Error(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_E_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x11 :	IRMA_E_v11(SrcAddr, PayloadLen, Payload);
					break;
	}
}	// IRMA_Message_Error
#endif	// end of "#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)"


/*---------------------------------------------------------------------------------------- I ---*/
byte IRMA_I_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v1.0 request message
	{
		Build_IRMA_I_v10_v11_Payload(0x10);
		send_buffered_uip_10_frame(SrcAddr);
		return(0);
	}

	Send_Error_InvL7Length(SrcAddr, 'I', PayloadLen);
	return(IRMAErr_InvMsgLen);
}	// IRMA_I_v10


byte IRMA_I_v11_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal;

	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v1.1 request message
	{
		Build_IRMA_I_v10_v11_Payload(0x11);
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_I_v11_Resp


byte IRMA_I_v11(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_I_v11_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_I_v11


byte IRMA_I_v12_Resp(byte PayloadLen, byte *Payload)
{
	byte HCIdx, HCCnt;
	byte ErrVal;

	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v1.2 request message
	{
		HCCnt = 0;
#ifndef DEVTEST		
		if( Height_Classification_SupportedB )
			HCCnt = Comm_ConfigData->Height_Class_No;
#endif
		init_uip_send_buff_writing_10('I', 0x12, 2 * HCCnt + IRMA_Iv12_PayloadLenMin);
		IRMASendBuff[IRMASendBuffWritePos++] = HCCnt;
		for( HCIdx = 0; HCIdx < HCCnt; HCIdx++ )
		{
			IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigData->Height_Classes_LowLim[HCIdx];
			IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigData->Height_Classes_UppLim[HCIdx];
		}
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_I_v12_Resp


byte IRMA_I_v12(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_I_v12_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_I_v12


void Build_IRMA_I_v17_Payload(void)
{
	byte PayloadLen;
	byte s;

	PayloadLen = 1 + MAX_FLASH_SECTOR_CNT;

	init_uip_send_buff_writing_10('I', 0x17, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = FlashSectorCnt;
	for( s = FIRST_SECTOR; s <= LAST_SECTOR; s++ )
		IRMASendBuff[IRMASendBuffWritePos++] = SectorInformation[s].okB;
}	// Build_IRMA_I_v17_Payload


byte IRMA_I_v17_Resp(byte PayloadLen, byte *Payload)
{
	byte ErrVal;

	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v1.7 request message
	{
		Build_IRMA_I_v17_Payload();
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_I_v17_Resp


byte IRMA_I_v17(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_I_v17_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_I_v17


#ifdef IRMA_I_V40
void Build_IRMA_I_v40_Payload(void)
{
	char A21_ClassId[IrmaDevClassIdLen] = {'A', '2', '1', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	byte PayloadLen;

	PayloadLen = IrmaDevClassIdLen + 12 * sizeof(word) + 2 * sizeof(dword);

	init_uip_send_buff_writing_10('I', 0x40, PayloadLen);
	memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)A21_ClassId, IrmaDevClassIdLen);
	IRMASendBuffWritePos += IrmaDevClassIdLen;
	Put_word_IRMASendBuff(SYSCON);
	Put_word_IRMASendBuff(BUSCON0);
	Put_word_IRMASendBuff(BUSCON1);
	Put_word_IRMASendBuff(BUSCON2);
	Put_word_IRMASendBuff(BUSCON3);
	Put_word_IRMASendBuff(BUSCON4);
	Put_word_IRMASendBuff(ADDRSEL1);
	Put_word_IRMASendBuff(ADDRSEL2);
	Put_word_IRMASendBuff(ADDRSEL3);
	Put_word_IRMASendBuff(ADDRSEL4);
	Put_word_IRMASendBuff(Comm_ConfigData->ASC0_real.s0bg);
	Put_dword_IRMASendBuff(Comm_ConfigData->ASC0_real.baudrate);
	Put_dword_IRMASendBuff((dword)Comm_ConfigData->ASC0_real.deviation);
	Put_word_IRMASendBuff(C1BTR);
}	// Build_IRMA_I_v40_Payload

	  
byte IRMA_I_v40(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v4.0 request message
	{
		Build_IRMA_I_v40_Payload();
		send_buffered_uip_10_frame(SrcAddr);
		return(0);
	}

	Send_Error_InvL7Length(SrcAddr, 'I', PayloadLen);
	return(IRMAErr_InvMsgLen);
}	// IRMA_I_v40
#endif	// end of "#ifdef IRMA_I_V40"


/*---------------------------------------------------------------------------------------- L ---*/
#ifndef NO_LOGGER
void Confirmation_Timeout(void)
{
	Logger_Info.Confirmation_ExpectedB = FALSE;
}	// Confirmation_Timeout


	#ifdef GLORIA
void G_14_to_10(byte *Data)
{
	byte i, *p;
	G_10_type GPS_G_10;
	G_14_type GPS_G_14;

	for( i = 0, p = (byte *)&GPS_G_14; i < G_14_TYPE_SIZE; i++, p++ )
		*p = Data[i];

	GPS_G_10.Lat        = (long)GPS_G_14.Lat_High << 16 | GPS_G_14.Lat_Low;
	GPS_G_10.Lng        = (long)GPS_G_14.Lng_High << 16 | GPS_G_14.Lng_Low;
	GPS_G_10.Alt        = (long)GPS_G_14.Alt << G_14_ALT_SHIFT;
	GPS_G_10.Alt       -= G_14_ALT_OFFSET;
	GPS_G_10.Speed      = (dword)GPS_G_14.Speed;
	GPS_G_10.Course     = 0;
	GPS_G_10.GPS_Status = (byte)GPS_G_14.GPS_Status;

	for( i = 0, p = (byte *)&GPS_G_10; i < G_10_TYPE_SIZE; i++, p++ )
		Data[i] = *p;
}	// G_14_to_10
	#endif


void Send_IRMA_Logger_Status_v10(byte DesAddr)
{
	byte *SendBuff, SendBuffWritePos;
	byte Payload[0xFF] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0};
	byte PayloadLen = 9;
	byte i;
	word Total;
	#if defined(DEVTEST) && defined(BURNIN)
	Time_type Time;
	Date_type Date;
	#endif

	if( !Logger_Memory_Error() )
	{
	#if defined(DEVTEST) && defined(BURNIN)
		if( GLORIA_BurnIn_ModeB )
		{ 
			Get_Date_Time(&Date, &Time);
			Payload[0] = Date.Year;
			Payload[1] = Date.Month;
			Payload[2] = Date.Day;
			Payload[3] = Time.Hour;
			Payload[4] = Time.Minutes;
			Payload[5] = Time.Seconds;
		}
	#endif
		Total = Get_Total_Records_Number();
	#ifdef LIMIT_EVENT_NO
		if( Total > 9999 )
			Total = 9999;
	#endif
		Payload[7] = Total;
		Payload[8] = Total >> 8;

		for( i = 0; i <= 0xFE; i++ )
		{
			if( (Total = Get_Records_Number(i)) != 0 )
			{
				Payload[PayloadLen++] = i;

	#ifdef LIMIT_EVENT_NO
				if( Total > 9999 )
					Total = 9999;
	#endif
				Payload[PayloadLen++] = Total;
				Payload[PayloadLen++] = Total >> 8;

				if( PayloadLen > (UIP_10_PAYLOADLEN_MAX - 3) ) 	/* Avoid datagam length oversize */
					break;
			}
		}
	}

	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'L', 0x10, PayloadLen, &SendBuff);
	for( i = 0; i < PayloadLen; i++ ) 
		SendBuff[SendBuffWritePos++] = Payload[i];
	Send_Frame(DesAddr, SendBuff);
}	// Send_IRMA_Logger_Status_v10


void Send_IRMA_Logger_Data_v10(byte DesAddr)
{
	byte *SendBuff, SendBuffWritePos;
	byte i, PayloadLen, Payload[0xFF];
	#ifdef GLORIA
	byte DataId, DataVer;
	#endif

	if( Get_Data_Record(&PayloadLen, Payload) != 0x00 )
	{
		Send_IRMA_Logger_Status_v10(DesAddr);
		return;
	}

	#ifdef GLORIA
	DataId  = Payload[7];
	DataVer = Payload[8];

	if( (DataId == 'G' || DataId == 'g') && DataVer == 0x14 )
	{
		G_14_to_10(&Payload[9]);
		Payload[8] = 0x10;
		PayloadLen = G_10_TYPE_SIZE + 9;
	}
	#endif

	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'L', 0x10, PayloadLen, &SendBuff);
	for( i = 0; i < PayloadLen; i++ ) 
		SendBuff[SendBuffWritePos++] = Payload[i];
	Send_Frame(DesAddr, SendBuff);
}	// Send_IRMA_Logger_Data_v10


void Send_IRMA_Logger_Status_v20(byte DesAddr, byte Format)
{
	byte *SendBuff, SendBuffWritePos;
	byte PayloadLen, Max_PayloadLen, DataId;
	word Total;

	PayloadLen = 3;
	Max_PayloadLen = UIP_10_PAYLOADLEN_MAX;
	Total = Get_Total_Records_Number();

	switch( Format )
	{
		case 's' :	break;
		case 'S' :
	#ifdef GLORIA
					if( DesAddr == EXT_NETWORK_SERVER_ADDR )
						Max_PayloadLen -= IRMA_rv10_EmbLen;
	#endif
					for( DataId = 0; DataId <= 0xFE; DataId++ )
					{
						if( Get_Records_Number(DataId) != 0 )
						{
							if( PayloadLen > Max_PayloadLen - 3 )
								break;
							else
								PayloadLen += 3;
						}
					}
					break;

		default  :	return;
	}

	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'L', 0x20, PayloadLen, &SendBuff);
	SendBuff[SendBuffWritePos++] = Format;
	SendBuff[SendBuffWritePos++] = Total;
	SendBuff[SendBuffWritePos++] = Total >> 8;

	if( Format == 'S' )
	{
		PayloadLen = 3;
		for( DataId = 0; DataId <= 0xFE; DataId++ )
		{
			if( (Total = Get_Records_Number(DataId)) != 0 )
			{
				SendBuff[SendBuffWritePos++] = DataId;
				SendBuff[SendBuffWritePos++] = Total;
				SendBuff[SendBuffWritePos++] = Total >> 8;
				PayloadLen += 3;
				if( PayloadLen > Max_PayloadLen - 3 )
					break;
			}
		}
	}		

	Send_Frame(DesAddr, SendBuff);
}	// Send_IRMA_Logger_Status_v20


void Send_IRMA_Logger_Data_v20(byte DesAddr, byte RecNum)
{
	byte *SendBuff, SendBuffWritePos;
	byte PayloadLen, Max_PayloadLen;
	Record_Range_type Rec_Ran;
	byte ErrorData;

	if( Logger_Info.Confirmation_ExpectedB )
	{
		Logger_Info.Confirmation_ExpectedB = FALSE;
		remove_timeout_job(Confirmation_Timeout);
	}

	if( Get_Total_Records_Number() == 0 )
	{
		Send_IRMA_Logger_Status_v20(DesAddr, 'S');	// Logger is empty
		return;
	}

	Max_PayloadLen = UIP_10_PAYLOADLEN_MAX;
	#ifdef GLORIA
	if( DesAddr == EXT_NETWORK_SERVER_ADDR )
		Max_PayloadLen -= IRMA_rv10_EmbLen;
	#endif

	SendBuff = IRMASendBuff;
	//														     +---> Message length updated later	-->
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'L', 0x20, 0, &SendBuff);	//					   |
	SendBuff[SendBuffWritePos++] = 'D';											//					   |
	//														 +---> place for 'D'					   |
	if( Read_Data_Records_Same_Date(RecNum, Max_PayloadLen - 1,					//					   |
		&SendBuff[SendBuffWritePos], &PayloadLen, &Rec_Ran) == 0 )				//					   |
	{	// Some error condition was found by function "Read_Data_Records_Same_Date"              	   |
		ErrorData = 'D';                                                        //                     |
		Send_IRMA_Error_v11(DesAddr, 'L', 0x20, IRMAErr_LoggerIntegrityError, 1, &ErrorData); //       |
		return;										                            //    				   |
	}																			//					   |
	PayloadLen++;	// Payload length is incremented due to inclusion of subcommand 'D'				   |
	SendBuff[UIP_10_FRAME_DL] = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;		//	<---------------- here ----

	Logger_Info.Record_Range.First = Rec_Ran.First;
	Logger_Info.Record_Range.Last  = Rec_Ran.Last;
	Logger_Info.Confirmation_ExpectedB = TRUE;
	Send_Frame(DesAddr, SendBuff);
	// Extended for GPRS communication. Different values for ISM and GSM/GPRS ?
	if( Comm_ConfigData->memory[1] > 0 )
		add_timeout_job(Comm_ConfigData->memory[1] * 1000, Confirmation_Timeout);
	else
		add_timeout_job(1000, Confirmation_Timeout);
}	// Send_IRMA_Logger_Data_v20


void Send_IRMA_Logger_Locked_v20(byte DesAddr, byte Old_LLS)
{
	byte *SendBuff, SendBuffWritePos;
	byte Current_LLS = Get_Logger_Locked_State();

	SendBuff = IRMASendBuff;
	SendBuffWritePos = Init_SendBuff_Writing(DesAddr, 'L', 0x20, 3, &SendBuff);
	SendBuff[SendBuffWritePos++] = 'L';
	SendBuff[SendBuffWritePos++] = Old_LLS;
	SendBuff[SendBuffWritePos++] = Current_LLS;
	Send_Frame(DesAddr, SendBuff);
}	// Send_IRMA_Logger_Locked_v20


byte IRMA_L_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	switch( PayloadLen )
	{
		case 0 :	
	#ifdef GLORIA
					/* If a logger status request is answered before the startup sequency is complete,
					then GLORIA will inform the base station that OBC address is 0. Evaluation of 
					boolean variable "GLORIA_InitializedB" is used as prevention against this 
					malfunction. Evaluation is restricted to radio download of logger data only. 
					In case of cable download A21 connector "C" is occupied and flag
					"ASC0_Service_Interface_OccupiedB" is equal to "TRUE". OBC address is not used
					in this case (usage of UIP instead of OBC protocol).*/
					if( !ASC0_Service_Interface_OccupiedB && !GLORIA_InitializedB )
						break;
	#endif
					Send_IRMA_Logger_Status_v10(SrcAddr);
					break;

		case 1 :	Send_IRMA_Logger_Data_v10(SrcAddr);
					break;

		case 6 :	if( Payload[0] == 0xFF && strcmp((char *)&Payload[1], "RESET") == 0 )
					{
						Logger_Info.Reset_Request_Source = RESET_REQUEST_IRMA_L;
						Init_Logger();
						Send_IRMA_Error_v11(SrcAddr, 'L', 0x10, IRMAErr_Confirmation, PayloadLen, Payload);
						break;
					}

		default :	Send_Error_InvL7Length(SrcAddr, 'L', PayloadLen);
					return(IRMAErr_InvMsgLen);
	}

	return(0);
}	// IRMA_L_v10


byte IRMA_L_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte SubCmd_Id, Old_LLS, ErrNo, PayloadIdx;

	if( PayloadLen > 0 )
	{
		SubCmd_Id = Payload[0];
		switch( SubCmd_Id )
		{
			case 'S' :	// Logger status request; 'S': long format, 's': short format
			case 's' :	if( PayloadLen != IRMA_L_S_PayloadLen ) break;
						Send_IRMA_Logger_Status_v20(SrcAddr, SubCmd_Id);
						return(0);

			case 'D' : 	// Logger data request
						if( PayloadLen != IRMA_L_D_PayloadLen ) break;
						Send_IRMA_Logger_Data_v20(SrcAddr, Payload[1]);
						return(0);

			case 'L' :	// "Logger Locked" state administration
						if( PayloadLen != IRMA_L_L_PayloadLen ) break;
						Old_LLS = Get_Logger_Locked_State();

						switch( Payload[1] )
						{
							case 0   : 
							case 1   :	Set_Logger_Locked_State(Payload[1]);
							case '?' :	Send_IRMA_Logger_Locked_v20(SrcAddr, Old_LLS);
										break;

							default :	Send_IRMA_Error_v10(SrcAddr, 'L', IRMAErr_InvL7Parameter, 1, &Payload[1]);
										return(IRMAErr_InvL7Parameter);
						}
						return(0);

			case 'C' : 	// External request for data record creation
						if( PayloadLen < IRMA_L_C_PayloadLenMin ) break;

						if( Get_Logger_Locked_State() == 1 )
							reload_timeout_job(Unlock_Logger);

						ErrNo = Log_External_Data_Record(PayloadLen - 1, &Payload[1]);
						for( PayloadIdx = 1; PayloadIdx < 7; PayloadIdx++ )			
							Payload[PayloadIdx] = Payload[PayloadIdx + 1];	

						Send_IRMA_Error_v11(SrcAddr, 'L', 0x20, ErrNo, 7, Payload);
						return(ErrNo);

			default :	Send_IRMA_Error_v10(SrcAddr, 'L', IRMAErr_UnkSubCmd, 1, &SubCmd_Id);
						return(IRMAErr_UnkSubCmd);
		}
	}

	Send_Error_InvL7Length(SrcAddr, 'L', PayloadLen);
	return(IRMAErr_InvMsgLen);
}	// IRMA_L_v20


void IRMA_Message_Logger(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_L_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x20 :	IRMA_L_v20(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'L', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Logger
#endif	// end of "#ifndef NO_LOGGER"


/*---------------------------------------------------------------------------------------- O ---*/
#if defined(SW4) || defined(SW2)
void Build_IRMA_O_v20_Payload(byte SwitchNo)
{
	byte PayloadLen;

	PayloadLen = sizeof(byte) + sizeof(word);
	init_uip_send_buff_writing_10('O', 0x20, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = SwitchNo;

	Put_word_IRMASendBuff(switch_puls_cnt[SwitchNo - 1]);
}	// Build_IRMA_O_v20_Payload


byte IRMA_O_v20_Resp(byte PayloadLen, byte *Payload)
{
	byte SwitchNo, ErrVal;

	if( PayloadLen == 1 )	// Incoming IRMA-O v2.0 request message
	{
	    SwitchNo = Payload[0];
		if( SwitchNo >= 1 && SwitchNo <= A21_SWITCHCNT )
		{
			Build_IRMA_O_v20_Payload(SwitchNo);
			return(0);
		}

		Build_IRMA_Error_v10('O', IRMAErr_InvL7Parameter, 1, &SwitchNo);
		return(IRMAErr_InvL7Parameter);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('O', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_O_v20_Resp


byte IRMA_O_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		Result = IRMA_O_v20_Resp(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'O', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_O_v20


void IRMA_Message_Port(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
	switch( DataVer )
	{
		case 0x10 :	IRMA_O_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x20 :	IRMA_O_v20(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'O', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// IRMA_Message_Port
#endif	// end of "#if defined(SW4) || defined(SW2)"


/*---------------------------------------------------------------------------------------- S ---*/
/*
void Send_IRMA_Status(byte DesAddr)
{
	byte lb[5] = {0, 0, 0, 0, 0};

#ifndef NO_LOGGER
	if( Get_RTC_Sync_Status() == TRUE )
		lb[0] |= 1;
	if( !Logger_Empty() )
		lb[0] |= 2;
	if( Logger_Overflow() )
		lb[0] |= 4;
#endif

	send_uip_10_frame(DesAddr, 'S', 0x10, 5, lb);
}	// Send_IRMA_Status


void Send_Status(void)
{
	Send_IRMA_Status(IRMA_Status_Address);
}	// Send_Status


void Set_Status_Interval(byte Interval, byte DesAddr)
{
	IRMA_Status_Address = DesAddr;

	if( Interval == 0 )
		remove_timer_job(Send_Status);
	else
		add_timer_job(Interval * 1000, Send_Status);
}	// Set_Status_Interval


byte IRMA_S_v10(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	switch( PayloadLen )
	{
		case 0	:	Send_IRMA_Status(SrcAddr);
					return(0);

		case 1	:	Set_Status_Interval(Payload[0], SrcAddr);
					return(0);

		default	:	Send_Error_InvL7Length(SrcAddr, 'S', PayloadLen);
					return(IRMAErr_InvMsgLen);
	}
}	// IRMA_S_v10
*/


void Build_IRMA_S_v20_Payload(void)
{
	byte PayloadLen, SenIdx;
	word *PayloadWordPtr, PayloadWord;

	PayloadLen = IRMA_Sv20_PayloadLenMin + Comm_ConfigData->sensors * sizeof(word);

	init_uip_send_buff_writing_10('S', 0x20, PayloadLen);
/*
typedef struct
{
	struct {
		BitField Out_OverflowB			: 1;			// bit  0
		BitField In_OverflowB			: 1;			// bit  1
		BitField OverrunErrorB			: 1;			// bit  2
		BitField FrameErrorB			: 1;			// bit  3
		BitField ParityErrorB			: 1;			// bit  4
	} ASC0;
	struct {
		BitField Tra_Msg_InvalidB 		: 1;			// bit  0
		BitField Rec_Msg_InvalidB 		: 1;			// bit  1
		BitField Rec_Msg_LostB			: 1;			// bit  2
		BitField Buffer_OverflowB		: 1;			// bit  3
		BitField reserved				: 10;
		BitField CAN_error_warnB		: 1;			// bit 14
		BitField CAN_bus_offB			: 1;			// bit 15
	} CAN1;
	struct {
		BitField Buffer_OverflowB				: 1;	// bit  0
		BitField Door_Contacts_Ever_ChangedB	: 1;	// bit  1
		BitField Door_States_Ever_Set_By_MsgB	: 1;	// bit  2
	} Sensor_Interface;
	struct {
		BitField RTC_ErrorB				: 1;			// bit  0
		BitField NVRAM_ErrorB			: 1;			// bit  1
		BitField RTC_SynchronizedB 		: 1;			// bit  2
	} Optional_Hardware;
	struct {
		BitField DataB					: 1;			// bit  0
		BitField OverflowB				: 1;			// bit  1
	} Logger;
} A21_Status_type;
*/
	PayloadWordPtr = (word *)&A21_Status.ASC0;
	Put_word_IRMASendBuff(*PayloadWordPtr);
	PayloadWordPtr = (word *)&A21_Status.CAN1;
	Put_word_IRMASendBuff(*PayloadWordPtr);
	PayloadWordPtr = (word *)&A21_Status.Sensor_Interface;
	Put_word_IRMASendBuff(*PayloadWordPtr);
	PayloadWordPtr = (word *)&A21_Status.Optional_Hardware;
	Put_word_IRMASendBuff(*PayloadWordPtr);
	PayloadWordPtr = (word *)&A21_Status.Logger;
	Put_word_IRMASendBuff(*PayloadWordPtr);
	IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigData->sensors;
	// LSB bits are arranged in the same way as in function area status STAi of IRMA-C
	// payload. Refer to implementation of functions get_current_fa_status and get_fa_status.
	for( SenIdx = 0; SenIdx < Comm_ConfigData->sensors; SenIdx++ )
	{
		PayloadWord = 0;
		if( DeviceStatus.sensor[SenIdx].initialization_errorB )
			PayloadWord |= BIT0; 
		if( DeviceStatus.sensor[SenIdx].runtime_errorB )
			PayloadWord |= BIT1; 
		if( DeviceStatus.sensor[SenIdx].sabotageB )
			PayloadWord |= BIT2;
		if( DeviceStatus.sensor[SenIdx].firmware_mismatchB )
			PayloadWord |= BIT3; 
		/* Idea:
		   - lower byte, BIT0..BIT7:  resettable status bits
		   - upper byte, BIT8..BIT15: non-resettable status bits
		*/
		Put_word_IRMASendBuff(PayloadWord);
	}
}	// Build_IRMA_S_v20_Payload

	  
#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)
byte IRMA_S_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	Payload;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		if( PayloadLen == 0 )	// Incoming IRMA-S v2.0 request message
		{
			Build_IRMA_S_v20_Payload();
			send_buffered_uip_10_frame(SrcAddr);
			return(0);
		}

		Send_Error_InvL7Length(SrcAddr, 'S', PayloadLen);
		return(IRMAErr_InvMsgLen);
	}

	Send_IRMA_Error_v10(SrcAddr, 'S', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
	return(IRMAErr_SrcAddrNotAllowed);
}	// IRMA_S_v20
#endif	// end of "#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)"
