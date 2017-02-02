/*==============================================================================================*
 |       Filename: communication.h                                                              |
 | Project/Module: A21, GATEWAY or OBC/module group Communication                               |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: General functions for module group Communication.	  						|
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef COMMUNICATION_INC				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "communication_defines.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef boolean Door_Flags_Type[FUNCTION_AREAS_MAX];


/*----- Publication of Global Variables --------------------------------------------------------*/
extern Configuration_System_type *Comm_ConfigSystem;
extern Configuration_Data_type *Comm_ConfigData;

extern Device_Status_type DeviceStatus;

extern volatile Door_Flags_Type Door_Opened_FlagB;
extern volatile Door_Flags_Type Door_Closed_FlagB;
extern volatile word door_opened_event_cnt[FUNCTION_AREAS_MAX];
extern volatile word door_closed_event_cnt[FUNCTION_AREAS_MAX];
#ifdef J1708_PROTOCOL
extern volatile boolean Send_J1708_Msgs_On_Door_Status_ChangeB;
#endif

#ifdef GLORIA
extern boolean GLORIA_InitializedB;
	#ifdef DEVTEST
extern boolean GLORIA_BurnIn_ModeB;
	#endif
#endif

#ifdef SIMCA_MODE
extern boolean SimCa_ModeB;
extern byte Door_port_simca;
#endif


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void communication_init(void);

void clear_door_opened_flags(void);
void clear_door_closed_flags(void);

void init_device_status(void);

void No_Job_Communication(void);

void init_door_jobs(void);
boolean add_door_opened_job(void *ptr);
boolean add_door_closed_job(void *ptr);
boolean remove_door_opened_job(void *ptr);
boolean remove_door_closed_job(void *ptr);
void call_door_opened_jobs(byte door_idx);
void call_door_closed_jobs(byte door_idx);

boolean Door_Contacts_Ever_Changed(void);
void Update_Door_Status(byte fa_idx, boolean closedB);
void check_door_contacts(void);
void synch_with_door_contacts(void);
boolean Set_Door_Status(byte fa_idx, boolean closedB);
boolean All_Doors_Closed(void);
boolean Last_Door_Closed(void);
boolean First_Door_Opened(void);
void call_door_jobs(boolean SendFlagB);
void check_door_contacts_and_call_door_jobs(void);


#define COMMUNICATION_INC
#endif	// end of "#ifndef COMMUNICATION_INC"
