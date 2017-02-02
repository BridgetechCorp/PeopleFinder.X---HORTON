#include <string.h>

#include "ecan.h"
#include "types.h"
#include "buffer.h"
#include "system.h"
//#include <ecan.h>



// Since the PC likes to read alot of messages at the same time
// allow up to 20 high speed messages to come over.
#define CAN_RX_BUFFER_SIZE 40

boolean CanRxBufferOverFlowOccured = FALSE;
boolean CanTxBufferOverFlowOccured = FALSE;

// CAN Messages in RAM
mID canTxMessage;
buffer_t can_rx_buffer;
mID canRxBuffer[CAN_RX_BUFFER_SIZE];

int8 can_address; // this is the address we get from the rotary dial

// Counter for CAN messages in buffer
// This value is adjusted inside an interrupt
volatile uint8 can_command = 0;

// Define ECAN Message Buffers
// this was from Craig: it works if optimizer turned off for this module.
// __eds__ unsigned int ecan1msgBuf[12][8] __attribute__((space(dma)));// 12 buffers, // 8 words each
// dspic33ep data sheet says if using DMAxSTA and DMAxSTB, memory buffers must be aligned to 64 BYTE boundaries (lowest 6 address bits 0)

// DWS trying things.. make sure ecan.h matchs up !!
// DWS  Seems to work for optimizer -O1 and -O2, but not -Os with my macro replacement for readEds()  (MPLAB 8.8 + C3.30)
// dspic33ep data sheet says if using DMAxSTA and DMAxSTB, memory buffers must be aligned to 64 BYTE boundaries (lowest 6 address bits 0)
__eds__ unsigned int ecan1msgBuf[12][8] __attribute__((eds,space(dma),aligned(64)));  // 12 buffers, 8 words each

// These are just statistics
int can_messages_rcvd = 0;
int can_messages_sent = 0;

// NOTE: the CAN address is 100 + the value of the rotary dial
// so it ranges from 100 to 115
int8 ReadCANAddressFromRotaryDial( void )
{
  can_address = 100;
  return can_address;
}


void ecan1ClkInit(void){
  /* FCAN is selected to be FCY
  FCAN = FCY = 40MHz 60MHz for dspic33ep*/
  C1CTRL1bits.CANCKS = ECAN_CANCKS; // FCAN is equal to twice Fp?
  //	C1CTRL1bits.CANCKS = 0x0; // FCAN is equal to twice Fp?

  /*
  Bit Time = (Sync Segment + Propagation Delay + Phase Segment 1 + Phase Segment 2)=20*TQ
  Phase Segment 1 = 8TQ
  Phase Segment 2 = 6Tq
  Propagation Delay = 5Tq
  Sync Segment = 1TQ
  CiCFG1<BRP> =(FCAN /(2 ×N×FBAUD))– 1
  */

  /* Synchronization Jump Width set to 4 TQ */
  C1CFG1bits.SJW = 0x3;
  /* Baud Rate Prescaler */
  C1CFG1bits.BRP = BRP_VAL;

  /* Phase Segment 1 time is 8 TQ */
  C1CFG2bits.SEG1PH=0x7;
  /* Phase Segment 2 time is set to be programmable */
  C1CFG2bits.SEG2PHTS = 0x1;
  /* Phase Segment 2 time is 6 TQ */
  C1CFG2bits.SEG2PH = 0x5;
  /* Propagation Segment time is 5 TQ */
  C1CFG2bits.PRSEG = 0x4;
  /* Bus line is sampled three times at the sample point */
  C1CFG2bits.SAM = 0x1;
}


