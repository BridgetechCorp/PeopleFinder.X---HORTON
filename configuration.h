/*==============================================================================================*
 |       Filename: configuration.h                                                              |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to evaluate device immanent configuration data and firmware        |
 |                 parameters stored in Flash EPROM sector 1.                                   |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CONFIGURATION_INC				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern Configuration_System_type	ConfigSystem;
extern Configuration_Data_type		ConfigData;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void setup_interrupt_controls(void);
void buscon_setup(void);
void IO_Port_2_Setup(void);


#define CONFIGURATION_INC
#endif	// end of "#ifndef CONFIGURATION_INC"
