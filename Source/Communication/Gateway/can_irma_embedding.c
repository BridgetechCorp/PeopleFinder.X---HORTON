/*==============================================================================================*
 |       Filename: can_irma_embedding.c                                                         |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions for CAN IRMA embedding of UIP 3.0 datagrams (compressed UIP 2.0    |
 |                 datagrams)                                                                   |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifndef GATEWAY
	#error "File can_irma_embedding.c included but symbol GATEWAY not defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167.h>
#include <stddef.h>
#include <string.h>

#include "..\..\interrupt_defines.h"
#include "..\..\kernel_interface.h"
#include "..\communication.h"
#include "communication_gdist500.h"
#include "..\can.h"
#include "..\can_irma.h"
#include "can_irma_embedding.h"
#include "can_irma_dist500.h"
#include "..\uip.h"
#include "uip_gdist500.h"
#ifndef NO_LOGGER
//	#include "..\logger.h"
//	#include "logger_gdist500.h"
#endif


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=COMM
#pragma class CO=COMM
#pragma class FB=COMMRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
can_addr_id_type sensor_addr_ids[CAN_SENSOR_LIST_LEN];

byte uip_30_rec_buf[UIP_20_PAYLOADLEN_MAX];
word uip_30_rec_buf_pos;
bool uip_30_rec_buf_validB;

bool to_enable_can_transparent_modeB;
bool can_transparent_modeB;
bool (*enable_can_transparent_mode_func)(void);

byte exp_can_msg_no;
word uip_30_desired_dl, uip_30_current_dl;

// Muzzle_ModeB == FALSE:
// Normal CAN IRMA operation.
// Muzzle_ModeB == TRUE:
// Transmission of CAN IRMA messages permitted only if queried by service device (PC).
// Example: Update of configuration data or firmware stored in CAN IRMA Sensors or A21C(L)
// is going on.
// Var. Muzzle_ModeB is initialized with value FALSE. It may be changed as a
// result of receiving of CAN IRMA message "muzzle (Maulkorb)" (message identifier
// 0x11) or according IRMA-A v5.0 request message.
bool Muzzle_ModeB;             

bool to_update_can_sensor_commu_idx_and_stepB;
bool to_delay_can_sensor_commuB;
bool to_continue_can_sensor_commuB;

/*
byte old_runlevel_log;
#ifndef NO_LOGGER
byte new_runlevel_log;
#endif
*/


/*----- Function Prototypes --------------------------------------------------------------------*/
bool start_can_sensor_direct_query_of_devimmanent_confdata(void);
void can_sensor_direct_query_of_devimmanent_confdata_failed(void);
void launch_start_can_sensor_query_of_devimmanent_confdata_by_eeprom_reading(void);
void can_sensor_query_of_devimmanent_confdata_finished(void);
void can_sensor_basic_conf_finished(void);
void launch_can_sensor_basic_conf_finished(void);
void send_ext_conf_msg_to_can_sensor(byte sen_idx);
void can_sensor_ext_conf_failed(void);
void can_sensor_init_finished(void);
void launch_can_sensor_init_finished(void);
void route_uip_30_msg_from_can(void);


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/

// Initialize global variables not properly initialized by function "clear_comm_RAM".
void init_global_var_can_irma_embedding(void)
{
	determine_analyzer_addr_id();

	enable_can_transparent_mode_func = enable_can_transparent_mode;
}	// init_global_var_can_irma_embedding


void init_uip_30_receiving(void)
{
	uip_30_desired_dl = 0; 
	uip_30_current_dl = 0;
	exp_can_msg_no = 1;
}	// init_uip_30_receiving


/*----------------------------------------------------------------------------*
 | Function:  										  |
 |----------------------------------------------------------------------------|
 | Description:                                                               |
 |                                                                            |
 *----------------------------------------------------------------------------*/
byte init_can_interf_gateway(void)
{
	begin_can1_init();

	conf_can_transmit_msg_obj();
	conf_can_receive_msg_objs();

	clear_can_receive_buffer();
	clear_can_transmit_buffer();

	init_uip_30_receiving();

	end_can1_init();

	return(0);							//no error
}	// init_can_interf_gateway


void call_muzzle_mode_activation_jobs(void)
{
	clear_can_transmit_buffer();
}	//call_muzzle_mode_activation_jobs


