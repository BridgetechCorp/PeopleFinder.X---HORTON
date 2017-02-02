/*==============================================================================================*
 |       Filename: uip_defines.h                                                                |
 | Project/Module: A21/module group Communication                                               |
 |           Date: 10/19/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of universal IRMA protocol (UIP).                             |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef UIP_DEFINES_INC					/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define UIP_10_FRAME_LENGTH_MIN			10
#define UIP_10_FRAME_LENGTH_MAX			200
// Usage of preprocessor macro "#define identifier expression":
// If "identifier" is found by preprocessor it is replaced by "expression". As a result
// brackets enclosing "expression" always have to be used to ensure intended evaluation
// of expression.
#define	UIP_10_PAYLOADLEN_MAX			(UIP_10_FRAME_LENGTH_MAX - UIP_10_FRAME_LENGTH_MIN)		// 190
#define UIP_10_FRAME_DES				4														//   4
#define UIP_10_FRAME_SRC				(UIP_10_FRAME_DES + 1)									//   5
#define UIP_10_FRAME_DL					(UIP_10_FRAME_SRC + 1)									//   6
#define UIP_10_FRAME_DK					(UIP_10_FRAME_DL  + 1)									//   7
#define UIP_10_FRAME_VER				(UIP_10_FRAME_DK  + 1)									//   8
#define UIP_10_FRAME_W1					(UIP_10_FRAME_VER + 1)									//   9
#define UIP_10_MESSAGE_DATAOFFSET		(UIP_10_FRAME_W1 - UIP_10_FRAME_DK)						//   2
#define UIP_10_MSG_LEN_MIN				(UIP_10_MESSAGE_DATAOFFSET)								//   2
#define UIP_10_MSG_LEN_MAX				(UIP_10_PAYLOADLEN_MAX + UIP_10_MESSAGE_DATAOFFSET)		// 192
#define UIP_10_OVERHEAD					(UIP_10_FRAME_LENGTH_MIN - UIP_10_MESSAGE_DATAOFFSET)	//   8

#define UIP_20_FRAME_LENGTH_MIN			15
#define UIP_20_FRAME_LENGTH_MAX			1415
#define	UIP_20_PAYLOADLEN_MAX			(UIP_20_FRAME_LENGTH_MAX - UIP_20_FRAME_LENGTH_MIN)		// 1400
#define UIP_20_FRAME_LEV				6														//    6
#define UIP_20_FRAME_DES				(UIP_20_FRAME_LEV + 1)									//    7
#define UIP_20_FRAME_SRC				(UIP_20_FRAME_DES + 1)									//    8
#define UIP_20_FRAME_DL					(UIP_20_FRAME_SRC + 1)									//    9
#define UIP_20_FRAME_DK					(UIP_20_FRAME_DL  + 2)									//   11
#define UIP_20_FRAME_VER				(UIP_20_FRAME_DK  + 1)									//   12
#define UIP_20_FRAME_CMD				(UIP_20_FRAME_VER + 1)									//   13
#define UIP_20_FRAME_W1					(UIP_20_FRAME_CMD + 1)									//   14
#define UIP_20_MESSAGE_DATAOFFSET		(UIP_20_FRAME_W1 - UIP_20_FRAME_DK)						//    3
#define UIP_20_MSG_LEN_MIN				(UIP_20_MESSAGE_DATAOFFSET)								//    3
#define UIP_20_MSG_LEN_MAX				(UIP_20_PAYLOADLEN_MAX + UIP_20_MESSAGE_DATAOFFSET)		// 1403
#define UIP_20_OVERHEAD					(UIP_20_FRAME_LENGTH_MIN - UIP_20_MESSAGE_DATAOFFSET)	//   12

#ifdef GATEWAY
#define UIP_FRAME_LENGTH_MAX			(UIP_20_FRAME_LENGTH_MAX)
#define	UIP_PAYLOADLEN_MAX				(UIP_20_PAYLOADLEN_MAX)
#define UIP_MSG_LEN_MAX					(UIP_20_MSG_LEN_MAX)
#else
#define UIP_FRAME_LENGTH_MAX			(UIP_10_FRAME_LENGTH_MAX)
#define	UIP_PAYLOADLEN_MAX				(UIP_10_PAYLOADLEN_MAX)
#define UIP_MSG_LEN_MAX					(UIP_10_MSG_LEN_MAX)
#endif

#define APAVER							0x10	// Most recent version of "A21 Protocol" supported by A21 firmware
#define APMVER							0x10	// Minimum version of "A21 Protocol" supported by A21 firmware

#define IRMAAddrReserveFirst			0xF0
#define IRMAAddrOBC						(IRMAAddrReserveFirst)
#define IRMAAddrServiceDev				0xFE
#define IRMAAddrUniversalDest			0xFF

/*----- IRMA addresses in GLORIA network -------------------------------------------------------*/
#define GLORIA_MASTER_ADDR				0x01	// IRMA address of GLORIA master (A21 base module)
#define EXT_NETWORK_SERVER_ADDR			0xFC	// IRMA address of external network server (IrisBaseStation or GLORIA web server)
#define GLORIA_MODULE_ADDR				0xFD	// IRMA address of GLORIA module
#define GLORIA_SLAVE1_ADDR				0x02	// IRMA address of first slave analyzer in a GLORIA bus.
#define GLORIA_SLAVE2_ADDR				0x03 	// IRMA address of second slave analyzer in a GLORIA bus.
#define GLORIA_SLAVE3_ADDR				0x04 	// IRMA address of third slave analyzer in a GLORIA bus.

