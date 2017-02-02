/*==============================================================================================*
 |       Filename: kernel_interface.c                                                           |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 11/25/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Interface of module group Kernel to module group Application.                |
 |                 Implementation of interface function.                                        |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>

#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#include "main.h"
#include "time.h"
#include "serio_asc0.h"
#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
	#include "signal.h"
#endif


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class FB=KERNELRAM


/*----- Global Variables -----------------------------------------------------------------------*/


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
byte Get_FA_Index(byte fa_addr)
{
	extern byte FA2index[MaxFAAddr];

	if( fa_addr > MaxFAAddr || fa_addr == 0 )
		return 0xFF;
	else
		return FA2index[fa_addr - 1];
}	// Get_FA_Index


void get_firmware_name(bool get_inherent_firmware_nameB, byte *res_buff, byte *res_buff_pos)
{
	byte huge *firmware_name;
	byte firmware_name_pos = 0;

	if( get_inherent_firmware_nameB )
		// Select inherent firmware name created by A21_ModuleManager. Empty string for firmware
		// without runtime modularity.
		firmware_name = (byte huge *)ADDR_APPLICATION2;
	else
		// Select firmware name identical to firmware file name.
		firmware_name = (byte huge *)ADDR_APPLICATION1;

	// Copy firmware name to result buffer.
	while( *firmware_name != 0xFF && firmware_name_pos++ < FIRMWARE_NAME_LENGTH - 1 )
		// Brackets are necessary to give dereferencing priority over increment.
		// As a result buffer position is incremented, not pointer to buffer position.
		res_buff[(*res_buff_pos)++] = *firmware_name++;

	// Fill remaining bytes of result buffer with 0.
	while( firmware_name_pos++ < FIRMWARE_NAME_LENGTH )
		// Brackets are necessary to give dereferencing priority over increment.
		// As a result buffer position is incremented, not pointer to buffer position.
		res_buff[(*res_buff_pos)++] = 0;
}	// get_firmware_name


bool is_inherent_firmware_name(void)
{
	byte firmware_name_pos;
	char hex_firmware_name[FIRMWARE_NAME_LENGTH], inh_firmware_name[FIRMWARE_NAME_LENGTH];
	bool equalB = FALSE;

	firmware_name_pos = 0;
	get_firmware_name(FALSE, (byte *)hex_firmware_name, &firmware_name_pos);
	firmware_name_pos = 0;
	get_firmware_name(TRUE,  (byte *)inh_firmware_name, &firmware_name_pos);

	uppercase(hex_firmware_name);
	uppercase(inh_firmware_name);

	equalB = strcmp(hex_firmware_name, inh_firmware_name) == 0;

	return(equalB);
}	// is_inherent_firmware_name


void Set_Runlevel(byte rlv)
{
	Runlevel = rlv;
}	// Set_Runlevel


byte Get_Runlevel(void)
{
	return(Runlevel);
}	// Get_Runlevel


bool Runlevel_4_or_5(void)
{
	return(Runlevel == 4 || Runlevel == 5);
}	// Runlevel_4_or_5


void Set_ParserParameter(byte terminalsymbol, word timeout, void huge *fptr, void huge *csptr)
{
	Parser.terminal_symbol = terminalsymbol;
	Parser.call_terminal_function = fptr;
	Parser.call_checksum_function = csptr;
	Parser.timeout = timeout;
}	// Set_ParserParameter


void Set_Parser_function(void huge *ptr)
{
	Parser.call_terminal_function = ptr;
}	// Set_Parser_function


word Get_Parser_timeout(void)
{
	return(Parser.timeout);
}	// Get_Parser_timeout


void Reset_Parser_Flags(void)
{
	Parser.call_terminalB = FALSE;
	Parser.call_checksumB = FALSE;
}	// Reset_Parser_Flags


bool SectorValid(byte sector)
{
	return(SectorInformation[sector].okB);
}	// SectorValid


#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
int Get_Sensor_Data(word *value, byte *sensor_idx, byte *element_adr)
{
	if( wSensorData_write == wSensorData_interface_read )
		return(EOF);

	NEXT(wSensorData_interface_read, wSensorData_Buffer_Size);
	*value = stSensorData[wSensorData_interface_read].value;
	*sensor_idx = stSensorData[wSensorData_interface_read].sensor_idx;
	*element_adr = stSensorData[wSensorData_interface_read].element_adr;
	return(0);
}	// Get_Sensor_Data
#endif


#ifdef SERIAL_SENSOR
void Put_Sensor_Data(word value, byte table_index, byte sensor_idx, byte element_adr)
{
	NEXT(wSensorData_write, wSensorData_Buffer_Size);
	if( wSensorData_write == wSensorData_interface_read && Runlevel == RUNLEVEL_APPL )
		A21_Status.Sensor_Interface.Buffer_OverflowB = TRUE;
	stSensorData[wSensorData_write].value = value;
	stSensorData[wSensorData_write].table_index = table_index;
	stSensorData[wSensorData_write].sensor_idx = sensor_idx;
	stSensorData[wSensorData_write].element_adr = element_adr;
}	// Put_Sensor_Data
#endif


