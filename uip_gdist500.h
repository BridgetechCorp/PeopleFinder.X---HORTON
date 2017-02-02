/*==============================================================================================*
 |       Filename: uip_gdist500.h                                                               |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of Universal IRMA Protocol (UIP) for IRMA 5 gateway.          |
 |                 A21C hardware is used as IRMA 5 gateway.                                     |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef UIP_GDIST500_INC  				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "uip_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void Init_IRMA(void);

/*---------------------------------------------------------------------------------------- A ---*/
void Send_IRMA_A_v50(dword, byte, byte *);
boolean enable_can_transparent_mode_uip(void);
boolean disable_can_transparent_mode_uip(void);

/*---------------------------------------------------------------------------------------- C ---*/
void send_irma_C_v50_current_counter_state_set_resp_gateway(void);

/*---------------------------------------------------------------------------------------- D ---*/
void get_D_10_data_gateway(byte fa_idx, D_10_type *D_10_data);
void send_irma_D_v10_door_state_set_resp_gateway(void);

/*---------------------------------------------------------------------------------------- I ---*/
#ifdef J1708_PROTOCOL
byte irma_I_v20_gateway(byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- S ---*/
void Send_IRMA_S_v21(void);


#define UIP_GDIST500_INC
#endif	// end of "#ifndef UIP_GDIST500_INC"