#define IRMA_Mv10_HeadLen				5
#define IRMA_Mv10_MaxMemByteCnt			(UIP_10_PAYLOADLEN_MAX - IRMA_Mv10_HeadLen)
#define IRMA_Mv11_HeadLen				4
#define IRMA_Mv20_HeadLen				6
#define IRMA_Mv20_MaxMemByteCnt			(UIP_20_PAYLOADLEN_MAX - IRMA_Mv20_HeadLen)

#define IRMA_Nv10_HeadLen 				4

#define IRMA_Pv10_HeadLen				6

#define IRMA_Rv10_PayloadLen   			1
#define IRMA_Rv10_S_PayloadLen 			5

#define IRMA_Bv10_PayloadLen 			8
#define IRMA_Bv11_PayloadLen 			12

#define IRMA_Cv30_PayloadLenMin			6

#define IRMA_Dv20_PayloadLenMin			4

#define IRMA_Ev10_PayloadLenMin			2
#define IRMA_Ev10_U_PayloadLen			3
#define IRMA_Ev11_PayloadLenMin			3

#define IRMA_Iv10_ProdIdLen				16    
#define IRMA_Iv11_ProdIdLen				32    
#define IRMA_Iv10_PayloadLenMin			24
#define IRMA_Iv11_PayloadLenMin			40
#define IRMA_Iv12_PayloadLenMin			1
#define IRMA_Iv31_PayloadLen 			4

#define IRMA_L_S_PayloadLen				1
#define IRMA_L_D_PayloadLen				2
#define IRMA_L_L_PayloadLen				2
#define IRMA_L_C_PayloadLenMin			10

#define IRMA_Sv20_PayloadLenMin			11	// ((sizeof(A21_Status) + sizeof(byte))
#define IRMA_Sv22_PayloadLenMin			1	// sizeof(byte)
#define IRMA_SvE0_PayloadLen			64	// (CANSENS_PER_ANA_MAX * sizeof(dword))

#define IRMA_Vv20_PayloadLen			FIRMWARE_NAME_LENGTH

#define IRMA_Zv10_PayloadLenMin			19	// ((sizeof(word) + SENSORS_MAX + sizeof(byte))
#define IRMA_Zv10_CycleTableLenMax		(UIP_10_PAYLOADLEN_MAX - IRMA_Zv10_PayloadLenMin)

#define IRMA_gv10_PayloadLen			21
#define IRMA_gv11_PayloadLen			27

#define IRMA_sv10_PayloadLen			2
#define IRMA_sv20_PayloadLen			11

#define IRMA_hv10_PayloadLen			3
#define IRMA_hv11_PayloadLen			4

#define IRMA_Av10_EmbLen				4	// EDES, ESRC, EDK, EVER
#define IRMA_Av50_minEmbLen				5	// <CAN identifier>4, DLC, <CAN data>DLC
#define IRMA_Av60_s_PayloadLenMin		10	// <CAN identifier>4, E_UIP_VER_LEV, <E_R_DL>2, EDK, EVER, ESCMD
#define IRMA_rv10_EmbLen				6	// DEVID, NETID, PN1, PN2, EDK, EVER

#define CustomProtIdStrLen				2

