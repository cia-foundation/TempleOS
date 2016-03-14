// Main TempleOS header

#help_index ""
extern class CTask;
extern class CDoc;
extern class CCPU;
extern class CHashClass;
extern class CHashFun;
extern class CAOTImportExport;
extern class CHeapCtrl;
extern class CAOTHeapGlbl;
extern class CAOT;
extern class CDC;
extern class CSrvCtrl;
extern class CIntermediateCode;

#define NULL	0
#define TRUE	1
#define FALSE	0
#define ON	1
#define OFF	0
#define MIN_I8 (-0x80)
#define MAX_I8 0x7F
#define MIN_U8 0
#define MAX_U8 0xFF
#define MIN_I16 (-0x8000)
#define MAX_I16 0x7FFF
#define MIN_U16 0
#define MAX_U16 0xFFFF
#define MIN_I32 (-0x80000000)
#define MAX_I32 0x7FFFFFFF
#define MIN_U32 0
#define MAX_U32 0xFFFFFFFF
#define MIN_I64 (-0x8000000000000000)
#define MAX_I64 0x7FFFFFFFFFFFFFFF
#define MIN_U64 0
#define MAX_U64 0xFFFFFFFFFFFFFFFF
#define INVALID_PTR	MAX_I64
#define STR_LEN 144

//(Int to F64 conversion is signed)
//Turn off 80-bit float constants with $LK,"OPTf_NO_BUILTIN_CONST",A="MN:OPTf_NO_BUILTIN_CONST"$.
#define MAX_U64_F64 (0x43F0000000000000(F64))
#define MAX_F64 (0x7FEFFFFFFFFFFFFF(F64))
#define MIN_F64 (0xFFEFFFFFFFFFFFFF(F64))
#define inf	(0x7FF0000000000000(F64))
#define ì	(0x7FF0000000000000(F64))
#define pi	(0x400921FB54442D18(F64))
#define ã	(0x400921FB54442D18(F64))
#define exp_1	(0x4005BF0A8B145769(F64)) //The number "e"
#define log2_10 (0x400A934F0979A371(F64))
#define log2_e	(0x3FF71547652B82FE(F64))
#define log10_2	(0x3FD34413509F79FF(F64))
#define loge_2	(0x3FE62E42FEFA39EF(F64))
#define sqrt2	(0x3FF6A09E667F3BCD(F64))
#define eps	(0x3CB0000000000000(F64))

#help_index "Data Types/Simple"
/*HolyC union structure is treated as a
whole if no member is specified,
similar to bit fields.

See $LK,"EndianI64",A="MN:EndianI64"$() and $LK,"::/Demo/SubIntAccess.CPP"$.
*/
U16i union U16
{
  I8i	i8[2];
  U8i	u8[2];
};

I16i union I16
{
  I8i	i8[2];
  U8i	u8[2];
};

U32i union U32
{
  I8i	i8[4];
  U8i	u8[4];
  I16	i16[2];
  U16	u16[2];
};

I32i union I32
{
  I8i	i8[4];
  U8i	u8[4];
  I16	i16[2];
  U16	u16[2];
};

U64i union U64
{
  I8i	i8[8];
  U8i	u8[8];
  I16	i16[4];
  U16	u16[4];
  I32	i32[2];
  U32	u32[2];
};

I64i union I64
{
  I8i	i8[8];
  U8i	u8[8];
  I16	i16[4];
  U16	u16[4];
  I32	i32[2];
  U32	u32[2];
};

#help_index "Math/Complex;Data Types/Complex"
public class Complex
{
  F64	x,y;
};

#help_index "Data Types/Circular Queue"
public class CQue
{
  CQue	*next,*last;
};

#help_index "Graphics/Data Types/D3I32;Math/Data Types/D3I32;Data Types/D3I32"
public class CD3I32 //Three dimensional I32 pt
{
  I32	x,y,z;
};
public class CQueD3I32 //Que of three dimensional I32 pts
{
  CQueD3I32	*next,*last;
  CD3I32	p;
};
#help_index "Math/Data Types;Data Types"
public class CD2I32 //Two dimensional I32 pt
{
  I32	x,y;
};
public class CD2I64 //Two dimensional I64 pt
{
  I64	x,y;
};
public class CD3I64 //Three dimensional I64 pt
{
  I64	x,y,z;
};
public class CD2 //Two dimensional F64 pt
{
  F64	x,y;
};

#help_index "Math/CD3;Data Types/CD3"
public class CD3 //Three dimensional F64 pt
{
  F64	x,y,z;
};

#help_index "Data Types/Queue Vector"
#define QUE_VECT_U8_CNT		512
public class CQueVectU8
{
  CQueVectU8	*next,*last;
  I64	total_cnt,node_cnt,min_index;
  U8	body[QUE_VECT_U8_CNT];
};

#help_index "Data Types/Fifo"
public class CFifoU8
{
  U8	*buf;
  I64	mask,in_ptr,out_ptr;
};
public class CFifoI64
{
  I64	*buf;
  I64	mask,in_ptr,out_ptr;
};

#help_index "Date/CDate"
#define CDATE_YEAR_DAYS		365.24225
#define CDATE_YEAR_DAYS_INT	36524225
#define CDATE_BASE_DAY_OF_WEEK	0
public I64 class CDate
{
  U32	time;
  I32	date;
};

#help_index "Date;Date/CDate"
public class CDateStruct
{
  U8	sec10000,sec100,sec,min,hour,
	day_of_week,day_of_mon,mon;
  I32	year;
};

#help_index "Math/ODE"
public class COrder2D3
{
  F64	x,y,z,
	DxDt,DyDt,DzDt;
};

#define MSF_INACTIVE		1
#define MSF_FIXED		2
public class CMass
{
  CMass	*next,*last;
  COrder2D3 *state,*DstateDt;

  U0	start;
  U32	flags,num;
  F64	mass,drag_profile_factor;
  U0	saved_state;
  F64	x,y,z,
	DxDt,DyDt,DzDt;
  U0	end;
};

#define SSF_INACTIVE		1
#define SSF_NO_COMPRESSION	2
#define SSF_NO_TENSION		4
public class CSpring
{
  CSpring *next,*last;
  CMass	*end1,*end2;
  F64	f,displacement; //set for you to check

  U0	start;
  U32	flags,num,
	end1_num,end2_num;
  F64	const,rest_len;
  U0	end;
};

//Ordinary Differential Equations
#define ODEF_HAS_MASSES	1
#define ODEF_PAUSED	2
#define ODEF_STARTED	4

public class CMathODE
{
  CMathODE *next,*last;
  I64	flags,n,n_internal;
  CMass	*next_mass,*last_mass;
  CSpring *next_spring,*last_spring;
  F64	drag_v,  //drag proportional to velocity
	drag_v2, //drag proportional to velocity squared
	drag_v3, //drag proportional to velocity cubed
	acceleration_limit, //This clips acceleration
	base_t,
	t,t_scale,
	h,h_min,h_max;

  //This is not precise, just a ballpark.
  //TempleOS CMathODE's are for video games
  //not science.  It bails if it takes
  //too long.
  F64	min_tolerance,max_tolerance;

  F64	tolerance_internal,
	*array_base,
	*state,
	*state_internal,
	*DstateDt,
	*state_scale,
	*initial_state,
	*temp0,*temp1,*temp2,*temp3,
	*temp4,*temp5,*temp6,*temp7;
  CTask	*mem_task,*win_task;
  U0	(*derivative)(CMathODE *o,F64 t,F64 *state,F64 *DstateDt);
  I64	user_data;
};

#help_index "Processor"
//IDT entry types
#define IDTET_TASK		0x05
#define IDTET_IRQ		0x0E
#define IDTET_TRAP		0x0F //Same as IRQ but doesnt do CLI.

//Interrupts
//0x00-0x1F are reserved by Intel
#define I_DIV_ZERO		0x00
#define I_SINGLE_STEP		0x01
#define I_NMI			0x02
#define I_BPT			0x03
#define I_PAGE_FAULT		0x0E
//0x20-0x2F are used for hardware
#define I_TIMER			0x20
//Software Interrupts
#define I_MP_CRASH		0x30
#define I_WAKE			0x31
#define I_DBG			0x32
//See $LK,"ST_INT_NAMES",A="MN:ST_INT_NAMES"$

//You might want to start bwd from
//0xFF for your own interrupts.
#define I_USER			0x40

#define MP_MAX_PROCESSORS	128

#define SYS_START_CR0		0x0031

#define RFLAGf_CARRY		0
#define RFLAGf_PARITY		2
#define RFLAGf_AUX_CARRY	4
#define RFLAGf_ZERO		6
#define RFLAGf_SIGN		7
#define RFLAGf_TRAP		8 //Single Step
#define RFLAGf_INT		9
#define RFLAGf_DIR		10
#define RFLAGf_OVERFLOW		11
#define RFLAGf_IOPL0		12 // I/O Privilege Level
#define RFLAGf_IOPL1		13
#define RFLAGf_NESTED_TASK	14
#define RFLAGf_RESUME		16
#define RFLAGf_V8086		17
#define RFLAGf_ALIGN_CHECK	18
#define RFLAGf_VINT		19 //Virtual Interrupt
#define RFLAGf_VINT_PENDING	20
#define RFLAGf_ID		21

#define RFLAGG_START		0x0000
#define RFLAGG_NORMAL		(1<<RFLAGf_INT)

//Global Descriptor Table
class CGDTEntry
{
  U64	lo,hi;
};

class CGDT
{
  CGDTEntry null;
  CGDTEntry boot_ds;
  CGDTEntry boot_cs;
  CGDTEntry cs32;
  CGDTEntry cs64;	//The $LK,"Charter",A="FI:::/Doc/Charter.TXT"$ says just ring-0.
  CGDTEntry cs64_ring3;	//$LK,"Ring3",A="FI:::/Demo/Lectures/Ring3.CPP"$, in case you want to play around.
  CGDTEntry ds;
  CGDTEntry ds_ring3;
  CGDTEntry tr[MP_MAX_PROCESSORS];
  CGDTEntry tr_ring3[MP_MAX_PROCESSORS];
};

class CSysLimitBase
{
  U16	limit;	//Offset of last byte, not size.
  U8	*base;	//&GDT or &IDT
};

#help_index "Memory/Info"
#define MEM_NUM_E820_ENTRIES	48
#define MEM_E820_ENTRY_SIZE	20
class CMemE820
{
  U8	*base;
  I64	len;
  U8	type,pad[3];
};

#help_index "Compiler/Internal"
//Loader flags
#define LDF_NO_ABSS		1
#define LDF_JUST_LOAD		2
#define LDF_SILENT		4

#define BIN_SIGNATURE_VAL	'TOSB'
class CBinFile
{//$LK,"Bin File Header Generation",A="FF:::/Compiler/CMain.CPP,16 ALIGN"$ by compiler.
  U16	jmp;
  U8	module_align_bits,
	reserved;
  U32	bin_signature;
  I64	module_org,
	patch_table_offset, //$LK,"Patch Table Generation",A="FF:::/Compiler/CMain.CPP,IET_ABS_ADDR"$
	file_size;
};

class CPatchTableAbsAddr
{
  U8	eit_abs_addr;	//$LK,"IET_ABS_ADDR",A="MN:IET_ABS_ADDR"$
  U32	abs_addres_cnt;
  U8	zero;
  U32	abs_addres[1];
};

//$LK,"CAOTImportExport",A="MN:CAOTImportExport"$ Types. Used in PatchTable.
#define IET_END			0
//reserved
#define IET_REL_I0		2 //Fictitious
#define IET_IMM_U0		3 //Fictitious
#define IET_REL_I8		4
#define IET_IMM_U8		5
#define IET_REL_I16		6
#define IET_IMM_U16		7
#define IET_REL_I32		8
#define IET_IMM_U32		9
#define IET_REL_I64		10
#define IET_IMM_I64		11
//reserved
#define IET_REL32_EXPORT	16
#define IET_IMM32_EXPORT	17
#define IET_REL64_EXPORT	18 //Not implemented
#define IET_IMM64_EXPORT	19 //Not implemented
#define IET_ABS_ADDR		20
#define IET_CODE_HEAP		21 //Not really used
#define IET_ZEROED_CODE_HEAP	22 //Not really used
#define IET_DATA_HEAP		23
#define IET_ZEROED_DATA_HEAP	24 //Not really used
#define IET_MAIN		25

#help_index "Boot"
class CKernel
{//Must match $LK,"OSStartUp",A="FF:::/Kernel/KStart.CPP,SYS_BOOT_BASE"$
  CBinFile h;
  U32	jmp,
	boot_src,
	boot_patch_table_base,
	sys_run_level;
  CDate	compile_time;

  U0	start;
  U32	boot_base;
  U16	sys_mem_E801[2];
  CMemE820 sys_mem_E820[MEM_NUM_E820_ENTRIES];
  CGDT	sys_gdt;
  CSysLimitBase sys_gdt_ptr;
  U16	sys_pci_busses;
};

//Run-Levels
#define RLf_16BIT		0
#define RLf_VGA			1
#define RLf_32BIT		2
#define RLf_PATCHED		3
#define RLf_64BIT		4
#define RLf_BOOT_HEAP		5
#define RLf_FULL_HEAP		6
#define RLf_RAW			7
#define RLf_INTERRUPTS		8
#define RLf_BLKDEV		9
#define RLf_MP			10
#define RLf_COMPILER		11
#define RLf_DOC			12
#define RLf_WINMGR		13
#define RLf_REGISTRY		14
#define RLf_HOME		15
#define RLf_AUTO_COMPLETE	16
#define RLf_ADAM_SERVER		17

#define RLF_16BIT		0x00001
#define RLF_VGA			0x00002
#define RLF_32BIT		0x00004
#define RLF_PATCHED		0x00008
#define RLF_64BIT		0x00010
#define RLF_BOOT_HEAP		0x00020
#define RLF_FULL_HEAP		0x00040
#define RLF_RAW			0x00080
#define RLF_INTERRUPTS		0x00100
#define RLF_BLKDEV		0x00200
#define RLF_MP			0x00400
#define RLF_COMPILER		0x00800
#define RLF_DOC			0x01000
#define RLF_WINMGR		0x02000
#define RLF_REGISTRY		0x04000
#define RLF_HOME		0x08000
#define RLF_AUTO_COMPLETE	0x10000
#define RLF_ADAM_SERVER		0x20000

#help_index "Processor"
#define LAPIC_BASE		0xFEE00000

#define LAPIC_APIC_ID		(LAPIC_BASE+0x020)
#define LAPIC_APIC_VERSION	(LAPIC_BASE+0x030)
#define LAPIC_TASK_PRIORITY	(LAPIC_BASE+0x080)
#define LAPIC_ARIBITRATION_PRIORITY (LAPIC_BASE+0x090)
#define LAPIC_PROCESSOR_PRIORITY (LAPIC_BASE+0x0A0)
#define LAPIC_EOI		(LAPIC_BASE+0x0B0)
#define LAPIC_LOG_DST		(LAPIC_BASE+0x0D0)
#define LAPIC_DFR		(LAPIC_BASE+0x0E0)
#define LAPIC_LDR		(LAPIC_BASE+0x0D0)

#define LAPICF_APIC_ENABLED	0x100
#define LAPIC_SVR		(LAPIC_BASE+0x0F0)

#define LAPIC_ISR		(LAPIC_BASE+0x100)
#define LAPIC_TMR		(LAPIC_BASE+0x180)
#define LAPIC_IRR		(LAPIC_BASE+0x200)
#define LAPIC_ICR_LOW		(LAPIC_BASE+0x300)
#define LAPIC_ICR_HIGH		(LAPIC_BASE+0x310)

#define LAPIC_LVT_TIMER		(LAPIC_BASE+0x320)
#define LAPIC_LVT_THERMAL	(LAPIC_BASE+0x330)
#define LAPIC_LVT_PERF		(LAPIC_BASE+0x340)
#define LAPIC_LVT_LINT0		(LAPIC_BASE+0x350)
#define LAPIC_LVT_LINT1		(LAPIC_BASE+0x360)
#define LAPIC_LVT_ERR		(LAPIC_BASE+0x370)

#define MPN_VECT		0x97
#define MP_VECT_ADDR		(MPN_VECT*0x1000)

//I/O APIC Memory mapped window
#define IOAPIC_REG		0xFEC00000 //U8
#define IOAPIC_DATA		0xFEC00010 //U32
//I/O APIC Regs
#define IOAPICID		0x00
#define IOAPICVER		0x01
#define IOAPICARB		0x02
#define IOREDTAB		0x10

//Model specific regs.
#define IA32F_SCE	0x001
#define IA32F_LME	0x100
#define IA32_LAPIC_BASE	0x01B
#define IA32_EFER	0xC0000080
#define IA32_FS_BASE	0xC0000100
#define IA32_GS_BASE	0xC0000101

class CAP16BitInit
{//AP Multicore
  U32	jmp;
  CSysLimitBase ap_gdt_ptr;
};

#help_index "Time/CPU Cycles;Time/HPET;Time/Jiffies"
//High Performance Event Timer
#define HPET_GCAP_ID	(0xFED00000+0x00)
#define HPET_GEN_CONF	(0xFED00000+0x10)
#define HPET_MAIN_CNT	(0xFED00000+0xF0)

public class CCntsGlbls
{
  I64	jiffies,	//$LK,"JIFFY_FREQ",A="MN:JIFFY_FREQ"$
	timer,		//$LK,"SYS_TIMER_FREQ",A="MN:SYS_TIMER_FREQ"$. Use $LK,"SysTimerRead",A="MN:SysTimerRead"$().
	time_stamp_freq,
	time_stamp_kHz_freq,
	time_stamp_freq_initial, //Initial freq, sampled once at boot time.
	HPET_freq,
	HPET_kHz_freq,
	HPET_initial;	//Initial count, sampled at boot time.
  Bool	time_stamp_calibrated;
};

