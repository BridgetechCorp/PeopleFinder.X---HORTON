/*==============================================================================================*
 |       Filename: uip_gdist500.c                                                               |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of Universal IRMA Protocol (UIP) for IRMA 5 gateway.          |
 |                 A21C hardware is used as IRMA 5 gateway.                                     |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifndef GATEWAY
	#error "File uip_gdist500.c included but symbol GATEWAY not defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "..\..\kernel_interface.h"
#include "..\communication.h"
#include "communication_gdist500.h"
#if defined(SW2) || defined(SW4)
	#include "..\switches.h"
#endif
#include "..\uip.h"
#include "uip_gdist500.h"
#include "..\can.h"
#include "can_irma_embedding.h"
#include "can_irma_dist500.h"
#ifdef IBIS_PROTOCOL
	#include "ibisip_gdist500.h"
#endif
#ifdef UJ1708IP
	#include "j1708_gdist500.h"
	#include "uj1708ip_gdist500.h"
#endif
#ifdef J1708_ORBITAL
	#include "j1708_gdist500.h"
	#include "j1708_orbital_gdist500.h"
#endif
#ifndef NO_LOGGER
	#include "..\logger.h"
	#include "logger_gdist500.h"
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
bool to_route_uip_30_msg_to_canB;

bool count_data_statussesB[CAN_FA_LIST_LEN];
counting_data_buffer_type uip_counting_data_buffers[CAN_FA_LIST_LEN];
byte fa_status_bytes[CAN_FA_LIST_LEN];

/*----- Function Prototypes --------------------------------------------------------------------*/
void read_uip_20_frame(void);
void uip_10_frame_process_gateway(int IRMARecBuffEndBackup);
void uip_20_frame_process(int IRMARecBuffEndBackup);

void irma_msg_embedding_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_baudrate_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_count_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_door_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_error_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_flags_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_information_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#ifndef NO_LOGGER
void irma_msg_logger_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#endif
void irma_msg_memory_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_new_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_port_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#ifdef TEST
void irma_msg_query_sensor_signal_test_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#endif
void irma_msg_programm_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_run_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_status_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#ifndef NO_LOGGER
void irma_msg_time_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#endif
void irma_msg_runlevel_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_version_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void irma_msg_execute_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#ifdef TEST
void irma_msg_inst_mode_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/

/*----- Function Tables ------------------------------------------------------------------------*/
const IRMACmdTable_type IRMACmdTable['z' - '@' + 1] = {		/* Do not erase any lines!!! */
	/* @ */	{ NULL },
	/* A */	{ irma_msg_embedding_gateway },
	/* B */	{ irma_msg_baudrate_gateway },
	/* C */	{ irma_msg_count_gateway },
	/* D */	{ irma_msg_door_gateway },
	/* E */	{ irma_msg_error_gateway },
	/* F */	{ irma_msg_flags_gateway },
	/* G */	{ NULL },
	/* H */	{ NULL },
	/* I */	{ irma_msg_information_gateway },
	/* J */	{ NULL },
	/* K */	{ NULL },
#ifndef NO_LOGGER
	/* L */	{ irma_msg_logger_gateway },
#else
	/* L */	{ NULL },
#endif
	/* M */	{ irma_msg_memory_gateway },
	/* N */	{ irma_msg_new_gateway },
	/* O */	{ irma_msg_port_gateway },
	/* P */	{ irma_msg_programm_gateway },
#ifdef TEST
	/* Q */	{ irma_msg_query_sensor_signal_test_gateway },
#else
	/* Q */	{ NULL },
#endif
	/* R */	{ irma_msg_run_gateway },
	/* S */	{ irma_msg_status_gateway },
#ifndef NO_LOGGER
	/* T */	{ irma_msg_time_gateway },
#else
	/* T */	{ NULL },
#endif
	/* U */	{ irma_msg_runlevel_gateway },
	/* V */	{ irma_msg_version_gateway },
	/* W */	{ NULL },
	/* X */	{ irma_msg_execute_gateway },
#ifdef TEST
	/* Y */	{ irma_msg_inst_mode_gateway },
#else
	/* Y */	{ NULL },
#endif
	/* Z */	{ NULL },
	/* [ */	{ NULL },
	/* \ */	{ NULL },
	/* ] */	{ NULL },
	/* ^ */	{ NULL },
	/* _ */	{ NULL },
	/* ` */	{ NULL },
	/* a */	{ NULL },
	/* b */	{ NULL },
	/* c */	{ NULL },
	/* d */	{ NULL },
	/* e */	{ NULL },
	/* f */	{ NULL },
	/* g */	{ NULL },
	/* h */	{ NULL },
	/* i */	{ NULL },
	/* j */	{ NULL },
	/* k */	{ NULL },
	/* l */	{ NULL },
	/* m */	{ NULL },
	/* n */	{ NULL },
	/* o */	{ NULL },
	/* p */	{ NULL },
	/* q */	{ NULL },
	/* r */	{ NULL },
	/* s */	{ NULL },
	/* t */	{ NULL },
	/* u */	{ NULL },
	/* v */	{ NULL },
	/* w */	{ NULL },
	/* x */	{ NULL },
	/* y */	{ NULL },
	/* z */	{ NULL },
};			  


/*----- Implementation of Functions ------------------------------------------------------------*/
void init_uip_counting_data_buffer(byte fa_idx)
{
	count_data_statussesB[fa_idx] = FALSE;
	init_counting_data_buffer(fa_idx, &uip_counting_data_buffers[fa_idx]);
	fa_status_bytes[fa_idx] = 0;
}	// init_uip_counting_data_buffer


void init_uip_counting_data_buffers(void)
{														  
	byte fa_idx;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		init_uip_counting_data_buffer(fa_idx);
}	// init_uip_counting_data_buffers


void Init_IRMA(void)
{
	init_uip_kernel();

	extended_uip_link = read_uip_20_frame;
	uip_10_frame_process = uip_10_frame_process_gateway;

	enable_can_transparent_mode_func = enable_can_transparent_mode_uip;
	to_enable_can_transparent_modeB = FALSE;
	to_route_uip_30_msg_to_canB = FALSE;

	IRMA_Init_Prot_Function = NULL;
	IRMA_Init_RS232 = stRS232_IRMA_Standard;
	IRMA_Init_Prot_Id = PROT_ID_IRMA;

#ifndef NO_LOGGER
	RunLevel_Log_Func = Log_RunLevel_Change;
	Logger_Set_Time_Func = Logger_Set_Time;
#endif

	init_uip_counting_data_buffers();

	obc_interface_data_update_func = NULL;
}	// Init_IRMA


void read_uip_20_frame(void)
{
	byte rec_byte, uip_ver, cs;
	word msg_len;
	int IRMARecBuffEndBackup;

	while( get_char_asc0_uip() )
	{
		rec_byte = IRMARecBuff[IRMARecBuffPos];

		switch( IRMARecBuffPos )
		{
			case  1 :
			case  2 :
			case  3 :
			case  4 :
			case  5 :	abort_uip_frame_reading();
						break;

			case  6 :	uip_ver = rec_byte & 0xF0;
						if( uip_ver != 0x20 )
							abort_uip_frame_reading();
						uip_service_lev = rec_byte & 0x0F;
						IRMARecBuffEnd = UIP_20_FRAME_W1;
						break;

			case  7 :	IrmaDesAddr = rec_byte;
						break;

			case  8 :	IrmaSrcAddr = rec_byte;
						break;

			// message length, LSB
			case  9 :	break;

			// message length, MSB
			case 10 :	msg_len = (IRMARecBuff[UIP_20_FRAME_DL + 1] << 8) + IRMARecBuff[UIP_20_FRAME_DL];
						if( (msg_len & 0x8000 ) == 0x8000 )
							uip_retry_flagB = TRUE;
						else
							uip_retry_flagB = FALSE;
						msg_len &= 0x7FFF;
						if( msg_len > UIP_20_MSG_LEN_MAX )
							abort_uip_frame_reading();
						// Calculate receive buffer index where checksum to be received will be stored.
						IRMARecBuffEnd = msg_len + UIP_20_OVERHEAD - 1;
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

			IRMARecBuffEndBackup = IRMARecBuffEnd;
			reset_uip_frame_reading();
			uip_20_frame_process(IRMARecBuffEndBackup);
			return;
		}	// end of "if( IRMARecBuffPos >= IRMARecBuffEnd )"
	}	// end of "while( get_char_asc0_uip() )"
}	// read_uip_20_frame


