/*==============================================================================================*
 |       Filename: setup.h                                                  Version: 1.00       |
 | Project/Module: A21/Complete Project                                        Date: 04/26/2007 |
 |        Authors: Hartmut Schneider (HS)                                                       |
 |                 Wladimir Plaggés (WP)                                                        |
 |----------------------------------------------------------------------------------------------|
 |    Description: Constant Definitions for system setup.                                       |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef SETUP_INC

/*----- System Setup : Option "TEST_HEX" -------------------------------------------------------*/
#if defined(TEST_HEX)
	#if defined(EMULATOR)
		#error "Wrong combination of user macros: TEST_HEX + EMULATOR"
	#endif

	#define ENABLE_FLASH_ERROR_LOG

	#define DEBUG_P2_4
	#define DEBUG_P2_5
	#define DEBUG_P2_6
	#define DEBUG_P2_7


/*----- System Setup : Option "EMULATOR" -------------------------------------------------------*/
#elif defined(EMULATOR)
	#define ENABLE_FLASH_ERROR_LOG

	#define NO_SECTOR_CHECK
	#define IGNORE_IBIS_CHECKSUM

	#define DEBUG_P2_4
	#define DEBUG_P2_5
	#define DEBUG_P2_6
	#define DEBUG_P2_7

	#define FLASH_MAN_ID				0x01
	#define FLASH_DEV_ID				0x51
#endif


/*----- System Setup : General Optional Constants Definitions ----------------------------------*/
#define STARTUP_DELAY
//#define GLORIA_VERSION_20			/* Gloria Version 2.0 */
//#define DEBUG_START_RS232_DEBUG_MODE
//#define DEBUG_IBIS_COMMUNICATION
//#define DEBUG_IBIS_COMMUNICATION_TEXT
//#define SIMULATE_DOOR_SWITCH
//#define MAX_2_CYCLES
//#define USE_T3OUT


#define SETUP_INC
#endif