void initECAN(void)
{
  //   char i;
  //	unsigned int test;
  //	unsigned int dorr;
  //   unsigned int *ptr;

  // !! chip errata fix !! for silicon rev B1
  // Elevate priority of DMA to avoid lost can RX interrupts. 
  MSTRPR = 0x20;

  // read the CAN address from the rotary dial
  ReadCANAddressFromRotaryDial();

  // Create rx buffer
  buffer_initialize (&can_rx_buffer,&canRxBuffer[0], sizeof (mID), CAN_RX_BUFFER_SIZE );

#ifdef _SIMULATE
  printf("SIMLUATION MODE ECAN not initialized\n");
  return;
#endif

  /* Request Configuration Mode */
  C1CTRL1bits.REQOP=4;
  while(C1CTRL1bits.OPMODE!=4) {}

  ecan1ClkInit();
  //C1FCTRL = 0xC01F; // No FIFO, 32 Buffers

  /*	Filter Configuration
  ecan1WriteRxAcptFilter(int n, long identifier, unsigned int exide,unsigned int bufPnt,unsigned int maskSel)
  n = 0 to 15 -> Filter number
  identifier -> SID <10:0> : EID <17:0>
  exide = 0 -> Match messages with standard identifier addresses
  exide = 1 -> Match messages with extended identifier addresses
  bufPnt = 0 to 14  -> RX Buffer 0 to 14
  bufPnt = 15 -> RX FIFO Buffer
  maskSel = 0	->	Acceptance Mask 0 register contains mask
  maskSel = 1	->	Acceptance Mask 1 register contains mask
  maskSel = 2	->	Acceptance Mask 2 register contains mask
  maskSel = 3	->	No Mask Selection
  */

  ecan1WriteRxAcptFilter(1, can_address ,1,1,0);
  C1CTRL1bits.WIN=1; // Enable the filter window

  /* Filter 0 to 6 are enabled enabled for Identifier match with incoming message */
  C1FEN1bits.FLTEN0=0x1;
  C1FEN1bits.FLTEN1=0x1;
  C1FEN1bits.FLTEN2=0x1;
  C1FEN1bits.FLTEN3=0x1;
  C1FEN1bits.FLTEN4=0x1;
  C1FEN1bits.FLTEN5=0x1;
  C1FEN1bits.FLTEN6=0x1;

  /* Acceptance Filter 0 point to buffer index N to store message */
  C1BUFPNT1bits.F0BP = 0x1; // filter 0 point to buffer index 1 (index 0 is reserved for the transmitter)
  C1BUFPNT1bits.F1BP = 0x2; // filter 1 points to buffer index 2
  C1BUFPNT1bits.F2BP = 0x3; // etc...
  C1BUFPNT1bits.F3BP = 0x4;
  C1BUFPNT2bits.F4BP = 0x5;
  C1BUFPNT2bits.F5BP = 0x6;
  C1BUFPNT2bits.F6BP = 0x7;

  /* Configure Acceptance Filter 0 to match standard identifier Filter Bits (11-bits): 0b011 1010 xxx with the mask setting, message with SID
  range 0x1D0-0x1D7 will be accepted by the ECAN module. */
  C1RXF0SIDbits.SID = can_address; // the can address
  C1RXF1SIDbits.SID = can_address; // the can address
  C1RXF2SIDbits.SID = can_address; // the can address
  C1RXF3SIDbits.SID = can_address; // the can address
  C1RXF4SIDbits.SID = can_address; // the can address
  C1RXF5SIDbits.SID = can_address; // the can address
  C1RXF6SIDbits.SID = can_address; // the can address

  /* Select Acceptance Filter Mask 0 for Acceptance Filter 0 */
  C1FMSKSEL1bits.F0MSK=0x0; // mask 0 used for filter 0
  C1FMSKSEL1bits.F1MSK=0x0; // mask 0 used for filter 1
  C1FMSKSEL1bits.F2MSK=0x0; // mask 0 used for filter 2
  C1FMSKSEL1bits.F3MSK=0x0; // mask 0 used for filter 3
  C1FMSKSEL1bits.F4MSK=0x0; // mask 0 used for filter 4
  C1FMSKSEL1bits.F5MSK=0x0; // mask 0 used for filter 5
  C1FMSKSEL1bits.F6MSK=0x0; // mask 0 used for filter 6

  /* Configure acceptance filter mask to lower 7 bits */
  /* Configure Acceptance Filter Mask 0 register to mask SID<2:0>
  * Mask Bits (11-bits) : 0b111 1111 1000 */
  C1RXM0SIDbits.SID  = 0b00000000000; // mask out all bits, because we should accept everything
  C1RXM0SIDbits.MIDE = 0;    // match either extended or standard if filter matches
  C1RXM0SIDbits.EID  = 0x00; // EID is a dont care in filter comparison

  C1FCTRLbits.DMABS = 3;	/* 12 buffer CAN Message Buffers in device RAM */
  C1FCTRLbits.FSA   = 0; /* FIFO START AREA bits */

  /* Clear Window Bit to Access ECAN
  * Control Registers */
  C1CTRL1bits.WIN=0x0;

  C1TR01CONbits.TXEN0=1;			/* ECAN1, Buffer 0  is a TRANSMIT Buffer */
  C1TR01CONbits.TXEN1=0;			/* ECAN1, Buffer 1  is a Receive Buffer */
  C1TR23CONbits.TXEN2=0;			/* ECAN1, Buffer 2  is a Receive Buffer */
  C1TR23CONbits.TXEN3=0;			/* ECAN1, Buffer 3  is a Receive Buffer */
  C1TR45CONbits.TXEN4=0;			/* ECAN1, Buffer 4  is a Receive Buffer */
  C1TR45CONbits.TXEN5=0;			/* ECAN1, Buffer 5  is a Receive Buffer */
  C1TR67CONbits.TXEN6=0;			/* ECAN1, Buffer 6  is a Receive Buffer */
  C1TR67CONbits.TXEN7=0;			/* ECAN1, Buffer 7  is a Receive Buffer */

  C1TR01CONbits.TX0PRI=0b11; 		/* Message Buffer 0 High Priority Level */
  C1TR01CONbits.TX1PRI=0b10; 		/* Message Buffer 1 Intermediate Priority Level */

  /* Enter Normal Mode */
  C1CTRL1bits.REQOP=0;
  while(C1CTRL1bits.OPMODE!=0) {}

  //Enable DMA Buffers
  dma0init();
  dma2init();

  /* Enable CAN1 interrupts */
  //IEC2bits.C1IE = 1;
  //C1INTEbits.TBIE = 0; // no need to enable interrupts
  //C1INTEbits.RBIE = 0; // no need to enable interrupts
}


