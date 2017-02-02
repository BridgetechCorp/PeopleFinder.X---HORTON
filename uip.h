/*==============================================================================================*
 |       Filename: uip.h                                                                        |
 | Project/Module: A21, GATEWAY or OBC/module group Communication                               |
 |           Date: 10/04/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of universal IRMA protocol (UIP).                             |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef UIP_INC							/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "uip_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern void (*IRMA_Init_Prot_Function)(void);
extern RS232_Parameter_type IRMA_Init_RS232;
extern byte IRMA_Init_Prot_Id;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/

/*---------------------------------------------------------------------------------------- B ---*/
byte IRMA_B(byte DataVer, byte PayloadLen, byte *Payload);
void IRMA_Message_BaudRate(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- D ---*/
void Get_D_10_Data(byte, D_10_type *);
void Write_D_10_Data_To_IRMASendBuff(byte FAIdx);
void Build_IRMA_D_v10_Payload(byte FAIdx);
byte irma_D_v10_door_state_setting(byte PayloadLen, byte *Payload);
#ifndef SERIAL_SENSOR
byte IRMA_D_v10_Resp(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_D_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
#endif
void Send_IRMA_Door_v10(byte, byte);
byte IRMA_D_v12_Resp(byte PayloadLen, byte *Payload);
byte IRMA_D_v12(byte SrcAddr, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- E ---*/
void irma_E_v10_communication_test(byte SrcAddr, byte PayloadLen, byte *Payload);
void irma_E_v10_logger_confirmation(byte SrcAddr, byte PayloadLen, byte *Payload);
void irma_E_v11_logger_confirmation(byte SrcAddr, byte PayloadLen, byte *Payload);
#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)
void IRMA_E_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_E_v11(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Error(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- I ---*/
byte IRMA_I_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_I_v11_Resp(byte PayloadLen, byte *Payload);
byte IRMA_I_v11(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_I_v12_Resp(byte PayloadLen, byte *Payload);
byte IRMA_I_v12(byte SrcAddr, byte PayloadLen, byte *Payload);
void Build_IRMA_I_v17_Payload(void);
byte IRMA_I_v17_Resp(byte PayloadLen, byte *Payload);
byte IRMA_I_v17(byte SrcAddr, byte PayloadLen, byte *Payload);
#ifdef IRMA_I_V40
void Build_IRMA_I_v40_Payload(void);
byte IRMA_I_v40(byte SrcAddr, byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- L ---*/
#ifdef GLORIA
void G_14_to_10(byte *Data);
#endif
byte IRMA_L_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_L_v20(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Logger(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- O ---*/
#if defined(SW4) || defined(SW2)
void Build_IRMA_O_v20_Payload(byte SwitchNo);
byte IRMA_O_v20_Resp(byte PayloadLen, byte *Payload);
byte IRMA_O_v20(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Port(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- S ---*/
void Build_IRMA_S_v20_Payload(void);
#if !defined(CAN_SENSOR) && !defined(SERIAL_SENSOR)
byte IRMA_S_v20(byte SrcAddr, byte PayloadLen, byte *Payload);
#endif


#define UIP_INC
#endif	// end of "#ifndef UIP_INC"
