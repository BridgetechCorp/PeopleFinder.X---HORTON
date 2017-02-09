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

#include <string.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */
#include "ecan.h"
#include "oscillator.h"
#include "CanIrma.h"
#include "timer.h"
#include "uart.h"
#include <stdio.h>
#include <ctype.h>


void DisplayTimeAndCount( int count );

// DSPIC33EP256MU806 Configuration Bit Settings


// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GSS = OFF                // General Segment Code-Protect bit (General Segment Code protect is disabled)
#pragma config GSSK = OFF               // General Segment Key bits (General Segment Write Protection and Code Protection is Disabled)

// FOSCSEL
#pragma config FNOSC = PRI              // Initial Oscillator Source Selection bits (Primary Oscillator (XT, HS, EC))
//#pragma config FNOSC = FRC              // Initial Oscillator Source Selection bits (Primary Oscillator (XT, HS, EC))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = ON             // Peripheral pin select configuration (Allow only one reconfiguration)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Wait Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
//#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)
#pragma config FWDTEN = ON             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // Power-on Reset Timer Value Select bits (128ms)
#pragma config BOREN = ON               // Brown-out Reset (BOR) Detection Enable bit (BOR is enabled)
#pragma config ALTI2C1 = OFF            // Alternate I2C pins for I2C1 (SDA1/SCK1 pins are selected as the I/O pins for I2C1)

// FICD
#pragma config ICS = PGD3               // ICD Communication Channel Select bits (Communicate on PGEC3 and PGED3)
#pragma config RSTPRI = PF              // Reset Target Vector Select bit (Device will obtain reset instruction from Primary flash)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FAS
#pragma config AWRP = OFF               // Auxiliary Segment Write-protect bit (Aux Flash may be written)
#pragma config APL = OFF                // Auxiliary Segment Code-protect bit (Aux Flash Code protect is disabled)
#pragma config APLK = OFF               // Auxiliary Segment Key bits (Aux Flash Write Protection and Code Protection is Disabled)


typedef enum
{
    INIT_STATE = 0,
    WAIT_EXTERNAL_ENABLE,
    //WAIT_EXTERNAL_RESET,
    //EXTERNAL_RESET_PRESSED,
    //EXTERNAL_RESET_RELEASED,
    //SABOTAGED,
    //SABOTAGED2,
    //SABOTAGED3,
    RUNNING,
            // PROX switches commmented out
    //WAIT_PROX1_PRESSED,
    //WAIT_PROX1_RELEASED,
    //WAIT_PROX1_PRESSED_2,
    //WAIT_PROX1_RELEASED_2,
    //WAIT_PROX1_PRESSED_3,
    //WAIT_PROX1_RELEASED_3,
    //WAIT_PROX2_PRESSED,
    //WAIT_PROX2_RELEASED,
  //  END_STATE
} my_state_t;

extern int global_timer;

my_state_t state = INIT_STATE;

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/
// declare a holder for the latest incoming CAN message
mID message;
mID messages[10];
mID txMessage;




uint16 PeopleCountIn  = 0;
uint16 PeopleCountOut = 0;

uint16 InOffset = 0;
uint16 OutOffset = 0;

int last_global_timer=0;

int can_msgs_pending = 0;

int FuncArea = 1;

#define MAXUARTSIZE 30
char uartcommand[MAXUARTSIZE];
int uartindex=0;

boolean SensorEnabled = false;
int SensorStatus = 0;
//int Sabotaged = 0;

//int ProxCount = 0;
extern boolean flashblink;
extern boolean slowblink;

unsigned long sensorCanAddress;

int system_disi_level;

void ClearAllMessages( void );

int (*RunLevel5Main)(void);

int process_can_message(mID messageType)
{ // Accepts a CAN message from the PC and organizes it for the slave to read/write


//  uint8 isread, from_address,  registerNum;
//  unsigned char canData[6]; // Unsure if this initialization is actually needed, considering the CAN message is sent based on the register's byte size
//  int16 i;


   // copy the message based on first byte in data.
  if (messageType.data[0] < 0x6)
  {
    memcpy(&messages[messageType.data[0]],&messageType,sizeof(mID));
  }
  else
  {
      putstr("BAD CAN DATA\r\n");
  }

  return messageType.data[0]; // return the last message id
  //for (i = 0; i < messageType.data_length - 2 ; i++)
  //  canData[i] = messageType.data[i + 2];
  //registerNum = messageType.data[1];
  //isread = messageType.data[0] >> 7;
  //from_address = messageType.data[0] & 0x7F;
}


