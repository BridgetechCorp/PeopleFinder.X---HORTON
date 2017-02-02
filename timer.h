/* 
 * File:   timer.h
 * Author: Craig
 *
 * Created on May 26, 2013, 6:38 AM
 */

#ifndef TIMER_H
#define	TIMER_H

extern int timer1_interrupts;

void timer1_initialize (void);

int GetTimerInterrupts( void );

void SetTimerInterrupts( int val );


unsigned long GetWaitDelay( void );
void SetWaitDelay( unsigned long val );

void delay( unsigned long time);

#endif	/* TIMER_H */

