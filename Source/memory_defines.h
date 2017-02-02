/*==============================================================================================*
 |       Filename: memory_defines.h                                                             |
 | Project/Module: A21                          	                                            |
 |           Date: 11/26/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Definition of general memory related constants for A21 firmware projects.    |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef MEMORY_DEFINES_INC				/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/

// Settings for Tasking C for C166/ST10 v7.0 r1:
//
// IRMA Opera module:
// LocReserveMem=03F94h-03FCDh,
//               04012h-05FCDh,
//               06014h-07FFFh,
//               080000h-0803FFh,
//               0C0080h-0DFFFFh
// LocHeapSize=256
// LocUserStackSize=2C00h
// LocClasses=KERNELRAM(083000h-08BFFFh),COMMRAM(08C000h-08EFFFh),
//            KERNEL(00000h-03EFFh,08400h-0B3FFh),{library list}(0B400h-0DEFFh),KERNEL_TAB(0DF00h-0DFFFh),
//            COMM(010000h-01FEFFh),COMM_TAB(01FF00h-01FFFFh),
//            APPLIC_ENTRY(020040h-0201FFh),
//            NV_SRAM(0C0000h-0DFFF7h) = additional memory class for firmware supporting NVRAM+RTC
// LocMemRam=0E000h-0E7FFh,080000h-09FFFFh,0C0000h-0DFFFFh
// LocMemRom=00000h-0DFFFh,10000h-3FFFFh
//
// A21 Firmware Parameters module:
// LocReserveMem=03F94h-03FCDh,
//               04046h-04083h,040CAh-040FFh,042A2h-042BFh,044E0h-044FFh,04D00h-04EFFh,
//               050C0h-057FFh,05804h-0581Fh,05822h-059FDh,05AF6h-05EFFh,05F10h-05FCDh,
//               06014h-07FFFh,
//               080000h-0803FFh,
//               0C0080h-0DFFFFh
// LocHeapSize=256
// LocUserStackSize=
// LocClasses=
// LocMemRam=0E000h-0E7FFh,080000h-09FFFFh,0C0000h-0DFFFFh
// LocMemRom=00000h-0DFFFh,10000h-3FFFFh
//
// IRMA Application module:
// LocReserveMem=03F94h-03FCDh,
//               04046h-04083h,040CAh-040FFh,042A2h-042FFh,043E0h-044FFh,04D00h-04FFFh,
//               050C0h-057FFh,05804h-0581Fh,05822h-059FDh,05AF6h-05EFFh,05F10h-05FCDh,
//               06014h-07FFFh,
//               080000h-0803FFh,
//               0C0080h-0DFFFFh
// LocHeapSize=256
// LocUserStackSize=2C00h
// LocClasses=APPLICRAM(08F000h-09FFFFh),
//            KERNEL(00000h-03EFFh,08400h-0B3FFh),KERNEL_TAB(0DF00h-0DFFFh),
//            COMM(010000h-01FEFFh),COMM_TAB(01FF00h-01FFFFh),
//            APPLIC_ENTRY(020040h-0201FFh),
//            {library list}(020200h-022FFFh),
//            APPLIC(023000h-03FFFFh)
// LocMemRam=0E000h-0E7FFh,080000h-09FFFFh,0C0000h-0DFFFFh
// LocMemRom=00000h-0DFFFh,10000h-3FFFFh

/*------------------- F l a s h   E P R O M ----------------------------------------------------*/
/* Sector 0	0'0000h - 0'3EFFh	KERNEL			Code of KERNEL									*/
/*			0'3F00h - 0'3FFFh					Device Immanent Configuration Data				*/
/* Sector 1	0'4000h - 0'5FFFh					Firmware Parameters								*/
/* Sector 2	0'6000h - 0'7FFFh					Error Logger									*/
/* Sector 3	0'8000h - 0'83FFh	RAMFUNCS		Overlay											*/
/*			0'8400h - 0'B3FFh	KERNEL			Code of KERNEL									*/
/*			0'B400h - 0'DEFFh					C Standard Libraries for IRMA Opera				*/
/*			0'DF00h - 0'DFFFh	KERNEL_TAB		Function Table as Interface for KERNEL			*/
/*			0'E000h - 0'FFFFh	reserved		XRAM, CAN, ESFR area, IRAM, SFR area			*/
/* Sector 4	1'0000h - 1'FEFFh	COMM			Code of COMM 									*/
/*			1'FF00h - 0'FFFFh	COMM_TAB		Function Table as Interface for COMM			*/
/* Sector 5	2'0000h - 2'003Fh					Firmware Name (name of Intel Hex file)			*/
/* 			2'0040h - 2'01FFh	APPLIC_ENTRY	Code of APPLIC_ENTRY							*/
/* 			2'0200h - 2'2FFFh					C Standard Libraries for IRMA Application		*/
/* 			2'3000h - 2'FFFFh	APPLIC			Code of APPLIC									*/
/* Sector 6	3'0000h - 3'003Fh					Inherent Firmware Name							*/
/* 			3'0040h - 3'FFFFh	APPLIC			Code of APPLIC									*/
/*------------------- R A M --------------------------------------------------------------------*/ 
/* SRAM		8'0000h - 8'03FFh	RAMFUNCS		Overlay											*/
/* 			8'0400h - 8'2FFFh	CUSTACK			User Stack										*/
/* 			8'3000h - 8'BFFFh	KERNELRAM		Variables of KERNEL 							*/
/* 			8'C000h - 8'EFFFh	COMMRAM 		Variables of COMM 								*/
/* 			8'F000h - 9'FFFFh	APPLICRAM		Variables of APPLIC	(Code Download Area)		*/
/* NVSRAM	C'0000h - C'007Fh	NV_SRAM			Non-volatile Logger Control Data	  			*/
/*			C'0080h - D'FFF7h	NV_SRAM			Timekeeper with (128k-8)byte SRAM	  			*/
/*			D'FFF8h - D'FFFFh					Timekeeper RTC registers, 8byte 	  			*/
/*----------------------------------------------------------------------------------------------*/ 