#define IRMAErr_Confirmation			0x00
// reserved: 0x01...0x03, internal errors of UIP stack
#define IRMAErr_UnkTypeIdent			0x04	// unknown type identifier (DK)
#define IRMAErr_UnkTypeIdentVer			0x05	// unknown version of type identifier (VER)
#define IRMAErr_InvMsgLen				0x06	// invalid length of message (DL)
#define IRMAErr_UnkSubCmd				0x07	// unknown message subcommand
#define IRMAErr_InvEmbMsg				0x08	// invalid embedded message
#define IRMAErr_SrcAddrNotAllowed		0x09	// source address (SRC) not permitted for this message
#define IRMAErr_InconsistentPayload		0x0A	// inconsistent payload
#define IRMAErr_UnkPayloadParam			0x0B
#define IRMAErr_UnkCommand				0x0C
#define IRMAErr_UnkCommProt				0x0D
// reserved: 0x0E
#define IRMAErr_SetNotAllowed			0x0F
#define IRMAErr_ExecutionErr			0x10
#define IRMAErr_InvFAAddr				0x11
#define IRMAErr_AllFAsNotAllowed		0x12
#define IRMAErr_InvASC0Param			0x13
#define IRMAErr_InvMemAddr				0x14
#define IRMAErr_MemWrite				0x15
#define IRMAErr_InvRunLevel				0x16
#define IRMAErr_InvMemByteCnt			0x17
#define IRMAErr_InvSenNo				0x18
#define IRMAErr_InstModeTimeOut			0x19
#define IRMAErr_InstModeOverflow		0x1A
#define IRMAErr_InvHeightClass			0x1B
#define IRMAErr_InvL7Parameter			0x1C
// specified in document iris UIP:
// 0x1D == Invalid parameter number
// 0x1E == Invalid version of parameter number
// 0x1F == Invalid syntax of configuration parameter
#define IRMAErr_InvRecConf				0x20
#define IRMAErr_WrongOverwriteFlag		0x21	// iris UIP: reserved
#define IRMAErr_InvEventTimestamp		0x22	// iris UIP: reserved
#define IRMAErr_WrongEventDataId		0x23	// iris UIP: reserved
#define IRMAErr_WrongEventDataVer		0x24	// iris UIP: reserved
#define IRMAErr_InvEventLength			0x25	// iris UIP: reserved
#define IRMAErr_LoggerFull				0x26	// iris UIP: reserved
#define IRMAErr_LoggerIntegrityError	0x27	// iris UIP: reserved
// reserved: 0x28...0x2F
#define IRMAErr_InvSenList				0x30
#define IRMAErr_InvCycleTab				0x31

#define IRMAPrefix1						'I'
#define IRMAPrefix2						'R'
#define IRMAPrefix3						'M'
#define IRMAPrefix4						'A'

#define IrmaDevClassIdLen				16	// "A21" for IRMA Analyzer

// Refer to comments before lines
//	if( !can_irma_instmode_activeB && !can_sensor_conf_changedB )
//	{
//		Set_Runlevel(RUNLEVEL_APPL);
// in source file "communication.c".
// IRMA Opera versions <= 2.99g:			IRMA_FRAME_TIMEOUT == 1000
// IRMA Opera versions >= 2.99h && <= 5.90:	IRMA_FRAME_TIMEOUT == 10
// IRMA Opera versions >= 5.91:				IRMA_FRAME_TIMEOUT == 100
// Timeout value of 100ms should be good compromise between need for fast resynchronization during
// reception of UIP frames and consideration of long applic_main_init and applic_main runtimes.
#define IRMA_FRAME_TIMEOUT				100	// Permitted idle time within receive IRMA frame < 100ms.
#define LOCAL_BUFFER_MAX				(UIP_FRAME_LENGTH_MAX)
#define DRIVE_DETECTION_SPEED			2778					/* 2778 = 10 km/h */
#define HALT_DETECTION_SPEED			1389					/* 1389 = 5 km/h */
#define HEIGHT_CLASS_OFFSET 			2

// Maximum IRMA segment length for UIP-over-CI message embedding is equal to 105 = 15 * 7 byte.
#define IRMA_Mv11_MemByteCnt_UppLim		(105 - 3 - IRMA_Mv11_HeadLen)	// 98

#define NOT_ALLOWED						0
#define ROM_ALLOWED						1
#define RAM_ALLOWED						2
#ifndef NO_LOGGER
	#define LOG_ALLOWED					3