#ifdef CAN_SENSOR
void Put_Sensor_Data_Block_Def(byte *signal_ptr, byte sensor_idx)
{
	byte i;
	#ifdef CXYO
	word signal_value;
	#endif

	for( i = 0; i < Dist4_ElemCnt_Def; i++ )
	{
		NEXT(wSensorData_write, wSensorData_Buffer_Size);
		if( wSensorData_write == wSensorData_interface_read && Runlevel == RUNLEVEL_APPL )
			A21_Status.Sensor_Interface.Buffer_OverflowB = TRUE;
	#ifdef CXYO
		signal_value = *signal_ptr++ << 8;
		signal_value += *signal_ptr++;
		stSensorData[wSensorData_write].value = signal_value;
	#else
		stSensorData[wSensorData_write].value = *signal_ptr++;
	#endif
		stSensorData[wSensorData_write].sensor_idx = sensor_idx;
		stSensorData[wSensorData_write].element_adr = 2 * i;
	}
}	// Put_Sensor_Data_Block_Def


void Put_Sensor_Data_Block_Emc(byte *signal_ptr, byte sensor_idx)
{
	byte i;

	for( i = 0; i < Elements_per_sensor; i++ )
	{
		NEXT(wSensorData_write, wSensorData_Buffer_Size);
		if( wSensorData_write == wSensorData_interface_read )
			A21_Status.Sensor_Interface.Buffer_OverflowB = TRUE;
		stSensorData[wSensorData_write].value = *signal_ptr++;
		stSensorData[wSensorData_write].sensor_idx = sensor_idx;
		if( Elements_per_sensor == Dist4_ElemCnt_Def )
			stSensorData[wSensorData_write].element_adr = 2 * i;
		else
			stSensorData[wSensorData_write].element_adr = i;
	}
}	// Put_Sensor_Data_Block_Emc
#endif


#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
void SensorData_alloc(void **data, void **write, word *buffer_size)
{
	*data = stSensorData;
	/* Tasking C compiler uses uninterruptable extended page sequence (instruction EXTP) for */
	/* pointer assignments if destination is not within this module. No explicit interrupt   */
	/* disabling needed.                                                                     */
	*write = &wSensorData_write;
	*buffer_size = wSensorData_Buffer_Size;
}	// SensorData_alloc
#endif


void Configuration_Data_Alloc(void **config_system, void **config_data)
{
	*config_system = &ConfigSystem;
	*config_data = &ConfigData;
}	// Configuration_Data_Alloc


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL_TAB
/*==============================================================================================*
 | KERNEL_TAB                                                                                   |
 *==============================================================================================*
 | I M P O R T A N T :  Do not change the order of the following section!                       |
 | Entry Address = 0xDF00 + 6*(Entry Number)                                                    |
 *==============================================================================================*/


/*----- Implementation of Functions ------------------------------------------------------------*/
/*===========================================================*
 | Entry Number  : 0                                         |
 | Entry Address : 0xDF00                                    |
 | Reserved for  : KernelMain                                |
 *===========================================================*/
