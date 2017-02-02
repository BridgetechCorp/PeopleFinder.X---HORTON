/*==============================================================================================*
 |       Filename: time.c                                                                       |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to control timer and time-out jobs launched by periodic T1         |
 |                 interrupt (period 1ms). Functions to support RTC in NVRAM incl. clock jobs.  |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include "..\interrupt_defines.h"
#include "..\kernel_interface.h"
#include "..\Configuration\configuration.h"
#if defined(A21CL) && !defined(NO_LOGGER)
	#include "serio_ssc.h"
#endif
#include "time.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class CO=KERNEL
#pragma class PR=KERNEL
#pragma class FC=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
// Pragma volatile dispensable for list variables timer_jobs and timeout_jobs because access to
// these lists outside T1INT_handler has to be performed while CAPCOM timer T1 interrupt is
// disabled to ensure list consistency.
timer_job_type timer_jobs[TIMER_JOBS_MAX];
timeout_job_type timeout_jobs[TIMEOUT_JOBS_MAX];
volatile bool timer_and_timeout_jobs_disabledB;
volatile byte timer_and_timeout_job_disabling_counter;
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile bool timeout_occuredB;					// Changed in ISR: Predicate volatile necessary.
volatile bool boTimeOutIntvElapsed;				// Changed in ISR: Predicate volatile necessary.

#ifndef NO_LOGGER
dword System_Ticks;
Clock_Job_type clock_jobs[CLOCK_JOBS_MAX]; 
	#ifndef A21CL
// Refer to comments regarding pragma volatile in source file "serio_asc0.c".
volatile RTC_type _huge *RTC;					// Pointer to real time clock of timekeeping RAM
	#endif
#endif


/*----- Function Prototypes --------------------------------------------------------------------*/
void init_timer_jobs(void);
void init_timeout_jobs(void);

#ifndef NO_LOGGER
void System_Tick(void);
void check_clock_jobs(void);
void update_clock_jobs(Time_type *, Time_type *);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/

// There are 2 approaches to use timer and time-out jobs:
// - 1st approach:
//	 Flag(s) is(are) changed by timer or time-out job.
//   Flag(s) is(are) evaluated by run level job.
// - 2nd approach:
//	 Global data are changed by timer or time-out job.
//   Evaluation or changing of these global data by functions running at ILVL == 0
//   (run level jobs, counting finished jobs, functions of IRMA Application module,
//   communication_RL4, communication_main, ...) has to be done while CAPCOM timer
//   T1 interrupt is disabled to ensure data consistency. Time of T1 interrupt
//   disabling has to be lower than 1ms to avoid disturbance of update of counters
//   timer_jobs[].counter and timeout_jobs[].counter.

void init_t1_for_timer_and_timeout_jobs(void)
{
	word rel;

	init_timer_jobs();
	init_timeout_jobs();
	// Calculate T1 tick count for specific fCPU. Inherent prescaler
	// factor 8 and conversion s->ms are considered by division by 8000.
	rel = ConfigSystem.fCPU / 8000;
	Save_ILVL_and_Disable_All_INTs();
	// T1 disabled (T1R = 0) and configured as Timer (T1M = 0) with 
	// fCPU prescaler factor 8 (T1I = 000b).
	// fCPU = 16.000MHz (A21C, A21CL):	T1 tick = 0.5µs,	rel == 2000
	// fCPU = 18.432MHz (A21S):			T1 tick = 0.434µs,	rel == 2304
	// T0 unaffected.
	_bfld(T01CON, 0xFF00, T01CON_VAL);
	// Calculate T1 reload value from T1 tick count:
	// T1 reload value = 0xFFFF - T1 tick count + 1
	rel = -rel;
	// Flag T1IR will be set if timer T1 overflows from 0xFFFF to 0x0000.
	// If overflowed T1 will be reloaded with T1REL.
	T1 = rel;
	T1REL = rel;
	T1R = TRUE;					// Start timer T1. Overflow interval 1ms.
	T1IE = ENABLE;				// 
	Restore_ILVL();
}	// init_t1_for_timer_and_timeout_jobs