//|-------------------------------------------------------------------------------------|
//| Function: Change_Muzzle_Mode()														|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Called by function process_can_irma_message on reception of CAN IRMA message with	|
//| identifier 0x11.																	|
//| IRMA Analyzer muzzle mode is activated or deactivated depending on first data byte	|
//| of received CAN IRMA message. If muzzle mode is active transmission of CAN IRMA		|
//| messages is permitted only if queried by service device (PC).						|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void Change_Muzzle_Mode(void)
{
	byte CAN_Msg_Data[2];

	Muzzle_ModeB = can_receive_buf[can_receive_buf_read + DATA_OFFSET];

	if( Muzzle_ModeB )
		call_muzzle_mode_activation_jobs();
	
	CAN_Msg_Data[0] = Muzzle_ModeB;
	CAN_Msg_Data[1] = 2;
	send_can_irma_msg(CAN_Msg_Data, 2, MUZZLE_MODE_CFM, IRMA_PC, analyzer_addr_id);
}	// Change_Muzzle_Mode


//|-------------------------------------------------------------------------------------|
//| Function: send_can_irma_msg															|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Create single CAN IRMA message and store it in CAN transmit buffer.					|
//| Finally transmission of CAN IRMA message is started.								|
//| Input:																				|
//| - *data_ptr			(pointer to data bytes of CAN IRMA messge)	 					|
//| - total_length		(total count of data bytes)			 							|
//| - can_irma_msg_id	(CAN IRMA message identifier)		 							|
//| - rec_class			(CAN IRMA receiver class)										|
//| - addr_id			(CAN IRMA device address)										|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void send_can_irma_msg(byte *data_ptr, byte total_length,
                       byte can_irma_msg_id, byte rec_class, can_addr_id_type addr_id)
{
	byte id_ll  = (can_irma_msg_id << 1) + (rec_class >> 2);
	byte id_lh  = (rec_class << 6) + addr_id.high;
	byte id_hl  = addr_id.mid;
	byte id_hh  = addr_id.low;
	byte data_idx;
 
	// If muzzle mode for IRMA Analyzer is active only CAN IRMA messages with receiver class
	// service device (PC) excluding counting result messages are transmitted.
	if( Muzzle_ModeB && (can_irma_msg_id == COUNT_RES_SEND || rec_class != IRMA_PC) )
		return;

	total_length = MIN(total_length, MAX_CAN_DATA_LEN);

    can_transmit_buf[can_transmit_buf_write].id_hh = id_ll;
    can_transmit_buf[can_transmit_buf_write].id_hl = id_lh;
    can_transmit_buf[can_transmit_buf_write].id_lh = id_hl;
    can_transmit_buf[can_transmit_buf_write].id_ll = id_hh;
    can_transmit_buf[can_transmit_buf_write].length = total_length;
	for( data_idx = 0; data_idx < total_length; data_idx++ )
		can_transmit_buf[can_transmit_buf_write].data[data_idx] = *data_ptr++;

	can_transmit_buf_write_update();	// Update buffer write pointer and transmit CAN message.
}	// send_can_irma_msg


