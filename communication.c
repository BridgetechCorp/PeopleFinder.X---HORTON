/*==============================================================================================*
 |       Filename: communication.c                                                              |
 | Project/Module: A21, GATEWAY or OBC/module group Communication                               |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: General functions for module group Communication.	  						|
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167cr.h>
#include <stdio.h>
#include <string.h>

#include "opera_version.h"
#include "kernel_interface.h"
#include "communication.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define DOOR_JOBS_MAX					10


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
//#pragma class PR=COMM
//#pragma class CO=COMM
//#pragma class FB=COMMRAM
//#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
Configuration_Data_type *Comm_ConfigData;
Configuration_System_type *Comm_ConfigSystem;

Device_Status_type DeviceStatus;

void (*door_opened_jobs[DOOR_JOBS_MAX])(byte);
void (*door_closed_jobs[DOOR_JOBS_MAX])(byte);

// - Door_Status_Synchronized == FALSE:
//   Initial state set by call of init_device_status_a21. Meaning:
//   Variable DeviceStatus initialized by call of init_device_status_a21,
//   all flags closedB == TRUE, all flags countingB == FALSE.
// - Door_Status_Synchronized == TRUE:
//   State set on end of first call of check_door_contacts. Meaning:
//   Flags closedB of variable DeviceStatus synchronized with current state of assigned door
//   signal inputs.
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile boolean Door_Status_Synchronized;
// - First_Doors_CheckingB == TRUE:
//   Initial state set by call of init_device_status_a21. Meaning:
//   Avoid setting of flags Door_Opened_FlagB(s) if door is already open on firmware start-up
//   (change DeviceStatus.door[].closedB == TRUE => DeviceStatus.door[].closedB == FALSE
//   without change of according door signal input state).
// - First_Doors_CheckingB == FALSE:
//   State set on end of first call of check_door_contacts_and_call_door_jobs. Meaning:
//   According flag Door_Opened_FlagB[] is set on each change
//   DeviceStatus.door[].closedB == TRUE => DeviceStatus.door[].closedB == FALSE.
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile boolean First_Doors_CheckingB;
volatile boolean All_Doors_Already_ClosedB;
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile Door_Flags_Type door_closed_oldB;
// Flags Door_Opened_FlagB and Door_Closed_FlagB are intended for evaluation by run level jobs.
// Run level jobs have to be used, if door opened jobs or door closed jobs cannot be used because
// of data consistency.
// - Setting by function call_door_jobs on according door state change.
// - Resetting by run level job evaluating flag.
volatile Door_Flags_Type Door_Opened_FlagB;
volatile Door_Flags_Type Door_Closed_FlagB;
volatile word door_opened_event_cnt[FUNCTION_AREAS_MAX];
volatile word door_closed_event_cnt[FUNCTION_AREAS_MAX];
#ifdef J1708_PROTOCOL
volatile boolean Send_J1708_Msgs_On_Door_Status_ChangeB;
#endif

#ifdef GLORIA
boolean GLORIA_InitializedB;
	#ifdef DEVTEST
boolean GLORIA_BurnIn_ModeB;
	#endif
#endif

#ifdef SIMCA_MODE
boolean SimCa_ModeB;
byte Door_port_simca;
#endif


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/

/*----- Sector Information Data ----------------------------------------------------------------*/
const Sector_Information_type Communication_Info  = {
		MGC_NAME,   			// Module group name
		IRMAOPERAVER,			// Version
		IRMAOPERAREV,			// Revision
		IRMAOPERARELEASEDATE,	// Release date
		IRMAOPERABUILDNO, 		// Build no.
		IRMAOPERAINTERFACEVER,	// IRMA OPERA interface version
		IRMAOPERACRC32TEMPL		// Checksum template
		};


