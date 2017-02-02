/*==============================================================================================*
 |       Filename: communication_gdist500.c                                                     |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Main module of module group Communication for IRMA 5 gateway.                |
 |                 A21 hardware is used for IRMA 5 gateway.                                     |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifndef GATEWAY
	#error "File communication_gdist500.c included but symbol GATEWAY not defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>

#include "..\..\opera_version.h"
#include "..\..\kernel_interface.h"
#include "..\communication.h"
#include "communication_gdist500.h"
#if defined(SW2) || defined(SW4)
	#include "..\switches.h"
#endif
#include "uip_gdist500.h"
#ifndef NO_LOGGER
	#include "..\logger.h"
	#include "logger_gdist500.h"
#endif
#include "..\..\application_start.h"
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
#include "..\can.h"
#include "can_irma_embedding.h"
#include "can_irma_dist500.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define COUNTING_FINISHED_JOBS_MAX		10


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=COMM
#pragma class CO=COMM
#pragma class FB=COMMRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile door_signal_interface_type door_signal_interface;
volatile bool all_doors_already_closedB;

obc_interface_data_type obc_interface_data, obc_interface_data_backup;

bool obc_interface_data_fa_cnt_increasedB, obc_interface_data_fa_cnt_decreasedB;

const void (*timeout_job_door_signal_delay_list[DOOR_CNT_MAX])(void);

void (*counting_finished_job[COUNTING_FINISHED_JOBS_MAX])(byte);

obc_interface_data_update_func_type obc_interface_data_update_func;


/*----- Function Prototypes --------------------------------------------------------------------*/
void init_global_var_comm(void);

void io_port_2_setup_gateway(void);

bool startup_communication_prot_prepared(void);
void init_startup_communication_prot(void);
void remove_startup_communication_prot_jobs(void);

int communication_RL4(void);

void init_device_status_gateway(void);

#ifndef NO_LOGGER
	#ifdef DOOR_OPEN_EVENT
void Log_Door_Open_Event_RL4(void);
	#endif
#endif

void timeout_job_door_signal_1_delay(void);
void timeout_job_door_signal_2_delay(void);
void timeout_job_door_signal_3_delay(void);
void timeout_job_door_signal_4_delay(void);
void timer_job_check_door_contacts_gateway(void);

void update_obc_counting_result_buffer_and_fa_status_buffer(byte fa_idx);

void init_fa_status_gateway_var(fa_status_gateway_type *fa_status_gateway);

#ifndef NO_LOGGER
void counting_finished_job_logger(byte fa_idx);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
int communication_main(void)
{
	int rlv_ret;

	communication_init();
	init_global_var_comm();

	io_port_2_setup_gateway();

#if defined(SW2) || defined(SW4)
	init_switches();
#endif

	if( !Recovery_Mode )
	{
		init_device_status_gateway();

		timeout_job_door_signal_delay_list[0] = timeout_job_door_signal_1_delay;
		timeout_job_door_signal_delay_list[1] = timeout_job_door_signal_2_delay;
		timeout_job_door_signal_delay_list[2] = timeout_job_door_signal_3_delay;
		timeout_job_door_signal_delay_list[3] = timeout_job_door_signal_4_delay;
	}

	if( !startup_communication_prot_prepared() )
		Set_Runlevel(1);

	// Runlevel is equal to RUNLEVEL_SERV if recovery mode is active. As a result function
	// communication_RL4 is not called.
	if( Recovery_Mode )
		init_startup_communication_prot();

	// Avoid software reset by function RunLevel_1_Service_Check. 
	Service_RunLevel_1();

	while( Get_Runlevel() >= RUNLEVEL_SERV )
	{
		if( Get_Runlevel() > RUNLEVEL_SERV )
		{
			rlv_ret = communication_RL4();

			remove_startup_communication_prot_jobs();
		}

		Call_Runlevel_Jobs();
 	}	// end of "while( Get_Runlevel() >= RUNLEVEL_SERV )"

	remove_timer_job(timer_job_check_door_contacts_gateway);

	return(0);
}	// communication_main


// Initialize global variables not properly initialized by function "clear_comm_RAM".
void init_global_var_comm(void)
{
	init_global_var_can_irma_embedding();
	init_global_var_can_irma_dist500();
}	// init_global_var_comm


void io_port_2_setup_gateway(void)
{
	byte p2_idx;

	// Maximum digital input count of A21 is 4.
	for( p2_idx = 0; p2_idx < DOOR_CNT_MAX; p2_idx++ )
	{
		Channel[p2_idx].ignoreB = FALSE;
		Channel[p2_idx].inputB = TRUE;
		Channel[p2_idx].duration.chatter.rising  = 10;
		Channel[p2_idx].duration.chatter.falling = 10;
	}
}	// io_port_2_setup_gateway


#ifdef SW4
void send_person_counter_SW4(void)
{
	bool person_in_1 = FALSE, person_out_1 = FALSE, person_in_2 = FALSE, person_out_2 = FALSE;

	Get_Persons_Counter_SW2(&person_in_1, &person_out_1);
	Get_Persons_Counter_SW4(&person_in_2, &person_out_2);

	set_switches(person_in_1, person_out_1, person_in_2, person_out_2);
}	// send_person_counter_SW4
#endif	// end of "#ifdef SW4"


#ifdef SW2
void send_person_counter_SW2(void)
{
	bool person_in = FALSE, person_out = FALSE;

	Get_Persons_Counter_SW2(&person_in, &person_out);

	set_switches(person_in, person_out);
}	// send_person_counter_SW2
#endif	// end of "#ifdef SW2"


bool startup_communication_prot_prepared(void)
{
	bool preparedB = FALSE;

	switch( Comm_ConfigData->communication_protocol )
	{
		case PROT_ID_UNKNOWN	:
		case PROT_ID_IRMA		: Prepare_IRMA();          preparedB = TRUE; break;

#ifdef IBIS_PROTOCOL
		case PROT_ID_IBIS		: prepare_ibisip_init();   preparedB = TRUE; break;
#endif

#ifdef UJ1708IP
		case PROT_ID_J1708		: Prepare_UJ1708IP();      preparedB = TRUE; break;
#endif

#ifdef J1708_ORBITAL
		case PROT_ID_J1708		: Prepare_J1708_Orbital(); preparedB = TRUE; break;
#endif
	}	// end of "switch( Comm_ConfigData->communication_protocol )"

	return(preparedB);
}	// startup_communication_prot_prepared


void init_startup_communication_prot(void)
{
	switch( Comm_ConfigData->communication_protocol )
	{
		case PROT_ID_UNKNOWN	:
		case PROT_ID_IRMA		: Init_IRMA();           break;

#ifdef IBIS_PROTOCOL
		case PROT_ID_IBIS		: init_ibisip_gateway(); break;
#endif

#ifdef UJ1708IP
		case PROT_ID_J1708		: Init_UJ1708IP();       break;
#endif

#ifdef J1708_ORBITAL
		case PROT_ID_J1708		: Init_J1708_Orbital();  break;
#endif
	}	// end of "switch( Comm_ConfigData->communication_protocol )"

	// Avoid software reset by function RunLevel_1_Service_Check. 
	Service_RunLevel_1();
}	// init_startup_communication_prot


