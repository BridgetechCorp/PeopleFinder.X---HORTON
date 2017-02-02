/*==============================================================================================*
 |       Filename: configuration.c                                                              |
 | Project/Module: A21, GATEWAY or OBC/module group Kernel                                      |
 |           Date: 02/07/2011 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Functions to evaluate device immanent configuration data and firmware        |
 |                 parameters stored in Flash EPROM sector 1.                                   |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


/*----- Plausibility Check of Project Settings -------------------------------------------------*/


/*----- Including of Header Files --------------------------------------------------------------*/
#include <reg167cr.h>
#include <stdio.h>
#include <string.h>

#include "..\interrupt_defines.h"
#include "..\kernel_interface.h"
#include "..\kernel\peripherals.h"
#include "configuration_data.h"


/*----- Constant Macro Definitions -------------------------------------------------------------*/


/*----- Variable Type Definitions --------------------------------------------------------------*/


/*----- Memory Class Assignments ---------------------------------------------------------------*/
#pragma class PR=KERNEL
#pragma class CO=KERNEL
#pragma class FB=KERNELRAM
#pragma noclear


/*----- Global Variables -----------------------------------------------------------------------*/
Configuration_System_type	ConfigSystem;
Configuration_Data_type		ConfigData;

dword fCPU_div_32;
bool fCPU_div_32_validB;

byte MaxFACnt;
byte MaxSensorCnt;
bool Height_Classification_SupportedB;

#ifdef DEVTEST
byte Elements_of_Sensor[SENSORS_MAX];
#endif
#if defined(DEVTEST) || defined(CAN_SENSOR)
byte Elements_per_sensor;
#endif

#if defined(CAN_SENSOR) && !defined(CXYO)
DIST408_requiredB[SENSORS_MAX];
#endif

// FA address range: 1...MaxFAAddr.
byte FA2index[MaxFAAddr];
// Element address range: 0x00...MaxElemAddr: Array size [][MaxElemAddr + 1] needed.
byte ElemIdxTable[SENSORS_MAX][MaxElemAddr + 1];


/*----- Function Prototypes --------------------------------------------------------------------*/


/*----- Global Constants -----------------------------------------------------------------------*/


/*----- Implementation of Functions ------------------------------------------------------------*/
void setup_interrupt_controls(void)
{
// Interrupt functions implemented in the same module (source file) use the same register bank
// by default (keyword "_using" not applied). That´s why these interrupt functions may never
// interrupt each other. For that reason according interrupts m u s t be assigned to the same
// interrupt priority level.
// For a certain interrupt priority level associated group priority level m u s t be unique
// for each source of interrupt.
	Save_ILVL_and_Disable_All_INTs();
	//                 +------ ILVL: Interrupt Priority Level (15 highest priority, 0 disabled)
	//                 |  +--- GLVL: Group Level (3 highest priority, 0 lowest priority)
#ifdef SERIAL_SENSOR
	T6IC   = xxIC_VAL(12, 3);		// sampling of serial sensors
	T8IC   = xxIC_VAL(12, 0);		// Timer 8 (CAPCOM2)
#endif
#if defined(CAN_SENSOR) || defined(GATEWAY)
	XP0IC  = xxIC_VAL(12, 0);		// CAN
#endif

	S0RIC  = xxIC_VAL(11, 3);		// ASC0 receive interrupt
	S0TBIC = xxIC_VAL(11, 2);		// ASC0 transmit buffer interrupt
	S0TIC  = xxIC_VAL(11, 1);		// ASC0 transmit interrupt
	T3IC   = xxIC_VAL(11, 0);		// Timer 3 (GPT1)

	T1IC   = xxIC_VAL( 2, 0);		// Timer 1 (CAPCOM1)

	T0IC   = xxIC_VAL( 0, 0);		// Timer 0 (CAPCOM1)
	T2IC   = xxIC_VAL( 0, 0);		// Timer 2 (GPT1)
	T4IC   = xxIC_VAL( 0, 0);		// Timer 4 (GPT1)
	T5IC   = xxIC_VAL( 0, 0);		// Timer 5 (GPT2)
	T7IC   = xxIC_VAL( 0, 0);		// Timer 7 (CAPCOM2)
#ifdef SERIAL_SENSOR
	XP0IC  = xxIC_VAL( 0, 0);		// CAN
#endif
	S0EIC  = xxIC_VAL( 0, 0); 		// ASC0 error interrupt
	SSCRIC = xxIC_VAL( 0, 0);		// SSC receive interrupt
	SSCEIC = xxIC_VAL( 0, 0);		// SSC error interrupt
	SSCTIC = xxIC_VAL( 0, 0);		// SSC transmit interrupt
	CC20IC = xxIC_VAL( 0, 0);
	CC21IC = xxIC_VAL( 0, 0);
	CC22IC = xxIC_VAL( 0, 0);
	CC23IC = xxIC_VAL( 0, 0);
	Restore_ILVL();
}


void buscon_setup(void)
{
	Configuration_Data_Kernel_type *rom_k = (Configuration_Data_Kernel_type *)ADDR_KERNEL_CONFIG;

	if(rom_k->buscon[0] != 0xFFFF)
		BUSCON0 = rom_k->buscon[0];
	if(rom_k->buscon[1] != 0xFFFF)
		BUSCON1 = rom_k->buscon[1];
	if(rom_k->buscon[2] != 0xFFFF)
		BUSCON2 = rom_k->buscon[2];
	if(rom_k->buscon[3] != 0xFFFF)
		BUSCON3 = rom_k->buscon[3];
	if(rom_k->buscon[4] != 0xFFFF)
		BUSCON4 = rom_k->buscon[4];
	if(rom_k->addrsel[0] != 0xFFFF)
		ADDRSEL1 = rom_k->addrsel[0];
	if(rom_k->addrsel[1] != 0xFFFF)
		ADDRSEL2 = rom_k->addrsel[1];
	if(rom_k->addrsel[2] != 0xFFFF)
		ADDRSEL3 = rom_k->addrsel[2];
	if(rom_k->addrsel[3] != 0xFFFF)
		ADDRSEL4 = rom_k->addrsel[3];
}