// Memory space of A21 analyzer is 1Mbyte large.
#define MaxMemAddrA21	0xFFFFFF


/*----- Flash EPROM ----------------------------------------------------------------------------*/
// Usage of preprocessor macro "#define identifier expression":
// If "identifier" is found by preprocessor it is replaced by "expression". As a result
// brackets enclosing "expression" always have to be used to ensure intended evaluation
// of expression.
#define SECTOR_INFORMATION_SIZE		(sizeof(Sector_Information_type) - 1)

#define ADDR_KERNEL1				0x00000								/* Beginning Sector 0 */
#define ADDR_KERNEL1_END			0x03FFF								/* 16 kB              */
#define ADDR_KERNEL1_INFO			(ADDR_KERNEL1_END - SECTOR_INFORMATION_SIZE)
#define ADDR_OPERA_MODU_NAME		0x03EC4
#define ADDR_KERNEL_CONFIG			0x03F00
#define ADDR_KERNEL2				0x08000								/* Beginning Sector 3 */
#define ADDR_KERNEL2_END			0x0DFFF								/* 24 kB              */
#define ADDR_KERNEL2_INFO			(ADDR_KERNEL2_END - SECTOR_INFORMATION_SIZE)
#define ADDR_KERNEL_POINTER			(ADDR_KERNEL2_INFO - 200)
#define SECTOR_KERNEL1				0
#define SECTOR_KERNEL2				3

#define ADDR_CONFIG_DATA			0x04000UL							/* Beginning Sector 1 */
#define ADDR_CONFIG_DATA_END		0x05FFF								/* 08 kB              */
#define ADDR_CONFIG_DATA_INFO		(ADDR_CONFIG_DATA_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_CONFIG_DATA			1

#define ADDR_STORAGE_DATA			0x06000								/* Beginning Sector 2 */
#define ADDR_STORAGE_DATA_END		0x07FFF								/* 08 kB              */
#define ADDR_STORAGE_DATA_INFO		(ADDR_STORAGE_DATA_END - SECTOR_INFORMATION_SIZE)
#define ADDR_KERNEL_CONFIG_BACKUP	0x07F00
#define SECTOR_STORAGE_DATA			2

#define ADDR_COMMUNICATION			0x10000								/* Beginning Sector 4 */
#define ADDR_COMMUNICATION_END		0x1FFFF								/* 64 kB              */
#define ADDR_COMMUNICATION_INFO		(ADDR_COMMUNICATION_END - SECTOR_INFORMATION_SIZE)
#define ADDR_COMMUNICATION_POINTER	(0x1FF00)
#define SECTOR_COMMUNICATION		4

#define ADDR_APPLICATION1			0x20000								/* Beginning Sector 5 */
#define ADDR_APPLICATION1_END  		0x2FFFF								/* 64 kB              */
#define ADDR_APPLICATION1_INFO		(ADDR_APPLICATION1_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION1			5

#define ADDR_APPLICATION2			0x30000								/* Beginning Sector 6 */
#define ADDR_APPLICATION2_END 		0x3FFFF								/* 64 kB              */
#define ADDR_APPLICATION2_INFO		(ADDR_APPLICATION2_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION2			6

#define ADDR_SECTOR_07				0x40000								/* Beginning Sector 7 */
#define ADDR_SECTOR_07_END			0x4FFFF								/* 64 kB              */
#define ADDR_SECTOR_07_INFO			(ADDR_SECTOR_07_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION3			7

