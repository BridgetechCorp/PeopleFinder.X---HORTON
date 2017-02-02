/*==============================================================================================*
 |       Filename: peripherals.c                                                                |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to handle digital inputs/outputs P2.00...P2.07 and for detection   |
 |                 of Logger memory and RTC.                                                    |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>
#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#include "peripherals.h"
#if !defined(NO_LOGGER) && defined(A21CL)
	#include "serio_ssc.h"
#endif


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class CO=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
Channel_type Channel[A21_Digital_IO_Cnt];
bool ChannelState_ChangedB[A21_Digital_IO_Cnt];
const void (*chatter[A21_Digital_IO_Cnt])(void);


/*----- Function Prototypes --------------------------------------------------------------------*/
bool getport2(byte chan_idx);

void chatter_0(void);
void chatter_1(void);
void chatter_2(void);
void chatter_3(void);
void chatter_4(void);
void chatter_5(void);
void chatter_6(void);
void chatter_7(void);

#ifndef NO_LOGGER
word Detect_NVRAM_Size(void);
	#ifdef A21CL
int Test_serial_RTC(void);
	#endif
#endif


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
// Initialize global variables not properly initialized by function "clear_kernel_RAM".
void init_global_var_peripherals(void)
{
	byte chan_idx;

	chatter[0] = chatter_0;
	chatter[1] = chatter_1;
	chatter[2] = chatter_2;
	chatter[3] = chatter_3;
	chatter[4] = chatter_4;
	chatter[5] = chatter_5;
	chatter[6] = chatter_6;
	chatter[7] = chatter_7;

	for( chan_idx = 0; chan_idx < A21_Digital_IO_Cnt; chan_idx++ )
		// Idle state of P2.00...07 = 1 (digital input open).
		Channel[chan_idx].stateB = getport2(chan_idx);
}


// Return value == 0:        According P2 bit == 0.
// Return value == 1:        According P2 bit == 1.
// Return value == EOF (-1): Function not applicable.
char Get_Channel(byte chan_no)
{
	if( chan_no == 0 || chan_no > A21_Digital_IO_Cnt || !Channel[chan_no - 1].inputB )
		return EOF;

	return(Channel[chan_no - 1].stateB);
}


void chatter_0(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 0);
	if(P2_Bit != Channel[0].stateB)
	{
		Channel[0].stateB = P2_Bit;
		ChannelState_ChangedB[0] = TRUE;
	}
	Channel[0].ignoreB = FALSE;
}

void chatter_1(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 1);
	if(P2_Bit != Channel[1].stateB)
	{
		Channel[1].stateB = P2_Bit;
		ChannelState_ChangedB[1] = TRUE;
	}
	Channel[1].ignoreB = FALSE;
}

void chatter_2(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 2);
	if(P2_Bit != Channel[2].stateB)
	{
		Channel[2].stateB = P2_Bit;
		ChannelState_ChangedB[2] = TRUE;
	}
	Channel[2].ignoreB = FALSE;
}

void chatter_3(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 3);
	if(P2_Bit != Channel[3].stateB)
	{
		Channel[3].stateB = P2_Bit;
		ChannelState_ChangedB[3] = TRUE;
	}
	Channel[3].ignoreB = FALSE;
}

void chatter_4(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 4);
	if(P2_Bit != Channel[4].stateB)
	{
		Channel[4].stateB = P2_Bit;
		ChannelState_ChangedB[4] = TRUE;
	}
	Channel[4].ignoreB = FALSE;
}

void chatter_5(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 5);
	if(P2_Bit != Channel[5].stateB)
	{
		Channel[5].stateB = P2_Bit;
		ChannelState_ChangedB[5] = TRUE;
	}
	Channel[5].ignoreB = FALSE;
}

void chatter_6(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 6);
	if(P2_Bit != Channel[6].stateB)
	{
		Channel[6].stateB = P2_Bit;
		ChannelState_ChangedB[6] = TRUE;
	}
	Channel[6].ignoreB = FALSE;
}

