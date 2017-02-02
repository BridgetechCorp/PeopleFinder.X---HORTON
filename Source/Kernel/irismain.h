/*==============================================================================================*
 |       Filename: main.h                                                                       |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Main module of module group Kernel.                   						|
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef MAIN_INC						/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "..\kernel_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern A21_Status_type A21_Status;
extern byte Runlevel;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void No_Job_Kernel(void);


#define MAIN_INC
#endif	// end of "#ifndef MAIN_INC"
