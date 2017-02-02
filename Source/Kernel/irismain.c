/*==============================================================================================*
 |       Filename: main.c                                                                       |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Main module of module group Kernel.                   						|
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "..\opera_version.h"
#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#include "serio_asc0.h"
#include "time.h"
#include "flash.h"
#include "peripherals.h"
#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
	#include "signal.h"
#endif
#include "crc.h"
#ifdef SERIAL_SENSOR
	#include "serial\sensor_serial.h"
#endif
#if defined(A21CL) && !defined(NO_LOGGER)
	#include "serio_ssc.h"
#endif
#include "..\communication_start.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Global Constants at Fixed Addresses ----------------------------------------------------*/
// Constants Kernel1_Info and Kernel2_Info cannot be located (E 270: absolute segmented element
// '...' cannot be located) by Tasking C for C166/ST10 v7.0r1, if memory class assignments are
// are placed before constant declarations. Same is true for Tasking C for C166/ST10 v8.8r1.

/*----- IRMA Opera Module Name -----------------------------------------------------------------*/
// Written by A21_ModuleManager.
const unsigned char Opera_Module_Name_ROM[60] _at(ADDR_OPERA_MODU_NAME) = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

/*----- Sector Information Data ----------------------------------------------------------------*/
const Sector_Information_type Kernel1_Info _at(ADDR_KERNEL1_INFO) = {
		MGK_NAME,				// Module group name
		IRMAOPERAVER,			// Version
		IRMAOPERAREV,			// Revision
		IRMAOPERARELEASEDATE,	// Release date
		IRMAOPERABUILDNO, 		// Build no.
		IRMAOPERAINTERFACEVER,	// IRMA OPERA interface version
		IRMAOPERACRC32TEMPL		// Checksum template
		};

const Sector_Information_type Kernel2_Info _at(ADDR_KERNEL2_INFO) = {
		MGK_NAME,				// Module group name
		IRMAOPERAVER,			// Version
		IRMAOPERAREV,			// Revision
		IRMAOPERARELEASEDATE,	// Release date
		IRMAOPERABUILDNO, 		// Build no.
		IRMAOPERAINTERFACEVER,	// IRMA OPERA interface version
		IRMAOPERACRC32TEMPL		// Checksum template
		};


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class CO=KERNEL
#pragma class PR=KERNEL
#pragma class FC=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
byte FlashSectorCnt;
SectorInformationTable_type SectorInformation[MAX_FLASH_SECTOR_CNT];

Operation_Flags_type Operation_Flags;

A21_Status_type A21_Status;

int (*RunLevel3Main)(void);
int (*RunLevel5Main)(void);

void (*runlevel_job[RUNLEVEL_JOB_SIZE])(void);

byte Runlevel;

// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile bool RunLevel_1_ServicedB;
bool Reset_Request_ReceivedB;

bool Recovery_Mode;									// TRUE after a failed update attempt

#if defined(CAN_SENSOR) || defined(GATEWAY)
byte can_sensor_init_status;
#endif


/*----- Function Prototypes --------------------------------------------------------------------*/
void hardware_setup(void);
void enable_can_emulation(void);
void runlevel_1_jobs(void);


/*----- Global Constants -----------------------------------------------------------------------*/
const Sector_Pointers_type Sector[FLASH_SECTOR_CNT_200] =
{
	{(byte huge *)ADDR_KERNEL1,       (byte huge *)ADDR_KERNEL1_END,       (Sector_Information_type *)ADDR_KERNEL1_INFO      }, 
	{(byte huge *)ADDR_CONFIG_DATA,   (byte huge *)ADDR_CONFIG_DATA_END,   (Sector_Information_type *)ADDR_CONFIG_DATA_INFO  }, 
	{(byte huge *)ADDR_STORAGE_DATA,  (byte huge *)ADDR_STORAGE_DATA_END,  (Sector_Information_type *)ADDR_STORAGE_DATA_INFO }, 
	{(byte huge *)ADDR_KERNEL2,       (byte huge *)ADDR_KERNEL2_END,       (Sector_Information_type *)ADDR_KERNEL2_INFO      }, 
	{(byte huge *)ADDR_COMMUNICATION, (byte huge *)ADDR_COMMUNICATION_END, (Sector_Information_type *)ADDR_COMMUNICATION_INFO}, 
	{(byte huge *)ADDR_APPLICATION1,  (byte huge *)ADDR_APPLICATION1_END,  (Sector_Information_type *)ADDR_APPLICATION1_INFO }, 
	{(byte huge *)ADDR_APPLICATION2,  (byte huge *)ADDR_APPLICATION2_END,  (Sector_Information_type *)ADDR_APPLICATION2_INFO } 
};


