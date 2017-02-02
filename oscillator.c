#include "system.h"
#include "oscillator.h"
// for  __dsPIC33EP512MC806__

void InitOscillator( void )
{
    int i;

    //return; /* TODO: REMOVE THIS LATER test for oscillator*/

  // The bootloader may or may not have run before the servo controller firmware was started.  If it
  // ran, it already configured the oscillator and PLL.  If that is the case, we first need to switch
  // back to the internal oscillator long enough to (re)configure the PLL.
//  __builtin_write_OSCCONH(0x00); // Initiate Clock Switch to Fast RC Oscillator  (NOSC = 0b000)
//  __builtin_write_OSCCONL(0x01);
//  while (OSCCONbits.COSC != 0b000) {} // Wait for Clock switch to occur

  // Now the PLL can be safely configured
 // PLLFBD             = ULS_PLLDIV;
 // CLKDIVbits.PLLPOST = ULS_PLLPOST; // 2 bits
 // CLKDIVbits.PLLPRE  = ULS_PLLPRE; // 5 bits

  // Finally, switch the glock to use the primary oscillator with PLL.
  //__builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary Oscillator with PLL  (NOSC = 0b011)
  //__builtin_write_OSCCONL(0x01);

  __builtin_write_OSCCONH(0b010); // Initiate Clock Switch to Primary Oscillator no PLL  (NOSC = 0b010)
  __builtin_write_OSCCONL(0x01); // intiate switch

 
  while (OSCCONbits.COSC != 0b010) {} // Wait for Clock switch to occur
 // while (OSCCONbits.LOCK != 1) {} // Wait for PLL to lock

   i=4;
  i=5;
}