/*----- Implementation of Functions ------------------------------------------------------------*/
// SRAM class "COMMRAM" completely initialized with 0. Wordwise writing to memory.
void clear_comm_RAM(void)
{
	unsigned int *p;

	p = (unsigned int *)ADDR_COMMRAM;
	while( p <= (unsigned int *)ADDR_COMMRAM_END )
		*(p++) = 0;
}


void communication_init(void)
{
	clear_comm_RAM();
	// Pointer Comm_ConfigSystem is used in function init_global_var_comm.
	Configuration_Data_Alloc(&Comm_ConfigSystem, &Comm_ConfigData);
}	// communication_init


void clear_door_opened_flags(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
		Door_Opened_FlagB[fa_idx] = FALSE;
}	// clear_door_opened_flags


void clear_door_closed_flags(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
		Door_Closed_FlagB[fa_idx] = FALSE;
}	// clear_door_closed_flags


void init_device_status(void)
{
	byte n;

	DeviceStatus.driveB = FALSE;

	Door_Status_Synchronized = FALSE;
	First_Doors_CheckingB = TRUE;
	All_Doors_Already_ClosedB = TRUE;

	for( n = 0; n < FUNCTION_AREAS_MAX; n++ )
	{
		DeviceStatus.door[n].closedB = TRUE;
		DeviceStatus.door[n].countingB = FALSE;
		DeviceStatus.door[n].left.motionB = FALSE;
		DeviceStatus.door[n].left.opening = 0;
		DeviceStatus.door[n].right.motionB = FALSE;
		DeviceStatus.door[n].right.opening = 0;
		door_closed_oldB[n] = TRUE;
		Door_Opened_FlagB[n] = FALSE;
		Door_Closed_FlagB[n] = FALSE;
	}

	synch_with_door_contacts();
}	// init_device_status


void No_Job_Communication(void)
{
	return;
}	// No_Job_Communication


// Lists door_opened_jobs[] and door_closed_jobs[] are referenced in interrupt service routine
// T1INT_handler. Furthermore functions to change these lists may be called by T1INT_handler. 
// Complete procedure from index search to pointer assignment must n o t be
// interrupted by T1INT_handler to avoid inconsistencies.

void init_door_jobs(void)
{
	byte n;

	for( n = 0; n < DOOR_JOBS_MAX; n++ )
	{
		door_opened_jobs[n] = NULL;
		door_closed_jobs[n] = NULL;
	}
}	// init_door_jobs


boolean add_door_opened_job(void  *ptr)
{
	byte n = 0;

	// Door jobs are called from "T1INT_handler".
	disable_timer_and_timeout_jobs();
	while( door_opened_jobs[n] != NULL && door_opened_jobs[n] != ptr && n < DOOR_JOBS_MAX )
		n++;
	if( n >= DOOR_JOBS_MAX )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	door_opened_jobs[n] = ptr;
	enable_timer_and_timeout_jobs();
	return(TRUE);
}	// add_door_opened_job


boolean add_door_closed_job(void  *ptr)
{
	byte n = 0;

	// Door jobs are called from "T1INT_handler".
	disable_timer_and_timeout_jobs();
	while( door_closed_jobs[n] != NULL && door_closed_jobs[n] != ptr && n < DOOR_JOBS_MAX )
		n++;
	if( n >= DOOR_JOBS_MAX )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	door_closed_jobs[n] = ptr;
	enable_timer_and_timeout_jobs();
	return(TRUE);
}	// add_door_closed_job


boolean remove_door_opened_job(void *ptr)
{
	byte n = 0;
	byte m;

	// Door jobs are called from "T1INT_handler".
	disable_timer_and_timeout_jobs();
	while( door_opened_jobs[n] != ptr && n < DOOR_JOBS_MAX )
		n++;
	if( n >= DOOR_JOBS_MAX )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	// Restore coherence of list if lost by job removal:
	for( m = n; m < DOOR_JOBS_MAX - 1; m++ )
	{
		if( door_opened_jobs[m + 1] == NULL )
			break;
		door_opened_jobs[m] = door_opened_jobs[m + 1];
	}
	door_opened_jobs[m] = NULL;
	enable_timer_and_timeout_jobs();
	return(TRUE);
}	// remove_door_opened_job