void chatter_7(void)
{
	bool P2_Bit;

	P2_Bit = _getbit(P2, 7);
	if(P2_Bit != Channel[7].stateB)
	{
		Channel[7].stateB = P2_Bit;
		ChannelState_ChangedB[7] = TRUE;
	}
	Channel[7].ignoreB = FALSE;
}


void check_P2(void)
{
	byte chan_idx;
	word chatter_time;

	for( chan_idx = 0; chan_idx < A21_Digital_IO_Cnt; chan_idx++ )
	{
		if( !Channel[chan_idx].ignoreB && getport2(chan_idx) != Channel[chan_idx].stateB )
		{
			if( Channel[chan_idx].stateB )
				chatter_time = Channel[chan_idx].duration.chatter.falling;
			else
				chatter_time = Channel[chan_idx].duration.chatter.rising;
			// Channel[chan_idx].ignoreB = TRUE indicates running chatter detection.
			Channel[chan_idx].ignoreB = TRUE;
			add_timeout_job(chatter_time, chatter[chan_idx]);
		}
	}
}	// check_P2


bool getport2(byte chan_idx)
{
	bool stateB = FALSE;

	switch(chan_idx)
	{
		case 0 :	stateB = _getbit(P2, 0); break;
		case 1 :	stateB = _getbit(P2, 1); break;
		case 2 :	stateB = _getbit(P2, 2); break;
		case 3 :	stateB = _getbit(P2, 3); break;
		case 4 :	stateB = _getbit(P2, 4); break;
		case 5 :	stateB = _getbit(P2, 5); break;
		case 6 :	stateB = _getbit(P2, 6); break;
		case 7 :	stateB = _getbit(P2, 7); break;
	}
	return(stateB);
}	// getport2


#ifndef NO_LOGGER
int Test_Logger_Memory_and_RTC(char *rtc_name)
{
	word Detected_NVRAM_Size;

	ConfigSystem.RTC_id = 0;
	A21_Status.Optional_Hardware.RTC_ErrorB = FALSE;
	A21_Status.Optional_Hardware.NVRAM_ErrorB = FALSE;
	if( strlen(rtc_name) > 0 )
	{
		Detected_NVRAM_Size = Detect_NVRAM_Size();
		if( strcmp(rtc_name, "AutoDetect") == 0 )
		{
			ConfigSystem.NVRAM_size = Detected_NVRAM_Size;

	#ifdef A21CL
			if( ConfigSystem.NVRAM_size == 512 )
			{
				ConfigSystem.RTC_id = 5;
				A21_Status.Optional_Hardware.RTC_ErrorB = !Test_serial_RTC();
			}
	#else
			if( ConfigSystem.NVRAM_size == 32 )
				ConfigSystem.RTC_id = 1;
			if( ConfigSystem.NVRAM_size == 128 )
				ConfigSystem.RTC_id = 2;
	#endif
		}
		else
		{
			if( Detected_NVRAM_Size == ConfigSystem.NVRAM_size )
			{
	#ifdef A21CL
				if( strcmp(rtc_name, "AT45DB041B+1305") == 0 )
				{
					ConfigSystem.RTC_id = 5;
					A21_Status.Optional_Hardware.RTC_ErrorB = !Test_serial_RTC();
				}
	#else
				if( strcmp(rtc_name, "DS1644") == 0 )
					ConfigSystem.RTC_id = 1;
				if( strcmp(rtc_name, "M48T128Y") == 0 )
					ConfigSystem.RTC_id = 2;
				if( strcmp(rtc_name, "M48T35Y") == 0 )
					ConfigSystem.RTC_id = 3;
				if( strcmp(rtc_name, "DS1646") == 0 )
					ConfigSystem.RTC_id = 4;
	#endif
			}
			else
			{
				ConfigSystem.NVRAM_size = 0;
				A21_Status.Optional_Hardware.RTC_ErrorB = TRUE;
				A21_Status.Optional_Hardware.NVRAM_ErrorB = TRUE;
			}
		}
	}
	else
		ConfigSystem.NVRAM_size = 0;

	if( ConfigSystem.RTC_id == 0 )
		A21_Status.Optional_Hardware.RTC_ErrorB = TRUE;
	if( ConfigSystem.NVRAM_size == 0 )
		A21_Status.Optional_Hardware.NVRAM_ErrorB = TRUE;

	return(ConfigSystem.RTC_id != 0);
}


	#ifdef A21CL
