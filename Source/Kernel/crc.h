/*==============================================================================================*
 |       Filename: crc.h                                                                        |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Calculation of CRC-32 checksums according to ANSI X3.66. 					|
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CRC_INC							/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
unsigned long Get_CRC(unsigned char _huge *start, unsigned char _huge *end);


#define CRC_INC
#endif	// end of "#ifndef CRC_INC"
