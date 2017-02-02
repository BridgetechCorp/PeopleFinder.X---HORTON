/*==============================================================================================*
 |       Filename: GDIST500_AA21C_CI_6_00.c                                                     |
 | Project/Module: A21                                                                          |
 |           Date: 11/12/2010 (to be updated on any change)                                     |
 |                 Change "Release date" according to "Version", "Revision".                    |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#include <reg167cr.h>
#include "..\..\kernel_defines.h"
#include "..\configuration_data.h"


//----- Sector Information Data ----------------------------------------------------------------
const Sector_Information_type Configuration_Info _at(ADDR_CONFIG_DATA_INFO) = {
				"SoftwParams",	// Sector name		
				6,				// Version
				00,				// Revision
				"Dec 11 2010",	// Release date
				"00:00:01",		// Build no.
				"12345",		// Dummy
				0x2E6B2E6F		// Checksum template = "o.k."
			};


#if defined(IBIS_PROTOCOL)
	#if IBISBaudR != 1200 && IBISBaudR != 9600 && IBISBaudR != 19200
		#error "Wrong value for constant IBISBaudR"
	#endif
#endif


//==============================================================================================
//=== W r i t e   d e f a u l t s   t o   c o n f i g u r a t i o n   d a t a   i n   R O M  ===
//==============================================================================================
// This data is to be substituted by configuration data which is written directly to the ROM	
// section by IRMA-A21-Windows or any other configuration tool.									

const Configuration_Data_ROM_type Configuration_Data_ROM _at(ADDR_CONFIG_GENERAL) = {
				FIRMWPARAMVER,				// Version of struct definition
				MINFIRMWPARAMREV,			// Revision of struct definition
				"ORIGIN",					// Configuration source, updated by service software
				#if defined(IBIS_PROTOCOL)
					{IBISBaudR,7,'E',2,0},	// Parameter for asynchronous serial interface
					2,				  		// Protocol: 2 = IBIS
					{1, 20},		  		// Half duplex and send delay
				#else
					{38400,8,'N',1,0},		// Parameter for asynchronous serial interface
					0,						// Protocol: 0 = IRMA
					{1, 20},				// Half duplex and send delay
				#endif
				38400,						// SSC baudrate
				{{0xFF, 0xFF, 0xFF},		// Parameter for pulse width modulator
				 {0xFF, 0xFF, 0xFF},
				 {0xFF, 0xFF, 0xFF},		// Period, width, offset in µs
				 {0xFF, 0xFF, 0xFF}},
				0,							// Number of function areas
				0,							// Number of sensors
				0,							// Total number of elements
				0,							// Number of different sensor types
				125,						// IRMA CAN baud rate: 125, 250, 500 or 1000
				0							// Installation mode timeout. No timeout used
											// if equal to 0.
			};

/*----- Name of Configuration Module -----------------------------------------------------------*/
const char Module_Name_ROM[60]	_at(ADDR_CONFIG_NAME) = {
				0							// fill with 0, parameter undefined, project file
			};								// name written by A21_ModuleManager on linking

/*----- Device Settings ------------------------------------------------------------------------*/
const Configuration_IRMA_ROM_type IRMA_Configuration_ROM	_at(ADDR_CONFIG_IRMA) = {
				1,							/* IRMA device id									*/
				"IRMA",						/* IRMA device tag (4 characters)					*/
				00							/* Time between two status reports (in seconds)		*/
			};

/*----- Function Areas Settings ----------------------------------------------------------------*/
const Function_Area_ROM_104_type Function_Area_ROM[16]		_at(ADDR_FUNCTION_AREA) = {
				0							// fill with 0, parameters undefined
			};

/*----- Height Classes -------------------------------------------------------------------------*/
const word Height_Classes_No								_at(ADDR_HEIGHT_CLASSES) = {
				0x0000,
			};