void remove_startup_communication_prot_jobs(void)
{
#ifdef IBIS_PROTOCOL
	remove_ibisip_jobs();
#endif
#ifdef UJ1708IP
	Remove_UJ1708IP_Jobs();
#endif
#ifdef J1708_ORBITAL
	Remove_J1708_Orbital_Jobs();
#endif
}	// remove_startup_communication_prot_jobs


#ifdef NO_LOGGER
int run_application_module(void)
#else
int run_application_module(bool Logger_OKB)
#endif
{
	int rlv_ret;

#ifndef NO_LOGGER
	if( Logger_OKB )
		Add_Counting_Finished_Job(counting_finished_job_logger);
#else
	#ifdef SW4
	add_timer_job(minPulsPeriod, send_person_counter_SW4);
	#endif
	#ifdef SW2
	add_timer_job(minPulsPeriod, send_person_counter_SW2);
	#endif
#endif
#ifdef J1708_PROTOCOL 
	Activate_Counting_Finished_Job_J1708();
#endif

	rlv_ret = RunLevel5Main();

#ifndef NO_LOGGER
	if( Logger_OKB )
		Remove_Counting_Finished_Job(counting_finished_job_logger);
#else
	#ifdef SW4
	remove_timer_job(send_person_counter_SW4);
	#endif
	#ifdef SW2
	remove_timer_job(send_person_counter_SW2);
	#endif
#endif
#ifdef J1708_PROTOCOL 
	Deactivate_Counting_Finished_Job_J1708();
#endif

	return(rlv_ret);
}	// run_application_module


void init_obc_interface_data_for_dummy_obc_communication(void)
{
	obc_interface_data.function_area_cnt = 2;
	obc_interface_data.function_area_list[0].address = 1;
	obc_interface_data.function_area_list[1].address = 2;

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// init_obc_interface_data_for_dummy_obc_communication


int communication_RL4(void)
{
	bool runlevel5_desiredB;
	bool to_init_startup_communication_protB;
#ifndef NO_LOGGER
	bool Logger_Memory_OKB;
	bool Logger_OKB;
#endif
	int rlv_ret;

#ifndef NO_LOGGER
	Logger_Memory_OKB = Init_Logger_Memory();
	Logger_OKB = Init_Logger();
	Log_RunLevel_Change(RUNLEVEL_SERV, RUNLEVEL_COMM);
#endif

#if !defined(NO_LOGGER) && defined(DOOR_OPEN_EVENT)
	if( Logger_OKB )
		Add_Runlevel_Job(Log_Door_Open_Event_RL4);
#endif

#if defined(SW4) || defined(SW2)
	calc_switching_params();
#endif

	to_init_startup_communication_protB = TRUE;

	runlevel5_desiredB = FALSE;
	// Check for valid IRMA Application module.
	if( SectorValid(SECTOR_APPLICATION1) && SectorValid(SECTOR_APPLICATION2) )
	{
		RunLevel5Main = application_main;
		runlevel5_desiredB = TRUE;
	}
	
	init_can_interf_gateway();
	send_can_irma_startup_msg();
	Add_Runlevel_Job(check_for_pending_can_irma_jobs);

	start_can_sensor_commu();

	// Ensure finishing of first CAN sensor communication cycle before initialization of ASC0 communication.
	while( can_sensor_commu_cycle_cnt < 1 )
	{
		// Avoid software reset by function RunLevel_1_Service_Check. 
		Service_RunLevel_1();

		Call_Runlevel_Jobs();
 	}	// end of "while( can_sensor_commu_cycle_cnt < 1 )"

	switch( Comm_ConfigData->communication_protocol )
	{
		#ifdef IBIS_PROTOCOL
			case  PROT_ID_IBIS	: init_ibisip_counting_data_buffers(); break;
		#endif

		#if defined(UJ1708IP) || defined(J1708_ORBITAL) 
			case  PROT_ID_J1708	: Init_J1708_Counting(); break;
		#endif

		default	: break;
	}

	while( Get_Runlevel() >= RUNLEVEL_COMM )
	{
		if( can_bus_off_state_detectedB )
		{
			start_can_busoff_recovery_sequence();
			can_bus_off_state_detectedB = FALSE;

			start_can_sensor_commu();
		}

		if( Get_Runlevel() < RUNLEVEL_APPL && runlevel5_desiredB )
		{
			Set_Runlevel(RUNLEVEL_APPL);
#ifndef NO_LOGGER
			Log_RunLevel_Change(RUNLEVEL_COMM, RUNLEVEL_APPL);
#endif
		}
							
		if( to_init_startup_communication_protB )
		{
			init_startup_communication_prot();
			to_init_startup_communication_protB = FALSE;

			// ASC0 debug mode is activated at end of function InitP3P4ForASC0 if symbol
			// DEBUG_START_RS232_DEBUG_MODE is defined
		}

		if( Get_Runlevel() > RUNLEVEL_COMM )
	#ifdef NO_LOGGER
			rlv_ret = run_application_module();
	#else
			rlv_ret = run_application_module(Logger_OKB);
	#endif

		Call_Runlevel_Jobs();
	}	// end of "while( Get_Runlevel() >= RUNLEVEL_COMM )"

	deactivate_obc_interface();

	Disable_CAN_Interrupts();
	Remove_CAN_IRMA_Jobs();

#if !defined(NO_LOGGER) && defined(DOOR_OPEN_EVENT)
	if( Logger_OKB )
		Remove_Runlevel_Job(Log_Door_Open_Event_RL4);
#endif

	return(0);
}	// communication_RL4


void InitDeviceStatusSensor(void)
{
	byte SenIdx;

	for( SenIdx = 0; SenIdx < SENSORS_MAX; SenIdx++ )
	{
		DeviceStatus.sensor[SenIdx].initialization_errorB = FALSE;
	    DeviceStatus.sensor[SenIdx].initialization_error_door_openB = FALSE;
		DeviceStatus.sensor[SenIdx].runtime_errorB = FALSE; 
		DeviceStatus.sensor[SenIdx].runtime_error_door_openB = FALSE;
		DeviceStatus.sensor[SenIdx].firmware_mismatchB = FALSE;
		DeviceStatus.sensor[SenIdx].sabotageB = FALSE;
	}	// end of for( SenIdx = 0; SenIdx < SENSORS_MAX; SenIdx++ )
}	// InitDeviceStatusSensor


void init_device_status_gateway(void)
{
	init_device_status();
	all_doors_already_closedB = TRUE;

	init_door_jobs();

	InitDeviceStatusSensor();
}	// init_device_status_gateway


#ifndef NO_LOGGER
	#ifdef DOOR_OPEN_EVENT
void Log_Door_Open_Event_RL4(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < DOOR_CNT_MAX; fa_idx++ )
	{
		if( Door_Opened_FlagB[fa_idx] )
		{
			Log_Door_State(fa_idx);
			Door_Opened_FlagB[fa_idx] = FALSE;
		}
	}
}	// Log_Door_Open_Event_RL4
	#endif
