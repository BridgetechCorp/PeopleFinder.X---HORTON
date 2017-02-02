/*==============================================================================================*
 |       Filename: can_irma_dist500.c                                                           |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Extended functions for CAN IRMA communication with DIST500 sensors.          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifndef GATEWAY
	#error "File can_irma_dist500.c included but symbol GATEWAY not defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167cr.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "opera_interface.h"
#include "kernel_interface.h"
#include "can_irma.h"
#include "can_irma_embedding.h"
#include "can_irma_dist500.h"
#include "communication.h"
#include "communication_gdist500.h"
#include "uip_gdist500.h"
//#ifndef NO_LOGGER
//	#include "logger_gdist500.h"
//#endif

extern int (*RunLevel5Main)(void);

/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	byte list[CAN_FA_LIST_LEN];
} function_area_address_list_type;


typedef struct
{
	byte list[CAN_SENSOR_LIST_LEN];
} uip_address_list_type;


/*----- Memory Class Assignments ---------------------------------------------------------------*/
//#pragma class CO=COMM
//#pragma class PR=COMM
//#pragma class FB=COMMRAM
//#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
boolean use_static_can_sensor_addrsB;
byte can_sensor_commu_step;
byte can_sensor_commu_idx;
boolean can_sensor_commu_cycle_failedB;
byte can_sensor_regular_commu_step;
boolean extended_sensor_commu_requiredB;
boolean first_can_sensor_ext_commu_stepB;
boolean can_sensor_static_data_changedB;

word can_sensor_commu_cycle_start_delay;
dword can_sensor_commu_cycle_cnt;
dword can_sensor_commu_cycle_error_cnt;
boolean first_can_sensor_commu_cycle_startedB;

byte can_sensor_cnt;
byte sorted_can_sensor_cnt;
byte can_sensor_cnt_backup;
can_sensor_data_type can_sensor_list[CAN_SENSOR_LIST_LEN];
can_sensor_data_type sorted_can_sensor_list[CAN_SENSOR_LIST_LEN];
can_sensor_data_type can_sensor_list_backup[CAN_SENSOR_LIST_LEN];

byte function_area_cnt;
function_area_data_type function_area_list[CAN_FA_LIST_LEN];

byte door_cnt;
door_data_type door_list[DOOR_CNT_MAX];


/*----- Function Prototypes --------------------------------------------------------------------*/
void timeout_job_can_sensor_scan(void);
void timeout_job_can_sensor_query(void);

boolean update_can_sensor_list(void);
void update_can_sensor_list_backup(void);
void update_sorted_can_sensor_list(void);

byte get_idx_of_can_sensor_in_list(can_addr_id_type sensor_addr_id);
byte get_idx_of_can_sensor_in_backup_list(dword sensor_addr);

boolean update_static_can_sensor_data(void);
boolean get_is_gateway_sensor_flag(byte sen_idx);
void check_can_sensor_list(void);
void update_function_area_and_door_list(void);

byte get_sensor_door_assignments(byte sen_idx, word *fa_addr, word *door_addr, byte *door_idx);
byte get_sensor_fa_assignments(byte sen_idx, word *fa_addr, word *door_addr, byte *fa_idx);

boolean update_counting_data_category_params(void);

boolean sensor_door_state_setting_confirmed(void);
boolean sensor_door_closing_before_reset_confirmed(void);
boolean sensor_status_received(void);
boolean sensor_fa_status_received(void);
boolean sensor_counting_data_received(void);
boolean sensor_counting_data_set_confirmed(void);
boolean sensor_counting_result_reset_confirmed(void);
boolean sensor_firmware_reset_confirmed(void);

void update_obc_interface_data(void);
void update_desired_data_of_can_sensor_list_backup(void);


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
// Initialize global variables not properly initialized by function "clear_comm_RAM".
void init_global_var_can_irma_dist500(void)
{
}	// init_global_var_can_irma_dist500


