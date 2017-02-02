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
#include "user.h"            /* variables/params used by user.c               */
#include "ecan.h"
#include "timer.h"

#define BUTTONCOUNT 50  // 50 millisecond debounce time

boolean Input1Pressed = false;
boolean Input2Pressed = false;
boolean Input3Pressed = false;
boolean Input4Pressed = false;

unsigned int Input1Count = 0;
unsigned int Input2Count = 0;
unsigned int Input3Count = 0;
unsigned int Input4Count = 0;

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/
//#include <p33Exxxx.h>
void CountUp( unsigned int *val)
{
    if (*val < BUTTONCOUNT )
        *val = *val + 1;
}

void CountDown( unsigned int *val)
{
    if (*val > 0 )
        *val = *val - 1;
}

void Decide( unsigned int val, boolean *var)
{
    if (val >= BUTTONCOUNT)
        *var = TRUE;
    else if (val == 0)
        *var = FALSE;
    // else Dont change state unless we hit 0 or button count
}

void DebounceInputs( void )
{
   (IN1)? CountUp(&Input1Count):CountDown(&Input1Count);
   (IN2)? CountUp(&Input2Count):CountDown(&Input2Count);
   (IN3)? CountUp(&Input3Count):CountDown(&Input3Count);
   (IN4)? CountUp(&Input4Count):CountDown(&Input4Count);

    Decide(Input1Count, &Input1Pressed);
    Decide(Input2Count, &Input2Pressed);
    Decide(Input3Count, &Input3Pressed);
    Decide(Input4Count, &Input4Pressed);
}
// forward declarations
void InitIOPorts( void );

/***********************************************************************************
* Function: lockIO
*
* Preconditions: None.
*
* Overview: This executes the necessary process to set the IOLOCK bit to lock
* I/O mapping from being modified.
*
* Input: None.
*
* Output: None.
*
*****************************************************************************/
void lockIO()
{
        asm volatile ("mov #OSCCON,w1 \n"
                "mov #0x46, w2 \n"
                "mov #0x57, w3 \n"
                "mov.b w2,[w1] \n"
                "mov.b w3,[w1] \n"
                "bset OSCCON, #6");
}


/*****************************************************************************
* Function: unlockIO
*
* Preconditions: None.
*
* Overview: This executes the necessary process to clear the IOLOCK bit to
* allow I/O mapping to be modified.
*
* Input: None.
*
* Output: None.
*
*****************************************************************************/
void unlockIO()
{
    asm volatile ("mov #OSCCON,w1 \n"
                "mov #0x46, w2 \n"
                "mov #0x57, w3 \n"
                "mov.b w2,[w1] \n"
                "mov.b w3,[w1] \n"
                "bclr OSCCON, #6");
}


void MapInputPeripherals( void )
{
  /* the following table are all the peripheral pin mappings in the system that are needed.
  * ===============================================================================
  *  INPUT MAPPINGS
  * =============================================================================== */
  unlockIO();
  // TODO: add INT1 interrupt handler for rotary encoder Z? not sure what well do, but couldbe used for calibration to get encoder counts per rev
//  RPINR0bits.INT1R  = 64; // map INT1 to RP64
  RPINR26bits.C1RXR = 65; // map CAN1 Receive to RP65
  // TODO: setup timer 6/7 for up clock, and timer 8/9 for down clock, then write code to extract position from these 2 32 bit counters
 
  //UART1 Receive U1RX RPINR62 U1RXR<6:0>
  RPINR18bits.U1RXR = 62; // map UART 1 Receive to RPI62
  // TODO: Setup new PWM output module for BLDC, read errata, center aligned mode has problems.

  /* ===============================================================================
  *  OUTPUT MAPPINGS
  * =============================================================================== */
  RPOR0bits.RP64R = 14; //* map RP64 to CAN1_TX
//U1TX 000001 RPn tied to UART1 Transmit
  RPOR7bits.RP97R = 1; // uart1 tx (RE5)  //U1TX 000001 RPn tied to UART1 Transmit
  lockIO();
}


void InitIOPorts( void )
{
    // setup the i/o port direction pins
    OUT1_TRIS = 0;
    OUT2_TRIS = 0;
    IN1_TRIS  = 1;
    IN2_TRIS  = 1;
    IN3_TRIS  = 1;
    IN4_TRIS  = 1;
    LED1_TRIS = 0;
    LED2_TRIS = 0;
    TESTPIN_TRIS = 0;

    MODEM_TX_TRIS = 0;
    MODEM_RX_TRIS = 1;
    MODEM_UART_EN_TRIS = 0;
    INVALID_TRIS       = 1; // input
    

    TESTPIN = 0;
    _TRISD0 = 0; // CAN_TX
    _TRISD1 = 1; // can_RX

    // setup the default output states
    OUT1 = 0;
    OUT2 = 0;
    LED1 = 1;
    LED2 = 0;
    
    // setup pullup bits
    //CNPUx and the CNPDx
    CNPUDbits.CNPUD11 = 1;
    
    
    
    MODEM_UART_EN = 0;
    

    // all analog ports are analog by default, set it to digital
    ANSELB = 0x0000;



}


/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    /* TODO Initialize User Ports/Peripherals/Project here */
    
    /* Setup analog functionality and port direction */
     InitIOPorts();

     MapInputPeripherals();
    /* Initialize peripherals */
     initECAN();

     /* start the 1 ms interrupt timer */
     timer1_initialize ();
}