void uip_10_frame_process_gateway(int IRMARecBuffEndBackup)
{
	byte rec_byte;

	if(	IrmaDesAddr != Comm_ConfigData->irma.address && IrmaDesAddr != IRMAAddrUniversalDest )
		return;

	rec_byte = IRMARecBuff[UIP_10_FRAME_DK];
	if( rec_byte < '@' || rec_byte > 'z' )
	{
		Send_IRMA_Error_v10(IrmaSrcAddr, rec_byte, IRMAErr_UnkTypeIdent, 0, NULL);
		return;
	}

	rec_byte -= '@';
	if( IRMACmdTable[rec_byte].call_function == NULL )
	{
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

	is_uip_20_sendB = FALSE;

	IRMACmdTable[rec_byte].call_function(0x10,
										 uip_service_lev,
										 uip_retry_flagB,
										 IrmaSrcAddr,
										 IRMARecBuff[UIP_10_FRAME_VER],
										 '\0',
										 (word)IRMARecBuff[UIP_10_FRAME_DL] - UIP_10_MESSAGE_DATAOFFSET,
										 &IRMARecBuff[UIP_10_FRAME_W1]);
}	// uip_10_frame_process_gateway


void uip_20_frame_process(int IRMARecBuffEndBackup)
{
	byte rec_byte;
	word payload_len;

	if(	IrmaDesAddr != Comm_ConfigData->irma.address && IrmaDesAddr != IRMAAddrUniversalDest )
		return;

	is_uip_20_sendB = TRUE;

	rec_byte = IRMARecBuff[UIP_20_FRAME_DK];
	if( rec_byte < '@' || rec_byte > 'z' )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
	
		Send_IRMA_Error_v10(IrmaSrcAddr, rec_byte, IRMAErr_UnkTypeIdent, 0, NULL);
		return;
	}

	rec_byte -= '@';
	if( IRMACmdTable[rec_byte].call_function == NULL )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
	
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

	payload_len = ((word)IRMARecBuff[UIP_20_FRAME_DL + 1] << 8) + (word)IRMARecBuff[UIP_20_FRAME_DL];
	payload_len -= UIP_20_MESSAGE_DATAOFFSET;

	IRMACmdTable[rec_byte].call_function(0x20,
										 uip_service_lev,
										 uip_retry_flagB,
										 IrmaSrcAddr,
										 IRMARecBuff[UIP_20_FRAME_VER],
										 (char)IRMARecBuff[UIP_20_FRAME_CMD],
										 payload_len,
										 &IRMARecBuff[UIP_20_FRAME_W1]);
}	// uip_20_frame_process


/*---------------------------------------------------------------------------------------- A ---*/
void write_A_v60_resp_msg_to_uip_send_buff(void)
{
	init_uip_send_buff_writing_20(uip_service_lev_send, 'A', 0x60, uip_sub_cmd_send, 0);
}	// write_A_v60_resp_msg_to_uip_send_buff


bool enable_can_transparent_mode_uip(void)
{
	bool can_transparent_mode_just_enabledB;

	can_transparent_mode_just_enabledB = enable_can_transparent_mode();

	if( to_route_uip_30_msg_to_canB )
	{
		route_uip_30_msg_to_can(&IRMARecBuff[UIP_20_FRAME_W1]);
		to_route_uip_30_msg_to_canB = FALSE;
	}

	write_A_v60_resp_msg_to_uip_send_buff();
	if( can_transparent_mode_just_enabledB )
		send_buffered_uip_20_frame(IrmaASrcAddr);

	return(can_transparent_mode_just_enabledB);
}	// enable_can_transparent_mode_uip


bool disable_can_transparent_mode_uip(void)
{
	bool can_transparent_mode_disabledB;

	can_transparent_mode_disabledB = disable_can_transparent_mode_dist500();

	write_A_v60_resp_msg_to_uip_send_buff();
	if( can_transparent_mode_disabledB )
		send_buffered_uip_20_frame(IrmaASrcAddr);

	return(can_transparent_mode_disabledB);
}	// disable_can_transparent_mode_uip


bool prepare_can_transparent_mode_enabling(void)
{
	bool can_transparent_mode_enable_flag_setB;

	// Store source address of IRMA "A" frame for transmission of response.
	IrmaASrcAddr = IrmaSrcAddr;

	can_transparent_mode_enable_flag_setB = set_can_transparent_mode_enable_flag();
	if( !can_transparent_mode_enable_flag_setB )
		enable_can_transparent_mode_uip();

	return(can_transparent_mode_enable_flag_setB);
}	// prepare_can_transparent_mode_enabling