//|-------------------------------------------------------------------------------------|
//| Function: send_startup_msg															|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Send CAN IRMA Analyzer startup message (message identifier 0x1F) to all CAN IRMA	|
//| devices.																			|
//|-------------------------------------------------------------------------------------|
void send_can_irma_startup_msg(void)
{
	byte CAN_Msg_Data[MAX_CAN_DATA_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

	CAN_Msg_Data[0] = (byte)(RunLevel5Main != NULL);

	send_can_irma_msg(CAN_Msg_Data, MAX_CAN_DATA_LEN, ANA_STARTUP_NOT, TO_ALL, analyzer_addr_id);
}	// send_can_irma_startup_msg


boolean to_query_counting_data_category_params(can_addr_id_type sensor_addr_id)
{
	byte sen_idx, sensor_fa_cnt, fa_idx = 0;
	word fa_addr = 0, door_addr = 0;
	boolean resultB;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_fa_cnt = get_sensor_fa_assignments(sen_idx, &fa_addr, &door_addr, &fa_idx);

	resultB = get_is_gateway_sensor_flag(sen_idx);
	resultB = resultB && !function_area_list[fa_idx].counting_data_category_params_queriedB;

	return(resultB);
}	// to_query_counting_data_category_params


void send_uip_30_msg_to_can_sensors(byte *uip_30_ptr)
{
	send_uip_30_msg_over_can(ALL_SENSORS, analyzer_addr_id, uip_30_ptr);
}	// send_uip_30_msg_to_can_sensors


void send_uip_30_msg_to_can_sensor(can_addr_id_type sensor_addr_id, byte *uip_30_ptr)
{
	send_uip_30_msg_over_can(SENSOR, sensor_addr_id, uip_30_ptr);
}	// send_uip_30_msg_to_can_sensor


void send_c_10_b_to_can_sensors(void)
{
	byte uip_30_buf[6] = { 0x31, 0x03, 0x00, 'c', 0x10, 'b' };
	byte *uip_30_ptr = uip_30_buf;

	send_uip_30_msg_to_can_sensors(uip_30_ptr);
}	// send_c_10_b_to_can_sensors


void send_K_10_g_to_can_sensor(can_addr_id_type sensor_addr_id, word group_id, word param_id)
{
	byte uip_30_buf[10] = { 0x32, 0x07, 0x00, 'K', 0x10, 'g' };
	byte *uip_30_ptr = uip_30_buf;

	uip_30_buf[6] = (byte)group_id;
	uip_30_buf[7] = (byte)(group_id >> 8);
	uip_30_buf[8] = (byte)param_id;
	uip_30_buf[9] = (byte)(param_id >> 8);

	send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_K_10_g_to_can_sensor


void send_D_30_s_to_can_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[6 + 1 + 6 * DOOR_CNT_MAX] = { 0x33, 0x00, 0x00, 'D', 0x30, 's' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_door_cnt, sensor_door_idx, door_idx = 0, door_signal_idx;
	byte left_wing_opening_degree, right_wing_opening_degree;
	word r_dl, fa_addr = 0, door_addr = 0;
	boolean door_closedB = FALSE;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_door_cnt = get_sensor_door_assignments(sen_idx, &fa_addr, &door_addr, &door_idx);

	r_dl = 3 + 1 + 6 * sensor_door_cnt;
	uip_30_buf[1] = (byte)r_dl;
	uip_30_buf[2] = (byte)(r_dl >> 8);

	uip_30_buf[6] = sensor_door_cnt;
	for( sensor_door_idx = 0; sensor_door_idx < sensor_door_cnt; sensor_door_idx++ )
	{
		if( can_sensor_list[sen_idx].use_desired_door_stateB )
		   	door_closedB = can_sensor_list[sen_idx].door_closed_desiredB;
		else
		{
			door_signal_idx = (byte)door_addr - 1;
		   	door_closedB = door_signal_interface.closed_flagsB[door_signal_idx];
		}

		if( door_closedB )
		{
			left_wing_opening_degree  = 0;
			right_wing_opening_degree = 0;
		}
		else
		{
			left_wing_opening_degree  = 100;
			right_wing_opening_degree = 100;
		}

		uip_30_buf[7  + 6 * sensor_door_idx] = (byte)fa_addr;
		uip_30_buf[8  + 6 * sensor_door_idx] = (byte)(fa_addr >> 8);
		uip_30_buf[9  + 6 * sensor_door_idx] = (byte)door_addr;
		uip_30_buf[10 + 6 * sensor_door_idx] = (byte)(door_addr >> 8);
		uip_30_buf[11 + 6 * sensor_door_idx] = left_wing_opening_degree;
		uip_30_buf[12 + 6 * sensor_door_idx] = right_wing_opening_degree;
	}

	if( sensor_door_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_D_30_s_to_can_sensor


void send_D_30_s_closed_to_can_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[6 + 1 + 6 * DOOR_CNT_MAX] = { 0x33, 0x00, 0x00, 'D', 0x30, 's' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_door_cnt, sensor_door_idx, door_idx = 0;
	byte left_wing_opening_degree, right_wing_opening_degree;
	word r_dl, fa_addr = 0, door_addr = 0;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_door_cnt = get_sensor_door_assignments(sen_idx, &fa_addr, &door_addr, &door_idx);

	r_dl = 3 + 1 + 6 * sensor_door_cnt;
	uip_30_buf[1] = (byte)r_dl;
	uip_30_buf[2] = (byte)(r_dl >> 8);

	uip_30_buf[6] = sensor_door_cnt;
	for( sensor_door_idx = 0; sensor_door_idx < sensor_door_cnt; sensor_door_idx++ )
	{
		left_wing_opening_degree  = 0;
		right_wing_opening_degree = 0;

		uip_30_buf[7  + 6 * sensor_door_idx] = (byte)fa_addr;
		uip_30_buf[8  + 6 * sensor_door_idx] = (byte)(fa_addr >> 8);
		uip_30_buf[9  + 6 * sensor_door_idx] = (byte)door_addr;
		uip_30_buf[10 + 6 * sensor_door_idx] = (byte)(door_addr >> 8);
		uip_30_buf[11 + 6 * sensor_door_idx] = left_wing_opening_degree;
		uip_30_buf[12 + 6 * sensor_door_idx] = right_wing_opening_degree;
	}

	if( sensor_door_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_D_30_s_closed_to_can_sensor


void send_S_31_g_to_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[6] = { 0x33, 0x03, 0x00, 'S', 0x31, 'g' };
	byte *uip_30_ptr = uip_30_buf;

	send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_S_31_g_to_sensor


void send_S_30_g_to_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[6] = { 0x33, 0x03, 0x00, 'S', 0x30, 'g' };
	byte *uip_30_ptr = uip_30_buf;

	send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_S_30_g_to_sensor


void send_C_60_c_to_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[8] = { 0x33, 0x05, 0x00, 'C', 0x60, 'c' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_fa_cnt, fa_idx = 0;
	word fa_addr = 0, door_addr = 0;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_fa_cnt = get_sensor_fa_assignments(sen_idx, &fa_addr, &door_addr, &fa_idx);

	uip_30_buf[6] = (byte)fa_addr;
	uip_30_buf[7] = (byte)(fa_addr >> 8);

	if( sensor_fa_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_C_60_c_to_sensor


void send_C_60_g_to_sensor(can_addr_id_type sensor_addr_id, char type_of_counting_data)
{
	byte uip_30_buf[11] = { 0x33, 0x08, 0x00, 'C', 0x60, 'g' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_fa_cnt, fa_idx = 0;
	word fa_addr = 0, door_addr = 0;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_fa_cnt = get_sensor_fa_assignments(sen_idx, &fa_addr, &door_addr, &fa_idx);

	uip_30_buf[6]  = (byte)fa_addr;
	uip_30_buf[7]  = (byte)(fa_addr >> 8);
	uip_30_buf[8]  = 0x00;							// reset flag
	uip_30_buf[9]  = (byte)type_of_counting_data;	// 'C' = current counter state, 'B' = buffered counting result
	uip_30_buf[10] = 0xFF;							// category, 0xFF = all

	if( sensor_fa_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_C_60_g_to_sensor


void send_C_60_r_to_sensor(can_addr_id_type sensor_addr_id, char type_of_counting_data)
{
	byte uip_30_buf[9] = { 0x33, 0x06, 0x00, 'C', 0x60, 'r' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_fa_cnt, fa_idx = 0;
	word fa_addr = 0, door_addr = 0;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_fa_cnt = get_sensor_fa_assignments(sen_idx, &fa_addr, &door_addr, &fa_idx);

	uip_30_buf[6] = (byte)fa_addr;
	uip_30_buf[7] = (byte)(fa_addr >> 8);
	uip_30_buf[8] = (byte)type_of_counting_data;	// 'C' = current counter state, 'B' = buffered counting result

	if( sensor_fa_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_C_60_r_to_sensor


void send_C_60_s_to_sensor(can_addr_id_type sensor_addr_id, char type_of_counting_data, boolean counting_finishedB)
{
	byte uip_30_buf[6 + 5 + 5 * COUNTING_DATA_CATEGORY_CNT_MAX] = { 0x33, 0x00, 0x00, 'C', 0x60, 's' };
	byte *uip_30_ptr = uip_30_buf;
	byte sen_idx, sensor_fa_cnt, fa_idx = 0;
	word r_dl, fa_addr = 0, door_addr = 0;
	byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;

	sen_idx = get_idx_of_can_sensor_in_list(sensor_addr_id);
	sensor_fa_cnt = get_sensor_fa_assignments(sen_idx, &fa_addr, &door_addr, &fa_idx);
	counting_data_category_cnt = can_sensor_list[sen_idx].counting_data_buffer_desired.counting_data_category_cnt;

	r_dl = 3 + 5 + 5 * counting_data_category_cnt;
	uip_30_buf[1] = (byte)r_dl;
	uip_30_buf[2] = (byte)(r_dl >> 8);

	uip_30_buf[6]  = (byte)fa_addr;
	uip_30_buf[7]  = (byte)(fa_addr >> 8);
	uip_30_buf[8]  = (byte)type_of_counting_data;	// 'C' = current counter state, 'B' = buffered counting result
	uip_30_buf[9]  = (byte)counting_finishedB;
	uip_30_buf[10] = counting_data_category_cnt;
	for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
	{
		category_id = can_sensor_list[sen_idx].counting_data_buffer_desired.counting_data[counting_data_category_idx].category_id;
		in          = can_sensor_list[sen_idx].counting_data_buffer_desired.counting_data[counting_data_category_idx].in;
		out         = can_sensor_list[sen_idx].counting_data_buffer_desired.counting_data[counting_data_category_idx].out;

		uip_30_buf[11 + 5 * counting_data_category_idx] = category_id;
		uip_30_buf[12 + 5 * counting_data_category_idx] = (byte)in;
		uip_30_buf[13 + 5 * counting_data_category_idx] = (byte)(in >> 8);
		uip_30_buf[14 + 5 * counting_data_category_idx] = (byte)out;
		uip_30_buf[15 + 5 * counting_data_category_idx] = (byte)(out >> 8);
	}

	if( sensor_fa_cnt > 0 )
		send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_C_60_s_to_sensor


void send_W_10_r_to_sensor(can_addr_id_type sensor_addr_id)
{
	byte uip_30_buf[11] = { 0x32, 0x03, 0x00, 'W', 0x10, 'r' };
	byte *uip_30_ptr = uip_30_buf;

	send_uip_30_msg_to_can_sensor(sensor_addr_id, uip_30_ptr);
}	// send_W_10_r_to_sensor


void init_can_sensor_control_vars(can_sensor_data_type *can_sensor_data)
{
	// used for communication control
	can_sensor_data->last_commu_step                = LAST_CAN_SENSOR_REGULAR_COMMU_STEP;
	can_sensor_data->new_counting_result_availableB = FALSE;
	can_sensor_data->commu_cycle_failedB            = FALSE;
	can_sensor_data->error_msg_receivedB            = FALSE; 
	can_sensor_data->to_init_can_sensor_dataB       = FALSE;
}	// init_can_sensor_control_vars


void init_can_sensor_data(can_sensor_data_type *can_sensor_data)
{
	memset(can_sensor_data, 0, sizeof(*can_sensor_data));

	// Value 0xFF indicates, that no function area is assigned.
	can_sensor_data->fa_list_idx = 0xFF;

	init_can_sensor_control_vars(can_sensor_data);
}	// init_can_sensor_data


void init_can_sensor_list(void)
{
	byte sen_idx;

	can_sensor_cnt = 0;
	for( sen_idx = 0; sen_idx < CAN_SENSOR_LIST_LEN; sen_idx++ )
		init_can_sensor_data(&can_sensor_list[sen_idx]);
}	// init_can_sensor_list


void init_can_sensor_list_backup(void)
{
	byte sen_idx;

	can_sensor_cnt_backup = 0;
	for( sen_idx = 0; sen_idx < CAN_SENSOR_LIST_LEN; sen_idx++ )
		init_can_sensor_data(&can_sensor_list_backup[sen_idx]);
}	// init_can_sensor_list_backup


void fill_can_sensor_list_with_static_addrs(void)
{
	byte sen_idx;

	for( sen_idx = 0; sen_idx < CAN_SENSOR_LIST_LEN; sen_idx++ )
	{
//		can_sensor_list[sen_idx].addr                 = address stored in Flash EPROM sector 5;
		can_sensor_list[sen_idx].static_data_knownB   = FALSE;
		can_sensor_list[sen_idx].static_data_validB   = FALSE;
		can_sensor_list[sen_idx].firmware_mismatchedB = FALSE;
	}
//	can_sensor_cnt = sensor count stored in Flash EPROM sector 5;
}	// fill_can_sensor_list_with_static_addrs


void clear_dynamic_data_errors_of_all_function_areas(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
		function_area_list[fa_idx].dynamic_data_validB = TRUE;
}	// clear_dynamic_data_errors_of_all_function_areas


void start_next_can_sensor_commu_cycle(void)
{
	if( use_static_can_sensor_addrsB )
		// Sensor addresses are static. Skip sensor scan.
		can_sensor_commu_step = 0x01;
	else
	{
		if( first_can_sensor_commu_cycle_startedB )
		{
			update_sorted_can_sensor_list();

			update_can_sensor_list_backup();
		}

		// Sensor addresses are dynamic. Perform sensor scan at first.
		can_sensor_commu_step = 0x00;

		init_can_sensor_list();
	}
	
	if( first_can_sensor_commu_cycle_startedB )
	{
		update_obc_interface_data();

		update_desired_data_of_can_sensor_list_backup();
	}

	can_sensor_commu_idx = 0;
	
	to_update_can_sensor_commu_idx_and_stepB = FALSE;
	to_delay_can_sensor_commuB = FALSE;
	to_continue_can_sensor_commuB = FALSE;
	can_sensor_commu_cycle_failedB = FALSE;
	can_sensor_regular_commu_step = 0;
	extended_sensor_commu_requiredB = FALSE;
	first_can_sensor_ext_commu_stepB = FALSE;
	can_sensor_static_data_changedB = FALSE;

	clear_dynamic_data_errors_of_all_function_areas();

	if( first_can_sensor_commu_cycle_startedB )
	{
		can_sensor_commu_cycle_cnt++;
		if( can_sensor_commu_cycle_start_delay > 0 )
		{
			to_delay_can_sensor_commuB = TRUE;
			StartTimeOutIntv(can_sensor_commu_cycle_start_delay);
		}
		else
			to_continue_can_sensor_commuB = TRUE;
	}
	else
		to_continue_can_sensor_commuB = TRUE;
	first_can_sensor_commu_cycle_startedB = TRUE;
	
	init_uip_30_receiving();
}	// start_next_can_sensor_commu_cycle


void init_can_sensor_ext_commu(void)
{
	can_sensor_regular_commu_step = can_sensor_commu_step;
	extended_sensor_commu_requiredB = TRUE;
	first_can_sensor_ext_commu_stepB = TRUE;
}	// init_can_sensor_ext_commu


void start_can_sensor_commu(void)
{
	can_sensor_commu_cycle_start_delay = 0;

	can_sensor_commu_cycle_cnt       = 0;
	can_sensor_commu_cycle_error_cnt = 0;
	first_can_sensor_commu_cycle_startedB = FALSE;

	memset(function_area_list,         0, sizeof(function_area_list));
	memset(door_list,                  0, sizeof(door_list));
	memset(sorted_can_sensor_list,     0, sizeof(sorted_can_sensor_list));
	memset(&obc_interface_data,        0, sizeof(obc_interface_data));
	memset(&obc_interface_data_backup, 0, sizeof(obc_interface_data_backup));

	if( use_static_can_sensor_addrsB )
		// Sensor addresses are static. Fill CAN sensor list with static sensor addresses.
		fill_can_sensor_list_with_static_addrs();
	else
		// Sensor addresses are dynamic. Initialize CAN sensor list.
		init_can_sensor_list_backup();
	
	start_next_can_sensor_commu_cycle();
}	// start_can_sensor_commu


void handle_uip_30_receive_error(void)
{
	can_sensor_commu_cycle_failedB = TRUE;
	// Ensure processing of flag can_sensor_commu_cycle_failedB by function
	// update_can_sensor_commu_idx_and_step.
	to_update_can_sensor_commu_idx_and_stepB = TRUE;

	init_uip_30_receiving();
}	// handle_uip_30_receive_error


void set_commu_cycle_failed_flag_of_can_sensor(byte sen_idx)
{
	if( sen_idx < can_sensor_cnt )
	{
		can_sensor_list[sen_idx].commu_cycle_failedB      = TRUE;
		can_sensor_list[sen_idx].to_init_can_sensor_dataB = TRUE;
	}
}	// set_commu_cycle_failed_flag_of_can_sensor


void set_error_msg_receive_flag_of_can_sensor(byte sen_idx)
{
	if( sen_idx < can_sensor_cnt )
	{
		can_sensor_list[sen_idx].error_msg_receivedB      = TRUE;
		can_sensor_list[sen_idx].to_init_can_sensor_dataB = TRUE;
	}
}	// set_error_msg_receive_flag_of_can_sensor


void clear_dynamic_data_valid_flag_of_function_area(byte fa_idx)
{
	if( fa_idx < function_area_cnt )
		function_area_list[fa_idx].dynamic_data_validB = FALSE;
}	// clear_dynamic_data_valid_flag_of_function_area


void eval_can_sensor_response(void)
{
	remove_timeout_job(timeout_job_can_sensor_query);

	if( !can_sensor_commu_cycle_failedB )
		switch( can_sensor_commu_step )
		{
			case 0x00 :	update_can_sensor_list();
						break;

			case 0x01 :
			case 0x02 :
			case 0x03 :
			case 0x04 :
			case 0x05 :
			case 0x06 :
			case 0x07 :
			case 0x08 :
			case 0x09 :
			case 0x0A :
			case 0x0B :
			case 0x0C :
			case 0x0D :
			case 0x0E :
			case 0x0F :
			case 0x10 :
			case 0x11 :
			case 0x12 :
			case 0x13 :
			case 0x14 :
			case 0x15 :
			case 0x16 :
			case 0x17 :
			case 0x18 :	can_sensor_commu_cycle_failedB = !update_static_can_sensor_data();
						break;

			case 0x19 :	can_sensor_commu_cycle_failedB = !update_counting_data_category_params();
						break;

			case 0x1A :	can_sensor_commu_cycle_failedB = !sensor_door_state_setting_confirmed();
						break;

			case 0x1B :	can_sensor_commu_cycle_failedB = !sensor_status_received();
						break;

			case 0x1C :	can_sensor_commu_cycle_failedB = !sensor_fa_status_received();
						break;

			case 0x1D :	if( can_sensor_list[can_sensor_commu_idx].counting_data_buffer_desired_activeB )
							can_sensor_commu_cycle_failedB = !sensor_counting_data_set_confirmed();
						else
							can_sensor_commu_cycle_failedB = !sensor_counting_data_received();
						break;

			case 0x1E :	can_sensor_commu_cycle_failedB = !sensor_counting_result_reset_confirmed();
						break;

			case 0x1F :	can_sensor_commu_cycle_failedB = !sensor_door_closing_before_reset_confirmed();
						break;

			case 0x20 :	can_sensor_commu_cycle_failedB = !sensor_firmware_reset_confirmed();
						break;
		}	//	end of "switch( can_sensor_commu_step )"

	if( can_sensor_commu_cycle_failedB )
		// Ensure processing of flag can_sensor_commu_cycle_failedB by function
		// update_can_sensor_commu_idx_and_step.
		to_update_can_sensor_commu_idx_and_stepB = TRUE;
}	// eval_can_sensor_response


void continue_can_sensor_communication(void)
{
	can_addr_id_type sensor_addr_id;
	boolean static_data_knownB, is_gateway_sensorB, new_counting_result_availableB;
	boolean sensor_firmware_reset_requiredB;

	if( can_sensor_list[can_sensor_commu_idx].commu_cycle_failedB )
	{
		to_update_can_sensor_commu_idx_and_stepB = TRUE;
		return;
	}

	sensor_addr_id = addr_to_can_addr_id(can_sensor_list[can_sensor_commu_idx].addr);
	static_data_knownB = can_sensor_list[can_sensor_commu_idx].static_data_knownB;
	is_gateway_sensorB = get_is_gateway_sensor_flag(can_sensor_commu_idx);
	sensor_firmware_reset_requiredB = can_sensor_list[can_sensor_commu_idx].error_msg_receivedB;

	new_counting_result_availableB = can_sensor_list[can_sensor_commu_idx].new_counting_result_availableB;

	switch( can_sensor_commu_step )
	{
		case 0x00 :	send_c_10_b_to_can_sensors();
					add_timeout_job(100, timeout_job_can_sensor_scan);
					break;

		case 0x01 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query product name, group id 0x100, param id 0x0000
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0100, 0x0000);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x02 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query device number, group id 0x100, param id 0x0002
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0100, 0x0002);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x03 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query production date, group id 0x200, param id 0x0000
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0200, 0x0000);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x04 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query communication module name, group id 0x300, param id 0x0000
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0000);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x05 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query communication module version, group id 0x300, param id 0x0001
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0001);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x06 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query communication module timestamp, group id 0x300, param id 0x0002
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0002);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x07 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query application module name, group id 0x300, param id 0x0100
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0100);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x08 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query application module version, group id 0x300, param id 0x0101
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0101);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x09 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query application module timestamp, group id 0x300, param id 0x0102
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0102);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0A :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query FPGA bitfile name 1, group id 0x300, param id 0x0200
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0200);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0B :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query FPGA bitfile version 1, group id 0x300, param id 0x0201
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0201);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0C :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query FPGA bitfile timestamp 1, group id 0x300, param id 0x0202
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0202);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0D :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query active application module name, group id 0x300, param id 0x0500
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0500);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0E :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query active application module version, group id 0x300, param id 0x0501
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0501);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x0F :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query active FPGA bitfile version, group id 0x300, param id 0x0502
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0300, 0x0502);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x10 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query sensor count, group id 0x500, param id 0x0300
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0500, 0x0300);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x11 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query own UIP address, group id 0x500, param id 0x0400
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0500, 0x0400);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x12 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query gateway UIP address, group id 0x500, param id 0x0402
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0500, 0x0402);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x13 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query function area count, group id 0x700, param id 0x0000
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0700, 0x0000);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x14 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query function area address 1, group id 0x700, param id 0x0100
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0700, 0x0100);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;


		case 0x15 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query door count, group id 0x800, param id 0x0000
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0800, 0x0000);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x16 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query door address 1, group id 0x800, param id 0x0100
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0800, 0x0100);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x17 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query door delay 1, group id 0x800, param id 0x0101
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0800, 0x0101);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x18 :	if( static_data_knownB )
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					else
					{
						// query door logic 1, group id 0x800, param id 0x0102
						send_K_10_g_to_can_sensor(sensor_addr_id, 0x0800, 0x0102);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					break;

		case 0x19 :	if( to_query_counting_data_category_params(sensor_addr_id) )
					{
						// query counting data category parameters
						send_C_60_c_to_sensor(sensor_addr_id);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;

		case 0x1A :	if( is_gateway_sensorB )
					{
						// update states of doors assigned to sensor
						send_D_30_s_to_can_sensor(sensor_addr_id);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;

		case 0x1B :	// query sensor status
					send_S_30_g_to_sensor(sensor_addr_id);
					add_timeout_job(100, timeout_job_can_sensor_query);
					break;

		case 0x1C :	if( is_gateway_sensorB )
					{
						// query status of function areas assigned to sensor
						send_S_31_g_to_sensor(sensor_addr_id);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;
		
		case 0x1D :	if( is_gateway_sensorB )
						if( new_counting_result_availableB )
						{
							// query counting result
							send_C_60_g_to_sensor(sensor_addr_id, 'B');
							// Take delay of 1s for error message into account.
							add_timeout_job(1100, timeout_job_can_sensor_query);
						}
						else
							if( can_sensor_list[can_sensor_commu_idx].counting_data_buffer_desired_activeB )
							{
								// set current counter state
								send_C_60_s_to_sensor(sensor_addr_id, 'C', FALSE);
								// Take delay of 1s for error message into account.
								add_timeout_job(1100, timeout_job_can_sensor_query);
							}
							else
							{
								// query current counter state
								send_C_60_g_to_sensor(sensor_addr_id, 'C');
								// Take delay of 1s for error message into account.
								add_timeout_job(1100, timeout_job_can_sensor_query);
							}
					else
					{
						// workaround: query counting data of slave	to detect possible
						// errornous state of sensor firmware
						send_C_60_g_to_sensor(sensor_addr_id, 'C');
						// Take delay of 1s for error message into account.
						add_timeout_job(1100, timeout_job_can_sensor_query);
					}
					break;

		case 0x1E :	if( is_gateway_sensorB && new_counting_result_availableB )
					{
						// reset counting result
						send_C_60_r_to_sensor(sensor_addr_id, 'B');
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;

		case 0x1F :	if( sensor_firmware_reset_requiredB )
					{
						// set door state to closed to prepare sensor firmware reset
						send_D_30_s_closed_to_can_sensor(sensor_addr_id);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;

		case 0x20 :	if( sensor_firmware_reset_requiredB )
					{
						// reset sensor firmware
						send_W_10_r_to_sensor(sensor_addr_id);
						add_timeout_job(100, timeout_job_can_sensor_query);
					}
					else
						to_update_can_sensor_commu_idx_and_stepB = TRUE;
					break;
	}	//	end of "switch( can_sensor_commu_step )"
}	// continue_can_sensor_communication


boolean sensor_found_in_list(dword sensor_addr)
{
	byte sen_idx;

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
		if( can_sensor_list[sen_idx].addr == sensor_addr )
			return(TRUE);

	return(FALSE);
}	// sensor_found_in_list


void enable_update_of_static_data_of_can_sensor(byte sen_idx_backup)
{
	can_sensor_list_backup[sen_idx_backup].static_data_knownB   = FALSE;
	can_sensor_list_backup[sen_idx_backup].static_data_validB   = FALSE;
	can_sensor_list_backup[sen_idx_backup].firmware_mismatchedB = FALSE;

	// Cancel update of desired data of CAN sensors possibly pending.
	can_sensor_list_backup[sen_idx_backup].door_closed_desired_activeB    = FALSE;
	can_sensor_list_backup[sen_idx_backup].door_closed_desired_confirmedB = FALSE;
	can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired_activeB    = FALSE;
	can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired_confirmedB = FALSE;
}	// enable_update_of_static_data_of_can_sensor


void check_for_missing_sensors(void)
{
	byte sen_idx, sen_idx_backup;
	dword sensor_addr;
	boolean missingB;

	for( sen_idx = 0; sen_idx < obc_interface_data.sensor_cnt; sen_idx++ )
	{
		sensor_addr = obc_interface_data.sensor_list[sen_idx].addr;

		missingB = !sensor_found_in_list(sensor_addr);
		if( missingB )
		{
			obc_interface_data.sensor_list[sen_idx].ever_commu_cycle_failedB = TRUE;

			// Ensure query of static data if sensor is connected again.
			sen_idx_backup = get_idx_of_can_sensor_in_backup_list(sensor_addr);
			if( sen_idx_backup < can_sensor_cnt_backup )
				enable_update_of_static_data_of_can_sensor(sen_idx_backup);
		}
	}
}	// check_for_missing_sensors


void update_can_sensor_commu_idx_and_step(void)
{
	boolean communication_cycle_finishedB = FALSE;

	if( can_sensor_commu_cycle_failedB )
	{
		// Communication cycle is not continued for sensor causing communication failure.
		// As a result communication cycle is completely aborted, if failure affects all sensors.
		set_commu_cycle_failed_flag_of_can_sensor(can_sensor_commu_idx);

		can_sensor_commu_cycle_error_cnt++;
		// Reset global flag to ensure that corresponding sensor specific flag is set for affected
		// sensor only.
		can_sensor_commu_cycle_failedB = FALSE;
	}

	if( can_sensor_commu_step == 0 )
	{
		// Communication cycle failed flag is set for missing sensor. If disconnected or switched-off
		// at the right moment, communication to this sensor did not fail. 
		check_for_missing_sensors();

		// Sensor scan finished. Start query of sensor data, if at least one sensor was found.
		// Repeat sensor scan otherwise.
		if( can_sensor_cnt > 0 )
		{
			can_sensor_commu_step++;
			to_continue_can_sensor_commuB = TRUE;
		}
		else
		{
			// Always flag first_can_sensor_commu_cycle_startedB == true at this point.
			can_sensor_commu_cycle_cnt++;
			can_sensor_commu_cycle_error_cnt++;
			if( can_sensor_commu_cycle_start_delay > 0 )
			{
				to_delay_can_sensor_commuB = TRUE;
				StartTimeOutIntv(can_sensor_commu_cycle_start_delay);
			}
			else
				to_continue_can_sensor_commuB = TRUE;
		}

		return;
	}
	
	if( can_sensor_static_data_changedB && can_sensor_commu_step >= LAST_CAN_SENSOR_STATIC_DATA_QUERY_STEP )
	{
		if( !can_sensor_list[can_sensor_commu_idx].commu_cycle_failedB && !can_sensor_list[can_sensor_commu_idx].error_msg_receivedB )
			can_sensor_list[can_sensor_commu_idx].static_data_knownB = TRUE;

		if( can_sensor_commu_idx + 1 >= can_sensor_cnt )
		{
			// Check static data of sensors. Set static data valid flag of sensors accordingly.
			// Afterwards functions like get_is_gateway_sensor_flag may be used for the rest of this
			// communication cycle and for all following communication cycle with
			// can_sensor_static_data_changedB == FALSE.
			check_can_sensor_list();

			update_function_area_and_door_list();

			// Reset flag to ensure that above program sequence is not processed repeatedly.
			can_sensor_static_data_changedB = FALSE;
		}
	}

	// Regular communication:
	// Communication step done for all sensors in the order of appearance in sensor list. This is
	// necessary to minimize delay for update of sensor door state.
	// Extended communication:
	// Extended communication is completed for certain sensor before changing to next sensor. This
	// is necessary to minimize delay between query and reset of buffered counting result. Refer
	// to comments regarding implementation of function send_C_60_g_to_sensor.

	if( extended_sensor_commu_requiredB )
	{
		if( first_can_sensor_ext_commu_stepB )
		{
			if(	can_sensor_list[can_sensor_commu_idx].new_counting_result_availableB )
			{
				can_sensor_commu_step++;
				can_sensor_list[can_sensor_commu_idx].last_commu_step = LAST_CAN_SENSOR_REGULAR_COMMU_STEP + 1;
			}
			if(	can_sensor_list[can_sensor_commu_idx].error_msg_receivedB )
			{
				can_sensor_commu_step = LAST_CAN_SENSOR_COMMU_STEP - 1;
				can_sensor_list[can_sensor_commu_idx].last_commu_step = LAST_CAN_SENSOR_COMMU_STEP;
			}
			first_can_sensor_ext_commu_stepB = FALSE;
		}
		else
		{
			can_sensor_commu_step++;
			if( can_sensor_commu_step > can_sensor_list[can_sensor_commu_idx].last_commu_step )
			{
				extended_sensor_commu_requiredB = FALSE;
				can_sensor_list[can_sensor_commu_idx].last_commu_step = LAST_CAN_SENSOR_REGULAR_COMMU_STEP;
				can_sensor_commu_step = can_sensor_regular_commu_step;
			}
		}
	}

	if( !extended_sensor_commu_requiredB )
	{
		can_sensor_commu_idx++;
		if( can_sensor_commu_idx >= can_sensor_cnt )
		{
			can_sensor_commu_idx = 0;
			
			can_sensor_commu_step++;
			if( can_sensor_commu_step > LAST_CAN_SENSOR_REGULAR_COMMU_STEP )
				communication_cycle_finishedB = TRUE;
		}
	}

	if( communication_cycle_finishedB )
		start_next_can_sensor_commu_cycle();

	to_continue_can_sensor_commuB = TRUE;
}	// update_can_sensor_commu_idx_and_step


void timeout_job_can_sensor_scan(void)
{
	// Sensor scan finished.
	to_update_can_sensor_commu_idx_and_stepB = TRUE;
}	// timeout_job_can_sensor_scan


void timeout_job_can_sensor_query(void)
{
	// Sensor did not respond to last query.
	can_sensor_commu_cycle_failedB = TRUE;
	// Ensure processing of flag can_sensor_commu_cycle_failedB by function
	// update_can_sensor_commu_idx_and_step.
	to_update_can_sensor_commu_idx_and_stepB = TRUE;
}	// timeout_job_can_sensor_query


boolean update_can_sensor_list(void)
{
	dword sensor_addr;
	boolean resultB, sensor_addr_knownB;
	byte sen_idx;
	word payload_len = 0;
	byte *payload = 0;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr > 0 && (sensor_addr & 0x00040000UL) != 0x00040000UL;

	if( resultB )
	{
		resultB = check_rec_uip_30_msg(1, 'c', 0x10, 'B', &payload_len, &payload);
		resultB = resultB && payload_len == 0;
	}

	if( resultB )
		resultB = can_sensor_cnt < CAN_SENSOR_LIST_LEN;

	if( resultB )
	{
		sensor_addr_knownB = FALSE;
		for( sen_idx = 0; sen_idx < can_sensor_cnt_backup; sen_idx++ )
		{
			sensor_addr_knownB = sensor_addr == can_sensor_list_backup[sen_idx].addr;
			if( sensor_addr_knownB )
			{
				// Sensor found in backup list. Copy sensor data from backup list.
			    can_sensor_list[can_sensor_cnt] = can_sensor_list_backup[sen_idx];
				can_sensor_cnt++;
				break;
			}
		}

		if( !sensor_addr_knownB )
		{
			// Sensor not found in backup list. Create new entry in list. Sensor data were initialized
			// by call of function init_can_sensor_list on start of communication cycle. Query of
			// static data forced by can_sensor_list[can_sensor_cnt].static_data_knownB == FALSE.
		    can_sensor_list[can_sensor_cnt].addr = sensor_addr;
			can_sensor_cnt++;
		}
	}

	return(resultB);
}	// update_can_sensor_list


void update_can_sensor_list_backup_item(byte sen_idx_backup, byte sen_idx)
{
	can_sensor_list_backup[sen_idx_backup] = can_sensor_list[sen_idx];
	init_can_sensor_control_vars(&can_sensor_list_backup[sen_idx_backup]);
	if( can_sensor_list[sen_idx].to_init_can_sensor_dataB )
		enable_update_of_static_data_of_can_sensor(sen_idx_backup);
}	// update_can_sensor_list_backup_item


void update_can_sensor_list_backup(void)
{
	byte sen_idx, sen_idx_backup;
	dword sensor_addr;
	boolean sensor_addr_knownB;

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
	{
		sensor_addr = can_sensor_list[sen_idx].addr;

		sensor_addr_knownB = FALSE;
		for( sen_idx_backup = 0; sen_idx_backup < can_sensor_cnt_backup; sen_idx_backup++ )
		{
			sensor_addr_knownB = sensor_addr == can_sensor_list_backup[sen_idx_backup].addr;
			if( sensor_addr_knownB )
			{
				// Sensor found in backup list. Copy sensor data from list.
				update_can_sensor_list_backup_item(sen_idx_backup, sen_idx);

				break;
			}
		}

		// Sensor not found in backup list. Create new entry in backup list if possible.
		if( !sensor_addr_knownB && can_sensor_cnt_backup < CAN_SENSOR_LIST_LEN )
		{
			update_can_sensor_list_backup_item(can_sensor_cnt_backup, sen_idx);

			can_sensor_cnt_backup++;
		}
	}
}	// update_can_sensor_list_backup


void update_sorted_can_sensor_list(void)
{
	byte sen_idx, sorted_sen_idx;
	dword sensor_addr;
	boolean sensor_addr_knownB;

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
	{
		sensor_addr = can_sensor_list[sen_idx].addr;

		sensor_addr_knownB = FALSE;
		for( sorted_sen_idx = 0; sorted_sen_idx < sorted_can_sensor_cnt; sorted_sen_idx++ )
		{
			sensor_addr_knownB = sensor_addr == sorted_can_sensor_list[sorted_sen_idx].addr;
			if( sensor_addr_knownB )
			{
				sorted_can_sensor_list[sorted_sen_idx] = can_sensor_list[sen_idx];

				break;
			}
		}
	}
}	// update_sorted_can_sensor_list


byte get_idx_of_can_sensor_in_list(can_addr_id_type sensor_addr_id)
{
	dword sensor_addr;
	byte sen_idx;
	boolean foundB = FALSE;

	sensor_addr = can_addr_id_to_addr(sensor_addr_id);
	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
	{
		foundB = can_sensor_list[sen_idx].addr == sensor_addr;
		if( foundB )
			break;
	}

	if( foundB )
		return(sen_idx);
	else
		return(0xFF);
}	// get_idx_of_can_sensor_in_list


byte get_idx_of_can_sensor_in_backup_list(dword sensor_addr)
{
	byte sen_idx;
	boolean foundB = FALSE;

	for( sen_idx = 0; sen_idx < can_sensor_cnt_backup; sen_idx++ )
	{
		foundB = can_sensor_list_backup[sen_idx].addr == sensor_addr;
		if( foundB )
			break;
	}

	if( foundB )
		return(sen_idx);
	else
		return(0xFF);
}	// get_idx_of_can_sensor_in_backup_list


boolean get_sensor_conf_data_str(char *des_ptr, byte *payload)
{
	word str_len;

	str_len = (word)*(payload++) + ((word)*(payload++) << 8);
	copy_str(des_ptr, (char *)payload, str_len);

	return(TRUE);
}	// get_sensor_conf_data_str


boolean get_sensor_conf_data_byte(byte *byte_ptr, byte *payload)
{
	word data_len;

	data_len = (word)*(payload++) + ((word)*(payload++) << 8);
	*byte_ptr = *payload;

	return(data_len == 1);
}	// get_sensor_conf_data_byte


boolean get_sensor_conf_data_word(word *word_ptr, byte *payload)
{
	word data_len;

	data_len = (word)*(payload++) + ((word)*(payload++) << 8);
	*word_ptr =(word)*(payload++) + ((word)*(payload++) << 8);

	return(data_len == 2);
}	// get_sensor_conf_data_word


boolean update_static_can_sensor_data(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	word group_id, param_id;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(2, 'K', 0x10, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(2, 'K', 0x10, 'G', &payload_len, &payload);
		// minimum payload: group identifier, parameter identifier, parameter length (3 words)
		resultB = resultB && payload_len >= 6;
	}

	if( resultB && !error_msg_receivedB )
	{
		group_id = (word)*(payload++) + ((word)*(payload++) << 8);
		param_id = (word)*(payload++) + ((word)*(payload++) << 8);;

		switch( group_id )
		{
			case 0x0100 :
				switch( param_id )
				{
					case 0x0000 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].prod_name,       payload);
						break;

					case 0x0002 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].dev_number,      payload);
						break;
				}
				break;

			case 0x0200 :
				switch( param_id )
				{
					case 0x0000 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].prod_date,        payload);
						break;
				}
				break;

			case 0x0300 :
				switch( param_id )
				{
					case 0x0000 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].comm_name,        payload);
						break;

					case 0x0001 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].comm_version,     payload);
						break;

					case 0x0002 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].comm_timestamp,   payload);
						break;

					case 0x0100 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].appl_name,        payload);
						break;

					case 0x0101 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].appl_version,     payload);
						break;

					case 0x0102 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].appl_timestamp,   payload);
						break;

					case 0x0200 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].fpga_name,        payload);
						break;

					case 0x0201 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].fpga_version,     payload);
						break;

					case 0x0202 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].fpga_timestamp,   payload);
						break;

					case 0x0500 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].appl_activeB,   payload);
						break;

					case 0x0501 :
						resultB = get_sensor_conf_data_str(can_sensor_list[can_sensor_commu_idx].act_dspf_version, payload);
						break;

					case 0x0502 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].act_fpga_version, payload);
						break;
				}
				break;

			case 0x0500 :
				switch( param_id )
				{
					case 0x0300 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].sensor_cnt,       payload);
						break;

					case 0x0400 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].uip_addr_self,    payload);
						break;

					case 0x0401 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].uip_addr_master,  payload);
						break;

					case 0x0402 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].uip_addr_gateway, payload);
						break;
				}
				break;

			case 0x0700 :
				switch( param_id )
				{
					case 0x0000 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].function_area_cnt,      payload);
						break;

					case 0x0100 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[0], payload);
						break;

					case 0x0101 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[0], payload);
						break;

					case 0x0200 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[1], payload);
						break;

					case 0x0201 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[1], payload);
						break;

					case 0x0300 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[2], payload);
						break;

					case 0x0301 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[2], payload);
						break;

					case 0x0400 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[3], payload);
						break;

					case 0x0401 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[3], payload);
						break;

					case 0x0500 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[4], payload);
						break;

					case 0x0501 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[4], payload);
						break;

					case 0x0600 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[5], payload);
						break;

					case 0x0601 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[5], payload);
						break;

					case 0x0700 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[6], payload);
						break;

					case 0x0701 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[6], payload);
						break;

					case 0x0800 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_addrs[7], payload);
						break;

					case 0x0801 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].function_area_funcs[7], payload);
						break;
				}
				break;

			case 0x0800 :
				switch( param_id )
				{
					case 0x0000 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_cnt,       payload);
						break;

					case 0x0100 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[0],  payload);
						break;

					case 0x0101 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[0], payload);
						break;

					case 0x0102 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[0], payload);
						break;

					case 0x0200 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[1],  payload);
						break;

					case 0x0201 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[1], payload);
						break;

					case 0x0202 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[1], payload);
						break;

					case 0x0300 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[2],  payload);
						break;

					case 0x0301 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[2], payload);
						break;

					case 0x0302 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[2], payload);
						break;

					case 0x0400 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[3],  payload);
						break;

					case 0x0401 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[3], payload);
						break;

					case 0x0402 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[3], payload);
						break;

					case 0x0500 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[4],  payload);
						break;

					case 0x0501 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[4], payload);
						break;

					case 0x0502 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[4], payload);
						break;

					case 0x0600 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[5],  payload);
						break;

					case 0x0601 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[5], payload);
						break;

					case 0x0602 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[5], payload);
						break;

					case 0x0700 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[6],  payload);
						break;

					case 0x0701 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[6], payload);
						break;

					case 0x0702 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[6], payload);
						break;

					case 0x0800 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_addrs[7],  payload);
						break;

					case 0x0801 :
						resultB = get_sensor_conf_data_word(&can_sensor_list[can_sensor_commu_idx].door_delays[7], payload);
						break;

					case 0x0802 :
						resultB = get_sensor_conf_data_byte(&can_sensor_list[can_sensor_commu_idx].door_logics[7], payload);
						break;
				}
				break;
		}	//	end of "switch( group_id )"

		can_sensor_static_data_changedB = TRUE;
		to_update_can_sensor_commu_idx_and_stepB = TRUE;
	}

	return(resultB);
}	// update_static_can_sensor_data


