/*==============================================================================================*
 |       Filename: peripherals.h                                                                |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to handle digital inputs/outputs P2.00...P2.07 and for detection   |
 |                 of TIMEKEEPER NVSRAM.                                                        |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef PERIPHERALS_INC					/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void init_global_var_peripherals(void);

void check_P2(void);
int Test_Logger_Memory_and_RTC(char *);


#define PERIPHERALS_INC
#endif	// end of "#ifndef PERIPHERALS_INC"