void KernelMain(void)
{
	#pragma asm
		EXTERN _main:FAR
		JMP _main
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 1                                         |
 | Entry Address : 0xDF06                                    |
 | Reserved for  : opera_SetRunlevel                         |
 *===========================================================*/
void opera_SetRunlevel(byte rlv)
{
	rlv;
	#pragma asm
		EXTERN _Set_Runlevel:FAR
		JMP _Set_Runlevel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 2                                         |
 | Entry Address : 0xDF0C                                    |
 | Reserved for  : opera_GetRunlevel                         |
 *===========================================================*/
byte opera_GetRunlevel(void)
{
	#pragma asm
		EXTERN _Get_Runlevel:FAR
		JMP _Get_Runlevel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 3                                         |
 | Entry Address : 0xDF12                                    |
 | Reserved for  : opera_CallRunlevelJobs                    |
 *===========================================================*/
void opera_CallRunlevelJobs(void)
{
	#pragma asm
		EXTERN _Call_Runlevel_Jobs:FAR
		JMP _Call_Runlevel_Jobs
	#pragma endasm
}


#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
/*===========================================================*
 | Entry Number  : 4                                         |
 | Entry Address : 0xDF18                                    |
 | Reserved for  : opera_GetSensorData                       |
 *===========================================================*/
int opera_GetSensorData(word *value, byte *sensor_idx, byte *element_adr )
{
	value; sensor_idx; element_adr;
	#pragma asm
		EXTERN _Get_Sensor_Data:FAR
		JMP _Get_Sensor_Data
	#pragma endasm
}
#else
/*===========================================================*
 | Entry Number  : 4                                         |
 | Entry Address : 0xDF18                                    |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobKernel_0xDF18(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}
#endif


/*===========================================================*
 | Entry Number  : 5                                         |
 | Entry Address : 0xDF1E                                    |
 | Reserved for  : opera_ConfigurationDataAlloc              |
 *===========================================================*/
void opera_ConfigurationDataAlloc(void **config_system, void **config_data)
{
	config_system; config_data;
	#pragma asm
		EXTERN _Configuration_Data_Alloc:FAR
		JMP _Configuration_Data_Alloc
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 6                                         |
 | Entry Address : 0xDF24                                    |
 | Reserved for  : opera_GetFAIndex                          |
 *===========================================================*/
byte opera_GetFAIndex(byte fa_addr)
{
	fa_addr;
	#pragma asm
		EXTERN _Get_FA_Index:FAR
		JMP _Get_FA_Index
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 7                                         |
 | Entry Address : 0xDF2A                                    |
 | Reserved for  : opera_PutSensorData       (SERIAL_SENSOR) |
 |                 opera_PutSensorDataBlock_Def (CAN_SENSOR) |
 *===========================================================*/
#ifdef SERIAL_SENSOR
void opera_PutSensorData(word value, byte table_index, byte sensor_idx, byte element_adr)
{
	value; table_index; sensor_idx; element_adr;
	#pragma asm
		EXTERN _Put_Sensor_Data:FAR
		JMP _Put_Sensor_Data
	#pragma endasm
}
#endif

#ifdef CAN_SENSOR
void opera_PutSensorDataBlock_Def(byte *signal_ptr, byte sensor_idx)
{
	signal_ptr; sensor_idx;
	#pragma asm
		EXTERN _Put_Sensor_Data_Block_Def:FAR
		JMP _Put_Sensor_Data_Block_Def
	#pragma endasm
}
#endif


/*===========================================================*
 | Entry Number  : 8                                         |
 | Entry Address : 0xDF30                                    |
 | Reserved for  : Future Use                (SERIAL_SENSOR) |
 |                 opera_PutSensorDataBlock_Emc (CAN_SENSOR) |
 *===========================================================*/
#ifdef SERIAL_SENSOR
void opera_NoJobKernel_0xDF30(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}
#endif

#ifdef CAN_SENSOR
void opera_PutSensorDataBlock_Emc(byte *signal_ptr, byte sensor_idx)
{
	signal_ptr; sensor_idx;
	#pragma asm
		EXTERN _Put_Sensor_Data_Block_Emc:FAR
		JMP _Put_Sensor_Data_Block_Emc
	#pragma endasm
}
#endif


/*===========================================================*
 | Entry Number  : 9                                         |
 | Entry Address : 0xDF36                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF36(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 10                                        |
 | Entry Address : 0xDF3C                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF3C(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 11                                        |
 | Entry Address : 0xDF42                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF42(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 12                                        |
 | Entry Address : 0xDF48                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF48(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 13                                        |
 | Entry Address : 0xDF4E                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF4E(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 14                                        |
 | Entry Address : 0xDF54                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF54(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 15                                        |
 | Entry Address : 0xDF5A                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF5A(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 16                                         |
 | Entry Address : 0xDF60                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF60(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 17                                        |
 | Entry Address : 0xDF66                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF66(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 18                                        |
 | Entry Address : 0xDF6C                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF6C(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 19                                        |
 | Entry Address : 0xDF72                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF72(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 20                                        |
 | Entry Address : 0xDF78                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF78(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 21                                        |
 | Entry Address : 0xDF7E                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF7E(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 22                                        |
 | Entry Address : 0xDF84                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF84(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 23                                        |
 | Entry Address : 0xDF8A                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF8A(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 24                                        |
 | Entry Address : 0xDF90                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF90(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 25                                        |
 | Entry Address : 0xDF96                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF96(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 26                                        |
 | Entry Address : 0xDF9C                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDF9C(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 27                                        |
 | Entry Address : 0xDFA2                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFA2(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 28                                        |
 | Entry Address : 0xDFA8                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFA8(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 29                                        |
 | Entry Address : 0xDFAE                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFAE(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 30                                        |
 | Entry Address : 0xDFB4                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFB4(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 31                                        |
 | Entry Address : 0xDFBA                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFBA(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 32                                        |
 | Entry Address : 0xDFC0                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFC0(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 33                                        |
 | Entry Address : 0xDFC6                                    |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobKernel_0xDFC6(void)
{
	#pragma asm
		EXTERN _No_Job_Kernel:FAR
		JMP _No_Job_Kernel
	#pragma endasm
}


/* Last two bytes in memory class KERNEL_TAB (0xDFCC-0xDFCD) are filled with instruction "RETS" */
void KERNEL_TAB_No_Entry (void)
{
	return;
}
