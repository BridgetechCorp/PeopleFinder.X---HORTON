/*==============================================================================================*
 |       Filename: interrupt_defines.h                                                          |
 | Project/Module: A21                                                                          |
 |           Date: 09/09/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description:                                                                              |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef INTERRUPT_DEFINES_INC			/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/

// Timer Interrupts

// T2
// T3	serio_asc0.c		bit timer
// T4	serio_asc0.c		bit timer reload (for T3)
// T5
// T6	sensor_serial.c		Sensor query

#define MAKE_xxIC_VAL(xxie, xxilvl, xxglvl) \
		( (xxglvl)|(xxilvl << 2) | (xxie ? 0x40 : 0) )
#define xxIC_VAL(xxilvl, xxglvl) \
		( (xxglvl)|(xxilvl << 2) )

#define T1INT		0x21		// CAPCOM timer 1
#define T3INT		0x23		// GPT1 timer 3
#define T6INT		0x26		// GPT2 timer 6
#define S0TINT		0x2A		// ASC0 transmit interrupt vector
#define S0RINT		0x2B		// ASC0 receive interrupt vector
#define T8INT		0x3E		// CAPCOM timer 8
#define CANINT		0x40		// CAN1 interrupt vector
#define S0TBINT		0x47		// ASC0 transmit buffer interrupt vector


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/


#define INTERRUPT_DEFINES_INC
#endif	// end of "#ifndef INTERRUPT_DEFINES_INC"