#endif	// end of "#ifndef NO_LOGGER"


bool all_doors_closed_gateway(void)
{
	byte fa_idx;

	if( obc_interface_data.function_area_cnt == 0 )
		return(TRUE);

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		if( !obc_interface_data.function_area_list[fa_idx].door_closedB )
			return(FALSE);

	return(TRUE);
}	// all_doors_closed_gateway


// Result TRUE only once after closing of last door.
bool last_door_closed_gateway(void)
{
	if( obc_interface_data.function_area_cnt == 0 )
		return(FALSE);

	if( all_doors_closed_gateway() )
		if( all_doors_already_closedB )
			return(FALSE);
		else
		{
			all_doors_already_closedB = TRUE;
			return(TRUE);
		}
	else
		return(FALSE);
}	// last_door_closed_gateway


// During first and second call of function update_obc_interface_data function result
// is given by initial door states of IRMA 5 sensors not controlled by gateway (closed after power-on).
bool first_door_opened_gateway(void)
{
	byte fa_idx;

	if( obc_interface_data.function_area_cnt == 0 )
		return(FALSE);

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		if( !obc_interface_data.function_area_list[fa_idx].door_closed_oldB )
			return(FALSE);

	return(TRUE);
}	// first_door_opened_gateway


void set_door_logic(byte door_signal_idx, char logic)
{
	door_signal_interface.positive_logicB[door_signal_idx] = logic == '+';
}	// set_door_logic


void set_door_delays(byte door_signal_idx, word opened_delay, word closed_delay)
{
	if( door_signal_idx < DOOR_CNT_MAX )
	{
		door_signal_interface.opened_delay[door_signal_idx] = opened_delay;
		door_signal_interface.closed_delay[door_signal_idx] = closed_delay;
	}
}	// set_door_delays


void init_timer_job_check_door_contacts_gateway(void)
{
	// Ensure synchronization of variable door_signal_interface with door signal inputs before
	// first transmission of door states to can sensors.
	if( !door_signal_interface.statuses_synchronizedB )
	{
		timer_job_check_door_contacts_gateway();
		add_timer_job(100, timer_job_check_door_contacts_gateway);
	}
}	// init_timer_job_check_door_contacts_gateway


bool door_contacts_ever_changed_gateway(void)
{
	byte p2_channel_idx;
	bool changedB = FALSE;

	for( p2_channel_idx = 0; p2_channel_idx < DOOR_CNT_MAX; p2_channel_idx++ )
		if( ChannelState_ChangedB[p2_channel_idx] )
			changedB = TRUE;

	return(changedB);
}	// door_contacts_ever_changed_gateway


bool get_logical_p2_channel_state(byte door_signal_idx)
{
	byte p2_channel_no;
	char p2_channel_state;
	bool door_closedB;

	p2_channel_no = door_signal_idx + 1;
	p2_channel_state = Get_Channel(p2_channel_no);

#ifdef DOOR_LOG_POS
   	door_closedB = (bool)p2_channel_state;		// logically positive
#else
	if( door_signal_interface.positive_logicB[door_signal_idx] )
	   	door_closedB =  (bool)p2_channel_state;	// logically positive
	else
	   	door_closedB = !(bool)p2_channel_state;	// logically negative
#endif

	return(door_closedB);
}	// get_logical_p2_channel_state


void timeout_job_door_signal_1_delay(void)
{
	bool current_input_stateB;

	current_input_stateB = get_logical_p2_channel_state(0);

	door_signal_interface.closed_flagsB[0]  = current_input_stateB;
	door_signal_interface.delay_runningB[0] = FALSE;
}	// timeout_job_door_signal_1_delay


void timeout_job_door_signal_2_delay(void)
{
	bool current_input_stateB;

	current_input_stateB = get_logical_p2_channel_state(1);

	door_signal_interface.closed_flagsB[1]  = current_input_stateB;
	door_signal_interface.delay_runningB[1] = FALSE;
}	// timeout_job_door_signal_2_delay


void timeout_job_door_signal_3_delay(void)
{
	bool current_input_stateB;

	current_input_stateB = get_logical_p2_channel_state(2);

	door_signal_interface.closed_flagsB[2]  = current_input_stateB;
	door_signal_interface.delay_runningB[2] = FALSE;
}	// timeout_job_door_signal_3_delay


void timeout_job_door_signal_4_delay(void)
{
	bool current_input_stateB;

	current_input_stateB = get_logical_p2_channel_state(3);

	door_signal_interface.closed_flagsB[3]  = current_input_stateB;
	door_signal_interface.delay_runningB[3] = FALSE;
}	// timeout_job_door_signal_4_delay


void check_door_signal_input_state(byte door_signal_idx)
{
	bool current_input_stateB, to_use_current_input_stateB = TRUE;

	if( door_signal_idx < DOOR_CNT_MAX )
	{
		current_input_stateB = get_logical_p2_channel_state(door_signal_idx);

		if( door_signal_interface.statuses_synchronizedB )
			if( door_signal_interface.delay_runningB[door_signal_idx] )
				to_use_current_input_stateB = FALSE;
			else
			{
		   		if( !current_input_stateB                                &&
		   		    door_signal_interface.closed_flagsB[door_signal_idx] &&
		   		    door_signal_interface.opened_delay[door_signal_idx] > 0 )
				{
					// Door signal input indicates change closed to open and opened delay has to be applied.
					add_timeout_job(door_signal_interface.opened_delay[door_signal_idx], timeout_job_door_signal_delay_list[door_signal_idx]);
					door_signal_interface.delay_runningB[door_signal_idx] = TRUE;
					to_use_current_input_stateB = FALSE;
				}

		   		if( current_input_stateB                                  && 
		   		    !door_signal_interface.closed_flagsB[door_signal_idx] &&
		   		    door_signal_interface.closed_delay[door_signal_idx] > 0 )
				{
					// Door signal input indicates change open to closed and closed delay has to be applied.
					add_timeout_job(door_signal_interface.closed_delay[door_signal_idx], timeout_job_door_signal_delay_list[door_signal_idx]);
					door_signal_interface.delay_runningB[door_signal_idx] = TRUE;
					to_use_current_input_stateB = FALSE;
				}
			}

		if( to_use_current_input_stateB )
			door_signal_interface.closed_flagsB[door_signal_idx] = current_input_stateB;
	}
}	// check_door_signal_input_state


