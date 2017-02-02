/* 
 * File:   uart.h
 * Author: Craig
 *
 * Created on November 7, 2015, 2:25 PM
 */

#ifndef UART_H
#define	UART_H

#include "types.h"

void InitUART1( void );
void putch( char c );
void putstr( char *str);
boolean getch(char *c);
void ClearUartReceiveBuffer( void );

#endif	/* UART_H */

