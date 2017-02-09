/* 
 * File:   CanIrma.h
 * Author: Craig Beiferman
 *
 * Created on May 4, 2013, 6:17 AM
 */

#ifndef CANIRMA_H
#define	CANIRMA_H

extern void SendIRMACounterStateRequest( void );
extern void SendIRMAFunctionAreaStatusRequest( void );
extern void SendIRMACounterStateRequest2( unsigned long addr, int functionArea );
extern void SendIRMACountDataReset( void );
extern void SendIRMAGetCanAddress( void );
extern void SendIRMASetDoorsOpen(void); // turn on the sensor
extern void SendIRMASetDoorsClose(void); // turn off the sensor
extern void SendIRMASensorStateRequest( void );

extern unsigned long sensorCanAddress;
#endif	/* CANIRMA_H */