/*----- Implementation of Functions ------------------------------------------------------------*/
// SRAM class "KERNELRAM" completely initialized with 0. Wordwise writing to memory.
void clear_kernel_RAM(void)
{
	word huge *p;

	p = (word huge *)ADDR_KERNELRAM;
	while( p <= (word huge *)ADDR_KERNELRAM_END )
		*(p++) = 0;
}


void clear_Operation_Flags(void)
{
	byte *FlagBytePtr = (byte *)&Operation_Flags;

	*FlagBytePtr       = 0;
	*(FlagBytePtr + 1) = 0;
	*(FlagBytePtr + 2) = 0;
	*(FlagBytePtr + 3) = 0;
}	// clear_Operation_Flags


void init_global_var_kernel(void)
{
	Runlevel = 1;

	clear_Operation_Flags();
	// Default settings for A21C firmware operation (DIST4 only):
#if defined(CAN_SENSOR) && !defined(CXYO)
	Operation_Flags.A21C_UpdateDoorStateB    = TRUE; 	// current door state in CAN IRMA FA status message
//	#ifdef EMC
// 	Operation_Flags.A21C_NoTempTransmissionB = TRUE;	// no temperature transmission
//	#endif
//	Operation_Flags.A21C_ReqCANErrCntsB      = FALSE;	// do not request CAN error counters
//	Operation_Flags.A21C_MaxOf2CyclesB       = FALSE;	// mean value if DIST4 cycle time is 8ms
#endif
	// Default settings for A21S firmware operation:
#ifdef SERIAL_SENSOR
	#ifndef DEVTEST
	Operation_Flags.A21S_CheckNeutralErrorB  = TRUE;	// check equilibrium level for pyroelectric sensor elements
//	#else
//	Operation_Flags.A21S_CheckNeutralErrorB  = FALSE;	// do not check equilibrium level for pyroelectric sensor elements
	#endif
	Operation_Flags.A21S_UpdateIREDActivityB = FALSE;	// stop IRED clock if all doors are closed
#endif

#ifdef EMULATOR
	Init_Global_Var_Flash();
#endif
	init_global_var_peripherals();
	init_global_var_serio_asc0();
#ifdef SERIAL_SENSOR
	init_global_var_sensor_serial();
#endif
}


void No_Job_Kernel(void)
{
	return;
}


//|-------------------------------------------------------------------------------------|
//| Function: startup_delay																|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Tasking C for C166/ST10 v7.0r1 generates this C167CR machine code for for loop:		|
//|   L1:																				|
//|   nop																				|
//|   jmp L1																			|
//| fCPU = 16.0000MHz:																	|
//| - loop run time is about 1.3µs														|
//| - total delay is about 750ms  														|
//| fCPU = 18.4320MHz:																	|
//| fCPU = 24.0000MHz:																	|
//| To be called by function main only.													|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void startup_delay(void)
{
	dword n;

//	_putbit(TRUE, DP3, 10);
//	_putbit(FALSE, P3, 10);
#ifdef GATEWAY
	// Startverzögerung von mindestens 10s.
	for( n = 0; n < 8000000; n++ )
#else
	for( n = 0; n < 580000; n++ )
#endif
		_nop();
//	_putbit(TRUE, P3, 10);
}


