/*==============================================================================================*
 |       Filename: flash.c                                                                      |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 03/31/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of functions for external Flash EPROM of type                 |
 |                 AMD AM29F400BB(200BB) and compatibles.                                       |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167cr.h>
#include <string.h>

#include "kernel_interface.h"
#include "flash.h"
#include "configuration_data.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class CO=KERNEL
#pragma class PR=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
flash_type flash;

#ifdef EMULATOR
sector_address_type sector_address[SectCnt_AM29F400BB];
#endif

#ifdef ENABLE_FLASH_ERROR_LOG
char backup_memory[BACKUP_MEMORY_SIZE];
char log_buffer[FLASH_LOG_BUFFER_SIZE];
#endif


/*----- Function Prototypes --------------------------------------------------------------------*/
#ifdef EMULATOR
int Get_Sector_Number(byte huge *);
int Emulator_Write_Word(word , word huge *);
#endif


/*----- Global Constants -----------------------------------------------------------------------*/
const byte flash_log_ini[20] _at(ADDR_STORAGE_DATA) =
	{'F', 'l', 'a', 's', 'h', ' ',
	 'p', 'r', 'o', 'g', 'r', 'a', 'm', 'm', 'e', 'd', '.', '\r', '\n', 0};


/*----- RAM Functions (Code Overlayed to RAM space 0x80000 - 0x801C1) --------------------------*/

/* EDE - Linker/Locator Options - Miscellaneous: Additional locator controls					*/
/*		 CLASSES(ROM_START RAMFUNCS ROM_END(08000h TO 081C1h))									*/
/*		 ORDER SECTIONS(* 'ROM_START', * 'RAMFUNCS', * 'ROM_END')								*/
/*		 OVERLAY (RAMFUNCS(080000h TO 0801C1h))													*/
/* EDE - Linker/Locator Options - Reserve: Reserve memory area(s)								*/
/*		 080000h-0801C1h																		*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=ROM_START


/*----- Implementation of Functions ------------------------------------------------------------*/
void Start_ROM_Dummy_Func(void)
{
}


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=RAMFUNCS


/*----- Implementation of Functions ------------------------------------------------------------*/
void Start_RAM_Dummy_Func(void)
{
}

void AM29F200B_Read_Info(byte *man_id, byte *dev_id)
{
	Save_ILVL_and_Disable_All_INTs();
	*(byte *)0x0AAA = 0xAA;		/* 1st unlock cycle		*/
	*(byte *)0x0555 = 0x55;		/* 2nd unlock cycle		*/
	*(byte *)0x0AAA = 0x90;		/* Autoselect			*/
	*man_id = *(byte *)0x00;	/* Read Manufacturer ID	*/
	*dev_id = *(byte *)0x02;	/* Read Device ID		*/
	*(byte *)0x0000 = 0xF0;		/* Reset				*/
	Restore_ILVL();
}


int AM29F200B_Erase_Sector(byte huge *MemPtr)
{
	byte test, busy, timeout;	

	Save_ILVL_and_Disable_All_INTs();

	*(byte *)0x0AAA = 0xAA;		/* 1st unlock cycle		*/
	*(byte *)0x0555 = 0x55;		/* 2nd unlock cycle		*/
	*(byte *)0x0AAA = 0x80;		/* Erase				*/
	*(byte *)0x0AAA = 0xAA;		/* 4st cycle			*/
	*(byte *)0x0555 = 0x55;		/* 5nd cycle			*/
	*MemPtr = 0x30;				/* 5nd cycle			*/
	while( (*MemPtr & 0x08) != 0x08 );		/* Wait for time-out	*/

	do
	{
		test = *MemPtr;
		busy = (test & 0x80) != 0x80;		/* DQ7: Data# Polling */
		timeout = (test & 0x20) == 0x20;	/* DQ5: Exceeded Timing Limits */
	} while( busy && !timeout );

	Restore_ILVL();

	if( timeout && busy )
		return(2);			  	/* Erase timeout error	*/

	if( *MemPtr != 0xFF )
		return(3);			  	/* Erase not successful	*/

	return(0);				  	/* Erase successful		*/
}