void IO_Port_2_Setup(void)
{
	byte f, i, n;

#ifdef EMC
	for( f = 0; f < FACntEMC; f++ )
#else
	for( f = 0; f < ConfigData.function_areas; f++ )
#endif
	{
		for( n = 0; n < 2; n++ )
		{
			i = ConfigData.function_area[f].door.input[n].channel_no;
			if( i > 0 && i <= A21_Digital_IO_Cnt )
			{
				Channel[i - 1].ignoreB = FALSE;
				Channel[i - 1].inputB = TRUE;
				Channel[i - 1].duration.chatter.rising  = ConfigData.function_area[f].door.input[n].chatter_time_01;
				Channel[i - 1].duration.chatter.falling = ConfigData.function_area[f].door.input[n].chatter_time_10;
			}
		}
#ifndef DEVTEST
		i = ConfigData.function_area[f].door.output.channel_no;
		if( i > 0 && i <= A21_Digital_IO_Cnt )
		{
			Channel[i - 1].outputB = TRUE;
			switch(i)
			{
				case 1 :	_putbit(TRUE, DP2, 0);
							_putbit(TRUE, P2, 0);
							_putbit(FALSE, P2, 0);
							break;
				case 2 :	_putbit(TRUE, DP2, 1);
							_putbit(TRUE, P2, 1);
							_putbit(FALSE, P2, 1);
							break;
				case 3 :	_putbit(TRUE, DP2, 2);
							_putbit(TRUE, P2, 2);
							_putbit(FALSE, P2, 2);
							break;
				case 4 :	_putbit(TRUE, DP2, 3);
							_putbit(TRUE, P2, 3);
							_putbit(FALSE, P2, 3);
							break;
				case 5 :	_putbit(TRUE, DP2, 4);
							_putbit(TRUE, P2, 4);
							_putbit(FALSE, P2, 4);
							break;
				case 6 :	_putbit(TRUE, DP2, 5);
							_putbit(TRUE, P2, 5);
							_putbit(FALSE, P2, 5);
							break;
				case 7 :	_putbit(TRUE, DP2, 6);
							_putbit(TRUE, P2, 6);
							_putbit(FALSE, P2, 6);
							break;
				case 8 :	_putbit(TRUE, DP2, 7);
							_putbit(TRUE, P2, 7);
							_putbit(FALSE, P2, 7);
							break;
			}
		}
#endif
	}
#ifdef DEVTEST
	#ifdef P2_OUT
	// Keep settings of P2H used in A21CL-3-GLORIA (/CE signals for RTC and DataFlash).
	_bfld(P2, 0x00FF, 0x0055);
	_bfld(DP2, 0x00FF, 0x00FF);	// Pins 0..7 of port P2 configured as outputs.
	#else
	// Keep settings of P2H used in A21CL-3-GLORIA (/CE signals for RTC and DataFlash).
	_bfld(P2, 0x00FF, 0x00FF);
		#if defined(SW4)
	_bfld(DP2, 0x00FF, 0x00F0);	// Pins 4, 5, 6 and 7 of port P2 configured as outputs.
		#elif defined(SW2)
	_bfld(DP2, 0x00FF, 0x00C0);	// Pins 6 and 7 of port P2 configured as outputs.
		#else
	_bfld(DP2, 0x00FF, 0x0000);	// All pins of port P2 configured as inputs.
		#endif
	#endif
#else
	#ifdef DEBUG_P2_7
	_putbit(TRUE, DP2, 7);
	_putbit(TRUE, P2, 7);
	_putbit(FALSE, P2, 7);
	#endif
	#ifdef DEBUG_P2_6
	_putbit(TRUE, DP2, 6);
	_putbit(TRUE, P2, 6);
	_putbit(FALSE, P2, 6);
	#endif
	#ifdef DEBUG_P2_5
	_putbit(TRUE, DP2, 5);
	_putbit(TRUE, P2, 5);
	_putbit(FALSE, P2, 5);
	#endif
	#ifdef DEBUG_P2_4
	_putbit(TRUE, DP2, 4);
	_putbit(TRUE, P2, 4);
	_putbit(FALSE, P2, 4);
	#endif
	#ifdef SW4
	// Keep settings of P2H used in A21CL-3-GLORIA (/CE signals for RTC and DataFlash).
	_bfld(P2, 0x00FF, 0x00FF);
	_bfld(DP2, 0x00FF, 0x00F0);	// Pins 4, 5, 6 and 7 of port P2 configured as outputs.
	#endif
	#ifdef SW2
	// Keep settings of P2H used in A21CL-3-GLORIA (/CE signals for RTC and DataFlash).
	_bfld(P2, 0x00FF, 0x00FF);
	_bfld(DP2, 0x00FF, 0x00C0);	// Pins 6 and 7 of port P2 configured as outputs.
	#endif
#endif
}	// IO_Port_2_Setup


void check_fCPU(void)
{
	dword ModuloVar;

    // Check ConfigSystem.fCPU for being configured well: 
    // <= 33000000, e. g. 16000000, 18432000, 20000000 or 24000000.
	// Intrinsic functions _divu32(), _modu32() not usable because of result may be > 65535.
	fCPU_div_32_validB = ConfigSystem.fCPU > 0;
	if( fCPU_div_32_validB )
	{
		fCPU_div_32 = ConfigSystem.fCPU / 32;
		ModuloVar = ConfigSystem.fCPU - fCPU_div_32 * 32;
		fCPU_div_32_validB = ModuloVar == 0 && 
		                     fCPU_div_32 <= 33000000 / 32 /*1031250*/;
	}
}	// check_fCPU