void init_timer_jobs(void)
{
	byte n;

	for( n = 0; n < TIMER_JOBS_MAX; n++ )
		timer_jobs[n].function = NULL;
}	// init_timer_jobs


// Lists timer_jobs[] and timeout_jobs[] are referenced in interrupt service routine T1INT_handler.
// Furthermore functions to change these lists are called by T1INT_handler. 
// Complete procedure from index search to last assignment to list must n o t be
// interrupted by T1INT_handler to avoid inconsistencies.

// Typical implementation to perform actions triggered by timer job:
// Flag to_perform_actionB is set by timer job.
// Flag to_perform_actionB is queried by run level job:
// if( to_perform_actionB )
// {
//    perform_action();
//    to_perform_actionB = FALSE;
// }
// T1 interrupt has to be disabled during running of program sequence of if then branch,
// if delay until call of perform_action is at least comparable to timer job interval.
// Otherwise immediate reset of flag to_perform_actionB cannot be ruled out.

// Add new timer job to list timer_jobs[] or update timer job already listed and restarted
// corresponding timer interval.
word add_timer_job(word ms, void huge *ptr)
{
	void huge *func_ptr;
	byte n = 0;

	disable_t1_interrupt();
	while( (func_ptr = timer_jobs[n].function) != NULL && func_ptr != ptr )
		if( ++n >= TIMER_JOBS_MAX )
		{
			enable_t1_interrupt();	
			return(0);
		}

	// If certain timer job is already stored in list timer_jobs[] value of counter_end is
	// restored or new value is assigned. Furthermore value counter is resetted. As a result
	// timer interval is restarted.
	timer_jobs[n].function    = ptr;
	timer_jobs[n].counter_end = ms;
	timer_jobs[n].counter     = 0;
	enable_t1_interrupt();	
	return(ms);
}	// add_timer_job


bool remove_timer_job(void huge *ptr)
{
	void huge *func_ptr;
	byte n = 0;
	byte m;

	disable_t1_interrupt();
	while( (func_ptr = timer_jobs[n].function) != ptr )
		if( func_ptr == NULL || ++n >= TIMER_JOBS_MAX )
		{
			enable_t1_interrupt();
			return(FALSE);
		}

	// Restore coherence of list if lost by job removal:
	for( m = n; m < TIMER_JOBS_MAX - 1; m++ )
	{
		if( timer_jobs[m + 1].function == NULL )
			break;
		timer_jobs[m].function    = timer_jobs[m + 1].function;
		timer_jobs[m].counter_end = timer_jobs[m + 1].counter_end;
		timer_jobs[m].counter     = timer_jobs[m + 1].counter;
	}
	timer_jobs[m].function = NULL;
	enable_t1_interrupt();
	return(TRUE);
}	// remove_timer_job


void init_timeout_jobs(void)
{
	byte n;

	for( n = 0; n < TIMEOUT_JOBS_MAX; n++ )
		timeout_jobs[n].function = NULL;
}	// init_timeout_jobs


// Add new time-out job to list timeout_jobs[] or update time-out job already listed and restarted
// corresponding time-out interval.
word add_timeout_job(word ms, void huge *ptr)
{
	void (*call_function)(void) = ptr;
	void huge *func_ptr;
	byte n = 0;

	if( ms == 0 )
	{
		call_function();
		return(0);
	}

	// Limitation ms <= 60000 ?

	// Take synchronization uncertainty into account.
	if( ms != 0xFFFF )
		ms++;

	disable_t1_interrupt();
	while( (func_ptr = timeout_jobs[n].function) != NULL && func_ptr != ptr )
		if( ++n >= TIMEOUT_JOBS_MAX )
		{
			enable_t1_interrupt();	
			return(0);
		}

	// If certain time-out job is already stored in list timeout_jobs[] value of counter is
	// restored or new value is assigned. As a result time-out interval is restarted.
	timeout_jobs[n].function       = ptr;
	timeout_jobs[n].counter_reload = ms;
	timeout_jobs[n].counter        = 0;
	enable_t1_interrupt();	
	return(ms);
}	// add_timeout_job


