/*==============================================================================================*
 |       Filename: communication_defines.h                                                      |
 | Project/Module: A21                                                                          |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Declarations of general meaning for module group Communication.              |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef COMMUNICATION_DEFINES_INC		/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "kernel_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define DOOR_STATE_CHANGE_INTV			100	// ms, interval of query of door signal inputs


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	bool driveB;
	struct
	{
		bool closedB;
		bool countingB;
		struct {
			bool motionB;
			byte opening;
		} left, right;
	} door[FUNCTION_AREAS_MAX];
	struct {
		bool initialization_errorB;
		bool initialization_error_door_openB;
		bool runtime_errorB;
		bool runtime_error_door_openB;
		bool sabotageB;
		bool firmware_mismatchB;
	} sensor[SENSORS_MAX];
} Device_Status_type;

typedef struct
{
	bool initialization_errorB;
	bool runtime_errorB;
	bool firmware_mismatchB;
	bool sabotageB;
} fa_status_type;
  
typedef fa_status_type fa_statuses_type[FUNCTION_AREAS_MAX];

typedef	struct 
{
	struct
	{
		BitField reserved	: 16;
	} StatusBits[SENSORS_MAX];
} ApplicSensorStatus_type;


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/


#define COMMUNICATION_DEFINES_INC
#endif	// end of "#ifndef COMMUNICATION_DEFINES_INC"