int AM29F200B_Write_Word(word w_word, word huge *destination)
{
	int error = 0;
	word test;
	bool busyB;
	bool timeoutB;
	unsigned long address;

	address = (unsigned long)destination;
	if(address & 0x0001L)
		return(4);
	if(address > 0x3FFFF)
		return(5);
	if(address >= 0x0E000 && address <= 0x0FFFF)
		return(6);
	Save_ILVL_and_Disable_All_INTs();
	*(byte *)0x0AAA = 0xAA;		/* 1st unlock cycle		*/
	*(byte *)0x0555 = 0x55;		/* 2nd unlock cycle		*/
	*(byte *)0x0AAA = 0xA0;		/* Program				*/
	*destination = w_word;
	do
	{
		test = *destination;
		busyB = ((test ^ w_word) & 0x0080) == 0x0080;	/* DQ7: Data# Polling, complement of datum */
		timeoutB = (test & 0x0020) == 0x0020;			/* DQ5: Exceeded Timing Limits */
	} while( busyB && !timeoutB );
	Restore_ILVL();
	if( busyB )
		error = 2;
	else
		if( *destination != w_word )
			error = 3;
	return(error);
}


void End_RAM_Dummy_Func(void)
{
}


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=ROM_END


/*----- Implementation of Functions ------------------------------------------------------------*/
void End_ROM_Dummy_Func(void)
{
}


/*----- Code being part of module group Kernel -------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class CO=KERNEL
#pragma class PR=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Implementation of Functions ------------------------------------------------------------*/
bool FlashProg_Allowed(dword MemAddr)
{
	bool res;
	res = (MemAddr >= ADDR_CONFIG_DATA  && MemAddr <= ADDR_CONFIG_DATA_END ) ||
//	      (MemAddr >= ADDR_STORAGE_DATA && MemAddr <= ADDR_STORAGE_DATA_END) ||
	      (MemAddr >= ADDR_APPLICATION1 && MemAddr <= ADDR_APPLICATION2_END);
	return(res);
}	// FlashProg_Allowed


int Erase_Sector(byte huge *MemPtr)
{
	dword Source_Start_Addr, Source_End_Addr, Backup_Start_Addr;
	bool  Backup_Needed;
	word Length;
	int ErrNo;
#ifdef EMULATOR
	byte huge *begin, *end;
	char s;
#endif

	Length;
	ErrNo = 0;
	if( MemPtr >= (byte huge *)ADDR_CONFIG_DATA && MemPtr <= (byte huge *)ADDR_CONFIG_DATA_END )
	{
		Backup_Needed     = TRUE;
		Source_Start_Addr = ADDR_CONFIG_DATA;
		Source_End_Addr   = ADDR_CONFIG_DATA_END;
		Backup_Start_Addr = ADDR_STORAGE_DATA;
	}
	else
	{
		Backup_Needed     = FALSE;
		Source_Start_Addr = MaxMemAddrA21;
		Source_End_Addr   = MaxMemAddrA21;
		Backup_Start_Addr = MaxMemAddrA21;
	}

#ifndef ENABLE_FLASH_ERROR_LOG
	if( Backup_Needed )
	{
	#ifdef EMULATOR
		begin = (byte huge *)ADDR_STORAGE_DATA;
		end   = (byte huge *)ADDR_STORAGE_DATA_END;
		while( begin <= end )
			*begin++ = 0xFF;
	#else
		if( (ErrNo = AM29F200B_Erase_Sector((byte huge *)ADDR_STORAGE_DATA)) != 0 )
			return(ErrNo);
	#endif

		while( Source_Start_Addr <= Source_End_Addr && ErrNo == 0 )
		{
			Length = MIN(Source_End_Addr - Source_Start_Addr + 1, 0xFF);
			ErrNo = Program_Flash(Length, (byte huge *)Source_Start_Addr, (byte huge *)Backup_Start_Addr);
			Source_Start_Addr += Length;
			Backup_Start_Addr += Length;
		}

		if( ErrNo != 0 )
			return(ErrNo);
	}
#endif	// end of #ifndef ENABLE_FLASH_ERROR_LOG

#ifdef EMULATOR
	s = Get_Sector_Number(MemPtr);
	begin  = sector_address[s].start;
	end    = sector_address[s].end;
	while( begin <= end )
		*begin++ = 0xFF;
#else
	ErrNo = AM29F200B_Erase_Sector(MemPtr);
#endif

	return(ErrNo);
}	// Erase_Sector


int Erase_Sector_No_Backup(byte huge *MemPtr)
{
	int ErrNo;
#ifdef EMULATOR
	byte huge *begin, *end;
	char s;
#endif

#ifdef EMULATOR
	ErrNo = 0;
	s = Get_Sector_Number(MemPtr);
	begin  = sector_address[s].start;
	end    = sector_address[s].end;
	while( begin <= end )
		*begin++ = 0xFF;
#else
	ErrNo = AM29F200B_Erase_Sector(MemPtr);
#endif

	return(ErrNo);
}	// Erase_Sector_No_Backup