#define JIFFY_FREQ		1000 // Hz
#define CDATE_FREQ		49710 // Hz
#define SYS_TIMER_FREQ		(18333*65536/1000) //Hz
#define SYS_TIMER0_PERIOD	(65536*182/10/JIFFY_FREQ)

#help_index "Call"
//Function Stack Frame
#define SF_RBP	0x00
#define SF_RIP	0x08
#define SF_ARG1	0x10
#define SF_ARG2	0x18
#define SF_ARG3	0x20
#define SF_ARG4	0x28
#define SF_ARG5	0x30
#define SF_ARG6	0x38
#define SF_ARG7	0x40
#define SF_ARG8	0x48

class CRAXRBCRCXRDX
{
  I64	rax,rbx,rcx,rdx;
};

//Asm callable function pointers.
//They work with $LK,"CallExtNum",A="MN:CallExtNum"$() when calling from HolyC.
#define EXT_WIN_TO_TOP		0
#define EXT_WIN_FOCUS		1
#define EXT_HEAPLOG_MALLOC	2
#define EXT_HEAPLOG_FREE	3
#define EXT_DBG_RESUME		4
#define EXT_NUM_ENTRIES		5

#help_index "Processor"
#define DFT_CACHE_LINE_WIDTH	64
#define SEMA_STRUCT_SIZE	DFT_CACHE_LINE_WIDTH

//Semaphores
class CSema
{//I read you're supposed to use one cache line.
//I don't know why.  Might be silly.
  Bool	val;
  U8	pad[SEMA_STRUCT_SIZE-sizeof(Bool)];
};
#define SYS_SEMA_DEBUG			0
#define SYS_SEMA_RECORD_MACRO		1
#define SYS_SEMA_SYS_TIMER		2
#define SYS_SEMA_SYS_DATE		3
#define SYS_SEMA_IN_DEBUGGER		4
#define SYS_SEMA_SND			5
#define SYS_SEMA_HEAPLOG_ACTIVE		6
#define SYS_SEMA_HEAPLOG_LOCK		7
#define SYS_SEMA_REFRESH_IN_PROGRESS	8
#define SYS_SEMA_FLUSH_VGA_IMAGE	9
#define SYS_SEMA_SINGLE_USER		10
#define SYS_SEMA_DSK_CACHE		11
#define SYS_SEMA_FAR_CALL32		12
#define SYS_SEMA_DEV_MEM		13
#define SYS_SEMA_VGA			14
#define SYS_SEMA_UPDATE_WIN_Z_BUF	15
#define SYS_SEMA_TT			16
#define SYS_SEMA_MUTE			17
#define SYS_SEMA_FORCE_WINMGR		18
#define SYS_SEMA_JUST_PUMP_MSGS		19
#define SYS_SEMA_TMBEAT			20
#define SYS_SEMA_FIX			21
#define NUM_SYS_SEMAS			22

#define SYSf_CTRL_ALT_DEL		0
#define SYSf_CTRL_ALT_C			1
#define SYSf_CTRL_ALT_X 		2
#define SYSf_CTRL_ALT_TAB		3

#help_index "Hash"
public class CHash
{
//Hash entries are not removed.  New entries eclipse old by being
  //in front for searches.  Old function code is not freed
  //because a reference might use it.
  CHash	*next;
  U8	*str;
  U32	type,use_cnt;
//use_cnt is never decremented.  It is incremented
  //each time a node is found in a hash search.
  //One place it is used is to tell if a externed function was unused
  //before being defined.  It is also used for popularity measures
  //in autocomplete.	You can do a $LK,"Who",A="MN:Who"$() to see
  //roughly how many times a symbol has been
  //referenced.	Beware of instances where
  //a hash entry is continuously searched for and
  //racks-up huge use_cnt numbers.
};

public class CHashTable
{
  CHashTable *next;
  I64	mask,locked_flags;
  CHash	**body;
};

#help_index "Hash/System"
//Hash table types
//$LK,"ST_HTT_TYPES",A="FF:::/Kernel/KDefine.CPP,ST_HTT_TYPES"$
#define HTt_EXPORT_SYS_SYM	0
#define HTt_IMPORT_SYS_SYM	1
#define HTt_DEFINE_STR		2
#define HTt_GLBL_VAR		3
#define HTt_CLASS		4
#define HTt_INTERNAL_TYPE	5
#define HTt_FUN			6
#define HTt_WORD		7
#define HTt_DICT_WORD		8
#define HTt_KEYWORD		9
#define HTt_ASM_KEYWORD		10
#define HTt_OPCODE		11
#define HTt_REG			12
#define HTt_FILE		13
#define HTt_MODULE		14
#define HTt_HELP_FILE		15
#define HTt_FRAME_PTR		16
#define HTt_NUM_TYPES		17

#define HTf_PRIVATE		23
#define HTf_PUBLIC		24
#define HTf_EXPORT		25
#define HTf_IMPORT		26
#define HTf_IMM			27
#define HTf_GOTO_LABEL		28
#define HTf_RESOLVED		29
#define HTf_UNRESOLVED		30
#define HTf_LOCAL		31

#define HTT_INVALID		0
#define HTT_EXPORT_SYS_SYM	0x00001 //CHashExport
#define HTT_IMPORT_SYS_SYM	0x00002 //CHashImport
#define HTT_DEFINE_STR		0x00004 //CHashDefineStr
#define HTT_GLBL_VAR		0x00008 //CHashGlblVar
#define HTT_CLASS		0x00010 //CHashClass
#define HTT_INTERNAL_TYPE	0x00020 //CHashClass
#define HTT_FUN			0x00040 //CHashFun
#define HTT_WORD		0x00080 //CHashAC only in AutoComplete table
#define HTT_DICT_WORD		0x00100 //CHashGeneric only in AutoComplete tbl
#define HTT_KEYWORD		0x00200 //CHashGeneric $LK,"KEYWORD",A="FF:::/Compiler/OpCodes.TXT,KEYWORD"$
#define HTT_ASM_KEYWORD		0x00400 //CHashGeneric $LK,"ASM_KEYWORD",A="FF:::/Compiler/OpCodes.TXT,ASM_KEYWORD"$
#define HTT_OPCODE		0x00800 //CHashOpcode
#define HTT_REG			0x01000 //CHashReg
#define HTT_FILE		0x02000 //CHashGeneric
#define HTT_MODULE		0x04000 //CHashGeneric
#define HTT_HELP_FILE		0x08000 //CHashSrcSym
#define HTT_FRAME_PTR		0x10000 //CHashGeneric
#define HTG_TYPE_MASK		0x1FFFF

#define HTF_PRIVATE		0x00800000
#define HTF_PUBLIC		0x01000000
#define HTF_EXPORT		0x02000000
#define HTF_IMPORT		0x04000000
#define HTF_IMM			0x08000000
#define HTF_GOTO_LABEL		0x10000000
#define HTF_RESOLVE		0x20000000
#define HTF_UNRESOLVED		0x40000000
#define HTF_LOCAL		0x80000000
#define HTG_FLAGS_MASK		0xFF000000

#define HTG_SRC_SYM	(HTT_DEFINE_STR|HTT_GLBL_VAR|HTT_FUN|HTT_CLASS|\
			HTT_EXPORT_SYS_SYM|HTT_HELP_FILE)
#define HTG_ALL 		-1

#define KERNEL_MODULE_NAME	"/Kernel/Kernel"

class CDbgInfo
{
  U32	min_line,max_line;
  U32	body[1]; //Code heap is 32-bit value
};

public class CHashSrcSym:CHash
{
  U8	*src_link,
	*index;
  CDbgInfo *dbg_info;
  U8	*import_name;
  CAOTImportExport *ie_lst;
};

public class CHashGeneric:CHash
{
  I64	user_data0,user_data1,user_data2;
};

#define REGT_R8		0
#define REGT_R16	1
#define REGT_R32	2
#define REGT_R64	3
#define REGT_SEG	4
#define REGT_FSTK	5
#define REGT_MM		6
#define REGT_XMM	7

public class CHashReg:CHash
{
  U8	reg_num,reg_type;
};

public class CHashAC:CHash
{
  I32	num;
  U32	hits;
};

public class CHashExport:CHashSrcSym
{
  I64	val;
};

public class CHashImport:CHashSrcSym
{
  U8	*module_base,
	*module_header_entry;
};

#help_index "Compiler/Internal"
#define MAX_PTR_STARS	4

//Member Lst Flags
#define MLF_DFT_AVAILABLE	1
#define MLF_LASTCLASS		2
#define MLF_STR_DFT_AVAILABLE	4
#define MLF_FUN			8
#define MLF_DOT_DOT_DOT		16
#define MLF_NO_UNUSED_WARN	32
#define MLF_STATIC		64

public class CArrayDim
{
  CArrayDim *next;
  I64	cnt,total_cnt;
};

#define MLMF_IS_STR	1
public class CMemberLstMeta
{
  CMemberLstMeta *next;
  U8	*str;
  I64	flags,
	user_data;
};

public class CMemberLst
{
  CMemberLst *next,*left,*right,
	*left_class_base,*right_class_base; //For finding dup class local vars.
  U8	*str;
  CHashClass *member_class,*member_class_base;
  CMemberLstMeta *meta;
  U32	use_cnt;
  U16	flags;
  I8	reg,pad;
  I64	offset,size;
  CArrayDim dim;
  U8	*static_data;
  union {
    I64	static_data_ip;
    I64 dft_val;
  }
  CHashFun *fun_ptr;
};

class CExternUsage
{
  CExternUsage *next;
  I64	ip;
};

#help_index "Hash/System;Compiler/Internal"
public class CHashDefineStr:CHashSrcSym
{
  U8	*data,**sub_lst;
  I64	cnt;
};

#define Cf_EXTERN		0
#define Cf_INTERNAL_TYPE	1

public class CHashClass:CHashSrcSym
{//See $LK,"PrsClassNew",A="MN:PrsClassNew"$().	base_class of $LK,"CHashFun",A="MN:CHashFun"$
  I64	size,neg_offset;
  U32	member_cnt;
  U8	ptr_stars_cnt,raw_type;
  U16	flags;
  CMemberLst *member_lst_and_root, //Head of linked list and root of tree.
	*member_class_base_root, //For finding dup class local vars.
	*last_in_member_lst;
  CHashClass	*base_class,
	*fwd_class;
};

//Function flags
#define Ff_INTERRUPT		8
#define Ff_HASERRCODE		9
#define Ff_ARGPOP		10
#define Ff_NOARGPOP		11
#define Ff_INTERNAL		12
#define Ff__EXTERN		13
#define Ff_DOT_DOT_DOT		14
#define Ff_RET1			15

public class CHashFun:CHashClass
{//See $LK,"PrsFunNew",A="MN:PrsFunNew"$().
  CHashClass *return_class;
  U32	arg_cnt,pad,
	used_reg_mask,clobbered_reg_mask;
  U8	*executable_addr;
  CExternUsage *ext_lst;
};

//Glbl Var Flags
#define GVF_FUN		1
#define GVF_IMPORT	2
#define GVF_EXTERN	4
#define GVF_DATA_HEAP	8
#define GVF_ALIAS	16
#define GVF_ARRAY	32

public class CHashGlblVar:CHashSrcSym
{
  I64	size,flags;
  CHashClass *var_class;
  CHashFun *fun_ptr;
  CArrayDim dim;
  U8	*data_addr;
  union {
    I64	data_addr_ip;
    CAOTHeapGlbl *heap_glbl;
  }
};
#assert offset(CHashClass.size)==offset(CHashGlblVar.size)

#help_index "DolDoc"
//See $LK,"TextBase Layer",A="HI:TextBase Layer"$.
#define ATTRF_BLINK	0x10000000
#define ATTRF_INVERT	0x20000000
#define ATTRF_SELECTED	0x40000000
#define ATTRF_UNDERLINE	0x80000000

#define ATTRf_BLINK		28
#define ATTRf_INVERT		29
#define ATTRf_SELECTED		30
#define ATTRf_UNDERLINE		31

//CDocEntry.type codes (Low 8 bits)
#define DOCT_TEXT		0
#define DOCT_NEW_LINE		1
#define DOCT_SOFT_NEW_LINE	2
#define DOCT_CURSOR		3
#define DOCT_TAB		4
#define DOCT_CLEAR		5
#define DOCT_PAGE_BREAK		6
#define DOCT_PAGE_LEN		7
#define DOCT_LEFT_MARGIN	8
#define DOCT_RIGHT_MARGIN	9
#define DOCT_HEADER		10
#define DOCT_FOOTER		11
#define DOCT_INDENT		12
#define DOCT_FOREGROUND		13
#define DOCT_BACKGROUND		14
#define DOCT_DFT_FOREGROUND	15
#define DOCT_DFT_BACKGROUND	16
#define DOCT_PMT		17
#define DOCT_WORD_WRAP		18
#define DOCT_UNDERLINE		19
#define DOCT_INVERT		20
#define DOCT_BLINK		21
#define DOCT_SHIFTED_X		22
#define DOCT_SHIFTED_Y		23
#define DOCT_CURSOR_MOVEMENT	24
#define DOCT_ANCHOR		25
#define DOCT_LINK		26
#define DOCT_BUTTON		27
#define DOCT_DATA		28
#define DOCT_CHECK_BOX		29
#define DOCT_LST		30
#define DOCT_MACRO		31
#define DOCT_MENU_VAL		32
#define DOCT_HEX_ED		33
#define DOCT_TREE		34
#define DOCT_SPRITE		35
#define DOCT_INS_BIN		36
#define DOCT_INS_BIN_SIZE	37
#define DOCT_SONG		38
#define DOCT_HIGHLIGHT		39
#define DOCT_HTML_CODE		40
#define DOCT_ERROR		41

#define DOCT_NUM_TYPES		42

//CDocEntry.type flags upper bits
#define DOCET_BLINK		ATTRF_BLINK
#define DOCET_INVERT		ATTRF_INVERT
#define DOCET_SELECTED		ATTRF_SELECTED
#define DOCET_UNDERLINE		ATTRF_UNDERLINE
#define DOCG_BL_IV_UL		0xB0000000

#define DOCEt_BLINK		ATTRf_BLINK
#define DOCEt_INVERT		ATTRf_INVERT
#define DOCEt_SELECTED		ATTRf_SELECTED
#define DOCEt_UNDERLINE		ATTRf_UNDERLINE

#define DOCG_DBL_BUF_FLAGS	(DOCF_OVERSTRIKE|DOCF_PLAIN_TEXT|\
		DOCF_PLAIN_TEXT_TABS|DOCF_MIN_SIZE|DOCF_NO_CURSOR|\
		DOCF_FORM|DOCF_DBL_DOLLARS|DOCF_DONT_SWAP_OUT|\
		DOCF_DO_FULL_REFRESH|DOCF_BORDER_DOC|DOCF_HIDE_CURSOR|\
		DOCF_DONT_HIGHLIGHT_CURSOR|DOCF_CARRIAGE_RETURN)

//CDocEntry.de_flags.  These first 16 are arg=
#define DOCEF_TAG		1
#define DOCEF_LEN		2
#define DOCEF_AUX_STR		4
#define DOCEF_DEFINE		8
#define DOCEF_HTML_LINK		0x10
#define DOCEF_LEFT_EXP		0x20
#define DOCEF_LEFT_MACRO	0x40
#define DOCEF_RIGHT_EXP		0x80
#define DOCEF_RIGHT_MACRO	0x100
#define DOCEF_HAS_BIN		0x200
#define DOCEF_BIN_PTR_LINK	0x400
#define DOCEF_RAW_TYPE		0x800
#define DOCEF_SHIFTED_X		0x1000
#define DOCEF_SHIFTED_Y		0x2000
#define DOCEF_SCROLLING_X	0x4000
#define DOCEF_USER_DATA		0x8000

//CDocEntry.de_flags.  These are +/- flags
#define DOCEF_LEFT_CB		0x10000
#define DOCEF_LEFT_AUTO		0x20000
#define DOCEF_RIGHT_CB		0x40000
#define DOCEF_RIGHT_AUTO	0x80000
#define DOCEF_LEFT_X		0x100000
#define DOCEF_CENTER_X		0x200000
#define DOCEF_RIGHT_X		0x400000
#define DOCEF_TOP_Y		0x800000
#define DOCEF_CENTER_Y		0x1000000
#define DOCEF_BOTTOM_Y		0x2000000
//HL...UL
#define DOCEF_TAG_CB		0x100000000
#define DOCEF_PAGE_REL_Y	0x200000000
#define DOCEF_MARGIN_REL_X	0x400000000
#define DOCEF_WIN_REL		0x800000000
#define DOCEF_LINK		0x1000000000
#define DOCEF_ESC		0x2000000000 //Send <ESC> (Exit and Save)
#define DOCEF_QUIT		0x4000000000 //Send <SHIFT-ESC> (Abort)
#define DOCEF_FROM_START	0x8000000000
#define DOCEF_HAS_BORDER	0x10000000000
#define DOCEF_SOLID_BORDER	0x20000000000
#define DOCEF_BORDER_PLOT	0x40000000000
#define DOCEF_CHECKED_COLLAPSED 0x80000000000 //Checked or Collapsed
#define DOCEF_CHECK_COLLAPSABLE 0x100000000000
#define DOCEF_REFRESH_DATA	0x200000000000
#define DOCEF_UPDATE_DATA	0x400000000000
//$LK,"DOCEF_DEREF_DATA",A="MN:DOCEF_DEREF_DATA"$ is confusing. $LK,"DocForm",A="MN:DocForm"$() makes doc_e->data point to members
//of class. For ints, it derefs doc_e->data as a ptr.  For strings, it doesn't.
#define DOCEF_DEREF_DATA	0x800000000000
#define DOCEF_REMALLOC_DATA	0x1000000000000
#define DOCEF_HAS_TERMINATOR	0x2000000000000
#define DOCEF_ZERO_BASED	0x4000000000000
#define DOCEF_HOLD		0x8000000000000
#define DOCEF_TREE		0x10000000000000
#define DOCEF_LST		0x20000000000000
#define DOCEF_SKIP		0x40000000000000
#define DOCEF_POPUP		0x80000000000000
#define DOCEF_SKIP_IN_FORM	0x100000000000000
#define DOCEF_FILTER_SKIP	0x200000000000000
#define DOCEF_NO_CLICK_ON	0x400000000000000
#define DOCEF_DONT_DRAW		0x800000000000000 //only works on sprites