//|-------------------------------------------------------------------------------------|
//| Function: hardware_setup															|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| This function is part of startup function __CSTART implemented in assembler source	|
//| file "cstartx.asm". Reset vector of C167CR points to function __CSTART. Function	|
//| hardware_setup is called immediately before EINIT.									|
//| Tasking EDE settings "CPU Configuration...", "Startup" are used to assign specific	|
//| startup function. Line "CpuCallEInit=_hardware_setup" is present in Tasking EDE		|
//| project file and option "CpuCallEInit" is activated.								|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void hardware_setup(void)
{
#if defined(CAN_SENSOR) || defined(GATEWAY)
	#ifdef EMULATOR
	enable_can_emulation();
	#endif
#endif
	buscon_setup();
}


//|-------------------------------------------------------------------------------------|
//| Function: enable_can_emulation														|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Connect CAN1 interface to certain port pins. Needed if A21 firmware runs on			|
//| Infineon C167E3 bondout chip utilized e.g. by in-circuit emulator Hitex DProbe167.	|
//| Value 000 has to be written to C167E3 IPC bit field (Interface Port Connection,		|
//| bits 10..08) to connect CAN1 interface to default port pins P4.5 and P4.6.			|
//| IPC bit field is placed in upper byte of CAN1 interrupt register at address 0xEF02.	|
//| INTID placed in lower byte is not changed when configuring port pins.				|
//| Refer to Hitex application note "C167E3 Module 002.pdf", page 4.					|
//| To be called by function hardware_setup only.										|
//| Input: non																			|
//| Output:	non																			|
//|-------------------------------------------------------------------------------------|
void enable_can_emulation(void)
{
	_putbit(TRUE, SYSCON, 2);  	// set bit XPEN == 1 (XBUS peripherals enabled)
	*(word *)0xEF00 |= 0x0040;	// set bit CCE == 1  (configuration change enabled)
	*(word *)0xEF02 &= 0x00FF;	// configure CAN_TXD = P4.6 and CAN_RXD = P4.5
	*(word *)0xEF00 &= 0xFFBF;	// set bit CCE == 0  (configuration change disabled)
}


void Read_Sector_Information(byte s)
{
	strncpy(SectorInformation[s].name, Sector[s].info->name, 15);
	SectorInformation[s].version     = Sector[s].info->version;
	SectorInformation[s].revision    = Sector[s].info->revision;
	strncpy(SectorInformation[s].date, Sector[s].info->date, 11);
	strncpy(SectorInformation[s].time, Sector[s].info->time, 8);
	SectorInformation[s].checksum    = Sector[s].info->checksum;
}


void Check_Sector(byte s)
{
	SectorInformation[s].checksum_ana = Get_CRC(Sector[s].start , Sector[s].end - sizeof(dword));

#ifndef NO_SECTOR_CHECK
	SectorInformation[s].okB = (SectorInformation[s].checksum == SectorInformation[s].checksum_ana);
#else
	SectorInformation[s].okB = TRUE;
#endif
}


void Check_Flash_Integrity(void)
{
	dword Source_Start_Addr, Dest_Start_Addr;
	word Length;
	int ErrNo;
	byte s;

	for( s = FIRST_SECTOR; s <= LAST_SECTOR_200; s++ )
	{
		Read_Sector_Information(s);
		Check_Sector(s);
	}

	if( !SectorValid(SECTOR_CONFIG_DATA) && SectorValid(SECTOR_STORAGE_DATA) &&
		strcmp(SectorInformation[SECTOR_STORAGE_DATA].name, "SoftwParams") == 0 )
	{
		ErrNo = Erase_Sector_No_Backup((byte huge *)ADDR_CONFIG_DATA);

		Source_Start_Addr = ADDR_STORAGE_DATA;
		Dest_Start_Addr   = ADDR_CONFIG_DATA;
		while( Source_Start_Addr <= ADDR_STORAGE_DATA_END && ErrNo == 0 )
		{
			Length = MIN(ADDR_STORAGE_DATA_END - Source_Start_Addr + 1, 0xFF);
			ErrNo = Program_Flash(Length, (byte _huge *)Source_Start_Addr, (byte _huge *)Dest_Start_Addr);
			Source_Start_Addr += Length;
			Dest_Start_Addr   += Length;
		}

		Read_Sector_Information(SECTOR_CONFIG_DATA);
		Check_Sector(SECTOR_CONFIG_DATA);
	}
}


