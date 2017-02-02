/*==============================================================================================*
 |       Filename: opera_version.h                                                              |
 | Project/Module: A21                          	                                            |
 |           Date: 04/03/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Constant definitions used for IRMA OPERA administration.                     |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Constants Definitions for IRMA OPERA Sectors Information -------------------------------*/
#define IRMAOPERAVER			6				// Version
#define IRMAOPERAREV			00				// Revision
//	Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
#define IRMAOPERARELEASEDATE	"Apr 03 2012"	// Release date
#define IRMAOPERABUILDNO		"00:00:01"		// Build no.
#define IRMAOPERAINTERFACEVER	"APP20"			// IRMA OPERA interface version
#define IRMAOPERACRC32TEMPL		0x2E6B2E6F		// Checksum template = "o.k."

//Constant definitions placed in "configuration_data.h" to simplify inclusion of header files. 
//#define	FIRMWPARAMVER			1				// Version of A21 firmware parameters.
//#define	MINFIRMWPARAMREV		7				// Minimum revision of A21 firmware parameters.


/*----- Name Definition for module group Kernel ------------------------------------------------*/

#if defined(CAN_SENSOR)
	#define IOP_CLASS			"CAN"		// IRMA 4 Analyzer using IRMA CAN interface
#elif defined(SERIAL_SENSOR)
	#define IOP_CLASS			"SSC"		// IRMA 4 Analyzer using synchronous serial interface
#elif defined(OBC)
	#define IOP_CLASS			"OBC"		// OBC emulation
#elif defined(GATEWAY)
	#define IOP_CLASS			"GWY"		// IRMA 5 gateway or master
#else
	#define IOP_CLASS			"???"
#endif

#if defined(IBIS_PROTOCOL)
	#define IOP_PROT			"IB"
#elif defined(J1708_PROTOCOL)
	#define IOP_PROT			"J8"
#elif defined(GLORIA)
	#define IOP_PROT			"GL"
#elif !defined(NO_LOGGER)
	#define IOP_PROT			"LO"
#else
	#define IOP_PROT			"IR"
#endif

// Flags regarding special hardware:
// bit 3: flag CXYO
// bit 2: not assigned
// bit 1: not assigned
// bit 0: flag A21CL
#if defined(CXYO)
	#if defined(A21CL)
		#define IOP_VAR1				"9"
	#else
		#define IOP_VAR1				"8"
	#endif
#else
	#if defined(A21CL)
		#define IOP_VAR1				"1"
	#else
		#define IOP_VAR1				"0"
	#endif
#endif

// Flags regarding firmware used for IRMA device testing:
// bit 3: flag DEVTEST
// bit 2: not assigned
// bit 1: flag EMC
// bit 0: flag BURNIN
#if defined(DEVTEST)
	#if defined(EMC)
		#if defined(BURNIN)
			#define IOP_VAR2			"B"
		#else
			#define IOP_VAR2			"A"
		#endif
	#else
		#if defined(BURNIN)
			#define IOP_VAR2			"9"
		#else
			#define IOP_VAR2			"8"
		#endif
	#endif
#else
	#if defined(EMC)
		#if defined(BURNIN)
			#define IOP_VAR2			"3"
		#else
			#define IOP_VAR2			"2"
		#endif
	#else
		#if defined(BURNIN)
			#define IOP_VAR2			"1"
		#else
			#define IOP_VAR2			"0"
		#endif
	#endif
#endif

// Flags regarding usage of port P2 for debugging:
// bit 3: flag DEBUG_P2_7
// bit 2: flag DEBUG_P2_6
// bit 1: flag DEBUG_P2_5
// bit 0: flag DEBUG_P2_4
#if defined(DEBUG_P2_7)
	#if defined(DEBUG_P2_6)
		#if defined(DEBUG_P2_5)
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"F"
			#else
				#define IOP_VAR3		"E"
			#endif
		#else
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"D"
			#else
				#define IOP_VAR3		"C"
			#endif
		#endif
	#else
		#if defined(DEBUG_P2_5)
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"B"
			#else
				#define IOP_VAR3		"A"
			#endif
		#else
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"9"
			#else
				#define IOP_VAR3		"8"
			#endif
		#endif
	#endif
#else
	#if defined(DEBUG_P2_6)
		#if defined(DEBUG_P2_5)
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"7"
			#else
				#define IOP_VAR3		"6"
			#endif
		#else
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"5"
			#else
				#define IOP_VAR3		"4"
			#endif
		#endif
	#else
		#if defined(DEBUG_P2_5)
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"3"
			#else
				#define IOP_VAR3		"2"
			#endif
		#else
			#if defined(DEBUG_P2_4)
				#define IOP_VAR3		"1"
			#else
				#define IOP_VAR3		"0"
			#endif
		#endif
	#endif