//|-------------------------------------------------------------------------------------|
//| Function: send_can_irma_msg_set														|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Create one or more CAN IRMA messages and store them in CAN transmit buffer.	CAN		|
//| IRMA message number is included as first data byte. As much messages as necessary	|
//| are created to enable transmission of all data byte specified by function			|
//| parameters *data_ptr and total_length.												|
//| Finally transmission of CAN IRMA message(s) is started.								|
//| Input:																				|
//| - *data_ptr			(pointer to data bytes of CAN IRMA messge(s)) 					|
//| - complete_msg_noB	(== TRUE: high nibble of CAN IRMA message number contains total	|
//|						count of messages)												|
//| - total_length		(total count of data bytes, message numbers not included)		|
//| - can_irma_msg_id	(CAN IRMA message identifier)		 							|
//| - rec_class			(CAN IRMA receiver class)										|
//| - addr_id			(CAN IRMA device address)										|
//| Output:	non																			|
//|																						|
//| Important limitation:																|
//| Maximum total count of data bytes is equal to 105 = 7 * 15 byte.					|
//|-------------------------------------------------------------------------------------|
void send_can_irma_msg_set(byte *data_ptr, bool complete_msg_noB, byte total_length,
                           byte can_irma_msg_id, byte rec_class, can_addr_id_type addr_id)
{
	byte id_ll  = (can_irma_msg_id << 1) + (rec_class >> 2);
	byte id_lh  = (rec_class << 6) + addr_id.high;
	byte id_hl  = addr_id.mid;
	byte id_hh  = addr_id.low;
	byte data_idx;
	byte can_msg_cnt = 1;			// count of CAN messages to be transmitted
	byte can_msg_no;
	byte can_msg_len;				// length of the current message
	byte no_of_data_bytes;		  	// number of data bytes in CAN message excluding
									// message number
 
	// If muzzle mode for IRMA Analyzer is active only CAN IRMA messages with receiver class
	// service device (PC) are transmitted.
	if( Muzzle_ModeB && rec_class != IRMA_PC )
		return;

	// Count of CAN messages to be transmitted is equal to 1 if total_length == 0.
	if( total_length > 0 )
	{
		can_msg_cnt = total_length / (MAX_CAN_DATA_LEN - 1);
		if( total_length > ((MAX_CAN_DATA_LEN - 1) * can_msg_cnt) )
			can_msg_cnt++;
	}

	for( can_msg_no = 1; can_msg_no <= can_msg_cnt; can_msg_no++ )
	{
		// Determine number of data bytes for next CAN message.
		can_msg_len = MIN((total_length + 1), MAX_CAN_DATA_LEN);
		no_of_data_bytes = can_msg_len - 1;

        can_transmit_buf[can_transmit_buf_write].id_hh = id_ll;
    	can_transmit_buf[can_transmit_buf_write].id_hl = id_lh;
    	can_transmit_buf[can_transmit_buf_write].id_lh = id_hl;
    	can_transmit_buf[can_transmit_buf_write].id_ll = id_hh;
		can_transmit_buf[can_transmit_buf_write].length = can_msg_len;
		if( can_msg_len > 0 )
		{
			if( complete_msg_noB )
				// High nibble of CAN IRMA message number contains count of CAN IRMA messages (>= 1, <= 15)
				// forming CAN IRMA message set.
				can_transmit_buf[can_transmit_buf_write].data[0] = can_msg_no | (can_msg_cnt << 4);
			else
				// CAN IRMA messages with identifier 0x43 transmitted by IRMA Analyzer contain truncated
				// CAN IRMA message number only where lower nibble is equal to number of current message
				// within set and higher nibble is always equal to zero.
				can_transmit_buf[can_transmit_buf_write].data[0] = can_msg_no;
			for( data_idx = 1; data_idx < can_msg_len; data_idx++ )
				can_transmit_buf[can_transmit_buf_write].data[data_idx] = *data_ptr++;
		}

        NEXT(can_transmit_buf_write, CAN_TRANSMIT_BUF_MSG_CNT);

		if( total_length > no_of_data_bytes )
			total_length -= no_of_data_bytes;
		else
			total_length = 0;
	}	// end of "for( can_msg_no = 1; can_msg_no <= can_msg_cnt; can_msg_no++ )"

	can_transmit();	 	// Transmit CAN message(s) (at least one).
}	// send_can_irma_msg_set


void send_can_status_query_msgs(void)
{
}	//send_can_status_query_msgs


void receive_can_error_counters(void)
{
}	// receive_can_error_counters


void check_can_sensor_error_counter_response(void)
{
}	// check_can_sensor_error_counter_response


void process_sensor_startup_message(void)
{
}	// process_sensor_startup_message