/* i.e. uint16_t <variable_name>; */
// *** CheckForCanMessages ****************************************************************//
// return 1 if a can message was received, 0 otherwise
int CheckForCanMessages( void )
{   
   // Determines whether or not a CAN message is waiting in the buffer
  // DANGER don't use while loop, just process one message at a time
  if (IsCanCommandAvailable())
  {
    can_msgs_pending++;    
    can_messages_rcvd++; // total count   
    retrieve_can_message(&message);   
    process_can_message(message);   // put messages into array based on first byte   
  }
    
  return can_msgs_pending;
}

int RunLevel5MainFunc(void);

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


void GetCANAddressFromBuffer( void )
{  
   sensorCanAddress = messages[1].id & 0x000FFFFF;
}

void FlushCanMessages( void )
{
    // flush all pending CAN message
    while (IsCanCommandAvailable())
    {
       retrieve_can_message(&message);
    }
    ClearAllMessages();
}

void ClearAllMessages( void )
{
    int i; //,j;
    
   // DisplayTimeAndCount(-1);
    // if (!UARTMODESW) { putstr("CLR\r\n"); }
    can_msgs_pending = 0;
    for (i=0;i<10;i++)
    {
        memset(&messages[i] ,0xFF ,sizeof(mID));      
    }
}

void DisplayTimeAndCount( int count )
{
    char str[50];
    int diff = global_timer - last_global_timer;
     
    sprintf(str,"%d %d\r\n",diff,count);
    putstr(str); 
    
    last_global_timer = global_timer;
}

void  DisplayMessage( void )
{
    int i,idx;
    char str[10];
    unsigned char c;
    
    putstr("display msg:\r\n"); 
    for (idx=1;idx<4;idx++)
    {
        for (i=0;i<8;i++)
        {
            putch('[');
            c = messages[idx].data[i];
            if (isalpha(c))
            {
                putch(c);
            }
            else
            {
              sprintf(str,"%x",c);
              putstr(str);
            }
            putch(']');
        }
        putstr("\r\n");
    }           
}