#endif

// Flags regarding regarding usage of port P2:
// bit 3: flag P2_OUT
// bit 2: not assigned
// bit 1: flag SW4
// bit 0: flag SW2
#if defined(P2_OUT)
	#if defined(SW4)
		#if defined(SW2)
			#define IOP_VAR4			"B"
		#else
			#define IOP_VAR4			"A"
		#endif
	#else
		#if defined(SW2)
			#define IOP_VAR4			"9"
		#else
			#define IOP_VAR4			"8"
		#endif
	#endif
#else
	#if defined(SW4)
		#if defined(SW2)
			#define IOP_VAR4			"3"
		#else
			#define IOP_VAR4			"2"
		#endif
	#else
		#if defined(SW2)
			#define IOP_VAR4			"1"
		#else
			#define IOP_VAR4			"0"
		#endif
	#endif
#endif

// Flags regarding Flash EPROM and start-up:
// bit 3: flag NO_SECTOR_CHECK
// bit 2: flag ENABLE_FLASH_ERROR_LOG
// bit 1: not assigned
// bit 0: flag STARTUP_DELAY
#if defined(NO_SECTOR_CHECK)
	#if defined(ENABLE_FLASH_ERROR_LOG)
		#if defined(STARTUP_DELAY)
			#define MGK_VAR1			"D"
		#else
			#define MGK_VAR1			"C"
		#endif
	#else
		#if defined(STARTUP_DELAY)
			#define MGK_VAR1			"9"
		#else
			#define MGK_VAR1			"8"
		#endif
	#endif
#else
	#if defined(ENABLE_FLASH_ERROR_LOG)
		#if defined(STARTUP_DELAY)
			#define MGK_VAR1			"5"
		#else
			#define MGK_VAR1			"4"
		#endif
	#else
		#if defined(STARTUP_DELAY)
			#define MGK_VAR1			"1"
		#else
			#define MGK_VAR1			"0"
		#endif
	#endif
#endif

// Flags regarding specialties of ASC0 usage:
// bit 3: not assigned
// bit 2: not assigned
// bit 1: flag USE_T3OUT
// bit 0: flag DEBUG_START_RS232_DEBUG_MODE
#if defined(USE_T3OUT)
	#if defined(DEBUG_START_RS232_DEBUG_MODE)
		#define MGK_VAR2				"3"
	#else
		#define MGK_VAR2				"2"
	#endif
#else
	#if defined(DEBUG_START_RS232_DEBUG_MODE)
		#define MGK_VAR2				"1"
	#else
		#define MGK_VAR2				"0"
	#endif
#endif

// Check for definition of special variant identifer. It is used for indication of
// special project file settings which cannot be evaluated by conditional compilation.
#ifdef SVARID
	#if   SVARID == 0
		#define	SVARIDSTR	"0"
	#elif SVARID == 1
		#define	SVARIDSTR	"1"
	#elif SVARID == 2
		#define	SVARIDSTR	"2"
	#elif SVARID == 3
		#define	SVARIDSTR	"3"
	#elif SVARID == 4
		#define	SVARIDSTR	"4"
	#elif SVARID == 5
		#define	SVARIDSTR	"5"
	#elif SVARID == 6
		#define	SVARIDSTR	"6"
	#elif SVARID == 7
		#define	SVARIDSTR	"7"
	#elif SVARID == 8
		#define	SVARIDSTR	"8"
	#elif SVARID == 9
		#define	SVARIDSTR	"9"
	#endif
#endif

/*--- MGK Name Construction ---*/
#define MGK_VAR			IOP_VAR1 ## "" ## IOP_VAR2 ## "" ## IOP_VAR3 ## "" ## IOP_VAR4 ## "" ## MGK_VAR1 ## "" ## MGK_VAR2

#ifdef SVARIDSTR
	#define MGK_NAME	IOP_CLASS ## "+" ## IOP_PROT ## ":" ## MGK_VAR ## "," ## SVARIDSTR
#else
	#define MGK_NAME	IOP_CLASS ## "+" ## IOP_PROT ## ":" ## MGK_VAR
#endif


/*----- Name Definition for module group Communication -----------------------------------------*/

