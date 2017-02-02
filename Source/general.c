/*==============================================================================================*
 |       Filename: general.c                                                                    |
 | Project/Module:                                                                              |
 |           Date: 11/25/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: General functions.                                                           |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <string.h>

#include "general.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class FB=KERNELRAM


/*----- Global Variables -----------------------------------------------------------------------*/


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
dword get_dword(byte *buffer)
{
	dword long_val;

	long_val  = (dword)buffer[3] << 24;
	long_val |= (dword)buffer[2] << 16;
	long_val |= (dword)buffer[1] << 8;
	long_val |= (dword)buffer[0];
	return(long_val);
}	// get_dword


void uppercase(char *str_ptr)
{
	word str_len, str_pos;
	char str_char;

	str_len = strlen(str_ptr);
	for( str_pos = 0; str_pos < str_len; str_pos++ )
	{
		str_char = *str_ptr;
		if( str_char >= 'a' &&  str_char <= 'z' )
			str_char = (char)((byte)str_char - 0x20);
		*str_ptr++ = str_char;
	}
}	// uppercase


void copy_str(char *des_ptr, char *src_ptr, word str_len)
{
	char *des_end_ptr;

	des_end_ptr = strncpy(des_ptr, src_ptr, str_len);

	des_end_ptr += (str_len - 1);
	if(	*des_end_ptr != 0x00 )
		*(++des_end_ptr) = 0x00;
}	// copy_str