byte IRMA_A_v60_d_e_Resp(bool to_enableB, word PayloadLen, byte *Payload, bool *to_respond_immediatelyB)
{
	byte ErrVal;

	Payload;
	if( PayloadLen > 0 )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		ErrVal = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('A', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	if( to_enableB )
	{
		to_route_uip_30_msg_to_canB = FALSE;
		*to_respond_immediatelyB = !prepare_can_transparent_mode_enabling();
	}
	else
	{
		disable_can_transparent_mode_uip();
		*to_respond_immediatelyB = TRUE;
	}

	return(0);
}	// IRMA_A_v60_d_e_Resp


byte IRMA_A_v60_s_Resp(word PayloadLen, byte *Payload, bool *to_respond_immediatelyB)
{
	byte ErrVal;

	Payload;
	if( PayloadLen < IRMA_Av60_s_PayloadLenMin )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		ErrVal = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('A', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	to_route_uip_30_msg_to_canB = TRUE;
	*to_respond_immediatelyB = !prepare_can_transparent_mode_enabling();

	return(0);
}	// IRMA_A_v60_s_Resp


byte IRMA_A_v60(byte SrcAddr, char sub_cmd, byte PayloadLen, byte *Payload)
{
    byte Result;
	bool to_respond_immediatelyB = TRUE;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		switch( sub_cmd )
		{
			case 'd' :
			case 'e' :	Result = IRMA_A_v60_d_e_Resp(sub_cmd == 'e', PayloadLen, Payload, &to_respond_immediatelyB);
						break;

			case 's' :	Result = IRMA_A_v60_s_Resp(PayloadLen, Payload, &to_respond_immediatelyB);
						break;

			default	 :	uip_service_lev_send = 0;
						uip_sub_cmd_send = 0;
		
						Build_IRMA_Error_v10('A', IRMAErr_UnkSubCmd, 1, (byte *)&sub_cmd);
						Result = IRMAErr_UnkSubCmd;
		}	// end of "switch( sub_cmd )"

		if( to_respond_immediatelyB )
			// Send IRMA error message or response message, if CAN transparent mode was already active.
			send_buffered_uip_20_frame(SrcAddr);
		return(Result);
	}
	else
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Send_IRMA_Error_v10(SrcAddr, 'A', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_A_v60


void irma_msg_embedding_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	retry_flagB;

	switch( DataVer )
	{
		case 0x60 :	uip_service_lev_send = service_lev;
					uip_sub_cmd_send = (char)((byte)sub_cmd - 0x20);
		
					IRMA_A_v60(SrcAddr, sub_cmd, PayloadLen, Payload);
					break;

		default	:	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					Send_IRMA_Error_v10(SrcAddr, 'A', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_embedding_gateway


/*---------------------------------------------------------------------------------------- B ---*/
void irma_msg_baudrate_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	byte ErrNo;

	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

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
}	// irma_msg_baudrate_gateway


/*---------------------------------------------------------------------------------------- C ---*/
bool query_fa_idx_valid_gatewayB(byte payload_len, byte *payload, byte *fa_idx)
{
	byte fa_addr;

	*fa_idx = 0xFF;
	if( payload_len == 1 )
	{
		fa_addr = payload[0];
		*fa_idx = get_idx_of_function_area_in_obc_interface_data(fa_addr);
		if( *fa_idx == 0xFF && fa_addr != 0xFF )
			return(FALSE);
	}
	return(TRUE);
}	// query_fa_idx_valid_gatewayB


void Write_C_2x_Data_To_IRMASendBuff(byte fa_idx, byte hc_idx)
{
	counting_data_buffer_type counting_data_buffer;
	word in = 0, out = 0;

	memset(&counting_data_buffer, 0, sizeof(counting_data_buffer));
	get_total_counter_states(fa_idx, &counting_data_buffer);

	if( hc_idx == 0xFF )
		get_counting_data_category_sums(counting_data_buffer, &in, &out);

	IRMASendBuff[IRMASendBuffWritePos++] = (byte)obc_interface_data.function_area_list[fa_idx].address;
	IRMASendBuff[IRMASendBuffWritePos++] = get_current_fa_status_byte_gateway(fa_idx);
	IRMASendBuff[IRMASendBuffWritePos++] = in;
	IRMASendBuff[IRMASendBuffWritePos++] = in >> 8;
	IRMASendBuff[IRMASendBuffWritePos++] = out;
	IRMASendBuff[IRMASendBuffWritePos++] = out >> 8;
}	// Write_C_2x_Data_To_IRMASendBuff


void Build_IRMA_C_v20_Payload(byte FAIdx)
{
	byte FACnt, PayloadLen, Idx;

	if( FAIdx != 0xFF )
		FACnt = 1;
	else
	{
		FACnt = obc_interface_data.function_area_cnt;
		FAIdx = 0;
	}

	PayloadLen = FACnt * C_20_TYPE_SIZE;
	init_uip_send_buff_writing_10('C', 0x20, PayloadLen);

	for( Idx = 0; Idx < FACnt; Idx++, FAIdx++ )
		Write_C_2x_Data_To_IRMASendBuff(FAIdx, 0xFF);
}	// Build_IRMA_C_v20_Payload


byte IRMA_C_v20_Resp(byte PayloadLen, byte *Payload)
{
	byte FAIdx, ErrVal;

	if( PayloadLen < 2 )		// Query of Counting Results
	{
		if( query_fa_idx_valid_gatewayB(PayloadLen, Payload, &FAIdx) )
		{
			Build_IRMA_C_v20_Payload(FAIdx);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('C', IRMAErr_InvFAAddr, 1, Payload);
			return(IRMAErr_InvFAAddr);
		}
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('C', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_C_v20_Resp


byte IRMA_C_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_C_v20_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_C_v20


#ifdef NO_LOGGER
void Write_C_3x_Data_To_IRMASendBuff(byte fa_idx, byte hc_idx)
{
	counting_data_buffer_type counting_data_buffer;
	byte fa_status_byte;
	word in = 0, out = 0;

	count_data_statussesB[fa_idx] = FALSE;
	counting_data_buffer          = uip_counting_data_buffers[fa_idx];
	fa_status_byte                = fa_status_bytes[fa_idx];

    if( hc_idx == 0xFF )
		get_counting_data_category_sums(counting_data_buffer, &in, &out);

	IRMASendBuff[IRMASendBuffWritePos++] = (byte)obc_interface_data.function_area_list[fa_idx].address;
	IRMASendBuff[IRMASendBuffWritePos++] = fa_status_byte;
	IRMASendBuff[IRMASendBuffWritePos++] = in;
	IRMASendBuff[IRMASendBuffWritePos++] = in >> 8;
	IRMASendBuff[IRMASendBuffWritePos++] = out;
	IRMASendBuff[IRMASendBuffWritePos++] = out >> 8;
}	// Write_C_3x_Data_To_IRMASendBuff


void Build_IRMA_C_v30_Payload(byte FAIdx)
{
	byte FACnt, PayloadLen, Idx;

	if( FAIdx != 0xFF )
		FACnt = 1;
	else
	{
		FACnt = obc_interface_data.function_area_cnt;
		FAIdx = 0;
	}

	PayloadLen = FACnt * C_20_TYPE_SIZE;
	init_uip_send_buff_writing_10('C', 0x30, PayloadLen);

	for( Idx = 0; Idx < FACnt; Idx++, FAIdx++ )
		Write_C_3x_Data_To_IRMASendBuff(FAIdx, 0xFF);
}	// Build_IRMA_C_v30_Payload


byte IRMA_C_v30_Resp(byte PayloadLen, byte *Payload)
{
	byte FAIdx, ErrVal;

	if( PayloadLen < 2 )		// Query of Counting Results
	{
		if( query_fa_idx_valid_gatewayB(PayloadLen, Payload, &FAIdx) )
		{
			Build_IRMA_C_v30_Payload(FAIdx);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('C', IRMAErr_InvFAAddr, 1, Payload);
			return(IRMAErr_InvFAAddr);
		}
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('C', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_C_v30_Resp


// Query of buffered counting results.
byte IRMA_C_v30(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_C_v30_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_C_v30
#endif	// end of "#ifdef NO_LOGGER"


byte IRMA_C_v50(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte FACnt, FAIdx, PayloadPos;
	word Value_in, Value_out;
	byte counting_data_category_cnt, counting_data_category_idx;
	counting_data_buffer_type counting_data_buffer;
	bool counting_finishedB;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		if( PayloadLen >= C_50_TYPE_SIZE && PayloadLen % C_50_TYPE_SIZE == 0 )
		{
			FACnt = obc_interface_data.function_area_cnt;
			if( FACnt > 0 && PayloadLen == FACnt * C_50_TYPE_SIZE )
			{
				for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
					if( Payload[FAIdx * C_50_TYPE_SIZE] != (byte)obc_interface_data.function_area_list[FAIdx].address )
					{
						// At least one parameter FAi doesn't match the corresponding function area
						// address configured in this IRMA Analyzer.
						Send_IRMA_Error_v10(SrcAddr, 'C', IRMAErr_SetNotAllowed, PayloadLen, Payload);
						return(IRMAErr_SetNotAllowed);
					}

				memset(&counting_data_buffer, 0, sizeof(counting_data_buffer));

				PayloadPos = 0;
				for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
				{
					Value_in  = ((word)Payload[PayloadPos + 3] << 8) + (word)Payload[PayloadPos + 2];
					Value_out = ((word)Payload[PayloadPos + 5] << 8) + (word)Payload[PayloadPos + 4];
					counting_data_category_cnt = obc_interface_data.function_area_list[FAIdx].counting_data_category_cnt;
					counting_data_buffer.counting_data[0].category_id = obc_interface_data.function_area_list[FAIdx].counting_data_category_param_array[0].category_id;
					counting_data_buffer.counting_data[0].in          = Value_in;
					counting_data_buffer.counting_data[0].out         = Value_out;
					for( counting_data_category_idx = 1; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
					{
						counting_data_buffer.counting_data[counting_data_category_idx].category_id = obc_interface_data.function_area_list[FAIdx].counting_data_category_param_array[counting_data_category_idx].category_id;
						counting_data_buffer.counting_data[counting_data_category_idx].in          = 0;
						counting_data_buffer.counting_data[counting_data_category_idx].out         = 0;
					}
					counting_data_buffer.counting_data_category_cnt = counting_data_category_cnt;

					counting_finishedB = (bool)Payload[PayloadPos + 1];

            		set_current_counter_state_by_msg_gateway(FAIdx, &counting_data_buffer, counting_finishedB);

					PayloadPos += C_50_TYPE_SIZE;
				}

				// Store source address for transmission of response.
				IrmaASrcAddr = SrcAddr;
				// Response transmission triggered by function 
				// send_irma_C_v50_current_counter_state_set_resp_gateway.

				return(0);
			}

			// Number of function areas included in payload doesn't match with the number
			// of function areas configured in this IRMA Analyzer.
			Send_IRMA_Error_v10(SrcAddr, 'C', IRMAErr_SetNotAllowed, PayloadLen, Payload);
			return(IRMAErr_SetNotAllowed);
		}

		Send_Error_InvL7Length(SrcAddr, 'C', PayloadLen);
		return(IRMAErr_InvMsgLen);
	}

	Send_IRMA_Error_v10(SrcAddr, 'C', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
	return(IRMAErr_SrcAddrNotAllowed);
}	// IRMA_C_v50


void send_irma_C_v50_current_counter_state_set_resp_gateway(void)
{
	Build_IRMA_C_v20_Payload(0xFF);

	send_buffered_uip_10_frame(IrmaASrcAddr);
}	// send_irma_C_v50_current_counter_state_set_resp_gateway


void irma_msg_count_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x20 :	IRMA_C_v20(SrcAddr, PayloadLen, Payload);
					break;

#ifdef NO_LOGGER
		case 0x30 :	IRMA_C_v30(SrcAddr, PayloadLen, Payload);
					break;
#endif	// end of "#ifdef NO_LOGGER"

		case 0x50 :	IRMA_C_v50(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'C', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_count_gateway


/*---------------------------------------------------------------------------------------- D ---*/
void get_D_10_data_gateway(byte fa_idx, D_10_type *D_10_data)
{
	D_10_data->FA_Addr = (byte)obc_interface_data.function_area_list[fa_idx].address;
	
	if( obc_interface_data.function_area_list[fa_idx].door_closedB )
	{
		D_10_data->Left  = 0;
		D_10_data->Right = 0;
	}
	else
	{
		D_10_data->Left  = 100;
		D_10_data->Right = 100;
	}
}	// get_D_10_data_gateway


void write_D_10_data_to_irma_send_buff_gateway(byte fa_idx)
{
	D_10_type buffer_D_10;

	get_D_10_data_gateway(fa_idx, &buffer_D_10);
	memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)&buffer_D_10, D_10_TYPE_SIZE);		
	IRMASendBuffWritePos += D_10_TYPE_SIZE;
}	// write_D_10_data_to_irma_send_buff_gateway


void write_all_D_10_data_to_irma_send_buff_gateway(void)
{
	byte fa_cnt, fa_idx;

	fa_cnt = obc_interface_data.function_area_cnt;
	for( fa_idx = 0; fa_idx < fa_cnt; fa_idx++ )
		write_D_10_data_to_irma_send_buff_gateway(fa_idx);
}	// write_all_D_10_data_to_irma_send_buff_gateway


void build_irma_D_v10_payload_gateway(byte FAIdx)
{
	byte fa_cnt;

	if( FAIdx != 0xFF )
	{
		init_uip_send_buff_writing_10('D', 0x10, 3);
		write_D_10_data_to_irma_send_buff_gateway(FAIdx);
	}
	else
	{
		fa_cnt = obc_interface_data.function_area_cnt;
		init_uip_send_buff_writing_10('D', 0x10, 3 * fa_cnt);
		for( FAIdx = 0; FAIdx < fa_cnt; FAIdx++ )
			write_D_10_data_to_irma_send_buff_gateway(FAIdx);
	}
}	// build_irma_D_v10_payload_gateway


byte irma_D_v10_door_state_setting_gateway(byte PayloadLen, byte *Payload)
{
	byte FACnt, FAIdx, PayIdx;
	bool Setting_PermittedB;

	FACnt = obc_interface_data.function_area_cnt;
    if( PayloadLen != D_10_TYPE_SIZE * FACnt )
	{
		// Number of function areas included in payload doesn't match with the number
		// of function areas configured in this IRMA Analyzer.
		Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
		return(IRMAErr_SetNotAllowed);
	}

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
		if( Payload[D_10_TYPE_SIZE * FAIdx] != (byte)obc_interface_data.function_area_list[FAIdx].address )
		{
			// At least one parameter FAi doesn't match the corresponding function area
			// address of OBC interface data.
			Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
			return(IRMAErr_SetNotAllowed);
		}

	for( PayIdx = 0, FAIdx = 0; PayIdx < PayloadLen; PayIdx += D_10_TYPE_SIZE, FAIdx++ )
	{
		Setting_PermittedB = FALSE;
		if( Payload[PayIdx + 1] == 0 && Payload[PayIdx + 2] == 0 )
            Setting_PermittedB = set_door_status_by_msg_gateway(FAIdx, TRUE);
		else if( Payload[PayIdx + 1] == 100 && Payload[PayIdx + 2] == 100 )
            Setting_PermittedB = set_door_status_by_msg_gateway(FAIdx, FALSE);

		if( !Setting_PermittedB )
		{
			Build_IRMA_Error_v10('D', IRMAErr_SetNotAllowed, PayloadLen, Payload);
			return(IRMAErr_SetNotAllowed);
		}
	}

	return(0);
}	// irma_D_v10_door_state_setting_gateway


void get_old_D_10_data_set_resp_gateway(byte fa_idx, D_10_type *D_10_data)
{
	D_10_data->FA_Addr = (byte)obc_interface_data.function_area_list[fa_idx].address;
	
	if( obc_interface_data.function_area_list[fa_idx].door_closed_oldB )
	{
		D_10_data->Left  = 0;
		D_10_data->Right = 0;
	}
	else
	{
		D_10_data->Left  = 100;
		D_10_data->Right = 100;
	}
}	// get_old_D_10_data_set_resp_gateway


void get_D_10_data_set_resp_gateway(byte fa_idx, D_10_type *D_10_data)
{
	D_10_data->FA_Addr = (byte)obc_interface_data.function_area_list[fa_idx].address;
	
	if( obc_interface_data.function_area_list[fa_idx].door_closedB )
	{
		D_10_data->Left  = 0;
		D_10_data->Right = 0;
	}
	else
	{
		D_10_data->Left  = 100;
		D_10_data->Right = 100;
	}
}	// get_D_10_data_set_resp_gateway


void send_irma_D_v10_door_state_set_resp_gateway(void)
{
	byte fa_idx;
	D_10_type buffer_D_10;

	init_uip_send_buff_writing_10('D', 0x10, 2 * D_10_TYPE_SIZE * obc_interface_data.function_area_cnt);

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		// old door states
		get_old_D_10_data_set_resp_gateway(fa_idx, &buffer_D_10);
		memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)&buffer_D_10, D_10_TYPE_SIZE);		
		IRMASendBuffWritePos += D_10_TYPE_SIZE;
	}
	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		// current door states
		get_D_10_data_set_resp_gateway(fa_idx, &buffer_D_10);
		memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)&buffer_D_10, D_10_TYPE_SIZE);		
		IRMASendBuffWritePos += D_10_TYPE_SIZE;
	}

	send_buffered_uip_10_frame(IrmaASrcAddr);
}	// send_irma_D_v10_door_state_set_resp_gateway


byte irma_D_v10_resp_gateway(byte SrcAddr, byte PayloadLen, byte *Payload, bool *to_send_resp_nowB)
{
	byte FAIdx, Result, ErrVal;

	*to_send_resp_nowB = TRUE;

	if( PayloadLen < 2 )					// query of door status
	{
		if( query_fa_idx_valid_gatewayB(PayloadLen, Payload, &FAIdx) )
		{
			build_irma_D_v10_payload_gateway(FAIdx);
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
			Result = irma_D_v10_door_state_setting_gateway(PayloadLen, Payload);
			*to_send_resp_nowB = Result > 0;
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
}	// irma_D_v10_resp_gateway


byte irma_D_v10_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;
	bool to_send_resp_nowB;

	Result = irma_D_v10_resp_gateway(SrcAddr, PayloadLen, Payload, &to_send_resp_nowB);
	if( to_send_resp_nowB )
		send_buffered_uip_10_frame(SrcAddr);
	else
		// Store source address for transmission of response.
		IrmaASrcAddr = SrcAddr;
	return(Result);
}	// irma_D_v10_gateway


byte irma_D_v12_resp_gateway(byte PayloadLen, byte *Payload)
{
	byte FACnt, FAIdx, ErrVal;

	Payload;
	if( PayloadLen == 0 )			// query of door event counters
	{
	    FACnt = obc_interface_data.function_area_cnt;
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
}	// irma_D_v12_resp_gateway


byte irma_D_v12_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = irma_D_v12_resp_gateway(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// irma_D_v12_gateway


#if defined(NO_LOGGER)
void Write_D_20_Data_To_IRMASendBuff(byte fa_idx)
{
	write_D_10_data_to_irma_send_buff_gateway(fa_idx);

	if( !count_data_statussesB[fa_idx] )
		count_data_statussesB[fa_idx] = query_buffered_counting_result_and_fa_status_buffer(fa_idx,
		                                                                                    &uip_counting_data_buffers[fa_idx],
		                                                                                    &fa_status_bytes[fa_idx]);

	IRMASendBuff[IRMASendBuffWritePos++] = (byte)count_data_statussesB[fa_idx];
}	// Write_D_20_Data_To_IRMASendBuff


void Build_IRMA_D_v20_Payload(byte FAIdx)
{
	byte FACnt;

	if( FAIdx != 0xFF )
	{
		init_uip_send_buff_writing_10('D', 0x20, 4);
		Write_D_20_Data_To_IRMASendBuff(FAIdx);
	}
	else
	{
		FACnt = obc_interface_data.function_area_cnt;
		init_uip_send_buff_writing_10('D', 0x20, 4 * FACnt);
		for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
			Write_D_20_Data_To_IRMASendBuff(FAIdx);
	}
}	// Build_IRMA_D_v20_Payload


byte IRMA_D_v20_Resp(byte PayloadLen, byte *Payload)
{
	byte FAIdx, ErrVal;

	if( PayloadLen < 2 )		// Query of Door State
	{
		if( query_fa_idx_valid_gatewayB(PayloadLen, Payload, &FAIdx) )
		{
			Build_IRMA_D_v20_Payload(FAIdx);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('D', IRMAErr_InvFAAddr, 1, Payload);
			return(IRMAErr_InvFAAddr);
		}
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('D', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// IRMA_D_v20_Resp


// Initialization of query of buffered counting results.
byte IRMA_D_v20(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = IRMA_D_v20_Resp(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// IRMA_D_v20
#endif	// end of "#if defined(NO_LOGGER)"


void irma_msg_door_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	irma_D_v10_gateway(SrcAddr, PayloadLen, Payload);
					break;

		case 0x12 :	irma_D_v12_gateway(SrcAddr, PayloadLen, Payload);
					break;

#if defined(NO_LOGGER)
		case 0x20 :	IRMA_D_v20(SrcAddr, PayloadLen, Payload);
					break;
#endif	// end of "#if defined(NO_LOGGER)"

		default   :	Send_IRMA_Error_v10(SrcAddr, 'D', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_door_gateway


/*---------------------------------------------------------------------------------------- E ---*/
void irma_msg_error_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_E_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x11 :	IRMA_E_v11(SrcAddr, PayloadLen, Payload);
					break;
	}
}	// irma_msg_error_gateway


/*---------------------------------------------------------------------------------------- F ---*/
void irma_msg_flags_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_F_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'F', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_flags_gateway


/*---------------------------------------------------------------------------------------- I ---*/
void build_irma_I_v10_v11_payload_gateway(byte DataVer)
{
	byte FACnt, FAIdx, FA_Application, PayloadLen, ProdIdLen;

    FACnt = obc_interface_data.function_area_cnt;
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
	memcpy((void *)&IRMASendBuff[IRMASendBuffWritePos], (void *)Comm_ConfigSystem->product_id, ProdIdLen - 1);
	IRMASendBuffWritePos += (ProdIdLen - 1);
	IRMASendBuff[IRMASendBuffWritePos++] = 0;
	IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigSystem->device_id.year;
	IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigSystem->device_id.serial_number;
	IRMASendBuff[IRMASendBuffWritePos++] = Comm_ConfigSystem->device_id.serial_number >> 8;
	Put_dword_IRMASendBuff(ADDR_CONFIG_DATA);
	IRMASendBuff[IRMASendBuffWritePos++] = FACnt;

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
	{
	 	IRMASendBuff[IRMASendBuffWritePos++] = (byte)obc_interface_data.function_area_list[FAIdx].address;

		FA_Application = 'C';
		IRMASendBuff[IRMASendBuffWritePos++] = FA_Application;
	}
}	// build_irma_I_v10_v11_payload_gateway

	  
byte irma_I_v10_v11_resp_gateway(byte DataVer, byte PayloadLen, byte *Payload)
{
	byte ErrVal;

	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-I v1.0 request message
	{
		build_irma_I_v10_v11_payload_gateway(DataVer);
		return(0);
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// irma_I_v10_v11_resp_gateway


byte irma_I_v10_v11_gateway(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = irma_I_v10_v11_resp_gateway(DataVer, PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// irma_I_v10_v11_gateway


void write_I_13_data_to_irma_send_buff_gateway(byte sidx)
{
	sensor_data_obc_type sensor_data_obc;
	byte char_idx;
	byte PayloadByte;

	sensor_data_obc = obc_interface_data.sensor_list[sidx];

	for( char_idx = 0; char_idx < IRMA_SEN_DEV_NO_LEN; char_idx++ )
	{
		if( sensor_data_obc.static_data_validB )
		{
		    PayloadByte = (byte)sensor_data_obc.dev_number[char_idx];
			if( PayloadByte <= 0x20 )
			    PayloadByte = 0;
			if( char_idx == IRMA_SEN_DEV_NO_LEN - 1 )
			    PayloadByte = 0;
		}
		else
		    PayloadByte = 0;
		IRMASendBuff[IRMASendBuffWritePos++] = PayloadByte;
	}
}	// write_I_13_data_to_irma_send_buff_gateway


void write_I_14_data_to_irma_send_buff_gateway(byte sidx)
{
	byte function_area_1_list_idx;

	function_area_1_list_idx = obc_interface_data.sensor_list[sidx].fa_list_idx;

	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// operation mode

	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// reserved for future use
	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// reserved for future use
	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// reserved for future use
	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// reserved for future use

	IRMASendBuff[IRMASendBuffWritePos++] = function_area_1_list_idx + 1;
	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// second function area
	IRMASendBuff[IRMASendBuffWritePos++] = 0;	// third function area
}	// write_I_14_data_to_irma_send_buff_gateway


void write_I_15_data_to_irma_send_buff_gateway(byte sidx)
{
	sensor_data_obc_type sensor_data_obc;
	byte char_idx;
	byte PayloadByte;

	sensor_data_obc = obc_interface_data.sensor_list[sidx];

	for( char_idx = 0; char_idx < CAN_IRMA_SEN_TYPE_NAME_LEN; char_idx++ )
	{
		if( sensor_data_obc.static_data_validB )
		{
		    PayloadByte = (byte)sensor_data_obc.prod_name[char_idx];
			if( PayloadByte <= 0x20 )
			    PayloadByte = 0;
			if( char_idx == CAN_IRMA_SEN_TYPE_NAME_LEN - 1 )
			    PayloadByte = 0;
		}
		else
		    PayloadByte = 0;
		IRMASendBuff[IRMASendBuffWritePos++] = PayloadByte;
	}
}	// write_I_15_data_to_irma_send_buff_gateway


void write_I_16_data_to_irma_send_buff_gateway(byte sidx)
{
	sensor_data_obc_type sensor_data_obc;
	byte char_idx;
	byte PayloadByte;

	sensor_data_obc = obc_interface_data.sensor_list[sidx];

	for( char_idx = 0; char_idx < CAN_IRMA_SEN_FIRMWARE_NAME_LEN; char_idx++ )
	{
		if( sensor_data_obc.static_data_validB )
		{
		    PayloadByte = (byte)sensor_data_obc.dspf_version[char_idx];
			if( PayloadByte <= 0x20 )
			    PayloadByte = 0;
			if( char_idx == CAN_IRMA_SEN_FIRMWARE_NAME_LEN - 1 )
			    PayloadByte = 0;
		}
		else
		    PayloadByte = 0;
		IRMASendBuff[IRMASendBuffWritePos++] = PayloadByte;
	}
}	// write_I_16_data_to_irma_send_buff_gateway


void build_irma_I_v18_payload_gateway(byte SenNo)
{
	byte PayloadLen;
	byte sidx;

	PayloadLen = 1 + CAN_IRMA_SEN_TYPE_NAME_LEN + IRMA_SEN_DEV_NO_LEN +
	             CAN_IRMA_SEN_FIRMWARE_NAME_LEN + 8;

	init_uip_send_buff_writing_10('I', 0x18, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = SenNo;

	sidx = SenNo - 1;
	write_I_15_data_to_irma_send_buff_gateway(sidx);
	write_I_13_data_to_irma_send_buff_gateway(sidx);
	write_I_16_data_to_irma_send_buff_gateway(sidx);
	write_I_14_data_to_irma_send_buff_gateway(sidx);
}	// build_irma_I_v18_payload_gateway


byte irma_I_v18_resp_gateway(byte PayloadLen, byte *Payload)
{
	byte SenNo, ErrVal;

	if( PayloadLen == 1 )	// Incoming IRMA-I v1.8 request message
	{
		SenNo = (byte)Payload[0];
		if( SenNo > 0 && SenNo <= obc_interface_data.sensor_cnt )
		{
			build_irma_I_v18_payload_gateway(SenNo);
			return(0);
		}
		else
		{
			Build_IRMA_Error_v10('I', IRMAErr_InvSenNo, 1, Payload);
			return(IRMAErr_InvSenNo);
		}
	}

	ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
	Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
	return(IRMAErr_InvMsgLen);
}	// irma_I_v18_resp_gateway


byte irma_I_v18_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	Result = irma_I_v18_resp_gateway(PayloadLen, Payload);
	send_buffered_uip_10_frame(SrcAddr);
	return(Result);
}	// irma_I_v18_gateway


#ifdef J1708_PROTOCOL
void add_irma_I_fa_info_gateway(const byte FACnt)
{
	byte FAIdx, FA_Application;

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
	{
	 	IRMASendBuff[IRMASendBuffWritePos++] = (byte)obc_interface_data.function_area_list[FAIdx].address;

		FA_Application = 'C';
		IRMASendBuff[IRMASendBuffWritePos++] = FA_Application;
	}
}	// add_irma_I_fa_info_gateway


byte irma_I_v20_gateway(byte PayloadLen, byte *Payload)
{
	byte FACnt;
	char A21ClassIdStr[A21ClassIdStrLen + 1];
	byte ErrVal;

	Payload;
	if( PayloadLen == 0 )
	{
	    FACnt = obc_interface_data.function_area_cnt;
		if( FACnt > MaxJ1708A21FACnt )
			FACnt = MaxJ1708A21FACnt;

		PayloadLen = 7 + FACnt * 2;
		init_uip_send_buff_writing_10('I', 0x20, PayloadLen);
		ExtractA21ClassIdStr(A21ClassIdStr);
		memcpy(&IRMASendBuff[IRMASendBuffWritePos], A21ClassIdStr, A21ClassIdStrLen);
		IRMASendBuffWritePos += A21ClassIdStrLen;
		memcpy(&IRMASendBuff[IRMASendBuffWritePos], CustomProtIdStr, CustomProtIdStrLen);
		IRMASendBuffWritePos += CustomProtIdStrLen;
		IRMASendBuff[IRMASendBuffWritePos++] = obc_interface_data.sensor_cnt;
		IRMASendBuff[IRMASendBuffWritePos++] = FACnt;
		add_irma_I_fa_info_gateway(FACnt);
		return(0);
	}
	else
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('I', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}
}	// irma_I_v20_gateway
#endif	// end of "#ifdef J1708_PROTOCOL"


void irma_msg_information_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :
		case 0x11 :	irma_I_v10_v11_gateway(SrcAddr, DataVer, PayloadLen, Payload);
					break;

		case 0x17 :	IRMA_I_v17(SrcAddr, PayloadLen, Payload);
					break;

		case 0x18 :	irma_I_v18_gateway(SrcAddr, PayloadLen, Payload);
					break;

		case 0x40 :	IRMA_I_v40(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'I', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_information_gateway


/*---------------------------------------------------------------------------------------- L ---*/
#ifndef NO_LOGGER
void irma_msg_logger_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_L_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x20 :	IRMA_L_v20(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'L', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_logger_gateway
#endif	// end of "#ifndef NO_LOGGER"


/*---------------------------------------------------------------------------------------- M ---*/
byte IRMA_M_v20_r_Resp(word PayloadLen, byte *Payload)
{
	byte ErrVal;
	dword MemAddr, MemAddrEnd;
	word MemByteCnt, MemByteIdx;

	if( PayloadLen != IRMA_Mv20_HeadLen )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		ErrVal = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('M', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr    = get_dword(Payload);
	MemByteCnt = ((word)Payload[5] << 8) + (word)Payload[4];
	MemAddrEnd = MemAddr + (dword)(MemByteCnt - 1);

	if( MemAddr > MaxMemAddrA21 )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Build_IRMA_Error_v10('M', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr/*Payload*/);
		return(IRMAErr_InvMemAddr);
	}

	if( MemByteCnt == 0 || MemByteCnt > IRMA_Mv20_MaxMemByteCnt || 
	    MemAddrEnd > MaxMemAddrA21 )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Build_IRMA_Error_v10('M', IRMAErr_InvMemByteCnt, 2, (byte *)&MemByteCnt);
		return(IRMAErr_InvMemByteCnt);
	}

	PayloadLen = IRMA_Mv20_HeadLen + MemByteCnt;
	init_uip_send_buff_writing_20(uip_service_lev_send, 'M', 0x20, uip_sub_cmd_send, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[0];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[1];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[2];
	IRMASendBuff[IRMASendBuffWritePos++] = Payload[3];
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)MemByteCnt;		 	// little endian
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)(MemByteCnt >> 8);

	for( MemByteIdx = 0; MemByteIdx < MemByteCnt; MemByteIdx++ )
		IRMASendBuff[IRMASendBuffWritePos++] = *(byte huge *)(MemAddr++);

	return(0);
}	// IRMA_M_v20_r_Resp


byte IRMA_M_v20_v_w_Resp(bool to_verifyB, word PayloadLen, byte *Payload)
{
	byte ErrVal, Writing_Permission, Writing_Permission_End;
	dword MemAddr, MemAddrEnd;
	word MemByteCnt;
	int ErrNo = 0;

	if( PayloadLen < IRMA_Mv20_HeadLen )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		ErrVal = PayloadLen + UIP_20_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('M', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	MemAddr    = get_dword(Payload);
	MemByteCnt = ((word)Payload[5] << 8) + (word)Payload[4];
	MemAddrEnd = MemAddr + (dword)(MemByteCnt - 1);

	Writing_Permission = Get_Writing_Permission(MemAddr);
	if( Writing_Permission == NOT_ALLOWED )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Build_IRMA_Error_v10('M', IRMAErr_InvMemAddr, 4, (byte *)&MemAddr/*Payload*/);
		return(IRMAErr_InvMemAddr);
	}
	Writing_Permission_End = Get_Writing_Permission(MemAddrEnd);

	if( MemByteCnt == 0 || MemByteCnt > IRMA_Mv20_MaxMemByteCnt ||
		Writing_Permission_End == NOT_ALLOWED || Writing_Permission_End != Writing_Permission )
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Build_IRMA_Error_v10('M', IRMAErr_InvMemByteCnt, 2, (byte *)&MemByteCnt);
		return(IRMAErr_InvMemByteCnt);
	}

	memcpy((void *)MemAddr, (void *)&Payload[6], MemByteCnt);
	if( to_verifyB )
		ErrNo = memcmp((void *)MemAddr, (void *)&Payload[6], MemByteCnt);
	else
		ErrNo = 0;

	if( ErrNo == 0 )
	{
		init_uip_send_buff_writing_20(uip_service_lev_send, 'M', 0x20, uip_sub_cmd_send, 0);
		return(0);
	}											 
	else
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Build_IRMA_Error_v10('M', IRMAErr_MemWrite, 4, (byte *)(&MemAddr));
		return(IRMAErr_MemWrite);
	}
}	// IRMA_M_v20_v_w_Resp


byte IRMA_M_v20(byte SrcAddr, char sub_cmd, word PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		switch( sub_cmd )
		{
			case 'r' :	Result = IRMA_M_v20_r_Resp(PayloadLen, Payload);
						break;

			case 'v' :
			case 'w' :	Result = IRMA_M_v20_v_w_Resp(sub_cmd == 'v', PayloadLen, Payload);
						break;

			default	 :	uip_service_lev_send = 0;
						uip_sub_cmd_send = 0;
		
						Build_IRMA_Error_v10('M', IRMAErr_UnkSubCmd, 1, (byte *)&sub_cmd);
						Result = IRMAErr_UnkSubCmd;
		}	// end of "switch( sub_cmd )"

		send_buffered_uip_20_frame(SrcAddr);
		return(Result);
	}
	else
	{
		uip_service_lev_send = 0;
		uip_sub_cmd_send = 0;
		
		Send_IRMA_Error_v10(SrcAddr, 'M', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// IRMA_M_v20


void irma_msg_memory_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	retry_flagB;

	switch( DataVer )
	{
		case 0x10 :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					IRMA_M_v10(SrcAddr, PayloadLen, Payload);
					break;

		case 0x20 :	uip_service_lev_send = service_lev;
					uip_sub_cmd_send = (char)((byte)sub_cmd - 0x20);
						
					IRMA_M_v20(SrcAddr, sub_cmd, PayloadLen, Payload);
					break;

		default   :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					Send_IRMA_Error_v10(SrcAddr, 'M', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_memory_gateway


/*---------------------------------------------------------------------------------------- N ---*/
void irma_msg_new_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_N_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'N', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_new_gateway


/*---------------------------------------------------------------------------------------- O ---*/
void irma_msg_port_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		SrcAddr;
		PayloadLen;
		Payload;
		case 0x10 :	IRMA_O_v10(SrcAddr, PayloadLen, Payload);
					break;

#if defined(SW4) || defined(SW2)
		case 0x20 :	IRMA_O_v20(SrcAddr, PayloadLen, Payload);
					break;
#endif	// end of "#if defined(SW4) || defined(SW2)"

		default   :	Send_IRMA_Error_v10(SrcAddr, 'O', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_port_gateway


/*---------------------------------------------------------------------------------------- P ---*/
void irma_msg_programm_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_P_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'P', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_programm_gateway


#ifdef TEST
/*---------------------------------------------------------------------------------------- Q ---*/
byte irma_Q_v20_resp_gateway(byte PayloadLen, byte *Payload)
{
	byte ErrVal, sen_no;
	dword dword_val;

	if( PayloadLen != 1 )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('Q', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	sen_no = Payload[0];
	if( sen_no == 0 || sen_no > obc_interface_data.sensor_cnt )
	{
		Build_IRMA_Error_v10('Q', IRMAErr_InvSenNo, 1, Payload);
		return(IRMAErr_InvSenNo);
	}

	// All numbers use little-endian byte order.
	// Payload[0]:					sensor no. (1..)
	// Payload[1]:					sensor status
	// Payload[2]..[5]:				cycle counter
	// Payload[6]..[9]:				error counter
	// Payload[10]..[11]:			count of sensor signal values included in current test (ValCnt), 
	//								ValCnt == 0: no test results available
	// Payload[12]: 				count of elements per sensor (ElemCnt)
	//								ElemCnt == 0: no element information available
	PayloadLen = 13;
	init_uip_send_buff_writing_10('Q', 0x20, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = sen_no;

	dword_val = 0;									// sensor status
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val = can_sensor_commu_cycle_cnt;			// cycle counter
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val = can_sensor_commu_cycle_error_cnt;	// error counter
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	dword_val >>= 8;
	IRMASendBuff[IRMASendBuffWritePos++] = (byte)dword_val;
	IRMASendBuff[IRMASendBuffWritePos++] = 0;		// count of sensor signal values included in current test
	IRMASendBuff[IRMASendBuffWritePos++] = 0;
	IRMASendBuff[IRMASendBuffWritePos++] = 0;		// count of elements per sensor

	return(0);
}	// irma_Q_v20_resp_gateway


byte irma_Q_v20_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr == IRMAAddrServiceDev )
	{
		Result = irma_Q_v20_resp_gateway(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'Q', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// irma_Q_v20_gateway


void irma_msg_query_sensor_signal_test_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	switch( DataVer )
	{
		case 0x20 :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					irma_Q_v20_gateway(SrcAddr, PayloadLen, Payload);
					break;

		default   :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					Send_IRMA_Error_v10(SrcAddr, 'Q', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_query_sensor_signal_test_gateway
#endif


/*---------------------------------------------------------------------------------------- R ---*/
void irma_msg_run_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_R_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'R', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_run_gateway


/*---------------------------------------------------------------------------------------- S ---*/
void build_irma_S_v20_payload_gateway(void)
{
	byte PayloadLen, SenIdx;
	word *PayloadWordPtr, PayloadWord;

	PayloadLen = IRMA_Sv20_PayloadLenMin + obc_interface_data.sensor_cnt * sizeof(word);

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

	IRMASendBuff[IRMASendBuffWritePos++] = obc_interface_data.sensor_cnt;
	for( SenIdx = 0; SenIdx < obc_interface_data.sensor_cnt; SenIdx++ )
	{
		PayloadWord = 0;
		// initialization error
		if( obc_interface_data.sensor_list[SenIdx].ever_static_data_invalidB )
			PayloadWord |= BIT0; 
		// runtime error
		if( obc_interface_data.sensor_list[SenIdx].ever_commu_cycle_failedB    ||
		    obc_interface_data.sensor_list[SenIdx].ever_error_msg_receivedB    ||
		    obc_interface_data.sensor_list[SenIdx].ever_invalid_config_paramsB ||
		    obc_interface_data.sensor_list[SenIdx].ever_slave_missingB )
			PayloadWord |= BIT1; 
		// sabotage error
		if( obc_interface_data.sensor_list[SenIdx].sabotageB )
			PayloadWord |= BIT2; 
		// firmware mismatch error
		if( obc_interface_data.sensor_list[SenIdx].ever_firmware_mismatchedB )
			PayloadWord |= BIT3; 
		Put_word_IRMASendBuff(PayloadWord);
	}
}	// build_irma_S_v20_payload_gateway


byte irma_S_v20_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	byte SenCnt, SenIdx, PayloadPos, fa_idx;
	word *PayloadWordPtr, PayloadWord;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr >= IRMAAddrReserveFirst )
	{
		if( PayloadLen == 0 )	// Incoming IRMA-S v2.0 request message
		{
			build_irma_S_v20_payload_gateway();
			send_buffered_uip_10_frame(SrcAddr);
			return(0);
		}

		if( PayloadLen >= IRMA_Sv20_PayloadLenMin && (PayloadLen - IRMA_Sv20_PayloadLenMin) % sizeof(word) == 0 )
		{
			SenCnt = obc_interface_data.sensor_cnt;
			if( SenCnt <= CAN_SENSOR_LIST_LEN &&
			    PayloadLen == IRMA_Sv20_PayloadLenMin + SenCnt * sizeof(word) &&
			    Payload[IRMA_Sv20_PayloadLenMin - 1] == SenCnt )
			{
				Save_ILVL_and_Disable_All_INTs();
				PayloadWordPtr = (word *)&A21_Status.ASC0;
				*PayloadWordPtr = ((word)Payload[1] << 8) + (word)Payload[0];
				PayloadWordPtr = (word *)&A21_Status.CAN1;
				*PayloadWordPtr = ((word)Payload[3] << 8) + (word)Payload[2];
				PayloadWordPtr = (word *)&A21_Status.Sensor_Interface;
				*PayloadWordPtr = ((word)Payload[5] << 8) + (word)Payload[4];
				PayloadWordPtr = (word *)&A21_Status.Optional_Hardware;
				*PayloadWordPtr = ((word)Payload[7] << 8) + (word)Payload[6];
				PayloadWordPtr = (word *)&A21_Status.Logger;
				*PayloadWordPtr = ((word)Payload[9] << 8) + (word)Payload[8];

				PayloadPos = IRMA_Sv20_PayloadLenMin;
				for( SenIdx = 0; SenIdx < SenCnt; SenIdx++ )
				{
					PayloadWord = ((word)Payload[PayloadPos + 1] << 8) + (word)Payload[PayloadPos];
					// initialization error
					obc_interface_data.sensor_list[SenIdx].ever_static_data_invalidB   = (PayloadWord & BIT0) == BIT0;
					// runtime error
					obc_interface_data.sensor_list[SenIdx].ever_commu_cycle_failedB    = (PayloadWord & BIT0) == BIT1;
					obc_interface_data.sensor_list[SenIdx].ever_error_msg_receivedB    = (PayloadWord & BIT0) == BIT1;
					obc_interface_data.sensor_list[SenIdx].ever_invalid_config_paramsB = (PayloadWord & BIT0) == BIT1;
					obc_interface_data.sensor_list[SenIdx].ever_slave_missingB         = (PayloadWord & BIT0) == BIT1;
					// sabotage error
					obc_interface_data.sensor_list[SenIdx].sabotageB                   = (PayloadWord & BIT2) == BIT2; 
					// firmware mismatch error
					obc_interface_data.sensor_list[SenIdx].ever_firmware_mismatchedB   = (PayloadWord & BIT3) == BIT3; 

					PayloadPos += sizeof(word);
				}
				Restore_ILVL();

				for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
					clear_persistent_error_flags_of_function_area_and_assigned_sensors(fa_idx);

				build_irma_S_v20_payload_gateway();
				send_buffered_uip_10_frame(SrcAddr);
				return(0);
			}

			// Number of sensors included in payload doesn't match with the number of sensors handled
			// by gateway.
			Send_IRMA_Error_v10(SrcAddr, 'S', IRMAErr_SetNotAllowed, PayloadLen, Payload);
			return(IRMAErr_SetNotAllowed);
		}

		Send_Error_InvL7Length(SrcAddr, 'S', PayloadLen);
		return(IRMAErr_InvMsgLen);
	}

	Send_IRMA_Error_v10(SrcAddr, 'S', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
	return(IRMAErr_SrcAddrNotAllowed);
}	// irma_S_v20_gateway


void Build_IRMA_S_v21_Payload(void)
{
	byte PayloadLen, SenIdx;

	PayloadLen = 1 + obc_interface_data.sensor_cnt * 3;

	init_uip_send_buff_writing_10('S', 0x21, PayloadLen);
	IRMASendBuff[IRMASendBuffWritePos++] = obc_interface_data.sensor_cnt;
	for( SenIdx = 0; SenIdx < obc_interface_data.sensor_cnt; SenIdx++ )
	{
		IRMASendBuff[IRMASendBuffWritePos++] = 0;
		IRMASendBuff[IRMASendBuffWritePos++] = 0;
		IRMASendBuff[IRMASendBuffWritePos++] = 0;
	}
}	// Build_IRMA_S_v21_Payload

	  
byte IRMA_S_v21(byte SrcAddr, byte PayloadLen, byte *Payload)
{
	Payload;
	if( PayloadLen == 0 )	// Incoming IRMA-S v2.1 request message
	{
		Add_Runlevel_Job(check_can_sensor_error_counter_response);
		Add_Def_Timeout_Job(100);

		send_can_status_query_msgs();
		return(0);
	}

	Send_Error_InvL7Length(SrcAddr, 'S', PayloadLen);
	return(IRMAErr_InvMsgLen);
}	// IRMA_S_v21


void Send_IRMA_S_v21(void)
{
	Build_IRMA_S_v21_Payload();
	send_buffered_uip_10_frame(IrmaSrcAddr);
}


void irma_msg_status_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x20 :	irma_S_v20_gateway(SrcAddr, PayloadLen, Payload);
					break;

		case 0x21 :	IRMA_S_v21(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'S', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_status_gateway


/*---------------------------------------------------------------------------------------- T ---*/
#ifndef NO_LOGGER
void irma_msg_time_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_T_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'T', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_time_gateway
#endif


/*---------------------------------------------------------------------------------------- U ---*/
void irma_msg_runlevel_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_U_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'U', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_runlevel_gateway


/*---------------------------------------------------------------------------------------- V ---*/
void irma_msg_version_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x20 :	IRMA_V_v20(SrcAddr, PayloadLen, Payload);
					break;

		case 0x21 :	IRMA_V_v21(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'V', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_version_gateway


/*---------------------------------------------------------------------------------------- X ---*/
void irma_msg_execute_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	uip_service_lev_send = 0;
	uip_sub_cmd_send = 0;

	switch( DataVer )
	{
		case 0x10 :	IRMA_X_v10(SrcAddr, PayloadLen, Payload);
					break;

		default   :	Send_IRMA_Error_v10(SrcAddr, 'X', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_execute_gateway


#ifdef TEST
/*---------------------------------------------------------------------------------------- Y ---*/
byte irma_Y_v10_resp_gateway(byte PayloadLen, byte *Payload)
{
	byte ErrVal, fa_addr, fa_idx;
	char subcmd_id;

	if( PayloadLen != 2 )
	{
		ErrVal = PayloadLen + UIP_10_MESSAGE_DATAOFFSET;
		Build_IRMA_Error_v10('Y', IRMAErr_InvMsgLen, 1, &ErrVal);
		return(IRMAErr_InvMsgLen);
	}

	fa_addr = Payload[1];
	fa_idx = get_idx_of_function_area_in_obc_interface_data(fa_addr);
	if( fa_idx == 0xFF )
	{
		Build_IRMA_Error_v10('Y', IRMAErr_InvFAAddr, 1, Payload);
		return(IRMAErr_InvFAAddr);
	}

	subcmd_id = (char)Payload[0];
	switch( subcmd_id )
	{
		case 'Q' :	PayloadLen = 1;
					init_uip_send_buff_writing_10('Y', 0x10, PayloadLen);
					IRMASendBuff[IRMASendBuffWritePos++] = fa_addr;
					return(0);

		default  :	Build_IRMA_Error_v10('Y', IRMAErr_UnkSubCmd, 1, (byte *)(&subcmd_id));
					return(IRMAErr_UnkSubCmd);
	}
}	// irma_Y_v10_resp_gateway


byte irma_Y_v10_gateway(byte SrcAddr, byte PayloadLen, byte *Payload)
{
    byte Result;

	// Error SrcAddr == IRMAAddrUniversalDest handled by function read_uip_frame.
	if( SrcAddr == IRMAAddrServiceDev )
	{
		Result = irma_Y_v10_resp_gateway(PayloadLen, Payload);
		send_buffered_uip_10_frame(SrcAddr);
		return(Result);
	}
	else
	{
		Send_IRMA_Error_v10(SrcAddr, 'Y', IRMAErr_SrcAddrNotAllowed, 1, &SrcAddr);
		return(IRMAErr_SrcAddrNotAllowed);
	}
}	// irma_Y_v10_gateway


void irma_msg_inst_mode_gateway(byte uip_ver, byte service_lev, bool retry_flagB, byte SrcAddr, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload)
{
	uip_ver;
	service_lev;
	retry_flagB;
	sub_cmd;

	switch( DataVer )
	{
		case 0x10 :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					irma_Y_v10_gateway(SrcAddr, PayloadLen, Payload);
					break;

		default   :	uip_service_lev_send = 0;
					uip_sub_cmd_send = 0;
		
					Send_IRMA_Error_v10(SrcAddr, 'Y', IRMAErr_UnkTypeIdentVer, 1, &DataVer);
					break;
	}
}	// irma_msg_inst_mode_gateway
#endif