boolean remove_door_closed_job(void *ptr)
{
	byte n = 0;
	byte m;

	// Door jobs are called from "T1INT_handler".
	disable_timer_and_timeout_jobs();
	while( door_closed_jobs[n] != ptr && n < DOOR_JOBS_MAX )
		n++;
	if( n >= DOOR_JOBS_MAX )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	// Restore coherence of list if lost by job removal:
	for( m = n; m < DOOR_JOBS_MAX - 1; m++ )
	{
		if( door_closed_jobs[m + 1] == NULL )
			break;
		door_closed_jobs[m] = door_closed_jobs[m + 1];
	}
	door_closed_jobs[m] = NULL;
	enable_timer_and_timeout_jobs();
	return(TRUE);
}	// remove_door_closed_job


//|-------------------------------------------------------------------------------------|
//| Function: call_door_opened_jobs														|
//|-------------------------------------------------------------------------------------|
//| Typically called by timer job !														|
//|-------------------------------------------------------------------------------------|
void call_door_opened_jobs(byte door_idx)
{
	byte n = 0;

	while( door_opened_jobs[n] != NULL && n < DOOR_JOBS_MAX )
	{
		(door_opened_jobs[n])(door_idx);
		n++;
	}
}	// call_door_opened_jobs


//|-------------------------------------------------------------------------------------|
//| Function: call_door_closed_jobs														|
//|-------------------------------------------------------------------------------------|
//| Typically called by timer job !														|
//|-------------------------------------------------------------------------------------|
void call_door_closed_jobs(byte door_idx)
{
	byte n = 0;

	while( door_closed_jobs[n] != NULL && n < DOOR_JOBS_MAX )
	{
		(door_closed_jobs[n])(door_idx);
		n++;
	}
}	// call_door_closed_jobs


#ifdef EMC
// EMC software uses only 1 function area but supervision of 4 door signals P2.00, P2.01, P2.02 and P2.03
// is necessary.
boolean Door_Contacts_Ever_Changed(void)
{
	byte fa_idx;
	boolean changedB = FALSE;

	for( fa_idx = 0; fa_idx < FACntEMC; fa_idx++ )
	{
		if( Comm_ConfigData->function_area[0/*fa_idx*/].door.input[0].type.door_switchB)
			if( ChannelState_ChangedB[fa_idx/*Comm_ConfigData->function_area[fa_idx].door.input[0].channel_no - 1*/] )
				changedB = TRUE;

		if( Comm_ConfigData->function_area[0/*fa_idx*/].door.input[1].type.door_switchB)
			if( ChannelState_ChangedB[fa_idx/*Comm_ConfigData->function_area[fa_idx].door.input[1].channel_no - 1*/] )
				changedB = TRUE;
	}
	return(changedB);
}	// Door_Contacts_Ever_Changed
#else	// else of "#ifdef EMC"
boolean Door_Contacts_Ever_Changed(void)
{
	byte fa_idx;
	boolean changedB = FALSE;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
	{
		if( Comm_ConfigData->function_area[fa_idx].door.input[0].type.door_switchB )
			if( ChannelState_ChangedB[Comm_ConfigData->function_area[fa_idx].door.input[0].channel_no - 1] )
				changedB = TRUE;

		if( Comm_ConfigData->function_area[fa_idx].door.input[1].type.door_switchB )
			if( ChannelState_ChangedB[Comm_ConfigData->function_area[fa_idx].door.input[1].channel_no - 1] )
				changedB = TRUE;
	}
	return(changedB);
}	// Door_Contacts_Ever_Changed
#endif	// end of "#ifdef EMC"