/*
boolean addr_found_in_fa_addrs(byte function_area_cnt, word function_area_addr)
{
	boolean foundB = FALSE;
	byte fa_idx;

	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
	{
		foundB = function_area_addr == function_area_list[fa_idx].address;
		if( foundB )
			break;
	}

	return(foundB);
}	// addr_found_in_fa_addrs
*/


boolean no_found_in_door_signal_nos(byte door_cnt, word door_signal_no)
{
	boolean foundB = FALSE;
	byte door_idx;

	for( door_idx = 0; door_idx < door_cnt; door_idx++ )
	{
		foundB = door_signal_no == door_list[door_idx].signal_no;
		if( foundB )
			break;
	}

	return(foundB);
}	// no_found_in_door_signal_nos


boolean get_is_gateway_sensor_flag(byte sen_idx)
{
	boolean resultB;

	resultB = sen_idx < can_sensor_cnt && can_sensor_list[sen_idx].static_data_knownB && can_sensor_list[sen_idx].static_data_validB;
	if( resultB )
		resultB = can_sensor_list[sen_idx].uip_addr_self == can_sensor_list[sen_idx].uip_addr_gateway;

	return(resultB);
}	// get_is_gateway_sensor_flag


void check_can_sensor_list(void)
{
	byte sen_idx;
	boolean resultB;
	word door_signal_no;

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
	{
		resultB = can_sensor_list[sen_idx].static_data_knownB;

		// Different function area count and door count are useful only, if door states are determined
		// by sensor firmware.
		if( resultB )
			resultB = can_sensor_list[sen_idx].function_area_cnt == can_sensor_list[sen_idx].door_cnt;

		// Recent implementations of sensor firmware and gateway firmware are limited to case, where
		// function area assignment and door assignment of sensor are unique.
		if( resultB )
			resultB = can_sensor_list[sen_idx].function_area_cnt == 1 && can_sensor_list[sen_idx].door_cnt == 1;

		if( resultB )
		{
			door_signal_no = can_sensor_list[sen_idx].door_addrs[0];
			resultB = door_signal_no >= 1 && door_signal_no <= DOOR_CNT_MAX;
		}

		can_sensor_list[sen_idx].static_data_validB = resultB;
	}
}	// check_can_sensor_list


