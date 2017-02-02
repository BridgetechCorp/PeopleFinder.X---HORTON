/*==============================================================================================*
 |       Filename: application_dummy.c                                                          |
 | Project/Module: A21/module group Application   	                                            |
 |           Date: 09/14/2010 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Dummy interface of module group Application to IRMA OPERA.                   |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
//#ifdef OBC
//	#include "..\obc_version.h"
//#else
	#include "..\opera_version.h"
//#endif
#include "..\opera_interface.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/
#define APPL_MODU_NAME			"Dummy"			// Module group name
#define APPL_MODU_VER			1				// Version
#define APPL_MODU_REV			1				// Revision
#define APPL_MODU_RELEASEDATE	"May 23 2008"	// Release date
#define APPL_MODU_BUILDNO		"00:00:01"		// Build no.


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
//#pragma class PR=APPLIC_ENTRY
//#pragma class CO=APPLIC_ENTRY


/*----- Global Variables -----------------------------------------------------------------------*/


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/

/*----- Sector Information Data ----------------------------------------------------------------*/
// CMB: const Sector_Information_type Application1_Info _at(ADDR_APPLICATION1_INFO) = {
const Sector_Information_type Application1_Info  = {
		APPL_MODU_NAME,			// Module group name
		APPL_MODU_VER,			// Version
		APPL_MODU_REV,			// Revision
		APPL_MODU_RELEASEDATE,	// Release date
		APPL_MODU_BUILDNO, 		// Build no.
		IRMAOPERAINTERFACEVER,	// IRMA OPERA interface version
		IRMAOPERACRC32TEMPL		// Checksum template
		};

//CMB:  const Sector_Information_type Application2_Info _at(ADDR_APPLICATION2_INFO) = {
const Sector_Information_type Application2_Info  = {
		APPL_MODU_NAME,			// Module group name
		APPL_MODU_VER,			// Version
		APPL_MODU_REV,			// Revision
		APPL_MODU_RELEASEDATE,	// Release date
		APPL_MODU_BUILDNO, 		// Build no.
		IRMAOPERAINTERFACEVER,	// IRMA OPERA interface version
		IRMAOPERACRC32TEMPL		// Checksum template
		};

/*----- A21 firmware name ----------------------------------------------------------------------*/
#ifdef EMULATOR
const unsigned char Firmware_File_Name_ROM[64] _at(ADDR_APPLICATION1) = {
	'E' , 'm' , 'u' , 'l' , 'a' , 't' , 'o' , 'r' , '\0', 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

const unsigned char Inherent_Firmware_Name_ROM[64] _at(ADDR_APPLICATION2) = {
	'E' , 'm' , 'u' , 'l' , 'a' , 't' , 'o' , 'r' , '\0', 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
#else
// Written by A21_Boot or A21_Update.
// CMB: const unsigned char Firmware_File_Name_ROM[64] _at(ADDR_APPLICATION1) = {
const unsigned char Firmware_File_Name_ROM[64]  = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

// Written by A21_ModuleManager.
//CMB: const unsigned char Inherent_Firmware_Name_ROM[64] _at(ADDR_APPLICATION2) = {
const unsigned char Inherent_Firmware_Name_ROM[64]  = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
#endif


/*----- Implementation of Functions ------------------------------------------------------------*/
// Function application_main has to be the first function in memory class APPLIC_ENTRY. 
int application_main(void)
{
 	while( opera_GetRunlevel() >= RUNLEVEL_APPL )
		opera_CallRunlevelJobs();

	return(0);
}