int Program_Flash(int length, byte huge *source, byte huge *destination)
{
	int ErrNo = 0;
	word p_word;
	byte lbyte;

	if( length > 0xFF )
		length = 0xFF;

	if( (word)destination & 0x0001 )	/* If start address is odd */
	{
		// Start address is odd. Do word alignment by reading byte at predecessor of start
		// address from Flash EPROM and using it as lower byte of program word.
		destination--;
		lbyte = *destination;
		p_word = (word)*source << 8;
		p_word |= lbyte;
		length--; source++;
		#ifdef EMULATOR
			ErrNo = Emulator_Write_Word(p_word, (word *)destination);
		#else
			ErrNo = AM29F200B_Write_Word(p_word, (word *)destination);
		#endif
		destination += 2;
	}

	while( length > 0 && ErrNo == 0 )
	{
		lbyte = *source;
		length--; source++;
		if( length == 0 )
		{
			// Count of program bytes is odd. Do word alignment by reading byte at successor of
			// end address from Flash EPROM and using it as upper byte of program word.
			p_word = *(destination + 1) << 8;
		}
		else
		{
			p_word = *source << 8;
			length--; source++;
		}
		p_word |= lbyte;
		#ifdef EMULATOR
			ErrNo = Emulator_Write_Word(p_word, (word *)destination);
		#else
			ErrNo = AM29F200B_Write_Word(p_word, (word *)destination);
		#endif
		destination += 2;
	}

	return(ErrNo);
}	// Program_Flash


void Detect_Flash_Type(byte *man_id, byte *dev_id)
{
#ifdef EMULATOR
	*man_id = FLASH_MAN_ID;
	*dev_id = FLASH_DEV_ID;
#else
	AM29F200B_Read_Info(man_id, dev_id);
#endif
}	// Detect_Flash_Type


void Copy_RAM_Functions(void)
{
	void (*func_addr)(void);
	word huge *to_addr;
	word huge *from_addr;
	word huge *stop_addr;


	func_addr = Start_RAM_Dummy_Func;
	to_addr   = (word huge *)func_addr;
	func_addr = Start_ROM_Dummy_Func;
	from_addr = (word huge *)func_addr;
	from_addr++;	/* Skip RETS instruction to get start address of start_ram_dummy in ROM */
	func_addr = End_ROM_Dummy_Func;
	stop_addr = (word huge *)func_addr;

	while( from_addr < stop_addr )
	{
		*(to_addr++) = *(from_addr++);
	}
}	// Copy_RAM_Functions


void Check_Flash_Type(void)
{
	Configuration_Data_Kernel_type *rom_k = (Configuration_Data_Kernel_type *)ADDR_KERNEL_CONFIG;

	Detect_Flash_Type(&flash.man_id, &flash.dev_id);
	if( rom_k->EPROM.manufacturer_id == 0xFF && rom_k->EPROM.device_id == 0xFF )
	{
		Program_Flash(1, &flash.man_id, &rom_k->EPROM.manufacturer_id);
		Program_Flash(1, &flash.dev_id, &rom_k->EPROM.device_id);
	}
}	// Check_Flash_Type


#ifdef EMULATOR
// Initialize global variables not properly initialized by function "clear_kernel_RAM".
void Init_Global_Var_Flash(void)
{
	sector_address[0].start  = (byte huge *)ADDR_KERNEL1;
	sector_address[0].end    = (byte huge *)ADDR_KERNEL1_END;
	sector_address[1].start  = (byte huge *)ADDR_CONFIG_DATA;
	sector_address[1].end    = (byte huge *)ADDR_CONFIG_DATA_END;
	sector_address[2].start  = (byte huge *)ADDR_STORAGE_DATA;
	sector_address[2].end    = (byte huge *)ADDR_STORAGE_DATA_END;
	sector_address[3].start  = (byte huge *)ADDR_KERNEL2;
	sector_address[3].end    = (byte huge *)ADDR_KERNEL2_END;
	sector_address[4].start  = (byte huge *)ADDR_COMMUNICATION;
	sector_address[4].end    = (byte huge *)ADDR_COMMUNICATION_END;
	sector_address[5].start  = (byte huge *)ADDR_APPLICATION1;
	sector_address[5].end    = (byte huge *)ADDR_APPLICATION1_END;
	sector_address[6].start  = (byte huge *)ADDR_APPLICATION2;
	sector_address[6].end    = (byte huge *)ADDR_APPLICATION2_END;
	sector_address[7].start  = (byte huge *)ADDR_SECTOR_07;
	sector_address[7].end    = (byte huge *)ADDR_SECTOR_07_END;
	sector_address[8].start  = (byte huge *)ADDR_SECTOR_08;
	sector_address[8].end    = (byte huge *)ADDR_SECTOR_08_END;
	sector_address[9].start  = (byte huge *)ADDR_SECTOR_09;
	sector_address[9].end    = (byte huge *)ADDR_SECTOR_09_END;
	sector_address[10].start = (byte huge *)ADDR_SECTOR_10;
	sector_address[10].end   = (byte huge *)ADDR_SECTOR_10_END;
}