int bytecmp(const void *a, const void *b)
{
	return(*(byte*)a - *(byte*)b);
}	// bytecmp


int dwordcmp(const void *a, const void *b)
{
	return(*(dword*)a - *(dword*)b);
}	// dwordcmp


// Function areas without gateway sensor are ignored.
// Function area addresses are sorted in ascending order.
byte create_sorted_list_of_function_area_addresses(function_area_address_list_type *function_area_address_list_ptr)
{
	byte sen_idx, function_area_cnt = 0;
	boolean is_gateway_sensorB;
	word function_area_addr;

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
	{
		is_gateway_sensorB = get_is_gateway_sensor_flag(sen_idx);
		if( is_gateway_sensorB )
		{
			function_area_addr = can_sensor_list[sen_idx].function_area_addrs[0];
			function_area_address_list_ptr->list[function_area_cnt] = function_area_addr;
			function_area_cnt++;
		}
	}

	qsort((void *)function_area_address_list_ptr, function_area_cnt, sizeof(byte), bytecmp);

	return(function_area_cnt);
}	// create_sorted_list_of_function_area_addresses


boolean check_uip_addrs_of_function_area(byte *sen_cnt, sensor_assignments_type *sensor_assignments_ptr, uip_address_list_type *uip_address_list_ptr)
{
	byte sen_idx, sen_list_idx, uip_addr;
	boolean resultB = FALSE;

	for( sen_idx = 0; sen_idx < *sen_cnt; sen_idx++ )
	{
		sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];

		uip_addr = can_sensor_list[sen_list_idx].uip_addr_self;
		uip_address_list_ptr->list[sen_idx] = uip_addr;
		resultB = can_sensor_list[sen_list_idx].uip_addr_gateway == 1;
		if( !resultB )
			break;
	}

	qsort((void *)uip_address_list_ptr, *sen_cnt, sizeof(byte), bytecmp);

	if( resultB )
		for( sen_idx = 0; sen_idx < *sen_cnt; sen_idx++ )
		{
			resultB = uip_address_list_ptr->list[sen_idx] == sen_idx + 1;
			if( !resultB )
				break;
		}

	return(resultB);
}	// check_uip_addrs_of_function_area


void sort_sensor_assignments(byte *sen_cnt, sensor_assignments_type *sensor_assignments_ptr, uip_address_list_type *uip_address_list_ptr)
{
	byte sen_idx, sen_list_idx, uip_addr, sen_idx_2;
	sensor_assignments_type sorted_sensor_assignments;

	for( sen_idx = 0; sen_idx < CAN_SENSOR_LIST_LEN; sen_idx++ )
		sorted_sensor_assignments.sensor_list_indexes[sen_idx] = 0;

	for( sen_idx = 0; sen_idx < *sen_cnt; sen_idx++ )
	{
		sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];

		uip_addr = can_sensor_list[sen_list_idx].uip_addr_self;
		for( sen_idx_2 = 0; sen_idx_2 < *sen_cnt; sen_idx_2++ )
			if( uip_address_list_ptr->list[sen_idx_2] == uip_addr )
			{
				sorted_sensor_assignments.sensor_list_indexes[sen_idx_2] = sen_list_idx;
				break;
			}
	}

	*sensor_assignments_ptr = sorted_sensor_assignments;
}	// sort_sensor_assignments