#define DOCEG_HAS_ALLOC		(DOCEF_TAG|DOCEF_AUX_STR|DOCEF_DEFINE|\
  DOCEF_HTML_LINK|DOCEF_LEFT_MACRO|DOCEF_RIGHT_MACRO|DOCEF_BIN_PTR_LINK|\
  DOCEF_REMALLOC_DATA)

#define DOCEG_HAS_ARG		((DOCEG_HAS_ALLOC&~DOCEF_REMALLOC_DATA)|\
  DOCEF_LEN|DOCEF_LEFT_EXP|DOCEF_RIGHT_EXP|DOCEF_HAS_BIN|DOCEF_RAW_TYPE|\
  DOCEF_SHIFTED_X|DOCEF_SHIFTED_Y|DOCEF_SCROLLING_X|DOCEF_USER_DATA)

#define DOCEG_DONT_EDIT	(DOCEF_DEFINE|DOCEF_HTML_LINK|\
  DOCEF_AUX_STR|DOCEF_BIN_PTR_LINK|DOCEF_SCROLLING_X|DOCEF_TAG_CB)

//These are identical to Doc flags
#define DOCEF_HIGHLIGHT		0x4000000
#define DOCEF_WORD_WRAP		0x8000000
#define DOCEF_BLINK		ATTRF_BLINK
#define DOCEF_INVERT		ATTRF_INVERT
#define DOCEF_SELECTED		ATTRF_SELECTED
#define DOCEF_UNDERLINE		ATTRF_UNDERLINE
#define DOCEf_HIGHLIGHT		26
#define DOCEf_WORD_WRAP		27
#define DOCEf_BLINK		ATTRf_BLINK
#define DOCEf_INVERT		ATTRf_INVERT
#define DOCEf_SELECTED		ATTRf_SELECTED
#define DOCEf_UNDERLINE		ATTRf_UNDERLINE

//CDocEntry.de_flags.  These first 16 are arg=
#define DOCEf_TAG		0
#define DOCEf_LEN		1
#define DOCEf_AUX_STR		2
#define DOCEf_DEFINE		3
#define DOCEf_HTML_LINK		4
#define DOCEf_LEFT_EXP		5
#define DOCEf_LEFT_MACRO	6
#define DOCEf_RIGHT_EXP		7
#define DOCEf_RIGHT_MACRO	8
#define DOCEf_HAS_BIN		9
#define DOCEf_BIN_PTR_LINK	10
#define DOCEf_RAW_TYPE		11
#define DOCEf_SHIFTED_X		12
#define DOCEf_SHIFTED_Y		13
#define DOCEf_SCROLLING_X	14
#define DOCEf_USER_DATA		15

//CDocEntry.de_flags.  These are +/- flags
#define DOCEf_LEFT_CB		16
#define DOCEf_LEFT_AUTO		17
#define DOCEf_RIGHT_CB		18
#define DOCEf_RIGHT_AUTO	19
#define DOCEf_LEFT_X		20
#define DOCEf_CENTER_X		21
#define DOCEf_RIGHT_X		22
#define DOCEf_TOP_Y		23
#define DOCEf_CENTER_Y		24
#define DOCEf_BOTTOM_Y		25
//HL...UL
#define DOCEf_TAG_CB		32
#define DOCEf_PAGE_REL_Y	33
#define DOCEf_MARGIN_REL_X	34
#define DOCEf_WIN_REL		35
#define DOCEf_LINK		36
#define DOCEf_ESC		37 //Send <ESC> (Exit and Save)
#define DOCEf_QUIT		38 //Send <SHIFT-ESC> (Abort)
#define DOCEf_FROM_START	39
#define DOCEf_HAS_BORDER	40
#define DOCEf_SOLID_BORDER	41
#define DOCEf_BORDER_PLOT	42
#define DOCEf_CHECKED_COLLAPSED 43 //Checked or Collapsed
#define DOCEf_CHECK_COLLAPSABLE 44
#define DOCEf_REFRESH_DATA	45
#define DOCEf_UPDATE_DATA	46
#define DOCEf_DEREF_DATA	47
#define DOCEf_REMALLOC_DATA	48
#define DOCEf_HAS_TERMINATOR	49
#define DOCEf_ZERO_BASED	50
#define DOCEf_HOLD		51
#define DOCEf_TREE		52
#define DOCEf_LST		53
#define DOCEf_SKIP		54
#define DOCEf_POPUP		55
#define DOCEf_SKIP_IN_FORM	56
#define DOCEf_FILTER_SKIP	57
#define DOCEf_NO_CLICK_ON	58
#define DOCEf_DONT_DRAW		59 //only works on sprites
#define DOCEf_NUM_FLAGS		60

public class CDocBin
{
  CDocBin *next,*last;
  I32	temp_use_cnt,renum_num;
  U8	*tag;
  U0	start;
  U32	num,flags,size,use_cnt;
  U0	end;
  U8	*data;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#define DOC_SCROLL_SPEED	8

#define DOCSS_NORMAL		0
#define DOCSS_SINGLE_QUOTE	1
#define DOCSS_DBL_QUOTE		2
#define DOCSS_COMMENT		3
#define DOCSS_CPP_Z_COMMENT	4

#define DOC_ATTR_DFT_TEXT	WHITE<<4+BLACK
#define DOC_COLOR_ALT_TEXT	LTGRAY
#define DOC_COLOR_LINK		RED
#define DOC_COLOR_MACRO		LTBLUE
#define DOC_COLOR_ANCHOR	DKGRAY
#define DOC_COLOR_TREE		PURPLE
#define DOC_COLOR_PMT		GREEN
#define DOC_COLOR_BIN		CYAN
#define DOC_COLOR_KEYWORD	BLUE
#define DOC_COLOR_KEYWORD2	LTRED
#define DOC_COLOR_COMMENT	GREEN
#define DOC_COLOR_QUOTE		BROWN

public class CDocSettings
{
  U32	final_u32_attr;
  I16	left_margin,right_margin,indent;
  U16	page_len,header,footer;
  I8	shifted_x,shifted_y;
  U8	state,comment_depth,paren_depth,brace_depth,
	cur_text_attr,dft_text_attr;
};

#define DOC_DFT 		MIN_I32

public class CDocEntryBase
{
//This is a shortened structure for
  //cmds like the text cmd which
  //don't require the full CDocEntry structure.
  CDocEntryBase *next,*last;
  U8	*tag;
  union {
    U8	type_u8; //this stores the code
    U32 type; //these store attr flags
  };
  I32	page_line_num;
  I64	de_flags;
  I32	x,y;
  U32	min_col,max_col;
  CDocSettings settings;
  I64	user_data;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#define DOCE_LEN_DFT	64

public class CDocEntry:CDocEntryBase
{
  union {
    I64 attr;
    I64 cursor_x_offset;
    I64 (*left_cb)(CDoc *doc,CDocEntry *doc_e);
    I64 left_exp;
  };
  U8	*left_macro;

  union {
    I64 cursor_y_offset;
    I64 (*right_cb)(CDoc *doc,CDocEntry *doc_e);
    I64 right_exp;
  };
  U8	*right_macro;

  U8	*(*tag_cb)(CDoc *doc,CDocEntry *doc_e,CTask *mem_task);
  U8	*define_str,
	*aux_str,
	*bin_ptr_link,
	*html_link,
	*my_fmt_data;
  I64	hex_ed_width;
  I32	scroll_len,
	len, //$LK,"DOCE_LEN_DFT",A="MN:DOCE_LEN_DFT"$
	bin_num;
  U8	raw_type,pad[3];
  CDocBin *bin_data;
  U8	*data;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

//$LK,"DocForm",A="MN:DocForm"$() $LK,"DocMenu",A="MN:DocMenu"$() $LK,"DocEd",A="MN:DocEd"$() $LK,"PopUpMenu",A="MN:PopUpMenu"$()
#define	DOF_MIN_SIZE		0x01
#define	DOF_INTERCEPT_TASK_END	0x02
#define DOF_DONT_HOME		0x04
#define DOF_WIN_MAX		0x08
#define DOF_DONT_TEXT_ATTR	0x10
#define DOF_DONT_WINMGR_SYNC	0x20
#define DOF_DONT_SHOW		0x40
//Combines with  $LK,"Editor Flags",A="MN:EDF_BAIL"$

class CEdFindText
{
  U8	find_text[STR_LEN] fmtstr "$$DA-P,LEN=STR_LEN-1,"
	"A=\"Find        :%s\"$$\n",
	replace_text[STR_LEN] fmtstr "$$DA-P,LEN=STR_LEN-1,"
	"A=\"Replace     :%s\"$$\n";
  Bool	replace			fmtstr "$$CB,\"Replace\"$$\n",
	scan_fwd		fmtstr "$$CB,\"Fwd\"$$\n",
	scan_selected_text	fmtstr "$$CB,\"Selection\"$$\n",
	match_case		fmtstr "$$CB,\"Match Case\"$$\n",
	whole_labels		fmtstr "$$CB,\"Whole Labels\"$$\n",
	local_var		fmtstr "$$CB,\"Rename Local Var\"$$\n",
	pmt,pad;
  I64	filter_lines		fmtstr "$$DA,A=\"Filter Lines:%d\"$$\n";
};

class CEdFileName
{
  U8	name[256] fmtstr "$$DA-P,LEN=255,A=\"FileName:%s\"$$";
};

//$LK,"Ed",A="MN:Ed"$()
#define EDF_BAIL		0x100
#define EDF_COLLAPSE		0x200
#define EDF_UNCOLLAPSE		0x400
#define EDF_WAS_WRITE		0x800 //Was exit ESC or SHIFT_ESC?
//Combines with $LK,"Document Flags",A="MN:DOF_MIN_SIZE"$

#define EDf_BAIL		8
#define EDf_COLLAPSE		9
#define EDf_UNCOLLAPSE		10
#define EDf_WAS_WRITE		11

// DOC header flags
#define DOCF_PLAIN_TEXT 	0x1
#define DOCF_PLAIN_TEXT_TABS	0x2 //has '\t', not DOCT_TAB
#define DOCF_NO_CURSOR		0x4
#define DOCF_CARRIAGE_RETURN	0x8
#define DOCF_DBL_DOLLARS	0x10
#define DOCF_COLOR_NAMES	0x20
//Reserved x2
#define DOCF_BORDER_DOC		0x100
#define DOCF_FORM		0x200
#define DOCF_MIN_SIZE		0x400
#define DOCF_HIDE_CURSOR	0x800  //use $LK,"DocCursor",A="MN:DocCursor"$
#define DOCF_DONT_HIGHLIGHT_CURSOR 0x1000 //use $LK,"DocHighlightCursor",A="MN:DocHighlightCursor"$
#define DOCF_NO_SCROLL_BARS	0x2000 //use $LK,"DocScroll",A="MN:DocScroll"$
#define DOCF_ALLOW_UNDO		0x4000
#define DOCF_DONT_SHOW		0x8000
#define DOCF_HAS_SONG		0x10000
#define DOCF_MORE		0x20000
#define DOCF_BWD_MOVEMENT	0x40000
#define DOCF_NULL_GRAB_SCROLL	0x80000
#define DOCF_DONT_SWAP_OUT	0x100000
#define DOCF_DO_FULL_REFRESH	0x200000
#define DOCF_BREAK_UNLOCKED	0x400000
//Reserved x1
#define DOCF_HIGHLIGHT		DOCEF_HIGHLIGHT
#define DOCF_WORD_WRAP		DOCEF_WORD_WRAP
#define DOCF_BLINK		DOCEF_BLINK
#define DOCF_INVERT		DOCEF_INVERT
#define DOCF_SELECTED		DOCEF_SELECTED
#define DOCF_UNDERLINE		DOCEF_UNDERLINE

#define DOCF_OVERSTRIKE 	0x100000000
#define DOCF_IN_DOLLAR		0x200000000
#define DOCF_SUPERSCRIPT_MODE	0x400000000
#define DOCF_SUBSCRIPT_MODE	0x800000000
#define DOCF_UNDO_DIRTY		0x1000000000

#define DOCf_PLAIN_TEXT 	0
#define DOCf_PLAIN_TEXT_TABS	1 //has '\t', not DOCT_TAB
#define DOCf_NO_CURSOR		2
#define DOCf_CARRIAGE_RETURN	3
#define DOCf_DBL_DOLLARS	4
#define DOCf_COLOR_NAMES	5
//Reserved x2
#define DOCf_BORDER_DOC		8
#define DOCf_FORM		9
#define DOCf_MIN_SIZE		10
#define DOCf_HIDE_CURSOR	11  //use $LK,"DocCursor",A="MN:DocCursor"$
#define DOCf_DONT_HIGHLIGHT_CURSOR 12  //use $LK,"DocHighlightCursor",A="MN:DocHighlightCursor"$
#define DOCf_NO_SCROLL_BARS	13  //use $LK,"DocScroll",A="MN:DocScroll"$
#define DOCf_ALLOW_UNDO		14
#define DOCf_DONT_SHOW		15
#define DOCf_HAS_SONG		16
#define DOCf_MORE		17
#define DOCf_BWD_MOVEMENT	18
#define DOCf_NULL_GRAB_SCROLL	19
#define DOCf_DONT_SWAP_OUT	20
#define DOCf_DO_FULL_REFRESH	21
#define DOCf_BREAK_UNLOCKED	22
//Reserved x1

#define DOCf_HIGHLIGHT		DOCEf_HIGHLIGHT
#define DOCf_WORD_WRAP		DOCEf_WORD_WRAP
#define DOCf_BLINK		DOCEf_BLINK
#define DOCf_INVERT		DOCEf_INVERT
#define DOCf_SELECTED		DOCEf_SELECTED
#define DOCf_UNDERLINE		DOCEf_UNDERLINE

#define DOCf_OVERSTRIKE 	32
#define DOCf_IN_DOLLAR		33
#define DOCf_SUPERSCRIPT_MODE	34
#define DOCf_SUBSCRIPT_MODE	35
#define DOCf_UNDO_DIRTY		36

//locked flags
#define DOClf_LOCKED		0
class CDocUndo
{
  CDocUndo *next,*last;
  I64	size,doc_flags,time_stamp;
  U8	*body;
};

//See $LK,"DocMenu",A="MN:DocMenu"$()
#define DOCM_CANCEL		(-1)

#define DOC_SIGNATURE_VAL	'DocS'

#define RECALCt_NORMAL		0x00
#define RECALCt_FIND_CURSOR	0x01
#define RECALCt_TO_SCREEN	0x02
#define RECALCG_MASK		0xFF

#define RECALCF_HAS_CURSOR	0x100
#define RECALCF_ADD_CURSOR	0x200
#define RECALCF_TO_HTML		0x400

public class CDoc //Linked Text File header
{//See $LK,"Doc",A="HI:Doc"$ for documentation.
  CDocEntryBase root;
  I64	flags,locked_flags;
  CDocEntry *cur_entry,*old_cur_entry;
  I32	cur_col,old_cur_col,
	line_start_col,top_line_num,
	dollar_buf_size,dollar_buf_ptr;
  U8	*dollar_buf; //When entering $$ cmds, it buffers them until the end $$.

  CTask *win_task,*mem_task,*owning_task;
  I32	page_line_num,undo_cnt,
	x,y,min_x,max_x,min_y,max_y;
  I64	line,col,best_d,
	old_win_top,old_win_bottom,
	old_win_left,old_win_right,
	cmd_U8;
  U32	doc_signature,cur_bin_num;
  I64	max_entries,
	updates_cnt;
  CEdFindText *find_replace;

  U8	*cur_sprite;//Set cur_sprite use (ip.pos.x,ip.pos.y) as input.
//Output is nearest. See $LK,"*_cur_elem_num=",A="FF:::/Adam/Gr/SpriteEd.CPP,*_cur_elem_num=doc->"$ and $LK,"::/Demo/Graphics/Pick.CPP"$.
  I64	nearest_sprite_elem_num;

  CEdFileName filename;
  I64	file_attr;
  I64	(*left_click_link)(CDoc *doc,CDocEntry *doc_e);
  I64	(*right_click_link)(CDoc *doc,CDocEntry *doc_e);

  //See $LK,"::/Apps/Psalmody/JukeBox.CPP"$
  U8	*user_put_data; //Passed to user_put_key() and user_put_s()
  Bool	(*user_put_key)(CDoc *doc,U8 *data,I64 ch,I64 sc);
  Bool	(*user_put_s)(CDoc *doc,U8 *data,U8 *st);

  CDoc	*parent_doc; //(When browsing deeper, opening deeper docs.)
  U64	desc; //8 characters. See $LK,"DocBorderLstDraw",A="MN:DocBorderLstDraw"$().

  CDocBin bin_root;
  CDocSettings settings_root;
  CDocUndo undo_root;