// if readstatus is true, then read the sensor status,
// otherwise, read the door count.
// aka,if door is closed, read sensor status, if door is open
// then read door count.
void ParseCanMessages( void )
{
//    static boolean MsgFlop = false;
    int k;
    int lastInCount=0;
    int num_msgs = CheckForCanMessages();
    
   
     
 // Check to see if a status message arrived, but no status is available
    if (num_msgs == 0)
    {
        return;
    }
    
    //DisplayTimeAndCount(num_msgs);
    
    
    if (num_msgs == 1)
    {
       if ((messages[1].data[4] == 'S') &&
           (messages[1].data[5] == 0x30) &&
           (messages[1].data[6] == 'G') &&
           (messages[1].data[7] == 0x00) ) // no status message waiting
       {
           // Sensor Status Query Response
            if (!UARTMODESW) { putstr("Sensor Status Query Response\r\n"); } 
            SensorStatus = 0; // no status is available
       }
       else if ((messages[1].data[4] == 'c') &&
                (messages[1].data[5] == 0x10) &&
                (messages[1].data[6] == 'B') )
       {           
            // this is a BROADCAST SCAN RESPONSE
            if (!UARTMODESW) { 
                putstr("BROADCAST SCAN RESPONSE\r\n");
                //DisplayMessage();               
            } 
       }
       else if  ((messages[1].data[4] == 'D') &&
                 (messages[1].data[5] == 0x30) &&
                 (messages[1].data[6] == 'S') )
       {
         // Door State Count Message (who requested this?)
            // this is sent when DoorOpen and DoorClose are requested
           //if (!UARTMODESW) { putstr("Door State Count Message\r\n"); } 
       }
       else if  ((messages[1].data[4] == 'C') &&
                 (messages[1].data[5] == 0x60) &&
                 (messages[1].data[6] == 'G') )
       {
         // premature Count Data Query Response
            //if (!UARTMODESW) { putstr("Count Data Query Response\r\n"); } 
       }
        else if  ((messages[1].data[4] == 'C') &&
                 (messages[1].data[5] == 0x60) &&
                 (messages[1].data[6] == 'R') )
       {        
            if (!UARTMODESW) { putstr("Count Reset Response\r\n"); } 
       }
       else
       {
           if (!UARTMODESW) { 
               //putstr("Unknown Message\r\n");
               //DisplayMessage();              
           }            
       }
     }


    // check to see if status arrived, and grab the
    // Sensor Status if available
    else if (num_msgs == 2)
    {
           if ((messages[1].data[4] == 'S') &&
               (messages[1].data[5] == 0x30) &&
               (messages[1].data[6] == 'G') )
           {
               SensorStatus = messages[2].data[1];
               if (SensorStatus)
               {
                   putstr("SABOTAGE\r\n");
                   // rats we have a sabotage message here
                   //
                   k=4;
               }
               else
               {
                   putstr("NO SABOTAGE\r\n"); 
               }
               //Sabotaged = 1;
               ClearAllMessages();
           }
     }

     // Check for the people count
    else if (num_msgs == 3)
    {
        //DisplayTimeAndCount(-33);
        if ((messages[1].data[4] == 'C') &&
            (messages[1].data[5] == 0x60) &&
            (messages[1].data[6] == 'G') )
        {
           
            
            lastInCount = PeopleCountIn;
            PeopleCountIn =  ((uint16)messages[2].data[7])<<8 | (uint16)messages[2].data[6];
           // if (PeopleCountIn > 10)
           // {
           //     putstr("bad count\r\n");
           //     DisplayMessage();
           // }
            
            //if (lastInCount != PeopleCountIn )
           // {
           //     if (!UARTMODESW) 
           //     {                   
           //       DisplayTimeAndCount(PeopleCountIn);                 
           //     }
           // }
            
            if (UARTMODESW)
            {
              PeopleCountOut = ((uint16)messages[3].data[2])<<8 | (uint16)messages[3].data[1];
            }
            else
            {
                // jason doesn't want the count out to work any more for default mode
              PeopleCountOut = 0;
            }
            
           
        }
        else if  ((messages[1].data[4] == 'C') &&
                 (messages[1].data[5] == 0x60) &&
                 (messages[1].data[6] == 'R') )
        {        
            if (!UARTMODESW) 
            { 
               // putstr("Count Reset Response\r\n");
               // DisplayMessage();              
            } 
        }
        else if ((messages[1].data[4] == 'C') &&
                 (messages[1].data[5] == 0x60) &&
                 (messages[1].data[6] == 0x21) &&
                 (messages[1].data[7] == 'g')  &&
                 (messages[2].data[5] == 'W')  &&
                 (messages[2].data[6] == 'r')  &&
                 (messages[2].data[7] == 'o')  &&
                 (messages[3].data[4] == 'f')  &&
                 (messages[3].data[5] == 'u')  &&
                 (messages[3].data[6] == 'n')  &&
                 (messages[3].data[7] == 'c')                 
                )
        {
         if (!UARTMODESW) {   putstr("BAD FUNCTION AREA\r\n");  }
            //FuncArea++;
            if (FuncArea > 30) 
            {
                if (!UARTMODESW) {   putstr("FUNCTION AREA WRAPPING TO 0\r\n");  }
                FuncArea=0;
            }
        }
        else
        {
            if (!UARTMODESW) { 
               // putstr("Unknown msg 2\r\n"); 
               // DisplayMessage();                
            } 
           k=4;
        }
        ClearAllMessages();
    }
    else // we have more than 3 CAN messages?
    {
        putstr("More than 3 msgs\r\n"); 
       ClearAllMessages(); 
    }

   
}


boolean ExternalResetPressed( void )
{
    return (Input2Pressed); // || Input2Pressed);
}


boolean ExternalEnablePressed( void )
{
    return (Input1Pressed); // || Input4Pressed);
}

