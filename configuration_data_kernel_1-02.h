/*==============================================================================================*
 |       Filename: configuration_data_kernel_1-02.h                                             |
 | Project/Module: A21                                                                          |
 |           Date: 08/05/2007 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Software parameters stored in Flash EPROM sector 1.                          |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----------------------------------------------------------------------------------------------*/
typedef struct {	/* Start address: $0'3F00 ---------- S T R U C T   V E R S I O N   1.02 ----*/
/* +------- Offset referring to start address													*/
/* |   +--- Length of data																		*/
/*$00  1 */	byte struct_version;		/* Version of struct definition							*/
/*$01  1 */	byte struct_revision;		/* Revision of struct definition						*/
/*$02 32 */	char product_id[32];		/* Product name of analyzer								*/
			struct {					/* Device id of analyzer								*/
/*$22  1 */		byte year;
/*$23  1 */		byte reserved;
/*$24  2 */		word serial_number;
			} device_id;
/*$26 16 */	char CPU_name[16];			/* Type of microcontroller (C167CR, ST10R167)			*/
/*$36 10 */	WORD buscon[5];				/* Override EDE BUSCON settings, 0xFFFF -> no changes	*/
/*$40  8 */	WORD addrsel[4];			/* Override EDE ADDRSEL settings, 0xFFFF -> no changes	*/
/*$48  4 */	DWORD fCPU;					/* Processor Clock in Hz								*/
/*$4C  4 */	DWORD fOSC;					/* Oscillator Clock in Hz								*/
			struct {					/* Type of flash EPROM									*/
/*$50  1 */		BYTE manufacturer_id;
/*$51  1 */		BYTE device_id;
/*$52 14 */		char name[14];
			} EPROM;
/*$60  2 */	WORD SRAM_size;				/* Size of SRAM in kB									*/
/*$62 16 */	char RTC_name[16];			/* Type of nonvolatile timekeeping RAM					*/
/*$72  2 */	WORD NVRAM_size;			/* Size of NVRAM in kB									*/
/*$74 16 */	char sensor_interface[16];	/* Type of sensor interface (SSC, CAN)					*/
/*$84 16 */	char system_interface[16];	/* Type of system interface (ASC0-RS232, ...)			*/
} Configuration_Data_Kernel_102_type;
