/*==============================================================================================*
 |       Filename: configuration_data.h                                                         |
 | Project/Module: A21                                                                          |
 |           Date: 09/08/2009 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CONFIGURATION_DATA_INC		/* Make sure that this file is only included once	*/
									/* within a module.									*/

#define ADDR_CONFIG_GENERAL		 ADDR_CONFIG_DATA				// 0x4000
#define ADDR_CONFIG_NAME		(ADDR_CONFIG_DATA + 0x0084)		// 0x4084
#define ADDR_CONFIG_IRMA		(ADDR_CONFIG_DATA + 0x00C0)		// 0x40C0
#define ADDR_FUNCTION_AREA		(ADDR_CONFIG_DATA + 0x0100)		// 0x4100
#define ADDR_HEIGHT_CLASSES		(ADDR_CONFIG_DATA + 0x0280)		// 0x4280
#define ADDR_PORT_PIN			(ADDR_CONFIG_DATA + 0x02C0)		// 0x42C0
#define ADDR_SENSOR_CONFIG		(ADDR_CONFIG_DATA + 0x0300)		// 0x4300
#define ADDR_EXT_SEN_CONFIG		(ADDR_CONFIG_DATA + 0x03E0)		// 0x43E0
#define ADDR_CYCLE_TABLE		(ADDR_CONFIG_DATA + 0x0500)		// 0x4500
#define ADDR_EXT_FUNCT_AREA		(ADDR_CONFIG_DATA + 0x0F00)		// 0x4F00
#define ADDR_SENSOR_TYPE		(ADDR_CONFIG_DATA + 0x1000)		// 0x5000
#define ADDR_OBC_DATA			(ADDR_CONFIG_DATA + 0x1800)		// 0x5800
#define ADDR_LOGGER_CONFIG		(ADDR_CONFIG_DATA + 0x1820)		// 0x5820
#define ADDR_GLORIA_CONFIG_107	(ADDR_CONFIG_DATA + 0x19FE)		// 0x59FE
#define ADDR_GLORIA_CONFIG		(ADDR_CONFIG_DATA + 0x1A00)		// 0x5A00
#define ADDR_REGISTER			(ADDR_CONFIG_DATA + 0x1F00)		// 0x5F00

// Refer to comment in header file "kernel_defines.h" related to disabled macro definition
// IBISBaudR.
#ifdef IBIS_PROTOCOL
	#if defined(IBIS9600)
		#define IBISBaudR 9600
	#elif defined(IBIS19200)
		#define IBISBaudR 19200
	#else
		#define IBISBaudR 1200
	#endif
#endif

/*----------------------------------------------------------------------------------------------*/


//#include "..\opera_version.h"
#define	FIRMWPARAMVER			1				// Version of A21 firmware parameters.
#define	MINFIRMWPARAMREV		7				// Minimum revision of A21 firmware parameters.

#include "configuration_data_kernel_1-02.h"

typedef Configuration_Data_Kernel_102_type Configuration_Data_Kernel_type;

#include "configuration_data_1-04.h"
#include "configuration_data_1-05.h"
#include "configuration_data_1-06.h"
#include "configuration_data_1-07.h"
#include "configuration_data_1-xx.h"

typedef Configuration_Data_ROM_107_type Configuration_Data_ROM_type;

#define CONFIGURATION_DATA_INC
#endif