void update_door_status_gateway(byte door_signal_idx)
{
	if( door_signal_interface.statuses_synchronizedB )
	{
		if( door_signal_interface.closed_old_flagsB[door_signal_idx] && !door_signal_interface.closed_flagsB[door_signal_idx] )
		{
			// Door just opened.
			Door_Opened_FlagB[door_signal_idx] = TRUE;
			door_opened_event_cnt[door_signal_idx]++;
		}

		if( !door_signal_interface.closed_old_flagsB[door_signal_idx] && door_signal_interface.closed_flagsB[door_signal_idx] )
		{
			// Door just closed.
			Door_Closed_FlagB[door_signal_idx] = TRUE;
			door_closed_event_cnt[door_signal_idx]++;
		}
	}

	door_signal_interface.closed_old_flagsB[door_signal_idx] = door_signal_interface.closed_flagsB[door_signal_idx];
}	// update_door_status_gateway


void timer_job_check_door_contacts_gateway(void)
{
	bool door_contacts_ever_changedB;
	byte door_signal_idx;

	door_contacts_ever_changedB = door_contacts_ever_changed_gateway();
	
	if( !door_signal_interface.statuses_synchronizedB || door_contacts_ever_changedB )
		for( door_signal_idx = 0; door_signal_idx < DOOR_CNT_MAX; door_signal_idx++ )
		{
			check_door_signal_input_state(door_signal_idx);
			update_door_status_gateway(door_signal_idx);
		}	// end of "for( door_signal_idx = 0; door_signal_idx < DOOR_CNT_MAX; door_signal_idx++ )"

	door_signal_interface.statuses_synchronizedB = TRUE;

#ifdef J1708_PROTOCOL
	if( door_contacts_ever_changedB )
		Send_J1708_Msgs_On_Door_Status_ChangeB = TRUE;
#endif
	A21_Status.Sensor_Interface.Door_Contacts_Ever_ChangedB = door_contacts_ever_changedB;
}	// timer_job_check_door_contacts_gateway


void update_door_state_set_permission_flag_of_obc_interface_data(void)
{
    obc_interface_data.door_state_setting_permittedB = !door_contacts_ever_changed_gateway();
	if( !obc_interface_data.door_state_setting_permittedB )
		obc_interface_data.use_desired_door_statesB = FALSE;
}	// update_door_state_set_permission_flag_of_obc_interface_data


bool set_door_status_by_msg_gateway(byte fa_idx, bool closedB)
{
	bool setting_permittedB;
	byte fa_idx_2;

	if( fa_idx >= obc_interface_data.function_area_cnt )
		return(FALSE);

	// UIP or UJ1708IP door state messages are not accepted, if hardware door signals ever changed
	// during firmware runtime.
	setting_permittedB = obc_interface_data.door_state_setting_permittedB;
	if( setting_permittedB )
	{
		if( !obc_interface_data.use_desired_door_statesB )
		{
			// Synchronize desired door states with current hardware door states to avoid unwanted
			// door state changes for other functions areas.
			for( fa_idx_2 = 0; fa_idx_2 < obc_interface_data.function_area_cnt; fa_idx_2++ )
				obc_interface_data.function_area_list[fa_idx_2].door_closed_desiredB =
				obc_interface_data.function_area_list[fa_idx_2].door_closedB;

			obc_interface_data.use_desired_door_statesB = TRUE;
		}

		obc_interface_data.function_area_list[fa_idx].door_closed_desiredB = closedB;
		obc_interface_data.function_area_list[fa_idx].door_closed_desired_set_by_msgB = TRUE;
		obc_interface_data_backup.function_area_list[fa_idx].door_closed_desiredB = closedB;
		obc_interface_data_backup.function_area_list[fa_idx].door_closed_desired_set_by_msgB = TRUE;

		// Door jobs are called and response message is transmitted on receiving of valid door state
		// setting confirmation from master sensor.
#ifdef J1708_PROTOCOL
		Send_J1708_Msgs_On_Door_Status_ChangeB = FALSE;
#endif
	}

	A21_Status.Sensor_Interface.Door_States_Ever_Set_By_MsgB = TRUE;
	return(setting_permittedB);
}	// set_door_status_by_msg_gateway


void set_current_counter_state_by_msg_gateway(byte fa_idx, const counting_data_buffer_type *counting_data_buffer, bool counting_finishedB)
{
	byte counting_data_category_cnt, counting_data_category_idx;

	counting_finishedB;

	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	counting_data_category_cnt = counting_data_buffer->counting_data_category_cnt;
	if( counting_data_category_cnt > obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt )
		return;

	for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
	{
		obc_interface_data.function_area_list[fa_idx].current_counter_states_desired[counting_data_category_idx] = counting_data_buffer->counting_data[counting_data_category_idx];
		obc_interface_data_backup.function_area_list[fa_idx].current_counter_states_desired[counting_data_category_idx] = counting_data_buffer->counting_data[counting_data_category_idx];
	}
	obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_set_by_msgB = TRUE;
	obc_interface_data_backup.function_area_list[fa_idx].current_counter_states_desired_set_by_msgB = TRUE;
}	// set_current_counter_state_by_msg_gateway


void send_resp_to_door_status_set_cmd(void)
{
#ifdef UJ1708IP
	switch( Comm_ConfigData->communication_protocol )
	{
		// Door status setting requested for all function areas. Confirmation will be transmitted
		// for all function areas too.
		case PROT_ID_UNKNOWN	:
		case PROT_ID_IRMA		: send_irma_D_v10_door_state_set_resp_gateway();	break;

		// Door status setting requested for single function area. Confirmation will be transmitted
		// for this function area only. Function PIDSetDoorStatus_Response evaluates global variable
		// RecMID for this purpose.
		case PROT_ID_J1708		: PIDSetDoorStatus_Response();						break;
	}	// end of "switch( Comm_ConfigData->communication_protocol )"
#else
	// Door status setting requested for all function areas. Confirmation will be transmitted
	// for all function areas too.
	send_irma_D_v10_door_state_set_resp_gateway();
#endif
}	// send_resp_to_door_status_set_cmd


void send_resp_to_current_counter_state_set_cmd(void)
{
	// Current counter state setting requested for all function areas. Confirmation will be transmitted
	// for all function areas too.
	send_irma_C_v50_current_counter_state_set_resp_gateway();
}	// send_resp_to_current_counter_state_set_cmd