const Height_Classes_ROM_type Height_Classes_ROM[16]		_at(ADDR_HEIGHT_CLASSES + 2) = {
				{0x00, 0x00},					/* Limits of height class 0 					*/ 
				{0x00, 0x00},					/* Limits of height class 1 					*/ 
				{0x00, 0x00},					/* Limits of height class 2 					*/ 
				{0x00, 0x00},					/* Limits of height class 3 					*/ 
				{0x00, 0x00},					/* Limits of height class 4 					*/ 
				{0x00, 0x00},					/* Limits of height class 5 					*/ 
				{0x00, 0x00},					/* Limits of height class 6 					*/ 
				{0x00, 0x00},					/* Limits of height class 7 					*/ 
				{0x00, 0x00},					/* Limits of height class 8 					*/ 
				{0x00, 0x00},					/* Limits of height class 9 					*/ 
				{0x00, 0x00},					/* Limits of height class 10 					*/ 
				{0x00, 0x00},					/* Limits of height class 11 					*/ 
				{0x00, 0x00},					/* Limits of height class 12 					*/ 
				{0x00, 0x00},					/* Limits of height class 13 					*/ 
				{0x00, 0x00},					/* Limits of height class 14 					*/ 
				{0x00, 0x00},					/* Limits of height class 15 					*/ 
			};

/*----- Port pin to function area assignments --------------------------------------------------*/
const  PortPin_FA_ROM_type PortPin_FA_ROM[16]				_at(ADDR_PORT_PIN) = {
				0							// fill with 0, parameters undefined
			};

/*----- Sensors Settings -----------------------------------------------------------------------*/
const Sensor_Config_ROM_type Sensor_Config_ROM[16]		_at(ADDR_SENSOR_CONFIG) = {
				0							// fill with 0, parameters undefined
			};

/*----- Extended Sensors Settings --------------------------------------------------------------*/
const  Ext_Sen_Config_ROM_Field_type Ext_Sen_Config_ROM		_at(ADDR_EXT_SEN_CONFIG) = {
				0							// fill with 0, parameters undefined
			};

/*----- Cycle Table ----------------------------------------------------------------------------*/
const Cycle_Table_ROM_type Cycle_Table_ROM[16 * 16]		_at(ADDR_CYCLE_TABLE) = {
				0							// fill with 0, parameters undefined
			};

/*----- Extended function area settings --------------------------------------------------------*/
const Ext_Funct_Area_ROM_Field_type Ext_Funct_Area_ROM			_at(ADDR_EXT_FUNCT_AREA) = {
				0							// fill with 0, parameters undefined
			};

/*----- Sensor Types Table ---------------------------------------------------------------------*/
const Sensor_Type_ROM_type Sensor_Type_ROM[6]			_at(ADDR_SENSOR_TYPE) = {
				0							// fill with 0, parameters undefined
			};

/*----- OBC-Data -------------------------------------------------------------------------------*/
const OBC_Data_ROM_type OBC_Data_ROM						_at(ADDR_OBC_DATA) = {
				0x0000010F,
			};

/*----- Logger-Data ----------------------------------------------------------------------------*/
const Logger_Config_ROM_type Logger_ROM					_at(ADDR_LOGGER_CONFIG) = {
				{0, 15},			 				/* Either minutes or seconds */
			};

