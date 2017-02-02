/*==============================================================================================*
 |       Filename: kernel_interface.h                                                           |
 | Project/Module: A21                          	                                            |
 |           Date: 03/20/2012 (to be updated on any change)                                     |
 |                                                                                              |
 |----------------------------------------------------------------------------------------------|
 |    Description: Interface of module group Kernel to module group Communication.              |
 |                                                                                              |
 *==============================================================================================*
 |                     i r i s - G m b H   infrared & intelligent sensors                       | 
 *==============================================================================================*/


#ifndef KERNEL_INTERFACE_INC			/* Make sure that this file is only included once		*/
										/* within a module.								 		*/


/*----- Including of Header Files --------------------------------------------------------------*/
//#include <reg167cr.h>

#include "kernel_defines.h"
#include "configuration_defines.h"
#include "uip_defines.h"


// Functions Disable_All_INTs and Enable_All_INTs are intended to create uninterruptable
// sequences in functions not being part of interrupt service routines.

// Functions Save_ILVL_and_Disable_All_INTs and Restore_ILVL are intended to create
// uninterruptable sequences in functions which may be part of interrupt service routines.

// Refer to application note AP16009 by Infineon.
//_inline void
//Disable_All_INTs(void)
//{
	// Set CPU priority level ILVL to highest value 15.
//	_bfld(PSW, 0xF000, 0xF000);
//	_nop();
//}

//_inline void
//Enable_All_INTs(void)
//{
	// Set CPU priority level ILVL to lowest value 0.
//	_bfld(PSW, 0xF000, 0x0000);
//}

// Uninterruptable sequences beginning with call of "Save_ILVL_and_Disable_..._INTs" and ending with
// call of "Restore_ILVL" may be nested provided that ILVL is always i n c r e a s e d by functions 
// "Save_ILVL_and_Disable_..._INTs".

// Example for valid nesting:
// Save_ILVL_and_Disable_T1_and_lower_priority_INTs();
// ...
// Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs();
// ...
// Save_ILVL_and_Disable_SSC_and_lower_priority_INTs();
// ...
// Save_ILVL_and_Disable_All_INTs();
// ...
// Restore_ILVL
// ...
// Restore_ILVL
// ...
// Restore_ILVL
// ...
// Restore_ILVL

// Example for invalid nesting:
// Save_ILVL_and_Disable_All_INTs();
// ...
// Save_ILVL_and_Disable_T1_and_lower_priority_INTs();
// ...
// Restore_ILVL
// ...
// Restore_ILVL

// Functions "Save_ILVL_and_Disable_..._INTs" and "Restore_ILVL" may be called from interrupt service
// routines too. However it must be ensured that ILVL is not decreased below ILVL value according to
// interrupt serviced.

inline void
Save_ILVL_and_Disable_All_INTs(void)
{
 //CMB:
}

inline void
Restore_ILVL(void)
{
 // CMB:
}


/*----- kernel_interface.c ---------------------------------------------------------------------*/
dword get_dword(byte *Buffer);
void uppercase(char *str_ptr);
void copy_str(char *des_ptr, char *src_ptr, word str_len);


/*----- serio_asc0.c ---------------------------------------------------------------------------*/
#define MaxJ1708MsgLen			20

extern char Current_ASC0_Prot_Id;

extern bool ASC0_Service_Interface_OccupiedB;
extern bool ASC0_Debug_ModeB;

extern RS232_Parameter_type stASC0_actual_parameter;
extern RS232_Parameter_type stRS232_IRMA_Standard;

extern ASC0Error_type  ASC0Error;

// Functions Disable_ASC0_and_lower_priority_INTs and Enable_All_INTs are intended to create
// uninterruptable sequences in functions not being part of interrupt service routines.

// Functions Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs and Restore_ILVL are intended to
// create uninterruptable sequences in functions which may be part of interrupt service routines.

// Refer to application note AP16009 by Infineon.
//_inline void
//Disable_ASC0_and_lower_priority_INTs(void)
//{
	// Set CPU priority level ILVL to value assigned to ASC0 interrupts.
//	_bfld(PSW, 0xF000, 0xB000);
//	_nop();
//}

inline void
Save_ILVL_and_Disable_ASC0_and_lower_priority_INTs(void)
{
	// Save PSW on stack, set CPU priority level ILVL to value assigned to ASC0 interrupts and
	// IEN to 1.
//#pragma asm
//	SCXT PSW,#0B800h
//	NOP
//#pragma endasm
}

