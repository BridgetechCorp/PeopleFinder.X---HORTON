/*==============================================================================================*
 |       Filename: communication_interface_gateway.c                                            |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Interface of module group Communication to module group Application.         |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/
#ifndef GATEWAY
	#error "File communication_interface_gateway.c included but symbol GATEWAY not defined"
#endif


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include "..\..\kernel_interface.h"
#include "..\communication.h"
#include "communication_gdist500.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=COMM
#pragma class FB=COMMRAM


/*----- Global Variables -----------------------------------------------------------------------*/


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
void Device_Status_Alloc(void **device_status)
{
	*device_status = &DeviceStatus;
}


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=COMM_TAB
/*==============================================================================================*
 | COMM_TAB                                                                                     |
 *==============================================================================================*
 | I M P O R T A N T :  Do not change the order of the following section!                       |
 | Entry Address = 0x1FF00 + 6*(Entry Number)                                                   |
 *==============================================================================================*/


/*----- Implementation of Functions ------------------------------------------------------------*/
/*===========================================================*
 | Entry Number  : 0                                         |
 | Entry Address : 0x1FF00                                   |
 | Reserved for  : opera_CommunicationMain                   |
 *===========================================================*/
void opera_CommunicationMain(void)
{
	#pragma asm
		EXTERN _communication_main:FAR
		JMP _communication_main
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 1                                         |
 | Entry Address : 0x1FF06                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF06(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 2                                         |
 | Entry Address : 0x1FF0C                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF0C(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 3                                         |
 | Entry Address : 0x1FF12                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF12(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 4                                         |
 | Entry Address : 0x1FF18                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF18(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 5                                         |
 | Entry Address : 0x1FF1E                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF1E(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 6                                         |
 | Entry Address : 0x1FF24                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF24(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 7                                         |
 | Entry Address : 0x1FF2A                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF2A(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 8                                         |
 | Entry Address : 0x1FF30                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF30(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 9                                         |
 | Entry Address : 0x1FF36                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF36(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 10                                        |
 | Entry Address : 0x1FF3C                                   |
 | Reserved for  : opera_AddDoorClosedJob                    |
 *===========================================================*/
int opera_AddDoorClosedJob(void huge *ptr)
{
	ptr;
	#pragma asm
		EXTERN _add_door_closed_job:FAR
		JMP _add_door_closed_job
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 11                                        |
 | Entry Address : 0x1FF42                                   |
 | Reserved for  : opera_RemoveDoorClosedJob                 |
 *===========================================================*/
int opera_RemoveDoorClosedJob(void huge *ptr)
{
	ptr;
	#pragma asm
		EXTERN _remove_door_closed_job:FAR
		JMP _remove_door_closed_job
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 12                                        |
 | Entry Address : 0x1FF48                                   |
 | Reserved for  : opera_DeviceStatusAlloc                   |
 *===========================================================*/
void opera_DeviceStatusAlloc(void **device_status)
{
	device_status;
	#pragma asm
		EXTERN _Device_Status_Alloc:FAR
		JMP _Device_Status_Alloc
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 13                                        |
 | Entry Address : 0x1FF4E                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF4E(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 14                                        |
 | Entry Address : 0x1FF54                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF54(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 15                                        |
 | Entry Address : 0x1FF5A                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF5A(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 16                                        |
 | Entry Address : 0x1FF60                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF60(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 17                                        |
 | Entry Address : 0x1FF66                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF66(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 18                                        |
 | Entry Address : 0x1FF6C                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF6C(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 19                                        |
 | Entry Address : 0x1FF72                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF72(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 20                                        |
 | Entry Address : 0x1FF78                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF78(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 21                                        |
 | Entry Address : 0x1FF7E                                   |
 | Reserved for  : analyzer firmware compatibility           |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF7E(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 22                                        |
 | Entry Address : 0x1FF84                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF84(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 23                                        |
 | Entry Address : 0x1FF8A                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF8A(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 24                                        |
 | Entry Address : 0x1FF90                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF90(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 25                                        |
 | Entry Address : 0x1FF96                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF96(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 26                                        |
 | Entry Address : 0x1FF9C                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FF9C(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 27                                        |
 | Entry Address : 0x1FFA2                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFA2(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 28                                        |
 | Entry Address : 0x1FFA8                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFA8(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 29                                        |
 | Entry Address : 0x1FFAE                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFAE(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 30                                        |
 | Entry Address : 0x1FFB4                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFB4(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 31                                        |
 | Entry Address : 0x1FFBA                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFBA(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 32                                        |
 | Entry Address : 0x1FFC0                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFC0(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/*===========================================================*
 | Entry Number  : 33                                        |
 | Entry Address : 0x1FFC6                                   |
 | Reserved for  : Future Use                                |
 *===========================================================*/
void opera_NoJobCommunication_0x1FFC6(void)
{
	#pragma asm
		EXTERN _No_Job_Communication:FAR
		JMP _No_Job_Communication
	#pragma endasm
}


/* Last two bytes in memory class COMM_TAB (0x1FFCC-0x1FFCD) are filled with instruction "RETS" */
void COMM_TAB_No_Entry (void)
{
	return;
}