void determine_FlashSectorCnt(void)
{
	byte device_id; 

	FlashSectorCnt = FLASH_SECTOR_CNT_200;
    device_id = ConfigSystem.EPROM.device_id;
	switch ( ConfigSystem.EPROM.manufacturer_id )
	{
		case 0x01 :	if( device_id == 0xAB)
						FlashSectorCnt = MAX_FLASH_SECTOR_CNT;
					break;

		case 0x04 :	if( device_id == 0xD6)
						FlashSectorCnt = MAX_FLASH_SECTOR_CNT;
					break;

		case 0x20 :	if( device_id == 0xAB)
						FlashSectorCnt = MAX_FLASH_SECTOR_CNT;
					break;
	}
}	// determine_FlashSectorCnt


// Copies device immanent configuration data from Flash EPROM sector 0 to variable ConfigSystem
// in RAM class "KERNELRAM".
void setup_config_system_1_02(void)
{
	Configuration_Data_Kernel_type *rom_k = (Configuration_Data_Kernel_type *)ADDR_KERNEL_CONFIG;

	strcpy(ConfigSystem.product_id, rom_k->product_id);
	ConfigSystem.device_id.year = rom_k->device_id.year;
	ConfigSystem.device_id.serial_number = rom_k->device_id.serial_number;
	strcpy(ConfigSystem.CPU_name, rom_k->CPU_name);
	ConfigSystem.fCPU = rom_k->fCPU;
	ConfigSystem.fOSC = rom_k->fOSC;
	ConfigSystem.EPROM.manufacturer_id = rom_k->EPROM.manufacturer_id;
	ConfigSystem.EPROM.device_id       = rom_k->EPROM.device_id;
	strcpy(ConfigSystem.EPROM.name, rom_k->EPROM.name);
	ConfigSystem.SRAM_size = rom_k->SRAM_size;
	strcpy(ConfigSystem.RTC_name, rom_k->RTC_name);
	ConfigSystem.NVRAM_size = rom_k->NVRAM_size;
	strcpy(ConfigSystem.sensor_interface, rom_k->sensor_interface);
	strcpy(ConfigSystem.system_interface, rom_k->system_interface);
	check_fCPU();
	determine_FlashSectorCnt();
}	// setup_config_system_1_02


#ifdef J1708_PROTOCOL
// Function ExtractA21ClassIdStr isn´t capable to create strings like "CLG"
// for GLORIA.
// Zero terminated string: At least A21ClassIdStrLen + 1 byte to be allocated
// for *A21ClassIdStr.
void ExtractA21ClassIdStr(char *A21ClassIdStr)
{
	char *ptr;
	byte idx = 0;

	A21ClassIdStr[0] = '_';
	A21ClassIdStr[1] = '_';
	A21ClassIdStr[2] = '_';
	A21ClassIdStr[3] = 0;
	ptr = strstr(ConfigSystem.product_id, A21Name);
	if (ptr == NULL)
	{
		// Actual device immanent configuration data: 
		// "C-2-I...", "S-2-R..." or "CL-3-GLORIA..."
		ptr = (char *)ConfigSystem.product_id;
		while (*ptr != 0 && *ptr != '-' && idx < A21ClassIdStrLen)
		{
			A21ClassIdStr[idx++] = *ptr++;
		}
	}
	else
	{
		// Prototypes of product names included in A21 software:
		// "IRMA-A21S", "IRMA-A21C"
		ptr += strlen(A21Name);
		while (*ptr != 0 && *ptr != '-' && idx < A21ClassIdStrLen)
		{
			A21ClassIdStr[idx++] = *ptr++;
		}
	}
}	// ExtractA21ClassIdStr
#endif


#if defined(IBIS_PROTOCOL) || defined(J1708_PROTOCOL)
// Zero terminated string: At least MaxA21DevNoStrLen + 1 byte to be allocated
// for *DevNoStr.
// String size is not exceeded even for values device_id.year == 255 and
// device_id.serial_number == 65535.
void CreateDevNoStr(char DelimChar, char *DevNoStr)
{
	if( SectorValid(SECTOR_KERNEL1) )		// Check for valid device immanent configuration data
	{
		sprintf(DevNoStr, "%02u", ConfigSystem.device_id.year);
		DevNoStr[2] = DelimChar;
		sprintf(&DevNoStr[3], "%04u", ConfigSystem.device_id.serial_number);
	}
	else
	{
		sprintf(DevNoStr, "??");
		DevNoStr[2] = DelimChar;
		sprintf(&DevNoStr[3], "????");
	}
}
#endif