int Get_Sector_Number(byte huge *MemPtr)
{
	int i;

	for(i = 0; i < SectCnt_AM29F400BB; i++)
		if(MemPtr >= sector_address[i].start && MemPtr <= sector_address[i].end)
			return(i);

	return(SectCnt_AM29F400BB);
}


int Emulator_Write_Word(word w_word, word huge *destination)
{
	int ErrNo = 0;

	if((word)destination & 0x0001)
		return(4);
	if(	destination < (word huge *) End_RAM_Dummy_Func &&
		destination > (word huge *) Start_RAM_Dummy_Func )
	{
		return(5);
	}
	*destination = w_word;
	if(*destination != w_word)
		ErrNo = 3;
	return(ErrNo);
}
#endif


/*----- Functions for Error Logging in Sector 2 ------------------------------------------------*/
#ifdef ENABLE_FLASH_ERROR_LOG
#include <stdio.h>
#include <string.h>

void Backup_Log_Flash(void)
{
	word m = 0;
	byte cr_lf[2] = { '\r', '\n' };
	byte *mem_start;
	byte *mem_end;

	mem_start = (byte *)ADDR_STORAGE_DATA;
	mem_end = (byte *)(ADDR_STORAGE_DATA_INFO - 1);
	while(*mem_end == 0xFF && mem_end > mem_start)		/* Search end of logger entries */
			mem_end--;
	mem_start = mem_end - BACKUP_MEMORY_SIZE;
	while(*mem_start != '\n' && mem_start < mem_end)	/* and search next entry */
		mem_start++;
	mem_start++;
	while(mem_start <= mem_end)							/* Save flash to RAM */
		backup_memory[m++] = *(mem_start++);
	Erase_Sector((byte huge *)ADDR_STORAGE_DATA);
	Program_Flash(20, (byte huge *)"Flash overrun.   \r\n", (byte huge *)ADDR_STORAGE_DATA);
	Program_Flash(m, (byte huge *)backup_memory, (byte huge *)(ADDR_STORAGE_DATA + 20));
}
		

void Log_Flash(char *text)
{
	int l;
	#ifndef NO_LOGGER
	Time_type t;
	Date_type d;
	#endif
	char *p;
	char *mem_end = (char *)ADDR_STORAGE_DATA_INFO;

	#ifndef NO_LOGGER
	Get_Date_Time(&d, &t);
	sprintf(log_buffer, "%02u.%02u.%02u;%02u:%02u:%02u;",
			d.Day, d.Month, d.Year, t.Hour, t.Minutes, t.Seconds);
	#else
	sprintf(log_buffer, "%02u.%02u.%02u;%02u:%02u:%02u;",
			0, 0, 0, 0, 0, 0);
	#endif
	strncat(log_buffer, text, FLASH_LOG_BUFFER_SIZE - 20);
	strcat(log_buffer, "\r\n");
	l = strlen(log_buffer);
	p = (char *)ADDR_STORAGE_DATA;
	while(*p != (char)0xFF && p < mem_end)
		p++;
	mem_end -= l;
	if(p >= mem_end)
	{
		Backup_Log_Flash();
		p = (char *)ADDR_STORAGE_DATA;
		while(*p != (char)0xFF && p <= mem_end)
			p++;
	}
	if(p != mem_end)
		Program_Flash(l, (byte *)log_buffer, (byte *)p);
}
		

void Init_Log_Flash(void)
{
	char *p;
	char s[22];

	p = (char *)ADDR_STORAGE_DATA;
	strncpy(s, p, 19);
	s[19] = '\0';
	if(	strcmp(s, "Flash programmed.\r\n") != 0 &&
		strcmp(s, "Flash overrun.   \r\n") != 0 )
	{
		Erase_Log_Flash();
	}
}


void Erase_Log_Flash(void)
{
	Erase_Sector((byte huge *)ADDR_STORAGE_DATA);
	Program_Flash(19, (byte huge *)"Flash programmed.\r\n", (byte huge *)ADDR_STORAGE_DATA);
}
#endif	// end of "#ifdef ENABLE_FLASH_ERROR_LOG"