boolean get_sensor_to_function_area_assignments(byte function_area_addr, byte *sen_cnt, sensor_assignments_type *sensor_assignments_ptr, boolean *static_data_of_all_assigned_sensors_knownB)
{
	byte sen_idx;
	boolean resultB;
	uip_address_list_type uip_address_list, *uip_address_list_ptr = &uip_address_list;

	*sen_cnt                                    = 0;
	*static_data_of_all_assigned_sensors_knownB = TRUE; 
	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
		if( can_sensor_list[sen_idx].function_area_addrs[0] == function_area_addr )
			if( can_sensor_list[sen_idx].static_data_knownB )
			{
				sensor_assignments_ptr->sensor_list_indexes[*sen_cnt] = sen_idx;
				(*sen_cnt)++;
			}
			else
				*static_data_of_all_assigned_sensors_knownB = FALSE; 

	resultB = *static_data_of_all_assigned_sensors_knownB;
	if( resultB )
		resultB = check_uip_addrs_of_function_area(sen_cnt, sensor_assignments_ptr, uip_address_list_ptr);
	if( resultB )
		sort_sensor_assignments(sen_cnt, sensor_assignments_ptr, uip_address_list_ptr);

	if( resultB )
		for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
			if( can_sensor_list[sen_idx].function_area_addrs[0] == function_area_addr )
			{
				resultB = *sen_cnt == can_sensor_list[sen_idx].sensor_cnt;
				if( !resultB )
					break;
			}

	return(resultB);
}	// get_sensor_to_function_area_assignments

	
void assign_static_function_area_data(byte fa_idx)
{
	byte sen_list_idx;

	sen_list_idx = function_area_list[fa_idx].sensor_assignments.sensor_list_indexes[0];

	function_area_list[fa_idx].function = can_sensor_list[sen_list_idx].function_area_funcs[0];
	strcpy(function_area_list[fa_idx].dspf_version, can_sensor_list[sen_list_idx].act_dspf_version);
	function_area_list[fa_idx].desired_sensor_cnt = can_sensor_list[sen_list_idx].sensor_cnt;
}	// assign_static_function_area_data


void init_function_area_data(byte fa_idx)
{
	function_area_list[fa_idx].counting_data_category_cnt             = 0;
	function_area_list[fa_idx].counting_data_category_params_queriedB = FALSE;
	function_area_list[fa_idx].static_data_knownB                     = FALSE;

	function_area_list[fa_idx].door_closedB        = TRUE;
	function_area_list[fa_idx].door_closed_oldB    = TRUE;
	function_area_list[fa_idx].dynamic_data_validB = TRUE;
}	// init_function_area_data


boolean check_validity_of_static_data_of_assigned_sensors(byte fa_idx)
{
	byte sen_cnt, sen_idx, sen_list_idx, door_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;
	word door_signal_no;
	boolean resultB = FALSE;

	sen_cnt = function_area_list[fa_idx].actual_sensor_cnt;
	sensor_assignments_ptr = &function_area_list[fa_idx].sensor_assignments;
	door_list_idx = function_area_list[fa_idx].door_list_idx;
	if( door_list_idx == 0xFF )
		door_signal_no = 0;
	else
		door_signal_no = door_list[door_list_idx].signal_no;

	for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
	{
		sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];

		resultB = can_sensor_list[sen_list_idx].static_data_validB;
		if( !resultB )
			break;

		resultB = can_sensor_list[sen_list_idx].door_addrs[0] == door_signal_no;
		if( !resultB )
			break;
	}

	return(resultB);
}	// check_validity_of_static_data_of_assigned_sensors


void remove_invalid_function_areas_from_list(byte *function_area_cnt)
{
	byte fa_idx, fa_idx_2;

	for( fa_idx = 0; fa_idx < *function_area_cnt; fa_idx++ )
		if( !function_area_list[fa_idx].static_data_validB )
		{
			for( fa_idx_2 = fa_idx; fa_idx_2 < *function_area_cnt; fa_idx_2++ )
				if( fa_idx_2 + 1 < *function_area_cnt )
					function_area_list[fa_idx_2] = function_area_list[fa_idx_2 + 1];

			(*function_area_cnt)--;
		}
}	//remove_invalid_function_areas_from_list


byte get_sensor_cnt_of_function_area(byte function_area_cnt, byte fa_idx)
{
	byte sensor_cnt = 0, desired_sensor_cnt, actual_sensor_cnt;

	if( fa_idx < function_area_cnt )
	{
		desired_sensor_cnt = function_area_list[fa_idx].desired_sensor_cnt;
		actual_sensor_cnt  = function_area_list[fa_idx].actual_sensor_cnt;

		if( desired_sensor_cnt > actual_sensor_cnt )
			// Take missing slave sensors into account.
			sensor_cnt = desired_sensor_cnt;
		else
			// Take additional slave sensors and misassigned master sensors into account.
			sensor_cnt = actual_sensor_cnt;
	}

	return(sensor_cnt);
}	// get_sensor_cnt_of_function_area


byte add_to_sorted_can_sensor_list(byte sorted_can_sensor_cnt, dword sensor_addr)
{
	byte sorted_sen_list_idx = sorted_can_sensor_cnt, sen_idx;

	for( sen_idx = 0; sen_idx < sorted_can_sensor_cnt; sen_idx++ )
		if( sorted_can_sensor_list[sen_idx].addr == sensor_addr)
		{
			sorted_sen_list_idx = sen_idx;
			break;
		}

	if( sorted_sen_list_idx < CAN_SENSOR_LIST_LEN )
		sorted_can_sensor_list[sorted_sen_list_idx].addr = sensor_addr;
	else
		// Value 0xFF indicates full list.
		sorted_sen_list_idx = 0xFF;

	return(sorted_sen_list_idx);
}	// add_to_sorted_can_sensor_list


// Count of sensors in sorted list is greater than count of sensors in current list in these cases:
// - At least one slave sensor is missing.
// - Function area addresses are not unique. As a result at least one sensor is multiply assigned to
//   function areas affected.
void create_sorted_can_sensor_list(byte function_area_cnt)
{
	byte sorted_can_sensor_cnt_buffer, sorted_can_sensor_cnt_buffer_2;
	byte fa_idx, sen_cnt, sen_idx, sen_list_idx, sorted_sen_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;
	dword sensor_addr;

	// Sorted sensor list contains invalid data during runtime of function.
	sorted_can_sensor_cnt = 0;
	memset(sorted_can_sensor_list, 0, sizeof(sorted_can_sensor_list));

	sorted_can_sensor_cnt_buffer = 0;
	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
	{
		if( fa_idx > 0 )
			sorted_can_sensor_cnt_buffer += get_sensor_cnt_of_function_area(function_area_cnt, fa_idx - 1);

		sen_cnt = function_area_list[fa_idx].actual_sensor_cnt;
		sensor_assignments_ptr = &function_area_list[fa_idx].sensor_assignments;

		sorted_can_sensor_cnt_buffer_2 = sorted_can_sensor_cnt_buffer;
		for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
		{
			sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];

			if( sen_list_idx < 0xFF )
			{
				can_sensor_list[sen_list_idx].fa_list_idx = fa_idx;
				sensor_addr = can_sensor_list[sen_list_idx].addr;

				sorted_sen_list_idx = add_to_sorted_can_sensor_list(sorted_can_sensor_cnt_buffer_2, sensor_addr);
				sensor_assignments_ptr->sensor_list_indexes[sen_idx] = sorted_sen_list_idx;

				if( sorted_sen_list_idx < 0xFF )
					sorted_can_sensor_cnt_buffer_2++;
			}
		}
	}
	if( function_area_cnt > 0 )
		sorted_can_sensor_cnt_buffer += get_sensor_cnt_of_function_area(function_area_cnt, function_area_cnt - 1);

	sorted_can_sensor_cnt = sorted_can_sensor_cnt_buffer;
}	// create_sorted_can_sensor_list


// On build of current function area list always counting data category parameters are queried for a l l
// function areas. This is necessary because function area data have to be refreshed completely. 
void update_function_area_and_door_list(void)
{
	byte function_area_cnt_buffer = 0, door_cnt_buffer = 0;
	function_area_address_list_type function_area_address_list, *function_area_address_list_ptr = &function_area_address_list;
	byte fa_idx, sen_cnt, sen_idx, door_idx, door_signal_idx;
	word function_area_addr, door_signal_no, closed_delay;
	sensor_assignments_type *sensor_assignments_ptr;
	boolean resultB = FALSE, static_data_of_all_assigned_sensors_knownB, new_doorB;

	// Function area list and door list contain invalid data during runtime of function.
	function_area_cnt = 0;
	door_cnt          = 0;
	memset(function_area_list, 0, sizeof(function_area_list));
	memset(door_list,          0, sizeof(door_list));

	function_area_cnt_buffer = create_sorted_list_of_function_area_addresses(function_area_address_list_ptr);

	for( fa_idx = 0; fa_idx < function_area_cnt_buffer; fa_idx++ )
	{
		// Value 0xFF indicates, that no door is assigned.
		function_area_list[fa_idx].door_list_idx = 0xFF;

		function_area_list[fa_idx].static_data_validB = TRUE;

		function_area_addr = function_area_address_list.list[fa_idx];
		function_area_list[fa_idx].address = function_area_addr;

		sensor_assignments_ptr = &function_area_list[fa_idx].sensor_assignments;
		resultB = get_sensor_to_function_area_assignments(function_area_addr, &sen_cnt, sensor_assignments_ptr, &static_data_of_all_assigned_sensors_knownB);
		function_area_list[fa_idx].actual_sensor_cnt                          = sen_cnt;
		function_area_list[fa_idx].static_data_of_all_assigned_sensors_knownB = static_data_of_all_assigned_sensors_knownB;
		if( !resultB )
			function_area_list[fa_idx].static_data_validB = FALSE;

		assign_static_function_area_data(fa_idx);
		init_function_area_data(fa_idx);
	}

	for( sen_idx = 0; sen_idx < can_sensor_cnt; sen_idx++ )
		if( get_is_gateway_sensor_flag(sen_idx) )
		{
			door_signal_no = can_sensor_list[sen_idx].door_addrs[0];

			new_doorB = !no_found_in_door_signal_nos(door_cnt_buffer, door_signal_no);
			if( new_doorB )
			{
				door_list[door_cnt_buffer].signal_no = door_signal_no;
				door_list[door_cnt_buffer].delay     =       can_sensor_list[sen_idx].door_delays[0];
				door_list[door_cnt_buffer].logic     = (char)can_sensor_list[sen_idx].door_logics[0];

				door_cnt_buffer++;
			}

			for( door_idx = 0; door_idx < door_cnt_buffer; door_idx++ )
				if( door_signal_no == door_list[door_idx].signal_no )
					break;
			function_area_addr = can_sensor_list[sen_idx].function_area_addrs[0];
			for( fa_idx = 0; fa_idx < function_area_cnt_buffer; fa_idx++ )
				if( function_area_addr == function_area_list[fa_idx].address )
					break;
			function_area_list[fa_idx].door_list_idx = door_idx;
		}

	for( fa_idx = 0; fa_idx < function_area_cnt_buffer; fa_idx++ )
	{
		resultB = check_validity_of_static_data_of_assigned_sensors(fa_idx);
		if( !resultB )
			function_area_list[fa_idx].static_data_validB = FALSE;

		// Check if function area address is unique.
		if( fa_idx < function_area_cnt_buffer - 1 )
		{
			resultB = function_area_list[fa_idx].address < function_area_list[fa_idx + 1].address;
			if( !resultB )
			{
				function_area_list[fa_idx].static_data_validB = FALSE;
				function_area_list[fa_idx + 1].static_data_validB = FALSE;
			}
		}
	}

	// Reduction of door list is not necessary, because it does not contain references to function area list.
	remove_invalid_function_areas_from_list(&function_area_cnt_buffer);

	for( door_idx = 0; door_idx < door_cnt_buffer; door_idx++ )
	{
		door_signal_idx = (byte)door_list[door_idx].signal_no - 1;
		// Ensure consistency of door signal parameters for timer_job_check_door_contacts_gateway.
		disable_timer_and_timeout_jobs();
		set_door_logic(door_signal_idx, door_list[door_idx].logic);
		closed_delay = door_list[door_idx].delay;
		// Door delays are queried by timer_job_check_door_contacts_gateway.
		set_door_delays(door_signal_idx, 0, closed_delay);
		enable_timer_and_timeout_jobs();
	}
	init_timer_job_check_door_contacts_gateway();

	create_sorted_can_sensor_list(function_area_cnt_buffer);
	update_sorted_can_sensor_list();

	function_area_cnt = function_area_cnt_buffer;
	door_cnt          = door_cnt_buffer;
}	// update_function_area_and_door_list