  I64	user_data;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#help_index "Windows"
/*
Fs->win_inhibit mask

Some inhibit actions on the task itself.
Some inhibit actions if the focus task
tries to do something.
*/
#define WIF_SELF_FOCUS		0x0001 //If active this task cannot have focus
		//MENU		0x0002
#define WIF_SELF_CTRLS		0x0004
#define WIF_SELF_IP_L		0x0008
		//IP_L_D	0x0010
#define WIF_SELF_IP_R		0x0020
		//IP_R_D	0x0040
#define WIF_SELF_IP_WHEEL	0x0080 //Does nothing, yet
#define WIF_SELF_BORDER		0x0100
#define WIF_SELF_GRAB_SCROLL	0x0200
#define WIF_SELF_DOC		0x0400
#define WIF_SELF_ODE		0x0800
#define WIF_SELF_KEY_DESC	0x1000
		//FOCUS		0x00010000
#define WIF_FOCUS_TASK_MENU	0x00020000
#define WIF_FOCUS_TASK_CTRLS	0x00040000
#define WIF_FOCUS_TASK_IP_L	0x00080000
#define WIF_FOCUS_TASK_IP_L_D	0x00100000
#define WIF_FOCUS_TASK_IP_R	0x00200000
#define WIF_FOCUS_TASK_IP_R_D	0x00400000
#define WIF_FOCUS_TASK_IP_WHEEL	0x00800000 //Does nothing, yet
#define WIF_FOCUS_TASK_BORDER	0x01000000
#define WIF_FOCUS_TASK_GRAB_SCROLL 0x02000000

#define WIG_DBL_CLICK		(WIF_FOCUS_TASK_IP_L_D|WIF_FOCUS_TASK_IP_R_D)
#define WIG_TASK_DFT	(WIF_FOCUS_TASK_MENU|WIG_DBL_CLICK|\
			0xFFFF-WIF_SELF_DOC-WIF_SELF_ODE)
#define WIG_USER_TASK_DFT	WIF_SELF_KEY_DESC

#define WIf_SELF_FOCUS		0
#define WIf_SELF_CTRLS		2
#define WIf_SELF_IP_L		3
#define WIf_SELF_IP_R		5
#define WIf_SELF_IP_WHEEL	7
#define WIf_SELF_BORDER		8
#define WIf_SELF_GRAB_SCROLL	9
#define WIf_SELF_DOC		10
#define WIf_SELF_ODE		11
#define WIf_SELF_KEY_DESC	12
#define WIf_FOCUS_TASK_MENU	17
#define WIf_FOCUS_TASK_CTRLS	18
#define WIf_FOCUS_TASK_IP_L	19
#define WIf_FOCUS_TASK_IP_L_D	20
#define WIf_FOCUS_TASK_IP_R	21
#define WIf_FOCUS_TASK_IP_R_D	22
#define WIf_FOCUS_TASK_IP_WHEEL	23
#define WIf_FOCUS_TASK_BORDER	24
#define WIf_FOCUS_TASK_GRAB_SCROLL 25

class CWinMgrTimingGlbls
{
  I64	last_total_jiffies,
	last_idle_pt_hits[MP_MAX_PROCESSORS],
	last_swap_cnter[MP_MAX_PROCESSORS];
  F64	last_calc_idle_time,calc_idle_delta_time;
  I64	calc_idle_cnt;
};

#define WMG_FPS			30

public class CWinMgrGlbls
{
  I64	updates;
  F64	ode_time,
	last_ode_time,
	fps,		//You can read but not write this. You have no control.
	last_refresh_tS;
  CTask *old_focus_task;
  CWinMgrTimingGlbls *t;
  Bool	show_menu,
	grab_scroll,
	grab_scroll_closed;
};

#help_index "AutoComplete"
#define ACf_INIT_IN_PROGRESS	0
#define AC_MAX_FILLINS 10
public class CAutoCompleteGlbls
{
  I64	num_words;
  CHashTable *hash_table;
  U8	*cur_word;
  I64	flags;
  CTask	*task;
  I64	partial_len,num_fillins,
	fillin_hits	[AC_MAX_FILLINS+1];
  CHashAC *fillin_matches[AC_MAX_FILLINS+1];
};

#help_index "AutoComplete/Dictionary"
#define ACD_WORD_FILENAME	"/Adam/AutoComplete/ACWords.DAT.Z"
#define ACD_DEF_FILENAME	"/Adam/AutoComplete/ACDefs.DAT"
#define ACD_DEF_FILENAME_Z	"/Adam/AutoComplete/ACDefs.DAT.Z"

#define ACD_H1			0
#define ACD_H1_END		1
#define ACD_DEF			2
#define ACD_DEF_END		3
#define ACD_PRONUNCIATION	4
#define ACD_PRONUNCIATION_END	5
#define ACD_POS			6
#define ACD_POS_END		7
#define ACD_EXTRA		8
#define ACD_EXTRA_END		9
#define ACD_BLK_SIZE		0x4000

#define ACD_END_CHAR		0x00
#define ACD_WORD_CHAR		0x01
#define ACD_DEF_CHAR		0x02
#define ACD_PRONUNCIATION_CHAR	0x03
#define ACD_POS_CHAR		0x04
#define ACD_EXTRA_CHAR		0x05

#define ACD_MAX_FILLINS 10
public class CAutoCompleteDictGlbls
{
  U8	*word_lst;
  I64	word_lst_size,num_words,num_fillins;
  U8	*fillins[ACD_MAX_FILLINS];
  Bool	has_words,has_defs;
};

#help_index "Compiler/Directive"
//Compiler $LK,"Option",A="MN:Option"$()s
//You might need to do #exe {Option();}
//Note: The #exe stmt is lexed-ahead,
//so it takes effect earlier than you might expect.
#define OPTf_ECHO		0x00
#define OPTf_TRACE		0x01
#define OPTf_WARN_UNUSED_VAR	0x10 //Applied to funs, not stmts
#define OPTf_WARN_PAREN		0x11 //Warn unnecessary parens
#define OPTf_WARN_DUP_TYPES	0x12 //Warn dup local var type stmts
#define OPTf_EXTERNS_TO_IMPORTS	0x20
#define OPTf_KEEP_PRIVATE	0x21
#define OPTf_NO_REG_VAR		0x22  //Applied to funs, not stmts
#define OPTf_GLBLS_ON_DATA_HEAP	0x23
//Disable 10-byte float consts for ã,log2_10,log10_2,loge_2
#define OPTf_NO_BUILTIN_CONST	0x24 //Applied to funs, not stmts
#define OPTf_USE_IMM64		0x25 //Not completely implemented

#define OPTF_ECHO		(1<<OPTf_ECHO)

#help_index "Compiler/Intermediate Code"
//See $LK,"ST_RAW_TYPES",A="MN:ST_RAW_TYPES"$
#define RT_I0		2
#define RT_U0		3
#define RT_I8		4
#define RT_U8		5
#define RT_I16		6
#define RT_U16		7
#define RT_I32		8
#define RT_U32		9
#define RT_I64		10
#define RT_PTR		10 //Signed to allow negative err codes. $LK,"DOCM_CANCEL",A="MN:DOCM_CANCEL"$
#define RT_U64		11
#define RT_F32		12 //Not implemented
#define RT_UF32		13 //Not implemented, Fictitious
#define RT_F64		14
#define RT_UF64		15 //Fictitious
#define RT_NUM_IT	16
#define RT_MASK		0x0F

#define TY_NULL			0x00
#define TY_IMM			0x10
#define TY_REG			0x20
#define TY_DISP			0x30
#define TY_SIB			0x40
#define TY_RIP_DISP32		0x50
//Not implemented.  Requires storage for glblvar struct in intermediate code.
#define TY_RIP_DISP32_IMPORT	0x60
#define TY_STK			0x70
#define TY_MASK			0xF0

#define ICF_R_TO_F64			0x000000001
#define ICF_R_TO_INT			0x000000002
#define ICF_A1_TO_F64			0x000000004
#define ICF_A1_TO_INT			0x000000008
#define ICF_A2_TO_F64			0x000000010
#define ICF_A2_TO_INT			0x000000020
#define ICF_USE_F64			0x000000040
#define ICF_USE_UNSIGNED		0x000000080
#define ICF_USE_INT			0x000000100 //highest priority
#define ICF_NO_DEPEND_RESULT		0x000000200
#define ICF_CODE_FINAL			0x000000400
#define ICF_NOT_ADDR			0x000000800
#define ICF_SHORT_JMP			0x000001000
#define ICF_A1_FIRST			0x000002000
#define ICF_PUSH_RESULT			0x000004000
#define ICF_PASS_TRACE			0x000008000
#define ICF_R_WAS_STK			0x000010000
#define ICF_A1_WAS_STK			0x000020000
#define ICF_A2_WAS_STK			0x000040000
#define ICF_PUSH_CMP			0x000080000 //for 50<i<j<=100 exps
#define ICF_POP_CMP			0x000100000 //for 50<i<j<=100 exps
#define ICF_DEPEND_RESULT		0x000200000
#define ICF_SWAP			0x000400000
#define ICf_DONT_PUSH_FLOAT0		23 // 3bits
#define ICf_DONT_POP_FLOAT0		26 // 3bits
#define ICF_ALTERNATE_TEMPLATE		0x020000000
#define ICF_LOCK			0x040000000
#define ICf_LOCK			30
#define ICF_NO_RIP			0x080000000
#define ICF_DEL_PREVIOUS_INS		0x100000000
#define ICF_PREVIOUS_DELETED		0x200000000
#define ICF_DONT_RESTORE		0x400000000
#define ICG_NO_CVT_MASK			0x7FFFFFF00

#define IC_BODY_SIZE			0x80

#define ECF_HAS_PUSH_CMP		0x01 //for 50<i<j<=100 exps

class CICArg
{
  U8	type,type_pointed_to;
  U16	reg; //low is reg, high is index_reg+scale<<6
  I64	disp;
};

class CICTreeLinks
{
  CHashClass *a1c,*a2c;
  CIntermediateCode *a1t,*a2t;
  CHashClass *class2;
};

class CIntermediateCodeBase
{
  CIntermediateCode *next,*last;
  U16	ic_code,
	ic_precedence;
  I16	ic_cnt,
	ic_last_cnt;
};

class CIntermediateCode:CIntermediateCodeBase
{
  I64	ic_flags,
	ic_data;

  CHashClass *ic_class,*ic_class2;

  U32	ic_line;
  CICArg a1,a2,r;
  union {
    U8 ic_body[IC_BODY_SIZE];
//Tree Links are created in OptPass012.  An ADD opcode, for exmaple,
    //points back to its two earlier addend instructions, not MAlloced,
    //just pointers to previous CIntermediate codes in the array.

    //Tree links get destroyed during Pass789A when they get overwrites
    //by machine code.	(Saves room to union the output machine code buffer
    //with these links, since they are not needed after pass4.

    //Tree links are mostly used during 012 and 3 for determining types.
    CICTreeLinks t;
  };
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

class CPrsStk
{
  I64	ptr,
	stk[255],
	ptr2,
	stk2[255];
};

#define CMT_LABEL		0
#define CMT_ASM_LABEL		1
#define CMT_GOTO_LABEL		2
#define CMT_STR_CONST		3
#define CMT_JMP_TABLE		4
#define CMT_FLOAT_CONSTS	5
#define CMT_ARRAY_DIM		6
#define CMT_HASH_ENTRY		7

#define CMF_POP_CMP		0x01
#define CMF_DEFINED		0x02
#define CMF_I8_JMP_TABLE	0x04
#define CMF_U8_JMP_TABLE	0x08
#define CMF_I16_JMP_TABLE	0x10
#define CMF_U16_JMP_TABLE	0x20

#define CM_MAX_CONSTS		16

class CCodeMisc
{
  CCodeMisc *next,*last,*fwd,*dft,*begin;
  U8	*str;
  U32	type,flags;
  I64	use_cnt;
  U8	*addr;
  union {
    I64 st_len;		//STR_CONST
    I64 num_consts;	//FLOAT_CONSTS
    I64 range;
    I64 ip;		//ASM_LABEL
  }
  union {
    CCodeMisc **jmp_table;
    F64	*float_consts;
    CArrayDim *dim;
    CHash *h;
  };
};

#help_index "Compiler/Assembler"
#define IEF_OP_SIZE16		0x001
#define IEF_OP_SIZE32		0x002
#define IEF_PLUS_OPCODE		0x004
#define IEF_DONT_SWITCH_MODES	0x008
#define IEF_DFT			0x010
#define IEF_NOT_IN_64_BIT	0x020
#define IEF_48_REX		0x040
#define IEF_REX_ONLY_R8_R15	0x080
#define IEF_REX_XOR_LIKE	0x100
#define IEF_STI_LIKE		0x200
#define IEF_ENDING_ZERO		0x400

//Slash value
#define SV_R_REG	8
#define SV_I_REG	9
#define SV_STI_LIKE	10 //$LK,"uasm_slash_val",A="FF:::/Compiler/AsmInit.CPP,uasm_slash_val"$ only.
#define SV_NONE		11
class CInstruction
{
  U8	ins_entry_num, //This entry num in opcode hash entry
	opcode_cnt,
	opcode[4];
  U16	flags;
  U8	slash_val,uasm_slash_val,opcode_modifier,
	arg1,arg2,
	size1,size2,//Size in bits
	pad;
};

//x86 opcodes
#define OC_OP_SIZE_PREFIX	0x66
#define OC_ADDR_SIZE_PREFIX	0x67
#define OC_LOCK_PREFIX		0xF0
#define OC_NOP			0x90
#define OC_BPT			0xCC
#define OC_CALL			0xE8
#define OC_JMP_REL8		0xEB
#define OC_NOP2			(OC_NOP<<8+OC_OP_SIZE_PREFIX)

#define PUSH_C_REGS PUSH RAX PUSH RCX PUSH RDX PUSH RBX PUSH R8 PUSH R9
#define POP_C_REGS POP R9 POP R8 POP RBX POP RDX POP RCX POP RAX

#define PUSH_REGS PUSH RAX PUSH RCX PUSH RDX PUSH RBX PUSH RBP PUSH RSI \
PUSH RDI PUSH R8 PUSH R9 PUSH R10 PUSH R11 PUSH R12 PUSH R13 PUSH R14 PUSH R15
#define POP_REGS POP R15 POP R14 POP R13 POP R12 POP R11 POP R10 POP R9 \
POP R8 POP RDI POP RSI POP RBP POP RBX POP RDX POP RCX POP RAX

#define NUM_REGS	16

#define REG_RAX	0
#define REG_RCX	1
#define REG_RDX	2
#define REG_RBX	3
#define REG_RSP	4
#define REG_RBP	5
#define REG_RSI	6
#define REG_RDI	7
#define REG_RIP	16	//Used by compiler, not really it's num
//Be careful: RBPu8, RSPu8, RSIu8, RDIu8 are 20-24
#define REG_NONE	32	//noreg flag sets it to this
#define REG_ALLOC	33	//reg flag sets it to this
#define REG_UNDEF	MIN_I8

#define REGG_C_CLOBBERED		0x003F //RAX,RCX,RDX,RBX
#define REGG_SAVED			0x0030 //RBP,RSP
#define REGG_C_TEMP_CLOBBERED		0x0300 //R8,R9
#define REGG_C_LOCAL_VARS		0xCCC0 //RSI,RDI,R10,R11,R14,R15
#define REGG_C_LOCAL_NON_PTR_VARS	0x3000 //R12,R13

#define AOT_BIN_BLK_BITS 16
#define AOT_BIN_BLK_SIZE (1<<AOT_BIN_BLK_BITS)

class CAOTBinBlk
{
  CAOTBinBlk *next;
  U8	body[AOT_BIN_BLK_SIZE];
};

I64 class CAbsCntsI64
{
  U16	abs_addres, //Only odd/even matters. Cnt of absolute addres in an exp.
	c_addres; //Only odd/even matters. Cnt of C addres in an exp.
  U32	externs; //Only nonzero matters. Some regions have externs banned.
};

class CAsmUndefHash
{//Only place created is $LK,"Exp Parser",A="FF:::/Compiler/PrsExp.CPP,sizeof(CAsmUndefHash)"$ when an undef is found in an ASM exp.
  CAsmUndefHash	*next;
  CHashExport	*hash;
};

class CAsmNum
{
  I64	i;
  U8	*machine_code;
  CAsmUndefHash	*local_asm_undef_hash,*glbl_asm_undef_hash;
  CAbsCntsI64 abs_cnts;
};

class CAsmNum2
{
  CAsmNum num;
  I64	U8_cnt,rel;
  Bool	imm_flag;
};

class CAsmIns
{
  CInstruction *tempi;
  CAsmNum2 imm,disp;
  I64	U8_cnt,last_opcode_U8,
	REX,ModrM,SIB;
  Bool	has_REX,has_ModrM,has_SIB,
	has_addr_prefix,
	has_operand_prefix,
	is_dft,pad[2];
};

class CAsmArg
{
  CAsmNum num;
  I64	seg,size, //Size in bytes
	reg1,reg2,fstk,
	scale;
  Bool	indirect,imm_or_off_present,just_seg,pad[5];
};

class CAsmUnresolvedRef
{
  CAsmUnresolvedRef *next;
  I64	type,line_num;
  U8	*machine_code;
  I64	ip,rel_ip;
  CAOT	*aot;
  U8	*str;	//Only for import glbls
  CAsmUndefHash *asm_undef_hash;
  Bool	U8_avail,
	imm_flag;//Only for import glbls
};

//Opcode Modifier
#define OM_NO 0
#define OM_CB 1
#define OM_CW 2
#define OM_CD 3
#define OM_CP 4
#define OM_IB 5
#define OM_IW 6
#define OM_ID 7

#define ARGT_NO		0
#define ARGT_REL8	1
#define ARGT_REL16	2
#define ARGT_REL32	3

#define ARGT_IMM8	4
#define ARGT_IMM16	5
#define ARGT_IMM32	6
#define ARGT_IMM64	7

#define ARGT_UIMM8	8
#define ARGT_UIMM16	9
#define ARGT_UIMM32	10
#define ARGT_UIMM64	11

#define ARGT_R8		12
#define ARGT_R16	13
#define ARGT_R32	14
#define ARGT_R64	15

#define ARGT_RM8	16
#define ARGT_RM16	17
#define ARGT_RM32	18
#define ARGT_RM64	19

#define ARGT_M8		20
#define ARGT_M16	21
#define ARGT_M32	22
#define ARGT_M64	23

#define ARGT_M1632	24 // Not implemented
#define ARGT_M16N32	25 // Not implemented
#define ARGT_M16N16	26 // Not implemented
#define ARGT_M32N32	27 // Not implemented

#define ARGT_MOFFS8	28
#define ARGT_MOFFS16	29
#define ARGT_MOFFS32	30
#define ARGT_MOFFS64	31

#define ARGT_AL		32
#define ARGT_AX		33
#define ARGT_EAX	34
#define ARGT_RAX	35

#define ARGT_CL		36
#define ARGT_DX		37
#define ARGT_SREG	39

#define ARGT_SS		40
#define ARGT_DS		41
#define ARGT_ES		42
#define ARGT_FS		43

#define ARGT_GS		44
#define ARGT_CS		45
#define ARGT_ST0	46
#define ARGT_STI	47

#define ARGT_MM		48 // Not implemented
#define ARGT_MM32	49 // Not implemented
#define ARGT_MM64	50 // Not implemented
#define ARGT_XMM	51 // Not implemented

#define ARGT_XMM32	52 // Not implemented
#define ARGT_XMM64	53 // Not implemented
#define ARGT_XMM128	54 // Not implemented
#define ARGT_XMM0	55 // Not implemented

#help_index "Compiler/Internal;Hash/System"
public class CHashOpcode:CHash
{
  U8	instruction_entry_cnt,
	pad[7];
  CInstruction ins[1];
};

#help_index "Compiler/Intermediate Code"
#define IEF_GOTO_LABEL		1
class CAOTImportExport
{
  CAOTImportExport *next,*last;
  I64	ip,flags;
  CAOT	*aot;
  U8	*str,
	*src_link,
	type,pad[7];
};

#define AAT_ADD_U8		0
#define AAT_SUB_U8		1
#define AAT_ADD_U16		2
#define AAT_SUB_U16		3
#define AAT_ADD_U32		4
#define AAT_SUB_U32		5
#define AAT_ADD_U64		6
#define AAT_SUB_U64		7
class CAOTAbsAddr
{
  CAOTAbsAddr *next;
  I64	ip;
  U8	type,pad[7];
};

class CAOTHeapGlblRef
{
  CAOTHeapGlblRef *next;
  I64	ip;
};

class CAOTHeapGlbl
{
  CAOTHeapGlbl *next;
  U8	*str;
  I64	size;
  CAOTHeapGlblRef *references;
};

class CAOT
{
  CAOT	*next,*last;
  U8	*buf;
  I64	ip,ip2,
	aot_U8s,
	max_align_bits,module_org;
  CAOT	*parent_aot;
  CAOTImportExport *next_ie,*last_ie;
  CAOTAbsAddr *abss;
  CAOTHeapGlbl *heap_glbls;
};

class CStreamBlk
{
  CStreamBlk *next,*last;
  U8	*body;
};

class CCodeCtrl
{
  CCodeCtrl *coc_next;
  CCodeMisc *coc_next_misc,*coc_last_misc;
  CIntermediateCodeBase	coc_root;
};

#help_index "Compiler/Lex"
#define __DATE__ #exe{StreamPrint("\"%D\"",Now);}
#define __TIME__ #exe{StreamPrint("\"%T\"",Now);}
#define __LINE__ #exe{StreamPrint("%d",\
	Fs->last_lex->lex_include_stk->line_num);}
#define __DEPTH__ #exe{StreamPrint("%d",\
	Fs->last_lex->lex_include_stk->depth);}