void update_desired_function_area_data(void)
{
	byte fa_idx, master_sen_idx;
	bool to_send_responseB = FALSE;

	// Adjust OBC interface data command confirmation flags according to CAN sensor list command
	// confirmation flags.
	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		master_sen_idx = obc_interface_data.function_area_list[fa_idx].sensor_assignments.sensor_list_indexes[0];

		if( obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB &&
			obc_interface_data.sensor_list[master_sen_idx].door_closed_desired_confirmedB )
		{
			obc_interface_data.function_area_list[fa_idx].door_closed_desired_confirmedB = TRUE;

			obc_interface_data.sensor_list[master_sen_idx].door_closed_desired_confirmedB = FALSE;
		}

		if( obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB &&
			obc_interface_data.sensor_list[master_sen_idx].counting_data_buffer_desired_confirmedB )
		{
			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_confirmedB = TRUE;

			obc_interface_data.sensor_list[master_sen_idx].counting_data_buffer_desired_confirmedB = FALSE;
		}
	}

	// Evaluate OBC interface data command confirmation flags.

	// Trigger transmission of response message to door status set command if possible.
	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		if( obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB )
		{
			to_send_responseB = TRUE;
			break;
		}
    if( to_send_responseB )
		for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
			if( obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB &&
			    !obc_interface_data.function_area_list[fa_idx].door_closed_desired_confirmedB )
			{
				to_send_responseB = FALSE;
				break;
			}
    if( to_send_responseB )
	{
		for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		{
			obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB    = FALSE;
			obc_interface_data.function_area_list[fa_idx].door_closed_desired_confirmedB = FALSE;
		}
		send_resp_to_door_status_set_cmd();

		to_send_responseB = FALSE;
	}

	// Trigger transmission of response message to current counter state set command if possible.
	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		if( obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB )
		{
			to_send_responseB = TRUE;
			break;
		}
    if( to_send_responseB )
		for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
			if( obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB &&
			    !obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_confirmedB )
			{
				to_send_responseB = FALSE;
				break;
			}
    if( to_send_responseB )
	{
		for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		{
			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB    = FALSE;
			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_confirmedB = FALSE;
		}
		send_resp_to_current_counter_state_set_cmd();

		to_send_responseB = FALSE;
	}

	// Evaluate OBC interface data command flags.

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		// Make door status set command effective.
		if( obc_interface_data.function_area_list[fa_idx].door_closed_desired_set_by_msgB )
		{
			obc_interface_data.function_area_list[fa_idx].door_closed_desired_activeB    = TRUE;
			obc_interface_data.function_area_list[fa_idx].door_closed_desired_confirmedB = FALSE;

			obc_interface_data.function_area_list[fa_idx].door_closed_desired_set_by_msgB = FALSE;
		}

		// Make current counter state set command effective.
		if( obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_set_by_msgB )
		{
			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_activeB    = TRUE;
			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_confirmedB = FALSE;

			obc_interface_data.function_area_list[fa_idx].current_counter_states_desired_set_by_msgB = FALSE;
		}
	}
}	// update_desired_function_area_data


// Gateway door jobs are called by function update_obc_interface_data, if valid door state set
// confirmation was received from gateway sensor during preceeding IRMA 5 Sensor communication cycle.
void call_fa_door_opened_jobs_gateway(byte fa_idx)
{
	all_doors_already_closedB = FALSE;

	obc_interface_data.function_area_list[fa_idx].countingB = TRUE;

	call_door_opened_jobs(fa_idx);

	// OBC interface data changed. Backup is updated as last step of function 
	// update_obc_interface_data.
}	// call_fa_door_opened_jobs_gateway


// Gateway door jobs are called by function update_obc_interface_data, if valid door state set
// confirmation was received from gateway sensor during preceeding IRMA 5 Sensor communication cycle.
void call_fa_door_closed_jobs_gateway(byte fa_idx)
{
	call_door_closed_jobs(fa_idx);
}	// call_fa_door_closed_jobs_gateway


// UIP and J1708 IRMA protocols:
// Setting of door state on receiving of proper message is supported but door jobs are used only,
// if recovery mode is not active.

// Gateway door jobs are called by function update_obc_interface_data, if valid door state set
// confirmation was received from gateway sensor during preceeding IRMA 5 Sensor communication cycle.
void call_fa_door_jobs_gateway(byte fa_idx)
{
	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	if( obc_interface_data.function_area_list[fa_idx].first_call_of_fa_door_jobs_doneB )
	{
		// On first call of door jobs flags
		// obc_interface_data.function_area_list[fa_idx].door_closed_oldB are equal to
		// initial door states of IRMA 5 sensors not controlled by gateway (closed after power-on).
		if( obc_interface_data.function_area_list[fa_idx].door_closed_oldB &&
		    !obc_interface_data.function_area_list[fa_idx].door_closedB )
		{
			call_fa_door_opened_jobs_gateway(fa_idx);

			return;
		}

		if( !obc_interface_data.function_area_list[fa_idx].door_closed_oldB &&
		    obc_interface_data.function_area_list[fa_idx].door_closedB )
		{
			call_fa_door_closed_jobs_gateway(fa_idx);

			return;
		}
	}
	else
	{
		// Ensure that proper door jobs are called on first sensor door status update.
		// Because initialization of ASC0 communication is done a f t e r finishing of
		// first can sensor communication cycle, adequate sequence has to be placed in
		// ASC0 communication protocol functions, if door jobs are used.
		if( obc_interface_data.function_area_list[fa_idx].door_closedB )
			call_fa_door_closed_jobs_gateway(fa_idx);
		else
			call_fa_door_opened_jobs_gateway(fa_idx);

		obc_interface_data.function_area_list[fa_idx].first_call_of_fa_door_jobs_doneB = TRUE;
	}

	// OBC interface data changed. Backup is updated as last step of function 
	// update_obc_interface_data.
}	// call_fa_door_jobs_gateway


void call_all_fa_door_jobs_gateway(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		call_fa_door_jobs_gateway(fa_idx);
}	// call_all_fa_door_jobs_gateway


bool Add_Counting_Finished_Job(void huge *ptr)
{
	byte n = 0;

	while( counting_finished_job[n] != NULL && counting_finished_job[n] != ptr && n < COUNTING_FINISHED_JOBS_MAX )
		n++;
	if( n >= COUNTING_FINISHED_JOBS_MAX )
		return(FALSE);
	counting_finished_job[n] = ptr;
	return(TRUE);
}


bool Remove_Counting_Finished_Job(void huge *ptr)
{
	byte n = 0;
	byte m;

	while( counting_finished_job[n] != ptr && n < COUNTING_FINISHED_JOBS_MAX )
		n++;
	if( n >= COUNTING_FINISHED_JOBS_MAX )
		return(FALSE);
	// Restore coherence of list if lost by job removal:
	for( m = n; m < COUNTING_FINISHED_JOBS_MAX - 1; m++ )
	{
		if( counting_finished_job[m + 1] == NULL )
			break;
		counting_finished_job[m] = counting_finished_job[m + 1];
	}
	counting_finished_job[m] = NULL;
	return(TRUE);
}


void call_counting_finished_jobs(byte fa_idx)
{
	byte n = 0;

	while( counting_finished_job[n] != NULL && n < COUNTING_FINISHED_JOBS_MAX )
	{
		(counting_finished_job[n])(fa_idx);
		n++;
	}
}


void call_counting_finished_jobs_gateway(byte fa_idx)
{
	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	if( obc_interface_data.function_area_list[fa_idx].new_counting_result_availableB );
		call_counting_finished_jobs(fa_idx);
}	// call_counting_finished_jobs_gateway


// Gateway counting finished jobs are called by function update_obc_interface_data, if flag
// obc_interface_data.function_area_list[].counting_finishedB was set on update of obc interface data.
void call_all_counting_finished_jobs_gateway(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		if( obc_interface_data.function_area_list[fa_idx].counting_finishedB )
		{
			obc_interface_data.function_area_list[fa_idx].countingB = FALSE;

			// Inherent counting finished job.
			update_obc_counting_result_buffer_and_fa_status_buffer(fa_idx);

			call_counting_finished_jobs_gateway(fa_idx);

			obc_interface_data.function_area_list[fa_idx].counting_finishedB = FALSE;
		}

	// OBC interface data changed. Backup is updated as last step of function 
	// update_obc_interface_data.
}	// call_all_counting_finished_jobs_gateway


