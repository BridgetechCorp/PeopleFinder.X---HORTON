/*==============================================================================================*
 |       Filename: configuration_data_1-04.h                                                    |
 | Project/Module: A21                                                                          |
 |           Date: 10/30/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                 Extensions of structure version 1.4.                                         |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004000 ---------- S T R U C T   V E R S I O N   1.04 ---*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	BYTE struct_version;				/* Version of struct definition					*/
/*$01  1 */	BYTE struct_revision;				/* Revision of struct definition				*/
/*$02 16 */	char configuration_name[16];		/* Name of configuration						*/
			struct {							/* Parameter for asynchronous serial interface	*/
/*$12  4 */		DWORD baudrate;					/*		Baudrate in bps							*/
/*$16  1 */		BYTE data_bits;					/*		7, 8, 9									*/
/*$17  1 */		char parity;					/*		'N' = None, 'O' = Odd, 'E' = Even		*/
/*$18  1 */		BYTE stop_bits;					/*		1, 2									*/
/*$19  1 */		BYTE flow_control;				/*		0 = no flow control, 1 = RTS/CTS		*/
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
				WORD period;					/* 		Period, unit: 탎						*/
				WORD pulse_width;				/*		Pulse width, unit: 탎					*/
				WORD offset;					/*		Offset, unit: 탎						*/
			} PWM[4];
/*$3C  1 */	BYTE function_areas;				/* Number of function areas						*/
/*$3D  1 */	BYTE sensors;						/* Number of sensors							*/
/*$3E  2 */	WORD elements;						/* Total number of elements						*/
/*$40  1 */	BYTE sensor_types;					/* Number of different sensor types				*/
/*$41  1 */	                                	/* word alignment								*/
} Configuration_Data_ROM_104_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004100 ---------- Function Areas -----------------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  2 */	WORD address;						/* Address of function area						*/
			struct {
/*$02  4 */		struct {
				/* 0 */	BitField type					: 8;
				/* 1 */	BitField height					: 8;	// For compatibility only
				/* 2 */	BitField stepsB					: 1;	// For compatibility only
						BitField position_externB		: 1;
						BitField position_exactB		: 1;
						BitField simulationB			: 1;
						BitField unused_3B				: 1;	// IRMA Opera 5.95 and 5.96: barB
				/* 3 */ BitField 						: 0;
				} info;
/*$06  8 */		struct {
				/* 0 */	BitField channel_no				: 8;	/* Port number of signal		*/
				/* 1 */	BitField door_switchB			: 1;	/* Type of signal				*/
						BitField closing_signalB		: 1;	/* Type of signal				*/
						BitField fifty_percentB			: 1;	/* Type of signal				*/
						BitField motion_signalB			: 1;	/* Type of signal				*/
						BitField invert_signalB			: 1;	/* Invert logic of signal		*/
						BitField left_leafB				: 1;	/* Signal refers to xxx leaf	*/
						BitField right_leafB			: 1;	/* 		single: left + right	*/
						BitField door_delayB     		: 1;	/* if set, chatter_10 in 1/10s	*/
				/* 2 */	BitField chatter_time_01		: 8;	/* Unit: ms						*/
				/* 3 */	BitField chatter_time_10		: 8;	/* Unit: ms						*/
				} input[2];
				struct {
/*$0E  2 */			BitField channel_no			: 4;
					BitField invert_signalB		: 1;
					BitField					: 0;	/* Alignment							*/
/*$10  2 */			BitField signal_width		: 16;	/* Unit: ms								*/
				} output;
			} door;
/*$12  2 */	WORD cycle_time;					/* Time between two cycles in 탎				*/
/*$14  1 */	BYTE no_of_sensors;					/* Number of sensors in this function area		*/
/*$15  1 */	BYTE dummy2;
/*$16  2 */	struct {							/* IRMA function bits							*/
				BitField countingB		: 1;	/* !!! Important !!!							*/
				BitField observationB	: 1;	/* Bits 0 - 6 are usable only because of		*/
				BitField controlB		: 1;	/* integration into CAN IRMA InitByte1.			*/
				BitField heightclassB	: 1;	/* always == 0 for structure version <= 1.6		*/
				BitField scalableB		: 1;	/* always == 0 for structure version <= 1.7		*/
			} function;
} Function_Area_ROM_104_type;
