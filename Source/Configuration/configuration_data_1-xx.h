/*==============================================================================================*
 |       Filename: configuration_data_1-xx.h                                                    |
 | Project/Module: A21                                                                          |
 |           Date: 09/08/2009 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Firmware parameters stored in Flash EPROM sector 1.                          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x0040C0 ---------- IRMA Communication Parameter ---------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  2 */	WORD address;						/* IRMA device id								*/
/*$02  6 */	char tag[6];						/* IRMA device tag (4 characters)				*/
/*$08  2 */	WORD status_interval;				/* Time between two status reports (in seconds)	*/
} Configuration_IRMA_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004100 ---------- Function Areas -----------------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  2 */	WORD address;						/* Address of function area						*/
			struct {
/*$02  4 */		struct {
				/* 0 */	BitField type					: 8;
				/* 1 */	BitField height					: 8;
				/* 2 */	BitField stepsB					: 1;
						BitField position_externB		: 1;
						BitField position_exactB		: 1;
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
						BitField 						: 0;	/* Alignment					*/
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
/*$12  2 */	WORD cycle_time;					/* Time between two cycles in µs				*/
/*$14  1 */	BYTE no_of_sensors;					/* Number of sensors in this function area		*/
/*$15  1 */	BYTE dummy2;
/*$16  2 */	struct {							/* IRMA function bits							*/
				BitField countingB		: 1;	/* !!! Important !!!							*/
				BitField observationB	: 1;	/* Bits 0 - 6 are usable only because of		*/
				BitField controlB		: 1;	/* integration into CAN IRMA InitByte1.			*/
			} function;
} Function_Area_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004300 ---------- Sensor Configuration -----------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	BYTE type_no;						/* Index of Sensor_Type_ROM 					*/
/*$01  1 */	BYTE dummy;
/*$02  4 */	DWORD address;						/* Address of sensor							*/
/*$06  2 */	struct {
				BitField turnedB		: 1;	/* Sensor is turned by 180 deg.					*/
				BitField som_specifiedB	: 1;	/* Sensor operation mode specified				*/
												/* FALSE: compatibility mode, DIST4:			*/
												/*        slave  if turnedB == FALSE,			*/
												/*        master if turnedB == TRUE				*/
												/* TRUE:  sensor op. mode given by sen_op_mode	*/
				BitField sen_op_mode	: 3;	/* sensor op. mode, DIST4: 0 = master, 1 = sla.	*/
			} environment;
/*$08  6 */	struct {
				BYTE function_area;				/* Function area in which sensor is part of		*/
				BYTE number;					/* Geometrical order of sensor in function area	*/
			} area[3];
} Sensor_Config_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x004500 ---------- Cycle Table --------------------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	BYTE element;				/* Address of element, e.g. 0x80, 0x20, ...				*/
/*$01  1 */	BYTE sensor;				/* Number of sensor which element belongs to			*/
/*$02  3 */	BYTE position[3];			/* Position in geometrical order in each function area	*/
/*$05  1 */	BYTE dummy;
/*$06  2 */	struct {							/* Environment of element						*/
				BitField no_neighbour_leftB		: 1;	/* Wall on the left side of the element	*/
				BitField no_neighbour_rightB	: 1;	/* Wall on the right side of the element*/
				BitField no_neighbour_line_leftB: 1;	/* Wall on the left side of the element	*/
				BitField no_neighbour_line_rightB:1;	/* Wall on the right side of the element*/
				BitField bar_leftB				: 1;	/* Dividing bar on the left side of el.	*/
				BitField bar_rightB				: 1;	/* Dividing bar on the right side of el.*/
				BitField bar_line_leftB			: 1;	/* Dividing bar on the left side of el.	*/
				BitField bar_line_rightB		: 1;	/* Dividing bar on the right side of el.*/
				BitField outer_elementB			: 1;	/* Outer element						*/
				BitField 						: 0;	/* alignment */
			} environment;
} Cycle_Table_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x005000 ---------- Sensor Types -------------------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00 16 */	char type[16];
/*$10 16 */	BYTE element_type[16];
} Sensor_Type_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x005800 ---------- On Board Computer Configuration ------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  4 */	unsigned long vehicle_id;
} OBC_Data_ROM_type;

/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: 0x005820 ---------- Logger Configuration -----------------*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  2 */	struct {
				unsigned char minutes;
				unsigned char seconds;
			} interval;
} Logger_Config_ROM_type;