void setup_config_data_1_07(void)
{
	byte FACnt, FAIdx, hc_idx, el, s, i;
	bool ValidB;

	Configuration_Data_ROM_107_type *rom_d = (Configuration_Data_ROM_107_type *)ADDR_CONFIG_GENERAL;
	Softw_Param_Module_Name_type *rom_cn   = (Softw_Param_Module_Name_type *)ADDR_CONFIG_NAME;
	Configuration_IRMA_ROM_type *rom_i     = (Configuration_IRMA_ROM_type *)ADDR_CONFIG_IRMA;
	Cycle_Table_ROM_type *rom_c            = (Cycle_Table_ROM_type *)ADDR_CYCLE_TABLE;
	Function_Area_ROM_104_type *rom_fa     = (Function_Area_ROM_104_type *)ADDR_FUNCTION_AREA;
	Ext_Funct_Area_ROM_type *rom_efa       = (Ext_Funct_Area_ROM_type *)ADDR_EXT_FUNCT_AREA;
	PortPin_FA_ROM_type *rom_pp            = (PortPin_FA_ROM_type *)ADDR_PORT_PIN;
	word *rom_hc_no                        = (word *)ADDR_HEIGHT_CLASSES;
	Height_Classes_ROM_type *rom_hc        = (Height_Classes_ROM_type *)(ADDR_HEIGHT_CLASSES + 2);
	Sensor_Config_ROM_type *rom_s          = (Sensor_Config_ROM_type *)ADDR_SENSOR_CONFIG;
	Ext_Sen_Config_ROM_type *rom_es        = (Ext_Sen_Config_ROM_type *)ADDR_EXT_SEN_CONFIG;
	Sensor_Type_ROM_type *rom_st           = (Sensor_Type_ROM_type *)ADDR_SENSOR_TYPE;
	OBC_Data_ROM_type *rom_obc             = (OBC_Data_ROM_type *)ADDR_OBC_DATA;
	Logger_Config_ROM_type *rom_logger     = (Logger_Config_ROM_type *)ADDR_LOGGER_CONFIG;
	Gloria_ROM_107_type *rom_gloria        = (Gloria_ROM_107_type *)ADDR_GLORIA_CONFIG_107;
	byte *rom_register                     = (byte *)ADDR_REGISTER;

	/* Configuration_Data_ROM */
	ConfigData.version  = rom_d->struct_version;
	ConfigData.revision = rom_d->struct_revision;

	strncpy(ConfigData.configuration_name, rom_cn->name, 15);

	ConfigData.ASC0.baudrate             = rom_d->ASC0.baudrate;
	ConfigData.ASC0.parity               = rom_d->ASC0.parity;
	ConfigData.ASC0.data_bits            = rom_d->ASC0.data_bits;
	ConfigData.ASC0.stop_bits            = rom_d->ASC0.stop_bits;
	ConfigData.ASC0.flow_control         = rom_d->ASC0.flow_control;
	ConfigData.ASC0_control.half_duplexB = rom_d->ASC0_control.half_duplexB;
	ConfigData.ASC0_control.send_delay   = rom_d->ASC0_control.send_delay;

	ConfigData.communication_protocol = rom_d->communication_protocol;

	ConfigData.SSC_baudrate = rom_d->SSC_baudrate;

	for( i = 0; i < 4; i++ )
	{
		ConfigData.PWM[i].period      = rom_d->PWM[i].period;
		ConfigData.PWM[i].pulse_width = rom_d->PWM[i].pulse_width;
		ConfigData.PWM[i].offset      = rom_d->PWM[i].offset;
	}
	
	ConfigData.function_areas = rom_d->function_areas;
	FACnt = ConfigData.function_areas;

	MaxFACnt = 0;
	MaxSensorCnt = 0;
	for( i = 0; i < FUNCTION_AREAS_MAX; i++ )
 		if( rom_fa[i].address > 0 && rom_fa[i].cycle_time > 0 && rom_fa[i].no_of_sensors > 0 )
	 		MaxFACnt++;
	for( i = 0; i < MaxFACnt; i++ )
 		MaxSensorCnt += rom_fa[i].no_of_sensors;

	ConfigData.sensors = 0;
	for( i = 0; i < ConfigData.function_areas; i++ )
 		ConfigData.sensors += rom_fa[i].no_of_sensors;

	ConfigData.sensor_types = rom_d->sensor_types;
	ConfigData.elements.total = 0;
	for( i = 0; i < ELEMENTS_TOTAL; i++ )
		for( FAIdx = 0; FAIdx < ConfigData.function_areas; FAIdx++ )
			if( (FAIdx + 1) == rom_s[rom_c[i].sensor - 1].area[0].function_area )
				ConfigData.elements.total++;

#if defined(DEVTEST) || defined(CAN_SENSOR)
	// Applicable only if number of elements per sensor is the same for all sensors.
    Elements_per_sensor = ConfigData.elements.total / ConfigData.sensors;
#endif

  	ConfigData.Instmode_Timeout_s =	rom_d->Instmode_Timeout_s;

	ConfigData.CAN_Baud_Rate = rom_d->CAN_Baud_Rate;

  	ConfigData.Preferred_Modem = rom_gloria->Preferred_Modem;

	/* Configuration_IRMA_ROM */
	ConfigData.irma.address = rom_i->address;
	strcpy(ConfigData.irma.tag, rom_i->tag);
	ConfigData.irma.status_interval = rom_i->status_interval;

	/* Function_Area_ROM */
#ifdef EMC
	for( i = 0; i < FACntEMC; i++ )
#else
	for( i = 0; i < ConfigData.function_areas; i++ )
#endif
	{
		ConfigData.function_area[i].door.bogotaB = FALSE;
		ConfigData.function_area[i].address = rom_fa[i].address;
		ConfigData.function_area[i].door.type = rom_fa[i].door.info.type;

		// There are IRMA basic and advanced counting modules using parameters
		// ConfigData.function_area[i].door.height and ConfigData.function_area[i].door.stepsB. 
		// That´s why these parameters are preserved for the sake of compatibility.
		// Replacements to be used:
		// ConfigData.ext_funct_area[i].door_height and ConfigData.ext_funct_area[i].door_props.stepsB.
		ConfigData.function_area[i].door.height = rom_fa[i].door.info.height * 10;
		ConfigData.function_area[i].door.stepsB = rom_fa[i].door.info.stepsB;

		ConfigData.function_area[i].door.position.from_externB = rom_fa[i].door.info.position_externB;
		ConfigData.function_area[i].door.position.exactB = rom_fa[i].door.info.position_exactB;
		ConfigData.function_area[i].door.position.simulationB = rom_fa[i].door.info.simulationB;

		for( s = 0; s < 2; s++ )
		{
			ConfigData.function_area[i].door.input[s].channel_no = rom_fa[i].door.input[s].channel_no;
			ConfigData.function_area[i].door.input[s].type.door_switchB = rom_fa[i].door.input[s].door_switchB;
			ConfigData.function_area[i].door.input[s].type.closing_signalB = rom_fa[i].door.input[s].closing_signalB;
			ConfigData.function_area[i].door.input[s].type.fifty_percentB = rom_fa[i].door.input[s].fifty_percentB;
			ConfigData.function_area[i].door.input[s].type.motion_signalB = rom_fa[i].door.input[s].motion_signalB;
			ConfigData.function_area[i].door.input[s].type.left_leafB = rom_fa[i].door.input[s].left_leafB;
			ConfigData.function_area[i].door.input[s].type.right_leafB = rom_fa[i].door.input[s].right_leafB;
			ConfigData.function_area[i].door.input[s].type.door_delayB = rom_fa[i].door.input[s].door_delayB;
			ConfigData.function_area[i].door.input[s].invert_signalB = rom_fa[i].door.input[s].invert_signalB;

			if( ConfigData.function_area[i].door.input[s].type.door_delayB )
			{	/* chatter time in 1/10s, used e.g. for door closure delay */
				if( ConfigData.function_area[i].door.input[s].invert_signalB )	{
					ConfigData.function_area[i].door.input[s].chatter_time_01 = 100 * (WORD)rom_fa[i].door.input[s].chatter_time_10;
					ConfigData.function_area[i].door.input[s].chatter_time_10 = rom_fa[i].door.input[s].chatter_time_01;
				}
				else	{
					ConfigData.function_area[i].door.input[s].chatter_time_10 = 100 * (WORD)rom_fa[i].door.input[s].chatter_time_10;
					ConfigData.function_area[i].door.input[s].chatter_time_01 = rom_fa[i].door.input[s].chatter_time_01;
				}
			}
			else
			{	/* chatter time in ms */
				if( ConfigData.function_area[i].door.input[s].invert_signalB )	{
					ConfigData.function_area[i].door.input[s].chatter_time_01 = rom_fa[i].door.input[s].chatter_time_10;
					ConfigData.function_area[i].door.input[s].chatter_time_10 = rom_fa[i].door.input[s].chatter_time_01;
				}
				else	{
					ConfigData.function_area[i].door.input[s].chatter_time_10 = rom_fa[i].door.input[s].chatter_time_10;
					ConfigData.function_area[i].door.input[s].chatter_time_01 = rom_fa[i].door.input[s].chatter_time_01;
				}
			}
		}

		ConfigData.function_area[i].door.simulation.after_count_wait_time = 0;
		ConfigData.function_area[i].door.simulation.door_closed_time = 0;
		ConfigData.function_area[i].door.simulation.door_event_countdown = 0;

		if( ConfigData.function_area[i].door.position.from_externB ||
			ConfigData.function_area[i].door.input[0].channel_no != 0 ||
			ConfigData.function_area[i].door.input[1].channel_no != 0 )
		{
			ConfigData.function_area[i].door.position.availableB = TRUE;
		}
		else
		{
			ConfigData.function_area[i].door.position.availableB = FALSE;
		}

		ConfigData.function_area[i].door.output.channel_no = rom_fa[i].door.output.channel_no;
		ConfigData.function_area[i].door.output.invert_signalB = rom_fa[i].door.output.invert_signalB;
		ConfigData.function_area[i].door.output.signal_width = rom_fa[i].door.output.signal_width;

		ConfigData.function_area[i].cycle_time = rom_fa[i].cycle_time;
		ConfigData.function_area[i].no_of_sensors = rom_fa[i].no_of_sensors;
		ConfigData.function_area[i].function.countingB = rom_fa[i].function.countingB;
		ConfigData.function_area[i].function.observationB = rom_fa[i].function.observationB;
		ConfigData.function_area[i].function.controlB = rom_fa[i].function.controlB;
		ConfigData.function_area[i].function.heightclassB = rom_fa[i].function.heightclassB;
		ConfigData.function_area[i].function.scalableB = rom_fa[i].function.scalableB;
	}

	/* Height_Classes_ROM */
	Height_Classification_SupportedB = TRUE;
#ifdef EMC
	for( FAIdx = 0; FAIdx < FACntEMC; FAIdx++ )
#else
	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
#endif
		if( !ConfigData.function_area[FAIdx].function.heightclassB )
		{
			Height_Classification_SupportedB = FALSE;
			break;
		}

	if( Height_Classification_SupportedB )
		for( hc_idx = 0; hc_idx < *rom_hc_no; hc_idx++ )
			if( rom_hc[hc_idx].LowLim >= rom_hc[hc_idx].UppLim )
			{
				Height_Classification_SupportedB = FALSE;
				break;
			}

	if( Height_Classification_SupportedB )
	{
		ConfigData.Height_Class_No = MIN(*rom_hc_no, HEIGHT_CLASS_NO_MAX);
		for( i = 0; i < ConfigData.Height_Class_No; i++ )
		{
			ConfigData.Height_Classes_LowLim[i] = rom_hc[i].LowLim;
			ConfigData.Height_Classes_UppLim[i] = rom_hc[i].UppLim;
		}
	}

	/* Sensor_Config_ROM */
	for( i = 0; i < ConfigData.sensors; i++ )
	{
		strcpy(ConfigData.sensor_config[i].type, rom_st[rom_s[i].type_no - 1].type); 
		ConfigData.sensor_config[i].address = rom_s[i].address;
		ConfigData.sensor_config[i].environment.turnedB = rom_s[i].environment.turnedB;
		ConfigData.sensor_config[i].environment.som_specifiedB = rom_s[i].environment.som_specifiedB;
		ConfigData.sensor_config[i].environment.sen_op_mode = rom_s[i].environment.sen_op_mode;
		for( s = 0; s < 3; s++ )
		{
			ConfigData.sensor_config[i].area[s].function_area = rom_s[i].area[s].function_area;
			ConfigData.sensor_config[i].area[s].number = rom_s[i].area[s].number;
		}
		for( s = 0; s < 0x0B; s++ )
			ConfigData.sensor_config[i].element_type[s] = rom_st[rom_s[i].type_no - 1].element_type[s];
//		ConfigData.sensor_config[i].element_type[0x0B] = 'V';
//		ConfigData.sensor_config[i].element_type[0x0C] = 'V';
//		ConfigData.sensor_config[i].element_type[0x0D] = 'V';
	}

	el = 0;
	for( i = 0; i < ELEMENTS_TOTAL; i++ )
	{
		ValidB = FALSE;
		for( FAIdx = 0; FAIdx < ConfigData.function_areas; FAIdx++ )
			if( (FAIdx + 1) == rom_s[rom_c[i].sensor - 1].area[0].function_area )
				ValidB = TRUE;

	    if( ValidB )
		{
		   ConfigData.cycle_table[el].element = rom_c[i].element;
		   // Cycle table files *.tab contain sensor numbers beginning with 1 while
		   // IRMA OPERA uses sensor index beginning with 0.
		   ConfigData.cycle_table[el].sensor = rom_c[i].sensor - 1;
		   ConfigData.cycle_table[el].element_type = 0;
		   for(s = 0; s < 3; s++)
		   	 ConfigData.cycle_table[el].position[0] = rom_c[i].position[0];
		   ConfigData.cycle_table[el].environment.no_neighbour_leftB = rom_c[i].environment.no_neighbour_leftB;
		   ConfigData.cycle_table[el].environment.no_neighbour_rightB = rom_c[i].environment.no_neighbour_rightB;
		   ConfigData.cycle_table[el].environment.no_neighbour_line_leftB = rom_c[i].environment.no_neighbour_line_leftB;
		   ConfigData.cycle_table[el].environment.no_neighbour_line_rightB = rom_c[i].environment.no_neighbour_line_rightB;
		   ConfigData.cycle_table[el].environment.bar_leftB = rom_c[i].environment.bar_leftB;
		   ConfigData.cycle_table[el].environment.bar_rightB = rom_c[i].environment.bar_rightB;
		   ConfigData.cycle_table[el].environment.bar_line_leftB = rom_c[i].environment.bar_line_leftB;
		   ConfigData.cycle_table[el].environment.bar_line_rightB = rom_c[i].environment.bar_line_rightB;
		   ConfigData.cycle_table[el].environment.outer_elementB = rom_c[i].environment.outer_elementB;
		   el++;
		}
	}

	/* OBC_Data_ROM */
	ConfigData.obc.vehicle_id = rom_obc->vehicle_id;

	/* Logger_ROM */
	ConfigData.logger.minutes = rom_logger->interval.minutes;
	ConfigData.logger.seconds = rom_logger->interval.seconds;

	/* Register_ROM */
	for( s = 0; s < 16; s++ )
		ConfigData.memory[s] = rom_register[s];

	memcpy(ConfigData.ext_funct_area, rom_efa, sizeof(Ext_Funct_Area_ROM_Field_type));
	/*
	for( i = 0; i < ConfigData.function_areas; i++ )
	{
		ConfigData.ext_funct_area[i].door_height = rom_efa[i].door_height;
		ConfigData.ext_funct_area[i].door_width = rom_efa[i].door_width;
	}
	*/

	memcpy(ConfigData.ext_sen_config, rom_es, sizeof(Ext_Sen_Config_ROM_Field_type));
	/*
	for( i = 0; i < ConfigData.sensors; i++	)
	{
		ConfigData.ext_sen_config[i].sensor_subtype = rom_es[i].sensor_subtype;
	}
	*/

// DIST4.08 operation mode 0 is used for burn-in.
#if defined(CAN_SENSOR) && !defined(CXYO) && !defined(DEVTEST)
	for( s = 0; s < ConfigData.sensors; s++ )
		DIST408_requiredB[s] = 
			ConfigData.ext_sen_config[s].sensor_subtype != (byte)'A' ||
			ConfigData.ext_sen_config[s].operation_mode != 3;
#endif
}	// setup_config_data_1_07