#define __FILE__ #exe{StreamPrint("\"%s\"",\
	Fs->last_lex->lex_include_stk->full_name);}
#define __DIR__  #exe{StreamDir;}

#define LFSF_DOC	1
#define LFSF_DEFINE	2
class CLexFile
{
  CLexFile *next;
  U8	*buf,
	*buf_ptr;
  I64	line_num,flags;
  U8	*full_name,
	*line_start;
  CDoc	*doc;
  CDocEntry *cur_entry;
  I32	depth;
  U8	last_U16,pad[3];
};

class CAOTCtrl
{
  I64	ip; //Instruction ptr
  CAsmArg arg1,arg2;
  CAOTBinBlk *bin;
  I64	num_bin_U8s,
	max_align_bits,module_org;
  CAsmUnresolvedRef *local_unresolved,*glbl_unresolved;
  CAOTAbsAddr *abss;
  CAOTHeapGlbl *heap_glbls;
  I64	lst_col,lst_last_ip;
  U8	*last_label,*lst_last_line;
  CLexFile *lst_last_lfn;
  I64	seg_size;
  Bool	lst;
};

//Tokens
#define TK_EOF		0
#define TK_SUPERSCRIPT	0x001
#define TK_SUBSCRIPT	0x002
#define TK_NORMALSCRIPT	0x003
#define TK_IDENT	0x100
#define TK_STR		0x101
#define TK_INTEGER	0x102
#define TK_CHAR_CONST	0x103
#define TK_F64		0x104
#define TK_PLUS_PLUS	0x105
#define TK_MINUS_MINUS	0x106
#define TK_DEREFERENCE	0x107
#define TK_DBL_COLON	0x108
#define TK_SHL		0x109
#define TK_SHR		0x10A
#define TK_EQU_EQU	0x10B
#define TK_NOT_EQU	0x10C
#define TK_LESS_EQU	0x10D
#define TK_GREATER_EQU	0x10E
#define TK_AND_AND	0x10F
#define TK_OR_OR	0x110
#define TK_XOR_XOR	0x111
#define TK_SHL_EQU	0x112
#define TK_SHR_EQU	0x113
#define TK_MUL_EQU	0x114
#define TK_DIV_EQU	0x115
#define TK_AND_EQU	0x116
#define TK_OR_EQU	0x117
#define TK_XOR_EQU	0x118
#define TK_ADD_EQU	0x119
#define TK_SUB_EQU	0x11A
#define TK_IF		0x11B
#define TK_IFDEF	0x11C
#define TK_IFNDEF	0x11D
#define TK_IFAOT	0x11E
#define TK_IFJIT	0x11F
#define TK_ENDIF	0x120
#define TK_ELSE		0x121
#define TK_MOD_EQU	0x122
#define TK_DOT_DOT	0x123
#define TK_DOT_DOT_DOT	0x124
#define TK_INS_BIN	0x125
#define TK_INS_BIN_SIZE 0x126
#define TK_NUM_TK	0x127

class CLexHashTableContext
{
  CLexHashTableContext *next;
  I64	old_flags,hash_mask;
  CHashFun *local_var_lst,
	*fun;
  CHashTable *hash_table_lst,
	*define_hash_table,
	*local_hash_table,
	*glbl_hash_table;
};

//CmpCtrl flags
#define CCF_PMT			1
#define CCf_PMT			0
#define CCF_QUESTION_HELP	2
#define CCF_DONT_FREE_BUF	4
#define CCF_NO_DEFINES		8
#define CCF_IN_IF		16
#define CCF_JUST_LOAD		32
#define CCF_KEEP_NEW_LINES	64
#define CCF_KEEP_DOT		128
#define CCF_KEEP_NUM_SIGN	256
#define CCF_KEEP_AT_SIGN	512
#define CCf_PASS_TRACE_PRESENT	10
#define CCF_NOT_CONST		0x0000010000
#define CCF_NO_REG_OPT		0x0000020000
#define CCF_IN_QUOTES		0x0000040000
#define CCF_EXE_BLK		0x0000080000
#define CCF_HAS_MISC_DATA	0x0000100000
#define CCF_HAS_RETURN		0x0000200000
#define CCF_ASM_EXPRESSIONS	0x0000400000
#define CCF_UNRESOLVED		0x0000800000
#define CCF_LOCAL		0x0001000000
#define CCF_FUN_EXP		0x0002000000
#define CCf_FUN_EXP		25
#define CCF_POSTINC		0x0004000000
#define CCF_POSTDEC		0x0008000000
#define CCF_PREINC		0x0010000000
#define CCF_PREDEC		0x0020000000
#define CCF_ARRAY		0x0040000000
#define CCF_RAX			0x0080000000
#define CCF_USE_LAST_U16	0x0100000000
#define CCf_USE_LAST_U16	32
#define CCF_LAST_WAS_DOT	0x0200000000
#define CCF_AOT_COMPILE		0x0400000000
#define CCf_AOT_COMPILE		34
#define CCF_NO_ABSS		0x0800000000
#define CCF_PAREN		0x1000000000
#define CCf_PAREN		36
#define CCF_CLASS_IP		0x2000000000
#define CCF_DONT_MAKE_RESULT	0x4000000000

public class CCmpCtrl
{
  CCmpCtrl *next,*last;
  I64	token,
	flags,
	cur_i64;
  F64	cur_f64;
  U8	*cur_str;
  I64	cur_str_len,
	class_ip;
  U8	*dollar_buf;
  I64	dollar_cnt;
  U8	*cur_help_index;
  I64	last_U16,
	min_line,max_line,last_line_num,
	lock_cnt;
  U32	*chars_bmp_alpha_numeric;
  CLexHashTableContext htc;
  CHashGeneric *hash_entry;
  CAbsCntsI64 abs_cnts;
  CAsmUndefHash *asm_undef_hash;
  CMemberLst *local_var_entry;
  CCodeMisc *lb_leave;
  U8	*cur_buf_ptr;
  CLexFile *lex_include_stk,
	*lex_prs_stk,
	*fun_lex_file;
  CStreamBlk *next_stream_blk,*last_stream_blk;
  CAOT	*aot;

  I64	pass,opts,pass_trace,saved_pass_trace,
	error_cnt,warning_cnt;

  //For intermediate codes with multiple float ops (int<->float conversions)
  I64	cur_ic_float_op_num,last_ic_float_op_num;
  CIntermediateCode *last_float_op_ic;
  Bool	last_dont_pushable,last_dont_popable,last_float_op_pos,
	dont_push_float,pad[4];

  CCodeCtrl coc;
  CPrsStk *ps;
  CAOTCtrl *aotc;
  I64	aot_depth,pmt_line;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#help_index "Compiler"
public class CCmpGlbls
{
  CHashTable *asm_hash;
  CHashClass *internal_types[RT_NUM_IT];
  CIntermediateCode ic_nop;
  U32	*dual_U16_tokens1,*dual_U16_tokens2,*dual_U16_tokens3,
	*binary_ops;
  I64	num_stk_temps,num_reg_vars,num_non_ptr_vars,
	stk_temps_mask,reg_vars_mask,non_ptr_vars_mask;
  U8	*to_stk_temps_map_odd,*to_stk_temps_map_even,
	*to_reg_vars_map,*non_ptr_vars_map;
  I64	size_arg_mask[9],
	compiled_lines;
};

#help_index "Debugging/Unassemble"
class CUAsmGlbls
{
  CInstruction **table_16_32,**table_64;
  I64	table_16_32_entries,
	table_64_entries,
	ins64_arg_mask,
	signed_arg_mask,
	mem_arg_mask;
};

#help_index "Devices;PCI"
class CPCIDev
{
  CPCIDev *next,*last;
  U16	vendor,dev_id;
  U8	bus,dev,fun,pad,
	sub_code,base_code,pad[6],
	*vendor_str,*dev_id_str;
};

#help_index "Devices;File/System;PCI"
public class CATARep
{
  CATARep *next;
  I64	num,type,base0,base1,unit,irq;
};

//See $LK,"::/Doc/Credits.TXT"$.
#define ATA_NOP			0x00
#define ATA_DEV_RST		0x08
#define ATA_PACKET		0xA0
#define ATA_READ_NATIVE_MAX	0xF8
#define ATA_READ_NATIVE_MAX_EXT	0x27
#define ATA_SET_MAX		0xF9
#define ATA_SET_MAX_EXT		0x37
#define ATA_READ_MULTI		0xC4
#define ATA_READ_MULTI_EXT	0x29
#define ATA_WRITE_MULTI		0xC5
#define ATA_WRITE_MULTI_EXT	0x39
#define ATA_ID_DEV		0xEC

#define ATAS_ERR	0x01
#define ATAS_DRQ	0x08
#define ATAS_DF		0x20
#define ATAS_DRDY	0x40
#define ATAS_BSY	0x80

#define ATAR0_DATA	0
#define ATAR0_FEAT	1
#define ATAR0_NSECT	2
#define ATAR0_SECT	3
#define ATAR0_LCYL	4
#define ATAR0_HCYL	5
#define ATAR0_SELECT	6
#define ATAR0_STAT	7
#define ATAR0_CMD	7
#define ATAR1_CTRL	2

#help_index "File/FileNames"
#define FILEMASK_JIT	"*.CPP*;*.HPP*"
#define FILEMASK_AOT	"*.CPP*;*.HPP*;*.PRJ*"
#define FILEMASK_SRC	"*.CPP*;*.HPP*;*.AUT*;*.PRJ*"
#define FILEMASK_TXT	FILEMASK_SRC ";*.TXT*;*.LOG*"
#define FILEMASK_GRA	"*.GRA*;*.BMP*"

#help_index "File/Low Level"
#define BLK_SIZE_BITS	9
#define BLK_SIZE	(1<<BLK_SIZE_BITS)
#define DVD_BLK_SIZE	(4*BLK_SIZE)
#define DVD_FILE_OFFSET	80
#define INVALID_CLUSTER (-1)

class CMBRPrt
{
  U8	active,	//0x80=active  0x00=inactive
	start_head;
  U16	start_cyl;
  U8	type,
	end_head;
  U16	end_cyl;
  U32	offset,	//Sects between MBR and first sect
	size;	//Sects in drv
};

class CMasterBoot
{
  U8	code[440];
  U32	media_id;
  U16	zero;
  CMBRPrt p[4];
  U16	signature; //AA55
};

class CRedSeaBoot
{
  U8	jump_and_nop[3],
	signature,reserved[4]; //MBR_PT_REDSEA=0x88. Distinguish from real FAT32
  I64	base_offset,	//For CD/DVD image copy.
	sects,
	root_cluster,
	bitmap_sects,
	unique_id;
  U8	code[462];
  U16	signature2;	//0xAA55
};

class CFAT32Boot
{
  U8	jump_and_nop[3],
	oem_name[8];
  U16	bytes_per_sect;
  U8	sects_per_cluster;
  U16	reserved_sects;
  U8	copies_of_fat;
  U16	max_root_dir_entries,	//Not used
	old_sects_in_drv;	//Not used
  U8	media_desc;		//F64 for hard disk
  U16	old_sects_per_fat,	//Not used
	sects_per_track,
	num_heads;
  U32	hidden_sects,
	sects,
	sects_per_fat;
  U16	flags,
	version;
  U32	root_cluster;
  U16	file_system_info_sect,
	backup_boot_sect;
  U8	reserved[12],
	log_drv_num,
	unused,
	ext_signature; //0x29
  U32	serial_num;
  U8	vol_name[11],
	fat_name[8],
	code[420];
  U16	signature;
};

class CFAT32FileInfoSect
{
  U32	signature1;
  U8	unknown[480];
  U32	signature2,
	free_clusters,
	most_recently_alloced;
  U8	reserved[12];
  U32	signature3;
};

class CFAT32DirEntry
{
  U8	name[11],
	attr,
	NTres,
	CrtTimeTenth;
  U16	CrtTime,
	CrtDate,
	LstAccDate,
	cluster_hi,
	WrtTime,
	WrtDate,
	cluster_lo;
  U32	size;
};

class CFAT32DirEntryLong
{
  U8	ord;
  U16	name1[5];
  U8	attr,
	type,
	xsum;
  U16	name2[6],
	zero,
	name3[2];
};

#define FAT32_ENTRIES_PER_BLK	(BLK_SIZE/sizeof(CFAT32DirEntry))
#define FAT32_ENTRIES_BITS	Bsf(FAT32_ENTRIES_PER_BLK)

class CPalindromeU16
{
  U16	little,big;
};

class CPalindromeU32
{
  U32	little,big;
};

class CATAPITrack
{
  U16	track_info_len;
  U8	track_lo,
	session_lo,
	reserved;
  U16	misc1;
  U8	misc2;
  U32	track_start,
	next_writable,
	free_blks;
  union {
    U32 fixed_packet_size;
    U32 blocking_factor;
  };
  union {
    U32 track_size;
    U32 rzone_end;
  };
  U32	last_recorded;
  U8	track_hi,
	session_hi;
  U16	reserved;
  U32	read_compatibility,
	next_layer_jmp,
	last_layer_jmp;
};

//ISO1 is ISO9660
//ISO2 is ISO13490
//ISO3 is ISO13346

class CISO1Date
{
  U8	year,mon,day,hour,min,sec,sec100;
};

class CISO1DirEntry
{
  U8	len,ext_attr_len;
  CPalindromeU32 loc;
  CPalindromeU32 size;
  CISO1Date date;
  U8	flags,file_unit_size,interleave;
  CPalindromeU16 vol_seq_num;
  U8	name_len,name;
};

class CISO1PathEntry
{
  U8	name_len,zero;
  U32	blk;
  U16	parent_entry_num,
	name[1]; //Aligned to U16 boundries
};

//ISO9660
#define ISO1T_BOOT_RECORD		0
#define ISO1T_PRI_VOL_DESC		1
#define ISO1T_SUPPLEMENTARY_DESC	2
#define ISO1T_VOL_DRV_DESC		3
#define ISO1T_TERMINATOR		255

class CISO1PriDesc
{
  U8	type,
	id[5],
	version,
	unused1,
	system_id[32],
	vol_id[32],
	unused2[8];
  CPalindromeU32 vol_space_size;
  U8	unused3[32];
  CPalindromeU16 vol_set_size;
  CPalindromeU16 vol_seq_num;
  CPalindromeU16 log_block_size;
  CPalindromeU32 path_table_size;
  U32	type_l_path_table,
	opt_type_l_path_table,
	type_m_path_table,
	opt_type_m_path_table;
  CISO1DirEntry root_dir_record;
  U8	vol_set_id[128],
	publisher_id[128],
	preparer_id[128],
	application_id[128],
	copyright_file_id[37],
	abstract_file_id[37],
	bibliographic_file_id[37],
	creation_date[17],
	modification_date[17],
	expiration_date[17],
	effective_date[17],
	file_structure_version,
	unused4,
	application_data[512],
	unused5[653];
};

#define ISO1_BASE_YEAR	1900
#define ISO1_ATTR_DIR	2

//Red Sea Attributes
//See $LK,"ST_FILE_ATTRS",A="MN:ST_FILE_ATTRS"$
#define RS_ATTR_READ_ONLY	0x01 //R
#define RS_ATTR_HIDDEN		0x02 //H
#define RS_ATTR_SYSTEM		0x04 //S
#define RS_ATTR_VOL_ID		0x08 //V
#define RS_ATTR_DIR		0x10 //D
#define RS_ATTR_ARCHIVE		0x20 //A
#define RS_ATTR_LONG_NAME	(RS_ATTR_READ_ONLY|RS_ATTR_HIDDEN|\
				RS_ATTR_SYSTEM|RS_ATTR_VOL_ID)