// Time-out interval of time-out job stored in list timeout_jobs[] is restarted.
word reload_timeout_job(void huge *ptr)
{
	void huge *func_ptr;
	byte n = 0;

	disable_t1_interrupt();
	while( (func_ptr = timeout_jobs[n].function) != ptr )
		if( func_ptr == NULL || ++n >= TIMEOUT_JOBS_MAX )
		{
			enable_t1_interrupt();	
			return(0);
		}

	timeout_jobs[n].counter = 0;
	enable_t1_interrupt();
	return(timeout_jobs[n].counter_reload);
}	// reload_timeout_job


// Any time-out job is removed automatically after calling. Do not remove time-out job
// by call "remove_timeout_job(SELF);" placed in body of respective time-out job.
bool remove_timeout_job(void huge *ptr)
{
	void huge *func_ptr;
	byte n = 0;
	byte m;

	disable_t1_interrupt();
	while( (func_ptr = timeout_jobs[n].function) != ptr )
		if( func_ptr == NULL || ++n >= TIMEOUT_JOBS_MAX )
		{
			enable_t1_interrupt();	
			return(FALSE);
		}

	// Restore coherence of list if lost by job removal:
	for( m = n; m < TIMEOUT_JOBS_MAX - 1; m++ )
	{
		if( timeout_jobs[m + 1].function == NULL )
			break;
		timeout_jobs[m].function       = timeout_jobs[m + 1].function;
		timeout_jobs[m].counter_reload = timeout_jobs[m + 1].counter_reload;
		timeout_jobs[m].counter        = timeout_jobs[m + 1].counter;
	}
	timeout_jobs[m].function = NULL;
	enable_t1_interrupt();
	return(TRUE);
}	// remove_timeout_job


bool Get_Timeout_Flag(void)
{
	return(timeout_occuredB);
}	// Get_Timeout_Flag


void Def_Timeout_Job(void)
{
	timeout_occuredB = TRUE;
}	// Def_Timeout_Job


word Add_Def_Timeout_Job(word ms)
{
	timeout_occuredB = FALSE;
	return( add_timeout_job(ms, Def_Timeout_Job) );
}	// Add_Def_Timeout_Job


bool Remove_Def_Timeout_Job(void)
{
	return( remove_timeout_job(Def_Timeout_Job) );
}	// Remove_Def_Timeout_Job


#ifndef NO_LOGGER
void Init_RTC(void)
{
	#ifndef A21CL
	switch(ConfigSystem.RTC_id)
	{
		case   1 :	RTC = (RTC_type *)0x0C7FF8;		/* DS1644    32 kB */
					break;
		case   2 :	RTC = (RTC_type *)0x0DFFF8;		/* M48T128Y 128 kB */
					break;
	}
	#endif

	if( A21_Status.Optional_Hardware.RTC_ErrorB )
	{
		System_Ticks = 0;
		add_timer_job(1000, System_Tick);
	}

	return;
}


void Set_RTC_Sync_Status(bool new_status)
{
	A21_Status.Optional_Hardware.RTC_SynchronizedB = new_status;
}


bool Get_RTC_Sync_Status(void)
{
	return(A21_Status.Optional_Hardware.RTC_SynchronizedB);
}


void System_Tick(void)
{
	System_Ticks++;
}


