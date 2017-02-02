/*==============================================================================================*
 |       Filename: can_irma.h                                                                   |
 | Project/Module: A21 or GATEWAY/module group Communication                                    |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Basic functions for CAN IRMA communication.                                  |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CAN_IRMA_INC					/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "..\kernel_defines.h"
#include "can.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/

/*----- Message Object Numbers -----------------------------------------------------------------*/
// 1 receive message object for messages addressed by IRMA-A21-Windows to certain IRMA Analyzer
// CAN IRMA message identifier, bits 28-22: any
// CAN IRMA receiver class,     bits 21-19: certain IRMA Analyzer
// CAN IRMA device address,     bits 18- 0: IRMA Analyzer address
#define RECIW_MSG   			1
// 8 receive message objects for messages addressed by IRMA Sensors to certain IRMA Analyzer
// CAN IRMA message identifier, bits 28-22: any
// CAN IRMA receiver class,     bits 21-19: certain IRMA Analyzer
// CAN IRMA device address,     bits 18- 0: IRMA Sensor address
#define RECS1_MSG     			2
#define RECS2_MSG    			3
#define RECS3_MSG    			4
#define RECS4_MSG    			5
#define RECS5_MSG    			6
#define RECS6_MSG    			7
#define RECS7_MSG    			8
#define RECS8_MSG    			9
// 1 transmit message object for all messages
// CAN IRMA message identifier, bits 28-22: any
// CAN IRMA receiver class,     bits 21-19: any
// CAN IRMA device address,     bits 18- 0: any
#define TRA_MSG     			14
// 1 receive message object for receive messages not associated with any other receive
// message object
// CAN IRMA message identifier, bits 28-22: any
// CAN IRMA receiver class,     bits 21-19: any
// CAN IRMA device address,     bits 18- 0: any
#define MSG_15	   				15

/*----- IRMA CAN Receiver Classes --------------------------------------------------------------*/
//(Bits 21-19 of CAN Message Identifier)
#define IRMA_PC					0
#define	SENSOR					1
#define	ALL_SENSORS				5
#define	ANALYZER				2
#define	ALL_ANALYZERS			6
#define	ALL_ANAS_AND_SERV_DEV	4
#define	TO_ALL					7

/*----- IRMA CAN Message Identifiers -----------------------------------------------------------*/
//(Bits 28-22 of CAN Message Identifier)
#define	REGISTRATION_ASK		0x00U
#define	REGISTRATION_ANS		0x40U
#define CONFIG_ASK				0x01U
#define CONFIG_ANS1				0x41U
#define CONFIG_ANS2				0x42U
#define CONFIG_ANS3				0x43U
#define SIMCA					0x06U
#define SEN_ERR_CNT				0x08U
#define SOFTWARE_RESET			0x10U
#define ERROR_MSG				0x25U
#define ANA_STARTUP_NOT			0x1EU	// IRMA Analyzer startup notification
#define SEN_STARTUP_NOT			0x20U	// IRMA Sensor startup notification
#define MUZZLE_MODE_SET			0x11U	// muzzle mode: set command (both active or inactive)
#define MUZZLE_MODE_CFM			0x12U	// muzzle mode:	confirmation
#define CONFIG_AS_DATA			0x15U	// basic configuration of IRMA Sensor: set command
#define CONFIG_AS_RECEIPT		0x16U	// basic configuration of IRMA Sensor: confirmation
#define SEN_EXTCFG_SET			0x17U	// extended configuration of IRMA Sensor: set command
#define SEN_EXTCFG_CFM			0x18U	// extended configuration of IRMA Sensor: confirmation
#define SEN_DICD_QUERY			0x19U	// device immanent configuration data of IRMA Sensor: query
#define SEN_DICD_RESULT			0x1AU	// device immanent configuration data of IRMA Sensor: result
#define SEN_EEP_SET_RANGE		0x68U	// access to EEPROM of IRMA Sensor: set memory range
#define SEN_EEP_CFM_RANGE		0x69U	// access to EEPROM of IRMA Sensor: confirm memory range
#define SEN_EEP_READ_CMD		0x64U	// access to EEPROM of IRMA Sensor: read command
#define SEN_EEP_READ_RES		0x65U	// access to EEPROM of IRMA Sensor: read result
#define APPLICATION_RUN			0x50U
#define SEND_SIGNALS_TO_AS		0x09U   // sensor signals, 8-bit
#define FA_STATUS_SEND			0x49U	// function area status: send periodically
#define SIGN_16_DATA			0x0AU	// sensor signals, 16-bit
#define SIGN_REQ_SEND			0x4AU	// sensor signal request: send periodically
#define SIGN_16_CXYO_DATA		0x0BU	// sensor signals, 16-bit, Cxyo
#define SIGN_CXYO_REQ_SEND		0x4BU	// sensor signal request Cxyo
#define COUNT_RES_SEND			0x7FU	// counting results: send on change
#define SEN_PHYADDR_SET			0x0FU	// switch IRMA Sensor address mode: logical => pyhsical
#define SEN_PHYADDR_CFM			0x1FU	// switch IRMA Sensor address mode: logical => pyhsical
#define UIPOCI_QUERY			0x70U	// UIP 1.0 over IRMA CAN, embedded UIP 1.0 segment, query
#define UIPOCI_RESP				0x71U	// UIP 1.0 over IRMA CAN, embedded UIP 1.0 segment, response
#define UIP_20_SEND				0x74U	// UIP 2.0 over IRMA CAN, sending
#define UIP_20_RECV				0x75U	// UIP 2.0 over IRMA CAN, receiving


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern can_addr_id_type analyzer_addr_id;


/*----- Publication of Global Constants --------------------------------------------------------*/
extern const can_addr_id_type service_dev_addr_id;


/*----- Publication of Function Prototypes -----------------------------------------------------*/
can_addr_id_type addr_to_can_addr_id(dword addr);
dword can_addr_id_to_addr(can_addr_id_type can_addr_id);
void determine_analyzer_addr_id(void);

void conf_can_transmit_msg_obj(void);
void conf_can_receive_msg_objs(void);

void can_transmit(void);
void can_transmit_buf_write_update(void);


#define CAN_IRMA_INC
#endif	// end of "#ifndef CAN_IRMA_INC"