void process_uip_30_msg(void)
{
	byte irma_can_msg_id, can_msg_no, can_data_len, can_data_idx, exp_can_data_len;
	byte *can_data_ptr;
	dword can_ident;
	byte uip_ver_service_level, uip_ver;
	word r_dl, uip_30_residual_dl;
	bool uip_30_msg_completeB;

	if( exp_can_msg_no == 0 || can_transparent_modeB )
		return;

	irma_can_msg_id = can_receive_buf[can_receive_buf_read + ID_HH_OFFSET] >> 1;
	can_data_len = can_receive_buf[can_receive_buf_read + LEN_OFFSET];

	if( irma_can_msg_id != UIP_20_RECV || can_data_len < 2 )
	{
		handle_uip_30_receive_error();
		return;
	}

    can_data_ptr = &can_receive_buf[can_receive_buf_read + DATA_OFFSET];
	can_msg_no = can_data_ptr[0];

	if( can_msg_no == exp_can_msg_no )
	{
		if( can_msg_no == 1 )
		{
			if( can_data_len < 7 )
			{
				handle_uip_30_receive_error();
				return;
			}

			can_ident  = (dword)can_receive_buf[can_receive_buf_read + ID_HH_OFFSET] << 21;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_HL_OFFSET] << 13;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_LH_OFFSET] << 5;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_LL_OFFSET] >> 3;
			can_ident &= 0x003FFFFFUL;

			uip_ver_service_level = can_data_ptr[1];
			uip_ver = uip_ver_service_level & 0xF0;
			r_dl = (word)can_data_ptr[2] + ((word)can_data_ptr[3] << 8);
			uip_30_desired_dl = r_dl & 0x7FFF;

			if( uip_ver != 0x30 || uip_30_desired_dl < 3 || uip_30_desired_dl > 1393 )
			{
				handle_uip_30_receive_error();
				return;
			}

			uip_30_rec_buf_validB = FALSE;
			uip_30_rec_buf_pos = 0;
			uip_30_rec_buf[uip_30_rec_buf_pos++] = (byte)can_ident;
			uip_30_rec_buf[uip_30_rec_buf_pos++] = (byte)(can_ident >> 8);
			uip_30_rec_buf[uip_30_rec_buf_pos++] = (byte)(can_ident >> 16);
			uip_30_rec_buf[uip_30_rec_buf_pos++] = 0;
			uip_30_rec_buf[uip_30_rec_buf_pos++] = uip_ver_service_level;
			uip_30_rec_buf[uip_30_rec_buf_pos++] = (byte)r_dl;
			uip_30_rec_buf[uip_30_rec_buf_pos++] = (byte)(r_dl >> 8);
			uip_30_rec_buf[uip_30_rec_buf_pos++] = can_data_ptr[4];
			uip_30_rec_buf[uip_30_rec_buf_pos++] = can_data_ptr[5];
			uip_30_rec_buf[uip_30_rec_buf_pos++] = can_data_ptr[6];
			if( can_data_len == MAX_CAN_DATA_LEN )
			{
				uip_30_rec_buf[uip_30_rec_buf_pos++] = can_data_ptr[7];
				uip_30_current_dl = 4;
			}
			else
				uip_30_current_dl = 3;
			if( uip_30_desired_dl > 3 )
				exp_can_data_len = MAX_CAN_DATA_LEN;
			else
				exp_can_data_len = 7;
		}
		else
		{
			for( can_data_idx = 1; can_data_idx < can_data_len; can_data_idx++ ) 
				uip_30_rec_buf[uip_30_rec_buf_pos++] = can_data_ptr[can_data_idx];
			uip_30_residual_dl = uip_30_desired_dl - uip_30_current_dl;
			if( uip_30_residual_dl >= 7 )
				exp_can_data_len = MAX_CAN_DATA_LEN;
			else
				exp_can_data_len = 1 + uip_30_residual_dl;
			uip_30_current_dl += (exp_can_data_len - 1);
		}

		if( can_data_len != exp_can_data_len )
		{
			handle_uip_30_receive_error();
			return;
		}

		uip_30_msg_completeB = uip_30_current_dl == uip_30_desired_dl;
		if( uip_30_msg_completeB )
		{
			init_uip_30_receiving();
			//
			uip_30_rec_buf_validB = TRUE;
		}
		else
			exp_can_msg_no++;
	}
	else
	{
		handle_uip_30_receive_error();
		return;
	}
}	// process_uip_30_msg


