/* 
 * File:   oscillator.h
 * Author: cbeiferman
 *
 * Created on September 14, 2012, 9:04 AM
 */

#ifndef OSCILLATOR_H
#define	OSCILLATOR_H


// we are going to be using a 40 MHz crystal
// COMPILATION related defines
// NOTE: Using the external crystal appears to be causing BROWNOUTS
// need to look into power supply design. (external crystal does cause extra current to be drawn)


//  __dsPIC33EP512MC806__  /* this is for the new board */
        #define CRYSTAL_FREQUENCY 40000000L
        #define FIN CRYSTAL_FREQUENCY

   /*     #define PLL_N1 5L
        #define PLL_M 30L
        #define PLL_N2 2L

        #define ULS_PLLPRE (PLL_N1-2L)
        #define ULS_PLLPOST ((PLL_N2/2L) - 1L)
        #define ULS_PLLDIV  (PLL_M - 2L)

        #define FREF (FIN / PLL_N1)
        #define FVCO (FIN * PLL_M / PLL_N1 )
    	#define FOSC ( (FIN * PLL_M) / (PLL_N1 * PLL_N2))   // 120 MHz, this is the clock source after the PLL

        #if (FREF>8000000L || FREF<800000L)
            #error FREF IS OUT OF RANGE
        #endif

        #if (FVCO < 120000000L || FVCO > 340000000L)
            #error FREF IS OUT OF RANGE
        #endif

        #if (FOSC>120000000L)
            #error FOSC is out of range for 85degrees C
        #endif
*/
        #define FOSC CRYSTAL_FREQUENCY

        #define FCY FOSC
        //#define FCY (FOSC/2)    //60 MIPS

        //#if (FCY != 60000000L)
        //    #error FCY is not 60 MIPS (What happened)
        //#endif

        #define FP FCY // 60 MIPS this is the clock source for the peripherals
        #define SYS_CLOCK_INSTRUCTION_FREQUENCY   (FCY/4)


void InitOscillator( void );


#endif	/* OSCILLATOR_H */