byte get_sensor_door_assignments(byte sen_idx, word *fa_addr, word *door_addr, byte *door_idx)
{
	byte sensor_door_cnt = 0;
	boolean resultB;

	resultB = sen_idx < can_sensor_cnt;
	if( resultB )
	{
		// Different function area count and door count are useful only, if door states are determined
		// by sensor firmware.
		sensor_door_cnt = can_sensor_list[sen_idx].door_cnt;
		resultB = can_sensor_list[sen_idx].function_area_cnt == sensor_door_cnt;
		if( resultB )
			// Recent implementations of sensor firmware and gateway firmware are limited to case, where
			// function area assignment and door assignment of sensor are unique.
			resultB = can_sensor_list[sen_idx].function_area_cnt == 1 && sensor_door_cnt == 1;
	}

	if( resultB )
	{
		*fa_addr   = can_sensor_list[sen_idx].function_area_addrs[0];
		*door_addr = can_sensor_list[sen_idx].door_addrs[0];

		for( *door_idx = 0; *door_idx < door_cnt; (*door_idx)++ )
			if( *door_addr == door_list[*door_idx].signal_no )
				break;
	}

	if( resultB )
		return(sensor_door_cnt);
	else
		return(0);
}	// get_sensor_door_assignments


byte get_sensor_fa_assignments(byte sen_idx, word *fa_addr, word *door_addr, byte *fa_idx)
{
	byte sensor_fa_cnt = 0;
	boolean resultB;

	resultB = sen_idx < can_sensor_cnt;
	if( resultB )
	{
		// Different function area count and door count are useful only, if door states are determined
		// by sensor firmware.
		sensor_fa_cnt = can_sensor_list[sen_idx].function_area_cnt;
		resultB = sensor_fa_cnt == can_sensor_list[sen_idx].door_cnt;
		if( resultB )
			// Recent implementations of sensor firmware and gateway firmware are limited to case, where
			// function area assignment and door assignment of sensor are unique.
			resultB = sensor_fa_cnt == 1 && can_sensor_list[sen_idx].door_cnt == 1;
	}

	if( resultB )
	{
		*fa_addr   = can_sensor_list[sen_idx].function_area_addrs[0];
		*door_addr = can_sensor_list[sen_idx].door_addrs[0];

		for( *fa_idx = 0; *fa_idx < function_area_cnt; (*fa_idx)++ )
			if( *fa_addr == function_area_list[*fa_idx].address )
				break;
	}

	if( resultB )
		return(sensor_fa_cnt);
	else
		return(0);
}	// get_sensor_fa_assignments


byte get_idx_of_function_area_in_list(word fa_addr)
{
	byte fa_idx;
	boolean foundB = FALSE;

	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
	{
		foundB = fa_addr == function_area_list[fa_idx].address;
		if( foundB )
			break;
	}

	if( foundB )
		return(fa_idx);
	else
		return(0xFF);
}	// get_idx_of_function_area_in_list


byte get_idx_of_door_in_list(word door_signal_no)
{
	byte door_idx;
	boolean foundB = FALSE;

	for( door_idx = 0; door_idx < door_cnt; door_idx++ )
	{
		foundB = door_signal_no == door_list[door_idx].signal_no;
		if( foundB )
			break;
	}

	if( foundB )
		return(door_idx);
	else
		return(0xFF);
}	// get_idx_of_door_in_list


/*
byte sen_idx_offs_for_function_area(byte fa_idx)
{
	byte sen_idx_offs = 0xFF, fa_scan;

	if( function_area_cnt > 0 )
	{
		sen_idx_offs = 0;
		for( fa_scan = 0; fa_scan < fa_idx; fa_scan++ )
			sen_idx_offs += function_area_list[fa_scan].desired_sensor_cnt;
	}

	return(sen_idx_offs);
}	// sen_idx_offs_for_function_area
*/


boolean update_counting_data_category_params(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	word function_area_addr;
	byte counting_data_category_cnt, counting_data_category_idx, fa_idx;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'C', 0x60, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'C', 0x60, 'C', &payload_len, &payload);
		resultB = resultB && payload_len >= 3;
	}

	if( resultB && !error_msg_receivedB )
	{
		function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
		counting_data_category_cnt = (byte)*(payload++);
		resultB = payload_len == 3 + 10 * counting_data_category_cnt;

		if( resultB )
			resultB = counting_data_category_cnt <= COUNTING_DATA_CATEGORY_CNT_MAX;

		if( resultB )
		{
			fa_idx = get_idx_of_function_area_in_list(function_area_addr);

			resultB = fa_idx < function_area_cnt;
			if( resultB )
			{
				for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
				{
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id    = (byte)*(payload++);
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].description_no = (byte)*(payload++);
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].param_1        = (word)*(payload++) + ((word)*(payload++) << 8);
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].param_2        = (word)*(payload++) + ((word)*(payload++) << 8);
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].param_3        = (word)*(payload++) + ((word)*(payload++) << 8);
					function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].param_4        = (word)*(payload++) + ((word)*(payload++) << 8);
				}

				// OBC firmware has to be capable to handle case where counting data category count is
				// equal to 0. Example IBIS BVG: Payload of data sets transmitted by gateway in response to
				// bX and bE queries is empty, but DS181 payload indicates counting data category count equal
				// to 0.
				function_area_list[fa_idx].counting_data_category_cnt = counting_data_category_cnt;
				function_area_list[fa_idx].counting_data_category_params_queriedB = TRUE;
				if( function_area_list[fa_idx].static_data_of_all_assigned_sensors_knownB )
					function_area_list[fa_idx].static_data_knownB = TRUE;
				if( counting_data_category_cnt == 0 )
					function_area_list[fa_idx].static_data_validB = FALSE;
			}
		}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// update_counting_data_category_params