#define ADDR_SECTOR_08				0x50000								/* Beginning Sector 8 */
#define ADDR_SECTOR_08_END			0x5FFFF								/* 64 kB              */
#define ADDR_SECTOR_08_INFO			(ADDR_SECTOR_08_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION4			8

#define ADDR_SECTOR_09				0x60000								/* Beginning Sector 9 */
#define ADDR_SECTOR_09_END			0x6FFFF								/* 64 kB              */
#define ADDR_SECTOR_09_INFO			(ADDR_SECTOR_09_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION5			9

#define ADDR_SECTOR_10				0x70000								/* Beginning Sector 10*/
#define ADDR_SECTOR_10_END			0x7FFFF								/* 64 kB              */
#define ADDR_SECTOR_10_INFO			(ADDR_SECTOR_10_END - SECTOR_INFORMATION_SIZE)
#define SECTOR_APPLICATION6			10

#define FIRST_SECTOR				SECTOR_KERNEL1
#define LAST_SECTOR_200				SECTOR_APPLICATION2
#define LAST_SECTOR					SECTOR_APPLICATION6
#define FLASH_SECTOR_CNT_200		(LAST_SECTOR_200 + 1)	// AM29F200BB:  7 sectors 
#define MAX_FLASH_SECTOR_CNT		(LAST_SECTOR + 1)		// AM29F400BB: 11 sectors 

#define FIRMWARE_NAME_LENGTH  		60


/*----- SRAM -----------------------------------------------------------------------------------*/
#define ADDR_KERNELRAM				0x83000
#define ADDR_KERNELRAM_END			0x8BFFF
#define ADDR_COMMRAM				0x8C000
#ifdef GATEWAY
#define ADDR_COMMRAM_END			0x9FFFF
#else
#define ADDR_COMMRAM_END			0x8EFFF
#endif
#define ADDR_APPLICRAM				0x8F000
#define ADDR_APPLICRAM_END			0x9FFFF


/*----- NVSRAM and Serial DataFlash ------------------------------------------------------------*/
#ifndef NO_LOGGER
	#define ADDR_NVRAM_START				0x0C0000
	#define ADDR_LOGGER_START				0x0C0080
	#define TIME_SYNCHRONIZED_OFFSET		0x00
	#define MGC_NAME_OFFSET					0x04
	#define IRMAOPERAVER_OFFSET				0x14
	#define IRMAOPERAREV_OFFSET				0x15
	#define IN_BACKUP_OFFSET				0x1A
	#define OUT_BACKUP_OFFSET				0x3A
	#define FIRST_BACKUP1_OFFSET			0x5A
	#define FIRST_BACKUP2_OFFSET			0x5E
	#define FIRST_BACKUP3_OFFSET			0x62
	#define LAST_BACKUP1_OFFSET				0x66
	#define LAST_BACKUP2_OFFSET				0x6A
	#define LAST_BACKUP3_OFFSET				0x6E
	#define LOG_PLAIN_HEADER_OFFSET			0x7F
	#ifndef A21CL
		#define LOGGER_RESERVED				(ADDR_LOGGER_START - ADDR_NVRAM_START + 0x10)
		#define ADDR_NVRAM_TEST_128			0x0DFFF0
		#define ADDR_NVRAM_TEST_32			0x0C7FF0
		#define ADDRESS_LOGGER_END			0x0DFFEF
	#else
		#define TIME_SYNCHRONIZED_INDEX		0
		#define MGC_NAME_INDEX				4
		#define IRMAOPERAVER_INDEX			20
		#define IRMAOPERAREV_INDEX			21
//		#define IN_BACKUP_INDEX				// No counting backup
//		#define OUT_BACKUP_INDEX			// No counting backup
		#define FIRST_BACKUP1_INDEX			22
		#define FIRST_BACKUP2_INDEX			26
		#define FIRST_BACKUP3_INDEX			30
		#define LAST_BACKUP1_INDEX			34
		#define LAST_BACKUP2_INDEX			38
		#define LAST_BACKUP3_INDEX			42
		#define NEXT_FIRST_INDEX			46
//		#define LOG_PLAIN_HEADER_INDEX		// No header type (always "compressed")

		#define LOGGER_START_OFFSET			0x210
		#define ADDRESS_LOGGER_END			0x13FE6F	// This is a logical address only, valid only when records
	#endif												// area is "imaginarily" translated "as if it were" allocated
#endif													// starting from 0x0C0080. This is used when dataflash memory is
														// accessed by means of messages IRMA-M and IRMA-P, in order to
														// keep addresses compatibility with NVRAM.


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/


#define MEMORY_DEFINES_INC
#endif	// end of "#ifndef MEMORY_DEFINES_INC"