/*----- Gloria ---------------------------------------------------------------------------------*/
const Gloria_ROM_107_type Gloria_ROM						_at(ADDR_GLORIA_CONFIG_107) = {
				0,									/* No modem preference						*/
				0,									/* Word alignment							*/
				"",									/* Phone number of OBC						*/
				"",									/* Phone number of base station				*/
				"",									/* SIM card pin code						*/
				"",									/* Server URL								*/
				"",									/* Network gateway URL						*/
				"",									/* Network access user name					*/
				"",									/* Network access password					*/
				0,									/* Network connection type: GSM, GPRS, ...	*/
				0,									/* Server connect interval (seconds)		*/
				{    0,		/* Response Delay Constant
				   	   		    +-->Aerocomm EEPROM Address
				   	   		    |   Parameter Name:          Range/Description:	*/
				  0x00,		/* 0x40 "Channel Number"         (0x00 - 0x37)      */
				  0x00,		/* 0x76	"System ID"              (0x00 - 0xFF)      */
				  0x00,		/* 0x58	"Interface Timeout"	     (0x02 - 0xFF)      */
				  0x00,		/* 0x5B	"RF Packet Size"   	     (0x01 - 0x80)      */
				  0x00,		/* 0x5C	"CTS On"   			     (0x01 - 0xFF)      */
				  0x00,		/* 0x5D	"CTS Off"			     (0x00 - 0xFE)      */
				  0x00,		/* 0x4C	"Transmit Retries" 	     (0x01 - 0xFF)      */
				  0x00,		/* 0x4D	"Broadcast Attempts"     (0x01 - 0xFF)      */
				  0x00,		/* 0x3F	"Stop Bit Delay"   	     (0x00 - 0xFF)      */
				  0x00,		/* 0x3D	"Range Refresh"   	     (0x01 - 0xFF)      */
				     0,		/* 0x41	"Server/Client Mode"     (0=Server; 1=Client)                                */
				     0,		/* 0x56 bit 0 "Auto Config"      (0=Use EEPROM Values; 1=Auto Configure Values)      */
				     0,		/* 0x56	bit 1 "Duplex"           (0=Half Duplex; 1=Full Duplex)		                 */
				     0,		/* 0x45	bit 6 "DES Enable"       (0=Disable Encryption; 1=Enable Encryption)		 */
				     0,		/* 0x56	bit 4 "Auto Destination" (0=Use Destination Address; 1=Use Auto Destination) */
				     0,		/* 0x45 bit 1 "RF Delivery"      (0=Addressed Packets; 1=Broadcast Packets)          */
				     0,		/* 0x56	bit 5 "Unicast Only"     (0=Disabled; 1=Enabled)				             */
				     0,		/* 0x56	bit 3 "Client Auto Channel" (0=Disabled; 1=Enabled)							 */
				     0,		/* 0x45	bit 5 "Sync To Channel"  (0=Disabled; 1=Enabled)							 */
				     0,		/* 0x45	bit 7 "One Beacon Mode"	 (0=Disabled; 1=Enabled)							 */
				     0,		/* 0x56 bit 2 "RTS Enable"       (0=Ignore RTS; 1=Transceiver obeys RTS)			 */
				     0,		/* 0x6E "Modem Mode"             (0=Disabled; 1=Enabled)							 */
				     0,		/* 0x7F	"RS485 DE"               (0=Disabled; 1=Enabled)							 */
				     0,		/* 0xC0	"Protocol Status/Receive ACK" (0=Disabled; 1=Enabled)						 */
				     0,		/* 0x6F	"Parity"                 (0=Disabled; 1=Enabled)							 */
				     0,		/* 0xC1	"Receive API"            (0=Disabled; 1=Enabled)							 */
				     0,		/* 0xC6 bit 7 "Enhanced API Control Enable"	(0=Enabled; 1=Disabled)	inv. logic!!!	 */
				     0,		/* 0xC6 bit 1 "API Transmit Packet Enable"  (0=Disabled; 1=Enabled)					 */
				     0,		/* 0xC6 bit 0 "Enhanced API Receive Packet Enable" (0=Disabled; 1=Enabled)		     */
				     0 },	/* 0xC6 bit 2 "Send Data Complete Enable"   (0=Disabled; 1=Enabled)				     */
			};

/*----- Register -------------------------------------------------------------------------------*/
const byte Register_ROM[16]						_at(ADDR_REGISTER) = {
				0xFF,		/* Register  0: 													*/
				0xFF,		/* Register  1:                       								*/
				0xFF,		/* Register  2:                       								*/
				0xFF,		/* Register  3:                       								*/
				0xFF,		/* Register  4:                       								*/
				0xFF,		/* Register  5:                       								*/
				0xFF,		/* Register  6:                       								*/
				0xFF,		/* Register  7:                       								*/
				0xFF,		/* Register  8:                       								*/
				0xFF,		/* Register  9:                       								*/
				0xFF,		/* Register 10:                       								*/
				0xFF,		/* Register 11:                       								*/
				0xFF,		/* Register 12:                       								*/
#if defined(IBIS_PROTOCOL)
				0xFF,		/* Register 13:														*/
				0xFF,		/* Register 14:														*/
#else
				0x1E,		/* Register 13: output switch puls duration in ms					*/
				0x1E,		/* Register 14: min. output switch puls pause in ms					*/
#endif
				0xFF,		/* Register 15:                             						*/
			};