void add_gloria_parameters_1_05(void)
{
	Gloria_ROM_105_type *rom_gl = (Gloria_ROM_105_type *)ADDR_GLORIA_CONFIG;

	strcpy(ConfigData.gloria.phone_nr_obc, rom_gl->phone_number_obc);
	strcpy(ConfigData.gloria.phone_nr_base, rom_gl->phone_number_base);
	strcpy(ConfigData.gloria.pin_code, rom_gl->pin_code);
	strcpy(ConfigData.gloria.server_url, rom_gl->server_url);
	strcpy(ConfigData.gloria.gateway_url, rom_gl->gateway_url);
	strcpy(ConfigData.gloria.user_name, rom_gl->user_name);
	strcpy(ConfigData.gloria.password, rom_gl->password);
	ConfigData.gloria.network_type = rom_gl->network_type;
	ConfigData.gloria.connect_interval = rom_gl->connect_interval;
}	// add_gloria_parameters_1_05


void add_gloria_parameters_1_06(void)
{
	Gloria_ROM_106_type *rom_gl = (Gloria_ROM_106_type *)ADDR_GLORIA_CONFIG;

	add_gloria_parameters_1_05();

	ConfigData.gloria.ISM_settings.Response_Delay_Constant = rom_gl->ISM_settings.Response_Delay_Constant;
	ConfigData.gloria.ISM_settings.RF_Channel_Number       = rom_gl->ISM_settings.RF_Channel_Number;
	ConfigData.gloria.ISM_settings.System_ID			   = rom_gl->ISM_settings.System_ID;
	ConfigData.gloria.ISM_settings.Interface_Timeout	   = rom_gl->ISM_settings.Interface_Timeout;
	ConfigData.gloria.ISM_settings.RF_Packet_Size		   = rom_gl->ISM_settings.RF_Packet_Size;
	ConfigData.gloria.ISM_settings.CTS_On				   = rom_gl->ISM_settings.CTS_On;
	ConfigData.gloria.ISM_settings.CTS_On_Hysteresis	   = rom_gl->ISM_settings.CTS_On_Hysteresis;
	ConfigData.gloria.ISM_settings.Transmit_Retries		   = rom_gl->ISM_settings.Transmit_Retries;
	ConfigData.gloria.ISM_settings.Broadcast_Attempts	   = rom_gl->ISM_settings.Broadcast_Attempts;
	ConfigData.gloria.ISM_settings.Stop_Bit_Delay		   = rom_gl->ISM_settings.Stop_Bit_Delay;
	ConfigData.gloria.ISM_settings.Range_Refresh		   = rom_gl->ISM_settings.Range_Refresh;
	ConfigData.gloria.ISM_settings.Server_Client_Mode	   = rom_gl->ISM_settings.Server_Client_Mode;
	ConfigData.gloria.ISM_settings.Auto_Config			   = rom_gl->ISM_settings.Auto_Config;
	ConfigData.gloria.ISM_settings.Full_Duplex			   = rom_gl->ISM_settings.Full_Duplex;
	ConfigData.gloria.ISM_settings.DES_Enable			   = rom_gl->ISM_settings.DES_Enable;
	ConfigData.gloria.ISM_settings.Auto_Destination		   = rom_gl->ISM_settings.Auto_Destination;
	ConfigData.gloria.ISM_settings.Broadcast_Mode		   = rom_gl->ISM_settings.Broadcast_Mode;
	ConfigData.gloria.ISM_settings.Unicast_Only			   = rom_gl->ISM_settings.Unicast_Only;
	ConfigData.gloria.ISM_settings.Auto_Channel			   = rom_gl->ISM_settings.Auto_Channel;
	ConfigData.gloria.ISM_settings.Sync_To_Channel		   = rom_gl->ISM_settings.Sync_To_Channel;
	ConfigData.gloria.ISM_settings.One_Beacon_Mode		   = rom_gl->ISM_settings.One_Beacon_Mode;
	ConfigData.gloria.ISM_settings.RTS_Enable			   = rom_gl->ISM_settings.RTS_Enable;
	ConfigData.gloria.ISM_settings.Modem_Mode			   = rom_gl->ISM_settings.Modem_Mode;
	ConfigData.gloria.ISM_settings.RS485_DE_RE			   = rom_gl->ISM_settings.RS485_DE_RE;
	ConfigData.gloria.ISM_settings.Protocol_Status		   = rom_gl->ISM_settings.Protocol_Status;
	ConfigData.gloria.ISM_settings.Parity_Mode			   = rom_gl->ISM_settings.Parity_Mode;
	ConfigData.gloria.ISM_settings.Receive_API			   = rom_gl->ISM_settings.Receive_API;
	ConfigData.gloria.ISM_settings.Enhanced_API_Enable	   = rom_gl->ISM_settings.Enhanced_API_Enable;
	ConfigData.gloria.ISM_settings.Transmit_API			   = rom_gl->ISM_settings.Transmit_API;
	ConfigData.gloria.ISM_settings.Enhanced_Receive_API	   = rom_gl->ISM_settings.Enhanced_Receive_API;
	ConfigData.gloria.ISM_settings.Send_Data_Complete	   = rom_gl->ISM_settings.Send_Data_Complete;
}	// add_gloria_parameters_1_06


