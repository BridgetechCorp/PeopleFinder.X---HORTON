/*==============================================================================================*
 |       Filename: time.h                                                                       |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to control timer and time-out jobs launched by periodic T1         |
 |                 interrupt (period 1m). Functions to support RTC in NVRAM incl. clock jobs.   |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef TIME_INC						/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define TIMER_JOBS_MAX		20
#define TIMEOUT_JOBS_MAX	20
#define T01CON_VAL			0x0000

#ifndef NO_LOGGER
	#define CLOCK_JOBS_MAX	10
	#ifndef A21CL
		#define RTC_READ	0x40
		#define RTC_WRITE	0x80
		#define RTC_RUN		0x00
	#endif
#endif


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
#ifndef NO_LOGGER
void Init_RTC(void);
void init_clock_jobs(void);
#endif

void init_global_var_time(void);
void init_t1_for_timer_and_timeout_jobs(void);

word StartTimeOutIntv(word TO_ms);
bool TimeOutIntvElapsed(void);
void CancelTimeOutIntv(void);


#define TIME_INC
#endif	// end of "#ifndef TIME_INC"
