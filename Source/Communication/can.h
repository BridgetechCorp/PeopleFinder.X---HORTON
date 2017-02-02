/*==============================================================================================*
 |       Filename: can.h                                                                        |
 | Project/Module: A21 or GATEWAY/module group Communication                                    |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Basic functions for CAN communication.                                       |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CAN_INC							/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "..\communication_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define MAX_CAN_DATA_LEN		8

//  Refer to Infineon C167CR Derivatives User´s Manual, V3.2, pages 18-11 and 18-12.
//
//  TSEG1 + 2: part of CAN bit time before sample point
//  TSEG2 + 1: part of CAN bit time after  sample point
//
//					                   fCPU								  1
//	Baudrate = ---------------------------------------------------- = ---------
//	           2 * ( TSEG2 + TSEG1 + 3) * ( BTR0_VAL_xxxxkbps + 1 )	   Nq * tq
//
//
// Ratio (TSEG1 + 2) / (TSEG2 + 1) = 3 is optimized for great propagation delays,
// i.e. for long cables. Oscillator tolerances of CAN IRMA devices have to be
// tight enough for the short period TSEG2.
// XTAL tolerances of CAN IRMA devices: <= 100ppm tolerance at 25 degrees Celsius
//                                      <= 100ppm temperature drift
// maximum XTAL frequency deviation of 2 device: 400ppm
// resulting maximum phase shift within a CAN frame of 150 bits: 0.06 * bit time
// Possible asymmetry of XTAL output signal is compensated to a great measure
// by frequency division of CAN controller (division ratio >= 2).
// Conclusion:
// XTAL tolerances of CAN IRMA devices are acceptable compared to TSEG2 = 0.25 * bit time.

// Refer to Infineon AP292201 "'C' CAN Driver Routines for the C166 Family", source file
// "INCAN16X.C".
// Setting SJW == 3 is used for all baud rates there. For baud rate 1Mbps CAN bit time consists
// of 10 time quanta and for all lower baud rates it consists of 20 time quanta. CPU frequency
// of 20MHz is used.
//
/*----- Baudrate Selection ---------------------------------------------------------------------*/
#define BTR0_VAL_0125kbps		0x7		// SJW = 0, BRP = 7
#define BTR0_VAL_0250kbps		0x3		// SJW = 0, BRP = 3	
#define BTR0_VAL_0500kbps		0x1		// SJW = 0, BRP = 1
#define BTR0_VAL_1000kbps		0x0		// SJW = 0, BRP = 0
#define BTR1_VAL_16MHz			0x14	// TSEG1 = 4, TSEG2 = 1,  8 time quanta per bit
#define BTR1_VAL_24MHz			0x27	// TSEG1 = 7, TSEG2 = 2, 12 time quanta per bit

/*----- Addresses and Offsets ------------------------------------------------------------------*/
#define CAN1_BASIC_ADDR			0xEF00	// start of CAN1 controller memory range
// Length of message objects of CAN1 controller of C167CR.
#define CAN_MSG_OBJ_LEN			16		// C compiler uses shift operation << 4 as simple and fast
										// replacement for multiplication with CAN_MSG_OBJ_LEN.
#ifdef GATEWAY
#define CAN_TRANSMIT_BUF_MSG_CNT	201		// 201 * 7 = 1407 (UIP v3.0: max 1406)
#define CAN_RECEIVE_BUF_MSG_CNT		201
#else
#define CAN_TRANSMIT_BUF_MSG_CNT	40
#define CAN_RECEIVE_BUF_MSG_CNT		128
#endif
// Usage of preprocessor macro "#define identifier expression":
// If "identifier" is found by preprocessor it is replaced by "expression". As a result
// brackets enclosing "expression" always have to be used to ensure intended evaluation
// of expression.
#define CAN_RECEIVE_BUF_LEN		(CAN_MSG_OBJ_LEN * CAN_RECEIVE_BUF_MSG_CNT)
#define ID_HH_OFFSET			0	
#define ID_HL_OFFSET			1
#define ID_LH_OFFSET			2
#define ID_LL_OFFSET			3
#define	LEN_OFFSET				4
#define	DATA_OFFSET				5
#define	STATUS_OFFSET 			13
#define	MO_STAT_OFFSET_L		14
#define	MO_STAT_OFFSET_H		15