/* ECAN1 buffer loaded with Identifiers and Data */
void ecan1WriteMessage(void)
{
  /* Writing the message for Transmission
  ecan1WriteTxMsgBufId(unsigned int buf, long txIdentifier, unsigned int ide, unsigned int remoteTransmit);
  ecan1WriteTxMsgBufData(unsigned int buf, unsigned int dataLength, unsigned int data1, unsigned int data2, unsigned int data3, unsigned int data4);
  buf -> Transmit Buffer number
  txIdentifier -> SID<10:0> : EID<17:0>
  ide = 0 -> Message will transmit standard identifier
  ide = 1 -> Message will transmit extended identifier
  remoteTransmit = 0 -> Normal message
  remoteTransmit = 1 -> Message will request remote transmission
  dataLength -> Data length can be from 0 to 8 bytes
  data1, data2, data3, data4 -> Data words (2 bytes) each
  */
  ecan1WriteTxMsgBufId(0,0x1FFEFFFF,0,0);
  ecan1WriteTxMsgBufData(0,8,0x1111,0x2222,0x3333,0x4444);
}


/*
// !!! I think this is causing the problems when optimzer is turned on.
// This reads 16 bit values from EDS (extended data space) memory pointers
unsigned int readEds( __eds__ unsigned int *x)
{
unsigned int val;
val = *x;
return val;
}
*/


