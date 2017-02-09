#include <xc.h>
#include "oscillator.h"
#include "user.h"
#include "system.h"
//*** timer1_initialize ******************************************************//

#define DEFAULT_TIMER1_PERIOD 1000

boolean flashblink=TRUE;
boolean slowblink=TRUE;
int flashblinktime=0;
int slowblinktime=0;
int timer1_interrupts = 0;
int global_timer=0;

unsigned long wait_delay =0;


unsigned long GetWaitDelay( void )
{
    unsigned long val;
    INTERRUPT_DISABLE;
    val = wait_delay;
    INTERRUPT_REENABLE;
    return val;
}

void SetWaitDelay( unsigned long val )
{
    INTERRUPT_DISABLE;
    wait_delay = val;
    INTERRUPT_REENABLE;
}

int GetTimerInterrupts( void )
{
    int val;
    INTERRUPT_DISABLE;
    val = timer1_interrupts;
    INTERRUPT_REENABLE;
    return val;
}

void SetTimerInterrupts( int val )
{
    INTERRUPT_DISABLE;
    timer1_interrupts = val;
    INTERRUPT_REENABLE;
}


void __attribute__((interrupt,no_auto_psv)) _T1Interrupt(void)
{
    global_timer++;
     if (wait_delay > 0) wait_delay--;
     slowblinktime++;
     if (slowblinktime >= 500)
     {
         slowblinktime = 0;
         slowblink = ! slowblink;
     }


     flashblinktime++;
     if (flashblinktime >= 100)
     {
         flashblinktime = 0;
         flashblink = ! flashblink;
     }
     timer1_interrupts++;
    // LED2 = (LED2)?0:1;
     TESTPIN = (TESTPIN)?0:1;
     DebounceInputs();
     //_T1IF = 0; // clear the interrupt flag?//
     IFS0bits.T1IF = 0; //Clear Timer1 interrupt flag
       // while(1);
}

void timer1_initialize (void)

{ // timer1_initialize
//  T1CON = 0x0000;                             // go to reset state

 // PR1 = (unsigned int) ((((unsigned long) SYS_CLOCK_INSTRUCTION_FREQUENCY * (DEFAULT_TIMER1_PERIOD)) / 1000000) - 1);
 // IPC0bits.T1IP = TIMER1_INTERRUPT_PRIORITY;  // set timer 1 interrupt priority
 // T1CONbits.TON = 1;                          // turn on the timer

  T1CONbits.TON = 0; // Disable Timer
T1CONbits.TCS = 0; // Select internal instruction cycle clock
T1CONbits.TGATE = 0; // Disable Gated Timer mode
T1CONbits.TCKPS = 0b011; // Select 1:8 Prescaler
TMR1 = 0x00; // Clear timer register
PR1 = 78; // Load the period value
IPC0bits.T1IP = 0x01; // Set Timer 1 Interrupt Priority Level
IFS0bits.T1IF = 0; // Clear Timer 1 Interrupt Flag
IEC0bits.T1IE = 1; // Enable Timer1 interrupt
T1CONbits.TON = 1; // Start Timer

 // IEC0bits.T1IE = 1;                          // enable timer 1 interrupts
} // timer1_initialize