#define RS_ATTR_LONG_NAME_MASK	(RS_ATTR_LONG_NAME|RS_ATTR_DIR|RS_ATTR_ARCHIVE)
#define RS_ATTR_DELETED		0x100 //X
#define RS_ATTR_RESIDENT	0x200 //T
#define RS_ATTR_COMPRESSED	0x400 //Z
#define RS_ATTR_CONTIGUOUS	0x800 //C
#define RS_ATTR_FIXED		0x1000 //F

#help_index "File/CD DVD"
//Media types for $LK,"DVDImageWrite",A="MN:DVDImageWrite"$()
#define MT_CD		1
#define MT_DVD		2

#help_index "File/Low Level;File/Program Routines"
//CDirEntry flags (Used by $LK,"FileMgr",A="MN:FileMgr"$())
#define DEF_PROCESSED		1
#define DEF_REMOVABLE		2
#define DEF_NOT_INITIALIZED	4

#define CDIR_FILENAME_LEN	38 //Must include terminator zero
public class CDirEntry
{
  CDirEntry *next,*parent,*sub;
  U8	*full_name;
  I64	flags,user_data;

  U0	start;
  U16	attr;
  U8	name[CDIR_FILENAME_LEN];
  I64	cluster,size;
  CDate datetime;
};
#define CDIR_SIZE (sizeof(CDirEntry)-offset(CDirEntry.start))

#help_index "File/Program Routines"
//File Util Flags
//See $LK,"ST_FILE_UTIL_FLAGS",A="MN:ST_FILE_UTIL_FLAGS"$
#define FUf_RECURSE		0
#define FUf_DIFF		1
#define FUf_IGNORE		2
#define FUf_ALL			3
#define FUf_CANCEL		4
#define FUf_REPLACE		5
#define FUf_RISKY		5
#define FUf_PUBLIC		6
#define FUf_MAP			7
#define FUf_EXPAND		8
#define FUf_SINGLE		9
#define FUf_JUST_DIRS		10
#define FUf_JUST_FILES		11
#define FUf_JUST_TXT		12
#define FUf_JUST_SRC		13
#define FUf_JUST_AOT		14
#define FUf_JUST_JIT		15
#define FUf_JUST_GRA		16
#define FUf_Z_OR_NOT_Z		17
#define FUf_CLUSTER_ORDER	18
#define FUf_SCAN_PARENTS	19
#define FUf_FLATTEN_TREE	20
#define FUf_WHOLE_LABELS	21
#define FUf_WHOLE_LABELS_BEFORE	22
#define FUf_WHOLE_LABELS_AFTER	23

#define FUF_RECURSE		0x000001 //r
#define FUF_DIFF		0x000002 //d
#define FUF_IGNORE		0x000004 //i
#define FUF_ALL			0x000008 //a
#define FUF_CANCEL		0x000010 //c
#define FUF_REPLACE		0x000020 //R
#define FUF_RISKY		0x000020 //R
#define FUF_PUBLIC		0x000040 //p
#define FUF_MAP			0x000080 //m
#define FUF_EXPAND		0x000100 //x
#define FUF_SINGLE		0x000200 //s
#define FUF_JUST_DIRS		0x000400 //D
#define FUF_JUST_FILES		0x000800 //F
#define FUF_JUST_TXT		0x001000 //T
#define FUF_JUST_SRC		0x002000 //S
#define FUF_JUST_AOT		0x004000 //A
#define FUF_JUST_JIT		0x008000 //J
#define FUF_JUST_GRA		0x010000 //G
#define FUF_Z_OR_NOT_Z		0x020000 //Z
#define FUF_CLUSTER_ORDER	0x040000 //O Move disk head one direction
#define FUF_SCAN_PARENTS	0x080000 //P
#define FUF_FLATTEN_TREE	0x100000 //f
#define FUF_WHOLE_LABELS	0x200000 //l
#define FUF_WHOLE_LABELS_BEFORE	0x400000 //lb
#define FUF_WHOLE_LABELS_AFTER	0x800000 //la

#define FUG_FILES_FIND	(FUF_RECURSE|FUF_SINGLE|FUF_CLUSTER_ORDER|\
  FUF_JUST_DIRS|FUF_JUST_FILES|FUF_JUST_TXT|\
  FUF_JUST_SRC|FUF_JUST_AOT|FUF_JUST_JIT|FUF_JUST_GRA|FUF_FLATTEN_TREE)
#define FUG_FILE_FIND	(FUF_JUST_DIRS|FUF_JUST_FILES|FUF_Z_OR_NOT_Z|\
  FUF_SCAN_PARENTS)

#help_index "File/Low Level"
//See $LK,"ST_BLKDEV_TYPES",A="MN:ST_BLKDEV_TYPES"$
#define BDT_NULL		0
#define BDT_RAM			1
#define BDT_ATA			2
#define BDT_ATAPI		3
#define BDT_NUM_TYPES		4

#define BDF_REMOVABLE		0x001
#define BDF_INITIALIZED		0x002
#define BDF_READ_ONLY		0x004
#define BDF_READ_ONLY_OVERRIDE	0x008
#define BDF_LAST_WAS_WRITE	0x010
#define BDF_READ_CACHE		0x020
#define BDF_FMT			0x040
#define BDF_INIT_IN_PROGRESS	0x080
#define BDF_EXT_SIZE		0x100

#define BDf_REMOVABLE		0
#define BDf_INITIALIZED		1
#define BDf_READ_ONLY		2
#define BDf_READ_ONLY_OVERRIDE	3
#define BDf_LAST_WAS_WRITE	4
#define BDf_READ_CACHE		5
#define BDf_FMT			6
#define BDf_INIT_IN_PROGRESS	7
#define BDf_EXT_SIZE		8

//locked flags
#define BDlf_LOCKED		0

#define NUM_BLKDEVS		26
#define BD_SIGNATURE_VAL	'BDSV'
public class CBlkDev
{
  CBlkDev *lock_fwding; //If two blkdevs on same controller, use just one lock
  I64	locked_flags;
  U32	bd_signature,
	type,flags;
  U8	first_drv_let,unit,pad[2];
  U32	base0,base1,
	blk_size;
  I64	min_blk,max_blk;
  U16	*dev_id_record;
  U8	*RAM_dsk;
  CTask *owning_task;
  F64	last_time;
  U32	max_reads,max_writes;
};

//Drive locked_flags
#define DVlf_LOCKED	0

//See $LK,"ST_DRV_TYPES",A="MN:ST_DRV_TYPES"$
#define FSt_NULL	0
#define FSt_REDSEA	1 //Supported
#define FSt_FAT32	2 //Supported except for short names, to some degree
#define FSt_ISO9660	3 //Supported (CD/DVD)
#define FSt_ISO13490	4 //Not Supported (CD/DVD)
#define FSt_ISO13346	5 //Not Supported (CD/DVD)
#define FSt_NTFS	6 //Not Supported
#define FSt_UNKNOWN	7
#define FSt_NUM_TYPES	8
#define FSG_TYPE_MASK	0x7FFF
//File system type flags
#define FStf_DISABLE	15

#define MBR_PT_FAT12	0x01
#define MBR_PT_FAT32a	0x0B
#define MBR_PT_FAT32b	0x0C
#define MBR_PT_FAT32c	0x1B
#define MBR_PT_FAT32d	0x1C
#define MBR_PT_FAT32e	0x8B
#define MBR_PT_FAT32f	0x8C
#define MBR_PT_NTFS	0x07
#define MBR_PT_REDSEA	0x88

class CFreeLst
{
  CFreeLst *next,*last;
  I64	start,size;
};

#define NUM_DRVS		26

#define DRV_SIGNATURE_VAL	'DVSV'
public class CDrv
{
//Don't access ->drv_let directly in case a drive has been remapped.
  //Use $LK,"Drv2Let",A="MN:Drv2Let"$().
  I64	locked_flags;
  U32	dv_signature;
  U8	drv_let,pad;
  U16	fs_type;
  I64	base_offset,
	size,
	file_system_info_sect,
	fat1,fat2,
	root_cluster,
	data_area,
	spc; //sectors per cluster
  CDate fat32_local_time_offset;
  CTask *owning_task;
  CBlkDev *bd;

  CFAT32FileInfoSect *fis;
  I64	fat_blk_dirty,
	cur_fat_blk_num;
  U32	*cur_fat_blk;
  CFreeLst *next_free,*last_free;
};

#define DSK_CACHE_HASH_SIZE	0x2000

class CCacheBlk
{
  CCacheBlk *next_lru,*last_lru;
  CCacheBlk *next_hash,*last_hash;
  CDrv	*dv;
  I64	blk;
  U8	body[BLK_SIZE];
};

#help_index "File/System"
public class CBlkDevGlbls
{
  CBlkDev *blkdevs;
  U8	*dft_iso_filename;  //See $LK,"blkdev.dft_iso_filename",A="FF:::/Kernel/KEnd.CPP,blkdev.dft_iso_filename"$
  U8	*temp_filename;
  CCacheBlk *cache_base,*cache_ctrl,**cache_hash_table;
  I64	cache_size,read_cnt,write_cnt;
  CDrv	*drvs,*let_to_drv[32];
  I64	auto_mount_ide_cnt;
  U8	boot_drv_let,first_hd_drv_let,first_dvd_drv_let;
  Bool	dvd_boot_is_good,pad[4];
};

#help_index "File/Internal"
public class CDirContext
{
  CDrv	*old_dv,*dv;
  U8	*old_dir,*mask;
};

#help_index "File/CFile"
#define FFB_NEXT_BLK		MAX_I64

#define FF_WRITE		1
#define FF_NEW_FILE		2
#define FF_BUF_DIRTY		4
#define FF_NEEDS_WRITE		8
#define FF_CONTIGUOUS		16
#define FF_USE_OLD_DATETIME	32

public class CFile //See $LK,"::/Demo/Dsk/DataBase.CPP"$.
{
  I64	flags;
  CDirEntry de;
  CDrv	*dv;
  I64	fblk_num,cluster,file_cluster_num,max_blk;
  U8	*cluster_buf;
};

#help_index "Memory/Heap"
#define _CFG_HEAP_DBG FALSE

#if _CFG_HEAP_DBG
class CMemUnused
{
  CHeapCtrl *hc;
  U8	*caller1,*caller2;
  CMemUnused *next;
  I64	size;
};
class CMemUsed
{
  CHeapCtrl *hc;
  U8	*caller1,*caller2;
  CMemUsed *next,*last;
  I64	size;
  U0	start;
};
#else
class CMemUnused
{
  U0	hc;
  U0	caller1,caller2;
  CMemUnused *next;
  I64	size;
};
class CMemUsed
{
  CHeapCtrl *hc;
  U0	caller1,caller2;
  U0	next,last;
  I64	size;
  U0	start;
};
#endif

#help_index "Memory/BlkPool"
/*TempleOS does not mess with page tables after it
identity-maps them during boot -- $LK,"SYS_INIT_MEM",A="MN:SYS_INIT_MEM"$.
$LK,"MEM_PAGE_SIZE",A="MN:MEM_PAGE_SIZE"$ is arbitrary, and is not related to CPU hardware.
I need some unit for memory allocation so I invented "$LK,"MEM_PAGE_SIZE",A="MN:MEM_PAGE_SIZE"$".

All virtual addresses are identical to physical, always.
There is no high kernel memory because RAM's physical
addresses are low.  (All motherboards that I know of
put RAM in low addresses of 64-bit address space.)
It's ring-0-only so all (or none) is kernel, remember.

2 Meg hardware pages are use except the first 2 Meg.
0xA0000-0xBFFFF must be write thru.  Hardware pages
don't matter except the caching bit.	Remember, it is ring-0-only
and identity-mapped!	Do I need to repeat?
Ring-0-only and 100% identity-mapped, all the time.

All code is placed in just the lowest 2 Gig, so the CALL REL32
instruction always works.  It never changes out of ring-0, remember,
so I don't need to waste cycles on use SYSCALL or software interrupts.
Everything just uses CALL REL32.
*/

#define MBS_USED_SIGNATURE_VAL		'MBUs'
#define MBS_UNUSED_SIGNATURE_VAL	'MBUn'
class CMemBlk
{
  CMemBlk *next,*last;
  U32	mb_signature,pages;
};

#define MRT_UNUSED	0
#define MRT_RAM		1
#define MRT_DEV		2

class CMemRange
{
  CMemRange *next,*last;
  U32	type,flags;
  U8	*base;
  I64	size;
};

#define MEM_PAGE_BITS		9
#define MEM_PAGE_SIZE		(1<<MEM_PAGE_BITS)
#define MEM_HEAP_HASH_SIZE	1024
#define MEM_FREE_PAGE_HASH_SIZE	0x100
//It is common to $LK,"MAlloc",A="MN:MAlloc"$() exact powers of two.  There is some overhead.
//We add $TX,"2",D="MEM_EXTRA_HASH2_PAGES"$ pages, so a request is not rounded-up to next power of two.
#define MEM_EXTRA_HASH2_PAGES	2

#define MEM_ADAM_STK		(MEM_PAGE_SIZE*512) //Like 16384*MEM_PAGE_SIZE
#define MEM_SETH_STK		(MEM_PAGE_SIZE*512)
#define MEM_INTERRUPT_STK	(MEM_PAGE_SIZE*512)
#define MEM_DFT_STK		(MEM_PAGE_SIZE*512)

#define TASK_HASH_TABLE_SIZE	(1<<10)

//locked flags
#define BPlf_LOCKED		0
public class CBlkPool
{
  I64	locked_flags,alloced_u8s,used_u8s;
  CMemBlk *mem_free_lst,
	*mem_free_2meg_lst, //This is for $LK,"::/Demo/Lectures/Mem/Mem2Meg.CPP"$
	*free_page_hash[MEM_FREE_PAGE_HASH_SIZE],
	*free_page_hash2[64-MEM_PAGE_BITS];
};

#help_index "Memory/HeapCtrl"
//locked flags
#define HClf_LOCKED		0

#define HEAP_CTRL_SIGNATURE_VAL	'HcSV'
public class CHeapCtrl
{
  CBlkPool *bp;
  U32	hc_signature,pad;
  I64	locked_flags,alloced_u8s,used_u8s;
  CTask *mem_task;
  CMemBlk *next_mem_blk,*last_mem_blk;
  CMemBlk *last_mergable;
  CMemUnused *malloc_free_lst;
  CMemUsed *next_um,*last_um;
  CMemUnused *heap_hash[MEM_HEAP_HASH_SIZE/sizeof(U8 *)];
};

#help_index "Devices"
public class CDevGlbls
{
  I64	*idt;
  U8	*mem64_ptr;
  U8	mp_apic_ids[MP_MAX_PROCESSORS];
  CMemRange mem32_root;
  CPCIDev pci_root;
};

#help_index "Graphics/Color;Graphics/Device Contexts"
//Raster operations
#define ROPB_EQU			0x00
#define ROPB_XOR			0x01
#define ROPB_COLLISION			0x02
#define ROPB_MONO			0x03
#define ROPBF_HALF_RANGE_COLOR		0x10
#define ROPBF_TWO_SIDED			0x20
#define ROPBF_DITHER			0x40
#define ROPBF_PROBABILITY_DITHER	0x80

#define ROP_EQU				(ROPB_EQU<<8)
#define ROP_XOR				(ROPB_XOR<<8)
#define ROP_COLLISION			(ROPB_COLLISION<<8)
#define ROP_MONO			(ROPB_MONO<<8)

