/*==============================================================================================*
 |       Filename: configuration_defines.h                                                      |
 | Project/Module: A21, GATEWAY or OBC                                                          |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description:                                                                              |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CONFIGURATION_DEFINES_INC		/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "kernel_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define MaxFAAddr			64			/* FA address: 1...64									*/

#define MaxElemAddr			0x0D		/* element address range: 0x00...0x0D (14-channel ADC)	*/
#define ElemAddr_800		0x0B		/* ADC test voltage 1/2(VREF - VREF-)					*/
#define ElemAddr_000		0x0C		/* ADC test voltage VREF-								*/
#define ElemAddr_FFF		0x0D		/* ADC test voltage VREF+								*/

#define END_OF_TABLE		0xFF		/* Mark end of table									*/

#define A21ClassIdStrLen  		3		// Examples: "S__", "C__", "CLG"
#define MaxA21DevNoStrLen		8		// Examples: "21 1234", "22_0775", "23-31859"
//#define MaxFAConfStrItemLen		4		// Examples: "1N-", "24P-"

// IRMA functions:
#define UNDEF_IRMA_FUNC			0x00	// undefined IRMA function

// Door types:
#define UNDEF_DOOR_TYPE			0x00	// undefined door type
#define INWARD_OPEN_DOOR		0x01	// inward opening glider door	(Innenschwenktür)
#define SLID_PLUG_DOOR			0x02	// sliding plug door			(Schwenkschiebetür)
#define OUTWARD_SLID_DOOR		0x03	// outward sliding plug door	(Außenschwenkschiebetür)
#define REVOLVING_DOOR			0x04	// revolving door				(Drehtür)
#define PUSH_TO_OPEN_DOOR       0x05    // passengers have to open the door by pushing it.
										// This leads to big signal amplitudes.

#define MAX_A21_SWITCHCNT		4
#ifdef SW4
	#define A21_SWITCHCNT 		MAX_A21_SWITCHCNT
#else
	#ifdef SW2
		#define A21_SWITCHCNT	2
	#else
		#define A21_SWITCHCNT	0
	#endif
#endif


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct {	/* ------------------------------------------------ Struct Version 1.00 --- */
	char	product_id[32];				/* Product name of analyzer								*/
	struct {							/* Device id of analyzer								*/
		byte year;
		word serial_number;
	} device_id;
	char	CPU_name[16];				/* Type of microcontroller (C167CR, ST10R167)			*/
	dword 	fCPU;						/* Processor Clock. Unit: Hz							*/
	dword 	fOSC;						/* Oscillator Clock. Unit: Hz							*/
	struct {							/* Type of flash EPROM									*/
		byte	manufacturer_id;		/* 		As provided by Flash-EPROM						*/
		byte	device_id;				/* 		As provided by Flash-EPROM						*/
		char	name[14];
	} EPROM;
	word 	SRAM_size;					/* Size of SRAM in kB, 1 kB = 1024 bytes				*/
	char 	RTC_name[16];				/* Type of nonvolatile RAM								*/
	word	RTC_id;						/* Internal id of RTC									*/
	word 	NVRAM_size;					/* Size of NVRAM in kB									*/
	char	sensor_interface[16];		/* Type of sensor interface (SSC, CAN)					*/
	char	system_interface[16];		/* Type of system interface (ASC0-RS232, ...)			*/
} Configuration_System_type;			/* Configuration_System_type ConfigSystem				*/

/* -------------------------------------------------------------------- Struct Version 1.00 --- */
/* Changes to this structure type definition must be thouroughly checked for compatibility to	*/
/* module group Application.																	*/
typedef struct {
	byte element;						/* Address of element, e.g. 0x80, 0x20, ...				*/
	byte sensor;						/* Number of sensor which element belongs to			*/
	byte element_type;
	byte position[3];					/* Position in geometrical order						*/
	struct {							/* Environment of element								*/
		BitField no_neighbour_leftB		: 1;	/* Wall is on the left side of the element		*/
		BitField no_neighbour_rightB	: 1;	/* Wall is on the right side of the element		*/
		BitField no_neighbour_line_leftB: 1;	/* Wall is on the left side of the element		*/
		BitField no_neighbour_line_rightB:1;	/* Wall is on the right side of the element		*/
		BitField bar_leftB				: 1;	/* Dividing bar is on the left side of the el.	*/
		BitField bar_rightB				: 1;	/* Dividing bar is on the right side of the el.	*/
		BitField bar_line_leftB			: 1;	/* Dividing bar is on the left side of the el.	*/
		BitField bar_line_rightB		: 1;	/* Dividing bar is on the right side of the el.	*/
		BitField outer_elementB			: 1;	/* Outer element								*/
	} environment;
} Cycle_Table_type;