void setup_fa_index_table(void)
{
	byte fa_addr_m1, fa_idx;

	for( fa_addr_m1 = 0; fa_addr_m1 < MaxFAAddr; fa_addr_m1++ )
		FA2index[fa_addr_m1] = 0xFF;

	for( fa_idx = 0; fa_idx < ConfigData.function_areas; fa_idx++ )
		// Use index range 0...MaxFAAddr - 1.
		FA2index[ConfigData.function_area[fa_idx].address - 1] = fa_idx;
}	// setup_fa_index_table


void setup_elem_index_table(void)
{
	byte s, e;

	for( s = 0; s < SENSORS_MAX; s++ )
	{
		for( e = 0; e <= MaxElemAddr; e++ )
			ElemIdxTable[s][e] = 0xFF;
#ifdef DEVTEST
		Elements_of_Sensor[s] = 0;
#endif
	}
	for( e = 0; e < ConfigData.elements.total; e++ )
	{
		s = ConfigData.cycle_table[e].sensor;
		ElemIdxTable[s][ConfigData.cycle_table[e].element] = e;
#ifdef DEVTEST
		Elements_of_Sensor[s]++;
#endif
	}
}	// setup_elem_index_table


void setup_additional_data(void)
{
	byte n, e;
	char *rom_sw = (char *)ADDR_APPLICATION1;

	ConfigData.elements.passive = 0;		/* Number of passive elements							*/
	ConfigData.elements.active = 0;			/* Number of active elements							*/
	ConfigData.elements.neutral = 0;		/* Number of neutral level elements						*/
	ConfigData.elements.distance = 0;		/* Number of distance elements							*/
	ConfigData.elements.test_voltage = 0;	/* Number of test voltage elements						*/
	ConfigData.elements.others = 0;			/* Other elements										*/
	for( n = 0; n < ConfigData.sensors; n++ )
	{
		for( e = 0; e < ELEMENTS_MAX; e++ )
		{
			switch( ConfigData.sensor_config[n].element_type[e] )
			{
				case 'P'  :	ConfigData.elements.passive++; break;
				case 'A'  :	ConfigData.elements.active++; break;
				case 'N'  :	ConfigData.elements.neutral++; break;
				case 'D'  :	ConfigData.elements.distance++; break;
				case 'V'  :	ConfigData.elements.test_voltage++; break;
				case '0'  :
				case 0x00 :
				case 0xFF :	break;
				default   :	ConfigData.elements.others++; break;
			}
		}
	}

#ifndef NO_LOGGER
	/* Check logger memory and RTC and set up parameter */
	Test_Logger_Memory_and_RTC(ConfigSystem.RTC_name);
#endif

	/* Copy Software Filename */
	if( (byte)*rom_sw == 0xFF )
		strcpy(ConfigData.software_filename, "unknown");
	else
		strncpy(ConfigData.software_filename, rom_sw, 63);
	if( A21_Status.Optional_Hardware.NVRAM_ErrorB )
	{
		ConfigData.logger.minutes = 0xFF;	/* Do not log! */
		ConfigData.logger.seconds = 0xFF;
	}
	setup_fa_index_table();
	setup_elem_index_table();
}	// setup_additional_data


