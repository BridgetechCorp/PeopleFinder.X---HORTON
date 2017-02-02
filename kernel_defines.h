/*==============================================================================================*
 |       Filename: kernel_defines.h                                                             |
 | Project/Module: A21                          	                                            |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Declarations of general meaning for module group Kernel.                     |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef KERNEL_DEFINES_INC				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "general.h" 			/* General defines												*/
#include "setup.h"				/* Defines for configurating the system                         */
#include "memory_defines.h"		/* Constants for global variables location with _at attribute   */
#include "types.h"

/*----- Constant Macro Definitions -------------------------------------------------------------*/

/*--- Analyzer General Characteristics ---------------------------------------------------------*/
#define	UNIT_CLASS			0x02	// 0x02 = IRMA Analyzer
#define UNIT_MODEL			0x21	// 0x21 = IRMA 4 Analyzer (A21)

#define ELEMENTS_MAX		14
#define SENSORS_MAX			16
#define FUNCTION_AREAS_MAX	16
#define DOOR_CNT_MAX		4
#define HEIGHT_CLASS_NO_MAX	16
// Usage of preprocessor macro "#define identifier expression":
// If "identifier" is found by preprocessor it is replaced by "expression". As a result
// brackets enclosing "expression" always have to be used to ensure intended evaluation
// of expression.
#define ELEMENTS_TOTAL		(ELEMENTS_MAX * SENSORS_MAX)

#define SENSOR_ELEMENTS_MAX	14

#define SSCSENS_PER_ANA_MAX	4
#define CANSENS_PER_ANA_MAX	8

#define FAS_PER_SEN_MAX		3

#define FACntEMC			4

#define RUNLEVEL_RESET		0	// setting of run level 0 causes software reset of IRMA Analyzer
#define RUNLEVEL_KERN		2	// run level remains at this value if communication module
								// is not valid
#define RUNLEVEL_SERV		3	// set by PC service software like e.g. A21_Assistant and
								// A21_Update, run level remains at this value if recovery mode
								// is active, only run level change 3 -> 0 permitted
#define RUNLEVEL_COMM		4	// IRMA Opera top run level
								// (IRMA installation mode or no CAN IRMA Sensor connected)
#define RUNLEVEL_APPL		5	// IRMA Application module running

#ifdef GATEWAY
#define SER_OUT_BUF_SIZE	5660	// 4 UIP 2.0 frames with maximum length 1415 byte
#define SER_IN_BUF_SIZE		5660	// 4 UIP 2.0 frames with maximum length 1415 byte
#else
#define SER_OUT_BUF_SIZE	1024
#define SER_IN_BUF_SIZE		1024
#endif

#define RUNLEVEL_JOB_SIZE	10

#define PROT_ID_UNKNOWN		-1
#define PROT_ID_IRMA		0
#define PROT_ID_CL			1
#define PROT_ID_IBIS		2
#define PROT_ID_J1708		3

#define Def_IRMABaudR 		38400
#define Def_IBISBaudR 		1200
#define Def_IBISINEOBaudR	9600
#define Def_J1708BaudR		9600

// Following definitions have to be reenabled if IRMA Opera variants with different
// IBIS baud rates should be used in future. Equivalent definitions in header file
// "configuration_data.h" have to be disabled in this case.	Furthermore Def_IBISBaudR
// has to be replaced by IBISBaudR in function "Setup_Config_Data_Default", source file
// "configuration.c".
//#ifdef IBIS_PROTOCOL
//	#if defined(IBIS9600)
//		#define IBISBaudR 9600
//	#elif defined(IBIS19200)
//		#define IBISBaudR 19200
//	#else
//		#define IBISBaudR 1200
//	#endif
//#endif

#define A21Name			   		"A21"

#define IRMA_SEN_DEV_NO_LEN				9	// terminating zero included
#define CAN_IRMA_SEN_TYPE_NAME_LEN		17	// terminating zero included
#define CAN_IRMA_SEN_FIRMWARE_NAME_LEN	9	// terminating zero included


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef	struct 
{
	BitField A21C_UpdateDoorStateB		: 1;			// bit  0
	BitField A21C_NoTempTransmissionB	: 1;			// bit  1
	BitField A21C_ReqCANErrCntsB		: 1;			// bit  2
	BitField A21C_MaxOf2CyclesB			: 1;			// bit  3
	BitField A21C_reserved				: 12;
	BitField A21S_CheckNeutralErrorB	: 1;			// bit  0
	BitField A21S_UpdateIREDActivityB	: 1;			// bit  1
	BitField A21S_reserved				: 14;
} Operation_Flags_type;