// This still has problems when the optimizer turned on -Os
// Seems to work for optimizer -O1 and -O2
#define readEds(ptr_eds)  (*(__eds__ unsigned int *)ptr_eds)


/******************************************************************************
*
*    Function:			rxECAN1
*    Description:       moves the message from the DMA memory to RAM
*
*    Arguments:			*message: a pointer to the message structure in RAM
*						that will store the message.
*	 Author:           Craig Beiferman
*
*
******************************************************************************/
void rxECAN( int bufidx )
{
  unsigned int temp;
  unsigned int ide=0;
  unsigned int srr=0;
  mID canmsg;

  memset(&canmsg,0,sizeof(mID));

  temp = readEds( &ecan1msgBuf[bufidx][0] );
  ide = temp & 0x0001;
  srr = temp & 0x0002;

  /* check to see what type of message it is */
  /* message is standard identifier */
  if(ide==0)
  {
    canmsg.id=(readEds( &ecan1msgBuf[bufidx][0]) & 0x1FFC) >> 2;
    canmsg.id= (temp & 0x1FFC) >> 2;
    canmsg.frame_type=CAN_FRAME_STD;
  }
  /* message is extended identifier */
  else
  {
    canmsg.id = ((unsigned long)(temp & 0x1FFC)) << 16;
    canmsg.id += (readEds( &ecan1msgBuf[bufidx][1] ) & 0x0FFF) << 6;
    canmsg.id += (readEds( &ecan1msgBuf[bufidx][2] ) & 0xFC00) >> 10;
    canmsg.frame_type=CAN_FRAME_EXT;
  }
  /* check to see what type of message it is */
  /* RTR message */
  if(srr==1)
  {
    canmsg.message_type=CAN_MSG_RTR;
  }
  /* normal message */
  else
  {
    canmsg.message_type=CAN_MSG_DATA;
    /*
    canmsg.data[0]=(unsigned char)readEds( &ecan1msgBuf[bufidx][3] );
    // The following line is causing us to jump into the _AddressError interrupt handler, when optimizer turned on
    canmsg.data[1]=(unsigned char)((readEds( &ecan1msgBuf[bufidx][3]) & 0xFF00) >> 8);
    canmsg.data[2]=(unsigned char)readEds( &ecan1msgBuf[bufidx][4]);
    canmsg.data[3]=(unsigned char)((readEds( &ecan1msgBuf[bufidx][4]) & 0xFF00) >> 8);
    canmsg.data[4]=(unsigned char)readEds( &ecan1msgBuf[bufidx][5]);
    canmsg.data[5]=(unsigned char)((readEds( &ecan1msgBuf[bufidx][5]) & 0xFF00) >> 8);
    canmsg.data[6]=(unsigned char)readEds( &ecan1msgBuf[bufidx][6]);
    canmsg.data[7]=(unsigned char)((readEds( &ecan1msgBuf[bufidx][6]) & 0xFF00) >> 8);
    canmsg.data_length=(unsigned char)(readEds( &ecan1msgBuf[bufidx][2]) & 0x000F);
    */
    // better optimized version of above
    // The following line is causing us to jump into the _AddressError interrupt handler, when optimizer turned on
    temp = readEds( &ecan1msgBuf[bufidx][3]);
    canmsg.data[0] = temp;
    canmsg.data[1] = temp>>8;
    temp = readEds( &ecan1msgBuf[bufidx][4]);
    canmsg.data[2] = temp;
    canmsg.data[3] = temp>>8;
    temp = readEds( &ecan1msgBuf[bufidx][5]);
    canmsg.data[4] = temp;
    canmsg.data[5] = temp>>8;
    temp = readEds( &ecan1msgBuf[bufidx][6]);
    canmsg.data[6] = temp;
    canmsg.data[7] = temp>>8;
    canmsg.data_length= (unsigned char)(readEds( &ecan1msgBuf[bufidx][2]) & 0x000F);
  }

  // If you get in here, the PC is simply sending CAN commands too quickly,
  // and we can't keep up. How do we automatically slow down PC?
  if (!buffer_put(&can_rx_buffer, (uint8*)&canmsg))
  {
      CanRxBufferOverFlowOccured = TRUE;
      AddErrorCode(CAN_RX_BUFFER_OVERFLOW);
  }
}



