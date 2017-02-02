/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__dsPIC33E__)
    	#include <p33Exxxx.h>
    #elif defined(__dsPIC33F__)
    	#include <p33Fxxxx.h>
    #endif
#endif


#include <stdint.h>          /* For uint16_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */

#include "system.h"          /* variables/params used by system.c             */
//#include "kernel_interface.h"


int system_disi_level;
/*
void Save_ILVL_and_Disable_All_INTs(void)
{
    // cmb
}

void Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs(void)
{
	// Save PSW on stack, set CPU priority level ILVL to value assigned to ASC0 interrupts and
	// IEN to 1.
//#pragma asm
//	SCXT PSW,#0B800h
//	NOP
//#pragma endasm
}

void Restore_ILVL(void)
{
    // cmb
}

void disable_t1_interrupt(void)
{

}

void enable_t1_interrupt(void)
{

}

void disable_timer_and_timeout_jobs(void)
{
	timer_and_timeout_jobs_disabledB = TRUE;
	timer_and_timeout_job_disabling_counter++;
}

void enable_timer_and_timeout_jobs(void)
{
	timer_and_timeout_job_disabling_counter--;
	if( timer_and_timeout_job_disabling_counter == 0 )
		timer_and_timeout_jobs_disabledB = FALSE;
}

void Save_ILVL_and_Disable_T1_and_lower_priority_INTs(void)
{
	// Save PSW on stack, set CPU priority level ILVL to value assigned to T1 interrupt and
	// IEN to 1.
    // CMB
//#pragma asm
//	SCXT PSW,#2800h
//	NOP
//#pragma endasm
}

*/

/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.c.                                          */
/*                                                                            */
/******************************************************************************/

/* Refer to the device Family Reference Manual Oscillator section for
information about available oscillator configurations.  Typically
this would involve configuring the oscillator tuning register or clock
switching useing the compiler's __builtin_write_OSCCON functions.
Refer to the C Compiler for PIC24 MCUs and dsPIC DSCs User Guide in the
compiler installation directory /doc folder for documentation on the
__builtin functions.*/

/* TODO Add clock switching code if appropriate.  An example stub is below.   */
void ConfigureOscillator(void)
{

#if 0
        /* Disable Watch Dog Timer */
        RCONbits.SWDTEN = 0;

        /* When clock switch occurs switch to Primary Osc (HS, XT, EC) */
        __builtin_write_OSCCONH(0x02);  /* Set OSCCONH for clock switch */
        __builtin_write_OSCCONL(0x01);  /* Start clock switching */
        while(OSCCONbits.COSC != 0b011);

        /* Wait for Clock switch to occur */
        /* Wait for PLL to lock, only if PLL is needed */
        /* while(OSCCONbits.LOCK != 1); */
#endif
}