// Functions to change list runlevel_job may be called by T1INT_handler. 
// Complete procedure from index search to pointer assignment must n o t be
// interrupted by T1INT_handler to avoid inconsistencies.

bool Add_Runlevel_Job(void huge *ptr)
{
	byte n = 0;

	disable_timer_and_timeout_jobs();
	while( runlevel_job[n] != NULL && runlevel_job[n] != ptr && n < RUNLEVEL_JOB_SIZE )
		n++;
	if( n >= RUNLEVEL_JOB_SIZE )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	runlevel_job[n] = ptr;
	enable_timer_and_timeout_jobs();	
	return(TRUE);
}	// Add_Runlevel_Job


// Only run level job on top of list runlevel_job[] may be removed by run level job.
bool Remove_Runlevel_Job(void huge *ptr)
{
	byte n = 0;
	byte m = 0;

	disable_timer_and_timeout_jobs();
	while( runlevel_job[n] != ptr && n < RUNLEVEL_JOB_SIZE )
		n++;
	if( n >= RUNLEVEL_JOB_SIZE )
	{
		enable_timer_and_timeout_jobs();
		return(FALSE);
	}
	// Restore coherence of list if lost by job removal:
	for( m = n; m < RUNLEVEL_JOB_SIZE - 1; m++ )
	{
		if( runlevel_job[m] == NULL )
			break;
		runlevel_job[m] = runlevel_job[m + 1];
	}
	runlevel_job[m] = NULL;
	enable_timer_and_timeout_jobs();	
	return(TRUE);
}	// Remove_Runlevel_Job


// Function Remove_Runlevel_Job may be called from run level job but call of other run level job
// may be left out during same call of Call_Runlevel_Jobs under these conditions:
// - Run level job from which function Remove_Runlevel_Job is called is not the last job within
//   list of run level jobs.
// - Removed run level job had lower or same position within list as removing job.
// Typically this is not a problem because run level job in question is called again during next
// call of Call_Runlevel_Jobs.
void Call_Runlevel_Jobs(void)
{
	byte n = 0;

#ifdef DEBUG_P2_4
	_putbit(TRUE, P2, 4);
#endif

	runlevel_1_jobs();

	// Run level jobs are called in the order of adding.
	while( runlevel_job[n] != NULL && n < RUNLEVEL_JOB_SIZE )
		runlevel_job[n++]();

	S0RIE = ENABLE;
	T1IE = ENABLE;

	// Set CPU priority level ILVL to lowest value 0 and IEN to 1.
	_bfld(PSW, 0xF800, 0x0800);

#ifdef DEBUG_P2_4
	_putbit(FALSE, P2, 4);
#endif
}	// Call_Runlevel_Jobs


void Reset_System(void)
{
	_int166(0);
}


void runlevel_1_jobs(void)
{
	Save_ILVL_and_Disable_All_INTs();
    ASC0_Service_Interface_OccupiedB = ASC0_Service_Interface_Occupied();
	if( !ASC0_Debug_ModeB && ASC0_Service_Interface_OccupiedB && ASC0_System_Interface_Forced() )
		ASC0_Debug_ModeB = TRUE;
	Restore_ILVL();

	if( Parser.call_terminalB  &&  Parser.call_terminal_function != NULL )
		Parser.call_terminal_function();
	if( Parser.call_checksumB  &&  Parser.call_checksum_function != NULL )
		Parser.call_checksum_function();
	// Transmission of response message is started within the same call of
	// "runlevel_1_jobs" if response message was stored in "bSerialOutBuffer" and
	// flag "boASC0TransmReleased" was set by "Parser.call_terminal_function"
	// or "Parser.call_checksum_function".
	Check_Send_Data_ASC0();

#if !defined(NO_LOGGER) && defined(A21CL)
	if( Reset_Request_ReceivedB && ASC0_Transmission_Finished() && DataFlash_Buffer_Empty() )
		Runlevel = 0;
#else
	if( Reset_Request_ReceivedB && ASC0_Transmission_Finished() )
		Runlevel = 0;
#endif

    RunLevel_1_ServicedB = TRUE;
}	// runlevel_1_jobs