//These are just for $LK,"ROPF_PROBABILITY_DITHER",A="MN:ROPF_PROBABILITY_DITHER"$
//See $LK,"DCLighting",A="MN:DCLighting"$().
#define ROPF_HALF_RANGE_COLOR		0x1000
#define ROPF_TWO_SIDED			0x2000

//These always go in the c1.rop of a CColorROPU32
#define ROPF_DITHER			0x40000000
#define ROPF_PROBABILITY_DITHER		0x80000000

#help_index "Graphics/Color"
#define TRANSPARENT		0xFF
#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED			4
#define PURPLE			5
#define BROWN			6
#define LTGRAY			7
#define DKGRAY			8
#define LTBLUE			9
#define LTGREEN			10
#define LTCYAN			11
#define LTRED			12
#define LTPURPLE		13
#define YELLOW			14
#define WHITE			15

#define NUM_COLORS		16
#define COLOR_INVALID		16
#define COLOR_MONO		0xFF

public U16 class CColorROPU16 //Don't use this, use CColorROPU32
{
  U8	color,rop;
};
public U32 class CColorROPU32
{
  CColorROPU16 c0,c1;
};
#define COLORROP_COLORS_MASK	0x00FF00FF
#define COLORROP_NO_ROP0_MASK	0xFFFF00FF
#define COLORROP_BITS		16
public U32 class CBGR24
{
  U8	b,g,r,pad;
};
public I64 class CBGR48
{
  U16	b,g,r,pad;
};

#help_index "Keyboard Devices;Char/Input;StdIn"
#define KBD_PORT	0x60
#define KBD_CTRL	0x64
public class CKbdStateGlbls
{
  I64	scan_code,	//See $LK,"scan codes",A="FI:::/Doc/CharOverview.TXT"$
	last_down_scan_code,
	cnt,		//Count of keys pressed since boot.
	timestamp,	//Output: GetTSC when event.
	new_key_timestamp; //Output: GetTSC when new key event.
  CFifoU8 *fifo,*fifo2; //Private
  CFifoI64 *scan_code_fifo;

  //BitTest, $LK,"Bt",A="MN:Bt"$(), with a merged scan code.
  //(Left and right shift merged, for example.)
  U32	down_bitmap[8],
//BitTest, $LK,"Bt",A="MN:Bt"$(), with an unmerged scan code.
	down_bitmap2[8];

  Bool	rst,		//Private: Reset KbdMouse
	irqs_working;	//Private
};

#help_index "Input Pointer/Mouse"
public class CMouseStateGlbls
{
  CD3I64 pos,		//Position in pixels
	prescale;
  CD3	scale;
  F64	speed;		//Output: How fast the user is moving it.
  I64	timestamp,	//Output: GetTSC when event.
	install_attempts, //Private
	pkt_size;	//Private
  CFifoU8 *fifo,*fifo2; //Private
  Bool	buttons[5],
	has_wheel,
	has_ext_buttons,
	evt,		//Private
	installed,	//Private
	install_in_progress, //Private
	irqs_working;	//Private
};

#help_index "Input Pointer"
#define IP_NULL		0
#define IP_MOUSE	1
public class CIPStateGlbls
{
  I64	dev;		//$LK,"IP_NULL",A="MN:IP_NULL"$ or $LK,"IP_MOUSE",A="MN:IP_MOUSE"$
  CD3I64 pos,		//Position in pixels
	pos_text,	//Position in text rows,cols
	presnap,
	offset;
  CD3	scale;
  I64	throttle,	//Throttle is [MIN_I32,MAX_I32]
	throttle_step;
  F64	speed;		//Output: How fast the user is moving it.
  I64	timestamp;	//Output: GetTSC when event.
  F64	dbl_time,	//Input: Time threshold for calling it a double click.
	left_dbl_time,	//Private
	right_dbl_time;	//Private
  Bool	lb,		//Left Button
	rb,		//Right Button
	show,
	has_wheel,
	left_dbl,	//Private
	left_down_sent, //Private
	right_dbl,	//Private
	right_down_sent;//Private
};

public class CGridGlbls
{
  Bool	snap	fmtstr "$$CB,\"Snap Grid\"$$\n",
	show	fmtstr "$$CB,\"Show Grid\"$$\n";
  U8	pad[6];
  F64	x	fmtstr "$$DA-TRM,RT=F64,A=\"X Spacing:%6.3f\"$$\n",
	y	fmtstr "$$DA-TRM,RT=F64,A=\"Y Spacing:%6.3f\"$$\n",
	z	fmtstr "$$DA-TRM,RT=F64,A=\"Z Spacing:%6.3f\"$$\n",
	x_offset fmtstr "$$DA-TRM,RT=F64,A=\"X Offset :%6.3f\"$$\n",
	y_offset fmtstr "$$DA-TRM,RT=F64,A=\"Y Offset :%6.3f\"$$\n",
	z_offset fmtstr "$$DA-TRM,RT=F64,A=\"Z Offset :%6.3f\"$$\n",
	x_speed	fmtstr "$$DA-TRM,RT=F64,A=\"X Speed  :%6.3f\"$$\n",
	y_speed	fmtstr "$$DA-TRM,RT=F64,A=\"Y Speed  :%6.3f\"$$\n",
	z_speed	fmtstr "$$DA-TRM,RT=F64,A=\"Z Speed  :%6.3f\"$$\n";
};

#help_index "Ctrls"
#define CTRLT_GENERIC		0
#define CTRLT_WIN_HSCROLL	1 //unique
#define CTRLT_WIN_VSCROLL	2 //unique
#define CTRLT_VIEWING_ANGLES	3 //unique

#define CTRLF_SHOW		1
#define CTRLF_BORDER		2
#define CTRLF_CAPTURE_LEFT_IP	4
#define CTRLF_CAPTURE_RIGHT_IP	8
#define CTRLF_CLICKED		16

#define WSSf_SET_TO_POS		0

public class CWinScroll
{
  I64	min,pos,max;
  U32	flags;
  U8	color,pad[3];
};

public class CViewAngles
{
  I64	sx,sy,sz;
  F64	ax,ay,az;
  CColorROPU32 cx,cy,cz,cbd,cbg,cfg;
};

public class CCtrl
{
  CCtrl *next,*last;
  CTask *win_task;
  I64	type,flags;

  //win pix coordinates
  I64	left,right,top,bottom;

  //screen pix coordinates (derived)
  I64	screen_left,screen_right,screen_top,screen_bottom;

  U8	*state;

  //called on resize
  U0	(*update_derived_vals)(CCtrl *c);
  U0	(*draw_it)(CDC *dc,CCtrl *c);

  Bool	(*inside_ctrl)(CCtrl *c,I64 x,I64 y); //For nonbox shapes
  U0	(*left_click)(CCtrl *c,I64 x,I64 y,Bool down);
  U0	(*right_click)(CCtrl *c,I64 x,I64 y,Bool down);
  U0	(*wheel_chg)(CCtrl *c,I64 delta);
};

#help_index "Menus"
public class CMenuEntry
{
  CMenuEntry *next;
  CMenuEntry *sub;
  U8	name[32];
  I64	msg_code,a1,a2;
  Bool	checked,dir,pad[6];
};

public class CMenu
{
  CMenu *next;
  CMenuEntry *sub;
  CTask *task;
  I64	flags;
  U8	attr,pad[7];
};

#help_index "Task"
class CBpt
{
  CBpt	*next;
  U8	*addr,
	val,pad[7];
};

class CExcept
{
  CExcept *next,*last;
  I64	handler_catch,handler_untry,
	rsp,rbp,rflags,rsi,rdi,r10,r11,r12,r13,r14,r15;
};

class CFPU
{
  U8	body[512];
};

#help_index "Task/Srv/Exe"
#define SVCf_WAKE_MASTER	0
#define SVCf_FOCUS_MASTER	1
#define SVCf_EXIT_ON_COMPLETE	2
#define SVCf_DONT_FILTER	3
#define SVCf_HIGHEST_PRIORITY	4
//MP flags
#define SVCf_DONE		5
#define SVCf_DISPATCHED		6
#define SVCf_FREE_ON_COMPLETE	7
#define SVCf_ADD_TO_QUE		8

#define SVCT_TEXT_INPUT		0 //$LK,"TaskText",A="MN:TaskText"$()	Feed StdIn
#define SVCT_MSG		1 //$LK,"TaskMsg",A="MN:TaskMsg"$()	Post msg
#define SVCT_EXE_STR		2 //$LK,"TaskExe",A="MN:TaskExe"$()	Compile & execute src code text
//MP cmds
#define SVCT_CALL		3 //$LK,"JobQue",A="MN:JobQue"$()	Tell MP to call function
#define SVCT_SPAWN_TASK		4 //$LK,"Spawn",A="MN:Spawn"$()	Tell MP to spawn task

class CSrvCmd
{
  CSrvCmd *next,*last;
  CSrvCtrl *ctrl;
  I64	cmd_code,flags,
	target_cpu,servant_cpu_num,
	msg_code;

  I64	(*addr)(U8 *fun_arg);
  U8	*fun_arg;

  U8	*aux_str;
  I64	aux1,aux2, //Sometimes called arg1 and arg2. (Windows msg param1 param2)
	result;

  CTask	*spawned_task,
	*master_task;
};

#define SVCRf_LOCKED		0
class CSrvCtrl
{
  CSrvCmd *next_waiting,*last_waiting;
  CSrvCmd *next_done,*last_done;
  I64	flags;
};

#help_index "Messages"
//See $LK,"::/Demo/MsgLoop.CPP"$
#define MSG_NULL		0
#define MSG_CMD			1
#define MSG_KEY_DOWN		2  //($LK,"ASCII",A="MN:CH_CTRLA"$,$LK,"scan code",A="FI:::/Doc/CharOverview.TXT"$) Press <CTRL-SHIFT-L>
#define MSG_KEY_UP		3  //($LK,"ASCII",A="MN:CH_CTRLA"$,$LK,"scan code",A="FI:::/Doc/CharOverview.TXT"$) Press <CTRL-SHIFT-L>
#define MSG_IP_MOVE		4  //(x,y)
#define MSG_IP_L_DOWN		5  //(x,y)
#define MSG_IP_L_UP		6  //(x,y)
#define MSG_IP_L_D_DOWN 	7  //(x,y)
#define MSG_IP_L_D_UP		8  //(x,y)
#define MSG_IP_R_DOWN		9  //(x,y)
#define MSG_IP_R_UP		10 //(x,y)
#define MSG_IP_R_D_DOWN 	11 //(x,y)
#define MSG_IP_R_D_UP		12 //(x,y)
#define MSG_FOCUS		13 //(flag,0) TRUE gaining focus, FALSE losing.
#define MSG_RESIZE		14 //(new columns,new rows).
#define MSG_MOVE		15 //(new left,new top).

#define MSG_KEY_DOWN_UP		-2  //Down & UP
#define MSG_IP_L_DOWN_UP	-5  //Down & Up
#define MSG_IP_L_D_DOWN_UP	-7  //Down & Up
#define MSG_IP_R_DOWN_UP	-9  //Down & Up
#define MSG_IP_R_D_DOWN_UP	-11 //Down & Up

#help_index "Task/Settings"
#define TSF_SAME_SONG	1
public class CTaskSettings
{
  CTaskSettings *next;
  U8	*cur_dir;
  I64	left,right,top,bottom;
  U0	(*draw_it)(CTask *task,CDC *dc);
  U0	(*task_end_cb)();
  CTask *song_task,*animate_task;
  I64	scroll_x,scroll_y,scroll_z;
  CBGR48 palette[NUM_COLORS];
  U32	win_inhibit;;
  U8	text_attr,title_src,
	border_attr,border_src,
	task_title[STR_LEN];
  Bool	border,hide_cursor,highlight_cursor,scroll,autocomplete,pad[3];
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#help_index "Task"
//CTask.border_src
#define BDS_CONST		0
#define BDS_CUR_DRV		1
#define BDS_ED_FILENAME_DRV	2

//CTask.title_src
#define TTS_CONST		0
#define TTS_LOCKED_CONST	1 //This is not strictly enforced
#define TTS_TASK_NAME		2
#define TTS_ED_FILENAME		3
#define TTS_CUR_LEX		4

//CTask.task_flags
#define TASKf_TASK_LOCK		0
#define TASKf_KILL_TASK		1
#define TASKf_SUSPENDED		2
#define TASKf_IDLE		3
#define TASKf_INPUT_FILTER_TASK	4
#define TASKf_FILTER_INPUT	5
#define TASKf_HAS_SONG		6
#define TASKf_DISABLE_BPTS	7
#define TASKf_AWAITING_MSG	8
#define TASKf_BREAK_LOCKED	9
#define TASKf_PENDING_BREAK	10
#define TASKf_BREAK_TO_SHIFT_ESC 11
#define TASKf_KILL_AFTER_DBG	12
#define TASKf_NONTIMER_RAND	13

//CTask.display_flags
#define DISPLAYf_SHOW			0
#define DISPLAYf_NOT_RAW		1
#define DISPLAYf_SILENT			2
#define DISPLAYf_NO_BORDER		3
#define DISPLAYf_WIN_ON_TOP		4
#define DISPLAYf_CHILDREN_NOT_ON_TOP	5

#define TASK_SIGNATURE_VAL		'TskS'
#define TASK_NAME_LEN			32
class CTaskStk
{
  CTaskStk *next_stk;
  I64	stk_size,stk_ptr;
  U0	stk_base;
};

#define DYING_JIFFIES	ToI64(JIFFY_FREQ/5)
class CTaskDying
{
  CTask *next,*last;
  I64	wake_jiffy;
};

public class CTask //The Fs segment reg points to current CTask.
{
  CTask *addr; //Self-addressed ptr
  U32	task_signature,win_inhibit;
U0 pad;#assert $$==offset(CTaskDying.wake_jiffy)
  I64	wake_jiffy;
  U32	task_flags,display_flags;

  CHeapCtrl *code_heap,*data_heap;

  CDoc	*put_doc,*display_doc, //When double buffering, these two differ.
	*border_doc;
  I64	win_left,win_right,win_top,win_bottom;

  CDrv	*cur_dv;
  U8	*cur_dir;

  CTask *parent_task,
	*next_task,*last_task,
	*next_input_filter_task,*last_input_filter_task,
	*next_sibling_task,*last_sibling_task,
	*next_child_task,*last_child_task;

  //These are derived from left,top,right,bottom
  I64	win_width,win_height,
	pix_left,pix_right,pix_width, //These are in pixs, not characters
	pix_top,pix_bottom,pix_height,
	border_pix_left,border_pix_right,
	border_pix_top,border_pix_bottom,

	scroll_x,scroll_y,scroll_z;

  //These must be in this order
  //for $LK,"TASK_CONTEXT_SAVE",A="FF:::/Kernel/Sched.CPP,TASK_CONTEXT_SAVE"$ and $LK,"_TASK_CONTEXT_RESTORE",A="FF:::/Kernel/Sched.CPP,_TASK_CONTEXT_RESTORE"$
  I64	rip,rflags,rsp,rsi,rax,rcx,rdx,rbx,rbp,rdi,
	r8,r9,r10,r11,r12,r13,r14,r15;
  CCPU	*gs;
  CFPU	*fpu_mmx;
  I64	time_slice_start,
	total_time, //Cycles
	swap_cnter;

  U0	(*draw_it)(CTask *task,CDC *dc);

  U8	task_title[STR_LEN],
	task_name[TASK_NAME_LEN],
	wallpaper_data[STR_LEN],

	title_src,border_src,
	text_attr,border_attr;
  U16	win_z_num,pad;

  CTaskStk *stk;

  CExcept *next_except,*last_except;
  I64	except_rbp,	//throw routine's RBP
	except_ch;	//throw(ch)
  U8	*except_caller1,*except_caller2;

  Bool	catch_except;
  Bool	new_answer;
  U8	answer_type,pad[5];
  I64	answer;
  F64	answer_time;
  CBpt	*bpt_lst;
  CCtrl	*next_ctrl,*last_ctrl;
  CMenu	*cur_menu;
  CTaskSettings *next_settings;
  CMathODE *next_ode,*last_ode;
  F64	last_ode_time;
  CHashTable *hash_table;

  CSrvCtrl srv_ctrl;
  CCmpCtrl *next_lex,*last_lex;

  I64	win_old_left,win_old_right,win_old_top,win_old_bottom;

  U0	(*task_end_cb)();
  CTask *song_task,*animate_task;
  I64	rand_seed,
	task_num,
	fault_num,fault_err_code;
  CTask *popup_task,
	*dbg_task;
  CWinScroll horz_scroll,vert_scroll;