typedef struct
{
	byte Response_Delay_Constant;   		/* Parameter for calcultation of delay in the response to ISM broadcast
											    +-->Aerocomm EEPROM Address
											    |   Parameter Name:          Range/Description:	*/
	byte RF_Channel_Number;   				/* 0x40 "Channel Number"		 (0x00 - 0x37)      */
	byte System_ID;   						/* 0x76	"System ID"              (0x00 - 0xFF)      */
	byte Interface_Timeout;   				/* 0x58	"Interface Timeout"		 (0x02 - 0xFF)      */
	byte RF_Packet_Size;   					/* 0x5B	"RF Packet Size"   		 (0x01 - 0x80)      */
	byte CTS_On;   							/* 0x5C	"CTS On"   				 (0x01 - 0xFF)      */
	byte CTS_On_Hysteresis;   				/* 0x5D	"CTS Off"				 (0x00 - 0xFE)      */
	byte Transmit_Retries;   				/* 0x4C	"Transmit Retries" 		 (0x01 - 0xFF)      */
	byte Broadcast_Attempts;   				/* 0x4D	"Broadcast Attempts"	 (0x01 - 0xFF)      */
	byte Stop_Bit_Delay;   					/* 0x3F	"Stop Bit Delay"   		 (0x00 - 0xFF)      */
	byte Range_Refresh;   					/* 0x3D	"Range Refresh"   		 (0x01 - 0xFF)      */
	BitField Server_Client_Mode		: 1;	/* 0x41	"Server/Client Mode"     (0=Server; 1=Client)                                */
	BitField Auto_Config			: 1;	/* 0x56 bit 0 "Auto Config"      (0=Use EEPROM Values; 1=Auto Configure Values)      */
	BitField Full_Duplex			: 1;	/* 0x56	bit 1 "Duplex"           (0=Half Duplex; 1=Full Duplex)		                 */
	BitField DES_Enable				: 1;	/* 0x45	bit 6 "DES Enable"       (0=Disable Encryption; 1=Enable Encryption)		 */
	BitField Auto_Destination		: 1;	/* 0x56	bit 4 "Auto Destination" (0=Use Destination Address; 1=Use Auto Destination) */
	BitField Broadcast_Mode			: 1;	/* 0x45 bit 1 "RF Delivery"      (0=Addressed Packets; 1=Broadcast Packets)          */
	BitField Unicast_Only			: 1;	/* 0x56	bit 5 "Unicast Only"     (0=Disabled; 1=Enabled)				             */
	BitField Auto_Channel			: 1;	/* 0x56	bit 3 "Client Auto Channel" (0=Disabled; 1=Enabled)							 */
	BitField Sync_To_Channel		: 1;	/* 0x45	bit 5 "Sync To Channel"  (0=Disabled; 1=Enabled)							 */
	BitField One_Beacon_Mode		: 1;	/* 0x45	bit 7 "One Beacon Mode"	 (0=Disabled; 1=Enabled)							 */
	BitField RTS_Enable				: 1;	/* 0x56 bit 2 "RTS Enable"       (0=Ignore RTS; 1=Transceiver obeys RTS)			 */
	BitField Modem_Mode				: 1;	/* 0x6E "Modem Mode"             (0=Disabled; 1=Enabled)							 */
	BitField RS485_DE_RE  			: 1;	/* 0x7F	"RS485 DE"               (0=Disabled; 1=Enabled)							 */
	BitField Protocol_Status		: 1;	/* 0xC0	"Protocol Status/Receive ACK" (0=Disabled; 1=Enabled)						 */
	BitField Parity_Mode			: 1;	/* 0x6F	"Parity"                 (0=Disabled; 1=Enabled)							 */
	BitField Receive_API			: 1;	/* 0xC1	"Receive API"            (0=Disabled; 1=Enabled)							 */
	BitField Enhanced_API_Enable	: 1;	/* 0xC6 bit 7 "Enhanced API Control Enable"	(0=Enabled; 1=Disabled)	inv. logic!!!	 */
	BitField Transmit_API			: 1;	/* 0xC6 bit 1 "API Transmit Packet Enable"  (0=Disabled; 1=Enabled)					 */
	BitField Enhanced_Receive_API	: 1;	/* 0xC6 bit 0 "Enhanced API Receive Packet Enable" (0=Disabled; 1=Enabled)		     */
	BitField Send_Data_Complete		: 1;	/* 0xC6 bit 2 "Send Data Complete Enable"   (0=Disabled; 1=Enabled)					 */
} ISM_settings_type;