int Test_DataFlash_512k(void)
{
	Init_SSC();
	return( (AT45DB041B_Status_Register() & 0x3C) == 0x1C );
}


int Test_serial_RTC(void)
{
	byte lb_write[5], lb_read[5];

	DS1305_Register_Write(CONTROL_REGISTER_WRITE, 0x00);

	strcpy((char *)lb_write, "IRMA");
	DS1305_RAM_Write(92, 4, lb_write);

	DS1305_RAM_Read(92, 4, lb_read);
	lb_read[4] = '\0';

	return(strcmp((char *)lb_read, "IRMA") == 0);
}


	#else
// NVRAMs DS1644 and M48T35Y have a size of 32 kbyte, NVRAMs DS1646 and M48T128Y
// have a size of 128 kbyte. In all of these devices eight most upper bytes 
// are used as registers of integrated real time clock. In contrast to DS1646
// and M48T128Y address inputs A15 and A16 are not present at devices DS1644
// and M48T35Y. Address signals A15 and A16 of microcontroller are not evaluated 
// in these devices. As a result 32 kbyte NVRAMs are present in microcontroller
// address space 4 times: 0xC0000-0xC7FFF, 0xC8000-0xCFFFF, 0xD0000-0xD7FFF and
// 0xD8000-0xDFFFF.
int Test_NVRAM_128k(void)
{
	const char *NVRAM_test_32 = (char *)ADDR_NVRAM_TEST_32;
	char *NVRAM_test_128 = (char *)ADDR_NVRAM_TEST_128;

	strcpy(NVRAM_test_128, "iRMa-01");
	if(strcmp(NVRAM_test_128, "iRMa-01") != 0)
		return(FALSE);
	strcpy(NVRAM_test_128, "iRMa-02");
	if(strcmp(NVRAM_test_128, "iRMa-02") != 0)
		return(FALSE);
	if(strcmp(NVRAM_test_32, "iRMa-02") == 0)
		return(FALSE);
	return(TRUE);
}


int Test_NVRAM_32k(void)
{
	const char *NVRAM_test_32 = (char *)ADDR_NVRAM_TEST_32;
	char *NVRAM_test_128 = (char *)ADDR_NVRAM_TEST_128;

	strcpy(NVRAM_test_128, "iRMa-01");
	if(strcmp(NVRAM_test_128, "iRMa-01") != 0)
		return(FALSE);
	strcpy(NVRAM_test_128, "iRMa-02");
	if(strcmp(NVRAM_test_32, "iRMa-02") != 0)
		return(FALSE);
	return(TRUE);
}
	#endif	// end of "#ifdef A21CL"


word Detect_NVRAM_Size(void)
{
	#ifdef A21CL
	if(Test_DataFlash_512k())
		return(512);
	#else
	if(Test_NVRAM_32k())
		return(32);

	if(Test_NVRAM_128k())
		return(128);
	#endif
	return(0);
}


bool RTC_Error(void)
{
	return(A21_Status.Optional_Hardware.RTC_ErrorB);
}

bool Logger_Memory_Error(void)
{
	return(A21_Status.Optional_Hardware.NVRAM_ErrorB);
}


void Set_Logger_Data_Status(bool new_status)
{
	A21_Status.Logger.DataB = new_status;
}


bool Logger_Empty(void)
{
	return(!A21_Status.Logger.DataB);
}


void Set_Logger_Overflow_Status(bool new_status)
{
	A21_Status.Logger.OverflowB = new_status;
}


bool Logger_Overflow(void)
{
	return(A21_Status.Logger.OverflowB);
}
#endif // end of "#ifndef NO_LOGGER"
