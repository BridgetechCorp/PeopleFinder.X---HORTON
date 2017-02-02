/*==============================================================================================*
 |       Filename: can_irma_embedding.h                                                         |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions for CAN IRMA embedding of UIP v2.0 messages.                       |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CAN_IRMA_EMBEDDING_INC 			/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "..\..\kernel_defines.h"
#include "..\..\communication_defines.h"
#include "..\can_irma.h"     		  


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define CAN_SENSOR_LIST_LEN						(CANSENS_PER_ANA_MAX)


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/
extern can_addr_id_type sensor_addr_ids[CAN_SENSOR_LIST_LEN];

extern bool to_enable_can_transparent_modeB;
extern bool (*enable_can_transparent_mode_func)(void);

// Muzzle_ModeB==TRUE: No CAN bus transmission by A21 firmware if service is going on.
extern bool Muzzle_ModeB;

extern bool to_update_can_sensor_commu_idx_and_stepB;
extern bool to_delay_can_sensor_commuB;
extern bool to_continue_can_sensor_commuB;

extern byte old_runlevel_log;
#ifndef NO_LOGGER
extern byte new_runlevel_log;
#endif


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void init_global_var_can_irma_embedding(void);
void init_uip_30_receiving(void);
byte init_can_interf_gateway(void);

void send_can_irma_msg(byte*, byte, byte, byte, can_addr_id_type);
void send_can_irma_msg_set(byte*, bool, byte, byte, byte, can_addr_id_type);
void check_for_pending_can_irma_jobs(void);
void start_can_busoff_recovery_sequence(void);
void send_can_status_query_msgs(void);
void check_can_sensor_error_counter_response(void);

dword get_can_irma_addr_of_rec_uip_30_msg(void);
bool check_rec_uip_30_msg(byte service_lev, char msg_id, byte msg_ver, char sub_cmd, word *payload_len, byte **payload);

void Remove_CAN_IRMA_Jobs(void);

bool set_can_transparent_mode_enable_flag(void);
bool enable_can_transparent_mode(void);
bool disable_can_transparent_mode(void);

void send_uip_30_msg_over_can(byte rec_class, can_addr_id_type addr_id, byte *uip_30_ptr);
void route_uip_30_msg_to_can(byte *uip_30_ptr);


#define CAN_IRMA_EMBEDDING_INC
#endif	// end of "#ifndef CAN_IRMA_EMBEDDING_INC"
