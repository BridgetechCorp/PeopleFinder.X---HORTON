/*==============================================================================================*
 |       Filename: Gateway.c                                                                    |
 | Project/Module: GATEWAY                       	                                            |
 |           Date: 10/04/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Device immanent configuration data for device class Gateway.                 |
 |                 Stored in Flash EPROM sector 0.                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#include ".\..\..\..\kernel_defines.h"
#include ".\..\..\configuration_data.h"


/*==============================================================================================*/
/*=== W r i t e   d e f a u l t s   t o   c o n f i g u r a t i o n   d a t a   i n   R O M  ===*/
/*==============================================================================================*/
/* Default data are to be substituted by device immanent configuration data which are written	*/
/* to Flash EPROM sector 0 by procduction tool A21_BootFirst.									*/	

/*----- System Configuration Data --------------------------------------------------------------*/
const Configuration_Data_Kernel_type Configuration_Data_Kernel _at(ADDR_KERNEL_CONFIG) = {
				1,							/* Version of struct definition						*/
				2,							/* Revision of struct definition					*/
				"Gateway",					/* Product name of analyzer							*/
				28,							/* Year												*/
				0,							/* Dummy											*/
				0,							/* Serial number of analyzer						*/
				"C167CR",					/* Type of microcontroller (C167CR, ST10R167)		*/
				{0xFFFF, 0xFFFF, 0xFFFF,	/* Override EDE BUSCON settings,					*/
				 0xFFFF, 0xFFFF},			/*		0xFFFF -> no changes						*/
				{0xFFFF, 0xFFFF,			/* Override EDE ADDRSEL settings,					*/
				 0xFFFF, 0xFFFF},			/*		0xFFFF -> no changes						*/
				16000000,					/* Processor Clock in Hz							*/
				4000000,					/* Oscillator Clock in Hz							*/
				{ 0xFF, 0xFF,				/* Type of flash EPROM								*/
				  "AM29F200BB" },
				128,						/* Size of SRAM in kB								*/
				"AutoDetect",				/* Type of nonvolatile RAM							*/
				0,							/* Size of NVRAM in kB								*/
//				"M48T128Y",					/* Type of nonvolatile RAM							*/
//				128,						/* Size of NVRAM in kB								*/
				"CAN",						/* Type of sensor interface (SSC, CAN)				*/
				"ASC0",						/* Type of system interface (ASC0-RS232, ...)		*/
			};