boolean sensor_door_state_setting_confirmed(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	byte door_status_cnt, door_status_idx;
	word function_area_addr, door_signal_no, fa_idx/*, door_signal_idx*/;
	byte left_wing_opening_degree_old, right_wing_opening_degree_old;
	byte left_wing_opening_degree, right_wing_opening_degree;
	boolean closedB = FALSE, closed_oldB = FALSE;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'D', 0x30, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'D', 0x30, 'S', &payload_len, &payload);
		// minimum payload: door status count (byte)
		resultB = resultB && payload_len >= 1;
	}

	if( resultB && !error_msg_receivedB )
	{
		door_status_cnt = *(payload++);
		resultB = payload_len == 1 + 8 * door_status_cnt;
		if( resultB )
			for( door_status_idx = 0; door_status_idx < door_status_cnt; door_status_idx++ )
			{
				function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
				door_signal_no     = (word)*(payload++) + ((word)*(payload++) << 8);
				left_wing_opening_degree_old  = (byte)*(payload++);
				right_wing_opening_degree_old = (byte)*(payload++);
				left_wing_opening_degree      = (byte)*(payload++);
				right_wing_opening_degree     = (byte)*(payload++);

				if( left_wing_opening_degree_old == 0 && right_wing_opening_degree_old == 0 )
                                {	closed_oldB = TRUE;}
				else
                                {
					resultB = left_wing_opening_degree_old == 100 && right_wing_opening_degree_old == 100;
                                }

				if( resultB )
					if( left_wing_opening_degree == 0 && right_wing_opening_degree == 0 )
						closedB = TRUE;
					else
						resultB = left_wing_opening_degree == 100 && right_wing_opening_degree == 100;

				if( !resultB )
					break;

				fa_idx = get_idx_of_function_area_in_list(function_area_addr);
				resultB = fa_idx < function_area_cnt;
				if( resultB )
				{
					function_area_list[fa_idx].door_closedB     = closedB;
					function_area_list[fa_idx].door_closed_oldB = closed_oldB;

					function_area_list[fa_idx].current_door_states_validB = TRUE;
				}
				else
					break;

				if( can_sensor_list[can_sensor_commu_idx].door_closed_desired_activeB )
				{
					can_sensor_list[can_sensor_commu_idx].door_closed_desired_confirmedB = TRUE;
					can_sensor_list[can_sensor_commu_idx].door_closed_desired_activeB    = FALSE;
				}
			}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_door_state_setting_confirmed


boolean sensor_door_closing_before_reset_confirmed(void)
{
	dword sensor_addr;
	boolean resultB;
	word payload_len = 0;
	byte *payload = 0;
	byte door_status_cnt, door_status_idx;
	word function_area_addr, door_signal_no;
	byte left_wing_opening_degree_old, right_wing_opening_degree_old;
	byte left_wing_opening_degree, right_wing_opening_degree;
	boolean closedB = FALSE, closed_oldB = FALSE;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		resultB = check_rec_uip_30_msg(3, 'D', 0x30, 'S', &payload_len, &payload);
		// minimum payload: door status count (byte)
		resultB = resultB && payload_len >= 1;
	}

	if( resultB )
	{
		door_status_cnt = *(payload++);
		resultB = payload_len == 1 + 8 * door_status_cnt;
		if( resultB )
			for( door_status_idx = 0; door_status_idx < door_status_cnt; door_status_idx++ )
			{
				function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
				door_signal_no     = (word)*(payload++) + ((word)*(payload++) << 8);
				left_wing_opening_degree_old  = (byte)*(payload++);
				right_wing_opening_degree_old = (byte)*(payload++);
				left_wing_opening_degree      = (byte)*(payload++);
				right_wing_opening_degree     = (byte)*(payload++);

				if( left_wing_opening_degree_old == 0 && right_wing_opening_degree_old == 0 )
					closed_oldB = TRUE;
				else
					resultB = left_wing_opening_degree_old == 100 && right_wing_opening_degree_old == 100;

				if( resultB )
					if( left_wing_opening_degree == 0 && right_wing_opening_degree == 0 )
						closedB = TRUE;
					else
						resultB = left_wing_opening_degree == 100 && right_wing_opening_degree == 100;

				if( !resultB )
					break;
			}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_door_closing_before_reset_confirmed


boolean sensor_status_received(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	byte status_info_cnt, status_info_idx, status_info;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'S', 0x30, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'S', 0x30, 'G', &payload_len, &payload);
		// minimum payload: status information count (byte)
		resultB = resultB && payload_len >= 1;
	}

	if( resultB && !error_msg_receivedB )
	{
		status_info_cnt = *(payload++);
		resultB = payload_len == 1 + status_info_cnt;
		if( resultB )
			for( status_info_idx = 0; status_info_idx < status_info_cnt; status_info_idx++ )
			{
				status_info = *(payload++);

				// Refer to Sensor API header file "UipTypes.h".
				switch( status_info )
				{
					case 0x01 :
						can_sensor_list[can_sensor_commu_idx].sabotageB = TRUE;
						break;

					case 0x05 :
						can_sensor_list[can_sensor_commu_idx].invalid_config_paramsB = TRUE;
						break;
				}
			}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_status_received


boolean sensor_fa_status_received(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	byte status_info_cnt, status_info_idx, status_info, fa_idx;
	word function_area_addr;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'S', 0x31, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'S', 0x31, 'G', &payload_len, &payload);
		// minimum payload: status information count (byte)
		resultB = resultB && payload_len >= 1;
	}

	if( resultB && !error_msg_receivedB )
	{
		status_info_cnt = *(payload++);
		resultB = payload_len == 1 + 3 * status_info_cnt;
		if( resultB )
			for( status_info_idx = 0; status_info_idx < status_info_cnt; status_info_idx++ )
			{
				function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
				status_info = *(payload++);

				fa_idx = get_idx_of_function_area_in_list(function_area_addr);
				resultB = fa_idx < function_area_cnt;
				if( resultB )
				{
					// Refer to Sensor API header file "UipTypes.h".
					switch( status_info )
					{
						case 0x01 :
							can_sensor_list[can_sensor_commu_idx].new_counting_result_availableB = TRUE;
							init_can_sensor_ext_commu();
							function_area_list[fa_idx].new_counting_result_availableB = TRUE;
							break;

						case 0x02 :
							can_sensor_list[can_sensor_commu_idx].slave_missingB = TRUE;
							break;
					}
				}
				else
					break;
			}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_fa_status_received


boolean sensor_counting_data_received(void)
{
	dword sensor_addr;
	boolean resultB, is_gateway_sensorB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	word function_area_addr;
	boolean reset_flagB;
	char type_of_counting_data;
	byte counting_data_category_cnt, counting_data_category_idx, fa_idx;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;
	is_gateway_sensorB = get_is_gateway_sensor_flag(can_sensor_commu_idx);

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'C', 0x60, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'C', 0x60, 'G', &payload_len, &payload);
		resultB = resultB && payload_len >= 5;
	}

	if( resultB && !error_msg_receivedB )
	{
		function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
		reset_flagB = (boolean)*(payload++);
		type_of_counting_data = (char)*(payload++);
		counting_data_category_cnt = (byte)*(payload++);
		resultB = payload_len == 5 + 5 * counting_data_category_cnt;

		if( resultB )
			resultB = counting_data_category_cnt <= COUNTING_DATA_CATEGORY_CNT_MAX;

		if( resultB && is_gateway_sensorB )
		{
			fa_idx = get_idx_of_function_area_in_list(function_area_addr);

			resultB = fa_idx < function_area_cnt;
			if( resultB )
			{
				for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
					if( type_of_counting_data == 'B' )
					{
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id = (byte)*(payload++);
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in          = (word)*(payload++) + ((word)*(payload++) << 8);
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out         = (word)*(payload++) + ((word)*(payload++) << 8);

						if( function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id != function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id)
							clear_dynamic_data_valid_flag_of_function_area(fa_idx);
					}
					else
					{
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id = (byte)*(payload++);
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in          = (word)*(payload++) + ((word)*(payload++) << 8);
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out         = (word)*(payload++) + ((word)*(payload++) << 8);

						if( function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id != function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id)
							clear_dynamic_data_valid_flag_of_function_area(fa_idx);
					}

				if( type_of_counting_data == 'B' )
					function_area_list[fa_idx].last_counting_result_validB   = TRUE;
				else
					function_area_list[fa_idx].current_counter_states_validB = TRUE;
			}
		}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_counting_data_received


boolean sensor_counting_data_set_confirmed(void)
{
	dword sensor_addr;
	boolean resultB, is_gateway_sensorB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	word function_area_addr;
	boolean counting_finishedB;
	char type_of_counting_data;
	byte counting_data_category_cnt, counting_data_category_idx, fa_idx;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;
	is_gateway_sensorB = get_is_gateway_sensor_flag(can_sensor_commu_idx);

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'C', 0x60, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'C', 0x60, 'S', &payload_len, &payload);
		resultB = resultB && payload_len >= 5;
	}

	if( resultB && !error_msg_receivedB )
	{
		function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
		type_of_counting_data = (char)*(payload++);
		counting_finishedB = (boolean)*(payload++);
		counting_data_category_cnt = (byte)*(payload++);
		resultB = payload_len == 5 + 5 * counting_data_category_cnt;

		if( resultB )
			resultB = counting_data_category_cnt <= COUNTING_DATA_CATEGORY_CNT_MAX;

		if( resultB && is_gateway_sensorB )
		{
			fa_idx = get_idx_of_function_area_in_list(function_area_addr);

			resultB = fa_idx < function_area_cnt;
			if( resultB )
			{
				for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
					if( type_of_counting_data == 'B' )
					{
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id = (byte)*(payload++);
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in          = (word)*(payload++) + ((word)*(payload++) << 8);
						function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out         = (word)*(payload++) + ((word)*(payload++) << 8);

						if( function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id != function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id)
							clear_dynamic_data_valid_flag_of_function_area(fa_idx);
					}
					else
					{
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id = (byte)*(payload++);
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in          = (word)*(payload++) + ((word)*(payload++) << 8);
						function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out         = (word)*(payload++) + ((word)*(payload++) << 8);

						if( function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id != function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id)
							clear_dynamic_data_valid_flag_of_function_area(fa_idx);
					}

				if( type_of_counting_data == 'B' )
					function_area_list[fa_idx].last_counting_result_validB   = counting_finishedB;
				else
					function_area_list[fa_idx].current_counter_states_validB = TRUE;

				can_sensor_list[can_sensor_commu_idx].counting_data_buffer_desired_confirmedB = TRUE;
				can_sensor_list[can_sensor_commu_idx].counting_data_buffer_desired_activeB    = FALSE;
			}
		}

		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_counting_data_set_confirmed


boolean sensor_counting_result_reset_confirmed(void)
{
	dword sensor_addr;
	boolean resultB, error_msg_receivedB = FALSE;
	word payload_len = 0;
	byte *payload = 0;
	word function_area_addr;
	char type_of_counting_data;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		error_msg_receivedB = check_rec_uip_30_msg(3, 'C', 0x60, '!', &payload_len, &payload);
		if( error_msg_receivedB )
		{
			set_error_msg_receive_flag_of_can_sensor(can_sensor_commu_idx);

			init_can_sensor_ext_commu();
			to_update_can_sensor_commu_idx_and_stepB = TRUE;
		}
	}

	if( resultB && !error_msg_receivedB )
	{
		resultB = check_rec_uip_30_msg(3, 'C', 0x60, 'R', &payload_len, &payload);
		// minimum payload: function area address (word) and counting data type (char)
		resultB = resultB && payload_len == 3;
	}

	if( resultB && !error_msg_receivedB )
	{
		function_area_addr = (word)*(payload++) + ((word)*(payload++) << 8);
		type_of_counting_data = (char)*(payload++);

		if( type_of_counting_data == 'B' )
			can_sensor_list[can_sensor_commu_idx].new_counting_result_availableB = FALSE;
		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_counting_result_reset_confirmed


boolean sensor_firmware_reset_confirmed(void)
{
	dword sensor_addr;
	boolean resultB;
	word payload_len = 0;
	byte *payload = 0;

	sensor_addr = get_can_irma_addr_of_rec_uip_30_msg();
	resultB = sensor_addr == can_sensor_list[can_sensor_commu_idx].addr;

	if( resultB )
	{
		resultB = check_rec_uip_30_msg(2, 'W', 0x10, 'R', &payload_len, &payload);
		resultB = resultB && payload_len == 0;
	}

	if( resultB )
	{
		can_sensor_list[can_sensor_commu_idx].error_msg_receivedB = FALSE;
		to_update_can_sensor_commu_idx_and_stepB = resultB;
	}

	return(resultB);
}	// sensor_firmware_reset_confirmed


void update_sensor_list_of_obc_interface_data(void)
{
	byte sen_idx, sen_idx_backup;
	dword sensor_addr;
	boolean sensor_addr_knownB;

	obc_interface_data.sensor_cnt = 0;
	for( sen_idx = 0; sen_idx < sorted_can_sensor_cnt; sen_idx++ )
	{
		// Avoid violation of list length.
		if( obc_interface_data.sensor_cnt >= CAN_SENSOR_LIST_LEN )
			break;

		sensor_addr = sorted_can_sensor_list[sen_idx].addr;

		sensor_addr_knownB = FALSE;
		for( sen_idx_backup = 0; sen_idx_backup < obc_interface_data_backup.sensor_cnt; sen_idx_backup++ )
		{
			sensor_addr_knownB = sensor_addr == obc_interface_data_backup.sensor_list[sen_idx_backup].addr;
			if( sensor_addr_knownB )
			{
				// Sensor found in backup list. Copy sensor data from backup list.
			    obc_interface_data.sensor_list[obc_interface_data.sensor_cnt] = obc_interface_data_backup.sensor_list[sen_idx_backup];
				obc_interface_data.sensor_cnt++;
				break;
			}
		}

		if( !sensor_addr_knownB )
		{
			// Sensor not found in backup list. Create new entry in list and initialize sensor data.
			memset(&obc_interface_data.sensor_list[obc_interface_data.sensor_cnt], 0, sizeof(obc_interface_data.sensor_list[obc_interface_data.sensor_cnt]));
			obc_interface_data.sensor_cnt++;
		}
	}

	for( sen_idx = 0; sen_idx < sorted_can_sensor_cnt; sen_idx++ )
	{
		// static data
		obc_interface_data.sensor_list[sen_idx].addr = sorted_can_sensor_list[sen_idx].addr;

		strcpy(obc_interface_data.sensor_list[sen_idx].prod_name,    sorted_can_sensor_list[sen_idx].prod_name);
		strcpy(obc_interface_data.sensor_list[sen_idx].dev_number,   sorted_can_sensor_list[sen_idx].dev_number);
		strcpy(obc_interface_data.sensor_list[sen_idx].dspf_version, sorted_can_sensor_list[sen_idx].act_dspf_version);

		obc_interface_data.sensor_list[sen_idx].fa_list_idx = sorted_can_sensor_list[sen_idx].fa_list_idx;

		obc_interface_data.sensor_list[sen_idx].static_data_validB = sorted_can_sensor_list[sen_idx].static_data_validB;

		// dynamic data
		obc_interface_data.sensor_list[sen_idx].commu_cycle_failedB = sorted_can_sensor_list[sen_idx].commu_cycle_failedB;
		obc_interface_data.sensor_list[sen_idx].error_msg_receivedB = sorted_can_sensor_list[sen_idx].error_msg_receivedB;

		if( !sorted_can_sensor_list[sen_idx].static_data_validB )
			obc_interface_data.sensor_list[sen_idx].ever_static_data_invalidB   = TRUE;
		if( sorted_can_sensor_list[sen_idx].firmware_mismatchedB )
			obc_interface_data.sensor_list[sen_idx].ever_firmware_mismatchedB   = TRUE;
		if( sorted_can_sensor_list[sen_idx].commu_cycle_failedB )
			obc_interface_data.sensor_list[sen_idx].ever_commu_cycle_failedB    = TRUE;
		if( sorted_can_sensor_list[sen_idx].error_msg_receivedB )
			obc_interface_data.sensor_list[sen_idx].ever_error_msg_receivedB    = TRUE;
		if( sorted_can_sensor_list[sen_idx].invalid_config_paramsB )
			obc_interface_data.sensor_list[sen_idx].ever_invalid_config_paramsB = TRUE;
		if( sorted_can_sensor_list[sen_idx].slave_missingB )
			obc_interface_data.sensor_list[sen_idx].ever_slave_missingB         = TRUE;

		obc_interface_data.sensor_list[sen_idx].sabotageB = sorted_can_sensor_list[sen_idx].sabotageB;

		// desired data
		obc_interface_data.sensor_list[sen_idx].door_closed_desired_activeB    = sorted_can_sensor_list[sen_idx].door_closed_desired_activeB;
		obc_interface_data.sensor_list[sen_idx].door_closed_desired_confirmedB = sorted_can_sensor_list[sen_idx].door_closed_desired_confirmedB;
		obc_interface_data.sensor_list[sen_idx].counting_data_buffer_desired_activeB    = sorted_can_sensor_list[sen_idx].counting_data_buffer_desired_activeB;
		obc_interface_data.sensor_list[sen_idx].counting_data_buffer_desired_confirmedB = sorted_can_sensor_list[sen_idx].counting_data_buffer_desired_confirmedB;
	}
}	// update_sensor_list_of_obc_interface_data


void update_fa_list_of_obc_interface_data(void)
{
	byte fa_idx, fa_idx_backup;
	word fa_addr;
	boolean fa_addr_knownB;
	boolean door_closedB, door_closed_oldB;
	byte door_list_idx, counting_data_category_cnt = 0, counting_data_category_idx, category_id;
	word in, out, in_old, out_old;

	obc_interface_data.function_area_cnt = 0;
	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
	{
		// Avoid violation of list length.
		if( obc_interface_data.function_area_cnt >= CAN_FA_LIST_LEN )
			break;

		fa_addr = function_area_list[fa_idx].address;

		fa_addr_knownB = FALSE;
		for( fa_idx_backup = 0; fa_idx_backup < obc_interface_data_backup.function_area_cnt; fa_idx_backup++ )
		{
			fa_addr_knownB = fa_addr == obc_interface_data_backup.function_area_list[fa_idx_backup].address;
			if( fa_addr_knownB )
			{
				// Function area found in backup list. Copy function area data from backup list.
			    obc_interface_data.function_area_list[obc_interface_data.function_area_cnt] = obc_interface_data_backup.function_area_list[fa_idx_backup];
				obc_interface_data.function_area_cnt++;
				break;
			}
		}

		if( !fa_addr_knownB )
		{
			// Function area not found in backup list. Create new entry in list and initialize function area data.
			memset(&obc_interface_data.function_area_list[obc_interface_data.function_area_cnt], 0, sizeof(obc_interface_data.function_area_list[obc_interface_data.function_area_cnt]));
			obc_interface_data.function_area_cnt++;
		}
	}

	for( fa_idx = 0; fa_idx < function_area_cnt; fa_idx++ )
	{
		// static data
		door_list_idx              = function_area_list[fa_idx].door_list_idx;
		counting_data_category_cnt = function_area_list[fa_idx].counting_data_category_cnt;

		obc_interface_data.function_area_list[fa_idx].address                    = function_area_list[fa_idx].address;
		obc_interface_data.function_area_list[fa_idx].function                   = function_area_list[fa_idx].function;
		obc_interface_data.function_area_list[fa_idx].desired_sensor_cnt         = function_area_list[fa_idx].desired_sensor_cnt;
		obc_interface_data.function_area_list[fa_idx].actual_sensor_cnt          = function_area_list[fa_idx].actual_sensor_cnt;
		obc_interface_data.function_area_list[fa_idx].sensor_assignments         = function_area_list[fa_idx].sensor_assignments;
		obc_interface_data.function_area_list[fa_idx].door_signal_idx			 = (byte)door_list[door_list_idx].signal_no - 1;
		obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt = counting_data_category_cnt;

		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			obc_interface_data.function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx] = function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx];
			// Ensure validity of counting category identifiers independently of receiving of counter data from master sensors.
			category_id = obc_interface_data.function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id;
			obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id = category_id;
			obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id   = category_id;
			obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].category_id = category_id;
		}

		obc_interface_data.function_area_list[fa_idx].static_data_validB = function_area_list[fa_idx].static_data_validB;

		// dynamic data
		if( commu_cycle_for_function_area_without_error(fa_idx) )
		{
			if( function_area_list[fa_idx].current_door_states_validB )
			{
				door_closedB     = function_area_list[fa_idx].door_closedB;
				door_closed_oldB = function_area_list[fa_idx].door_closed_oldB;
				obc_interface_data.function_area_list[fa_idx].door_closedB     = door_closedB;
				obc_interface_data.function_area_list[fa_idx].door_closed_oldB = door_closed_oldB;

				function_area_list[fa_idx].current_door_states_validB = FALSE;
			}

			if( function_area_list[fa_idx].current_counter_states_validB )
			{
				for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
				{
					in_old  = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in;
					out_old = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out;
					in  = function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in;
					out = function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out;
					if( in > in_old )
						in -= in_old;
					else
						in = 0;
					if( out > out_old )
						out -= out_old;
					else
						out = 0;

					// Perform update of OBC interface data on increment of current counter state only.
					// This restriction is necessary to avoid unexpected decrements of counter values in
					// IRMA-C v2.0 payload. Refer to "Ber IRMA Matrix - Dekrement-Problem bei der IRMA-C-v2.0-Abfrage beseitigt 20110907.eml",
					// WiW-ID 12117.
					// However restriction must not be applied if update of current counter state was triggered
					// by receiving message IRMA-C v5.0.
					if( in > 0 || out >  0 || obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB )
						obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx] = function_area_list[fa_idx].current_counter_states[counting_data_category_idx];

					// Only one couple of output switches available. Counting results are summed up
					// for all function areas configured.
#if defined(SW2) && !defined(SW_VEHICLE)
			  		obc_interface_data.Persons_In_Counter_SW[0]  += in;
			  		obc_interface_data.Persons_Out_Counter_SW[0] += out;
#endif
					// Two couples of output switches available. Counting results for function areas
					// 1 and 2 are outputted by according pulses.
#if defined(SW4) && !defined(SW_VEHICLE)
					if( fa_idx == 0 )
					{
				  		obc_interface_data.Persons_In_Counter_SW[0]  += in;
			  			obc_interface_data.Persons_Out_Counter_SW[0] += out;
					}
					if( fa_idx == 1 )
					{
				  		obc_interface_data.Persons_In_Counter_SW[1]  += in;
			  			obc_interface_data.Persons_Out_Counter_SW[1] += out;
					}
#endif
				}

				function_area_list[fa_idx].current_counter_states_validB = FALSE;
			}

			if( function_area_list[fa_idx].last_counting_result_validB )
			{
				if( function_area_list[fa_idx].new_counting_result_availableB )
					obc_interface_data.function_area_list[fa_idx].new_counting_result_availableB = TRUE;
				for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
				{
					category_id = function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id;
					in          = function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in;
					out         = function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out;
					obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id = category_id;
					obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in          = in;
					obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out         = out;

					// Only one couple of output switches available. Counting results are summed up
					// for all function areas configured.
#if defined(SW2) && defined(SW_VEHICLE)
			  		obc_interface_data.Persons_In_Counter_SW[0]  += in;
			  		obc_interface_data.Persons_Out_Counter_SW[0] += out;
#endif
					// Two couples of output switches available. Counting results for function areas
					// 1 and 2 are outputted by according pulses.
#if defined(SW4) && defined(SW_VEHICLE)
					if( fa_idx == 0 )
					{
				  		obc_interface_data.Persons_In_Counter_SW[0]  += in;
			  			obc_interface_data.Persons_Out_Counter_SW[0] += out;
					}
					if( fa_idx == 1 )
					{
				  		obc_interface_data.Persons_In_Counter_SW[1]  += in;
			  			obc_interface_data.Persons_Out_Counter_SW[1] += out;
					}
#endif

					// Current counters of sensor are already cleared, if availability of new
					// counting result is indicated by sensor. Reset of current counter state
					// of OBC interface data at this point is necessary to ensure right values
					// of total counter states.
					obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in  = 0;
					obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out = 0;
				}
				function_area_list[fa_idx].new_counting_result_availableB = FALSE;

				function_area_list[fa_idx].last_counting_result_validB = FALSE;

				update_fa_status_gateway_var(fa_idx, &obc_interface_data.function_area_list[fa_idx].fa_status);

				obc_interface_data.function_area_list[fa_idx].counting_finishedB = TRUE;
			}
		}

		if( !function_area_list[fa_idx].static_data_validB )
			obc_interface_data.function_area_list[fa_idx].ever_static_data_invalidB  = TRUE;
		if( !function_area_list[fa_idx].dynamic_data_validB )
			obc_interface_data.function_area_list[fa_idx].ever_dynamic_data_invalidB = TRUE;
	}
	obc_interface_data.function_area_cnt = function_area_cnt;
}	// update_fa_list_of_obc_interface_data


