/*==============================================================================================*
 |       Filename: configuration_data_1-05.h                                                    |
 | Project/Module: A21                                                                          |
 |           Date: 09/08/2009 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                 Extensions of structure version 1.5.                                         |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


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
/*$E7  1 */	byte connect_interval;				/* 												*/
} Gloria_ROM_105_type;