void ecan1WriteRxAcptFilter(int n, long identifier, unsigned int exide, unsigned int bufPnt,unsigned int maskSel)
{
  unsigned long sid10_0=0, eid15_0=0, eid17_16=0;
  unsigned int *sidRegAddr,*bufPntRegAddr,*maskSelRegAddr, *fltEnRegAddr;
  C1CTRL1bits.WIN=1;

  // Obtain the Address of CiRXFnSID, CiBUFPNTn, CiFMSKSELn and CiFEN register for a given filter number "n"
  sidRegAddr = (unsigned int *)(&C1RXF0SID + (n << 1));
  bufPntRegAddr = (unsigned int *)(&C1BUFPNT1 + (n >> 2));
  maskSelRegAddr = (unsigned int *)(&C1FMSKSEL1 + (n >> 3));
  fltEnRegAddr = (unsigned int *)(&C1FEN1);

  // Bit-filed manupulation to write to Filter identifier register
  if(exide==1) { 	// Filter Extended Identifier
    eid15_0 = (identifier & 0xFFFF);
    eid17_16= (identifier>>16) & 0x3;
    sid10_0 = (identifier>>18) & 0x7FF;

    *sidRegAddr=(((sid10_0)<<5) + 0x8) + eid17_16;	// Write to CiRXFnSID Register
    *(sidRegAddr+1)= eid15_0;					// Write to CiRXFnEID Register

  }else{			// Filter Standard Identifier
    sid10_0 = (identifier & 0x7FF);
    *sidRegAddr=(sid10_0)<<5;					// Write to CiRXFnSID Register
    *(sidRegAddr+1)=0;							// Write to CiRXFnEID Register
  }

  *bufPntRegAddr = (*bufPntRegAddr) & (0xFFFF - (0xF << (4 *(n & 3)))); // clear nibble
  *bufPntRegAddr = ((bufPnt << (4 *(n & 3))) | (*bufPntRegAddr));       // Write to C1BUFPNTn Register

  *maskSelRegAddr = (*maskSelRegAddr) & (0xFFFF - (0x3 << ((n & 7) * 2))); // clear 2 bits
  *maskSelRegAddr = ((maskSel << (2 * (n & 7))) | (*maskSelRegAddr));      // Write to C1FMSKSELn Register

  *fltEnRegAddr = ((0x1 << n) | (*fltEnRegAddr)); // Write to C1FEN1 Register

  C1CTRL1bits.WIN=0;
}


void ecan1WriteRxAcptMask(int m, long identifier, unsigned int mide, unsigned int exide)
{
  unsigned long sid10_0=0, eid15_0=0, eid17_16=0;
  unsigned int *maskRegAddr;
  C1CTRL1bits.WIN=1;

  // Obtain the Address of CiRXMmSID register for given Mask number "m"
  maskRegAddr = (unsigned int *)(&C1RXM0SID + (m << 1));

  // Bit-filed manipulation to write to Filter Mask register
  if(exide==1)
  { 	// Filter Extended Identifier
    eid15_0 = (identifier & 0xFFFF);
    eid17_16= (identifier>>16) & 0x3;
    sid10_0 = (identifier>>18) & 0x7FF;

    if(mide==1)
      *maskRegAddr=((sid10_0)<<5) + 0x0008 + eid17_16;	// Write to CiRXMnSID Register
    else
      *maskRegAddr=((sid10_0)<<5) + eid17_16;	// Write to CiRXMnSID Register
    *(maskRegAddr+1)= eid15_0;					// Write to CiRXMnEID Register
  }
  else
  {			// Filter Standard Identifier
    sid10_0 = (identifier & 0x7FF);
    if(mide==1)
      *maskRegAddr=((sid10_0)<<5) + 0x0008;					// Write to CiRXMnSID Register
    else
      *maskRegAddr=(sid10_0)<<5;					// Write to CiRXMnSID Register

    *(maskRegAddr+1)=0;							// Write to CiRXMnEID Register
  }

  C1CTRL1bits.WIN=0;
}