void Setup_Config_Data_Default(void)
{
#if defined(IBIS_PROTOCOL)
	#ifdef IBIS_INEO
	ConfigData.ASC0.baudrate             = Def_IBISINEOBaudR;
	ConfigData.ASC0.parity               = 'N';
	ConfigData.ASC0.data_bits            = 8;
	ConfigData.ASC0.stop_bits            = 1;
	#else
	// Refer to comment in header file "kernel_defines.h" related to disabled macro definition
	// IBISBaudR.
	ConfigData.ASC0.baudrate             = Def_IBISBaudR/*IBISBaudR*/;
	ConfigData.ASC0.parity               = 'E';
	ConfigData.ASC0.data_bits            = 7;
	ConfigData.ASC0.stop_bits            = 2;
	#endif
	ConfigData.ASC0.flow_control         = 0;
	ConfigData.ASC0_control.half_duplexB = TRUE;
	ConfigData.ASC0_control.send_delay   = 50;
	ConfigData.communication_protocol    = PROT_ID_IBIS;

#elif defined(J1708_PROTOCOL)
	ConfigData.ASC0.baudrate             = Def_J1708BaudR;
	ConfigData.ASC0.parity               = 'N';
	ConfigData.ASC0.data_bits            = 8;
	ConfigData.ASC0.stop_bits            = 1;
	ConfigData.ASC0.flow_control         = 0;
	ConfigData.ASC0_control.half_duplexB = TRUE;
	ConfigData.ASC0_control.send_delay   = 1;
	ConfigData.communication_protocol    = PROT_ID_J1708;

#else	// IRMA_PROTOCOL
	ConfigData.ASC0.baudrate             = Def_IRMABaudR;
	ConfigData.ASC0.parity               = 'N';
	ConfigData.ASC0.data_bits            = 8;
	ConfigData.ASC0.stop_bits            = 1;
	ConfigData.ASC0.flow_control         = 0;
	ConfigData.communication_protocol    = PROT_ID_IRMA;
	#if defined(GLORIA)
	ConfigData.ASC0_control.half_duplexB = FALSE;
	ConfigData.ASC0_control.send_delay   = 0;
	#else
	ConfigData.ASC0_control.half_duplexB = TRUE;
	ConfigData.ASC0_control.send_delay   = 50;
	#endif
#endif

	ConfigData.irma.address = 1;
}	// Setup_Config_Data_Default


