/*==============================================================================================*
 |       Filename: communication_gdist500.h                                                     |
 | Project/Module: GATEWAY/module group Communication                                           |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Main module of module group Communication for IRMA 5 gateway.                |
 |                 A21 hardware is used for IRMA 5 gateway.                                     |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef COMMUNICATION_GDIST500_INC		/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include "..\..\communication_defines.h"
#include "can_irma_dist500.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	bool statuses_synchronizedB;

	// Flags closed_old_flagsB are not valid on first call of function update_door_status_gateway
	// after start-up of gateway firmware (initial value FALSE). Flag statuses_synchronizedB initialized
	// with FALSE is evaluated by update_door_status_gateway to handle this fact.
	// Refer to comment regarding flags door_closedB and door_closed_oldB of function_area_data_obc_type
	// structure.
	bool closed_flagsB[DOOR_CNT_MAX];
	bool closed_old_flagsB[DOOR_CNT_MAX];

	bool positive_logicB[DOOR_CNT_MAX];

	word opened_delay[DOOR_CNT_MAX];
	word closed_delay[DOOR_CNT_MAX];
	bool delay_runningB[DOOR_CNT_MAX];
} door_signal_interface_type;

typedef struct
{
	// static
	dword addr;
	char prod_name[65];
	char dev_number[17];
	char dspf_version[17];

	byte fa_list_idx;
	
	bool static_data_validB;

	// dynamic
	bool commu_cycle_failedB;
	bool error_msg_receivedB;

	// used for persistent storage of failures
	bool ever_static_data_invalidB;
	bool ever_firmware_mismatchedB;
	bool ever_commu_cycle_failedB;
	bool ever_error_msg_receivedB;
	bool ever_invalid_config_paramsB;
	bool ever_slave_missingB;

	// non-persistent error flags
	bool sabotageB;

	// desired data
	bool door_closed_desired_activeB;
	bool door_closed_desired_confirmedB;
	bool counting_data_buffer_desired_activeB;
	bool counting_data_buffer_desired_confirmedB;
} sensor_data_obc_type;

typedef struct
{
	// static
	word address;
	word function;
	byte desired_sensor_cnt;
	byte actual_sensor_cnt;
	sensor_assignments_type sensor_assignments;
	byte door_signal_idx;
	byte counting_data_category_cnt;
	counting_data_category_params_type counting_data_category_param_array[COUNTING_DATA_CATEGORY_CNT_MAX];

	bool height_classification_supportedB;
	byte height_class_cnt;

	bool static_data_validB;
	
	// dynamic
	// Both door states door_closedB[] and door_closed_oldB[] are queried by one D 0x30 s command. As a
	// result flag door_closed_oldB[] is valid already on first call of function update_obc_interface_data
	// after start of CAN sensor communication. It is equal to initial door state of IRMA 5 sensor
	// not controlled by gateway (closed after power-on) in this case.
	// Refer to comment regarding flags closed_flagsB and closed_old_flagsB of door_signal_interface_type
	// structure.
	bool door_closedB;
	bool door_closed_oldB;
	bool first_call_of_fa_door_jobs_doneB;
	bool countingB;
	counting_data_type current_counter_states[COUNTING_DATA_CATEGORY_CNT_MAX];
	bool new_counting_result_availableB;
	counting_data_type last_counting_result[COUNTING_DATA_CATEGORY_CNT_MAX];
	fa_status_gateway_type fa_status;
	bool counting_finishedB;

	bool dynamic_data_validB;

	// used for query of counting result
	bool new_counting_result_bufferedB;
	counting_data_type counting_result_buffer[COUNTING_DATA_CATEGORY_CNT_MAX];
	fa_status_gateway_type fa_status_buffer;

	// used for persistent storage of failures
	bool ever_static_data_invalidB;
	bool ever_dynamic_data_invalidB;

	// used for test of counting data query
	bool test_modeB;
	bool new_counting_result_available_test_modeB;
	counting_data_buffer_type counting_data_buffer_test_mode;

	// desired data
	// Typically desired data are received by command messages like IRMA-D v1.0 and IRMA-C v5.0.
	bool door_closed_desiredB;
	bool door_closed_desired_set_by_msgB;
	bool door_closed_desired_activeB;
	bool door_closed_desired_confirmedB;

	counting_data_type current_counter_states_desired[COUNTING_DATA_CATEGORY_CNT_MAX];
	bool current_counter_states_desired_set_by_msgB;
	bool current_counter_states_desired_activeB;
	bool current_counter_states_desired_confirmedB;
} function_area_data_obc_type;

typedef struct
{
	byte sensor_cnt;
	sensor_data_obc_type sensor_list[CAN_SENSOR_LIST_LEN];

	byte function_area_cnt;
	function_area_data_obc_type function_area_list[CAN_FA_LIST_LEN];

	bool door_state_setting_permittedB;
	bool use_desired_door_statesB;

	// used for transmission of counting result by output switches
	word Persons_In_Counter_SW[2];
	word Persons_Out_Counter_SW[2];
} obc_interface_data_type;

typedef void (*obc_interface_data_update_func_type)(void);


/*----- Publication of Global Variables --------------------------------------------------------*/
extern volatile door_signal_interface_type door_signal_interface;