  I64	user_data;
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

class CTSS
{
  U32	res1;
  I64	rsp0,rsp1,rsp2,res2,
	ist1,ist2,ist3,ist4,ist5,ist6,ist7,res3;
  U16	res4,io_map_offset;
  U8	io_map[0x10000/8];
  I64	*st0,*st1,*st2;
  U16	tr,tr_ring3;
};

#define ans	(Fs->answer)
#define ansf	(Fs->answer(F64))

#define _RAX Fs->rax
#define _RBX Fs->rbx
#define _RCX Fs->rcx
#define _RDX Fs->rdx
#define _RSI Fs->rsi
#define _RDI Fs->rdi
#define _RBP Fs->rbp
#define _RSP Fs->rsp
#define _RIP Fs->rip
#define _R8  Fs->r8
#define _R9  Fs->r9
#define _R10 Fs->r10
#define _R11 Fs->r11
#define _R12 Fs->r12
#define _R13 Fs->r13
#define _R14 Fs->r14
#define _R15 Fs->r15

#help_index "MultiCore"
#define CPUf_RAN_A_TASK		0
#define CPUf_DYING_TASK_QUE	1

public class CCPU //The Gs segment reg points to current CCPU.
{
  CCPU	*addr; //Self-addressed ptr
  I64	num,cpu_flags,
	startup_rip,
	idle_pt_hits;
  F64	idle_factor;
  I64	total_jiffies;
  CTask *seth_task,*idle_task;
  I64	tr,   //task reg
	swap_cnter;
  U0	(*profiler_timer_irq)(CTask *task);
  CTaskDying *next_dying,*last_dying;
  I64	kill_jiffy;
  CTSS	*tss;
  I64	start_stk[16];
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#help_index "Memory/Page Tables"
#define MEM_MIN_MEG		512 //512 Meg minimum.
#define MEM_MAPPED_SPACE	0x2000000000 //Arbitrarily set to 128 Gig.
#define MEM_NUM_PML1		1 //First 2 Meg are 4K resolution.
#define MEM_NUM_PML2		(MEM_MAPPED_SPACE>>(9+21))
#define MEM_NUM_PML3		1
#define MEM_NUM_PML4		1
#define MEM_PROTECTED_LOW_LIMIT	0x6FFF

#define SYS_FIXED_AREA	0x100000
public class CSysFixedArea
{
  U8	pml1[MEM_NUM_PML1*0x1000],pml2[MEM_NUM_PML2*0x1000],
	pml3[MEM_NUM_PML3*0x1000],pml4[MEM_NUM_PML4*0x1000];
  $$=($$+15)&-16;
  CFPU	init_fpu_mmx;
  CCPU	boot_cpu;
  CTask	adam;
  CBlkPool adam_bp;
  CHeapCtrl adam_hc;
  $$=($$+MEM_PAGE_SIZE-1)&-MEM_PAGE_SIZE;
};

#help_index "Char"
#define CH_CTRLA	0x01
#define CH_CTRLB	0x02
#define CH_CTRLC	0x03
#define CH_CTRLD	0x04
#define CH_CTRLE	0x05
#define CH_CTRLF	0x06
#define CH_CTRLG	0x07
#define CH_CTRLH	0x08
#define CH_CTRLI	0x09
#define CH_CTRLJ	0x0A
#define CH_CTRLK	0x0B
#define CH_CTRLL	0x0C
#define CH_CTRLM	0x0D
#define CH_CTRLN	0x0E
#define CH_CTRLO	0x0F
#define CH_CTRLP	0x10
#define CH_CTRLQ	0x11
#define CH_CTRLR	0x12
#define CH_CTRLS	0x13
#define CH_CTRLT	0x14
#define CH_CTRLU	0x15
#define CH_CTRLV	0x16
#define CH_CTRLW	0x17
#define CH_CTRLX	0x18
#define CH_CTRLY	0x19
#define CH_CTRLZ	0x1A
#define CH_CURSOR	0x05
#define CH_BACKSPACE	0x08
#define CH_FORM_FEED	0x0C
#define CH_ESC		0x1B
#define CH_SHIFT_ESC	0x1C
#define CH_SHIFT_SPACE	0x1F
#define CH_SPACE	0x20

#define ST_ERR_ST   "$$LTRED$$$$BK,1$$ERROR:$$FG$$$$BK,0$$ "
#define ST_WARN_ST  "$$RED$$$$BK,1$$WARNING:$$FG$$$$BK,0$$ "

//Scan code flags
#define SCf_E0_PREFIX	7
#define SCf_KEY_UP	8
#define SCf_SHIFT	9
#define SCf_CTRL	10
#define SCf_ALT 	11
#define SCf_CAPS	12
#define SCf_NUM 	13
#define SCf_SCROLL	14
#define SCf_NEW_KEY	15
#define SCf_IP_L_DOWN	16
#define SCf_IP_R_DOWN	17
#define SCf_DELETE	18
#define SCf_INS		19
#define SCf_NO_SHIFT	30
#define SCf_KEY_DESC	31
#define SCF_E0_PREFIX	(1<<SCf_E0_PREFIX)
#define SCF_KEY_UP	(1<<SCf_KEY_UP)
#define SCF_SHIFT	(1<<SCf_SHIFT)
#define SCF_CTRL	(1<<SCf_CTRL)
#define SCF_ALT		(1<<SCf_ALT)
#define SCF_CAPS	(1<<SCf_CAPS)
#define SCF_NUM		(1<<SCf_NUM)
#define SCF_SCROLL	(1<<SCf_SCROLL)
#define SCF_NEW_KEY	(1<<SCf_NEW_KEY)
#define SCF_IP_L_DOWN	(1<<SCf_IP_L_DOWN)
#define SCF_IP_R_DOWN	(1<<SCf_IP_R_DOWN)
#define SCF_DELETE	(1<<SCf_DELETE)
#define SCF_INS 	(1<<SCf_INS)
#define SCF_NO_SHIFT	(1<<SCf_NO_SHIFT)
#define SCF_KEY_DESC	(1<<SCf_KEY_DESC)

//TempleOS places a 1 in bit 7 for
//keys with an E0 prefix.
//See $LK,"::/Doc/CharOverview.TXT"$ and $LK,"KbdHandler",A="MN:KbdHandler"$().
#define SC_ESC		0x01
#define SC_BACKSPACE	0x0E
#define SC_TAB		0x0F
#define SC_ENTER	0x1C
#define SC_SHIFT	0x2A
#define SC_CTRL		0x1D
#define SC_ALT		0x38
#define SC_CAPS		0x3A
#define SC_NUM		0x45
#define SC_SCROLL	0x46
#define SC_CURSOR_UP	0x48
#define SC_CURSOR_DOWN	0x50
#define SC_CURSOR_LEFT	0x4B
#define SC_CURSOR_RIGHT 0x4D
#define SC_PAGE_UP	0x49
#define SC_PAGE_DOWN	0x51
#define SC_HOME		0x47
#define SC_END		0x4F
#define SC_INS		0x52
#define SC_DELETE	0x53
#define SC_F1		0x3B
#define SC_F2		0x3C
#define SC_F3		0x3D
#define SC_F4		0x3E
#define SC_F5		0x3F
#define SC_F6		0x40
#define SC_F7		0x41
#define SC_F8		0x42
#define SC_F9		0x43
#define SC_F10		0x44
#define SC_F11		0x57
#define SC_F12		0x58
#define SC_PAUSE	0x61
#define SC_GUI		0xDB
#define SC_PRTSCRN1	0xAA
#define SC_PRTSCRN2	0xB7

#help_index "Char;Debugging/Raw Output;TextBase Layer/Char"
//text.raw_flags
#define RWF_IN_DOLLAR	1
#define RWF_LAST_DOLLAR	2
#define RWF_SHOW_DOLLAR	4
#define RWF_SCROLL	8

public class CTextGlbls
{
  I64	raw_col,raw_flags;
  U8	*raw_screen_image;
  I64	rows,cols;	//Use TEXT_ROWS,TEXT_COLS
  U64	*font,*aux_font;
};

#define FONT_WIDTH	8
#define FONT_HEIGHT	8

#help_index "Graphics"
#define GR_WIDTH	640
#define GR_HEIGHT	480

//z-vals less than zero are in front of screen and not drawn.
//we want to shift all Z-vals into a drawable range.
//GR_Z_ALL is set to half of the Z-range which is an I32.
#define GR_Z_ALL	(MAX_I32/2)

#help_index "Graphics/Device Contexts"
//Low 8 bits reserved for flags that go into saved bitmaps
#define DCF_COMPRESSED		1
#define DCF_PALETTE		2
#define DCF_NO_TRANSPARENTS	4 //Can be used to optimized $LK,"GrBlot",A="MN:GrBlot"$().

#define DCF_TRANSFORMATION	0x100

//See $LK,"DCSymmetrySet",A="MN:DCSymmetrySet"$() or $LK,"DCSymmetry3Set",A="MN:DCSymmetry3Set"$()
#define DCF_SYMMETRY		0x200

//Must be used with DCF_SYMMETRY set also.
//See $LK,"::/Demo/Games/BigGuns.CPP"$
#define DCF_JUST_MIRROR		0x400

#define DCF_LOCATE_NEAREST	0x800
#define DCF_DONT_DRAW		0x1000
#define DCF_ALIAS		0x2000
#define DCF_SCREEN_BITMAP	0x4000
#define DCF_FILL_NOT_COLOR	0x8000
#define DCF_RECORD_EXTENTS	0x10000
#define DCF_ON_TOP		0x20000

//$LK,"DCSave",A="MN:DCSave"$() flags.
#define DCSF_COMPRESSED		1
#define DCSF_PALETTE_GET	2

#define DCS_SIGNATURE_VAL	'DvCS'

class CGrSym
{
  I32	sx,sy,sz,pad;
//Normal of symmetry plane
  I64	snx,sny,snz;
};

public class CDC
{
  U0	start;
  I32	width,width_internal,
	height,
	flags;
  U0	end;
  CBGR48 palette[NUM_COLORS];

  //public (Change directly)
  CColorROPU32 color,
	bkcolor, //Set for use with $LK,"ROP_COLLISION",A="MN:ROP_COLLISION"$
	color2; //Internally used for $LK,"GrFloodFill",A="MN:GrFloodFill"$()
  CD3I32 ls; //Light source (should be normalized to 65536).

  //dither_probability_u16 is basically a U16.
  //It is activated by $LK,"ROPF_PROBABILITY_DITHER",A="MN:ROPF_PROBABILITY_DITHER"$.
  //0x0000 =100% color.c0
  //0x8000 =50%  color.c0   50% color.c1
  //0x10000=100% color.c1
  //See $LK,"::/Demo/Graphics/SunMoon.CPP"$ and	$LK,"::/Demo/Graphics/Shading.CPP"$.
  U64 dither_probability_u16;

  CDC *brush;

  //Set with $LK,"DCMat4x4Set",A="MN:DCMat4x4Set"$().  $LK,"Free",A="MN:Free"$() before setting.
  I64	*r, //rotation matrix of quads decimal in lo
	r_norm; //shifted 32 bits.  Used for scaling pen width

  //public (Change directly)
  I32	x,y,z,
	pen_width;

  //Can be changed from the default $LK,"DCTransform",A="MN:DCTransform"$()
  U0	(*transform)(CDC *dc,I64 *x,I64 *y,I64 *z);
//Can be changed from the default $LK,"DCLighting",A="MN:DCLighting"$()
  U0	(*lighting)(CDC *dc,CD3I32 *p1,CD3I32 *p2,
	CD3I32 *p3,CColorROPU32 color);

  //Set by $LK,"DCSymmetrySet",A="MN:DCSymmetrySet"$() or $LK,"DCSymmetry3Set",A="MN:DCSymmetry3Set"$()
  CGrSym sym;

  I64	nearest_dist,
	cur_sprite_elem_num,
	nearest_sprite_elem_num; //num in a CSprite elem lst

  I32	cur_x,cur_y,cur_z,pad;
  I64	collision_cnt;

  //Set by $LK,"DCF_RECORD_EXTENTS",A="MN:DCF_RECORD_EXTENTS"$
  I64	min_x,max_x,min_y,max_y; //screen coordinates

  U32	dc_signature,pad;
  CTask	*mem_task,*win_task;
  CDC	*alias;
  U8	*body;

  //Set by $LK,"DCDepthBufAlloc",A="MN:DCDepthBufAlloc"$()
  I32	*depth_buf;
  I64	db_z; //private
  U0 pad;#assert !($$&7) //(compiler lags one token)
};

#help_index "Devices"
//VGA Memory
#define VGAM_GRAPHICS	0xA0000
#define VGAM_TEXT	0xB8000

//VGA I/O ports
#define VGAP_ATTR_INDEX		0x03C0
#define VGAP_ATTR_DATA_WRITE	0x03C0 //alias
#define VGAP_ATTR_DATA_READ	0x03C1
#define VGAP_MISC_OUTPUT	0x03C2
#define VGAP_INDEX		0x03C4
#define VGAP_DATA		0x03C5
#define VGAP_PALETTE_MASK	0x03C6
#define VGAP_REG_READ		0x03C7
#define VGAP_REG_WRITE		0x03C8
#define VGAP_PALETTE_DATA	0x03C9
#define VGAP_CRTC_INDEX		0x03D4
#define VGAP_CRTC_DATA		0x03D5
#define VGAP_INPUT_STAT		0x03DA

//VGA Registers
#define VGAR_MAP_MASK		0x02

#help_index "Compression/Piece by Piece"
#define ARC_MAX_BITS	12

#define CT_NONE 	1
#define CT_7_BIT	2
#define CT_8_BIT	3
class CArcEntry
{
  CArcEntry *next;
  U16	basecode;
  U8	ch,pad;
  U32	pad;
};

public class CArcCtrl
{
  I64	src_pos,src_size,
	dst_pos,dst_size;
  U8	*src_buf,*dst_buf;
  I64	min_bits,min_table_entry;
  CArcEntry *cur_entry,*next_entry;
  I64	cur_bits_in_use,next_bits_in_use;
  U8	*stk_ptr,*stk_base;
  I64	free_index,free_limit,
	saved_basecode,
	entry_used,
	last_ch;
  CArcEntry compress[1<<ARC_MAX_BITS],
	*hash[1<<ARC_MAX_BITS];
};

#help_index "Compression"
public class CArcCompress
{
  I64	compressed_size,expanded_size;
  U8	compression_type;
  U0	body;
};

#define "Char/Input;StdIn"
//GetStr flags
#define GSF_SHIFT_ESC_EXIT	1 //This kills task on <SHIFT-ESC>
#define GSF_WITH_NEW_LINE	2

#define "Char/Operations"
//Flags for StrUtil and MStrUtil
#define SUF_REM_CTRL_CHARS	0x001
#define SUF_REM_LEADING		0x002
#define SUF_REM_TRAILING	0x004
#define SUF_REM_SPACES		0x008
#define SUF_SINGLE_SPACE	0x010
#define SUF_TO_UPPER		0x020
#define SUF_TO_LOWER		0x040
#define SUF_S2T			0x080
#define SUF_T2S			0x100 // Only works with MStrUtil
#define SUF_SCALE_INDENT	0x200
#define SUF_SAFE_DOLLAR		0x400

//Flags for StrFind
#define SFF_IGNORE_CASE		1
#define SFF_WHOLE_LABELS_BEFORE	2
#define SFF_WHOLE_LABELS_AFTER	4
#define SFG_WHOLE_LABELS	(SFF_WHOLE_LABELS_BEFORE|SFF_WHOLE_LABELS_AFTER)

//Flags for LstMatch
#define LMF_IGNORE_CASE		1
#define LMF_EXACT		2

#help_index "Keyboard Devices/System"
#define KDF_HAS_DESCS	1
class CKeyDevEntry
{
  CKeyDevEntry *next,*last;
  I64	priority,flags;
  Bool	(*put_key)(I64 ch,I64 sc);
  Bool	(*put_s)(U8 *st);
};

class CKeyDevGlbls
{
  CKeyDevEntry put_key_root;
  U0	(**fp_ctrl_alt_cbs)(I64 sc);
  I64	ctrl_alt_in_irq_flags,
	**ctrl_alt_ret_addr; //addr of ret addr on stack in kbd irq
  U8	**ctrl_alt_no_shift_descs,**ctrl_alt_shift_descs,
	desc[STR_LEN],
	*handler;
};

#help_index "Snd"
class CSndData
{
  CSndData *next,*last;
  F64	freq, //Hz
	time;
};

public class CSndGlbls
{
  F64	freq;
  CSndData record_root;
  Bool	record;
};

#help_index "Debugging/FunSeg"
#define FUN_SEG_CACHE_SIZE	256
class CFunSegCache
{
  I64	base,limit;
  F64	time_stamp;
  U8	str[1]; //FUN_SEG_CACHE_STR_LEN
$$=64;
};
#define FUN_SEG_CACHE_STR_LEN	(sizeof(CFunSegCache)-offset(CFunSegCache.str))

#help_index "Debugging"
class CMPCrash
{
  I64	cpu_num;
  CTask *task;
  I64	rip;
  U8	*msg;
  I64	msg_num;
};

public class CDbgGlbls
{
  CTask *focus_task;
  U8	*msg;
  I64	msg_num;
  CMPCrash *mp_crash;
  U8	*int_fault_code,
	*fix_file_line;
  CFunSegCache *fun_seg_cache;
  I64	fun_seg_cache_index;
};

#help_index "Boot"
//Boot related
#define BOOT_RAM_BASE		0x07C00
#define BOOT_RAM_LIMIT		0x97000
#define BOOT_STK_SIZE		BLK_SIZE

#define BOOT_SRC_NULL		0
#define BOOT_SRC_RAM		1
#define BOOT_SRC_HARDDRV	2
#define BOOT_SRC_DVD		3

// $LK,"Auto",A="MN:Auto"$("") StdIn for call to $LK,"MakeOSDrvBootInstall",A="MN:MakeOSDrvBootInstall"$().
#define STD_DISTRO_DVD_CFG	"Tb\nScale2Mem(2048,0x40000)\nt \n\n\n\n"

#help_index "Misc/Registry"
//Registry sys_msg_flags. $LK,"RegOneTimePopUp",A="MN:RegOneTimePopUp"$()
#define ARf_FLOODFILL			0
#define ARf_CSPRITE_INS_CLIPBOARD	1
#define ARf_PLANAR_SYMMETRY		2
#define ARf_PSALMODY_JUKEBOX		3
#define ARf_MESH_ED			4
#define ARf_CSPRITE_PTS_RECTANGLES	5
#define ARf_MANAGE_SLIDER		6

#help_index "Misc/Progress Bars"
#define NUM_PROGRESS_BARS	4
#define PROGRESS_DESC_LEN	(64-8-8)
class CProgress
{
  I64	val,max;
  U8	desc[PROGRESS_DESC_LEN];
};

#help_index ""