#endif


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	byte FA_Addr;
	byte FA_Status;
	word PersonsIn;
	word PersonsOut;
} C_20_type;
// Usage of byte constant enhances code optimization.
#define C_20_TYPE_SIZE	(byte)sizeof(C_20_type)	// 6

typedef struct
{
	byte HC_Lower_Limit;
	byte HC_Upper_Limit;
	byte FA_Addr;
	byte FA_Status;
	word PersonsIn;
	word PersonsOut;
} C_21_type;
#define C_21_TYPE_SIZE	(byte)sizeof(C_21_type)	// 8

typedef struct
{
	byte FA_Addr;
	byte FA_Status;
	word PersonsIn;
	word PersonsOut;
	BitField Lat_Low	: 16;		// Latitude, lower bytes
	BitField Lat_High	: 16;		// Latitude, higher bytes
	BitField Lng_Low	: 16;		// Longitude, lower bytes
	BitField Lng_High	: 16;		// Longitude, higher bytes
	BitField Speed		: 16;		// Speed
	BitField Alt		: 13;		// Altitude
	BitField GPS_Status	:  3;		// GPS Status
} C_40_type;
#define C_40_TYPE_SIZE	(byte)sizeof(C_40_type)	// 18

typedef struct
{
	byte HC_Lower_Limit;
	byte HC_Upper_Limit;
	byte FA_Addr;
	byte FA_Status;
	word PersonsIn;
	word PersonsOut;
	BitField Lat_Low	: 16;		// Latitude, lower bytes
	BitField Lat_High	: 16;		// Latitude, higher bytes
	BitField Lng_Low	: 16;		// Longitude, lower bytes
	BitField Lng_High	: 16;		// Longitude, higher bytes
	BitField Speed		: 16;		// Speed
	BitField Alt		: 13;		// Altitude
	BitField GPS_Status	:  3;		// GPS Status
} C_41_type;
#define C_41_TYPE_SIZE	(byte)sizeof(C_41_type)						// 20

#define C_50_TYPE_SIZE	(byte)(2 * sizeof(byte) + 2 * sizeof(word))	//  6

typedef struct
{
	byte FA_Addr;
	byte Left;
	byte Right;
} D_10_type;
// sizeof(D_10_type) == 4 !
#define D_10_TYPE_SIZE	3

typedef struct
{
	byte FA_Addr;
	byte Left;
	byte Right;
	BitField Lat_Low	: 16;		// Latitude, lower bytes
	BitField Lat_High	: 16;		// Latitude, higher bytes
	BitField Lng_Low	: 16;		// Longitude, lower bytes
	BitField Lng_High	: 16;		// Longitude, higher bytes
	BitField Speed		: 16;		// Speed
	BitField Alt		: 13;		// Altitude
	BitField GPS_Status	:  3;		// GPS Status
} D_11_type;
#define D_11_TYPE_SIZE	15

typedef struct
{
	signed long Lat;
	signed long Lng;
	signed long Alt;
	dword Speed;
	dword Course;
	byte GPS_Status;
} G_10_type;	/* 5 * 4 + 1 = 21 Bytes = 168 Bit */
// Usage of byte constant enhances code optimization.
#define G_10_TYPE_SIZE	21

typedef struct
{
	BitField Lat_Low	: 16;		// Latitude, lower bytes
	BitField Lat_High	: 16;		// Latitude, higher bytes
	BitField Lng_Low	: 16;		// Longitude, lower bytes
	BitField Lng_High	: 16;		// Longitude, higher bytes
	BitField Speed		: 16;		// Speed
	BitField Alt		: 13;		// Altitude,	bits 12 - 0
	BitField GPS_Status	:  3;		// GPS Status,	bits 15 - 13
// In specification iris UIP lower byte of bit field containing Altitude and GPS status is named
// ALT1 and upper byte is named AL_ST.
} G_14_type;
#define G_14_TYPE_SIZE	(byte)sizeof(G_14_type)	// 12

#define G_14_ALT_SHIFT		10
#define G_14_ALT_OFFSET		500000	// in 0,001 m

typedef struct
{
	byte ISM;
	byte GPS;
	byte TMA;
	byte GSM;
} Hardware_Info_type;

#define s_20_TYPE_SIZE		11


#define UIP_DEFINES_INC
#endif	// end of "#ifndef UIP_DEFINES_INC"