// Flags regarding specialties of UIP implementation:
// bit 3: not assigned
// bit 2: not assigned
// bit 1: not assigned
// bit 0: not assigned
#define MGC_VAR1	 							"0"

// Flags regarding specialties of custom protocol or extended UIP implementation:

#if !defined(IBIS_PROTOCOL) && !defined(J1708_PROTOCOL) && !defined(GLORIA) && defined(NO_LOGGER)
	#define MGC_VAR2 							"0"
#endif

// Recently not considered: IBIS_USE2DIGITADDR
#if defined(IBIS_PROTOCOL)
// bit 3: flag IBIS_INEO
// bit 2: flag IBIS_IRMA4
// bit 1: flag IBIS_STATUS6
// bit 0: flag IBIS_SNIFFER
	#if defined(IBIS_INEO)
		#if defined(IBIS_IRMA4)
			#if defined(IBIS_STATUS6)
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"F"
				#else
					#define MGC_VAR2			"E"
				#endif
			#else
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"D"
				#else
					#define MGC_VAR2			"C"
				#endif
			#endif
		#else
			#if defined(IBIS_STATUS6)
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"B"
				#else
					#define MGC_VAR2			"A"
				#endif
			#else
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"9"
				#else
					#define MGC_VAR2			"8"
				#endif
			#endif
		#endif
	#else
		#if defined(IBIS_IRMA4)
			#if defined(IBIS_STATUS6)
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"7"
				#else
					#define MGC_VAR2			"6"
				#endif
			#else
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"5"
				#else
					#define MGC_VAR2			"4"
				#endif
			#endif
		#else
			#if defined(IBIS_STATUS6)
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"3"
				#else
					#define MGC_VAR2			"2"
				#endif
			#else
				#if defined(IBIS_SNIFFER)
					#define MGC_VAR2			"1"
				#else
					#define MGC_VAR2			"0"
				#endif
			#endif
		#endif
	#endif
#endif

#if defined(J1708_PROTOCOL)
// bit 3: flag J1708_ORBITAL
// bit 2: flag J1708_XEROX
// bit 1: flag UJ1708IP and integer UJ1708IPCFG
// bit 0: flag UJ1708IP and integer UJ1708IPCFG
	#if defined(J1708_ORBITAL)
		#define MGC_VAR2						"8"
	#elif defined(J1708_XEROX)
		#define MGC_VAR2						"4"
	#elif defined(UJ1708IP)
		#if   UJ1708IPCFG == 4
			#define MGC_VAR2   					"3"
		#elif UJ1708IPCFG == 3
			#define MGC_VAR2					"2"
		#elif UJ1708IPCFG == 2
			#define MGC_VAR2					"1"
		#elif UJ1708IPCFG == 1
			#define MGC_VAR2					"0"
		#endif
	#else
		#define MGC_VAR2						"?"
	#endif
#endif

#if !defined(NO_LOGGER) && !defined(IBIS_PROTOCOL)
// bit 3: flag SIMULATE_DOOR_SWITCH
// bit 2: flag LIMIT_EVENT_NO
// bit 1: flag DOOR_OPEN_EVENT
// bit 0: flag NO_COUNTER_BACKUP
	#if defined(SIMULATE_DOOR_SWITCH)
		#if defined(LIMIT_EVENT_NO)
			#if defined(DOOR_OPEN_EVENT)
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"F"
				#else
					#define MGC_VAR2		   	"E"
				#endif
			#else
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"D"
				#else
					#define MGC_VAR2	   		"C"
				#endif
			#endif
		#else
			#if defined(DOOR_OPEN_EVENT)
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2		   	"B"
				#else
					#define MGC_VAR2		   	"A"
				#endif
			#else
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"9"
				#else
					#define MGC_VAR2			"8"
				#endif
			#endif
		#endif
	#else
		#if defined(LIMIT_EVENT_NO)
			#if defined(DOOR_OPEN_EVENT)
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"7"
				#else
					#define MGC_VAR2		   	"6"
				#endif
			#else
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"5"
				#else
					#define MGC_VAR2	   		"4"
				#endif
			#endif
		#else
			#if defined(DOOR_OPEN_EVENT)
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2		   	"3"
				#else
					#define MGC_VAR2		   	"2"
				#endif
			#else
				#if defined(NO_COUNTER_BACKUP)
					#define MGC_VAR2	   		"1"
				#else
					#define MGC_VAR2			"0"
				#endif
			#endif
		#endif
	#endif
#endif