void ActivateASC0SysTransm(void);
void DeactivateASC0SysTransm(void);
void ActivateASC0SendDelay(void);
void DeactivateASC0SendDelay(void);

bool ASC0_baudrate_valid(dword desired_baudrate);
word InitASC0(RS232_Parameter_type *parameter, byte prot_id);
bool j1708_prot_active(void);

void Check_Send_Data_ASC0(void);
int CheckChar_ASC0(void);
int GetChar_ASC0(void);
#ifdef J1708_PROTOCOL
void NextJ1708RecMsgStart(void);
#endif
word PutByte_ASC0(byte c);
word PutChar_ASC0(char c);
word PutWord_ASC0(word c);
word PutString_ASC0(const char *data);
word WriteData_ASC0(const byte *data, word size);
bool WaitForEndOfTransmission(word Release_TO_ms, word Flush_TO_ms);
void Clear_TerminalSymbol(void);

void Change_ASC0_Debug_Mode(bool valueB);

void Update_ASC0TimeoutRef(void);
bool ASC0TimeoutCond(void);
bool ASC0_Transmission_Finished(void);


/*----- time.c ---------------------------------------------------------------------------------*/
extern volatile bool timer_and_timeout_jobs_disabledB;
extern volatile byte timer_and_timeout_job_disabling_counter;

inline void
disable_t1_interrupt(void)
{
    // CMB:
	//T1IE = DISABLE;
	//_nop();
}

inline void
enable_t1_interrupt(void)
{
    // CMB
	//T1IE = ENABLE;
}

inline void
disable_timer_and_timeout_jobs(void)
{
	timer_and_timeout_jobs_disabledB = TRUE;
	timer_and_timeout_job_disabling_counter++;
}

inline void
enable_timer_and_timeout_jobs(void)
{
	timer_and_timeout_job_disabling_counter--;
	if( timer_and_timeout_job_disabling_counter == 0 )
		timer_and_timeout_jobs_disabledB = FALSE;
}

inline void
Save_ILVL_and_Disable_T1_and_lower_priority_INTs(void)
{
	// Save PSW on stack, set CPU priority level ILVL to value assigned to T1 interrupt and
	// IEN to 1.
    // CMB
//#pragma asm
//	SCXT PSW,#2800h
//	NOP
//#pragma endasm
}

void Set_RTC_Sync_Status(bool);
bool Get_RTC_Sync_Status(void);

int Get_Date_Time(Date_type *, Time_type *);
bool Set_Date_Time(Date_type *, Time_type *, Date_type *, Time_type *);

void add_clock_job(word, void *);
void remove_clock_job(void *);

word add_timer_job(word, void *);
bool remove_timer_job(void *);

word add_timeout_job(word ms, void *);
word reload_timeout_job(void *);
bool remove_timeout_job(void *);

bool Get_Timeout_Flag(void);
word Add_Def_Timeout_Job(word ms);
bool Remove_Def_Timeout_Job(void);

word StartTimeOutIntv(word TO_ms);
bool TimeOutIntvElapsed(void);
void CancelTimeOutIntv(void);
void Idle_ms(word ms);

word time_ms(word time_us);


/*----- sensor_serial.c ------------------------------------------------------------------------*/
#ifdef SERIAL_SENSOR
// Refer to application note AP16009 by Infineon.
//_inline void
//Disable_SSC_and_lower_priority_INTs(void)
//{
	// Set CPU priority level ILVL to value assigned to SSC interrupts.
//	_bfld(PSW, 0xF000, 0xC000);
//	_nop();
//}

_inline void
Save_ILVL_and_Disable_SSC_and_lower_priority_INTs(void)
{
	// Save PSW on stack, set CPU priority level ILVL to value assigned to SSC interrupts and
	// IEN to 1.
#pragma asm
	SCXT PSW,#0C800h
	NOP
#pragma endasm
}

void Init_Sensors_SSC(void);
void Stop_Sensors_SSC(void);
void PWM_Start(void);
void PWM_Stop(void);
#endif


/*----- kernel_interface.c ---------------------------------------------------------------------*/
byte Get_FA_Index(byte fa_addr);

void get_firmware_name(bool get_inherent_firmware_nameB, byte *res_buff, byte *res_buff_pos);
bool is_inherent_firmware_name(void);

void Set_Runlevel(byte rlv);
byte Get_Runlevel(void);
bool Runlevel_4_or_5(void);