int Get_Date_Time(Date_type *Date, Time_type *Time)
{
	byte i, BCD, Date_Time[7];

	if( A21_Status.Optional_Hardware.RTC_ErrorB )
	{
		Date->Year    = 0xFF;
		Date->Month   = 0xFF;
		Date->Day     = (byte)(System_Ticks >> 24);
		Time->Hour	  = (byte)(System_Ticks >> 16);
		Time->Minutes = (byte)(System_Ticks >> 8);
		Time->Seconds = (byte)System_Ticks;

		return(EOF);
	}

	#ifdef A21CL
	DS1305_Date_Time_Read(Date_Time);
	#else
	// To avoid interference by call of "Get_Date_Time" or "Set_Date_Time" within timer or time-out job.
	disable_t1_interrupt();
	RTC->Control = RTC_READ;
	Date_Time[6] = RTC->Year;
	Date_Time[5] = RTC->Month & 0x1F;
	Date_Time[4] = RTC->Day & 0x3F;
	Date_Time[2] = RTC->Hour & 0x3F;
	Date_Time[1] = RTC->Minutes & 0x7F;
	Date_Time[0] = RTC->Seconds & 0x7F;
	RTC->Control = RTC_RUN;
	enable_t1_interrupt();
	#endif

	/* BCD to Binary Conversion */
	for( i = 0; i < 7; i++ )
	{
		if( i != 3 )
		{
			BCD = Date_Time[i];
			Date_Time[i] = 10 * (BCD >> 4) + (BCD & 0x0F);
		}
	}

	Date->Year    = Date_Time[6];
	Date->Month   = Date_Time[5];
	Date->Day     = Date_Time[4];
	Time->Hour	  = Date_Time[2];
	Time->Minutes = Date_Time[1];
	Time->Seconds = Date_Time[0];

	return(0);
}	// Get_Date_Time


bool Set_Date_Time(Date_type *Old_Date, Time_type *Old_Time, Date_type *New_Date, Time_type *New_Time)
{
	byte i, Nib_High, Nib_Low, Date_Time[7];

	if( A21_Status.Optional_Hardware.RTC_ErrorB || New_Date->Year > 99 ||
	   New_Date->Month == 0 || New_Date->Month   > 12 ||
	   New_Date->Day   == 0 || New_Date->Day     > 31 ||
	   New_Time->Hour  > 23 || New_Time->Minutes > 59 || New_Time->Seconds > 59 )
	{
		return(FALSE);
	}

	Get_Date_Time(Old_Date, Old_Time);

	Date_Time[6] = New_Date->Year;
	Date_Time[5] = New_Date->Month;
	Date_Time[4] = New_Date->Day;
	Date_Time[2] = New_Time->Hour;
	Date_Time[1] = New_Time->Minutes;
	Date_Time[0] = New_Time->Seconds;

	/* Binary to BCD Conversion */
	for( i = 0; i < 7 ; i++ )
	{
		if( i != 3 )
		{
			Nib_High = Date_Time[i] / 10;
			Nib_Low  = Date_Time[i] % 10;
			Date_Time[i] = (Nib_High << 4) + Nib_Low;
		}
	}

	Save_ILVL_and_Disable_T1_and_lower_priority_INTs();
	/* To avoid interference by:

	- Call of "Get_Date_Time" or "Set_Date_Time" within timer or time-out job while writing to struct "RTC"
	  (necessary only if macro A21CL is not defined).

	- Call of "check_clock_jobs" before execution of "update_clock_jobs"
	  (necessary in both cases, with or without A21CL).

	Usage of inline function "Save_ILVL_and_Disable_T1_and_lower_priority_INTs" is necessary,
	"disable_t1_interrupt" can`t be used here because there is a call to "enable_t1_interrupt"
	within function "DS1305_Date_Time_Write" (Nesting not allowed according to explanation placed in file
	"kernel_interface.h"). */

	#ifdef A21CL
	DS1305_Date_Time_Write(Date_Time);
	#else
	RTC->Control = RTC_WRITE;
	RTC->Year    = Date_Time[6];
	RTC->Month   = Date_Time[5];
	RTC->Day     = Date_Time[4];
	RTC->Hour    = Date_Time[2];
	RTC->Minutes = Date_Time[1];
	RTC->Seconds = Date_Time[0] & 0x7F;	// Set seconds and start oscillator if not running
	RTC->Control = RTC_RUN;
	#endif

	update_clock_jobs(Old_Time, New_Time);
	Restore_ILVL();
	return(TRUE);
}	// Set_Date_Time