// Configures message buffer ID
// (But does not configure message data)
void ecan1WriteTxMsgBufId(unsigned int buf, long txIdentifier, unsigned int ide, unsigned int remoteTransmit)
{
  unsigned long word0=0, word1=0, word2=0;
  unsigned long sid10_0=0, eid5_0=0, eid17_6=0;//,a;
  if(ide)
  {
    eid5_0  = (txIdentifier & 0x3F);
    eid17_6 = (txIdentifier>>6) & 0xFFF;
    sid10_0 = (txIdentifier>>18) & 0x7FF;
    word1 = eid17_6;
  }
  else
  {
    sid10_0 = (txIdentifier & 0x7FF);
  }


  if(remoteTransmit==1) { 	// Transmit Remote Frame

    word0 = ((sid10_0 << 2) | ide | 0x2);
    word2 = ((eid5_0 << 10)| 0x0200);}

  else {

    word0 = ((sid10_0 << 2) | ide);
    word2 = (eid5_0 << 10);
  }

  // Obtain the Address of Transmit Buffer in DMA RAM for a given Transmit Buffer number

  if(ide)
    ecan1msgBuf[buf][0] = (word0 | 0x0002);
  else
    ecan1msgBuf[buf][0] = word0;

  ecan1msgBuf[buf][1] = word1;
  ecan1msgBuf[buf][2] = word2;
}


/* ECAN Transmit Data
Inputs :
buf -> Transmit Buffer Number
dataLength -> Length of Data in Bytes to be transmitted
data1/data2/data3/data4 ->  Transmit Data Bytes
*/
// This function assumes destination ID has been set up in the buffer prior to this call by
// ecan1WriteTxMsgBufId()
// ? does this actually trigger the sending of the canbus data?
// I think it just sets up the buffer, data but doesn't trigger the DMA transfer??
void ecan1WriteTxMsgBufData(unsigned int buf, unsigned int dataLength, unsigned int data1, unsigned int data2, unsigned int data3, unsigned int data4)
{

// DWS ?? fix ?? should we be doing readEds() here?
  // OLD: ecan1msgBuf[buf][2] = ((ecan1msgBuf[buf][2] & 0xFFF0) + dataLength) ;
  ecan1msgBuf[buf][2] = ((readEds(&ecan1msgBuf[buf][2]) & 0xFFF0) + dataLength); // NEW
  ecan1msgBuf[buf][3] = data1;
  ecan1msgBuf[buf][4] = data2;
  ecan1msgBuf[buf][5] = data3;
  ecan1msgBuf[buf][6] = data4;
}


/*------------------------------------------------------------------------------
Disable RX Acceptance Filter
void ecan1DisableRXFilter(int n)
------------------------------------------------------------------------------
n -> Filter number [0-15]
*/
void ecan1DisableRXFilter(int n)
{
  unsigned int *fltEnRegAddr;
  C1CTRL1bits.WIN=1;
  fltEnRegAddr = (unsigned int *)(&C1FEN1);
  *fltEnRegAddr = (*fltEnRegAddr) & (0xFFFF - (0x1 << n));
  C1CTRL1bits.WIN=0;
}