void Set_ParserParameter(byte terminalsymbol, word timeout, void *fptr, void *csptr);
void Set_Parser_function(void *ptr);
word Get_Parser_timeout(void);
void Reset_Parser_Flags(void);

bool SectorValid(byte sector);

void SensorData_alloc(void **data, void **write, word *buffer_size);

void Serial_Out_Buffer_Alloc(void **data, void **read, void **write, word *buffer_size);


/*----- main.c ---------------------------------------------------------------------------------*/
extern byte FlashSectorCnt;
extern SectorInformationTable_type SectorInformation[];

extern Operation_Flags_type Operation_Flags;

extern A21_Status_type A21_Status;

extern int (*RunLevel3Main)(void);
extern int (*RunLevel5Main)(void);

extern bool Reset_Request_ReceivedB;

extern bool Recovery_Mode;

#if defined(CAN_SENSOR) || defined(GATEWAY)
extern byte can_sensor_init_status;
#endif

bool Add_Runlevel_Job(void *ptr);
bool Remove_Runlevel_Job(void *ptr);
void Call_Runlevel_Jobs(void);
void Reset_System(void);
void Service_RunLevel_1(void);


/*----- peripherals.c --------------------------------------------------------------------------*/
#define A21_Digital_IO_Cnt			8	// P2.00...P2.07

extern Channel_type Channel[A21_Digital_IO_Cnt];
extern bool ChannelState_ChangedB[A21_Digital_IO_Cnt];

char Get_Channel(byte chan_no);
#ifndef NO_LOGGER
bool RTC_Error(void);
bool Logger_Memory_Error(void);
void Set_Logger_Data_Status(bool);
bool Logger_Empty(void);
void Set_Logger_Overflow_Status(bool);
bool Logger_Overflow(void);
#endif


/*----- flash.c --------------------------------------------------------------------------------*/
bool FlashProg_Allowed(dword MemAddr);
int Erase_Sector(byte  *);
int Program_Flash(int length, byte  *source, byte  *destination);

#ifdef ENABLE_FLASH_ERROR_LOG
void Log_Flash(char *text);
void Erase_Log_Flash(void);
#endif


/*----- signal.c -------------------------------------------------------------------------------*/
#ifdef SERIAL_SENSOR
	#define S8ypvv_ElemCnt_Sig					11
	#define S8ypvv_ElemCnt_Max					SENSOR_ELEMENTS_MAX

//	#define DefS8xxxxCycleTime					22900
#endif

#ifdef CAN_SENSOR
	#define Dist4_DistElemCnt					4
	#define Dist4_SignElemCnt					8
	// Usage of preprocessor macro "#define identifier expression":
	// If "identifier" is found by preprocessor it is replaced by "expression". As a result
	// brackets enclosing "expression" always have to be used to ensure intended evaluation
	// of expression.
	#define Dist4_ElemCnt_Def					(Dist4_DistElemCnt + 1)
	#define Dist4_ElemCnt_Emc					(Dist4_SignElemCnt + 1)

//	#define DefDist4CycleTime					4000
//	#define CanCycleTimeFactor					5
//	#define CanCycleTimeInstMode_ms				((DefDist4CycleTime * CanCycleTimeFactor) / 1000)

	#define Cxyo_SignElemCnt					4
#endif

#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
typedef struct
{
	word value;
	word cycle_count;
	byte table_index;
	byte sensor_idx;
	byte element_adr;
	char element_type;
} SensorData_type;

extern volatile bool SensorData_readyB;
extern volatile SensorStatus_type SensorStatus[SENSORS_MAX];
extern volatile bool initial_sensor_commutestB;

void Init_Signal_Buffer(void);
void Set_Signal_Buffer_Overflow(bool);
bool Signal_Buffer_Overflow(void);
#endif


/*----- configuration.c ------------------------------------------------------------------------*/
extern dword fCPU_div_32;
extern bool fCPU_div_32_validB;
extern byte MaxFACnt;
extern byte MaxSensorCnt;
extern bool Height_Classification_SupportedB;
#if defined(DEVTEST) || defined(CAN_SENSOR)
extern byte Elements_per_sensor;
#endif
#if defined(CAN_SENSOR) && !defined(CXYO)
extern DIST408_requiredB[SENSORS_MAX];
#endif

void Setup_Config_Data(void);
bool Rigth_Configuration_Version(void);
void Configuration_Data_Alloc(void **, void **);
void setup_fa_index_table(void);
void setup_elem_index_table(void);