void init_clock_jobs(void)
{
	byte i;

	for( i = 0; i < CLOCK_JOBS_MAX; i++ )
		clock_jobs[i].function = NULL;

	if( !A21_Status.Optional_Hardware.RTC_ErrorB )
		add_timer_job(60000, check_clock_jobs);
}


void add_clock_job(word Interval, void huge *Ptr)
{
	Date_type Current_Date;
	Time_type Current_Time;
	void huge *Func_Ptr;
	word Min_Next;
	byte i;

	if( A21_Status.Optional_Hardware.RTC_ErrorB )
		return;

	if( Interval > 1440 )
		Interval = 1440;	// Maximum allowed period is once a day

	Save_ILVL_and_Disable_T1_and_lower_priority_INTs();
	/* There is a call to "enable_t1_interrupt" within "Get_Date_Time" */

	i = 0;
	while( (Func_Ptr = clock_jobs[i].function) != NULL && Func_Ptr != Ptr )
	{
		if( ++i >= CLOCK_JOBS_MAX )
		{
			Restore_ILVL();
			return;
		}
	}

	Get_Date_Time(&Current_Date, &Current_Time);
	
	Min_Next = (Current_Time.Hour * 60) + Current_Time.Minutes + Interval;
	if( Min_Next < 1440 )
	{
		clock_jobs[i].next.tomorrow = FALSE;	// The job will be executed today
	}
	else
	{
		Min_Next -= 1440;
		clock_jobs[i].next.tomorrow = TRUE;	// The job will be executed tomorrow (lazy software!)
	}

	clock_jobs[i].function     = Ptr;
	clock_jobs[i].interval     = Interval;
	clock_jobs[i].next.minutes = Min_Next;

	Restore_ILVL();
}	// add_clock_job


void remove_clock_job(void huge *Ptr)
{
	void huge *Func_Ptr;
	byte i, j;

	disable_t1_interrupt();

	i = 0;
	while( (Func_Ptr = clock_jobs[i].function) != Ptr )
	{
		if( Func_Ptr == NULL || ++i >= CLOCK_JOBS_MAX )
		{
			enable_t1_interrupt();	
			return;
		}
	}

	for( j = i; j < CLOCK_JOBS_MAX - 1; j++ )
	{
		if( clock_jobs[j + 1].function == NULL )
			break;

		clock_jobs[j].function      = clock_jobs[j + 1].function;
		clock_jobs[j].interval      = clock_jobs[j + 1].interval;
		clock_jobs[j].next.minutes  = clock_jobs[j + 1].next.minutes;
		clock_jobs[j].next.tomorrow = clock_jobs[j + 1].next.tomorrow;
	}
	clock_jobs[j].function = NULL;

	enable_t1_interrupt();	
}	// remove_clock_job


void check_clock_jobs(void)
{
	word Min_Next, Total_Minutes_Today, Interval;
	Date_type Current_Date;
	Time_type Current_Time;
	byte i;
	
	Get_Date_Time(&Current_Date, &Current_Time);
	Total_Minutes_Today = Current_Time.Hour * 60 + Current_Time.Minutes;

	for( i = 0; clock_jobs[i].function != NULL && i < CLOCK_JOBS_MAX; i++ )
	{
		if( clock_jobs[i].next.tomorrow )
		{
			if( Current_Time.Hour > 0 )
				continue;
			else
				clock_jobs[i].next.tomorrow = FALSE;	// It's a new day, wake up!
		}

		if( Total_Minutes_Today >= clock_jobs[i].next.minutes )
		{
			Interval = clock_jobs[i].interval;
			Min_Next = clock_jobs[i].next.minutes + Interval;
			if( Min_Next <= Total_Minutes_Today )		// Next time must be in the future !!!
				Min_Next = Total_Minutes_Today + Interval;

			if( Min_Next < 1440 )
			{
				clock_jobs[i].next.tomorrow = FALSE;	// The next time for this job will be still today
			}
			else
			{
				Min_Next -= 1440;
				clock_jobs[i].next.tomorrow = TRUE;	// The next time for this job will be tomorrow
			}
			clock_jobs[i].next.minutes = Min_Next;
			clock_jobs[i].function();				// Execute the Job
		}
	}
}	// check_clock_jobs