typedef struct {
	byte version;
	byte revision;
	char configuration_name[16];		/* Name of configuration								*/
	char software_filename[64];
	RS232_Parameter_type	ASC0;		/* Asynchronous serial interface parameter				*/
	struct ASC0_Real_struct {			/* Exact baudrate and deviation to demanded baudrate	*/
		word s0bg;
		dword baudrate;
		signed long deviation;
	} ASC0_real;						/* Exact baudrate and deviation to demanded baudrate	*/
	struct {							/* Parameter of interface mode							*/
		boolean half_duplexB;				/*		0 = full duplex, 1 = half duplex on ASC0		*/
		word send_delay;				/* 		delay response, unit: ms						*/
	} ASC0_control;
	// Constant PROT_ID_UNKNOWN is equal number - 1.
	char communication_protocol;		/* Communication protocol: 0 = IRMA, 1 = CL, 2 = IBIS, 3 = J1708 */
	dword SSC_baudrate;					/* Synchronous serial interface baudrate				*/
	struct {							/* Parameter for pulse width modulator					*/
		word period;
		word pulse_width;
		word offset;
		word pp;						/*		Settings for PWM register:	PPx period register	*/
		word pw;						/*			PWx pulse width register					*/
		word pt;						/*			PTx counter register						*/
	} PWM[4];
	byte function_areas;				/* Number of function areas								*/
	byte sensors;						/* Number of sensors									*/
	struct {
		int total;						/* Total number of elements								*/
		int passive;					/* Number of passive elements							*/
		int active;						/* Number of active elements							*/
		int neutral;					/* Number of neutral level elements						*/
		int distance;					/* Number of distance elements							*/
		int test_voltage;				/* Number of test voltage elements						*/
		int others;						/* Other elements										*/
	} elements;
	char sensor_types;					/* Number of different sensor types						*/
	struct {							/* IRMA Parameter										*/
		word address;					/* 		IRMA device id									*/
		char tag[6];					/*		IRMA device tag (4 characters)					*/
		word status_interval;			/*		Seconds between two status reports				*/
	} irma;
	struct {							/* Function area information							*/
		word address;						/* Address of function area							*/
		struct {
			byte type;					/* Type of door											*/
			word height;				/* Replaced by ext_funct_area[].door_height.			*/
			boolean stepsB;				/* Replaced by ext_funct_area[].door_props.stepsB		*/
			struct {
				BitField availableB			: 1;
				BitField from_externB		: 1;
				BitField exactB				: 1; 	/* Exact door position is provided			*/
				BitField simulationB		: 1;
				BitField unused_3B			: 1;	/* IRMA Opera 5.95 and 5.96: barB			*/
			} position;
			boolean bogotaB;
			struct {
				byte after_count_wait_time;
				byte door_closed_time;
				byte door_event_countdown;
			} simulation;
			struct {
				byte channel_no;						/* Port number of signal				*/
				struct {								/* Type of signal						*/
					BitField door_switchB		: 1;
					BitField closing_signalB	: 1;
					BitField fifty_percentB		: 1;
					BitField motion_signalB		: 1;
					BitField left_leafB			: 1;
					BitField right_leafB		: 1;
				    BitField door_delayB		: 1;
				} type;
				boolean invert_signalB;
				word chatter_time_01;					/* Unit: ms						  		*/
				word chatter_time_10;					/* Unit: ms						  		*/
			} input[2];
			struct {
				byte channel_no;
				boolean invert_signalB;
				word signal_width;						/* Unit: ms								*/
			} output;
		} door;
		word cycle_time;					/* Time between two cycles in µs						*/
		byte no_of_sensors;					/* Number of sensors in this function area				*/
		struct {							/* Function / job										*/
			BitField countingB		: 1;
			BitField observationB	: 1;
			BitField controlB		: 1;
			BitField heightclassB	: 1;
			BitField scalableB		: 1;
		} function;
	} function_area[FUNCTION_AREAS_MAX];
	struct {
		char type[16];						/* Type of sensor, e.g. "8xxxx"							*/
		dword address;						/* Address of sensor									*/
		struct {
			BitField turnedB		: 1;	/* Sensor is turned by 180 deg.							*/
			BitField som_specifiedB	: 1;	/* Sensor operation mode specified						*/
											/* FALSE: compatibility mode, DIST4:					*/
											/*        slave  if turnedB == FALSE,					*/
											/*        master if turnedB == TRUE						*/
											/* TRUE:  sensor operation mode given by sen_op_mode	*/
			BitField sen_op_mode	: 3;	/* Sensor operation mode, DIST4: 0 = master, 1 = slave	*/
		} environment;
		struct {
			byte function_area;				/* Function area in which sensor is part of				*/
			byte number;					/* Number of sensor in function area					*/
		} area[FAS_PER_SEN_MAX];
//		byte no_of_channels;
		byte element_type[14];
	} sensor_config[SENSORS_MAX];
	Cycle_Table_type cycle_table[ELEMENTS_TOTAL];
	struct {
		dword vehicle_id;
	} obc;
	struct {
		word minutes;
		word seconds;
	} logger;
	struct {
		char phone_nr_obc[22];
		char phone_nr_base[22];
		char pin_code[10];
		char server_url[64];
		char gateway_url[64];				/* Network gateway URL							*/
		char user_name[32];					/* Network access user name						*/
		char password[16];					/* Network access password						*/
		byte network_type;					/* Network connection type: GSM, GPRS, ...		*/
		byte connect_interval;
		ISM_settings_type ISM_settings;
	} gloria;
	byte memory[16];
	byte Height_Class_No;
	byte Height_Classes_LowLim[HEIGHT_CLASS_NO_MAX];
	byte Height_Classes_UppLim[HEIGHT_CLASS_NO_MAX];
	word Instmode_Timeout_s;
	word CAN_Baud_Rate;
	byte Preferred_Modem;
	struct {
		word door_height;					/* Door height in mm					 		*/
		word door_width;					/* Door width in mm								*/
		struct {
			BitField prefer_inB		: 1;	/* Preference of incoming people	 			*/
			BitField prefer_outB   	: 1;	/* Preference of outgoing people	 			*/
			BitField prefer_in2B   	: 1;	/* Pref. of incoming people, 2. subarea			*/
			BitField prefer_out2B	: 1;	/* Pref. of outgoing people, 2. subarea			*/
			BitField baggageB		: 1;	/* Many people carrying baggage or bicycle 		*/
			BitField schoolbusB		: 1;	/* Many children						  	 	*/
			BitField trainB			: 1;	/* 0 = bus, 1 = train							*/
			BitField dist_sen_doorB	: 1;	/* Sensor far from door							*/
			BitField reserved		: 8;	/* Reserved for future use						*/
		} counting;
		struct {
			BitField stepsB			: 1;	/* Steps present at door			 			*/
			BitField steps_downB   	: 1;	/* Steps present at door are downwards 			*/
			BitField barB 		  	: 1;	/* Bar present at door							*/
			BitField reserved		: 13;	/* Reserved for future use						*/
		} door_props;
		word sensor_distance;				/* Distance between sensors in mm				*/
		word reserved_3;					/* Reserved for future use						*/
		word reserved_4;					/* Reserved for future use						*/
		word reserved_5;					/* Reserved for future use						*/
	} ext_funct_area[FUNCTION_AREAS_MAX];
	struct {
		byte sensor_subtype;  				/* IRMA-S-advanced:								*/
											/* type of pyroelectric detector, 'T', 'U', …	*/
											/* IRMA-S-3D:									*/
											/* DIST4 height range, 'A', 'B', …				*/
		byte operation_mode;				/* IRMA-S-3D: operation mode					*/
		word reserved_2;					/* Reserved for future use						*/
		word reserved_3;					/* Reserved for future use						*/
		word reserved_4;					/* Reserved for future use						*/
		word reserved_5;					/* Reserved for future use						*/
		word reserved_6;					/* Reserved for future use						*/
		word reserved_7;					/* Reserved for future use						*/
		word reserved_8;					/* Reserved for future use						*/
	} ext_sen_config[SENSORS_MAX];
} Configuration_Data_type;

typedef struct ASC0_Real_struct ASC0_Real_type;


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/


#define CONFIGURATION_DEFINES_INC
#endif	// end of "#ifndef CONFIGURATION_DEFINES_INC"
