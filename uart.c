#include <xc.h>
#include "types.h"
#include "buffer.h"


#define UART_RX_SIZE 60

buffer_t uart_rx_buffer;
char uartRxBuffer[UART_RX_SIZE];

/******************************************************************************
 * Function:  void __attribute__ ((interrupt, no_auto_psv)) _U1RXInterrupt(void)
 *
 * PreCondition: UART Module must be Initialized with receive interrupt enabled.
 *
 * Input:        None
 *                  
 * Output:       None
 *
 * Side Effects: None
 *
 * Overview:     UART receive interrupt service routine called whenever a byte
 *               of data received in UART Rx buffer.
 *****************************************************************************/
void __attribute__ ( (interrupt, no_auto_psv) ) _U1RXInterrupt( void )
{
   // LATA = U1RXREG;
    char c = U1RXREG;
    
    if (!buffer_put(&uart_rx_buffer, (char*)&c))
    {
        c=4;
     // UartRxBufferOverFlowOccured = TRUE;
     // AddErrorCode(UART_RX_BUFFER_OVERFLOW);
    }
    
    IFS0bits.U1RXIF = 0;
}

/******************************************************************************
 * Function:   void __attribute__ ((interrupt, no_auto_psv)) _U1TXInterrupt(void)
 *
 * PreCondition: UART Module must be Initialized with transmit interrupt enabled.
 *
 * Input:        None
 *                  
 * Output:       None
 *
 * Side Effects: None
 *
 * Overview:     UART transmit interrupt service routine called whenever a data
 *               is sent from UART Tx buffer
 *****************************************************************************/
void __attribute__ ( (interrupt, no_auto_psv) ) _U1TXInterrupt( void )
{
    IFS0bits.U1TXIF = 0;
}



/******************************************************************************
 * Function:        void InitUART1()
 *
 * PreCondition:    None
 *
 * Input:           None
 *                  
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function configures UART module
 *****************************************************************************/
void InitUART1( void )
{    
    buffer_initialize (&uart_rx_buffer,&uartRxBuffer[0], sizeof (char), UART_RX_SIZE );
          
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets
    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only
    // configure U1MODE
    U1MODEbits.UARTEN = 0;  // Bit15 TX, RX DISABLED, ENABLE at end of func

    //U1MODEbits.notimplemented;// Bit14
    U1MODEbits.USIDL = 0;   // Bit13 Continue in Idle
    U1MODEbits.IREN = 0;    // Bit12 No IR translation
    U1MODEbits.RTSMD = 0;   // Bit11 Simplex Mode

    //U1MODEbits.notimplemented;// Bit10
    U1MODEbits.UEN = 0;     // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0;    // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0;  // Bit6 No Loop Back
    U1MODEbits.ABAUD = 0;   // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.BRGH = 0;    // Bit3 16 clocks per bit period
    U1MODEbits.PDSEL = 0;   // Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0;   // Bit0 One Stop Bit

    // Load a value into Baud Rate Generator.  Example is for 9600.
    // See section 19.3.1 of datasheet.
    //  U1BRG = (Fcy/(16*BaudRate))-1
    //  U1BRG = (37M/(16*9600))-1
    //  U1BRG = 240
    //U1BRG = 10;  // 8.8us          // 60Mhz osc, 9600 Baud
    //U1BRG = 96; // 80us
    U1BRG = 125; //
    // Load all values in for U1STA SFR
    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0;   //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15

    //U1STAbits.notimplemented = 0;//Bit12
    U1STAbits.UTXBRK = 0;   //Bit11 Disabled
    U1STAbits.UTXEN = 0;    //Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0;    //Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0;     //Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0;  //Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0;    //Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0;    //Bit4 *Read Only Bit*
    U1STAbits.PERR = 0;     //Bit3 *Read Only Bit*
    U1STAbits.FERR = 0;     //Bit2 *Read Only Bit*
    U1STAbits.OERR = 0;     //Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0;    //Bit0 *Read Only Bit*
    IPC7 = 0x4400;          // Mid Range Interrupt Priority level, no urgent reason
    IFS0bits.U1TXIF = 0;    // Clear the Transmit Interrupt Flag
    IEC0bits.U1TXIE = 0;    // disable Transmit Interrupts
    IFS0bits.U1RXIF = 0;    // Clear the Recieve Interrupt Flag
    IEC0bits.U1RXIE = 1;    // Enable Recieve Interrupts
    //RPOR1bits.RP37R = 1;    //RP37/RB5 as U1TX
    //RPINR18bits.U1RXR = 38; //RP38/RB6 as U1RX
    U1MODEbits.UARTEN = 1;  // And turn the peripheral on
    U1STAbits.UTXEN = 1;
}



void putch( char c )
{
    while (U1STAbits.UTXBF);
    U1TXREG = c;
}

void putstr( char *str)
{
    char c;
    do
    {
        c = *str++;
        if (c!=0)
        {
            putch(c);
        }
    } while (c != 0);
        
}

boolean getch(char *c)
{
  return buffer_get(&uart_rx_buffer, c);      
}


void ClearUartReceiveBuffer( void )
{
    buffer_clear(&uart_rx_buffer);
}