void clear_obc_interface_counting_result_buffer(byte fa_idx)
{
	byte counting_data_category_cnt, counting_data_category_idx;

	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
	for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
	{
		obc_interface_data.function_area_list[fa_idx].new_counting_result_bufferedB = FALSE;

		obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].in  = 0;
		obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].out = 0;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// clear_obc_interface_counting_result_buffer


byte get_idx_of_function_area_in_obc_interface_data(word fa_addr)
{
	byte fa_idx;
	bool foundB = FALSE;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
	{
		foundB = fa_addr == obc_interface_data.function_area_list[fa_idx].address;
		if( foundB )
			break;
	}

	if( foundB )
		return(fa_idx);
	else
		return(0xFF);
}	// get_idx_of_function_area_in_obc_interface_data


void init_counting_data_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer)
{
	byte counting_data_category_cnt, counting_data_category_idx, category_id;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		return;
	}

	counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
	for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
	{
		category_id = obc_interface_data.function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id;
		counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
		counting_data_buffer->counting_data[counting_data_category_idx].in          = 0;
		counting_data_buffer->counting_data[counting_data_category_idx].out         = 0;
	}
	counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
}	// init_counting_data_buffer


void get_current_counter_states(byte fa_idx, counting_data_buffer_type *counting_data_buffer)
{
	byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		return;
	}

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			category_id = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].category_id;
			in          = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].in;
			out         = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].out;
			counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
			counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
			counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
		}
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
	}
	else
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			category_id = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id;
			in          = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in;
			out         = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out;
			counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
			counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
			counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
		}
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
	}
}	// get_current_counter_states


bool get_new_counting_result(byte fa_idx, counting_data_buffer_type *counting_data_buffer)
{
	bool new_counting_result_availableB;
    byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		return(FALSE);
	}

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		new_counting_result_availableB = obc_interface_data.function_area_list[fa_idx].new_counting_result_available_test_modeB;
		if( new_counting_result_availableB )
		{
			counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data_category_cnt;
			for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
			{
				category_id = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].category_id;
				in          = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].in;
				out         = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx].out;
				counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
				counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
				counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
			}
			counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
			obc_interface_data.function_area_list[fa_idx].new_counting_result_available_test_modeB = FALSE;
		}
		else
			init_counting_data_buffer(fa_idx, counting_data_buffer);
	}
	else
	{
		new_counting_result_availableB = obc_interface_data.function_area_list[fa_idx].new_counting_result_availableB;
		if( new_counting_result_availableB )
		{
			counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
			for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
			{
				category_id = obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id;
				in          = obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in;
				out         = obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out;
				counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
				counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
				counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
				category_id = obc_interface_data.function_area_list[fa_idx].counting_data_category_param_array[counting_data_category_idx].category_id;
				obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].category_id = category_id;
				obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].in          = 0;
				obc_interface_data.function_area_list[fa_idx].last_counting_result[counting_data_category_idx].out         = 0;
			}
			counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
			obc_interface_data.function_area_list[fa_idx].new_counting_result_availableB = FALSE;
		}
		else
			init_counting_data_buffer(fa_idx, counting_data_buffer);
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();

	return(new_counting_result_availableB);
}	// get_new_counting_result


void get_total_counter_states(byte fa_idx, counting_data_buffer_type *counting_data_buffer)
{
	byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		return;
	}

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data_category_cnt;
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
	}
	else
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			category_id  = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].category_id;
			in           = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].in;
			out          = obc_interface_data.function_area_list[fa_idx].current_counter_states[counting_data_category_idx].out;
			in          += obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].in;
			out         += obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].out;
			counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
			counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
			counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
		}
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
	}
}	// get_total_counter_states


void update_obc_counting_result_buffer_and_fa_status_buffer(byte fa_idx)
{
	bool new_counting_result_availableB;
	counting_data_buffer_type counting_data_buffer;
	byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;
	fa_status_gateway_type fa_status_buffer;

	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	new_counting_result_availableB = get_new_counting_result(fa_idx, &counting_data_buffer);
	if( new_counting_result_availableB )
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		if( obc_interface_data.function_area_list[fa_idx].new_counting_result_bufferedB )
		{
			for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
			{
				category_id = counting_data_buffer.counting_data[counting_data_category_idx].category_id;
				in          = counting_data_buffer.counting_data[counting_data_category_idx].in;
				out         = counting_data_buffer.counting_data[counting_data_category_idx].out;
				obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].category_id  = category_id;
				obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].in          += in;
				obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].out         += out;
			}

			fa_status_buffer = obc_interface_data.function_area_list[fa_idx].fa_status_buffer;
			merge_fa_statuses_gateway(&fa_status_buffer, obc_interface_data.function_area_list[fa_idx].fa_status);
			obc_interface_data.function_area_list[fa_idx].fa_status_buffer = fa_status_buffer;
		}
		else
		{
			for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
				obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx] = counting_data_buffer.counting_data[counting_data_category_idx];
			
			obc_interface_data.function_area_list[fa_idx].fa_status_buffer = obc_interface_data.function_area_list[fa_idx].fa_status;
		}

		init_fa_status_gateway_var(&obc_interface_data.function_area_list[fa_idx].fa_status);

		obc_interface_data.function_area_list[fa_idx].new_counting_result_bufferedB = TRUE;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// update_obc_counting_result_buffer_and_fa_status_buffer


void get_buffered_counting_result_and_fa_status_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer, byte *fa_status_byte)
{
	byte counting_data_category_cnt, counting_data_category_idx, category_id;
	word in, out;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		*fa_status_byte = 0;
		return;
	}

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data_category_cnt;
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;
	}
	else
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			category_id = obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].category_id;
			in          = obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].in;
			out         = obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].out;
			counting_data_buffer->counting_data[counting_data_category_idx].category_id = category_id;
			counting_data_buffer->counting_data[counting_data_category_idx].in          = in;
			counting_data_buffer->counting_data[counting_data_category_idx].out         = out;
		}
		counting_data_buffer->counting_data_category_cnt = counting_data_category_cnt;

		*fa_status_byte = get_fa_status_byte_gateway(obc_interface_data.function_area_list[fa_idx].fa_status_buffer);
	}
}	// get_buffered_counting_result_and_fa_status_buffer


