/*==============================================================================================*
 |       Filename: opera_interface.h                                                            |
 | Project/Module: A21                                                                          |
 |           Date: 10/22/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Interface of module groups Kernel and Communication to module group          |
 |                 Application.                                                                 |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef OPERA_INTERFACE_INC				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "kernel_defines.h"
#include "configuration_defines.h"
#include "communication_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/

/*----- Modul Group Kernel ---------------------------------------------------------------------*/
void opera_SetRunlevel(byte rlv);
byte opera_GetRunlevel(void);
void opera_CallRunlevelJobs(void);

void opera_ConfigurationDataAlloc(void **config_system, void **config_data);

int opera_GetSensorData(word *value, byte *sensor_idx, byte *element_adr );
#ifdef SERIAL_SENSOR
void opera_PutSensorData(word value, byte table_index, byte sensor_idx, byte element_adr);
#endif

#ifdef CAN_SENSOR
void opera_PutSensorDataBlock_Def(byte *signal_ptr, byte sensor_idx);
void opera_PutSensorDataBlock_Emc(byte *signal_ptr, byte sensor_idx);
#endif

/*----- Modul Group Communication --------------------------------------------------------------*/
void opera_IncreasePersonsInCounter(byte area);
void opera_DecreasePersonsInCounter(byte area);
void opera_IncreasePersonsOutCounter(byte area);
void opera_DecreasePersonsOutCounter(byte area);

void opera_IncreasePersonsClassInCounter(byte area, byte class);
void opera_DecreasePersonsClassInCounter(byte area, byte class);
void opera_IncreasePersonsClassOutCounter(byte area, byte class);
void opera_DecreasePersonsClassOutCounter(byte area, byte class);

void opera_CountingFinished(byte area);


int opera_AddDoorClosedJob(void  *ptr);
int opera_RemoveDoorClosedJob(void  *ptr);

void opera_DeviceStatusAlloc(void **device_status);
void opera_ApplicSensorStatusAlloc(void **applic_sensor_status);


#define OPERA_INTERFACE_INC
#endif	// end of "OPERA_INTERFACE_INC"
