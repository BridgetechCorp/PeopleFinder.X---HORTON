/*==============================================================================================*
 |       Filename: configuration_data_1-07.h                                                    |
 | Project/Module: A21                                                                          |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                 Extensions of structure version 1.7.                                         |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                 Extensions of structure version 1.7.                                         |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#include "..\kernel_defines.h"


/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004000 ---------- S T R U C T   V E R S I O N   1.07 ---*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	byte struct_version;				/* Version of struct definition					*/
/*$01  1 */	byte struct_revision;				/* Revision of struct definition				*/
/*$02 16 */	char configuration_source[16];		/* Source of firmware parameters:				*/
												/* Zero-terminated string "ORIGIN" contained	*/
												/* in Intel Hex file, signature of service		*/
												/* software "SNvv.rr.ss.bbbb" otherwise.		*/
												/* SN   = service software name,				*/
												/*        "Bo" for A21_Boot,					*/
												/*        "As" for A21_Assistant,				*/
												/*        "Up" for A21_Update.					*/
												/* vv   = version of service software.			*/
												/* rr   = revision of service software.			*/
												/* ss   = subrevision of service software.		*/
												/* bbbb = build of service software.			*/
			struct {							/* Parameter for asynchronous serial interface	*/
/*$12  4 */		dword baudrate;					/*		Baud rate in bps 						*/
/*$16  1 */		byte data_bits;					/*		7, 8, 9									*/
/*$17  1 */		char parity;					/*		'N' = None, 'O' = Odd, 'E' = Even		*/
/*$18  1 */		byte stop_bits;					/*		1, 2									*/
/*$19  1 */		byte flow_control;				/*		0 = no flow control, 1 = RTS/CTS		*/
			} ASC0;
/*$1A  1 */	byte communication_protocol;		/* Communication protocol:						*/
												/*		0 = IRMA, 1 = CL, 2 = IBIS, 3 = J1708	*/
/*$1B  1 */										/* word alignment								*/
			struct {							/* Parameter of interface mode					*/
/*$1C  1 */		bool half_duplexB;				/* 0 = full duplex, 1 = half duplex on ASC0		*/
/*$1D  1 */	                                	/* word alignment								*/
/*$1E  2 */		word send_delay;				/* delay response, unit: ms						*/
			} ASC0_control;
/*$20  4 */	dword SSC_baudrate;
/*$24 24 */	struct {							/* Pulse width modulator settings				*/
				word period;					/* 		Period, unit: µs						*/
				word pulse_width;				/*		Pulse width, unit: µs					*/
				word offset;					/*		Offset, unit: µs						*/
			} PWM[4];
/*$3C  1 */	byte function_areas;				/* Number of function areas						*/
/*$3D  1 */	byte sensors;						/* Number of sensors							*/
/*$3E  2 */	word elements;						/* Total number of elements						*/
/*$40  1 */	byte sensor_types;					/* Number of different sensor types				*/
/*$41  1 */										/* word alignment								*/
/*$42  2 */	word CAN_Baud_Rate;					/* CAN IRMA baud rate: 125, 250, 500 or 1000	*/
/*$44  2 */	word Instmode_Timeout_s;			/* Installation mode timeout. No timeout used	*/
												/* if equal to 0.								*/
} Configuration_Data_ROM_107_type;

typedef struct {	/* Start address: 0x004084 ---------- Firmware Parameter Module Name -------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00 60 */	char name[60];						/* module name									*/
} Softw_Param_Module_Name_type;

/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x0042C0 ---------- Port pin to function area assignments */
/*   +------ Offset referring to start address													*/
/*   |  +--- Length of data																		*/
/*0x00  1 */	byte port_no;					/* no. of C167CR port,							*/
												/* e.g. 2 for P2 or 3 for P3					*/
/*0x01  1 */	byte pin_no;		  			/* no. of port pin								*/
/*0x02  2 */	struct {
					BitField function_area	: 8;	/* func. area to which port pin is assigned	*/
					BitField outputB  		: 1;	/* 0 = input, 1 = output		 			*/
					BitField index			: 2;	/* input[index] or output[index] 			*/
					BitField invert_signalB	: 1;	/* invert logic of signal		 			*/
				} properties;
} PortPin_FA_ROM_type;

/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x0043E0 ---------- Extended sensor settings -------------*/
/*   +------ Offset referring to start address													*/
/*   |  +--- Length of data																		*/
/*0x00  1 */	byte sensor_subtype;  			/* IRMA-S-advanced:								*/
												/* type of pyroelectric detector, 'T', 'U', …	*/
												/* IRMA-S-3D:									*/
												/* DIST4 height range, 'A', 'B', …				*/