void HandleUartCommands(void) 
{
    char cc;

    while (getch(&cc)) {
        LED1 = !LED1;
        cc = toupper(cc);
        putch(cc);
        if (cc != '\n' && cc != '\r') {
            uartcommand[uartindex++] = cc;
            if (uartindex >= MAXUARTSIZE) {
                putstr("\r\nBUFFER OVERFLOW (use carriage return to terminate commands)\r\n");
                uartindex = 0;
            }
        } else {           
            if (!UARTMODESW) {
                putstr("\r\nERROR: DEFAULT MODE (Change switch 6 for UART MODE)\r\n");
            }                // ok we have a command do something about it
            else if (strncmp("ENABLE", uartcommand, 6) == 0) {
                SendIRMASetDoorsOpen(); // turn on sensor
                SensorEnabled = true;
                putstr("\r\nENABLING SENSOR\r\n");
            } else if (strncmp("DISABLE", uartcommand, 7) == 0) {
                SendIRMASetDoorsClose();    // Close doors to stop counting
                SensorEnabled = false;
                putstr("\r\nDISABLING SENSOR\r\n");                
            } 
            //else if (strncmp("RESET", uartcommand, 5) == 0) {
            //    //SendIRMACountDataReset(); // ack this command does not work                 
            //    putstr("\r\nRESET COUNT\r\n");            
            //} 
            else if (strncmp("STATUS", uartcommand, 6) == 0) {
                ParseCanMessages();                 
                if (SensorEnabled)
                {
                    sprintf(uartcommand, "\r\nIN:%d\r\n", PeopleCountIn);
                    putstr(uartcommand);               
                    sprintf(uartcommand, "OUT:%d\r\n", PeopleCountOut);
                    putstr(uartcommand);
                   // putstr("SENSOR:ON\r\n");
                }
                else                                                            
                {
                    putstr("\r\nSENSOR DISABLED\r\n");
                }
                   
            } else if (strncmp("HELP", uartcommand, 4) == 0) {
                putstr("\r\n=============================\r\n");
                putstr("ENABLE to turn on sensor and begin counting\r\n");
                putstr("DISABLE to turn off sensor and reset count\r\n");                          
                putstr("STATUS to get the current count\r\n");
                putstr("=============================\r\n");
            } 
            else if (uartindex == 0)
            {
               putstr("\r\n"); 
            }
            else
            {
                putstr("\r\nUNKNOWN COMMAND (");
                uartcommand[uartindex++] = '\0';
                putstr(uartcommand);
                putstr(") type HELP for available commands.\r\n");
            }
            
            // clear out the uartcommand, and the uartindex
            // after we process command
            for (uartindex=0;uartindex<MAXUARTSIZE;uartindex++)
            {
                uartcommand[uartindex] = 0;
            }
            uartindex = 0;            
        }
    }
}



void GetFunctionArea(unsigned long addr)
{                         
    // establish which function area the sensor is on.
    int counter = 100;
    char str[10];
    while (counter--)
    {
        if ( !UARTMODESW) 
        {
            putstr("GET Function Area ");
            sprintf(str, "%d\r\n", FuncArea);
            putstr(str);
        }
        SendIRMACounterStateRequest2(addr,FuncArea);
        SetWaitDelay(1000);    
        while (GetWaitDelay()!=0);   
        ParseCanMessages();  
        SetWaitDelay(1000);    
        while (GetWaitDelay()!=0);               
    }    
}
   
  
void Delay( int timeMs )
{
    int t =  global_timer + timeMs;
    while (global_timer != t);
}


// this function will properly handle the count
// calculation when the internal counter wraps
// at 65535
uint16_t GetCountWithWrap( uint16_t last, uint16_t val )
{
   // char str[40];
    long preval, newval;
    long count;
    preval = last;
    newval = val;
    count = newval - preval;
    if (count < 0)
    {
        count += 65536;
    }
    //sprintf(str, "%li %li %d\r\n", preval,newval,(uint16_t)count);
    //putstr(str);
    return (uint16_t)count;
}