/*--- MGC Name Construction ---*/
#define MGC_VAR			IOP_VAR1 ## "" ## IOP_VAR2 ## "" ## IOP_VAR3 ## "" ## IOP_VAR4 ## "" ## MGC_VAR1 ## "" ## MGC_VAR2

#ifdef SVARIDSTR
	#define MGC_NAME	IOP_CLASS ## "+" ## IOP_PROT ## ":" ## MGC_VAR ## "," ## SVARIDSTR
#else
	#define MGC_NAME	IOP_CLASS ## "+" ## IOP_PROT ## ":" ## MGC_VAR
#endif


/*----- General User Macros Restrictions -------------------------------------------------------*/
#if defined(DELIVERY)
	#error "Symbol DELIVERY is not necessary anymore, it is the default setting"
#endif

#if defined(NO_CL)
	#error "Symbol NO_CL is not necessary anymore, it is the default setting"
#endif

#if defined(CAN_500kbps)
	#error "Symbol CAN_500kbps is not necessary anymore, CAN baud rate is now firmware parameter"
#endif

#if defined(CAN_SENSOR)
	#if defined(SERIAL_SENSOR)
		#error "Wrong combination of user macros: CAN_SENSOR + SERIAL_SENSOR"
	#endif
#else
	#if !defined(OBC) && !defined(GATEWAY)
		#if !defined(SERIAL_SENSOR)
			#error "It hasn't been defined either CAN_SENSOR nor SERIAL_SENSOR"
		#endif	

		#if defined(SIMCA_MODE)
			#error "Wrong combination of user macros: SERIAL_SENSOR + SIMCA_MODE"
		#endif

		#if defined(A21CL)
			#error "Wrong combination of user macros: SERIAL_SENSOR + A21CL"
		#endif
	#endif
#endif

#if defined(OBC)
	#if defined(CAN_SENSOR)
		#error "Wrong combination of user macros: OBC + CAN_SENSOR"
	#endif

	#if defined(SIMCA_MODE)
		#error "Wrong combination of user macros: OBC + SIMCA_MODE"
	#endif

	#if defined(SERIAL_SENSOR)
		#error "Wrong combination of user macros: OBC + SERIAL_SENSOR"
	#endif

	#if defined(DEVTEST)
		#error "Wrong combination of user macros: OBC + DEVTEST"
	#endif

	#if defined(GLORIA)
		#error "Wrong combination of user macros: OBC + GLORIA"
	#endif

	#if defined(GATEWAY)
		#error "Wrong combination of user macros: OBC + GATEWAY"
	#endif
#endif

#if defined(GATEWAY)
	#if defined(CAN_SENSOR)
		#error "Wrong combination of user macros: GATEWAY + CAN_SENSOR"
	#endif

	#if defined(SIMCA_MODE)
		#error "Wrong combination of user macros: GATEWAY + SIMCA_MODE"
	#endif

	#if defined(SERIAL_SENSOR)
		#error "Wrong combination of user macros: GATEWAY + SERIAL_SENSOR"
	#endif

	#if defined(DEVTEST)
		#error "Wrong combination of user macros: GATEWAY + DEVTEST"
	#endif

	#if defined(GLORIA)
		#error "Wrong combination of user macros: GATEWAY + GLORIA"
	#endif
#endif

#if !defined(DEVTEST)
	#if defined(EMC)
		#error "Wrong combination of user macros: symbol EMC without symbol DEVTEST"
	#endif

	#if defined(BURNIN)
		#error "Wrong combination of user macros: symbol BURNIN without symbol DEVTEST"
	#endif

	#if defined(P2_OUT)
		#error "Wrong combination of user macros: symbol P2_OUT without symbol DEVTEST"
	#endif
#else
	#if defined(SIMCA_MODE)
		#error "Wrong combination of user macros: DEVTEST + SIMCA_MODE"
	#endif

	#if defined(EMC)		
		#if defined(BURNIN)
			#error "Wrong combination of user macros: EMC + BURNIN"
		#endif

		#if defined(P2_OUT)
			#error "Wrong combination of user macros: EMC + P2_OUT"
		#endif
	#endif
#endif

#if defined(IBIS_PROTOCOL)
	#if defined(J1708_PROTOCOL)
		#error "Wrong combination of user macros: IBIS_PROTOCOL + J1708_PROTOCOL"
	#endif

	#if defined(GLORIA)
		#error "Wrong combination of user macros: IBIS_PROTOCOL + GLORIA"
	#endif

	#if defined(SW4) && !defined(DEVTEST)
		#error "Wrong combination of user macros: IBIS_PROTOCOL + SW4 without symbol DEVTEST"
	#endif
	#if defined(SW2) && !defined(DEVTEST)
		#error "Wrong combination of user macros: IBIS_PROTOCOL + SW2 without symbol DEVTEST"
	#endif

	#if !defined(NO_LOGGER) && !defined(DEVTEST)
		#error "Wrong combination of user macros: IBIS_PROTOCOL + not NO_LOGGER without symbol DEVTEST"
	#endif