void Update_Door_Status(byte fa_idx, boolean closedB)
{
	// DeviceStatus.door[] is updated by timer job check_door_contacts and
	// may be queried in interrupt service routines.
	Save_ILVL_and_Disable_All_INTs();
	DeviceStatus.door[fa_idx].closedB = closedB;
	if( DeviceStatus.door[fa_idx].closedB )
	{
		DeviceStatus.door[fa_idx].left.opening = 0;
		DeviceStatus.door[fa_idx].right.opening = 0;
	}
	else
	{
		DeviceStatus.door[fa_idx].left.opening = 100;
		DeviceStatus.door[fa_idx].right.opening = 100;
	}
	Restore_ILVL();
}	// Update_Door_Status


#ifdef SIMCA_MODE
char Get_Channel_simca(byte chan_no)
{
	ChannelState_ChangedB[chan_no] = TRUE;
	chan_no--;
	return(Door_port_simca >> chan_no & BIT0);
}
#endif


#ifdef EMC
// EMC software uses only 1 function area but supervision of 4 door signals P2.00, P2.01, P2.02 and P2.03
// is necessary. Special version of check_door_contacts is used to establish assignment of these 4 signals
// to DeviceStatus.door[0].closedB:
// DeviceStatus.door[0].closedB = FALSE if P2.00 == 1 && P2.01 == 1 && P2.02 == 1 && P2.03 == 1 (desired state)
// DeviceStatus.door[0].closedB = TRUE  if P2.00 == 0 || P2.01 == 0 || P2.02 == 0 || P2.03 == 0 (disturbed state)
void check_door_contacts(void)
{
	boolean Door_Contacts_Ever_ChangedB;
	byte fa_idx;
	byte i;
	boolean door_switch1B;
	boolean door_switch2B;
	boolean closed_new1B;
	boolean closed_new2B;
	boolean closedB;
	boolean one_door_closedB = FALSE;

	Door_Contacts_Ever_ChangedB = Door_Contacts_Ever_Changed();	// Special EMC version needed.
	//
	for( fa_idx = 0; fa_idx < FACntEMC; fa_idx++ )
	{
		door_switch1B = Comm_ConfigData->function_area[0].door.input[0].type.door_switchB;
		door_switch2B = Comm_ConfigData->function_area[0].door.input[1].type.door_switchB;
		if( door_switch1B || door_switch2B )
		{
			closed_new1B = TRUE;
			closed_new2B = TRUE;
			closedB = TRUE;
			if( door_switch1B )
			{
				// Check channel numbers 1...4: P2.00, P2.01, P2.02 and P2.03
				i = fa_idx + 1;	// Comm_ConfigData->function_area[fa_idx].door.input[0].channel_no;
				if( Comm_ConfigData->function_area[0].door.input[0].invert_signalB )
				    closed_new1B = Get_Channel(i);	// Logically positive.
				else
				    closed_new1B = !Get_Channel(i);	// Logically negative.
			}
			if( door_switch2B )
			{
				// Check channel numbers 1...4: P2.00, P2.01, P2.02 and P2.03
				i = fa_idx + 1;	// Comm_ConfigData->function_area[fa_idx].door.input[1].channel_no;
				if( Comm_ConfigData->function_area[0].door.input[1].invert_signalB )
					closed_new2B = Get_Channel(i);	// Logically positive.
				else
					closed_new2B = !Get_Channel(i);	// Logically negative.
			}
			if( !closed_new1B || !closed_new2B )
				closedB = FALSE;
			one_door_closedB |= closedB;
		}	// end of if( door_switch1B || door_switch2B )
	}	// end of for( fa_idx = 0; fa_idx < 4; fa_idx++ )
	//
	if( !Door_Status_Synchronized || Door_Contacts_Ever_ChangedB )
		Update_Door_Status(0, one_door_closedB);

	A21_Status.Sensor_Interface.Door_Contacts_Ever_ChangedB = Door_Contacts_Ever_ChangedB;
	Door_Status_Synchronized = TRUE;
}	// check_door_contacts
#else	// else of "#ifdef EMC"
void check_door_contacts(void)
{
	boolean Door_Contacts_Ever_ChangedB;
	byte fa_idx;
	byte i;
	boolean door_switch1B;
	boolean door_switch2B;
	boolean closed_new1B;
	boolean closed_new2B;
	boolean closedB;

	Door_Contacts_Ever_ChangedB = Door_Contacts_Ever_Changed();
	//
	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
	{
		door_switch1B = Comm_ConfigData->function_area[fa_idx].door.input[0].type.door_switchB;
		door_switch2B = Comm_ConfigData->function_area[fa_idx].door.input[1].type.door_switchB;
		if( door_switch1B || door_switch2B )
		{
			closed_new1B = TRUE;
			closed_new2B = TRUE;
			closedB = TRUE;
			if( door_switch1B )
			{
				i = Comm_ConfigData->function_area[fa_idx].door.input[0].channel_no;
				if( Comm_ConfigData->function_area[fa_idx].door.input[0].invert_signalB )
				{
	#if defined(SERIAL_SENSOR) || !defined(SIMCA_MODE)
				   	closed_new1B = Get_Channel(i);	// Logically positive.
	#endif
	#if defined(CAN_SENSOR) && defined(SIMCA_MODE)
				    if( !SimCa_ModeB )   	closed_new1B = Get_Channel(i);	// Logically positive.
				    else			     	closed_new1B = Get_Channel_simca(i);
	#endif
				}
				else
				{
	#if defined(SERIAL_SENSOR) || !defined(SIMCA_MODE)
				   	closed_new1B = !Get_Channel(i);	// Logically negative.
	#endif
	#if defined(CAN_SENSOR) && defined(SIMCA_MODE)
				    if( !SimCa_ModeB )   	closed_new1B = !Get_Channel(i);	// Logically negative.
				    else			     	closed_new1B = !Get_Channel_simca(i);
	#endif
				}
			}
			if( door_switch2B )
			{
				i = Comm_ConfigData->function_area[fa_idx].door.input[1].channel_no;
				if( Comm_ConfigData->function_area[fa_idx].door.input[1].invert_signalB )
					closed_new2B = Get_Channel(i);	// Logically positive.
				else
					closed_new2B = !Get_Channel(i);	// Logically negative.
			}
			if( !closed_new1B || !closed_new2B )
				closedB = FALSE;
			//
			if( !Door_Status_Synchronized || Door_Contacts_Ever_ChangedB )
				Update_Door_Status(fa_idx, closedB);
		}	// end of "if( door_switch1B || door_switch2B )"
	}	// end of "for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )"

	A21_Status.Sensor_Interface.Door_Contacts_Ever_ChangedB = Door_Contacts_Ever_ChangedB;
	Door_Status_Synchronized = TRUE;
}	// check_door_contacts
#endif 	// end of "#ifdef EMC"