typedef	struct 
{
	BitField Out_OverflowB				: 1;			// bit  0
	BitField In_OverflowB				: 1;			// bit  1
	BitField OverrunErrorB				: 1;			// bit  2
	BitField FrameErrorB				: 1;			// bit  3
	BitField ParityErrorB				: 1;			// bit  4
} ASC0Error_type;

typedef struct
{
	ASC0Error_type ASC0;
	struct {
		BitField Tra_Msg_InvalidB 		: 1;			// bit  0
		BitField Rec_Msg_InvalidB 		: 1;			// bit  1
		BitField Rec_Msg_LostB			: 1;			// bit  2
		BitField Buffer_OverflowB		: 1;			// bit  3
		BitField reserved				: 10;
		BitField CAN_error_warnB		: 1;			// bit 14
		BitField CAN_bus_offB			: 1;			// bit 15
	} CAN1;
	struct {
		BitField Buffer_OverflowB				: 1;	// bit  0
		BitField Door_Contacts_Ever_ChangedB	: 1;	// bit  1
		BitField Door_States_Ever_Set_By_MsgB	: 1;	// bit  2
	} Sensor_Interface;
	struct {
		BitField RTC_ErrorB				: 1;			// bit  0
		BitField NVRAM_ErrorB			: 1;			// bit  1
		BitField RTC_SynchronizedB 		: 1;			// bit  2
	} Optional_Hardware;
	struct {
		BitField DataB					: 1;			// bit  0
		BitField OverflowB				: 1;			// bit  1
	} Logger;
} A21_Status_type;

typedef struct
{
	char name[16];
	byte version;
	byte revision;
	char date[12];
	char time[10];
	char IRMAOPERA_IntVer[6];
	dword checksum;
} Sector_Information_type;  

typedef struct
{
	char name[16];
	byte version;
	byte revision;
	char date[12];
	char time[10];
	dword checksum;
	dword checksum_ana;
	boolean okB;
} SectorInformationTable_type;

typedef struct
{
	byte  *start;
	byte  *end;
	Sector_Information_type *info;
} Sector_Pointers_type;

typedef struct							/* Struct for asynchronous serial interface				*/
{
	dword baudrate;						/*		Baudrate in bps									*/
	byte data_bits;						/*		7, 8, 9											*/
	char parity;						/*		'N' = None, 'O' = Odd, 'E' = Even				*/
	byte stop_bits;						/*		1, 2											*/
	byte flow_control;					/*		0 = no flow control, 1 = RTS/CTS				*/
} RS232_Parameter_type;

typedef struct
{
	byte terminal_symbol;
	boolean call_terminalB;
	boolean call_checksumB;
	void (*call_terminal_function)(void);
	void (*call_checksum_function)(void);
	word timeout;
} Parser_type;

typedef struct
{
	byte Year;
	byte Month;
	byte Day;
} Date_type;

typedef struct
{
	byte Hour;
	byte Minutes;
	byte Seconds;
} Time_type;

#if !defined(NO_LOGGER) && !defined(A21CL)
typedef struct
{
	byte Control;
	byte Seconds;
	byte Minutes;
	byte Hour;
	byte Day_Of_Week;
	byte Day;
	byte Month;
	byte Year;
} RTC_type;
#endif

typedef struct
{
	void (*function)(void);
	word counter_end;
	word counter;
} timer_job_type;

typedef struct
{
	void (*function)(void);
	word counter_reload;
	word counter;
} timeout_job_type;

#ifndef NO_LOGGER
typedef struct
{
	void (*function)(void);
	word interval;
	struct 
	{
		word minutes;
		boolean tomorrow;
	} next;
} Clock_Job_type;
#endif

typedef struct
{
	boolean inputB;
	boolean outputB;
	boolean stateB;
	boolean ignoreB;
	union {
		struct {
			word rising;
			word falling;
		} chatter;
		word pulse_width;
	} duration;
} Channel_type;

typedef struct
{
	boolean hardware_errorB;
	boolean neutral_errorB;
	boolean range_errorB[SENSOR_ELEMENTS_MAX];
	boolean no_range_testB;
} SensorStatus_type;


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/


#define KERNEL_DEFINES_INC
#endif	// end of "#ifndef KERNEL_DEFINES_INC"