//|-------------------------------------------------------------------------------------|
//| Function: process_can_irma_message													|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Called by run level job check_for_pending_can_irma_jobs if CAN IRMA receive buffer	|
//| is not empty. Received CAN IRMA message is processed by call of corresponding 		|
//| service function. 																	|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void process_can_irma_message(void)
{
	byte *buf_ptr, can_irma_msg_id, receiver_class;

	buf_ptr = can_receive_buf + can_receive_buf_read;
	// Calculate number of data bytes from message configuration register of
	// CAN message object.
	buf_ptr[LEN_OFFSET] = (buf_ptr[LEN_OFFSET] >> 4) & 0x0F;

	// Flag MSGLST == 1 found in CAN receive buffer ?
	if( (buf_ptr[MO_STAT_OFFSET_H] & FLAG_SET_MSGLST) == FLAG_SET_MSGLST )
		A21_Status.CAN1.Rec_Msg_LostB = TRUE;

	// Flag RXOK == 0 for received CAN message ?
	if( (buf_ptr[STATUS_OFFSET] & 0x10) != 0x10 )
	{
		// Yes, discard CAN message.
		A21_Status.CAN1.Rec_Msg_InvalidB = TRUE;
		can_receive_buf_read += CAN_MSG_OBJ_LEN;
		if( can_receive_buf_read >= CAN_RECEIVE_BUF_LEN )
			can_receive_buf_read = 0;
		return;
	} 

	receiver_class = (buf_ptr[0] & 0x01) << 2 | (buf_ptr[1] & 0xC0) >> 6;
	// Ignore CAN IRMA messages addressed to service device or sensor(s).
	if( receiver_class < ALL_ANAS_AND_SERV_DEV || receiver_class == ALL_SENSORS )
	{
		can_receive_buf_read += CAN_MSG_OBJ_LEN;
		if( can_receive_buf_read >= CAN_RECEIVE_BUF_LEN )
			can_receive_buf_read = 0;
		return;
	}

	can_irma_msg_id = buf_ptr[ID_HH_OFFSET] >> 1;

	switch( can_irma_msg_id )
	{
		// Transmitted by CAN IRMA service device, typically PC running IRMA-A21-Windows.
		case MUZZLE_MODE_SET:
			Change_Muzzle_Mode();
			break;

		// CAN IRMA Sensor startup message,
		// identifier 0x20, receiver class 111 (all CAN IRMA devices), 
		// logical address or device address of IRMA Sensor
		// Transmitted by CAN IRMA Sensor(s).
		case SEN_STARTUP_NOT:
			process_sensor_startup_message();
			break;

		// Restart of analyzer, e.g. after update of firmware parameters or firmware.
		case SOFTWARE_RESET:
			_int166(0);
			break;

		// Transmitted by CAN IRMA Sensor(s).
	    case SEN_ERR_CNT:
			receive_can_error_counters();
			break;

		// UIP 2.0 over CAN IRMA, receiving,
		// message identifier 0x75, receiver class 010 (certain IRMA Analyzer), 
		// CAN IRMA Analyzer device address
		// Transmitted by CAN IRMA service device, typically PC running IRMA MATRIX service software.
		case UIP_20_RECV:
			if( can_transparent_modeB )
				route_uip_30_msg_from_can();
			else
				process_uip_30_msg();
			break;

		default:
			break;
	} //end of switch( can_irma_msg_id )

	/*Increase read pointer*/
	can_receive_buf_read += CAN_MSG_OBJ_LEN;
	if( can_receive_buf_read >= CAN_RECEIVE_BUF_LEN )
		can_receive_buf_read = 0;
}	// process_can_irma_message


//|-------------------------------------------------------------------------------------|
//| Function: check_for_pending_can_irma_jobs											|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Called as run level job. Added to run level jobs by function communication_RL4.		|
//| CAN IRMA receive buffer is checked for not being empty. Function					|
//| process_can_irma_message is called for each received CAN IRMA message.				|
//| Furtheron CAN IRMA status message send flag is checked. If true CAN IRMA status		|
//| message buffer content is copied to CAN IRMA transmission buffer and transmission	|
//| is started.																			|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void check_for_pending_can_irma_jobs(void)
{
	byte cnt = 0;

	while( can_receive_buf_read != can_receive_buf_write )
	{
		// UIP v3.0 message stored in buffer uip_30_rec_buf has to be processed before reading of
		// CAN receive buffer is continued.
		if( !can_transparent_modeB && uip_30_rec_buf_validB )
			break;

		process_can_irma_message();

		// Process up to 32 received CAN IRMA messages on 1 call of
		// check_for_pending_can_irma_jobs at maximum.
		if( cnt++ >= 32/*8*/ )
			return;
	}

	if( !can_transparent_modeB )
	{
		if( uip_30_rec_buf_validB )
		{
			eval_can_sensor_response();
			// Invalidate message to prevent repeated processing.
			uip_30_rec_buf_validB = FALSE;
		}

		if( to_update_can_sensor_commu_idx_and_stepB )
			if( to_enable_can_transparent_modeB )
			{
				if( enable_can_transparent_mode_func != NULL )
				{
					// Flag can_transparent_modeB is set true.
					enable_can_transparent_mode_func();
					to_enable_can_transparent_modeB = FALSE;
				}
			}
			else
			{
				update_can_sensor_commu_idx_and_step();
				to_update_can_sensor_commu_idx_and_stepB = FALSE;
			}

		if( to_delay_can_sensor_commuB )
			if( TimeOutIntvElapsed() )
			{
				CancelTimeOutIntv();
				to_delay_can_sensor_commuB    = FALSE;
				to_continue_can_sensor_commuB = TRUE;
			}

		if( to_continue_can_sensor_commuB )
		{
			continue_can_sensor_communication();
			to_continue_can_sensor_commuB = FALSE;
		} 
	}

	if( !can_bus_off_state_detectedB && can_busoff() )
	{
		clear_can_transmit_buffer();

		disable_timer_and_timeout_jobs();
		Set_Runlevel(RUNLEVEL_COMM);
		can_bus_off_state_detectedB = TRUE;
		enable_timer_and_timeout_jobs();
	}

/*
#ifndef NO_LOGGER
	if( new_runlevel_log > 0 )
	{
		Log_RunLevel_Change(old_runlevel_log, new_runlevel_log);
		new_runlevel_log = 0;
	}
#endif
*/
}	// check_for_pending_can_irma_jobs