#ifdef J1708_PROTOCOL
void ExtractA21ClassIdStr(char *A21ClassIdStr);
#endif

#if defined(IBIS_PROTOCOL) || defined(J1708_PROTOCOL)
void CreateDevNoStr(char DelimChar, char *DevNoStr);
void CreateFAConfStr(byte FACnt, char *FAConfStr);
#endif

byte get_height_classes_count(void);

#ifdef DEVTEST
byte Elements_per_FA(byte FAIdx);
byte Get_ElemIdx(byte SenIdx, byte ElemAddr);
#endif


/*----- serio_ssc.c ----------------------------------------------------------------------------*/
#if defined(A21CL) && !defined(NO_LOGGER)
bool DataFlash_Read(dword, word, byte *);
bool DataFlash_Write(dword, word, byte *);
void DS1305_RAM_Read(byte, byte, byte *);
void DS1305_RAM_Write(byte, byte, byte *);
void Write_Time_Synchronized_Date(Date_type *d);
void Read_Time_Synchronized_Date(Date_type *d);
#endif


/*----- logger_kernel.c ------------------------------------------------------------------------*/
typedef struct
{
	dword First;
	dword Last;
} Record_Range_type;

typedef struct
{
	BitField InitializedB			: 1;
	BitField LockedB            	: 1;
	BitField Locked_Before_ErrorB   : 1;
	BitField Confirmation_ExpectedB	: 1;
	BitField ErrorB					: 1;
	Record_Range_type Record_Range;
	word Corrupt_Data_Records;
	byte Reset_Request_Source;
} Logger_Info_type;

typedef struct
{
	word Minutes;
	word Seconds;
} Logger_Interval_type;

extern Logger_Info_type Logger_Info;
extern Logger_Interval_type Logger_Interval;

void Set_Logger_Locked_State(byte parameter);
byte Get_Logger_Locked_State(void);
void Unlock_Logger(void);

void Logger_Memory_Read(dword MemAddr, byte *Buffer, byte Length);
int Logger_Memory_Write(dword Destination, byte *Source, byte Length);

bool Valid_Logger_Address(dword MemAddr);
bool Valid_Logger_Range(dword MemAddr, dword MemAddrEnd);
bool Logger_Write_Allowed(dword MemAddr);


/*----- uip_kernel.c ---------------------------------------------------------------------------*/
#define MinEmbIRMAL7MsgLen		2		// DL according to IRMA protocol specification.
#define MaxEmbIRMAL7MsgLen		15		//
#define J1708EmbIRMAOffsLen		(MaxJ1708MsgLen - MaxEmbIRMAL7MsgLen)

#ifdef GLORIA

/* Specific Codes for "e" Data Records v1.0 with
   <Event_Code> = EventCode_GLORIA_COMM_ERROR */
#define UNKNOWN_GLORIA_PROTOCOL		0x01
#define HW_INFO_FAILED				0x02
#define EXT_HW_INFO_FAILED			0x03
#define GMS_VERSION_FAILED			0x04
#define GMS_RLV_SET_FAILED			0x05
#define EXT_INFO_FAILED				0x06
#define GMS_RLV_REQ_FAILED			0x07
#define STATUS_QUERY_FAILED			0x08

typedef struct
{
	BitField Compatibility_ModeB            : 1;
	BitField Startup_Sequence_In_ProgressB	: 1;
	BitField Startup_Sequence_InterruptedB 	: 1;
	BitField IntNetwork_HandshakeB			: 1;
	BitField IntNetwork_Handshake_TimeoutB	: 1;
	BitField Hardware_Info_RequestB 		: 1;
	BitField ExtHardware_Info_RequestB		: 1;
	BitField Software_Version_RequestB		: 1;
	BitField GMS_RunLevel_SettingB			: 1;
	BitField ExtInfo_TransferB				: 1;
	BitField ExtInfo_Transfer_v21B			: 1;
	BitField GMS_RunLevel_RequestB			: 1;
	BitField Status_QueryB					: 1;
	BitField No_First_Status_ReceptionB     : 1;
	BitField GPS_QueryB						: 1;
	BitField Log_GPS_DataB					: 1;
	BitField Communication_ErrorB			: 1;
	BitField Information_ExpectedB			: 1;
	BitField Hardware_ExpectedB				: 1;
	BitField ExtHardware_ExpectedB			: 1;
	BitField Software_Version_ExpectedB		: 1;
	BitField GMS_RunLevel_2_ExpectedB		: 1;
	BitField ExtInfo_End_ExpectedB			: 1;
	BitField GMS_RunLevel_4_ExpectedB		: 1;
	BitField Status_ExpectedB				: 1;
	BitField GPS_ExpectedB					: 1;
	BitField GMS_RunLevel_0_ExpectedB		: 1;
	BitField Check_ExtNetwork_CommB			: 1;
	BitField Start_TC65_WorkaroundB			: 1;
	BitField Reset_TimeoutB					: 1;
	BitField Restart_CommunicationB			: 1;
	BitField ExtNetwork_Message_ReceivedB	: 1;
	#ifdef GLORIA_UIP_GSM
	BitField Reset_GMS_After_Init_ErrorB	: 1;
	#endif
} GLORIA_Flags_type;
#endif