// Function "check_door_contacts" may not be a timer job during execution 
// of function "synch_with_door_contacts" or execution of timer jobs
// must be disabled by call of function "disable_timer_and_timeout_jobs" or
// by call of function "Save_ILVL_and_Disable_T1_and_lower_priority_INTs"
// before calling "synch_with_door_contacts".
void synch_with_door_contacts(void)
{
	Door_Status_Synchronized = FALSE;
	check_door_contacts();
}	// synch_with_door_contacts


boolean Set_Door_Status(byte fa_idx, boolean closedB)
{
	boolean Setting_PermittedB;

	// UIP or UJ1708IP door state messages are not accepted if hardware door signals ever changed
	// during firmware runtime.
    Setting_PermittedB = !Door_Contacts_Ever_Changed();

	if( Setting_PermittedB )
	{
		// Runtime check for SDIST4_AA21C_CX-6.00_IZZ_PV-4.54_T32 using TRACE32 dated 02/23/2010:
		// About 100us for both calls Update_Door_Status and call_door_jobs.
		disable_timer_and_timeout_jobs();
		Update_Door_Status(fa_idx, closedB);

		// UIP and J1708 IRMA protocols:
		// Setting of door state on receiving of proper message is supported but door jobs are
		// used only, if recovery mode is not active.
		call_door_jobs(FALSE);
		enable_timer_and_timeout_jobs();
	}

	return(Setting_PermittedB);
}	// Set_Door_Status