/* Dma Initialization for ECAN1 Transmission */
void dma0init(void)
{
  DMAPWC = 0;
  DMARQC = 0;
  DMA0CON=0x2020;
  DMA0PAD=0x0442;	/* ECAN 1 (C1TXD) */
  DMA0CNT=0x0007;   // up to 8 messages
  DMA0REQ=0x0046;	/* ECAN 1 Transmit */
  DMA0STAH = __builtin_dmapage(ecan1msgBuf);
  DMA0STAL = __builtin_dmaoffset(ecan1msgBuf);
  DMA0CONbits.CHEN=1;
  _DMA0IE = 1;
}


/* Dma Initialization for ECAN1 Reception */
void dma2init(void)
{
  DMAPWC = 0; // clear the DMA write collision flags
  DMARQC = 0; // clear DMA request collision status register??
  //DMA2CON=0x0020;
  //DMA2PAD=0x0440;	/* ECAN 1 (C1RXD) */
  //DMA2CNT=0x0007;    /* Up to 8 messages */
  //DMA2REQ=0x0022;	/* ECAN 1 Receive */
  //DMA2STAH = __builtin_dmapage(ecan1msgBuf);
  //DMA2STAL = __builtin_dmaoffset(ecan1msgBuf);
  //DMA2CONbits.CHEN=1;
  //_DMA2IE = 1;

  //DMA2CONbits.SIZE = 0x0; // WORD
  //DMA2CONbits.DIR = 0x0;  // reead from peripheral write to DSPRAM or RAM
  DMA2CONbits.AMODE = 0x2; // continuous mode single buffer,
  DMA2CONbits.MODE = 0x0;  // peripheral indirect addressing

  DMA2PAD = (volatile unsigned int)&C1RXD; // point to ECAN1 RX register

  DMA2STAL = __builtin_dmaoffset(ecan1msgBuf);
  DMA2STAH = __builtin_dmapage(ecan1msgBuf);

  DMA2CNT = 7;  // TODO, should this be 1 or 7?
  DMA2REQ = 0x22; // ECAN1 rx data ready

  IEC1bits.DMA2IE = 1; // enable DMA Channel 2 interrupt
  DMA2CONbits.CHEN = 0x1;	 // Enable DMA Channel 2
}


boolean IsCanCommandAvailable(void)
{
  if (can_command)
  {
    can_command--;

    return TRUE;
  }
  else
    return FALSE;
}


void retrieve_can_message(mID* message)
{
  buffer_get(&can_rx_buffer, (uint8*)message);
}


void __attribute__((interrupt, auto_psv)) _DMA0Interrupt(void)
{
  _DMA0IF = 0;//Clear the DMA0 Interrupt Flag
}


// Note, this fuction is called from inside _DMA2Interrupt
void TransferCANMessageFromDMAtoBuffer()
{
  // Data sheet says RXFUL bits only available when WIN bit is cleared.
  C1CTRL1bits.WIN=0; // not sure if needed here

  if (C1RXFUL1bits.RXFUL1==1)
  {
    can_command++;
    rxECAN(1);
    C1RXFUL1bits.RXFUL1=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL2==1)
  {
    can_command++;
    rxECAN(2);
    C1RXFUL1bits.RXFUL2=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL3==1)
  {
    can_command++;
    rxECAN(3);
    C1RXFUL1bits.RXFUL3=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL4==1)
  {
    can_command++;
    rxECAN(4);
    C1RXFUL1bits.RXFUL4=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL5==1)
  {
    can_command++;
    rxECAN(5);
    C1RXFUL1bits.RXFUL5=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL6==1)
  {
    can_command++;
    rxECAN(6);
    C1RXFUL1bits.RXFUL6=0; // don't clear this until we process message
  }
  if (C1RXFUL1bits.RXFUL7==1)
  {
    can_command++;
    rxECAN(7);
    C1RXFUL1bits.RXFUL7=0; // don't clear this until we process message
  }
}