void update_clock_jobs(Time_type *Old_Time, Time_type *New_Time)
{
	word Old_Minutes, New_Minutes, Remaining_Minutes, Min_Next;
	byte i;

	Old_Minutes = (Old_Time->Hour * 60) + Old_Time->Minutes;
	New_Minutes = (New_Time->Hour * 60) + New_Time->Minutes;

	for( i = 0; clock_jobs[i].function != NULL && i < CLOCK_JOBS_MAX; i++ )
	{
		if( !clock_jobs[i].next.tomorrow )
		{
			if( clock_jobs[i].next.minutes > Old_Minutes )
				Remaining_Minutes = clock_jobs[i].next.minutes - Old_Minutes - 1;
			else
				Remaining_Minutes = 0;
		}
		else
		{
			Remaining_Minutes = 1440 + clock_jobs[i].next.minutes - Old_Minutes - 1;
			clock_jobs[i].next.tomorrow = FALSE;
		}

		Min_Next = New_Minutes + Remaining_Minutes;

		/* To avoid possible malfunction when the time has been set to a new value "23:59:xx" and the current
		Clock Job would have to be executed within the current minute. In this case if the next call to
		function "Check_Clock_Job" is done after the day change, the next execution of the Clock Job would
		be done exactly one day after the expected time! (at "23:59:xx" of the next day) */
		if( Min_Next == 1439 && New_Minutes == 1439 ) 
			Min_Next++;
		
		if( Min_Next < 1440 )
		{
			clock_jobs[i].next.tomorrow = FALSE;	// The job will be executed today
		}
		else
		{
			Min_Next -= 1440;
			clock_jobs[i].next.tomorrow = TRUE;	// The job will be executed tomorrow
		}

		clock_jobs[i].next.minutes = Min_Next;
	}
}	// update_clock_jobs
#endif