//	#if !defined(NO_LOGGER)
//		#error "Logger is not allowed for IBIS_PROTOCOL projects"
//	#endif

	#if defined(IBIS9600) && defined(IBIS19200)
		#error "Wrong combination of user macros: IBIS9600 + IBIS19200"
	#endif
#else
	#if defined(IBIS9600) || defined(IBIS19200)
		#error "Wrong combination of user macros: IBIS-baudrate definition without symbol IBIS_PROTOCOL"
	#endif

	#if defined(IBIS_IRMA4) || defined(IBIS_STATUS6) || defined(IBIS_INEO) || defined(IBIS_SNIFFER)
		#error "Wrong combination of user macros: IBIS-protocol characteristic without symbol IBIS_PROTOCOL"
	#endif
#endif

#if defined(J1708_PROTOCOL)
	#if defined(GLORIA)
		#error "Wrong combination of user macros: J1708_PROTOCOL + GLORIA"
	#endif

	#if defined(SW4)
		#error "Wrong combination of user macros: J1708_PROTOCOL + SW4"
	#endif
	#if defined(SW2)
		#error "Wrong combination of user macros: J1708_PROTOCOL + SW2"
	#endif

	#if !defined(NO_LOGGER)
		#error "Logger is not allowed for J1708_PROTOCOL projects"
	#endif

	#if defined(J1708_ORBITAL)
		#if defined(UJ1708IP)
			#error "Wrong combination of user macros: J1708_ORBITAL + UJ1708IP"
		#endif

		#if UJ1708IPCFG >= 1 && UJ1708IPCFG <= 4
			#error "UJ1708IPCFG is allowed only when UJ1708IP is defined"
		#endif
	#elif defined(UJ1708IP)
		#if UJ1708IPCFG < 1 || UJ1708IPCFG > 4
			#error "Wrong value for symbol UJ1708IPCFG"
		#endif
	#else
		#error "It hasn't been defined either J1708_ORBITAL nor UJ1708IP"
	#endif
#else
	#if defined(J1708_ORBITAL) || defined(UJ1708IP)
		#error "Wrong combination of user macros: J1708-protocol definition without symbol J1708_PROTOCOL"
	#endif
#endif

#if defined(NO_LOGGER)
	#if defined(DOOR_OPEN_EVENT)
		#error "Wrong combination of user macros: NO_LOGGER + DOOR_OPEN_EVENT"
	#endif

	#if defined(NO_COUNTER_BACKUP)
		#error "Wrong combination of user macros: NO_LOGGER + NO_COUNTER_BACKUP"
	#endif

	#if defined(LIMIT_EVENT_NO)
		#error "Wrong combination of user macros: NO_LOGGER + LIMIT_EVENT_NO"
	#endif
//#else
//	#if defined(SW4)
//		#error "Wrong combination of user macros: symbol SW4 without symbol NO_LOGGER"
//	#endif
//	#if defined(SW2)
//		#error "Wrong combination of user macros: symbol SW2 without symbol NO_LOGGER"
//	#endif
#endif

#if defined(GLORIA)
	#if defined(NO_LOGGER)
		#error "Wrong combination of user macros: GLORIA + NO_LOGGER"
	#endif
	#if defined(SW4)
		#error "Wrong combination of user macros: GLORIA + SW4"
	#endif
	#if defined(SW2)
		#error "Wrong combination of user macros: GLORIA + SW2"
	#endif
#else
	#if defined(SIMULATE_DOOR_SWITCH)
		#error "Wrong combination of user macros: symbol SIMULATE_DOOR_SWITCH without symbol GLORIA"
	#endif

	#if defined(GLORIA_UIP_GSM)
		#error "Wrong combination of user macros: symbol GLORIA_UIP_GSM without symbol GLORIA"
	#endif

	#if defined(I31_RELAX)
		#error "Wrong combination of user macros: symbol I31_RELAX without symbol GLORIA"
	#endif
#endif

#if defined(SW4)
	#if defined(SW2)
		#error "Wrong combination of user macros: SW4 + SW2"
	#endif
#endif