dword get_can_irma_addr_of_rec_uip_30_msg(void)
{
	dword can_irma_addr;

	can_irma_addr  =  (dword)uip_30_rec_buf[2] << 16;
	can_irma_addr += ((dword)uip_30_rec_buf[1] <<  8);
	can_irma_addr +=  (dword)uip_30_rec_buf[0];
	can_irma_addr &= 0x0007FFFFUL;

	return(can_irma_addr);
}	// get_can_irma_addr_of_rec_uip_30_msg


bool check_rec_uip_30_msg(byte service_lev, char msg_id, byte msg_ver, char sub_cmd, word *payload_len, byte **payload)
{
	bool resultB;

	resultB =      (uip_30_rec_buf[4] & 0x0F) == service_lev &&
			  (char)uip_30_rec_buf[7]         == msg_id      &&
			        uip_30_rec_buf[8]         == msg_ver     &&
			  (char)uip_30_rec_buf[9]         == sub_cmd;

	*payload_len = ((word)uip_30_rec_buf[6] << 8) + (word)uip_30_rec_buf[5] - 3;
	*payload = &uip_30_rec_buf[10];

	return(resultB);
}	// check_rec_uip_30_msg


// FALSE: CAN transparent mode enable flag unchanged.
//  TRUE: CAN transparent mode enable flag set.
bool set_can_transparent_mode_enable_flag(void)
{
	if( !can_transparent_modeB )
		to_enable_can_transparent_modeB = TRUE;

	return(!can_transparent_modeB);
}	// set_can_transparent_mode_enable_flag


// FALSE: CAN transparent mode already enabled.
//  TRUE: CAN transparent mode just enabled.
bool enable_can_transparent_mode(void)
{
	bool can_transparent_mode_oldB;

	can_transparent_mode_oldB = can_transparent_modeB;
	if( !can_transparent_modeB )
	{
		init_uip_30_receiving();

		can_transparent_modeB = TRUE;
	}

	return(!can_transparent_mode_oldB);
}	// enable_can_transparent_mode


bool disable_can_transparent_mode(void)
{
	bool can_transparent_mode_oldB;

	can_transparent_mode_oldB = can_transparent_modeB;
	can_transparent_modeB = FALSE;

	return(can_transparent_mode_oldB);
}	// disable_can_transparent_mode


