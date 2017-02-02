/*==============================================================================================*
 |       Filename: can_irma_dist500.h                                                           |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Extended functions for CAN IRMA communication with DIST500 sensors.          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef CAN_IRMA_DIST500_INC 			/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "can_irma_embedding.h"     		  


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define FA_STATUS_LEN	 						4		// count of data bytes in function area
														// status messages, used for can status
														// queries

#define CAN_FA_LIST_LEN							(CAN_SENSOR_LIST_LEN)

#define COUNTING_DATA_CATEGORY_CNT_MAX			10

// Query of counting data category parameters (C 0x60 c query) addresses function area not sensor.
#define LAST_CAN_SENSOR_STATIC_DATA_QUERY_STEP	0x18	// C 0x60 c query not included
// Extended communication sequence is inserted into regular communication cycle on certain events,
// e.g. on availability of new counting result or on receiving of UIP 2.0 error message. Steps of all 
// extended communication sequences are placed after last regular communication step. After finishing
// extended communication sequence regular communication cycle is continued with next step following
// step on which extended communication event was raised.
#define LAST_CAN_SENSOR_REGULAR_COMMU_STEP		0x1D
#define LAST_CAN_SENSOR_COMMU_STEP				0x20


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	byte category_id;
	word in;
	word out;
} counting_data_type;

typedef struct
{
	byte counting_data_category_cnt;
	counting_data_type counting_data[COUNTING_DATA_CATEGORY_CNT_MAX];
} counting_data_buffer_type;

typedef struct
{
	// static
	dword addr;										// queried by c 0x10 b broadcast
	
	// Zero terminated strings: Additional byte has to be allocated.
	char prod_name[65];
	char dev_number[17];
	char prod_date[21];
	char comm_name[33];
	char comm_version[17];
	char comm_timestamp[21];
	char appl_name[33];
	char appl_version[17];
	char appl_timestamp[21];
	char fpga_name[33];
	char fpga_version[17];
	char fpga_timestamp[21];
	bool appl_activeB;
	char act_dspf_version[17];
	word act_fpga_version;
	byte sensor_cnt;
	byte uip_addr_self;
	byte uip_addr_master;
	byte uip_addr_gateway;
	byte function_area_cnt;
	word function_area_addrs[CAN_FA_LIST_LEN];
	word function_area_funcs[CAN_FA_LIST_LEN];
	byte door_cnt;
	word door_addrs[DOOR_CNT_MAX];
	word door_delays[DOOR_CNT_MAX];
	byte door_logics[DOOR_CNT_MAX];
	byte assignment_cnt;
	word fa_assignments[CAN_FA_LIST_LEN];
	word door_assignments[DOOR_CNT_MAX];

	byte fa_list_idx;
	
	bool static_data_knownB;
	bool static_data_validB;
	bool firmware_mismatchedB;
	
	// dynamic

	// used for communication control
	byte last_commu_step;
	bool new_counting_result_availableB;			// S 0x31 g query, master sensor only
	bool commu_cycle_failedB;
	bool error_msg_receivedB;
	bool to_init_can_sensor_dataB;

	// sensor status
	bool invalid_config_paramsB;					// S 0x30 g query
	bool sabotageB;									// S 0x30 g query
	bool slave_missingB;							// S 0x31 g query, master sensor only

	// desired data
	bool use_desired_door_stateB;
	bool door_closed_desiredB;
	bool door_closed_desired_activeB;
	bool door_closed_desired_confirmedB;
	counting_data_buffer_type counting_data_buffer_desired;
	bool counting_data_buffer_desired_activeB;
	bool counting_data_buffer_desired_confirmedB;
} can_sensor_data_type;

typedef struct
{
	byte sensor_list_indexes[CAN_SENSOR_LIST_LEN];
} sensor_assignments_type;

typedef struct
{
	byte category_id;
	byte description_no;
	word param_1;
	word param_2;
	word param_3;
	word param_4;
} counting_data_category_params_type;

typedef struct
{
	// static
	word address;
	word function;
	char dspf_version[17];
	byte desired_sensor_cnt;
	byte actual_sensor_cnt;
	sensor_assignments_type sensor_assignments;
	byte counting_data_category_cnt;
	counting_data_category_params_type counting_data_category_param_array[COUNTING_DATA_CATEGORY_CNT_MAX];
	bool counting_data_category_params_queriedB;
	byte door_list_idx;

	bool static_data_of_all_assigned_sensors_knownB;
	bool static_data_knownB;							// set true, if C 0x60 c query successful
	bool static_data_validB;
	
	// dynamic
	bool door_closedB;
	bool door_closed_oldB;
	bool current_door_states_validB;

	counting_data_type current_counter_states[COUNTING_DATA_CATEGORY_CNT_MAX];
	bool current_counter_states_validB;

	bool new_counting_result_availableB;
	counting_data_type last_counting_result[COUNTING_DATA_CATEGORY_CNT_MAX];
	bool last_counting_result_validB;

	bool dynamic_data_validB;
} function_area_data_type;

typedef struct
{
	// static
	word signal_no;
	word delay;
	char logic;
} door_data_type;

typedef struct
{
	bool initialization_errorB;
	bool runtime_errorB;
	bool firmware_mismatchB;
	bool sabotageB;
} fa_status_gateway_type;
  

/*----- Publication of Global Variables --------------------------------------------------------*/
extern dword can_sensor_commu_cycle_cnt;
extern dword can_sensor_commu_cycle_error_cnt;
extern bool first_can_sensor_commu_cycle_startedB;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void init_global_var_can_irma_dist500(void);

void send_can_irma_startup_msg(void);

void start_can_sensor_commu(void);
void handle_uip_30_receive_error(void);
void eval_can_sensor_response(void);
void continue_can_sensor_communication(void);
void update_can_sensor_commu_idx_and_step(void);

void update_obc_interface_data_backup(void);

void clear_persistent_error_flags_of_function_area_and_assigned_sensors(byte fa_idx);
void clear_persistent_error_flags_of_all_function_areas_and_assigned_sensors(void);

bool disable_can_transparent_mode_dist500(void);


#define CAN_IRMA_DIST500_INC
#endif	// end of "#ifndef CAN_IRMA_DIST500_INC"