/*0x01  1 */	byte operation_mode;			/* IRMA-S-3D: operation mode					*/
/*0x02  2 */	word reserved_2;				/* Reserved for future use						*/
/*0x04  2 */	word reserved_3;				/* Reserved for future use						*/
/*0x06  2 */	word reserved_4;				/* Reserved for future use						*/
/*0x08  2 */	word reserved_5;				/* Reserved for future use						*/
/*0x0A  2 */	word reserved_6;				/* Reserved for future use						*/
/*0x0C  2 */	word reserved_7;				/* Reserved for future use						*/
/*0x0E  2 */	word reserved_8;				/* Reserved for future use						*/
} Ext_Sen_Config_ROM_type;

typedef Ext_Sen_Config_ROM_type Ext_Sen_Config_ROM_Field_type[SENSORS_MAX];

/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004F00 ---------- Extended function area settings ------*/
/*   +------ Offset referring to start address													*/
/*   |  +--- Length of data																		*/
/*0x00  2 */	word door_height;				/* Door height in mm					 		*/
/*0x02  2 */	word door_width;				/* Door width in mm								*/
/*0x04  2 */	struct {
		/* 00 */	BitField prefer_inB		: 1;	/* Preference of incoming people	 		*/
					BitField prefer_outB   	: 1;	/* Preference of outgoing people	 		*/
					BitField prefer_in2B   	: 1;	/* Pref. of incoming people, 2. subarea		*/
					BitField prefer_out2B	: 1;	/* Pref. of outgoing people, 2. subarea		*/
					BitField baggageB		: 1;	/* Many people carrying baggage or bicycle 	*/
					BitField schoolbusB		: 1;	/* Many children						   	*/
					BitField trainB			: 1;	/* 0 = bus, 1 = train						*/
					BitField dist_sen_doorB	: 1;	/* Sensor far from door						*/
		/* 01 */	BitField reserved		: 8;	/* Reserved for future use					*/
				} counting;
/*0x06  2 */	struct {
		/* 00 */	BitField stepsB			: 1;	/* Steps present at door			 		*/
					BitField steps_downB   	: 1;	/* Steps present at door are downwards 		*/
					BitField barB 		  	: 1;	/* Bar present at door						*/
					BitField reserved_1		: 5;	/* Reserved for future use					*/
		/* 01 */	BitField reserved_2		: 8;	/* Reserved for future use					*/
				} door_props;
/*0x08  2 */	word sensor_distance; 			/* Distance between sensors in mm				*/
/*0x0A  2 */	word reserved_3;				/* Reserved for future use						*/
/*0x0C  2 */	word reserved_4;				/* Reserved for future use						*/
/*0x0E  2 */	word reserved_5;				/* Reserved for future use						*/
} Ext_Funct_Area_ROM_type;

typedef Ext_Funct_Area_ROM_type Ext_Funct_Area_ROM_Field_type[FUNCTION_AREAS_MAX];

/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x0059FE ---------- GLORIA Configuration -----------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	byte Preferred_Modem;				/* Radio modem selection:						*/
												/* 0x00: No preference, GSM/GPRS modem			*/
												/*       activated if both radio modems are		*/
												/*       installed. Error if no radio modem		*/
												/*       installed.								*/
												/* 0x01: ISM modem activated. Error if no		*/
												/*       ISM modem installed.					*/
												/* 0x04: GSM/GPRS modem activated. Error if no	*/
												/*       GSM/GPRS modem installed.				*/
												/* 0xFF: Radio modem not used.					*/