//|-------------------------------------------------------------------------------------|
//| Function: route_uip_30_msg_from_can													|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Route UIP 2.0 over CAN IRMA frame (message identifier 0x75) stored in CAN receive	|
//| buffer to IRMA service device (transmit UIP 2.0 message 'A' 0x60 '*').				|
//|-------------------------------------------------------------------------------------|
void route_uip_30_msg_from_can(void)
{
	byte irma_can_msg_id, can_msg_no, can_data_len, can_data_idx, exp_can_data_len;
	byte *can_data_ptr;
	dword can_ident;
	byte uip_ver_service_level, uip_ver;
	word r_dl, a_60_payload_len, uip_30_residual_dl;
	bool uip_30_msg_completeB;

	if( exp_can_msg_no == 0 || !can_transparent_modeB )
		return;

	irma_can_msg_id = can_receive_buf[can_receive_buf_read + ID_HH_OFFSET] >> 1;
	can_data_len = can_receive_buf[can_receive_buf_read + LEN_OFFSET];

	if( irma_can_msg_id != UIP_20_RECV || can_data_len < 2 )
	{
		init_uip_30_receiving();
		return;
	}

    can_data_ptr = &can_receive_buf[can_receive_buf_read + DATA_OFFSET];
	can_msg_no = can_data_ptr[0];

	if( can_msg_no == exp_can_msg_no )
	{
		if( can_msg_no == 1 )
		{
			if( can_data_len < 7 )
			{
				init_uip_30_receiving();
				return;
			}

			can_ident  = (dword)can_receive_buf[can_receive_buf_read + ID_HH_OFFSET] << 21;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_HL_OFFSET] << 13;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_LH_OFFSET] << 5;
			can_ident |= (dword)can_receive_buf[can_receive_buf_read + ID_LL_OFFSET] >> 3;
			can_ident &= 0x003FFFFFUL;

			uip_ver_service_level = can_data_ptr[1];
			uip_ver = uip_ver_service_level & 0xF0;
			r_dl = (word)can_data_ptr[2] + ((word)can_data_ptr[3] << 8);
			uip_30_desired_dl = r_dl & 0x7FFF;

			if( uip_ver != 0x30 || uip_30_desired_dl < 3 || uip_30_desired_dl > 1393 )
			{
				init_uip_30_receiving();
				return;
			}

			a_60_payload_len = 7 + uip_30_desired_dl;
			// service level 2: service
			init_uip_send_buff_writing_20(2, 'A', 0x60, '*', a_60_payload_len);
			IRMASendBuff[IRMASendBuffWritePos++] = (byte)can_ident;
			IRMASendBuff[IRMASendBuffWritePos++] = (byte)(can_ident >> 8);
			IRMASendBuff[IRMASendBuffWritePos++] = (byte)(can_ident >> 16);
			IRMASendBuff[IRMASendBuffWritePos++] = 0;
			IRMASendBuff[IRMASendBuffWritePos++] = uip_ver_service_level;
			IRMASendBuff[IRMASendBuffWritePos++] = (byte)r_dl;
			IRMASendBuff[IRMASendBuffWritePos++] = (byte)(r_dl >> 8);
			IRMASendBuff[IRMASendBuffWritePos++] = can_data_ptr[4];
			IRMASendBuff[IRMASendBuffWritePos++] = can_data_ptr[5];
			IRMASendBuff[IRMASendBuffWritePos++] = can_data_ptr[6];
			if( can_data_len == MAX_CAN_DATA_LEN )
			{
				IRMASendBuff[IRMASendBuffWritePos++] = can_data_ptr[7];
				uip_30_current_dl = 4;
			}
			else
				uip_30_current_dl = 3;
			if( uip_30_desired_dl > 3 )
				exp_can_data_len = MAX_CAN_DATA_LEN;
			else
				exp_can_data_len = 7;
		}
		else
		{
			for( can_data_idx = 1; can_data_idx < can_data_len; can_data_idx++ ) 
				IRMASendBuff[IRMASendBuffWritePos++] = can_data_ptr[can_data_idx];
			uip_30_residual_dl = uip_30_desired_dl - uip_30_current_dl;
			if( uip_30_residual_dl >= 7 )
				exp_can_data_len = MAX_CAN_DATA_LEN;
			else
				exp_can_data_len = 1 + uip_30_residual_dl;
			uip_30_current_dl += (exp_can_data_len - 1);
		}

		if( can_data_len != exp_can_data_len )
		{
			init_uip_30_receiving();
			return;
		}

		uip_30_msg_completeB = uip_30_current_dl == uip_30_desired_dl;
		if( uip_30_msg_completeB )
		{
			init_uip_30_receiving();

			send_buffered_uip_20_frame(IrmaASrcAddr);
		}
		else
			exp_can_msg_no++;
	}
	else
	{
		init_uip_30_receiving();
		return;
	}
}	// route_uip_30_msg_from_can


