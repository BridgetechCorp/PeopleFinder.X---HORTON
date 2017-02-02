/*==============================================================================================*
 |       Filename: serio_asc0.h                                                                 |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of basic I/O functions for communication via RS-232 service   |
 |                 interface or system interface (IBIS, RS-485, J1708) using serial port ASC0.  |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef SERIO_ASC0_INC					/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern Parser_type Parser;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void init_global_var_serio_asc0(void);

void InitP3P4ForASC0(void);

word InitASC0(RS232_Parameter_type *parameter, byte prot_id);

void Check_Send_Data_ASC0(void);

bool ASC0_Service_Interface_Occupied(void);
bool ASC0_System_Interface_Forced(void);


#define SERIO_ASC0_INC
#endif	// end of "#ifndef SERIO_ASC0_INC"