#define CONTROL         		0x00	// CAN register control segment
#define STATUS					0x01
#define INT_REG   				0x02
#define BIT_TIMING0     		0x04
#define BIT_TIMING1     		0x05
#define GLOBAL_MASK1			0x06
#define GLOBAL_MASK2    		0x07
#define GLOBAL_MASK3    		0x08
#define GLOBAL_MASK4    		0x09
#define GLOBAL_MASK5    		0x0A
#define GLOBAL_MASK6    		0x0B
#define MSG15_MASK1     		0x0C
#define MSG15_MASK2     		0x0D
#define MSG15_MASK3     		0x0E
#define MSG15_MASK4     		0x0F
#define MSG_OFFSET      		0x10	// Message object distance
#define MSG_CTRL0    			0x00	// Message object structure
#define MSG_CTRL1    			0x01
#define MSG_ARBIT0      		0x02
#define MSG_ARBIT1      		0x03
#define MSG_ARBIT2      		0x04
#define MSG_ARBIT3      		0x05
#define MSG_CONFIG      		0x06
#define MSG_DATA        		0x07

/*----- Message Object Flags -------------------------------------------------------------------*/
// 01: flag is not set
// 10: flag is set
// FLAG_SET and WFLG_SET constants can be used as flag query masks.
#define FLAG_MSGVAL         	0xC0
#define FLAG_TXIE           	0x30
#define FLAG_RXIE           	0x0C
#define FLAG_INTPND         	0x03
#define FLAG_RMTPND         	0xC0
#define FLAG_TXRQ	         	0x30
#define FLAG_CPUUPD         	0x0C
#define FLAG_MSGLST         	0x0C
#define FLAG_NEWDAT         	0x03
#define FLAG_SET_MSGVAL     	0x80
#define FLAG_SET_TXIE       	0x20
#define FLAG_SET_RXIE       	0x08
#define FLAG_SET_INTPND     	0x02
#define FLAG_SET_RMTPND     	0x80
#define FLAG_SET_TXRQ	     	0x20
#define FLAG_SET_CPUUPD     	0x08
#define FLAG_SET_MSGLST     	0x08
#define FLAG_SET_NEWDAT     	0x02
#define FLAG_RESET_MSGVAL   	0x40
#define FLAG_RESET_TXIE     	0x10
#define FLAG_RESET_RXIE     	0x04
#define FLAG_RESET_INTPND   	0x01
#define FLAG_RESET_RMTPND   	0x40
#define FLAG_RESET_TXRQ		   	0x10
#define FLAG_RESET_CPUUPD   	0x04
#define FLAG_RESET_MSGLST   	0x04
#define FLAG_RESET_NEWDAT   	0x01

#define WFLG_RMTPND         	0xC000
#define WFLG_TXRQ	         	0x3000
#define WFLG_CPUUPD         	0x0C00
#define WFLG_MSGLST         	0x0C00
#define WFLG_NEWDAT         	0x0300
#define WFLG_SET_RMTPND     	0x8000
#define WFLG_SET_TXRQ	     	0x2000
#define WFLG_SET_CPUUPD     	0x0800
#define WFLG_SET_MSGLST     	0x0800
#define WFLG_SET_NEWDAT     	0x0200
#define WFLG_RESET_RMTPND   	0x4000
#define WFLG_RESET_TXRQ		   	0x1000
#define WFLG_RESET_CPUUPD   	0x0400
#define WFLG_RESET_MSGLST   	0x0400
#define WFLG_RESET_NEWDAT   	0x0100


/*----- Variable Type Definitions --------------------------------------------------------------*/
#if defined(CAN_SENSOR) || defined(GATEWAY)
typedef struct {
	byte id_hh;
	byte id_hl;
	byte id_lh;
	byte id_ll;
	byte length;
	byte data[MAX_CAN_DATA_LEN];
} can_transmit_buf_type;

typedef struct {
	byte high;
	byte mid;
	byte low;
} can_addr_id_type;
#endif


/*----- Publication of Global Variables --------------------------------------------------------*/
extern byte *canreg[256];  				// pointer to register array of CAN1 controller

extern byte can_receive_buf[];			// ring buffer for received CAN messages
extern word can_receive_buf_read;		// pointer to position where the next message is read from
extern word can_receive_buf_write; 		// index of position where the next message is written to
// Usage of global variables can_receive_buf_write_pag and can_receive_buf_write_pof enables
// minimization of time needed by interrupt service routine CANINT_handler for complete readout
// of CAN receive message objects. Variables are initialized by function init_can_interf and
// updated immediately after update of buffer pointer can_receive_buf_write by interrupt service
// routine CANINT_handler.
extern word can_receive_buf_write_pag;
extern word can_receive_buf_write_pof;

extern can_transmit_buf_type can_transmit_buf[CAN_TRANSMIT_BUF_MSG_CNT];
extern byte can_transmit_buf_read;
extern byte can_transmit_buf_write;

extern bool can_bus_off_state_detectedB;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void clear_can_receive_buffer(void);
void clear_can_transmit_buffer(void);

void begin_can1_init(void);
void end_can1_init(void);

void Disable_CAN_Interrupts(void);

bool can_busoff(void);
void start_can_busoff_recovery_sequence(void);


#define CAN_INC
#endif	// end of "#ifndef CAN_INC"