typedef struct
{
#ifdef GATEWAY
	void (*call_function)(byte, byte, bool, byte, byte, char, word, byte *);
#else
	void (*call_function)(byte, byte, byte, byte *);
#endif
} IRMACmdTable_type;

typedef void (*Flags_Set_Func_type)(void);

#ifndef NO_LOGGER				
typedef void (*RunLevel_Log_Func_type)(byte, byte);
typedef void (*Logger_Set_Time_Func_type)(void);
#endif

extern bool IRMA_Frame_ReceivedB;

extern byte IRMARecBuff[UIP_FRAME_LENGTH_MAX];
extern int  IRMARecBuffPos;
extern int  IRMARecBuffEnd;

#ifdef GATEWAY
extern byte uip_service_lev;
extern bool uip_retry_flagB;
#endif

extern byte IrmaDesAddr;
extern byte IrmaSrcAddr;

#ifdef GATEWAY
extern bool is_uip_20_sendB;
extern byte uip_service_lev_send;
extern byte uip_sub_cmd_send;
#endif
extern byte IRMASendBuff[UIP_FRAME_LENGTH_MAX];
extern word IRMASendBuffWriteStart;
extern word IRMASendBuffWritePos;

extern void (*extended_uip_link)(void);
extern void (*uip_10_frame_process)(int);

#ifndef GLORIA
extern bool Routing_To_SysInterf_ActiveB;
#endif

#ifdef GLORIA
extern byte GLORIASendBuff[UIP_10_FRAME_LENGTH_MAX];
extern byte GLORIASendBuffWritePos;
extern byte IRMA_ASC0_Overrun_Errors;
extern GLORIA_Flags_type far GLORIA_Flags;
extern byte GLORIA_Communication_Error_Counter;
extern byte GLORIA_Communication_Error_Code;
#endif

#if defined(DEVTEST) && defined(GLORIA) || defined(CAN_SENSOR) || defined(GATEWAY)
extern byte IrmaASrcAddr;
#endif

#ifdef DEVTEST
	#ifdef GLORIA
extern bool Resp_to_EmbMsg_ExpectedB;
extern int  EmbMsgDI;
extern byte EmbMsgDesAddr;
extern byte EmbMsgSrcAddr;
	#endif
extern byte MaxSectionPayloadLen;
extern word SectionTransmissionDelay;
#endif

extern byte IRMA_M_MemByteCnt_UppLim;

extern Flags_Set_Func_type Flags_Set_Func;

#ifndef NO_LOGGER				
extern RunLevel_Log_Func_type RunLevel_Log_Func;
extern Logger_Set_Time_Func_type Logger_Set_Time_Func;
#endif

#ifdef GLORIA
extern byte GLORIA_Software_Version[IRMA_Vv20_PayloadLen];
#endif

void Prepare_IRMA(void);
#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR) || defined(OBC)
void init_uip_10_kernel(void);
#endif
#ifdef GATEWAY
void init_uip_kernel(void);
#endif

void Switch_IRMA_Prot_Check(void);

void reset_uip_frame_reading(void);
void abort_uip_frame_reading(void);

byte calculate_irma_checksum(byte *frame, word framelen);

bool get_char_asc0_uip(void);

#if defined(CAN_SENSOR) || defined(SERIAL_SENSOR)
void uip_10_frame_process_a21(int IRMARecBuffEndBackup);
#endif

void Put_word_IRMASendBuff(word value);
void Put_dword_IRMASendBuff(dword value);

