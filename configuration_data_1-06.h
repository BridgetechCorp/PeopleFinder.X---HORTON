/*==============================================================================================*
 |       Filename: configuration_data_1-06.h                                                    |
 | Project/Module: A21                                                                          |
 |           Date: 09/08/2009 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                 Extensions of structure version 1.6.                                         |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004280 ---------- Height Classes -----------------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00 01 */	byte LowLim;					/* Lower limit of height class */
/*$01 01 */	byte UppLim;					/* Upper limit of height class */
} Height_Classes_ROM_type;


/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x005A00 ---------- GLORIA Configuration -----------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00 22 */	char phone_number_obc[22];			/* Phone number of OBC							*/
/*$16 22 */	char phone_number_base[22];			/* Phone number of base station					*/
/*$2C 10 */	char pin_code[10];					/* SIM card pin code							*/
/*$36 64 */	char server_url[64];				/* Server URL									*/
/*$76 64 */	char gateway_url[64];				/* Network gateway URL							*/
/*$B6 32 */	char user_name[32];					/* Network access user name						*/
/*$D6 16 */	char password[16];					/* Network access password						*/
/*$E6  1 */	byte network_type;					/* Network connection type: GSM, GPRS, ...		*/
/*$E7  1 */	byte connect_interval;				/* Server connect interval (seconds)			*/
/*$E8 14 */	struct
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
} Gloria_ROM_106_type;