bool query_buffered_counting_result_and_fa_status_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer, byte *fa_status_byte)
{
	bool new_counting_result_bufferedB;
	byte counting_data_category_cnt, counting_data_category_idx;

	if( fa_idx >= obc_interface_data.function_area_cnt )
	{
		memset(counting_data_buffer, 0, sizeof(*counting_data_buffer));
		*fa_status_byte = 0;
		return(FALSE);
	}

	new_counting_result_bufferedB = obc_interface_data.function_area_list[fa_idx].new_counting_result_bufferedB;
	obc_interface_data.function_area_list[fa_idx].new_counting_result_bufferedB = FALSE;

	if( new_counting_result_bufferedB )
	{
		get_buffered_counting_result_and_fa_status_buffer(fa_idx, counting_data_buffer, fa_status_byte);

		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
			obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].in  = 0;
			obc_interface_data.function_area_list[fa_idx].counting_result_buffer[counting_data_category_idx].out = 0;
		}
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();

	return(new_counting_result_bufferedB);
}	// query_buffered_counting_result_and_fa_status_buffer


void get_counting_data_category_sums(counting_data_buffer_type counting_data_buffer, word *in, word *out)
{
#ifndef OBC_COUNT_CATEGORY_1_ONLY
    byte counting_data_category_cnt, counting_data_category_idx;
#endif

#ifdef OBC_COUNT_CATEGORY_1_ONLY
	*in  = counting_data_buffer.counting_data[0].in;
	*out = counting_data_buffer.counting_data[0].out;
#else
	counting_data_category_cnt = counting_data_buffer.counting_data_category_cnt;

	*in  = 0;
	*out = 0;
	for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
	{
		*in  += counting_data_buffer.counting_data[counting_data_category_idx].in;
		*out += counting_data_buffer.counting_data[counting_data_category_idx].out;
	}
#endif
}	// get_counting_data_category_sums