int16_t main(void)
{
    int testcount=0;
    int count;

    /* Configure the oscillator for the device */
    InitOscillator();

    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;

    /* Initialize IO ports and peripherals */
    InitApp();

    InitUART1();
   
    putstr("\r\n");
    putstr("BT2I Version 5.0\r\n");   
    if (UARTMODESW)
    {       
        putstr("Uart Mode\r\n");   
    }
    else
    {       
        putstr("Default Mode\r\n"); 
    }    
    putstr("Connecting to Sensor\r\n");
    

    // Always start with both outputs on, to indicate there is a problem.
    OUT1 = TRUE;
    OUT2 = TRUE;

    // First Get the CAN address
    SendIRMAGetCanAddress();
    SetTimerInterrupts(0);
    while ( GetTimerInterrupts() < 400) // wait 20 milliseconds for response
    {
        if (CheckForCanMessages())
        {
            putstr("Connection Established\r\n");
            GetCANAddressFromBuffer();           
            break; // break out of while loop, yee haw, we got a CAN address back
        }
        else
        {            
            // No CAN bus was detected, keep trying forever
            if (testcount++ > 30) 
                putstr("Sensor not detected\r\n");
            SendIRMAGetCanAddress();
            SetTimerInterrupts(0);
            LED1 = (LED1)?FALSE:TRUE;
            LED2 = (LED2)?FALSE:TRUE;
        }
    }
    
    if ( !UARTMODESW) putstr("Request Function Area\r\n");
    
    
    //GetFunctionArea(sensorCanAddress);
    //SendIRMAFunctionAreaStatusRequest();
    SetWaitDelay(100);
    while (GetWaitDelay()!=0)// wait 100 milliseconds for response
    {       
        if (CheckForCanMessages())
        {
          //if ( !UARTMODESW) putstr("Got FA response\r\n");           
        }        
    }
   // DisplayMessage();
   
      
    SendIRMASetDoorsClose();    // Close doors to stop counting
    //SendIRMACountDataReset();
    SetWaitDelay(100);
    while (GetWaitDelay()!=0)
        ParseCanMessages(); //(FALSE); // read door count
    SendIRMACounterStateRequest(); 
    SetWaitDelay(100);
    while (GetWaitDelay()!=0)
        ParseCanMessages(); //ReadCount(FALSE); // read door count
       // if sensor still has count, clear it out
    InOffset = PeopleCountIn;
    OutOffset = PeopleCountOut;

    // start oini intialization state
    state = INIT_STATE;

    while (1)
    {
        ClrWdt();
        
        //DisplayTimeAndCount(PeopleCountIn);
        ParseCanMessages();  
        
        if ( GetTimerInterrupts() > 50) // every 50 milliseconds read count
        {
            SetTimerInterrupts(0);
            ClearAllMessages(); 
           // putstr("CNT rsqt:\r\n");
            SendIRMACounterStateRequest();  // make new count request
            //DisplayTimeAndCount(PeopleCountIn);
            //SendIRMASensorStateRequest();                
        }
        
        HandleUartCommands();  
        
        if ( UARTMODESW)
        {
            if (flashblink) { OUT1 = 0;  OUT2 = 1; LED2=1; }
            else            { OUT1 = 1;  OUT2 = 0; LED2=0; }
        }        
        else // default mode
        {
          switch(state)
          {
            case INIT_STATE:
                 // Always start with both outputs on, to indicate there is a problem.
               OUT1 = TRUE;
               OUT2 = TRUE;
               SendIRMASetDoorsClose();    // Close doors to stop counting
               SendIRMACounterStateRequest();
               ClearAllMessages(); // throw away any current messages
               SetTimerInterrupts(0); // dont read can messages for another 50 ms
               SendIRMACounterStateRequest();
                                      // else it might be corrupted
               //SendIRMACountDataReset();
               state = WAIT_EXTERNAL_ENABLE; //WAIT_PROX1_PRESSED;
               if (!UARTMODESW) { putstr("Waiting For Enable\r\n"); } 
    	       break;

            case WAIT_EXTERNAL_ENABLE:
                OUT1 = TRUE;
                OUT2 = TRUE;
                if ( ExternalEnablePressed())
                {
                    //SendIRMACountDataReset();
                       // Clear the people count
                    InOffset  = PeopleCountIn;
                    OutOffset = PeopleCountOut;
                    SendIRMASetDoorsOpen(); // turn on sensor
                    SendIRMACounterStateRequest();
                    ClearAllMessages(); // throw away any current messages
                    SetTimerInterrupts(0); // don't read can messages for another 50 ms
                    SendIRMACounterStateRequest();
                    state = RUNNING; //WAIT_PROX1_PRESSED;
                    if (!UARTMODESW) { putstr("External Enable pressed\r\n"); } 
                }
                else
                {
                    if (slowblink)
                    {
                        LED1 = 0;  LED2 = 0;
                    }
                    else
                    {
                        LED1 = 1;  LED2 = 1;
                    }
                }
                break;

            case RUNNING:
                if (!ExternalEnablePressed())
                {
                     state = INIT_STATE;                    
                }
                else if (ExternalResetPressed() )
                {
                    if (!UARTMODESW) {
                        putstr("reset pressed\r\n");                        
                    } 
                    if (flashblink) { LED1 = 0;  LED2 = 1; }
                    else            { LED1 = 1;  LED2 = 0; }
                    InOffset = PeopleCountIn;
                    OutOffset = PeopleCountOut;
                }
                else
                {
                    // wrap the result, because the counter will wrap at 65535
                     count = GetCountWithWrap(InOffset,PeopleCountIn) - GetCountWithWrap(OutOffset,PeopleCountOut);
                     //count = (PeopleCountIn-InOffset) - (PeopleCountOut-OutOffset);
                     switch (count)
                     {
                        case 0: // 0 people were counted
                           OUT1 = FALSE;  LED1 = 1;
                           OUT2 = FALSE;  LED2 = 1;
                           break;
                        case 1: // 1 person was counted
                           OUT1 = FALSE;  LED1 = 1;
                           OUT2 = TRUE;   LED2 = 0;
                           break;
                        default:  // 2 or more people counted
                           OUT1 = TRUE;   LED1 = 0;
                           OUT2 = FALSE;  LED2 = 1;
                           break;
                     }                   
                }
                break;
             } // switch (state))
        } // if (!UARTMODESW)
    }
}


