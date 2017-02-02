#ifndef _SYSTEM_H
#define _SYSTEM_H

//#include "p33Exxxx.h"
//#include <p33ep512mc806.h>
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

#include "oscillator.h"
#include "error.h"


// Define special values
#define ANALOG_PIN 1
#define DIGITAL_PIN 0

#define ANALOG_INPUT 1
#define OPEN_DRAIN   1
#define INPUT   1
#define OUTPUT  0

// Define special macros

#define INTERRUPT_DISABLE  {asm volatile ("disi #0x3FFF"); system_disi_level++;  }
#define INTERRUPT_REENABLE {DISICNT = (--system_disi_level > 0) ? 0x3FFF : 0; }
#define INTERRUPT_PROTECT(protected_statement) {INTERRUPT_DISABLE; protected_statement; INTERRUPT_REENABLE;}


// Define operating constants#
#define SYS_POWERUP_DELAY                 100       // mS



#define DEFAULT_TIMER1_FREQUENCY 100 // this is default for BRUSHLESS MOTORS



//#define DEFAULT_TIMER1_PERIOD 371 //  10.KHz Servo Loop runs at 10KHz (interrupt every 1oous)
//#define DEFAULT_TIMER1_PERIOD 360 //  10.3
//#define DEFAULT_TIMER1_PERIOD 340 // 10.9KHz
//#define DEFAULT_TIMER1_PERIOD             320 //  11.6K
//#define DEFAULT_TIMER1_PERIOD             300 // 12.36k
//#define  DEFAULT_TIMER1_PERIOD             200 // 18.62k
//#define DEFAULT_TIMER1_PERIOD             100 //  37.2Khz

#define DEFAULT_TIMER2_PERIOD             3000      // uS (NOT USED RIGHT NOW)

// I want this to be a 10ms timer
#define DEFAULT_TIMER3_FREQUENCY         100   //  interrupt every 10ms (100Hz)


#define DEFAULT_TIMER45_FREQUENCY         375000    // Hz (NOT USED RIGHT NOW)


//#define UART1_RX_BUFFER_SIZE              300
//#define UART1_TX_BUFFER_SIZE              300
//#define UART2_RX_BUFFER_SIZE              300
//#define UART2_TX_BUFFER_SIZE              300
#define CAN1_RX_BUFFER_SIZE          10
#define CAN1_TX_BUFFER_SIZE          10


// priorities 0 to 7
// 0 equals disabled
// 1 is lowest priority
// 7 is highest priority (handeled first
// Define interrupt priorities
//#define I2C_MASTER_INTERRUPT_PRIORITY     6
//#define I2C_SLAVE_INTERRUPT_PRIORITY      6
//#define UART1_RX_INTERRUPT_PRIORITY       6
//#define UART2_RX_INTERRUPT_PRIORITY       6

#define QEI_INTERRUPT_PRIORITY            7

#define CAN1_INTERRUPT_PRIORITY           6

#define TIMER1_INTERRUPT_PRIORITY         5
#define TIMER2_INTERRUPT_PRIORITY         7 // we can never miss the quadrature updates or we could lose counts
#define TIMER3_INTERRUPT_PRIORITY         5
#define TIMER4_INTERRUPT_PRIORITY         5
#define TIMER5_INTERRUPT_PRIORITY         5


//#define UART1_TX_INTERRUPT_PRIORITY       4
//#define UART2_TX_INTERRUPT_PRIORITY       4

#define AD_INTERRUPT_PRIORITY             3
#define CN_INTERRUPT_PRIORITY             3

#define IC5_INTERRUPT_PRIORITY            2
#define IC6_INTERRUPT_PRIORITY            2
#define IC7_INTERRUPT_PRIORITY            2
#define IC8_INTERRUPT_PRIORITY            2

#define CAN_SEND_TIME (150/10)  //  200ms timer 3 runs every 10ms


extern int system_disi_level;

#endif // Sent