#if defined(SW4) || defined(SW2)
void Get_Persons_Counter_SW2(bool *person_in, bool *person_out)
{
	*person_in  = FALSE;
	*person_out = FALSE;

	if( obc_interface_data.Persons_In_Counter_SW[0] > 0x1000 )	// counter underflow ?
		obc_interface_data.Persons_In_Counter_SW[0] = 0;
	if( obc_interface_data.Persons_Out_Counter_SW[0] > 0x1000 )
		obc_interface_data.Persons_Out_Counter_SW[0] = 0;

	if( obc_interface_data.Persons_In_Counter_SW[0] > 0 )
	{
		*person_in = TRUE;
		obc_interface_data.Persons_In_Counter_SW[0]--;
	}
	if( obc_interface_data.Persons_Out_Counter_SW[0] > 0 )
	{
		*person_out = TRUE;
		obc_interface_data.Persons_Out_Counter_SW[0]--;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// Get_Persons_Counter_SW2
#endif


#ifdef SW4
void Get_Persons_Counter_SW4(bool *person_in, bool *person_out)
{
	*person_in  = FALSE;
	*person_out = FALSE;

	if( obc_interface_data.Persons_In_Counter_SW[1] > 0x1000 )	// counter underflow ?
		obc_interface_data.Persons_In_Counter_SW[1] = 0;
	if( obc_interface_data.Persons_Out_Counter_SW[1] > 0x1000 )
		obc_interface_data.Persons_Out_Counter_SW[1] = 0;

	if( obc_interface_data.Persons_In_Counter_SW[1] > 0 )
	{
		*person_in = TRUE;
		obc_interface_data.Persons_In_Counter_SW[1]--;
	}
	if( obc_interface_data.Persons_Out_Counter_SW[1] > 0 )
	{
		*person_out = TRUE;
		obc_interface_data.Persons_Out_Counter_SW[1]--;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// Get_Persons_Counter_SW4
#endif


bool commu_cycle_for_function_area_without_error(byte fa_idx)
{
	byte sen_cnt, sen_idx, sen_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;
	bool resultB = TRUE;

	if( fa_idx < obc_interface_data.function_area_cnt )
	{
		sen_cnt = obc_interface_data.function_area_list[fa_idx].actual_sensor_cnt;
		sensor_assignments_ptr = &obc_interface_data.function_area_list[fa_idx].sensor_assignments;

		for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
		{
			sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];
			if( sen_list_idx < obc_interface_data.sensor_cnt )
			{
				if( obc_interface_data.sensor_list[sen_list_idx].commu_cycle_failedB )
					resultB = FALSE;
				if( obc_interface_data.sensor_list[sen_list_idx].error_msg_receivedB )
					resultB = FALSE;
			}
		}
	}

	return(resultB);
}	// commu_cycle_for_function_area_without_error


bool function_area_without_communication_error(byte fa_idx)
{
	byte sen_cnt, sen_idx, sen_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;
	bool resultB = TRUE;

	if( fa_idx < obc_interface_data.function_area_cnt )
	{
		sen_cnt = obc_interface_data.function_area_list[fa_idx].actual_sensor_cnt;
		sensor_assignments_ptr = &obc_interface_data.function_area_list[fa_idx].sensor_assignments;

		for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
		{
			sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];
			if( sen_list_idx < obc_interface_data.sensor_cnt )
			{
				if( obc_interface_data.sensor_list[sen_list_idx].ever_commu_cycle_failedB )
					resultB = FALSE;
				if( obc_interface_data.sensor_list[sen_list_idx].ever_error_msg_receivedB )
					resultB = FALSE;
			}
		}
	}

	return(resultB);
}	// function_area_without_communication_error


void init_fa_status_gateway_var(fa_status_gateway_type *fa_status_gateway)
{
	fa_status_gateway->initialization_errorB = FALSE;
	fa_status_gateway->runtime_errorB        = FALSE;
	fa_status_gateway->firmware_mismatchB    = FALSE;
	fa_status_gateway->sabotageB             = FALSE;
}	// init_fa_status_gateway_var


void update_fa_status_gateway_var(byte fa_idx, fa_status_gateway_type *fa_status_gateway)
{
	byte sen_cnt, sen_idx, sen_list_idx;
	sensor_assignments_type *sensor_assignments_ptr;

	if( fa_idx < obc_interface_data.function_area_cnt )
	{
		if( obc_interface_data.function_area_list[fa_idx].ever_static_data_invalidB )
			fa_status_gateway->initialization_errorB = TRUE;

		if( obc_interface_data.function_area_list[fa_idx].ever_dynamic_data_invalidB )
			fa_status_gateway->runtime_errorB = TRUE;

		sen_cnt = obc_interface_data.function_area_list[fa_idx].actual_sensor_cnt;
		sensor_assignments_ptr = &obc_interface_data.function_area_list[fa_idx].sensor_assignments;

		for( sen_idx = 0; sen_idx < sen_cnt; sen_idx++ )
		{
			sen_list_idx = sensor_assignments_ptr->sensor_list_indexes[sen_idx];
			if( sen_list_idx < obc_interface_data.sensor_cnt )
			{
				if( obc_interface_data.sensor_list[sen_list_idx].ever_static_data_invalidB )
					fa_status_gateway->initialization_errorB = TRUE;
				if( obc_interface_data.sensor_list[sen_list_idx].ever_firmware_mismatchedB )
					fa_status_gateway->initialization_errorB = TRUE;

				if( obc_interface_data.sensor_list[sen_list_idx].ever_commu_cycle_failedB )
					fa_status_gateway->runtime_errorB = TRUE;
				if( obc_interface_data.sensor_list[sen_list_idx].ever_error_msg_receivedB )
					fa_status_gateway->runtime_errorB = TRUE;
			}
		}
	}
}	// update_fa_status_gateway_var


void merge_fa_statuses_gateway(fa_status_gateway_type *fa_status_gateway, fa_status_gateway_type fa_status_gateway_2)
{
	fa_status_gateway->initialization_errorB = fa_status_gateway->initialization_errorB || fa_status_gateway_2.initialization_errorB;
	fa_status_gateway->runtime_errorB        = fa_status_gateway->runtime_errorB        || fa_status_gateway_2.runtime_errorB;
	fa_status_gateway->sabotageB             = fa_status_gateway->sabotageB             || fa_status_gateway_2.sabotageB;
	fa_status_gateway->firmware_mismatchB    = fa_status_gateway->firmware_mismatchB    || fa_status_gateway_2.firmware_mismatchB;
}	// merge_fa_statuses_gateway


byte get_fa_status_byte_gateway(fa_status_gateway_type fa_status_gateway)
{
	byte status_byte = 0;

	if( fa_status_gateway.initialization_errorB )
		status_byte |= BIT0;
	if( fa_status_gateway.runtime_errorB )
		status_byte |= BIT1;
	if( fa_status_gateway.sabotageB )
		status_byte |= BIT2;
	if( fa_status_gateway.firmware_mismatchB )
		status_byte |= BIT3;

	return(status_byte);
}	// get_fa_status_byte_gateway


byte get_current_fa_status_byte_gateway(byte fa_idx)
{
    fa_status_gateway_type fa_status_gateway;
	byte status_byte = 0;

	if( fa_idx < obc_interface_data.function_area_cnt )
	{
		init_fa_status_gateway_var(&fa_status_gateway);

		update_fa_status_gateway_var(fa_idx, &fa_status_gateway);

		status_byte = get_fa_status_byte_gateway(fa_status_gateway);
	}

	return(status_byte);
}	// get_current_fa_status_byte_gateway


#ifndef NO_LOGGER
void counting_finished_job_logger(byte fa_idx)
{
	log_counting_result(fa_idx);
}	// counting_finished_job_logger
#endif


// Gateway door jobs are called by function update_obc_interface_data, if valid door state set
// confirmation was received from gateway sensor during preceeding IRMA 5 Sensor communication cycle.
void door_opened_job_obc_interface_test_mode(byte fa_idx)
{
	byte counting_data_category_cnt, counting_data_category_idx, limit;
	counting_data_type *counting_data_ptr;

	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		counting_data_category_cnt = obc_interface_data.function_area_list[fa_idx].counting_data_category_cnt;
		for( counting_data_category_idx = 0; counting_data_category_idx < counting_data_category_cnt; counting_data_category_idx++ )
		{
#ifndef OBC_NO_COUNT_CATEGORIES
			limit = 100 + 10 * counting_data_category_idx;
#else
			if( counting_data_category_idx == 0 )
				limit = 100;
			else
				limit = 0;
#endif

			counting_data_ptr = &obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode.counting_data[counting_data_category_idx];

			if( counting_data_ptr->in == 0 && counting_data_ptr->out == 0 )
		        // After start of OBC interface test mode all test mode counters are = 0.
        		// Initialization with start values is done on first call of test mode door opened
        		// job.
				counting_data_ptr->in = limit;
			else
			{
				if( counting_data_ptr->in == 0 || counting_data_ptr->out >= limit )
					counting_data_ptr->in = limit;
				else
					counting_data_ptr->in--;
				if( counting_data_ptr->out >= limit )
					counting_data_ptr->out = 0;
				else
					counting_data_ptr->out++;
			}
		}
	}

	// OBC interface data backup is updated as last step of function update_obc_interface_data.
}	// door_opened_job_obc_interface_test_mode


// Gateway door jobs are called by function update_obc_interface_data, if valid door state set
// confirmation was received from gateway sensor during preceeding IRMA 5 Sensor communication cycle.
void door_closed_job_obc_interface_test_mode(byte fa_idx)
{
	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
		obc_interface_data.function_area_list[fa_idx].new_counting_result_available_test_modeB = TRUE;

	// OBC interface data backup is updated as last step of function update_obc_interface_data.
}	// door_closed_job_obc_interface_test_mode

void start_obc_interface_test_mode(byte fa_idx)
{
	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

 	obc_interface_data.function_area_list[fa_idx].new_counting_result_available_test_modeB = FALSE;
	init_counting_data_buffer(fa_idx, &obc_interface_data.function_area_list[fa_idx].counting_data_buffer_test_mode);
	obc_interface_data.function_area_list[fa_idx].test_modeB = TRUE;
	if( !obc_interface_data.function_area_list[fa_idx].door_closedB )
        door_opened_job_obc_interface_test_mode(fa_idx);

	add_door_opened_job(door_opened_job_obc_interface_test_mode);
	add_door_closed_job(door_closed_job_obc_interface_test_mode);

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// start_obc_interface_test_mode


void stop_obc_interface_test_mode(byte fa_idx)
{
	if( fa_idx >= obc_interface_data.function_area_cnt )
		return;

	if( obc_interface_data.function_area_list[fa_idx].test_modeB )
	{
		remove_door_opened_job(door_opened_job_obc_interface_test_mode);
		remove_door_closed_job(door_closed_job_obc_interface_test_mode);

		obc_interface_data.function_area_list[fa_idx].test_modeB = FALSE;
	}

	// OBC interface data changed. Backup has to be updated.
	update_obc_interface_data_backup();
}	// stop_obc_interface_test_mode


void deactivate_obc_interface(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < obc_interface_data.function_area_cnt; fa_idx++ )
		stop_obc_interface_test_mode(fa_idx);
}	// deactivate_obc_interface


// Zero terminated string: At least FACnt * 5 byte to be allocated
// for *FAConfStr ("0" ... "255" for FA address).
void create_fa_conf_str_gateway(byte FACnt, char *FAConfStr)
{
	byte Len = 0, FAIdx, door_signal_idx;
	char c;

	FAConfStr[0] = '\0';

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
	{
		if( obc_interface_data.use_desired_door_statesB )
			c = 'C';
		else
		{
		    door_signal_idx = obc_interface_data.function_area_list[FAIdx].door_signal_idx;
			if( door_signal_interface.positive_logicB[door_signal_idx] )
				c = 'P';
			else
				c = 'N';
		}

		// Function sprintf writes terminating zero.
		sprintf(&FAConfStr[Len], "%u%c", obc_interface_data.function_area_list[FAIdx].address, c);
		Len = strlen(FAConfStr);

		// If FA configuration item is not the last one, terminating zero is
		// overwritten by '-'.
		if( FAIdx < FACnt - 1 )
			FAConfStr[Len++] = '-';
	}
}	// create_fa_conf_str_gateway