// Remove all timer, time-out and run level jobs related to CAN IRMA communication.
void Remove_CAN_IRMA_Jobs(void)
{
	Remove_Runlevel_Job(check_for_pending_can_irma_jobs);
}	// Remove_CAN_IRMA_Jobs


void send_uip_30_msg_over_can(byte rec_class, can_addr_id_type addr_id, byte *uip_30_ptr)
{
	byte uip_ver_service_level;
	word r_dl, dl, uip30_len, payload_len;
	byte can_frame_cnt, can_frame_no, can_data_len, can_data_idx, no_of_uip_30_bytes;

	uip_ver_service_level = *uip_30_ptr++;

	r_dl = (word)*uip_30_ptr++ + ((word)*uip_30_ptr++ << 8);
	dl = r_dl & 0x7FFF;
	uip30_len = 3 + dl;
	payload_len = dl - UIP_20_MESSAGE_DATAOFFSET; 

	can_frame_cnt = (byte)(uip30_len / (MAX_CAN_DATA_LEN - 1));
	if( uip30_len > ((MAX_CAN_DATA_LEN - 1) * (word)can_frame_cnt) )
		can_frame_cnt++;

	for( can_frame_no = 1; can_frame_no <= can_frame_cnt; can_frame_no++ )
	{
		// Determine number of data bytes for next CAN message.	Consider message no.
		// as first CAN data byte.
		can_data_len = MIN((uip30_len + 1), MAX_CAN_DATA_LEN);
		no_of_uip_30_bytes = can_data_len - 1;

        can_transmit_buf[can_transmit_buf_write].id_hh   = (UIP_20_SEND << 1) + (rec_class >> 2);
        can_transmit_buf[can_transmit_buf_write].id_hl   = (rec_class << 6) + addr_id.high;
        can_transmit_buf[can_transmit_buf_write].id_lh   = addr_id.mid;
        can_transmit_buf[can_transmit_buf_write].id_ll   = addr_id.low;
		can_transmit_buf[can_transmit_buf_write].length  = can_data_len;
 		can_transmit_buf[can_transmit_buf_write].data[0] = can_frame_no;

		if( can_frame_no == 1 )
		{
	 		can_transmit_buf[can_transmit_buf_write].data[1] = uip_ver_service_level;
	 		can_transmit_buf[can_transmit_buf_write].data[2] = (byte)r_dl;
	 		can_transmit_buf[can_transmit_buf_write].data[3] = (byte)(r_dl >> 8);
	 		can_transmit_buf[can_transmit_buf_write].data[4] = *uip_30_ptr++;
	 		can_transmit_buf[can_transmit_buf_write].data[5] = *uip_30_ptr++;
	 		can_transmit_buf[can_transmit_buf_write].data[6] = *uip_30_ptr++;
			if( can_data_len == MAX_CAN_DATA_LEN )
		 		can_transmit_buf[can_transmit_buf_write].data[7] = *uip_30_ptr++;
		}
		else
			for( can_data_idx = 1; can_data_idx < can_data_len; can_data_idx++ ) 
				can_transmit_buf[can_transmit_buf_write].data[can_data_idx] = *uip_30_ptr++;

	    NEXT(can_transmit_buf_write, CAN_TRANSMIT_BUF_MSG_CNT);

		if( uip30_len > no_of_uip_30_bytes )
			uip30_len -= no_of_uip_30_bytes;
		else
			uip30_len = 0;
	}

	can_transmit();
}	// send_uip_30_msg_over_can


void route_uip_30_msg_to_can(byte *uip_30_ptr)
{
	dword can_ident;
	byte rec_class;
	can_addr_id_type addr_id;

	// Bits 24 - 28 may be ignored because they always have to be equal to 0.
	can_ident = (dword)*uip_30_ptr++ + ((dword)*uip_30_ptr++ << 8) + ((dword)*uip_30_ptr++ << 16);
	rec_class = (byte)((can_ident & 0x00380000UL) >> 19);
	can_ident <<= 3;
	addr_id.low  = (byte)can_ident;
	addr_id.mid  = (byte)(can_ident >> 8);
	addr_id.high = (byte)(can_ident >> 16 & 0x3F);

	// Adjust UIP v3.0 frame pointer to byte UIP_VER_LEV.
	uip_30_ptr += 1;
	send_uip_30_msg_over_can(rec_class, addr_id, uip_30_ptr);
}	// route_uip_30_msg_to_can