// To be used if function runlevel_1_jobs is not called within 1s due to
// processing of long loops.
void Service_RunLevel_1(void)
{
    RunLevel_1_ServicedB = TRUE;
}


void RunLevel_1_Service_Check(void)
{
    if( RunLevel_1_ServicedB )
		RunLevel_1_ServicedB = FALSE;
	else
		Reset_System();	// Software reset.
}	// RunLevel_1_Service_Check


void main(void)
{
	PICON = 0x00EF;			// Special threshold mode: Hysteresis for inputs at ports
							// P2, P3, P6, P7, P8.

	startup_delay();

	clear_kernel_RAM();
	init_global_var_kernel();

	Copy_RAM_Functions();
	Check_Flash_Integrity();
	Check_Flash_Type();
#ifdef ENABLE_FLASH_ERROR_LOG
	Init_Log_Flash();
#endif

	Setup_Config_Data();

	setup_interrupt_controls();

	init_t1_for_timer_and_timeout_jobs();
#ifndef NO_LOGGER
	Init_RTC();
	init_clock_jobs();
#endif

	if( !Recovery_Mode )
	{
		IO_Port_2_Setup();
#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
		Init_Signal_Buffer();
#endif
#ifdef SERIAL_SENSOR
		Init_SSC_PWM();
		PWM_Start();
		Init_Sensors_SSC();
#endif
	}

	RunLevel_1_ServicedB = FALSE;
	add_timer_job(1000, RunLevel_1_Service_Check);

	if( !Recovery_Mode )
	{
		add_timer_job(10, check_P2);

#if defined(A21CL) && !defined(NO_LOGGER)
		Init_DataFlash_Buffer();
#endif
	}

	InitP3P4ForASC0();
	if( SectorValid(SECTOR_COMMUNICATION) )		// check for valid communication module
	{
		InitASC0(&ConfigData.ASC0, ConfigData.communication_protocol);

		if( !Recovery_Mode && Rigth_Configuration_Version() )
			Runlevel = RUNLEVEL_COMM;
		else
			Runlevel = RUNLEVEL_SERV;

		RunLevel3Main = communication_main;
#ifdef ENABLE_FLASH_ERROR_LOG
		Log_Flash("Communication tiqui-taca");
#endif
	}
	else
	{
		ConfigData.ASC0 = stRS232_IRMA_Standard;
		ConfigData.communication_protocol = PROT_ID_IRMA;
		InitASC0(&ConfigData.ASC0, ConfigData.communication_protocol);

		Runlevel = RUNLEVEL_KERN;

#ifdef ENABLE_FLASH_ERROR_LOG
		Log_Flash("Communication not found");
#endif
	}

	while( Runlevel != RUNLEVEL_RESET )
	{
		if( Runlevel > RUNLEVEL_KERN )
			RunLevel3Main();
		Call_Runlevel_Jobs();
	}
	Reset_System();
}	// main


#pragma noframe
interrupt (0x02) _using(CSTART_RBANK) void NMITRAP_handler(void)
{
	// Execute software reset.
	_int166(0);
}	//	NMITRAP_handler


interrupt (0x04) _using(CSTART_RBANK) void STOTRAP_handler(void)
{
	// Execute software reset.
	_int166(0);
}	//	STOTRAP_handler


interrupt (0x06) _using(CSTART_RBANK) void STUTRAP_handler(void)
{
	// Execute software reset.
	_int166(0);
}	//	STUTRAP_handler


interrupt (0x0A) _using(CSTART_RBANK) void BTRAP_handler(void)
{
	// Execute software reset.
	_int166(0);
}	//	BTRAP_handler