void init_uip_send_buff_writing_10(char DataId, byte DataVer, byte PayloadLen);
void send_uip_10_frame(byte DesAddr, char DataId, byte DataVer, byte PayloadLen, byte *Payload);
void send_buffered_uip_10_frame(byte DesAddr);
#ifdef GATEWAY
void init_uip_send_buff_writing_20(byte service_level, char DataId, byte DataVer, char sub_cmd, word PayloadLen);
void send_uip_20_frame(byte service_level, byte DesAddr, char DataId, byte DataVer, char sub_cmd, word PayloadLen, byte *Payload);
void send_buffered_uip_20_frame(byte DesAddr);
#endif
void repeat_last_uip_frame_transmission(void);

#ifdef GLORIA
void Init_GLORIA_SendBuff(void);
void Init_GLORIA_SendBuff_Writing(byte DesAddr, char DataId, byte DataVer, byte PayloadLen);
void Send_GLORIA_Frame(void);
void Send_GLORIA_Query(byte DataId, byte DataVer);
#endif

byte Init_SendBuff_Writing(byte DesAddr, char DataId, byte DataVer, byte PayloadLen, byte **SendBuff);
void Send_Frame(byte DesAddr, byte *SendBuff);

#ifdef DEVTEST
void init_multisection_irma_frame_transmission(byte DesAddr);
#endif

void Build_IRMA_Error_v10(byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData);
#ifdef GLORIA
void Build_GLORIA_Error_v10(byte DesAddr, byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData);
#endif
void Send_IRMA_Error_v10(byte DesAddr, byte ErrDataId, byte ErrNo, byte ErrDataLen, byte *ErrData);
void Send_Error_InvL7Length(byte DesAddr, byte ErrDataId, byte PayloadLen);
void Send_IRMA_Error_v11(byte DesAddr, byte ErrDataId, byte ErrDataVer, byte ErrNo, byte ErrDataLen, byte *ErrData);

bool Query_FAIdx_ValidB(byte PayloadLen, byte *Payload, byte *FAIdx);
#ifndef DEVTEST
bool Query_HCLims_ValidB(byte *Payload, byte *HCIdx);
#endif

/*---------------------------------------------------------------------------------------- F ---*/
void Build_IRMA_F_v10_Payload(void);
byte IRMA_F_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_F_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Flags(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- I ---*/
void Add_IRMA_I_FAInfo(const byte FACnt);
void Build_IRMA_I_v10_v11_Payload(byte DataVer);

/*---------------------------------------------------------------------------------------- J ---*/
byte IRMA_J_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_J_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_IRed(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- M ---*/
byte IRMA_M_v10_Resp(byte PayloadLen, byte *Payload);
#ifdef CAN_SENSOR
byte IRMA_M_v11_Resp(byte PayloadLen, byte *Payload);
#endif
byte IRMA_M_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Memory(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- N ---*/
byte IRMA_N_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_N_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_New(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- O ---*/
void Build_IRMA_O_v10_Payload(byte PortNo);
byte IRMA_O_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_O_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
#if !defined(SW4) && !defined(SW2)
void IRMA_Message_Port(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- P ---*/
byte Get_Writing_Permission(dword MemAddr);
byte IRMA_P_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_P_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Program(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- R ---*/
byte IRMA_R_v10_Resp(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_R_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Run(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- T ---*/
#ifndef NO_LOGGER
void Build_IRMA_T_v10_Payload(void);
byte IRMA_T_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_T_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Time(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);
#endif

/*---------------------------------------------------------------------------------------- U ---*/
byte IRMA_U_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_U_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_RunLevel(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- V ---*/
byte IRMA_V_v20_Resp(byte SrcAddr, byte PayloadLen, byte *Payload, byte **SendBuff);
byte IRMA_V_v20(byte SrcAddr, byte PayloadLen, byte *Payload);
byte IRMA_V_v21_Resp(byte SrcAddr, byte PayloadLen, byte *Payload, byte **SendBuff);
byte IRMA_V_v21(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Version(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

/*---------------------------------------------------------------------------------------- X ---*/
byte IRMA_X_v10_Resp(byte PayloadLen, byte *Payload);
byte IRMA_X_v10(byte SrcAddr, byte PayloadLen, byte *Payload);
void IRMA_Message_Execute(byte SrcAddr, byte DataVer, byte PayloadLen, byte *Payload);

#ifdef GLORIA
void Set_GMS_RunLevel(byte);
void Send_GMS_RunLevel_Request(void);
void Software_Version_Request_Timeout(void);
void Send_IRMA_Version_v20(void);
#endif


#define KERNEL_INTERFACE_INC
#endif	// end of "#ifndef KERNEL_INTERFACE_INC"
