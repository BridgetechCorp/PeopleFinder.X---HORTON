#ifndef __ECAN1_CONFIG_H__
#define __ECAN1_CONFIG_H__ 

#include "types.h"
#include "buffer.h"

/* CAN Baud Rate Configuration 		*/
// __dsPIC33EP512MC806__
// !! NOTE CHIP ERRATA - CANCKS BIT IS INVERTED FROM DATA SHEET!!
// 0 : FCAN = 2*FCY
// 1 : FCAN = FCY  (COMPATIBLE WITH OLDER PIC33F OR PIC24H)
	#define ECAN_CANCKS 0x0
	#define FCAN  	(FP*2L)
	#define BITRATE 250000L
	#define NTQ 	20L		// 20 Time Quanta in a Bit Time    
	
	#define FTQ NTQ*BITRATE
	
	#define BRP_VAL		((FCAN/(2L*FTQ))-1L)
   // #if (BRPVAL<=0)
//		ERROR 
//	#endif


/* CAN Message Buffer Configuration */
// Warning don't change unless you change the DMA DMABS settings
#define  ECAN1_MSG_BUF_LENGTH 	16 
// from Craig, works without optimizer
//  extern __eds__ unsigned int ecan1msgBuf[12][8] __attribute__((eds,space(dma)));// 12 buffers, // 8 words each
// DWS  Seems to work for optimizer -O1 and -O2, but not -Os with my macro replacement for readEds()  (MPLAB 8.8 + C3.30)
// dspic33ep data sheet says if using DMAxSTA and DMAxSTB, memory buffers must be aligned to 64 BYTE boundaries (lowest 6 address bits 0)
extern __eds__ unsigned int ecan1msgBuf[12][8] __attribute__((eds,space(dma),aligned(64)));// 12 buffers, 8 words each

/* Structure for CAN data */
#define CAN_MSG_DATA	0x01 // message type 
#define CAN_MSG_RTR		0x02 // data or RTR
#define CAN_FRAME_EXT	0x03 // Frame type
#define CAN_FRAME_STD	0x04 // extended or standard
#define CAN_BUF_FULL	0x05
#define CAN_BUF_EMPTY	0x06

/* message structure in RAM */
typedef struct{
	/* keep track of the buffer status */
	unsigned char buffer_status;
	/* RTR message or data message */
	unsigned char message_type;
	/* frame type extended or standard */
	unsigned char frame_type;
	/* buffer being used to reference the message */
	//unsigned char buffer;
	/* 29 bit id max of 0x1FFF FFFF 
	*  11 bit id max of 0x7FF */
	unsigned long id; 
	/* message data */
	unsigned char data[8];	
	/* received message data length */
	unsigned char data_length;
}mID;

extern mID canTxMessage;
extern buffer_t can_rx_buffer;
extern  int8 can_address;
extern int can_messages_rcvd;
extern int can_messages_sent;


/* Function Prototype 	*/
void initECAN(void);
void dma0init(void); 
void dma2init(void);
void ecan1WriteRxAcptFilter(int n, long identifier, unsigned int exide,unsigned int bufPnt,unsigned int maskSel);
void ecan1WriteRxAcptMask(int m, long identifierMask, unsigned int mide,unsigned int exide);
void ecan1WriteTxMsgBufId(unsigned int buf, long txIdentifier, unsigned int ide, unsigned int remoteTransmit);
void ecan1WriteTxMsgBufData(unsigned int buf, unsigned int dataLength, unsigned int data1, unsigned int data2, unsigned int data3, unsigned int data4);
void ecan1DisableRXFilter(int n);
boolean IsCanCommandAvailable (void);
inline boolean IsCanTXFree(void);
void SendCanMessage( long id, char *data, int datalen );
int8 ReadCANAddressFromRotaryDial( void );
void rxECAN( int bufidx );
void sendECAN(mID *message) ;
void retrieve_can_message(mID* message);


#endif
