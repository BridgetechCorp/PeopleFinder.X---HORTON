/*==============================================================================================*
 |       Filename: flash.h                                                                      |
 | Project/Module: A21/module group Kernel         	                                            |
 |           Date: 09/10/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Implementation of functions for external Flash EPROM of type                 |
 |                 AMD AM29F400BB(200BB) and compatibles.                                       |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef FLASH_INC						/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define SectCnt_AM29F200BB			7
#define SectCnt_AM29F400BB			11

#ifdef ENABLE_FLASH_ERROR_LOG
	#define BACKUP_MEMORY_SIZE		(4 * 1024)
	#define FLASH_LOG_BUFFER_SIZE	80
#endif


/*----- Variable Type Definitions --------------------------------------------------------------*/
typedef struct
{
	byte man_id;
	byte dev_id;
} flash_type;

#ifdef EMULATOR
typedef struct
{
	byte huge *start;
	byte huge *end;
} sector_address_type;
#endif


/*----- Publication of Global Variables --------------------------------------------------------*/


/*----- Publication of Global Constants --------------------------------------------------------*/


/*----- Publication of Function Prototypes -----------------------------------------------------*/
void Copy_RAM_Functions(void);
void Check_Flash_Type(void);
int Erase_Sector_No_Backup(byte _huge *);

#ifdef EMULATOR
void Init_Global_Var_Flash(void);
#endif

#ifdef ENABLE_FLASH_ERROR_LOG
void Init_Log_Flash(void);
#endif


#define FLASH_INC
#endif	// end of "#ifndef FLASH_INC"