void __attribute__((interrupt, auto_psv)) _DMA2Interrupt(void)
{
  //_LATC14 ^= 1; // TODO: REMOVE THIS LATER
  if (C1RXFUL1)
  {
    TransferCANMessageFromDMAtoBuffer();
  }
  _DMA2IF = 0;//Clear the DMA0 Interrupt Flag
}



void __attribute__((interrupt, no_auto_psv))_C1Interrupt(void)
{
  IFS2bits.C1IF = 0;        // clear interrupt flag

  if(C1INTFbits.TBIF)
  {
    C1INTFbits.TBIF = 0;
  }

  if(C1INTFbits.RBIF)
  {
    // read the message
    /*  if (C1RXFUL1bits.RXFUL1==1)
    {
    _LATC14 ^= 1; // TODO: REMOVE THIS LATER
    //rx_ecan1message.buffer=1;
    canRxMessage.buffer=1;
    C1RXFUL1bits.RXFUL1=0;

    //GreenLedToggle();
  }*/

    //	rxECAN(&canRxMessage);
    //rxECAN1(&rx_ecan1message);

    C1INTFbits.RBIF = 0;
    //	can_command++;
  }
}


void SendStatus( int id, int status )
{
  ecan1WriteTxMsgBufId  (  0, id, 0, 0); // standard identifier, no rtr bit
  ecan1WriteTxMsgBufData(  0, 2, status, 0, 0, 0);
}


extern mID * pMsgDebug;

// Send a pre-constructed message
// !!! It looked like this function is receiving incorrect address of message from dictionary.c  process_can_mesage !!!
// But I think it is a debugger problem.
// I put code in below to check the pointer and it is indeed correct.
void sendECAN(mID *message)
{
  long id;
  char *data;
  int datalen;

  can_messages_sent++;

  id = (long)message->id;
  data = (char *)&message->data[0];
  datalen = message->data_length;

  SendCanMessage( id, data, datalen );
}


// returns TRUE if CAN TX hardware is free
inline boolean IsCanTXFree(void)
{
  return (C1TR01CONbits.TXREQ0==0);
}


// lowest-level canbus send.
// Anyone sending a CAN message should call this function only 
// so the DMA transmit buffers are filled in properly
void SendCanMessage( long id, char *data, int datalen )
{
  static int dmabuffer = 0; // we are always going to use DMA buffer 0 for transmit, and buffer 1 for receive??

  unsigned int word1=0;
  unsigned int word2=0;
  unsigned int word3=0;
  unsigned int word4=0;
  unsigned long timeout =0;

  // block if we are waiting to transmit some more data over the
  // CAN bus.
  while (C1TR01CONbits.TXREQ0==1)
  {
    if (timeout++ >= 100000)
    {
      // this is not good, we have been in this loop for 100000 cycles,
      // let's leave and turn off motor because the CAN bus must be stuck somehow
       C1TR01CONbits.TXREQ0==0; // Not sure if this works to stop transmit.

      AddErrorCode(CAN_TX_TIMEOUT);      
      return;
    }
    word1 =0;
  }

  ecan1WriteTxMsgBufId  (  dmabuffer, id, 1, 0); // standard identifier, no rtr bit

  if (datalen>0) word1 |= (unsigned int)data[0] & 0x00FF;
  if (datalen>1) word1 |= (unsigned int)data[1]<<8;
  if (datalen>2) word2 |= (unsigned int)data[2] & 0x00FF;
  if (datalen>3) word2 |= (unsigned int)data[3]<<8;
  if (datalen>4) word3 |= (unsigned int)data[4] & 0x00FF;
  if (datalen>5) word3 |= (unsigned int)data[5]<<8;
  if (datalen>6) word4 |= (unsigned int)data[6] & 0x00FF;
  if (datalen>7) word4 |= (unsigned int)data[7]<<8;

  ecan1WriteTxMsgBufData(  dmabuffer, datalen, word1, word2, word3, word4);
  C1TR01CONbits.TXREQ0=1;
}