/*$01  1 */	byte dummy;							/* word alignment								*/
/*$02 22 */	char phone_number_obc[22];			/* Phone number of OBC							*/
/*$18 22 */	char phone_number_base[22];			/* Phone number of base station					*/
/*$2E 10 */	char pin_code[10];					/* SIM card pin code							*/
/*$38 64 */	char server_url[64];				/* Server URL									*/
/*$78 64 */	char gateway_url[64];				/* Network gateway URL							*/
/*$B8 32 */	char user_name[32];					/* Network access user name						*/
/*$D8 16 */	char password[16];					/* Network access password						*/
/*$E8  1 */	byte network_type;					/* Network connection type: GSM, GPRS, ...		*/
/*$E9  1 */	byte connect_interval;				/* Server connect interval (seconds)			*/
/*$EA 14 */	struct
			{
			/* 00 */ byte Response_Delay_Constant;   		/* Parameter for calcultation of delay in the response to ISM broadcast
															    +-->Aerocomm EEPROM Address
															    |   Parameter Name:          Range/Description:	*/
			/* 01 */ byte RF_Channel_Number;   				/* 0x40 "Channel Number"		 (0x00 - 0x37)      */
			/* 02 */ byte System_ID;   						/* 0x76	"System ID"              (0x00 - 0xFF)      */
			/* 03 */ byte Interface_Timeout;   				/* 0x58	"Interface Timeout"		 (0x02 - 0xFF)      */
			/* 04 */ byte RF_Packet_Size;   				/* 0x5B	"RF Packet Size"   		 (0x01 - 0x80)      */
			/* 05 */ byte CTS_On;   						/* 0x5C	"CTS On"   				 (0x01 - 0xFF)      */
			/* 06 */ byte CTS_On_Hysteresis;   				/* 0x5D	"CTS Off"				 (0x00 - 0xFE)      */
			/* 07 */ byte Transmit_Retries;   				/* 0x4C	"Transmit Retries" 		 (0x01 - 0xFF)      */
			/* 08 */ byte Broadcast_Attempts;   			/* 0x4D	"Broadcast Attempts"	 (0x01 - 0xFF)      */
			/* 09 */ byte Stop_Bit_Delay;   				/* 0x3F	"Stop Bit Delay"   		 (0x00 - 0xFF)      */
			/* 10 */ byte Range_Refresh;   					/* 0x3D	"Range Refresh"   		 (0x01 - 0xFF)      */
			/* 11 */ BitField Server_Client_Mode	: 1;	/* 0x41	"Server/Client Mode"     (0=Server; 1=Client)                                */
					 BitField Auto_Config			: 1;	/* 0x56 bit 0 "Auto Config"      (0=Use EEPROM Values; 1=Auto Configure Values)      */
					 BitField Full_Duplex			: 1;	/* 0x56	bit 1 "Duplex"           (0=Half Duplex; 1=Full Duplex)		                 */
					 BitField DES_Enable			: 1;	/* 0x45	bit 6 "DES Enable"       (0=Disable Encryption; 1=Enable Encryption)		 */
					 BitField Auto_Destination		: 1;	/* 0x56	bit 4 "Auto Destination" (0=Use Destination Address; 1=Use Auto Destination) */
					 BitField Broadcast_Mode		: 1;	/* 0x45 bit 1 "RF Delivery"      (0=Addressed Packets; 1=Broadcast Packets)          */
					 BitField Unicast_Only			: 1;	/* 0x56	bit 5 "Unicast Only"     (0=Disabled; 1=Enabled)				             */
					 BitField Auto_Channel			: 1;	/* 0x56	bit 3 "Client Auto Channel" (0=Disabled; 1=Enabled)							 */
			/* 12 */ BitField Sync_To_Channel		: 1;	/* 0x45	bit 5 "Sync To Channel"  (0=Disabled; 1=Enabled)							 */
					 BitField One_Beacon_Mode		: 1;	/* 0x45	bit 7 "One Beacon Mode"	 (0=Disabled; 1=Enabled)							 */
					 BitField RTS_Enable			: 1;	/* 0x56 bit 2 "RTS Enable"       (0=Ignore RTS; 1=Transceiver obeys RTS)			 */
					 BitField Modem_Mode			: 1;	/* 0x6E "Modem Mode"             (0=Disabled; 1=Enabled)							 */
					 BitField RS485_DE_RE			: 1;	/* 0x7F	"RS485 DE"               (0=Disabled; 1=Enabled)							 */
					 BitField Protocol_Status		: 1;	/* 0xC0	"Protocol Status/Receive ACK" (0=Disabled; 1=Enabled)						 */
					 BitField Parity_Mode			: 1;	/* 0x6F	"Parity"                 (0=Disabled; 1=Enabled)							 */
					 BitField Receive_API			: 1;	/* 0xC1	"Receive API"            (0=Disabled; 1=Enabled)							 */
			/* 13 */ BitField Enhanced_API_Enable	: 1;	/* 0xC6 bit 7 "Enhanced API Control Enable"	(0=Enabled; 1=Disabled)	inv. logic!!!	 */
					 BitField Transmit_API			: 1;	/* 0xC6 bit 1 "API Transmit Packet Enable"  (0=Disabled; 1=Enabled)					 */
					 BitField Enhanced_Receive_API	: 1;	/* 0xC6 bit 0 "Enhanced API Receive Packet Enable" (0=Disabled; 1=Enabled)		     */
					 BitField Send_Data_Complete	: 1;	/* 0xC6 bit 2 "Send Data Complete Enable"   (0=Disabled; 1=Enabled)					 */
			} ISM_settings;
} Gloria_ROM_107_type;
