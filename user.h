#ifndef USER_H
#define USER_H

#include "types.h"
/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

/* TODO User level functions prototypes (i.e. InitApp) go here */

void InitApp(void);         /* I/O and Peripheral Initialization */


// define the TRIS bits
#define OUT1_TRIS        (TRISBbits.TRISB8)
#define OUT2_TRIS        (TRISBbits.TRISB9)
#define IN1_TRIS         (TRISBbits.TRISB4)
#define IN2_TRIS         (TRISBbits.TRISB5)
#define IN3_TRIS         (TRISBbits.TRISB6)
#define IN4_TRIS         (TRISBbits.TRISB7)
#define LED1_TRIS        (TRISEbits.TRISE1)
#define LED2_TRIS        (TRISEbits.TRISE0)

#define TESTPIN_TRIS     (TRISBbits.TRISB3)

#define MODEM_TX_TRIS      (TRISFbits.TRISF1)
#define MODEM_RX_TRIS      (TRISCbits.TRISC14)
#define MODEM_UART_EN_TRIS (TRISFbits.TRISF4)
#define INVALID_TRIS       (TRISFbits.TRISF0)


//define the OUTPUT bits
#define OUT1            (LATBbits.LATB8)
#define OUT2            (LATBbits.LATB9)
#define LED1            (LATEbits.LATE1)
#define LED2            (LATEbits.LATE0)
#define TESTPIN         (LATBbits.LATB3)

#define MODEM_UART_EN   (LATFbits.LATF4)




// define the INPUT bits
#define IN1           (PORTBbits.RB4)
#define IN2           (PORTBbits.RB5)
#define IN3           (PORTBbits.RB6)
#define IN4           (PORTBbits.RB7)

// I need pullup on this pin
#define UARTMODESW    (PORTDbits.RD11)

#define INVALID       (PORTFbits.RF0) //indicates if the uart rx has valid levels

extern void DebounceInputs( void );

extern boolean Input1Pressed;
extern boolean Input2Pressed;
extern boolean Input3Pressed;
extern boolean Input4Pressed;
#endif