void update_obc_interface_data_backup(void)
{
	byte sen_idx, sen_idx_backup, fa_idx, fa_idx_backup;
	dword sensor_addr;
	boolean sensor_addr_knownB, fa_addr_knownB;
	word fa_addr;

	for( sen_idx = 0; sen_idx < obc_interface_data.sensor_cnt; sen_idx++ )
	{
		sensor_addr = obc_interface_data.sensor_list[sen_idx].addr;

		sensor_addr_knownB = FALSE;
		for( sen_idx_backup = 0; sen_idx_backup < obc_interface_data_backup.sensor_cnt; sen_idx_backup++ )
		{
			sensor_addr_knownB = sensor_addr == obc_interface_data_backup.sensor_list[sen_idx_backup].addr;
			if( sensor_addr_knownB )
			{
				// Sensor found in backup list. Copy sensor data from list.
				obc_interface_data_backup.sensor_list[sen_idx_backup] = obc_interface_data.sensor_list[sen_idx];

				break;
			}
		}

		if( !sensor_addr_knownB && obc_interface_data_backup.sensor_cnt < CAN_SENSOR_LIST_LEN )
		{
			// Sensor not found in backup list. Create new entry in backup list if possible.
		    obc_interface_data_backup.sensor_list[obc_interface_data_backup.sensor_cnt] = obc_interface_data.sensor_list[sen_idx];

			obc_interface_data_backup.sensor_cnt++;
		}
	}

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		fa_addr = obc_interface_data.function_area_list[fa_idx].address;

		fa_addr_knownB = FALSE;
		for( fa_idx_backup = 0; fa_idx_backup < obc_interface_data_backup.function_area_cnt; fa_idx_backup++ )
		{
			fa_addr_knownB = fa_addr == obc_interface_data_backup.function_area_list[fa_idx_backup].address;
			if( fa_addr_knownB )
			{
				// Function area found in backup list. Copy function area data from list.
				obc_interface_data_backup.function_area_list[fa_idx_backup] = obc_interface_data.function_area_list[fa_idx];

				break;
			}
		}

		if( !fa_addr_knownB && obc_interface_data_backup.function_area_cnt < CAN_FA_LIST_LEN )
		{
			// Function area not found in backup list. Create new entry in backup list if possible.
		    obc_interface_data_backup.function_area_list[obc_interface_data_backup.function_area_cnt] = obc_interface_data.function_area_list[fa_idx];

			obc_interface_data_backup.function_area_cnt++;
		}
	}
}	// update_obc_interface_data_backup


// Query of gateway by OBC is possible after first call of update_obc_interface_data. Queries are
// answered for known function areas and assigned sensors.
void update_obc_interface_data(void)
{
	obc_interface_data_fa_cnt_increasedB = FALSE;
	obc_interface_data_fa_cnt_decreasedB = FALSE;
	if( function_area_cnt > obc_interface_data.function_area_cnt )
		obc_interface_data_fa_cnt_increasedB = TRUE;
	if( function_area_cnt < obc_interface_data.function_area_cnt )
		obc_interface_data_fa_cnt_decreasedB = TRUE;

	// Suppress step-by-step increment of obc interface data function area count after partly or
	// completely IRMA 5 Sensor communication interrupt. Without this suppression following effects
	// occur as long as
	// obc_interface_data.function_area_cnt < obc_interface_data_backup.function_area_cnt:
	// - IBISIP:
	//   Queries are not answered for FA addresses not recovered yet.
	// - UIP:
	//   Some function areas are missed in payload of response messages comprising all function areas
	//   (e.g. IRMA-C v2.0 [0xFF] response payload).
	if( (Comm_ConfigData->memory[15] & 0x01) == 0x00 &&
	    obc_interface_data.sensor_cnt > 0            &&
	    sorted_can_sensor_cnt < obc_interface_data.sensor_cnt )
	{
//		Service_RunLevel_1();	// for enabling of breakpoint only

		return;
	}

	// Ensure consistency of OBC interface data with respect to timer and time-out jobs, e.g.
	// function J1708SysDiag.
	disable_timer_and_timeout_jobs();

	update_sensor_list_of_obc_interface_data();

	update_fa_list_of_obc_interface_data();

	update_door_state_set_permission_flag_of_obc_interface_data();

	// Update desired data for next CAN sensor communication cycle.
	// Examples: desired door states and desired counter states.
	// Typically desired data are received by command messages like IRMA-D v1.0 and IRMA-C v5.0.
	update_desired_function_area_data();

	// Update data depending on OBC interface data, e.g. desired J1708 sensor status.
	if( obc_interface_data_update_func != NULL )
		obc_interface_data_update_func();

	enable_timer_and_timeout_jobs();

	call_all_fa_door_jobs_gateway();

	// OBC interface data have to be consistent on call of counting finished jobs, e.g. because of
	// quering of obc_interface_data.function_area_cnt.
	call_all_counting_finished_jobs_gateway();

	// Flags obc_interface_data.function_area_list[].first_call_of_fa_door_jobs_doneB are set by
	// function call_all_fa_door_jobs_gateway.
	update_obc_interface_data_backup();
}	// update_obc_interface_data


void update_desired_data_of_can_sensor_list_backup(void)
{
	byte sen_idx_backup, master_sen_idx, fa_idx;
	byte counting_data_category_cnt, counting_data_category_idx;
	dword sensor_addr;
	boolean sensor_addr_knownB;

	for( sen_idx_backup = 0; sen_idx_backup < can_sensor_cnt_backup; sen_idx_backup++ )
	{
		// Initialize all CAN sensor list command confirmation flags.
		can_sensor_list_backup[sen_idx_backup].door_closed_desired_confirmedB          = FALSE;
		can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired_confirmedB = FALSE;

		sensor_addr = can_sensor_list_backup[sen_idx_backup].addr;

		for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		{
			master_sen_idx = obc_interface_data.function_area_list[fa_idx].sensor_assignments.sensor_list_indexes[0];
			sensor_addr_knownB = sensor_addr == obc_interface_data.sensor_list[master_sen_idx].addr;

			if( sensor_addr_knownB )
			{
				// Start update of desired data of CAN sensors if requested.
				if( obc_interface_data.use_desired_door_statesB )
				{
					if( obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB )
					{
						can_sensor_list_backup[sen_idx_backup].use_desired_door_stateB = TRUE;
						can_sensor_list_backup[sen_idx_backup].door_closed_desiredB = obc_interface_data.function_area_list[fa_idx].door_closed_desiredB;
						can_sensor_list_backup[sen_idx_backup].door_closed_desired_activeB = TRUE;
					}
				}
				else
					can_sensor_list_backup[sen_idx_backup].use_desired_door_stateB = FALSE;

				if( obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB )
				{
					counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
					for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
						can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired.counting_data[counting_data_category_idx] = obc_interface_data.function_area_list[fa_idx].current_counter_states_desired[counting_data_category_idx];
					can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired.counting_data_category_cnt = counting_data_category_cnt;
					can_sensor_list_backup[sen_idx_backup].counting_data_buffer_desired_activeB = TRUE;
				}

				break;
			}
		}
	}
}	// update_desired_data_of_can_sensor_list_backup


void clear_persistent_error_flags_of_function_area_and_assigned_sensors(byte fa_idx)
{
	byte sen_cnt, sen_idx, sen_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;

	if( fa_idx < obc_interface_data.function_area_cnt )
	{
		sen_cnt = obc_interface_data.function_area_list[fa_idx].actual_sensor_cnt;
		sensor_assignments_ptr = &obc_interface_data.function_area_list[fa_idx].sensor_assignments;

		for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
		{
			sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];

			if( sen_list_idx < obc_interface_data.sensor_cnt )
			{
				obc_interface_data.sensor_list[sen_list_idx].ever_static_data_invalidB   = FALSE;
				obc_interface_data.sensor_list[sen_list_idx].ever_firmware_mismatchedB   = FALSE;
				obc_interface_data.sensor_list[sen_list_idx].ever_commu_cycle_failedB    = FALSE;
				obc_interface_data.sensor_list[sen_list_idx].ever_error_msg_receivedB    = FALSE;
				obc_interface_data.sensor_list[sen_list_idx].ever_invalid_config_paramsB = FALSE;
				obc_interface_data.sensor_list[sen_list_idx].ever_slave_missingB         = FALSE;
			}
		}

		obc_interface_data.function_area_list[fa_idx].ever_static_data_invalidB  = FALSE;
		obc_interface_data.function_area_list[fa_idx].ever_dynamic_data_invalidB = FALSE;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// clear_persistent_error_flags_of_function_area_and_assigned_sensors


void clear_persistent_error_flags_of_all_function_areas_and_assigned_sensors(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		clear_persistent_error_flags_of_function_area_and_assigned_sensors(fa_idx);
}	// clear_persistent_error_flags_of_all_function_areas_and_assigned_sensors


boolean disable_can_transparent_mode_dist500(void)
{
	boolean can_transparent_mode_disabledB;

	can_transparent_mode_disabledB = disable_can_transparent_mode();

	if( can_transparent_mode_disabledB )
		start_can_sensor_commu();

	return(can_transparent_mode_disabledB);
}	// disable_can_transparent_mode_dist500