extern obc_interface_data_type obc_interface_data, obc_interface_data_backup;

extern bool obc_interface_data_fa_cnt_increasedB, obc_interface_data_fa_cnt_decreasedB;

extern obc_interface_data_update_func_type obc_interface_data_update_func;


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
int communication_main(void);

void InitDeviceStatusSensor(void);

bool all_doors_closed_gateway(void);
bool last_door_closed_gateway(void);
bool first_door_opened_gateway(void);
void set_door_logic(byte door_signal_idx, char logic);
void set_door_delays(byte door_signal_idx, word opened_delay, word closed_delay);
void init_timer_job_check_door_contacts_gateway(void);
void update_door_state_set_permission_flag_of_obc_interface_data(void);
bool set_door_status_by_msg_gateway(byte fa_idx, bool closedB);
void set_current_counter_state_by_msg_gateway(byte fa_idx, const counting_data_buffer_type *counting_data_buffer, bool counting_finishedB);
void update_desired_function_area_data(void);
void call_all_fa_door_jobs_gateway(void);

bool Add_Counting_Finished_Job(void  *ptr);
bool Remove_Counting_Finished_Job(void  *ptr);
void call_all_counting_finished_jobs_gateway(void);

//void clear_obc_interface_counting_data(byte fa_idx);
void clear_obc_interface_counting_result_buffer(byte fa_idx);

byte get_idx_of_function_area_in_obc_interface_data(word fa_addr);

void init_counting_data_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer);
void get_current_counter_states(byte fa_idx, counting_data_buffer_type *counting_data_buffer);
void get_total_counter_states(byte fa_idx, counting_data_buffer_type *counting_data_buffer);
void get_buffered_counting_result_and_fa_status_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer, byte *fa_status_byte);
bool query_buffered_counting_result_and_fa_status_buffer(byte fa_idx, counting_data_buffer_type *counting_data_buffer, byte *fa_status_byte);
void get_counting_data_category_sums(counting_data_buffer_type counting_data_buffer, word *in, word *out);
void Get_Persons_Counter_SW2(bool *person_in, bool *person_out);
void Get_Persons_Counter_SW4(bool *person_in, bool *person_out);

bool commu_cycle_for_function_area_without_error(byte fa_idx);
bool function_area_without_communication_error(byte fa_idx);
void update_fa_status_gateway_var(byte fa_idx, fa_status_gateway_type *fa_status_gateway);
void merge_fa_statuses_gateway(fa_status_gateway_type *fa_status_gateway, fa_status_gateway_type fa_status_gateway_2);
byte get_fa_status_byte_gateway(fa_status_gateway_type fa_status_gateway);
byte get_current_fa_status_byte_gateway(byte fa_idx);

void start_obc_interface_test_mode(byte fa_idx);
void stop_obc_interface_test_mode(byte fa_idx);

void deactivate_obc_interface(void);

void create_fa_conf_str_gateway(byte FACnt, char *FAConfStr);

#define COMMUNICATION_GDIST500_INC
#endif	// end of "#ifndef COMMUNICATION_GDIST500_INC"