void Setup_Config_Data(void)
{
	/* Copies device immanent configuration data from Flash EPROM sector 0
	   to variable ConfigSystem in RAM class "KERNELRAM" */

	setup_config_system_1_02();

	// Check for valid firmware parameters.
	if( SectorValid(SECTOR_CONFIG_DATA) )
	{
		/* If firmware parameters are valid, they are copied from Flash EPROM
		   sector 1 to variable ConfigData in RAM class "KERNELRAM" */

		setup_config_data_1_07();

		add_gloria_parameters_1_06();

        /*
		switch( ConfigData.version )
		{
			case  1 :	switch( ConfigData.revision )
						{
							case  5 :	add_gloria_parameters_1_05();
										break;
							case  6 :
							case  7 :	add_gloria_parameters_1_06();
										break;
							default :	break;
						}
						break;
			default :	break;
		}
		*/

		setup_additional_data();
	}
	else
	{
		/* If there are no valid firmware parameters, then fill some items of
		   variable ConfigData with default values */

		Setup_Config_Data_Default();
		Recovery_Mode = TRUE;
	}
}	// Setup_Config_Data


// Each revision of A21 firmware parameters is downward compatible to earlier revisions.
// Incompatible upgrade of A21 firmware parameters has to be indicated by increment of
// version number (ConfigData.version).
bool Rigth_Configuration_Version(void)
{
	return( ConfigData.version == FIRMWPARAMVER && ConfigData.revision >= MINFIRMWPARAMREV );
}


#if defined(IBIS_PROTOCOL) || defined(J1708_PROTOCOL)
// Zero terminated string: At least FACnt * 5 byte to be allocated
// for *FAConfStr ("0" ... "255" for FA address).
void CreateFAConfStr(byte FACnt, char *FAConfStr)
{
	byte Len = 0;
	byte FAIdx;
	char c;

	for( FAIdx = 0; FAIdx < FACnt; FAIdx++ )
	{
		c = 'W';		// No information about door state available.
		if( ConfigData.function_area[FAIdx].door.position.availableB )
		{
			c = 'C';	// Door position message at system interface.
			if(	ConfigData.function_area[FAIdx].door.input[1].channel_no != 0 &&
				ConfigData.function_area[FAIdx].door.input[1].type.door_switchB )
			{
				c = 'N';		// Door contact logically negative.
				if( ConfigData.function_area[FAIdx].door.input[1].invert_signalB )
					c = 'P';	// Door contact logically positive.
			}
			// Input 0 characteristic has higher priority and overwrites
			// input 1 characteristic.
			if(	ConfigData.function_area[FAIdx].door.input[0].channel_no != 0 &&
				ConfigData.function_area[FAIdx].door.input[0].type.door_switchB )
			{
				c = 'N';
				if( ConfigData.function_area[FAIdx].door.input[0].invert_signalB )
					c = 'P';
			}
		}

		sprintf(&FAConfStr[Len], "%u%c", ConfigData.function_area[FAIdx].address, c);
		Len = strlen(FAConfStr);

		// If FA configuration item is not the last one terminating zero is
		// overwritten by '-'.
		if( FAIdx < FACnt - 1 )
			FAConfStr[Len++] = '-';
	}
}	// CreateFAConfStr
#endif


byte get_height_classes_count(void)
{
	byte HCCnt;

	HCCnt = 1;
	if( Height_Classification_SupportedB )
		HCCnt = ConfigData.Height_Class_No;

    return(HCCnt);
}	// get_height_classes_count


#ifdef DEVTEST
byte Elements_per_FA(byte FAIdx)
{
	// Only applicable if number of elements per sensor is equal for all configured sensors.
	return(ConfigData.function_area[FAIdx].no_of_sensors * Elements_per_sensor);
	// Alternative:
	// Calculation using Elements_of_Sensor[].
}	// Elements_per_FA


byte Get_ElemIdx(byte SenIdx, byte ElemAddr)
{
	if( SenIdx > SENSORS_MAX || ElemAddr > (MaxElemAddr + 1) )
		return 0xFF;
	else
		return ElemIdxTable[SenIdx][ElemAddr];
}	// Get_ElemIdx
#endif