//|-------------------------------------------------------------------------------------|
//| Function: T1INT_handler						 										|
//|-------------------------------------------------------------------------------------|
//| Description:																		|
//| Interrupt service routine (ISR) for CAPCOM timer 1.									|
//| Upon entry into T1INT interrupt service routine bit T1IR is cleared automatically.	|
//| Refer to page 5-7 of Infineon C167CR Derivatives User´s Manual, V3.2.				|
//| Default register bank 0 is replaced by register bank TIME_RB for ISR runtime.		|
//| Register banks 1 ... 3 are assigned to ISR register banks in the order as source 	|
//| files are listed in Tasking EDE project file.										|
//| Typically register bank 1 is used as TIME_RB.										|
//| Counters of all occupied positions of list timer_jobs[] are incremented.				|
//| Timer jobs listed in timer_jobs[] are called if corresponding counter has reached	|
//| end value. Corresponding counter is resetted before call of time job.				|
//| Counters of all occupied positions of list timeout_jobs[] are decremented. 			|
//| Time-out jobs listed in timeout_jobs[] are called if corresponding counter has		|
//| reached 0. After calling time-out job is removed from list automatically.			|
//|-------------------------------------------------------------------------------------|
interrupt (T1INT) void T1INT_handler(void)
{
	void huge *func_ptr;
	void (*call_function)(void);
	byte n = 0;

	while( (func_ptr = timer_jobs[n].function) != NULL && n < TIMER_JOBS_MAX )
	{
		if( timer_jobs[n].counter >= timer_jobs[n].counter_end )
		{
			if( !timer_and_timeout_jobs_disabledB )
			{
				timer_jobs[n].counter = 0;
				call_function = func_ptr;
				// - Call of add_timer_job from timer job:
				//   First increment of counter is made already before leaving T1INT_handler.
				// - Call of remove_timer_job from timer job:
				//   Counter increment is skipped for timer job(s) having list index higher
				//   than n before job removal and equal to or lower than n after job removal.
				//   Following conditions have to apply to create this situation.
				//   * Timer job(s) with index between 0 and n was(were) removed from list
				//     timer_jobs[].
				//   * At least one job with index higher than n was stored in list timer_jobs[]
				//     before job removal.
				//   As a result of skipping of counter increment call of affected timer job(s)
				//   may be delayed by 1ms.
				call_function();
			}
	 	}
		else
			timer_jobs[n].counter++;

		n++;
	}

	n = 0;
	while( (func_ptr = timeout_jobs[n].function) != NULL && n < TIMEOUT_JOBS_MAX )
	{
		if( timeout_jobs[n].counter >= timeout_jobs[n].counter_reload )
		{
			if( !timer_and_timeout_jobs_disabledB )
			{
				call_function = func_ptr;
				// - Call of add_timeout_job from time-out job:
				//   First increment of counter is made already before leaving T1INT_handler.
				// - Call of reload_timeout_job from time-out job:
				//   First increment of counter is made already before leaving T1INT_handler
				//   if reload_timeout_job is applied to time-out job with list index higher than n.
				// - Call of remove_timeout_job from time-out job:
				//   Counter increment is skipped for time-out job(s) having list index higher
				//   than n before job removal and equal to or lower than n after job removal.
				//   Following conditions have to apply to create this situation.
				//   * Time-out job(s) with index between 0 and n was(were) removed from list
				//     timeout_jobs[].
				//   * At least one job with index higher than n was stored in list timeout_jobs[]
				//     before job removal.
				//   As a result of skipping of counter increment call of affected time-out job(s)
				//   may be delayed by 1ms.
				call_function();
				// Automatic removal of time-out job called before. Do not remove time-out job by
				// call "remove_timeout_job(SELF);" placed in body of respective time-out job.
				remove_timeout_job(func_ptr);
			}
	 	}
		else
			timeout_jobs[n].counter++;

		n++;
	}

	IEN = DISABLE;	// Refer to application note AP16009 by Infineon.
}	// T1INT_handler


void SetTimeOutIntvElapsed(void)
{
	boTimeOutIntvElapsed = TRUE;
}	// SetTimeOutIntvElapsed


word StartTimeOutIntv(word TO_ms)
{
	word ActualTO_ms;

	// Ensure T1 interrupt handler not being called between assignment boTimeOutIntvElapsed
	// = FALSE and global interrupt disabling made in function add_timeout_job.
	remove_timeout_job(SetTimeOutIntvElapsed);
	boTimeOutIntvElapsed = FALSE;
	// Variable boTimeOutIntvElapsed == TRUE after returning from function add_timeout_job
	// if called with TO_ms == 0.
    ActualTO_ms = add_timeout_job(TO_ms, SetTimeOutIntvElapsed);
	return(ActualTO_ms);
}	// StartTimeOutIntv


bool TimeOutIntvElapsed(void)
{
	return(boTimeOutIntvElapsed);
}	// TimeOutIntvElapsed


void CancelTimeOutIntv(void)
{
	remove_timeout_job(SetTimeOutIntvElapsed);
}	// CancelTimeOutIntv


void Idle_ms(word ms)
{
	word actual_ms;

	if( ms > 0 )
	{
		actual_ms = StartTimeOutIntv(ms);
		if( actual_ms >= ms )
			do {} while( !TimeOutIntvElapsed() );
		CancelTimeOutIntv();
	}
}	// Idle_ms


word time_ms(word time_us)
{
	word time_ms_loc, time_rounded_down;

	time_ms_loc = time_us / 1000;
	time_rounded_down = time_ms_loc * 1000;
	if( time_us - time_rounded_down >= 500 )
		time_ms_loc += 1;

	return(time_ms_loc);
}	// time_ms
