/*==============================================================================================*
 |       Filename: general.h                                                                    |
 | Project/Module:                                                                              |
 |           Date: 09/09/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: General macro definitions and type definitions.                              |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef GENERAL_INC						/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*--- General Constants Definitions ------------------------------------------------------------*/
#define TRUE		1
#define FALSE		0
#define ENABLE		1
#define DISABLE		0
#define ON			1
#define OFF			0

#define DEFAULT		0

#define BIT0		0x0001U
#define BIT1   		0x0002U
#define BIT2    	0x0004U
#define BIT3    	0x0008U
#define BIT4    	0x0010U
#define BIT5    	0x0020U
#define BIT6    	0x0040U
#define BIT7    	0x0080U
#define BIT8		0x0100U
#define BIT9   		0x0200U
#define BIT10    	0x0400U
#define BIT11    	0x0800U
#define BIT12    	0x1000U
#define BIT13    	0x2000U
#define BIT14    	0x4000U
#define BIT15    	0x8000U


/*--- Global Macro Definitions -----------------------------------------------------------------*/
// Step to next position in ring buffer. Usage of macro NEXT may yield greater program code than
// usage of equivalent C statements if parameter a is a global variable and parameter m is a
// constant.
#define NEXT(a, m)				((++a < m) ? (a) : (a = 0))
#define MIN(a, b)				((a<b)?(a):(b))
#define MAX(a, b)				((a>b)?(a):(b))
#define RECORDS_IN_BUF(r, w, m)	((r <= w) ? (w - r) : (w + m - r))
#define BETWEEN(x, a, b)		((x >= a) && (x <= b))
#define DIFF_ABS(a, b)			((a < b) ? (b - a) : (a - b))
#define NEG(x)					(x >= ZERO_LEVEL)
#define POS(x)					(x <  ZERO_LEVEL)


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef unsigned char	bool;
typedef unsigned char	byte;
typedef unsigned int	word;
typedef unsigned long	dword;
typedef unsigned int	bitfield;

typedef bool 		BOOL;
typedef bool 		Bool;
typedef byte		BYTE;
typedef byte		Byte;
typedef word		WORD;
typedef word		Word;
typedef dword		DWORD;
typedef dword		LONG;
typedef bitfield	BitField;


#define GENERAL_INC
#endif	// end of "#ifndef GENERAL_INC"
