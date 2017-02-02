#ifndef ERROR_H
#define ERROR_H
#include "types.h"

typedef enum
{
	//Random errors
	NO_ERROR                        = 0,
    PROCESSSOR_RESET                = 1,	
	NVM_UNABLE_TO_SAVE 				= 3,
	FIRMWARE_ERROR                  = 4,
	
	
	//Dictionary Errors
	INVALID_DICT_ENTRY 				= 8,
	DICT_DATA_MISMATCH 				= 9,
	DICT_INVALID_INDEX 				= 10,
	DICT_INVALID_DATA 				= 11,
	DICT_DATA_EXCEEDS_CAN_SIZE		= 12, 
	DICT_IS_READONLY 				= 13,
	DICT_IS_WRITEONLY 				= 14,
	DICT_DATA_IS_CORRUPT 			= 15,
	DICT_DOES_NOT_EXIST				= 16,
	DICT_SIZE_TOO_BIG               = 17,


	// CAN BUS ERRORS
    CAN_RX_BUFFER_OVERFLOW         = 20,
	CAN_TX_TIMEOUT                 = 21,
	
	// ENCODER PROBLEM
    ENCODER_READING_ERROR           =30,
	ENCODER_SETUP_ERROR             =31,
	
    
	// COMMAND ERRORS
	DATA_COMMAND_INVALID_FOR_SLAVE 	= 47,
	DATA_COMMAND_NO_COMMAND_STORED 	= 48,
	
	COMMAND_INVALID_FOR_SLAVE 		= 50,
	
	// SERVO ERRORS
	FOLLOWING_ERROR					= 57,
	NOT_CALIBRATED					= 58,
	NOT_CONFIGURED					= 59,
	NOT_SERVOED						= 60,
	BAD_PEAK_I_SETTING              = 61,
	SERVO_STARVING                  = 62,
	MOTOR_STALLED                   = 63,
	
	// HALL ERRORS
	HALLS_SATURATED					= 65,
	HALLS_NO_SIGNAL					= 66,
	
	// BOOTUP_ERRORS
	RESET_POWERUP					= 69,
	RESET_BROWNOUT					= 70,
	RESET_IDLE						= 71,
	RESET_SLEEP						= 72,
	RESET_WATCHDOG_TIMEOUT			= 73,
	RESET_SOFTWARE_ENABLE			= 74,
	RESET_SOFTWARE_RESET			= 75,
	RESET_MCLR_RESET				= 76,
	RESET_VREGS						= 77,
	RESET_CONFIG_MM					= 78,
	RESET_ILLEGAL_OPCODE			= 79,
	RESET_TRAP_CONFLICT				= 80,
	
	
	
	
	// FLASH_ERRORS
	FLASH_CHECKSUM_FAIL             = 90,
	EXCEEDED_RESERVED_FLASH			= 91,	
	FLASH_FAILED_TO_READ_SLAVES		= 92,
	FLASH_WRITE_ERROR               = 93,
	FLASH_READ_ERROR                = 94,
	FLASH_SAVED_PARAMETERS          = 95,
	
	
	// HOMING ERRORS
	HOMING_FAILED                   = 100,
	HOMING_BAD_NEG_LIM    			= 101,
	HOMING_BAD_POS_LIM				= 102,
	HOMING_BAD_HOME_LIM				= 103,
	HOMING_UNEXPECTED_NEGLIM        = 104,
	HOMING_UNEXPECTED_POSLIM        = 105,
	HOMING_MOTOR_STUCK              = 106,
	HOMING_WRONG_DIRECTION          = 107,
	HOMING_ERROR_TIMEOUT            = 108,
	
	
	// TRAJECTORY
	TRAJECTORY_BUFFER_FULL			= 118,
	TRAJECTORY_CHECKSUM_FAIL		= 119,
	TRAJECTORY_FAILED               = 120,
	
	// MOTOR ERRORS
	SENSOR_NOT_CALIBRATED			= 125,
	SERVO_NOT_ENABLED				= 126,
	ACCELERATION_IS_NULL			= 127,
	VELOCITY_IS_NULL				= 128,
	PROPORTION_IS_NULL				= 129,
	SERVO_ERR_UNDER_CURRENT         = 130,
	SERVO_ERR_OVER_CURRENT          = 131,
	SERVO_ERR_SETTLE_TIMEOUT        = 132,
	SERVO_ALREADY_MOVING            = 133, 
	MOTORPOLES_IS_NULL              = 134,


    // LIMIT SENSOR ERRORS
	COLLIDED_WITH_NEGLIM           = 140,
	COLLIDED_WITH_POSLIM           = 141,
	COLLIDED_WITH_HOMEFLAG         = 142,
	
	//CALIBRATION ERROR
	BAD_ENCODER_COUNT			   = 150,	
	BAD_MOTOR_DIRECTION	           = 151,

	IN_BOOTLOADER_MODE              = 163,

	// AUTO_TUNE_ERROR
	NO_AUTOTUNE_VARS_ENABLED       = 170,
	AUTOTUNE_OUTSIDE_BOUNDS        = 171,
	       
	    
}error_codes;


void ClearErrorLog( void ); //called on startup, and when the PC want to clear out the last error
void 		AddErrorCode		(error_codes error);
void        RemoveErrorCode	    (error_codes error);
boolean 	DuplicateError		(error_codes error);


#define SIZE_ERROR_BUFFER 4
extern uint8 Error[SIZE_ERROR_BUFFER];
extern int8 errorflag;

#endif