// To be called by timer or time-out job o n l y. Function All_Doors_Closed could return invalid
// result if interrupted by timer job check_door_contacts or timer job
// check_door_contacts_and_call_door_jobs.
boolean All_Doors_Closed(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
		if( !DeviceStatus.door[fa_idx].closedB )
			return(FALSE);

	return(TRUE);
}	// All_Doors_Closed


boolean Last_Door_Closed(void)
{
	if( All_Doors_Closed() )
		if( All_Doors_Already_ClosedB )
			return(FALSE);
		else
		{
			All_Doors_Already_ClosedB = TRUE;
			return(TRUE);
		}
	else
		return(FALSE);
}	// Last_Door_Closed


// To be called by door open job o n l y.
// Always result TRUE returned on first call after firmware start-up because array
// door_closed_oldB is initialized with value TRUE.
boolean First_Door_Opened(void)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
		if( !door_closed_oldB[fa_idx] )
			return(FALSE);

	return(TRUE);
}	// First_Door_Opened


void call_door_jobs(boolean SendFlagB)
{
	byte fa_idx;

	for( fa_idx = 0; fa_idx < Comm_ConfigData->function_areas; fa_idx++ )
		/* Change of door state ?	*/
		if( door_closed_oldB[fa_idx] != DeviceStatus.door[fa_idx].closedB )
		{
#ifdef J1708_PROTOCOL
			Send_J1708_Msgs_On_Door_Status_ChangeB = SendFlagB;
#else
			SendFlagB;
#endif

			if( door_closed_oldB[fa_idx] )
			{
				/* Yes: Door was just opened.	*/
				All_Doors_Already_ClosedB = FALSE;
#ifdef SERIAL_SENSOR
				if(	Operation_Flags.A21S_UpdateIREDActivityB && First_Door_Opened() )
					PWM_Start();
#endif
				// Flag for counting must not be setted in installation mode (runlevel == 4).
				if( Get_Runlevel() == RUNLEVEL_APPL )
					DeviceStatus.door[fa_idx].countingB = TRUE;
				call_door_opened_jobs(fa_idx);
				// Avoid setting of Door_Opened_FlagB(s) if door is already open on
				// firmware start-up.
				if( !First_Doors_CheckingB )
				{
					Door_Opened_FlagB[fa_idx] = TRUE;
					door_opened_event_cnt[fa_idx]++;
				}
			}
			else
			{
				/* Yes: Door was just closed.	*/
#ifdef SERIAL_SENSOR
				if(	Operation_Flags.A21S_UpdateIREDActivityB && All_Doors_Closed() )
					PWM_Stop();
#endif
				call_door_closed_jobs(fa_idx);
				Door_Closed_FlagB[fa_idx] = TRUE;
				door_closed_event_cnt[fa_idx]++;
			}
			door_closed_oldB[fa_idx] =  DeviceStatus.door[fa_idx].closedB;
		}

	First_Doors_CheckingB = FALSE;
}	// call_door_jobs


//|-------------------------------------------------------------------------------------|
//| Function: check_door_contacts_and_call_door_jobs									|
//|-------------------------------------------------------------------------------------|
//| Called as timer job !																|
//|-------------------------------------------------------------------------------------|
void check_door_contacts_and_call_door_jobs(void)
{
	check_door_contacts();

	call_door_jobs(TRUE);
}	// check_door_contacts_and_call_door_jobs
