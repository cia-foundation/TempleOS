//This is the main TempleOS header file
//for data structures and classes.
//There is no code in this file.

//The internal types are U16i,I32i, etc.
//They are defined $LK,"internal_types_table","MN:internal_types_table"$.
//HolyC has a feature where a
//union structure is treated as a
//whole if no member is specified.
//See $LK-A,"::/TempleOS/Demo/SubIntAccess.CPP"$.
public U16i union U16
{
  I8i i8[2];
  U8i u8[2];
};

public I16i union I16
{
  I8i i8[2];
  U8i u8[2];
};

public U32i union U32
{
  I8i i8[4];
  U8i u8[4];
  I16 i16[2];
  U16 u16[2];
};

public I32i union I32
{
  I8i i8[4];
  U8i u8[4];
  I16 i16[2];
  U16 u16[2];
};

public U64i union U64
{
  I8i i8[8];
  U8i u8[8];
  I16 i16[4];
  U16 u16[4];
  I32 i32[2];
  U32 u32[2];
};

public I64i union I64
{
  I8i i8[8];
  U8i u8[8];
  I16 i16[4];
  U16 u16[4];
  I32 i32[2];
  U32 u32[2];
};

public class Complex
{
  F64 x,y;
};

public class CD2I32  //two dimensional pt
{
  I32 x,y;
};

public class CD3I32  //three dimensional pt
{
  I32 x,y,z;
};

public class CD2I64  //two dimensional pt
{
  I64 x,y;
};

public class CD3I64  //three dimensional pt
{
  I64 x,y,z;
};

public class CD2  //two dimensional pt
{
  F64 x,y;
};

public class CD3  //three dimensional pt
{
  F64 x,y,z;
};

public class CQue
{
  CQue *next,*last;
};

#define NULL	0
#define TRUE	1
#define FALSE	0
#define ON	1
#define OFF	0
#define Bool	I8
#define MAX_U8 0xFF
#define MIN_U8 0
#define MAX_I8 0x7F
#define MIN_I8 (-0x80)
#define MAX_U16 0xFFFF
#define MIN_U16 0
#define MAX_I16 0x7FFF
#define MIN_I16 (-0x8000)
#define MAX_U32 0xFFFFFFFF
#define MIN_U32 0
#define MAX_I32 0x7FFFFFFF
#define MIN_I32 (-0x80000000)
#define MAX_U64 0xFFFFFFFFFFFFFFFF

//(Int to F64 conversion is signed)
#define MAX_U64_F64 (0x43F0000000000000(F64))

#define MIN_U64 0
#define MAX_I64 0x7FFFFFFFFFFFFFFF
#define MIN_I64 (-0x8000000000000000)

#define MAX_F64 (0x7FEFFFFFFFFFFFFF(F64))
#define MIN_F64 (0xFFEFFFFFFFFFFFFF(F64))
#define inf	(0x7FF0000000000000(F64))
#define ì	(0x7FF0000000000000(F64))
#define pi	(0x400921FB54442D18(F64))
#define ã	(0x400921FB54442D18(F64))
#define exp_1	(0x4005BF0A8B145769(F64)) //The number "e"
#define log2_10 (0x400A934F0979A371(F64))
#define log2_e	(0x3FF71547652B82FE(F64))
#define log10_2 (0x3FD34413509F79FF(F64))
#define loge_2	(0x3FE62E42FEFA39EF(F64))
#define sqrt2	(0x3FF6A09E667F3BCD(F64))

#define eps	(0x3CB0000000000000(F64))

#define INVALID_PTR	MAX_I64
#define INVALID_CLUSTER -1

//x86 opcodes
#define OC_OP_SIZE_PREFIX	0x66
#define OC_ADDR_SIZE_PREFIX	0x67
#define OC_LOCK_PREFIX		0xF0
#define OC_NOP			0x90
#define OC_BPT			0xCC
#define OC_CALL			0xE8
#define OC_JMP_REL8		0xEB
#define OC_NOP2			(OC_NOP<<8+OC_OP_SIZE_PREFIX)

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

extern class CTask;
extern class CDoc;
extern class CCPU;
extern class CHashClass;
extern class CHashFun;
extern class CExeImportExport;
extern class CHeapCtrl;
extern class CExeHeapGlbl;
extern class CDC;
extern class CSrvCtrl;
extern class CIntermediateCode;

#define QUE_VECT_U8_CNT		512
class CQueVectU8
{
  CQueVectU8 *next,*last;
  I64 total_cnt,node_cnt,min_index;
  U8 body[QUE_VECT_U8_CNT];
};

public class CFifoU8
{
  U8 *buf;
  I64 mask,in_ptr,out_ptr;
};

public class CFifoI64
{
  I64 *buf;
  I64 mask,in_ptr,out_ptr;
};

#define TF_HH24_MM_SS		0
#define TF_HH24_MM		1
#define TF_HH12_MM_SS_AM	2
#define TF_HH12_MM_AM		3
#define TF_HH12_MM_SSA		4
#define TF_HH12_MMA		5
#define TF_H_M			6
#define TF_H_M_S		7
#define TF_D_H_M		8
#define TF_D_H_M_S		9
#define TF_D_H_M_Sh		10
#define TF_H_M_Sh		11
#define TF_Sh			12
#define TF_M_Sh			13
#define TF_H_M_Sh2		14
#define TF_D_H_M_Sht		15
#define TF_H_M_Sht		16
#define TF_Sht			17
#define TF_M_Sht		18
#define TF_H_M_Sht2		19
#define TF_HH24			20
#define TF_MM			21
#define TF_SS			22
#define TF_NUM			23

#define DF_MM_DD_YY		0
#define DF_MM_DD		1
#define DF_wek_MM_DD		2
#define DF_wek_MM_DD_YY		3
#define DF_MM_DD_YYYY		4
#define DF_wek_MM_DD_YYYY	5
#define DF_mon_DD		6
#define DF_wek_mon_DD		7
#define DF_mon_DD_YYYY		8
#define DF_wek_mon_DD_YYYY	9
#define DF_month_DD		10
#define DF_week_month_DD	11
#define DF_month_DD_YYYY	12
#define DF_week_month_DD_YYYY	13
#define DF_DD_MM_YY		14
#define DF_DD_MM		15
#define DF_wek_DD_MM		16
#define DF_wek_DD_MM_YY		17
#define DF_DD_MM_YYYY		18
#define DF_wek_DD_MM_YYYY	19
#define DF_DD_mon		20
#define DF_wek_DD_mon		21
#define DF_DD_mon_YYYY		22
#define DF_wek_DD_mon_YYYY	23
#define DF_DD_month		24
#define DF_week_DD_month	25
#define DF_DD_month_YYYY	26
#define DF_week_DD_month_YYYY	27
#define DF_YY_MM_DD		28
#define DF_YYYY_MM_DD		29
#define DF_YYMMDD		30
#define DF_YYYYMMDD		31
#define DF_MMDDYY		32
#define DF_MMDDYYYY		33
#define DF_wek			34
#define DF_week			35
#define DF_mon			36
#define DF_month		37
#define DF_YYYY_mon		38
#define DF_YYYY_month		39
#define DF_YY_mon		40
#define DF_YY_month		41
#define DF_mon_YYYY		42
#define DF_month_YYYY		43
#define DF_mon_YY		44
#define DF_month_YY		45
#define DF_YY			46
#define DF_YYYY			47
#define DF_D			48
#define DF_M			49
#define DF_Y			50
#define DF_D2			51
#define DF_M2			52
#define DF_Y2			53
#define DF_D3			54
#define DF_M3			55
#define DF_Y3			56
#define DF_NUM			57

#define CDATE_YEAR_DAYS		365.24225
#define CDATE_YEAR_DAYS_INT	36524225
#define CDATE_BASE_DAY_OF_WEEK	0
public I64 class CDate
{
  U32 time;
  I32 date;
};

public class COrder2D3
{
  F64 x,y,z,DxDt,DyDt,DzDt;
};

#define MSF_INACTIVE		1
#define MSF_FIXED		2
public class CMass
{
  CMass *next,*last;
  COrder2D3 *state,*DstateDt;

  U0 start_saved_area;
  U32 flags,num;
  F64 mass,drag_profile_factor;
  U0 saved_state;
  F64 x,y,z,DxDt,DyDt,DzDt;
  U0 end_saved_area;
};

#define SSF_INACTIVE		1
#define SSF_NO_COMPRESSION	2
#define SSF_NO_TENSION		4
public class CSpring
{
  CSpring *next,*last;
  CMass *end1,*end2;
  F64 f,displacement; //set for you to check

  U0 start_saved_area;
  U32 flags,num,
     end1_num,end2_num;
  F64 const,rest_len;
  U0 end_saved_area;
};

//Ordinary Differential Equations
#define ODEF_HAS_MASSES	1
#define ODEF_PAUSED	2
#define ODEF_STARTED	4

#define ODE_SIGNATURE_VAL	'ODES'
public class CMathODE
{
  CMathODE *next,*last;
  U32 ode_signature,pad;
  I64 flags,n,n_internal;
  CMass   *next_mass,*last_mass;
  CSpring *next_spring,*last_spring;
  F64 drag_v;  //drag proportional to velocity
  F64 drag_v2; //drag proportional to velocity squared
  F64 drag_v3; //drag proportional to velocity cubed
  F64 acceleration_limit; //This clips acceleration
  F64 base_t;
  F64 t,t_scale;
  F64 h,h_min,h_max;

  //This is not precise, just a ballpark
  //TempleOS CMathODE's are for video games
  //not science.  It bails if it takes
  //too long.
  F64 min_tolerance,max_tolerance;

  F64 tolerance_internal;
  F64 *array_base;
  F64 *state;
  F64 *state_internal;
  F64 *DstateDt;
  F64 *state_scale;
  F64 *initial_state;
  F64 *temp0,*temp1,*temp2,*temp3,
	 *temp4,*temp5,*temp6,*temp7;
  CTask *mem_task,*win_task;
  U0 (*derivative)(CMathODE *o,F64 t,F64 *state,F64 *DstateDt);
  I64 user_data;
};

class CBinFile
{
/* See $LK,"Bin File Header Generation","FF:::/TempleOS/Compiler/CmpMain.CPP,TEMPLEOS_XSUM"$ by compiler.

Bin files have this 16-byte header,
followed by code,
followed by loader patch table.

See $LK,"Loader Patch Table Generation","FF:::/TempleOS/Compiler/CmpMain.CPP,EIE_MODULE_ALIGNMENT"$ and $LK,"BinRep","MN:BinRep"$().
*/
  U16 jmp;
  U16 type;
  U32 loader_patch_table_offset;
  U32 file_size;
  U32 xsum;
};

class CBinFilePatchTableWithoutOrg
{
  U8 eie_module_alignment;
  U32 module_alignment;
  U8 zero1;
  U8 eie_abs_addr;
  U32 abs_addres_cnt;
  U8 zero2;
  U32 abs_addres[1];
};

class CBinFilePatchTableWithOrg
{
  U8 eie_module_alignment;
  U32 module_alignment;
  U8 zero1;
  U8 eie_module_org;
  U32 module_org;
  U8 zero2;
  U8 eie_abs_addr;
  U32 abs_addres_cnt;
  U8 zero3;
  U32 abs_addres[1];
};

//IDT entry types
#define IDTET_TASK		0x05
#define IDTET_IRQ		0x0E
#define IDTET_TRAP		0x0F //Same as IRQ but doesnt do CLI.

//Interrupts
//0x00-0x1F are reserved by Intel
#define I_DIVIDE_BY_ZERO	0x00
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
//See $LK,"ST_INT_NAMES","MN:ST_INT_NAMES"$

//You might want to start backward from
//0xFF for your own interrupts.
#define I_USER			0x40


#define MP_MAX_PROCESSORS	64

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

#define SYS_START_RFLAGS	0x0000
#define SYS_NORMAL_RFLAGS	(1<<RFLAGf_INT)

#define NUM_MEM_E820_ENTRIES	48
#define MEM_E820_ENTRY_SIZE	20
class CMemE820
{
  U8 *base;
  I64 len;
  U8 type,pad[3];
};

//Global Descriptor Table
#define GDT_NULL_SEL		0x00
#define GDT_BOOT_DS_SEL 	0x10
#define GDT_BOOT_CS_SEL		0x20
#define GDT_CS_SEL		0x30
#define GDT_CS64_SEL		0x40 //The $LK,"Charter","FI:::/TempleOS/Doc/Charter.TXT"$ says just ring-0.
#define GDT_CS64_RING3_SEL	0x50 //$LK,"Ring3","FI:::/TempleOS/Demo/Lectures/Ring3.CPP"$, in case you want to play around.
#define GDT_ZERO_DS_SEL		0x60
#define GDT_ZERO_DS_RING3_SEL	0x70
#define GDT_CPU_TR_SELS		0x80
#define GDT_CPU_TR_RING3_SELS	(GDT_CPU_TR_SELS      +MP_MAX_PROCESSORS*16)
#define GDT_SIZE		(GDT_CPU_TR_RING3_SELS+MP_MAX_PROCESSORS*16)

class CKernel
{ //Must match $LK,"OSStartUp","FF:::/TempleOS/Kernel/KStart.CPP,SYS_BOOT_BASE"$
  CBinFile h; //16 bytes, 1 paragraph
  U32 jmp;
  U32 boot_base;
  U32 boot_patch_table_base;
  CDate compile_time;
  U32 os_32start;
  U32 boot_code;
  U16 sys_mem_E801[2];
  CMemE820 sys_mem_E820[NUM_MEM_E820_ENTRIES];
  U8  sys_gdttab[GDT_SIZE];
  U16 sys_gr_mode;
  U16 sys_pci_busses;
};

//Run-Levels
#define RLf_16BIT		0
#define RLf_32BIT		1
#define RLf_PATCHED		2
#define RLf_64BIT		3
#define RLf_BOOT_HEAP		4
#define RLf_FULL_HEAP		5
#define RLf_RAW			6
#define RLf_INTERRUPTS		7
#define RLf_MP			8
#define RLf_DSK			9
#define RLf_COMPILER		10
#define RLf_DOC			11
#define RLf_WINMGR		12
#define RLf_LOGIN		13
#define RLf_REGISTRY		14
#define RLf_ADAM2		15
#define RLf_ADAM_SERVER		16

#define RLF_16BIT		0x000001
#define RLF_32BIT		0x000002
#define RLF_PATCHED		0x000004
#define RLF_64BIT		0x000008
#define RLF_BOOT_HEAP		0x000010
#define RLF_FULL_HEAP		0x000020
#define RLF_RAW			0x000040
#define RLF_INTERRUPTS		0x000080
#define RLF_MP			0x000100
#define RLF_DSK			0x000200
#define RLF_COMPILER		0x000400
#define RLF_DOC			0x000800
#define RLF_WINMGR		0x001000
#define RLF_LOGIN		0x002000
#define RLF_REGISTRY		0x004000
#define RLF_ADAM2		0x008000
#define RLF_ADAM_SERVER		0x010000

//Loader flags
#define LDF_NO_ABSS		1
#define LDF_JUST_LOAD		2
#define LDF_SILENT		4

//High Performance Event Timer
#define HPET_GCAP_ID	(0xFED00000+0x00)
#define HPET_GEN_CONF	(0xFED00000+0x10)
#define HPET_MAIN_CNT	(0xFED00000+0xF0)

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

#define LAPICF_APIC_ENABLED		0x100
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
//I/O APIC Registers
#define IOAPICID		0x00
#define IOAPICVER		0x01
#define IOAPICARB		0x02
#define IOREDTAB		0x10

//Model specific registers.
#define IA32F_SCE	0x001
#define IA32F_LME	0x100
#define IA32_LAPIC_BASE	0x01B
#define IA32_EFER	0xC0000080
#define IA32_FS_BASE	0xC0000100
#define IA32_GS_BASE	0xC0000101

//Multi-Processor
class CMPMain
{
  U32 jmp;
  I64 sys_temp_ptr;
};

class CMPCrash
{
  CTask *task;
  I64 cpu_num;
  U8 *msg;
  I64 msg_num;
};


#define JIFFY_FREQ		1000 // Hz
#define CDATE_FREQ		49710 // Hz
#define SYS_TIMER_FREQ		(18333*65536/1000) //Hz

#define SYS_TIMER0_PERIOD	(65536*182/10/JIFFY_FREQ)

#define DFT_CACHE_LINE_WIDTH	64
#define SEMA_STRUCT_SIZE	DFT_CACHE_LINE_WIDTH

//Semaphores
class CSema
{//I read you're supposed to use one cache line.	I don't know why.  Might be silly.
  Bool val;
  U8 pad[SEMA_STRUCT_SIZE-sizeof(Bool)];
};

#define SYS_SEMA_DEBUG			0
#define SYS_SEMA_DETERMINISTIC		1
#define SYS_SEMA_RECORD_MACRO		2
#define SYS_SEMA_SYS_TIMER		3
#define SYS_SEMA_SYS_DATE		4
#define SYS_SEMA_IN_DEBUGGER		5
#define SYS_SEMA_SND			6
#define SYS_SEMA_HEAPLOG_ACTIVE		7
#define SYS_SEMA_HEAPLOG_LOCK		8
#define SYS_SEMA_REFRESH_IN_PROGRESS	9
#define SYS_SEMA_FLUSH_VGA_IMAGE	10
#define SYS_SEMA_SINGLE_USER		11
#define SYS_SEMA_DSK_CACHE		12
#define SYS_SEMA_FAR_CALL32		13
#define SYS_SEMA_DEV_MEM		14
#define SYS_SEMA_VGA			15
#define SYS_SEMA_UPDATE_WIN_Z_BUF	16
#define SYS_SEMA_TT			17
#define SYS_SEMA_MUTE			18
#define SYS_SEMA_FORCE_WINMGR		19
#define SYS_SEMA_TMBEAT			20
#define SYS_SEMA_FIX			21
#define NUM_SYS_SEMAS			22

#define SYSf_CTRL_ALT_DEL		0
#define SYSf_CTRL_ALT_C			1
#define SYSf_CTRL_ALT_X 		2
#define SYSf_CTRL_ALT_TAB		3

//Asm callable function pointers.
//They work with $LK,"CallExtNum","MN:CallExtNum"$() when calling from HolyC.
#define EXT_WIN_TO_TOP		0
#define EXT_WIN_FOCUS		1
#define EXT_HEAPLOG_MALLOC	2
#define EXT_HEAPLOG_FREE	3
#define EXT_RESUME_DBG		4
#define EXT_NUM_ENTRIES		5

//Hash table types
//$LK,"ST_HTT_TYPES","FF:::/TempleOS/Kernel/Define1a.CPP,ST_HTT_TYPES"$
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

#define HTf_PRIVATE		25
#define HTf_PUBLIC		26
#define HTf_EXPORT		27
#define HTf_IMPORT		28
#define HTf_IMM			29
#define HTf_UNRESOLVED		30
#define HTf_LOCAL		31

#define HTT_INVALID		0
#define HTT_EXPORT_SYS_SYM	0x001 //CHashExport
#define HTT_IMPORT_SYS_SYM	0x002 //CHashImport
#define HTT_DEFINE_STR		0x004 //CHashDefineStr
#define HTT_GLBL_VAR		0x008 //CHashGlblVar
#define HTT_CLASS		0x010 //CHashClass
#define HTT_INTERNAL_TYPE	0x020 //CHashClass
#define HTT_FUN			0x040 //CHashFun
#define HTT_WORD		0x080 //CHashWS -- only in WS table
#define HTT_DICT_WORD		0x100 //CHashGeneric -- only in WS table
#define HTT_KEYWORD		0x200 //CHashGeneric
#define HTT_ASM_KEYWORD		0x400 //CHashGeneric
#define HTT_OPCODE		0x800 //CHashOpcode
#define HTT_REG			0x1000 //CHashReg
#define HTT_FILE		0x2000 //CHashGeneric
#define HTT_MODULE		0x4000 //CHashGeneric
#define HTT_HELP_FILE		0x8000 //CHashSrcSym
#define HTT_FRAME_PTR		0x10000 //CHashGeneric
#define HTG_TYPE_MASK		0x1FFFF

#define HTF_PRIVATE		0x02000000
#define HTF_PUBLIC		0x04000000
#define HTF_EXPORT		0x08000000
#define HTF_IMPORT		0x10000000
#define HTF_IMM			0x20000000
#define HTF_UNRESOLVED		0x40000000
#define HTF_LOCAL		0x80000000
#define HTG_FLAGS_MASK		0xFE000000

#define HTG_SRC_SYM		(HTT_DEFINE_STR|HTT_GLBL_VAR|HTT_FUN|HTT_CLASS|HTT_EXPORT_SYS_SYM|HTT_HELP_FILE)
#define HTG_ALL 		-1

#define KERNEL_MODULE_NAME	"/TempleOS/Kernel/Kernel"

class CDbgInfo
{
  U32 min_line,max_line;
  U8 *body[1];
};

public class CHash
{
//Hash entries are not normally ever removed.  Normally, a new
//entry is added to eclipse the old by being in front of it
//in future searches.  Old function code and stuff is not disposed
//in case a reference uses it and old hash entries are kept, too.
  CHash *next;
//Once in a while, the whole hash table is traverse -- $LK,"PSectFind","MN:PSectFind"$(), for example.
//To speed traversal of all symbols, the following links
//allow skipping all the empty slots in a hash table.
  CHash *next_added,*last_added;
  U8 *str;
  U32 type,use_cnt;
//use_cnt is never decremented.  It is incremented
//each time a node is found in a hash search.
//One place it is used is to tell if a externed function was unused
//before being defined.  It is also used for popularity measures
//in wordstat.	You can do a $LK,"Who","MN:Who"$() to see
//roughly how many times a symbol has been
//referenced.	Beware of instances where
//a hash entry is continuously searched for and
//racks-up huge use_cnt numbers.
};

public class CHashSrcSym:CHash
{
  U8 *src_link;
  U8 *index;
  CDbgInfo *debug;
  U8 *import_name;
  CExeImportExport *ie_lst;
};

public class CHashGeneric:CHash
{
  I64 user_data0,user_data1,user_data2;
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
  U8 reg_num,reg_type;
};

public class CHashTable
{
  CHashTable *next;
  CHash *next_added,*last_added;
  I64 mask,locked_flags;
  CHash **body;
};

public class CHashWS:CHash
{
  I32 num;
  U32 hits;
  U16 last_vowel,last_consonant; //Not implemented
};

public class CHashExport:CHashSrcSym
{
  I64 val;
};

public class CHashImport:CHashSrcSym
{
  U8 *module_base;
  U8 *module_header_entry;
};

public class CHashDefineStr:CHashSrcSym
{
  U8 *data,**sub_lst;
  I64 cnt;
  U8 *caller;
};

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
  I64 cnt,total_cnt;
};

#define MLMF_IS_STR	1
public class CMemberLstMeta
{
  CMemberLstMeta *next;
  U8 *str;
  I64 flags;
  I64 user_data;
};

public class CMemberLst
{
  CMemberLst *next,*left,*right,*left2,*right2;
  U8 *str;
  CHashClass *member_class,*member_class2;
  CMemberLstMeta *meta;
  U32 use_cnt;
  U16 flags;
  I8 register,pad;
  I64 offset;
  CArrayDim dim;
  U8 *static_data;
  union {
    I64 static_data_ip;
    I64 dft_val;
  }
  CHashFun *fun_ptr;
};

#define Cf_EXTERN		0
#define Cf_INTERNAL_TYPE	1

public class CHashClass:CHashSrcSym
{ //See $LK,"PrsClassNew","MN:PrsClassNew"$().	baseclass of $LK,"CHashFun","MN:CHashFun"$
  I64 size; //must be at same offset as $LK,"CHashGlblVar","MN:CHashGlblVar"$.size
  U32 member_cnt;
  U8 ptr_cnt,sub_type,flags[2];
  CMemberLst *member_lst,*member_lst2;
  CMemberLst *last_in_member_lst;
  CHashClass *base_class;
  //fun return or ptr to
  //union-no-member-specified class
  union {
    CHashClass *return_class;	//for funs
    CHashClass *fwd_class; //for classes
  }
};

class CExternUsage
{
  CExternUsage *next;
  I64 ip;
};

//Function flags
#define Ff_INTERNAL		8
#define Ff_INTERRUPT		9
#define Ff_HASERRCODE		10
#define Ff_DOT_DOT_DOT		11
#define Ff_RET1			12
#define Ff_ARGPOP		13
#define Ff_NOARGPOP		14

public class CHashFun:CHashClass
{ //See $LK,"PrsFunNew","MN:PrsFunNew"$().
  U32 arg_cnt,pad;
  U32 used_reg_mask,clobbered_reg_mask;
  U8 *executable_addr;
  CExternUsage *ext_lst;
};

//Internal type definitions
#define IT_I0		0
#define IT_U0		1
#define IT_I8		2
#define IT_U8		3
#define IT_I16		4
#define IT_U16		5
#define IT_I32		6
#define IT_U32		7
#define IT_I64		8
#define IT_PTR		8
#define IT_U64		9
#define IT_F64		10
#define IT_UF64		11 //fictitious placeholder
#define IT_NUM_IT	12

#define IT_MASK		0x0F

#define TY_NULL			0x00
#define TY_IMM			0x10
#define TY_REG			0x20
#define TY_DISP			0x30
#define TY_SIB			0x40
#define TY_RIP_DISP32		0x50
#define TY_RIP_DISP32_IMPORT	0x60 //not implemented--requires storage for glblvar struct in intermediate code
#define TY_STK			0x70

#define TY_MASK			0xF0

//Glbl Var Flags
#define GVAF_FUN	1
#define GVAF_IMPORT	2
#define GVAF_EXTERN	4
#define GVAF_DATA_HEAP	8
#define GVAF_ALIAS	16
#define GVAF_ARRAY	32

public class CHashGlblVar:CHashSrcSym
{
  I64 size; //must be at same offset as $LK,"CHashClass","MN:CHashClass"$.size
  I64 flags;
  CHashClass *var_class;
  CHashFun *fun_ptr;
  CArrayDim dim;
  U8 *data_addr;
  union {
    I64 data_addr_ip;
    CExeHeapGlbl *heap_glbl;
  }
};

//Acct Registry sys_msg_flags
#define ARf_FLOODFILL			0
#define ARf_CSPRITE_INS_CLIPBOARD	1
#define ARf_CSPRITE_PICK_GRAPHICALLY	2
#define ARf_PLANAR_SYMMETRY		3
#define ARf_MUSIC_ORGAN_JUKEBOX		4
#define ARf_MESH_ED			5
#define ARf_CSPRITE_PTS_RECTANGLES	6
#define ARf_MANAGE_SLIDER		7

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

#define SVCT_TEXT_INPUT		0
#define SVCT_MSG		1
#define SVCT_EXE_STR		2
//MP cmds
#define SVCT_CALL		3
#define SVCT_SPAWN_TASK		4

class CSrvCmd
{
  CSrvCmd *next,*last;
  CSrvCtrl *ctrl;
  I64 cmd_code;
  I64 flags;

  I64 target_cpu;
  I64 servant_cpu_num;

  I64 msg_code;

  I64 (*add)(U8 *fun_arg);
  U8 *fun_arg;

  U8  *aux_str;
  I64 aux1,aux2;
  I64 result;

  CTask *spawned_task;
  CTask *master_task;
};

#define SVCRf_LOCKED		0
class CSrvCtrl
{
  CSrvCmd *next_waiting,*last_waiting;
  CSrvCmd *next_done,*last_done;
  I64 flags;
};

class CBpt
{
  CBpt *next;
  U8 *addr;
  U8 val,pad[7];
};

//See $LK,"ExtScreenCodes","FA:::/TempleOS/Adam/Gr/GrScreen.CPP,ExtScreenCodes"$
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
#define DOCT_DATA		7
#define DOCT_PAGE_LEN		8
#define DOCT_LEFT_MARGIN	9
#define DOCT_RIGHT_MARGIN	10
#define DOCT_HEADER		11
#define DOCT_FOOTER		12
#define DOCT_INDENT		13
#define DOCT_FOREGROUND		14
#define DOCT_BACKGROUND		15
#define DOCT_DFT_FOREGROUND	16
#define DOCT_DFT_BACKGROUND	17
#define DOCT_PMT		18
#define DOCT_WORD_WRAP		19
#define DOCT_UNDERLINE		20
#define DOCT_INVERT		21
#define DOCT_BLINK		22
#define DOCT_SHIFTED_X		23
#define DOCT_SHIFTED_Y		24
#define DOCT_CURSOR_MOVEMENT	25
#define DOCT_ANCHOR		26
#define DOCT_LINK		27
#define DOCT_BUTTON		28
#define DOCT_CHECK_BOX		29
#define DOCT_MACRO		30
#define DOCT_MENU_VAL		31
#define DOCT_HEX_ED		32
#define DOCT_TREE		33
#define DOCT_SPRITE		34
#define DOCT_INS_BINARY		35
#define DOCT_INS_BINARY_TYPE	36
#define DOCT_INS_BINARY_SIZE	37
#define DOCT_BPT		38
#define DOCT_SONG		39
#define DOCT_HIGHLIGHT		40
#define DOCT_LOCK_TO_PAGE	41
#define DOCT_LOCK_TO_CENTER	42
#define DOCT_HTML_IMG		43
#define DOCT_HTML_VIDEO		44
#define DOCT_HTML_CODE		45
#define DOCT_ERROR		46

#define DOCT_NUM_TYPES		47

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

#define DOCG_DBL_BUF_FLAGS	(DOCF_OVERSTRIKE|DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS| \
				DOCF_MIN_SIZE|DOCF_NO_CURSOR|DOCF_FORM| \
				DOCF_DBL_DOLLARS|DOCF_DONT_SWAP_OUT| \
				DOCF_DO_FULL_REFRESH|DOCF_BORDER_DOC|DOCF_HIDE_CURSOR|DOCF_CARRIAGE_RETURN)

//CDocEntry.flag flags
#define DOCEF1_AUX_STR		1
#define DOCEF1_LINK		2
#define DOCEF1_LEFT_CB		4
#define DOCEF1_LEFT_EXP		8
#define DOCEF1_LEFT_MACRO	0x10
#define DOCEF1_LEFT_AUTO	0x20
#define DOCEF1_RIGHT_CB		0x40
#define DOCEF1_RIGHT_EXP	0x80
#define DOCEF1_RIGHT_MACRO	0x100
#define DOCEF1_RIGHT_AUTO	0x200
#define DOCEF1_TAG_CB		0x400
#define DOCEF1_HAS_BIN		0x800
#define DOCEF1_ESC		0x1000 //Send <ESC> (Exit and Save)
#define DOCEF1_QUIT		0x2000 //Send <SHIFT-ESC> (Abort)
#define DOCEF1_LEFT_X		0x4000
#define DOCEF1_CENTER_X		0x8000
#define DOCEF1_RIGHT_X		0x10000
#define DOCEF1_TOP_Y		0x20000
#define DOCEF1_CENTER_Y		0x40000
#define DOCEF1_BOTTOM_Y		0x80000
#define DOCEF1_SHIFTED_X	0x100000
#define DOCEF1_SHIFTED_Y	0x200000
#define DOCEF1_SCROLLING_X	0x400000
#define DOCEF1_SCROLLING_Y	0x800000
//HL...UL
#define DOCEF1_HAS_BORDER	0x100000000
#define DOCEF1_SOLID_BORDER	0x200000000
#define DOCEF1_BORDER_PLOT	0x400000000
#define DOCEF1_CHECKED_COLLAPSED 0x800000000 //checked or collapsed
#define DOCEF1_CHECK_COLLAPSABLE 0x1000000000
#define DOCEF1_REFRESH_DATA	0x2000000000
#define DOCEF1_DATA_IS_PTR	0x4000000000
#define DOCEF1_HAS_TERMINATOR	0x8000000000
#define DOCEF1_ZERO_BASED	0x10000000000
#define DOCEF1_HOLD		0x20000000000
#define DOCEF1_TREE		0x40000000000
#define DOCEF1_SKIP		0x80000000000
#define DOCEF1_ALIAS		0x100000000000
#define DOCEF1_POPUP		0x200000000000
#define DOCEF1_PAGE_REL_Y	0x400000000000
#define DOCEF1_MARGIN_REL_X	0x800000000000
#define DOCEF1_FROM_START	0x1000000000000
#define DOCEF1_SKIP_IN_FORM	0x2000000000000
#define DOCEF1_WIDTH		0x4000000000000
#define DOCEF1_HEIGHT		0x8000000000000
#define DOCEF1_HARD_SKIP	0x10000000000000
#define DOCEF1_FILTER_SKIP	0x20000000000000
#define DOCEF1_NO_CLICK_ON	0x40000000000000
#define DOCEF1_DONT_DRAW	0x80000000000000 //only works on sprites
#define DOCEF1_UPDATE_DATA	0x100000000000000
#define DOCEF1_U8		0x200000000000000
#define DOCEF1_U16		0x400000000000000
#define DOCEF1_U32		0x800000000000000
#define DOCEF1_U64		0x1000000000000000
#define DOCEF1_DEFINE		0x2000000000000000
#define DOCEF1_WIN_REL		0x4000000000000000
#define DOCEF1_HTML_LINK	0x8000000000000000

#define DOCEF2_BIN_PTR_LINK	0x1

#define DOCEG1_DONT_EDIT	(DOCEF1_TAG_CB|DOCEF1_DEFINE|DOCEF1_HTML_LINK|DOCEF1_SCROLLING_X)
#define DOCEG2_DONT_EDIT	DOCEF2_BIN_PTR_LINK

//These are identical to Doc flags
#define DOCEF1_HIGHLIGHT	0x1000000
#define DOCEF1_LOCK_TO_PAGE	0x2000000
#define DOCEF1_LOCK_TO_CENTER	0x4000000
#define DOCEF1_WORD_WRAP	0x8000000
#define DOCEF1_BLINK		ATTRF_BLINK
#define DOCEF1_INVERT		ATTRF_INVERT
#define DOCEF1_SELECTED		ATTRF_SELECTED
#define DOCEF1_UNDERLINE	ATTRF_UNDERLINE
#define DOCEf1_HIGHLIGHT	24
#define DOCEf1_LOCK_TO_PAGE	25
#define DOCEf1_LOCK_TO_CENTER	26
#define DOCEf1_WORD_WRAP	27
#define DOCEf1_BLINK		ATTRf_BLINK
#define DOCEf1_INVERT		ATTRf_INVERT
#define DOCEf1_SELECTED		ATTRf_SELECTED
#define DOCEf1_UNDERLINE	ATTRf_UNDERLINE

#define DOCEf1_AUX_STR		0
#define DOCEf1_LINK		1
#define DOCEf1_LEFT_CB		2
#define DOCEf1_LEFT_EXP		3
#define DOCEf1_LEFT_MACRO	4
#define DOCEf1_LEFT_AUTO	5
#define DOCEf1_RIGHT_CB		6
#define DOCEf1_RIGHT_EXP	7
#define DOCEf1_RIGHT_MACRO	8
#define DOCEf1_RIGHT_AUTO	9
#define DOCEf1_TAG_CB		10
#define DOCEf1_HAS_BIN		11
#define DOCEf1_ESC		12 //Send <ESC> (Exit and Save)
#define DOCEf1_QUIT		13 //Send <SHIFT-ESC> (Abort)
#define DOCEf1_LEFT_X		14
#define DOCEf1_CENTER_X		15
#define DOCEf1_RIGHT_X		16
#define DOCEf1_TOP_Y		17
#define DOCEf1_CENTER_Y		18
#define DOCEf1_BOTTOM_Y		19
#define DOCEf1_SHIFTED_X	20
#define DOCEf1_SHIFTED_Y	21
#define DOCEf1_SCROLLING_X	22
#define DOCEf1_SCROLLING_Y	23
//HL...UL
#define DOCEf1_HAS_BORDER	32
#define DOCEf1_SOLID_BORDER	33
#define DOCEf1_BORDER_PLOT	34
#define DOCEf1_CHECKED_COLLAPSED 35
#define DOCEf1_CHECK_COLLAPSABLE 36 //checked or collapsed
#define DOCEf1_REFRESH_DATA	37
#define DOCEf1_DATA_IS_PTR	38
#define DOCEf1_HAS_TERMINATOR	39
#define DOCEf1_ZERO_BASED	40
#define DOCEf1_HOLD		41
#define DOCEf1_TREE		42
#define DOCEf1_SKIP		43
#define DOCEf1_ALIAS		44
#define DOCEf1_POPUP		45
#define DOCEf1_PAGE_REL_Y	46
#define DOCEf1_MARGIN_REL_X	47
#define DOCEf1_FROM_START	48
#define DOCEf1_SKIP_IN_FORM	49
#define DOCEf1_WIDTH		50
#define DOCEf1_HEIGHT		51
#define DOCEf1_HARD_SKIP	52
#define DOCEf1_FILTER_SKIP	53
#define DOCEf1_NO_CLICK_ON	54
#define DOCEf1_DONT_DRAW	55 //only works on sprites
#define DOCEf1_UPDATE_DATA	56
#define DOCEf1_U8		57
#define DOCEf1_U16		58
#define DOCEf1_U32		59
#define DOCEf1_U64		60
#define DOCEf1_DEFINE		61
#define DOCEf1_WIN_REL		62
#define DOCEf1_HTML_LINK	63
#define DOCEf1_NUM_FLAGS	64

#define DOCEf2_BIN_PTR_LINK	0
#define DOCEf2_NUM_FLAGS	1

#define DOCBT_GENERIC_DATA	1
#define DOCBT_CSPRITE		2
#define DOCBT_DEBUG_LINES	3

public class CDocBin
{
  CDocBin *next,*last;
  I32 temp_use_cnt,renum_num;
  U8 *tag;
  U0 start;
  U32 type,num,flags,size,use_cnt;
  U8 *data;
};

#define DOC_SCROLL_SPEED	8

#define DOCSS_NORMAL		0
#define DOCSS_SINGLE_QUOTE	1
#define DOCSS_DBL_QUOTE		2
#define DOCSS_COMMENT		3
#define DOCSS_CPP_Z_COMMENT	4

#define DOCSSF_BPT		8

#define TERM_ATTR_TEXT		WHITE<<4+BLUE
#define TERM_ATTR_BORDER	WHITE<<4+LTBLUE
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
  CDocSettings *next,*last;
  U0 start;
  I16 left_margin,right_margin,indent;
  U16 page_len,header,footer;
  I8 shifted_x,shifted_y;
  U8 state,comment_depth,paren_depth,brace_depth;
  U8 cur_text_attr,dft_text_attr;
};

#define DOC_DFT 		MIN_I32

public class CDocEntryBase
{
//This is a shortened structure for
//cmds like the text cmd which
//don't require the full CDocEntry structure.
  CDocEntryBase *next,*last;
  U8 *tag;
  union {
    U8 type_u8; //this stores the code
    U32 type; //these store attr flags
  };
  I32 page_line_num;
  I64 flags1,flags2;
  I32 x,y;
  U32 min_col,max_col;

  //These are here to allow recalculating the Doc
  //from a pt other than the beginning.  Without
  //these, you do not know the current state.
  //Margins will change to allow multiple columns
  //of text on the same page.  (cursor move back up the page.)
  CDocSettings *settings;

  I64 user_data;
};

public class CDocEntry:CDocEntryBase
{
  union {
    I64 attr;
    I64 cursor_x_offset;
    I64 (*left_cb)(CDoc *doc,CDocEntry *doc_e);
    I64 left_exp;
  };
  U8 *left_macro;

  union {
    I64 cursor_y_offset;
    I64 (*right_cb)(CDoc *doc,CDocEntry *doc_e);
    I64 right_exp;
  };
  U8 *right_macro;

  U8 *(*tag_cb)(CDoc *doc,CDocEntry *doc_e,
		   CTask *mem_task);
  U8 *define_str;
  U8 *aux_str;
  U8 *bin_ptr_link;
  U8 *html_link;
  U8 *my_fmt_data;
  I64 hex_ed_width;
  union {
    I32 scroll_len;
    I32 width;
  }
  union {
    I32 len;
    I32 height;
  }
  I32 bin_num;
  I32 pad;
  CDocBin *bin_data;

  U8 *data;
};

//$LK,"DocFormDo","MN:DocFormDo"$() $LK,"DocMenuDo","MN:DocMenuDo"$() $LK,"DocDo","MN:DocDo"$() $LK,"PopUpMenu","MN:PopUpMenu"$()
#define	DOF_MIN_SIZE		1
#define	DOF_INTERCEPT_TASK_END	2
#define DOF_DONT_HOME		4
#define DOF_WIN_MAX		8
#define DOF_DONT_TEXT_ATTR	16

class CEdFindText
{
  U8	find_text[136]		fmtstr "$$DA-P,135,\"Find        :%s\"$$\n";
  U8	replace_text[136]	fmtstr "$$DA-P,135,\"Replace     :%s\"$$\n";
  Bool replace			fmtstr "$$CB,\"Replace\"$$\n";
  Bool scan_fwd			fmtstr "$$CB,\"Fwd\"$$\n";
  Bool scan_selected_text	fmtstr "$$CB,\"Selection\"$$\n";
  Bool match_case		fmtstr "$$CB,\"Match Case\"$$\n";
  Bool whole_labels		fmtstr "$$CB,\"Whole Labels\"$$\n";
  Bool local_var		fmtstr "$$CB,\"Rename Local Var\"$$\n";
  Bool pmt;
  U8	 pad;
  I64	 filter_lines		fmtstr "$$DA,131,  \"Filter Lines:%d\"$$\n";
};

class CEdFileName
{
  U8 name[256] fmtstr "$$DA-P,255,\"File Name:%s\"$$";
};

//$LK,"Ed","MN:Ed"$()
#define EDF_BAIL		1
#define EDF_COLLAPSE		2
#define EDF_UNCOLLAPSE		4

#define EDf_BAIL		0
#define EDf_COLLAPSE		1
#define EDf_UNCOLLAPSE		2

// DOC header flags
#define DOCF_OVERSTRIKE 	0x1
#define DOCF_PLAIN_TEXT 	0x2
#define DOCF_PLAIN_TEXT_WITH_TABS 0x4 //has '\t', not DOCT_TAB
#define DOCF_MIN_SIZE		0x8
#define DOCF_NO_CURSOR		0x10 // Makes DocInsStr remove
#define DOCF_IN_DOLLAR		0x20
#define DOCF_MORE		0x40
#define DOCF_FORM		0x80
#define DOCF_DBL_DOLLARS	0x100
#define DOCF_NULL_GRAB_SCROLL	0x200
#define DOCF_BACKWARD_MOVEMENT	0x400
#define DOCF_HAS_SONG		0x800
#define DOCF_UNDO_DIRTY		0x1000
#define DOCF_ALLOW_UNDO		0x2000
#define DOCF_SUPERSCRIPT_MODE	0x4000
#define DOCF_SUBSCRIPT_MODE	0x8000
#define DOCF_DONT_SWAP_OUT	0x10000
#define DOCF_DONT_SHOW		0x20000
#define DOCF_DO_FULL_REFRESH	0x40000
#define DOCF_BREAK_UNLOCKED	0x80000
#define DOCF_BORDER_DOC		0x100000
#define DOCF_HIDE_CURSOR	0x200000 //use $LK,"DocCursor","MN:DocCursor"$
#define DOCF_CARRIAGE_RETURN	0x400000
#define DOCF_NO_SCROLL_BARS	0x800000 //use $LK,"DocScroll","MN:DocScroll"$

#define DOCF_HIGHLIGHT		DOCEF1_HIGHLIGHT
#define DOCF_LOCK_TO_PAGE	DOCEF1_LOCK_TO_PAGE
#define DOCF_LOCK_TO_CENTER	DOCEF1_LOCK_TO_CENTER
#define DOCF_WORD_WRAP		DOCEF1_WORD_WRAP
#define DOCF_BLINK		DOCEF1_BLINK
#define DOCF_INVERT		DOCEF1_INVERT
#define DOCF_SELECTED		DOCEF1_SELECTED
#define DOCF_UNDERLINE		DOCEF1_UNDERLINE

#define DOCf_OVERSTRIKE 	0
#define DOCf_PLAIN_TEXT 	1
#define DOCf_PLAIN_TEXT_WITH_TABS 2 //has '\t', not DOCT_TAB
#define DOCf_MIN_SIZE		3
#define DOCf_NO_CURSOR		4 // Makes DocInsStr remove
#define DOCf_IN_DOLLAR		5
#define DOCf_MORE		6
#define DOCf_FORM		7
#define DOCf_DBL_DOLLARS	8
#define DOCf_NULL_GRAB_SCROLL	9
#define DOCf_BACKWARD_MOVEMENT	10
#define DOCf_HAS_SONG		11
#define DOCf_UNDO_DIRTY		12
#define DOCf_ALLOW_UNDO		13
#define DOCf_SUPERSCRIPT_MODE	14
#define DOCf_SUBSCRIPT_MODE	15
#define DOCf_DONT_SWAP_OUT	16
#define DOCf_DONT_SHOW		17
#define DOCf_DO_FULL_REFRESH	18
#define DOCf_BREAK_UNLOCKED	19
#define DOCf_BORDER_DOC		20
#define DOCf_HIDE_CURSOR	21  //use $LK,"DocCursor","MN:DocCursor"$
#define DOCf_CARRIAGE_RETURN	22
#define DOCf_NO_SCROLL_BARS	23

#define DOCf_HIGHLIGHT		DOCEf1_HIGHLIGHT
#define DOCf_LOCK_TO_PAGE	DOCEf1_LOCK_TO_PAGE
#define DOCf_LOCK_TO_CENTER	DOCEf1_LOCK_TO_CENTER
#define DOCf_WORD_WRAP		DOCEf1_WORD_WRAP
#define DOCf_BLINK		DOCEf1_BLINK
#define DOCf_INVERT		DOCEf1_INVERT
#define DOCf_SELECTED		DOCEf1_SELECTED
#define DOCf_UNDERLINE		DOCEf1_UNDERLINE


//locked flags
#define DOClf_LOCKED		0
class CDocUndo
{
  CDocUndo *next,*last;
  I64 size;
  I64 doc_flags;
  I64 time_stamp;
  U8 *body;
};

//See $LK,"DocMenuDo","MN:DocMenuDo"$()
#define DOCM_EXIT		0
#define DOCM_CANCEL		(-1)

#define DOC_SIGNATURE_VAL		'DocS'

#define DOC_RECALC_ALL		0
#define DOC_RECALC_PARTIAL	1
#define DOC_RECALC_FIND_CURSOR	2
#define DOC_RECALC_TO_SCREEN	3

public class CDoc //Linked Text File header
{  //See $LK,"Doc","HI:Doc"$ for documentation.
  CDocEntryBase root;
  U8 *dummy_display;
  I64 flags,locked_flags;
  CDocEntry *cur_entry,*old_cur_entry;
  I32 cur_data_col,old_cur_data_col,
     line_start_col,cur_top_line_num;
  I64 swap_out_time;
  I32 ext_ascii_val,ext_ascii_cnt;
  I32 dollar_buf_size,dollar_buf_ptr;
  U8 *dollar_buf;
  CTask *win_task,*mem_task;
  CDocEntry *recalc_start;
  I32 page_line_num,undo_cnt;
  I32 x,y,min_x,max_x,min_y,max_y;
  I64 line,col,best_d;
  I64 old_win_top,old_win_bottom,
     old_win_left,old_win_right;

  I64 cmd_U8;

  U32 doc_signature;
  I32 max_entries,ww_lines_back,cur_bin_num;

  I64 refresh_cnt;
  CTask *owning_task;
  CEdFindText *find_replace;
  I32 start_x,start_y;

  U8 *cur_sprite;

//not document num, but num in a CSprite collection
  I64 cur_sprite_elem_num;

  CEdFileName filename;
  I64 file_attr;
  I64 (*left_click_link)(CDoc *doc,CDocEntry *doc_e);
  I64 (*right_click_link)(CDoc *doc,CDocEntry *doc_e);

  U8 *user_put_data;
  Bool (*user_put_key)(CDoc *doc,U8 *data,I64 ch,I64 sc);
  Bool (*user_put_s)(CDoc *doc,U8 *data,U8 *st);

  CDoc *parent_doc;
  U8 desc[8]; //Be careful -- only 7 characters and terminator

  CDocBin  bin_root;	      //size & 7 ==4
  CDocSettings settings_root; //size & 7 ==4
  CDocUndo undo_root;

  I64 user_data;
U0 pad;#assert !($$&7) //(compiler lags one token)
};

//MGetS flags
#define GETSF_SHIFT_ESC_EXIT	1 //This kills task on <SHIFT-ESC>
#define GETSF_WITH_NEW_LINE	2

//Compiler Options
//  Use $LK,"OptOn","MN:OptOn"$() $LK,"OptOff","MN:OptOff"$()

//You might need to do #exe {OptOn();}
//Note: The #exe statement is lexed-ahead, so it takes effect earlier than you might expect.
#define OPTf_ECHO		0x00
#define OPTf_LEX_TRACE		0x01
#define OPTf_PARSE_TRACE	0x02
#define OPTf_OPT_TRACE		0x03
#define OPTf_COMPILE_TRACE	0x04
#define OPTf_WARN_UNUSED_VAR	0x10 //Applied to funs, not statements
#define OPTf_WARN_PAREN		0x11 //Warn unnecessary parens
#define OPTf_WARN_DUP_TYPES	0x12 //Warn dup local var type statements
#define OPTf_EXTERNS_TO_IMPORTS	0x20
#define OPTf_KEEP_PRIVATE	0x21
#define OPTf_NO_REG_VAR		0x22  //Applied to funs, not statements
#define OPTf_REPLACE_SYMS	0x23
#define OPTf_GLBLS_ON_DATA_HEAP	0x24
//Disable 10-byte float consts for ã,log2_10,log10_2,loge_2
#define OPTf_NO_BUILTIN_CONST	0x25 //Applied to funs, not statements
#define OPTf_USE_IMM64		0x26 //Not completely implemented

#define OPTF_ECHO		(1<<OPTf_ECHO)
#define OPTF_PARSE_TRACE	(1<<OPTf_PARSE_TRACE)

#define LFSF_IS_INCLUDED	1
#define LFSF_IS_DOC		2
#define LFSF_IS_DEFINE		4

#define LFSf_IS_INCLUDED	0
#define LFSf_IS_DOC		1
#define LFSf_IS_DEFINE		2

class CLexFile
{
  U8 *buf;
  U8 *buf_ptr;
  I64 line_num,flags;
  U8 *name;
  U8 *line_start;
  CDoc *doc;
  CDocEntry *cur_entry;
  U8 last_U16,pad[7];
};

class CLexUndef
{
  CLexUndef *next;
  CHashExport *hash;
};

#define ICF_R_TO_F64			0x000000001
#define ICF_R_TO_INT			0x000000002
#define ICF_P1_TO_F64			0x000000004
#define ICF_P1_TO_INT			0x000000008
#define ICF_P2_TO_F64			0x000000010
#define ICF_P2_TO_INT			0x000000020
#define ICF_USE_F64			0x000000040
#define ICF_USE_UNSIGNED		0x000000080
#define ICF_USE_INT			0x000000100 //highest priority
#define ICF_NO_DEPEND_RESULT		0x000000200
#define ICF_CODE_FINAL			0x000000400
#define ICF_NOT_ADDR			0x000000800
#define ICF_SHORT_JMP			0x000001000
#define ICF_P1_FIRST			0x000002000
#define ICF_PUSH_RESULT			0x000004000
#define ICF_TRACE			0x000008000
#define ICF_R_WAS_STK			0x000010000
#define ICF_P1_WAS_STK			0x000020000
#define ICF_P2_WAS_STK			0x000040000
#define ICF_PUSH_CMP			0x000080000 //for 50<i<j<=100 expressions
#define ICF_POP_CMP			0x000100000 //for 50<i<j<=100 expressions
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

#define IC_BODY_SIZE			48

#define ECF_HAS_PUSH_CMP		0x01 //for 50<i<j<=100 expressions


class CICArg
{
  U8 type,ptype;
  U16 reg; //low is reg, high is index_reg+scale<<6
  I64 disp;
};

class CICTreeLinks
{
  CHashClass *p1c,*p2c;
  CIntermediateCode *p1t,*p2t;
  CHashClass *class2;
};

class CIntermediateCodeBase
{
  CIntermediateCode *next,*last;
  U16 ic_opcode;
  U16 ic_precedence;
  I16 ic_cnt;
  I16 ic_last_cnt;
};

class CIntermediateCode:CIntermediateCodeBase
{
  I64 ic_flags;
  I64 ic_data;

  CHashClass *ic_class;
  CHashClass *ic_class2;

  U32 ic_line;
  CICArg p1,p2,r;

  U8 *ic_ext_body;
  union {
    U8 ic_body[IC_BODY_SIZE];

    //Tree Links are created in OptPass012.  An ADD opcode, for exmaple,
    //points back to its two earlier addend instructions -- not MAlloced,
    //just pointers to previous CIntermediate codes in the array.

    //Tree links get destroyed during Pass789 when they get overwrites
    //by machine code.	(Saves room to union the output machine code buffer
    //with these links, since they are not needed after pass4.

    //Tree links are mostly used during 012 and 3 for determining types.
    CICTreeLinks t;
  };
U0 pad;#assert !($$&7) //(compiler lags one token)
};

class CPrsStk
{
  I64 ptr;
  I64 stk[255];
  I64 ptr2;
  I64 stk2[255];
};

#define CMT_LABEL		0
#define CMT_GOTO_LABEL		1
#define CMT_STR_CONST		2
#define CMT_JMP_TABLE		3
#define CMT_FLOAT_CONSTS	4
#define CMT_ARRAY_DIM		5
#define CMT_HASH_ENTRY		6

#define CMF_USED		1
#define CMF_POP_CMP		2
#define CMF_DEFINED		4

#define CM_MAX_CONSTS		16

class CLexCodeMisc
{
  CLexCodeMisc *next,*last,*fwd,*dft;
  U8 *str;
  U32 type,flags;
  U8 *addr;
  union {
    I64 st_len;
    I64 num_consts;
    I64 range;
  }
  union {
    U8 **jmp_table;
    F64 *float_consts;
    CArrayDim *dim;
    CHash *h;
  };
};

#define IEF_OP_SIZE16		0x001
#define IEF_OP_SIZE32		0x002
#define IEF_DONT_SWITCH_MODES	0x004
#define IEF_PLUS_OPCODE		0x008
#define IEF_DFT			0x010
#define IEF_NO_REX		0x020
#define IEF_40_REX		0x040
#define IEF_48_REX		0x080
#define IEF_REX_ONLY_R8_R15	0x100
#define IEF_REX_XOR_LIKE	0x200
#define IEF_STI			0x400
#define IEF_ENDING_ZERO		0x800

#define SV_R_REGISTER	8
#define SV_I_REGISTER	9
#define SV_NONE		10
class CInstruction
{
  U8 ins_entry_num; //This entry num in opcode hash entry
  U8 opcode_cnt;
  U8 opcode[4];
  U16 flags;
  U8 slash_val,opcode_modifier;
  U8 arg1,arg2;
  U32 pad;
};

class CHashOpcode:CHash
{
  U8 instruction_entry_cnt;
  U8 pad[7];
  CInstruction ins[1];
};

#define ASM_CODE_BLK_BITS 16
#define ASM_CODE_BLK_SIZE (1<<ASM_CODE_BLK_BITS)

class CAsmCodeBlk
{
  CAsmCodeBlk *next;
  U8 body[ASM_CODE_BLK_SIZE];
};

I64 class CAbsCntsI64
{
  U16 abs_addres;
  U16 c_addres;
  U16 externs,pad;
};

class CAsmNum
{
  I64 i;
  U8 *ex;
  CLexUndef *undef_local,*undef_glbl;
  CAbsCntsI64 abs_cnts;
};

class CAsmNum2
{
  CAsmNum num;
  I64 U8_cnt,rel;
  Bool imm_flag;
};

class CAsmIns
{
  CInstruction *tempi;
  CAsmNum2 imm,disp;
  I64 U8_cnt,last_opcode_U8;
  I64 REX,ModrM,SIB;
  Bool has_REX,has_ModrM,has_SIB;
  Bool has_addr_prefix;
  Bool has_operand_prefix;
  Bool is_dft,pad[2];
};

class CAsmArg
{
  CAsmNum num;
  I64 seg,size,reg1,reg2,fstk;
  I64 scale;
  Bool indirect,imm_or_off_present,just_seg,pad[5];
};

class CAsmUnresolvedRef
{
  CAsmUnresolvedRef *next;
  I64 type,line_num;
  U8 *ex;
  I64 ip,rel_ip;
  U8 *str;  //Only for import glbls
  CLexUndef *undef_hash;
  Bool U8_avail;
  Bool imm_flag;  //Only for import glbls
};


#define EIE_END			0
//reserved
#define EIE_REL_EXPORT		2
#define EIE_IMM_EXPORT		3
#define EIE_REL_U8		4
#define EIE_IMM_U8		5
#define EIE_REL_U16		6
#define EIE_IMM_U16		7
#define EIE_REL_U32		8
#define EIE_IMM_U32		9
#define EIE_REL_U64		10
#define EIE_IMM_U64		11
//reserved
#define EIE_MAIN		16
#define EIE_MODULE_ALIGNMENT	17
#define EIE_MODULE_ORG		18
//reserved
#define EIE_ABS_ADDR		32
#define EIE_HEAP_GLBL		33
//See $LK,"::/TempleOS/Adam/Utils/BinRep.CPP","FL:::/TempleOS/Adam/Utils/BinRep.CPP"$

class CExeImportExport
{
  CExeImportExport *next,*last;
  I64 ip;
  U8 *str;
  U8 *src_link;
  U8 type;
};

#define EAA_ADD_U8		0
#define EAA_SUB_U8		1
#define EAA_ADD_U16		2
#define EAA_SUB_U16		3
#define EAA_ADD_U32		4
#define EAA_SUB_U32		5
#define EAA_ADD_U64		6
#define EAA_SUB_U64		7
class CExeAbsAddr
{
  CExeAbsAddr *next;
  I64 ip;
  U8 type,pad[7];
};

class CExeHeapGlblRef
{
  CExeHeapGlblRef *next;
  I64 ip;
};

class CExeHeapGlbl
{
  CExeHeapGlbl *next;
  U8 *str;
  I64 size;
  CExeHeapGlblRef *references;
};

class CExe
{
  U8 *ex;
  I64 ex_U8s;
  I64 max_align,module_org;
  CExe *parent_exe;
  CExeImportExport *next_ie,*last_ie;
  CExeAbsAddr *abss;
  CExeHeapGlbl *heap_glbls;
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

#define ARGT_UIMM8	4
#define ARGT_UIMM16	5
#define ARGT_UIMM32	6
#define ARGT_UIMM64	7

#define ARGT_R8		8
#define ARGT_R16	9
#define ARGT_R32	10
#define ARGT_R64	11

#define ARGT_IMM8	12
#define ARGT_IMM16	13
#define ARGT_IMM32	14
#define ARGT_IMM64	15

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

//Tokens
#define TK_EOF			0
#define TK_SUPERSCRIPT		0x001
#define TK_SUBSCRIPT		0x002
#define TK_NORMALSCRIPT		0x003
#define TK_IDENT		0x100
#define TK_STR			0x101
#define TK_INTEGER		0x102
#define TK_CHAR_CONST		0x103
#define TK_F64			0x104
#define TK_PLUS_PLUS		0x105
#define TK_MINUS_MINUS		0x106
#define TK_DEREFERENCE		0x107
#define TK_DBL_COLON		0x108
#define TK_SHL			0x109
#define TK_SHR			0x10A
#define TK_EQUAL_EQUAL		0x10B
#define TK_NOT_EQUAL		0x10C
#define TK_LESS_EQUAL		0x10D
#define TK_GREATER_EQUAL	0x10E
#define TK_AND_AND		0x10F
#define TK_OR_OR		0x110
#define TK_XOR_XOR		0x111
#define TK_SHL_EQUAL		0x112
#define TK_SHR_EQUAL		0x113
#define TK_MUL_EQUAL		0x114
#define TK_DIV_EQUAL		0x115
#define TK_AND_EQUAL		0x116
#define TK_OR_EQUAL		0x117
#define TK_XOR_EQUAL		0x118
#define TK_ADD_EQUAL		0x119
#define TK_SUB_EQUAL		0x11A
#define TK_IF			0x11B
#define TK_IFDEF		0x11C
#define TK_IFNDEF		0x11D
#define TK_IFAOT		0x11E
#define TK_IFJIT		0x11F
#define TK_ENDIF		0x120
#define TK_ELSE			0x121
#define TK_MOD_EQUAL		0x122
#define TK_DOT_DOT		0x123
#define TK_DOT_DOT_DOT		0x124
#define TK_INS_BINARY		0x125
#define TK_INS_BINARY_TYPE	0x126
#define TK_INS_BINARY_SIZE	0x127
#define TK_NUM_TK		0x128

//Lex flags
#define LF_PMT			1
#define Lf_PMT			0
#define LF_DONT_FREE_BUF	2
#define LF_NO_DEFINES		4
#define LF_IN_IF		8
#define LF_JUST_LOAD		16
#define LF_KEEP_NEW_LINES	32
#define LF_KEEP_DOT		64
#define LF_KEEP_NUM_SIGN	128
#define LF_KEEP_AT_SIGN		256
#define Lf_OPT_TRACE_PRESENT	9
#define LF_NOT_CONST		0x0000010000
#define LF_NO_REG_OPT		0x0000020000
#define LF_IN_QUOTES		0x0000040000
#define LF_EXE_BLK		0x0000080000
#define LF_HAS_MISC_DATA	0x0000100000
#define LF_HAS_RETURN		0x0000200000
#define LF_ASM_EXPRESSIONS	0x0000400000
#define LF_UNRESOLVED		0x0000800000
#define LF_LOCAL		0x0001000000
#define LF_FUN_EXP		0x0002000000
#define Lf_FUN_EXP		25
#define LF_POSTINC		0x0004000000
#define LF_POSTDEC		0x0008000000
#define LF_PREINC		0x0010000000
#define LF_PREDEC		0x0020000000
#define LF_ARRAY		0x0040000000
#define LF_RAX			0x0080000000
#define LF_USE_LAST_U16		0x0100000000
#define LF_LAST_WAS_DOT		0x0200000000
#define LF_AOT_COMPILE		0x0400000000
#define Lf_AOT_COMPILE		34
#define LF_NO_ABSS		0x0800000000
#define LF_PAREN		0x1000000000
#define Lf_PAREN		36
#define LF_CLASS_IP		0x2000000000
#define LF_DONT_MAKE_RESULT	0x4000000000
#define LEX_MAX_IDENT	134 //Max is 132
#define LFN_CNT		8
#define LFN_STK_CNT	32

class CAsmCtrl
{
  I64 ip;	//instruction ptr
  CAsmArg arg1,arg2;
  CAsmCodeBlk *cb;
  I64 num_ex_U8s;
  I64 max_align,module_org;
  CAsmUnresolvedRef *local_unresolved,*glbl_unresolved;
  CExeAbsAddr *abss;
  CExeHeapGlbl *heap_glbls;
  I64 display_col,last_ip;
  U8 *last_label,*last_line_lsted;
  CLexFile *last_lfn;
  I64 seg_size;
  Bool has_locals,lst;
};

class CStreamBlk
{
  CStreamBlk *next,*last;
  U8 *body;
};

class CLexCodeCtrl
{
  CLexCodeCtrl	*lcc_next;
  CLexCodeMisc	*lcc_next_misc,*lcc_last_misc;
  CIntermediateCodeBase	lcc_root;
};

#define NUM_REGS	16

class CLexHashTableContext
{
  CLexHashTableContext *next;
  I64 old_flags;
  I64 hash_mask;
  CHashFun	*local_var_lst;
  CHashTable	*hash_table_lst;
  CHashTable	*define_hash_table;
  CHashTable	*local_hash_table;
  CHashTable	*glbl_hash_table;
};

public class CLex
{
  CLex	*next,*last;
  I64	token;
  I64	flags;
  I64	cur_i;
  F64	cur_f;
  U8	*ident;
  I64	ident_len;
  I64	class_ip;
  U8	*dollar;
  I64	dollar_cnt;
  U8	*cur_index;
  I64	last_U16;
  I64	min_line,max_line,last_line_num;
  I64	braces_cnt;
  I64	statement_cnt;
  I64	lock_cnt;
  U32	*alpha_numeric_bitmap;
  CLexHashTableContext htc;
  CHashGeneric	*hash_entry;
  CAbsCntsI64	abs_cnts;
  CLexUndef	*undef_hash_entry;
  CMemberLst	*local_var_entry;
  CLexCodeMisc	*lb_leave;
  U8 *cur_buf_ptr;
  CLexFile *cur_lfn,*fun_lfn;
  I64 stk_ptr;
  CStreamBlk *next_stream_blk,*last_stream_blk;
  CExe *exe_struct;

  I64 pass;
  I64 opts;
  I64 pass_trace_mask;
  I64 error_cnt,warning_cnt;

  //For intermediate codes with multiple float ops (int<->float conversions)
  I64 cur_ic_float_op_num,last_ic_float_op_num;
  CIntermediateCode *last_float_op_ic;
  Bool last_dont_pushable,last_dont_popable,last_float_op_pos,
	 dont_push_float,pad[4];

  CLexCodeCtrl lcc;
  CAsmCtrl *a;
  I64 a_depth;

  U0 end_of_zeroed_members;
  CLexFile lfns[LFN_CNT];
  CLexFile stk[LFN_STK_CNT];
U0 pad;#assert !($$&7) //(compiler lags one token)
};


class CPCIDev
{
  CPCIDev *next,*last;
  U16 vendor,dev_id;
  U8 bus,dev,fun,pad;
  U8 sub_code,base_code,pad[6];
  U8 *vendor_str,*dev_id_str;
};

#define TEMPLEOS_XSUM	0xA5CF3796

#define FILEMASK_JIT	"*.CPP*;*.HPP*"
#define FILEMASK_AOT	"*.CPP*;*.HPP*;*.PRJ*"
#define FILEMASK_SRC	"*.CPP*;*.HPP*;*.AUT*;*.PRJ*"
#define FILEMASK_TXT	FILEMASK_SRC ";*.TXT*;*.LOG*"
#define FILEMASK_GRA	"*.GRA*;*.BMP*"

public class CMBRPrt
{
  U8 active;	//0x80=active  0x00=inactive
  U8 start_head;
  U16 start_cyl;
  U8 type;
  U8 end_head;
  U16 end_cyl;
  U32 offset;	//Sects between MBR and first sect
  U32 size;	//Sects in partition
};

public class CMasterBoot
{
  U8 boot_code[440];
  U32 media_id;
  U16 zero;
  CMBRPrt p[4];
  U16 signature; //AA55
};

public class CBootStruct
{
  U8 jump_and_nop[3];
  U8 signature;		//MBR_PT_REDSEA=0x88
  U16 U8s_per_sect;
  U16 sects_per_cluster;
  I64 sects;
  I64 root_cluster;
  U32 bitmap_sects;
  U32 unique_id;
  U8 code[478];
  U16 signature2;	//0xAA55
};

class CFAT32Boot
{
  U8 jump_and_nop[3];
  U8 oem_name[8];
  U16 bytes_per_sect;
  U8 sects_per_cluster;
  U16 reserved_sects;
  U8 copies_of_fat;
  U16 max_root_dir_entries;	//Not used
  U16 old_sects_in_prt;		//Not used
  U8 media_desc;		//F64 for hard disk
  U16 old_sects_per_fat;	//Not used
  U16 sects_per_track;
  U16 num_heads;
  U32 hidden_sects;
  U32 sects;
  U32 sects_per_fat;
  U16 flags;
  U16 version;
  U32 root_cluster;
  U16 file_system_info_sect;
  U16 backup_boot_sect;
  U8 reserved[12];
  U8 log_drv_num;
  U8 unused;
  U8 ext_signature; //0x29
  U32 serial_num;
  U8 vol_name[11];
  U8 fat_name[8];
  U8 code[420];
  U16 signature;
};

class CFAT32FileInfoSect
{
  U32 signature1;
  U8 unknown[480];
  U32 signature2;
  U32 free_clusters;
  U32 most_recently_allocated;
  U8 reserved[12];
  U32 signature3;
};

class CFAT32DirEntry
{
  U8 name[11];
  U8 attr;
  U8 NTres;
  U8 CrtTimeTenth;
  U16 CrtTime;
  U16 CrtDate;
  U16 LstAccDate;
  U16 cluster_hi;
  U16 WrtTime;
  U16 WrtDate;
  U16 cluster_lo;
  U32 size;
};

class CFAT32DirEntryLong
{
  U8 ord;
  U16 name1[5];
  U8 attr;
  U8 type;
  U8 xsum;
  U16 name2[6];
  U16 zero;
  U16 name3[2];
};

#define FAT32_ENTRIES_PER_BLK	(BLK_SIZE/sizeof(CFAT32DirEntry))
#define FAT32_ENTRIES_BITS	Bsf(FAT32_ENTRIES_PER_BLK)

public class CPalindromeU16
{
  U16 little,big;
};

public class CPalindromeU32
{
  U32 little,big;
};

class CATAPITrack
{
  U16 track_info_len;
  U8 track_lo;
  U8 session_lo;
  U8 reserved;
  U16 misc1;
  U8 misc2;
  U32 track_start;
  U32 next_writable;
  U32 free_blks;
  union {
    U32 fixed_packet_size;
    U32 blocking_factor;
  };
  union {
    U32 track_size;
    U32 rzone_end;
  };
  U32 last_recorded;
  U8 track_hi;
  U8 session_hi;
  U16 reserved;
  U32 read_compatibility;
  U32 next_layer_jmp;
  U32 last_layer_jmp;
};

//ISO1 is ISO9660
//ISO2 is ISO13490
//ISO3 is ISO13346

class CISO1Date
{
  U8 year,mon,day,hour,min,sec,sec100;
};

class CISO1DirEntry
{
  U8 len;
  U8 ext_attr_len;
  CPalindromeU32 loc;
  CPalindromeU32 size;
  CISO1Date date;
  U8 flags;
  U8 file_unit_size;
  U8 interleave;
  CPalindromeU16 vol_seq_num;
  U8 name_len;
  U8 name;
};

class CISO1PathEntry
{
  U8 name_len;
  U8 zero;
  U32 blk;
  U16 parent_entry_num;
  U16 name[1];	//Aligned to U16 boundries
};

//9660
#define ISO1T_BOOT_RECORD		0
#define ISO1T_PRI_VOL_DESC		1
#define ISO1T_SUPPLEMENTARY_DESC	2
#define ISO1T_VOL_PRT_DESC		3
#define ISO1T_TERMINATOR		255

class CISO1PriDesc
{
  U8 type;
  U8 id[5];
  U8 version;
  U8 unused1;
  U8 system_id[32];
  U8 vol_id[32];
  U8 unused2[8];
  CPalindromeU32 vol_space_size;
  U8 unused3[32];
  CPalindromeU16 vol_set_size;
  CPalindromeU16 vol_seq_num;
  CPalindromeU16 log_block_size;
  CPalindromeU32 path_table_size;
  U32 type_l_path_table;
  U32 opt_type_l_path_table;
  U32 type_m_path_table;
  U32 opt_type_m_path_table;
  CISO1DirEntry root_dir_record;
  U8 vol_set_id[128];
  U8 publisher_id[128];
  U8 preparer_id[128];
  U8 application_id[128];
  U8 copyright_file_id[37];
  U8 abstract_file_id[37];
  U8 bibliographic_file_id[37];
  U8 creation_date[17];
  U8 modification_date[17];
  U8 expiration_date[17];
  U8 effective_date[17];
  U8 file_structure_version;
  U8 unused4;
  U8 application_data[512];
  U8 unused5[653];
};

#define ISO1_BASE_YEAR	1900

//media types
#define MT_CD		1
#define MT_DVD		2

public class CDateStruct
{
  U8 sec10000,sec100,sec,min,hour;
  U8 day_of_week,day_of_mon,mon;
  I32 year;
};


//CDirEntry flags (Used by $LK,"FileMgr","MN:FileMgr"$())
#define DEF_PROCESSED		1
#define DEF_REMOVABLE		2
#define DEF_NOT_INITIALIZED	4

#define CDIR_FILENAME_LEN	38 //Must have terminator
public class CDirEntry
{
  CDirEntry *next,*parent,*sub;
  U8 *full_name;
  I64 flags;
  I64 user_data;

  U0 start;
  U16 attr;
  U8 name[CDIR_FILENAME_LEN];
  I64 cluster;
  I64 size;
  CDate datetime;
};

#define CDIR_SIZE (sizeof(CDirEntry)-offset(CDirEntry.start))

//See $LK,"ST_BLKDEV_TYPES","MN:ST_BLKDEV_TYPES"$
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

#define NUM_BLKDEVS		32
#define BD_SIGNATURE_VAL	'BDSV'
public class CBlkDev
{
  CBlkDev *lock_fwding;
  I64 locked_flags;
  U32 type,flags;
  U32 bd_signature;
  U8 first_drv_let,unit,pad[2];
  U32 base0,base1;
  U32 blk_size;
  I64 offset,min_blk,max_blk;
  U16 *dev_id_record;
  U8 *RAM_dsk;
  CTask *owning_task;
  F64 last_time;
  U32 max_reads,max_writes;
};


class CATARep
{
  CATARep *next;
  I64 num,type,base0,base1,unit,irq;
};

//Partition locked flags
#define Plf_LOCKED	0

//See $LK,"ST_PRT_TYPES","MN:ST_PRT_TYPES"$
#define PT_NULL		0
#define PT_REDSEA	1 //Supported
#define PT_FAT32	2 //Supported except for short names, to some degree
#define PT_ISO9660	3 //Supported (CD-ROM)
#define PT_ISO13490	4 //Not Supported (CD-ROM)
#define PT_ISO13346	5 //Not Supported (CD-ROM)
#define PT_NTFS		6 //Not Supported
#define PT_UNKNOWN	7
#define PT_NUM_TYPES	8

#define PTf_DISABLE	31 //High bit of type is disable

#define MBR_PT_FAT12	0x01
#define MBR_PT_FAT32a	0x0B
#define MBR_PT_FAT32b	0x0C
#define MBR_PT_FAT32c	0x1B
#define MBR_PT_FAT32d	0x1C
#define MBR_PT_FAT32e	0x8B
#define MBR_PT_FAT32f	0x8C
#define MBR_PT_NTFS	0x07
#define MBR_PT_REDSEA	0x88

public class CFreeLst
{
  CFreeLst *next,*last;
  I64 start,size;
};

#define NUM_PRTS		32

#define PRT_SIGNATURE_VAL	'PTSV'
public class CPrt
{
//Don't access ->drive directly in case a drive has been remapped.
//Use $LK,"Prt2Drv","MN:Prt2Drv"$().
  I64 locked_flags;
  U32 type,  //Bit31 is used as a disable, since it will be unknown
      flags; //Flags not used at the moment
  U32 pt_signature;
  U8 drv_let,text_attr,pad[2];
  I64 offset;
  I64 size;
  I64 file_system_info_sect;
  I64 map1,map2;
  I64 root_cluster;
  I64 data;
  I64 spc; //sects_per_cluster
  CTask *owning_task;
  CBlkDev *bdev;

  CFAT32FileInfoSect *fis;
  I64 num_buffered_map_blks;
  I64 cur_map_blk_num;
  U32 *cur_map_blk;
  U8 *buffered_map_dirty_bits;
  CFreeLst *next_free,*last_free;

  Bool (*rblks)(CPrt *p,U8 *buf,I64 blk,I64 cnt);
  Bool (*wblks)(CPrt *p,U8 *buf,I64 blk,I64 cnt);
};

#define DSK_CACHE_HASH_SIZE	0x2000

public class CCacheBlk
{
  CCacheBlk *next_lru,*last_lru;
  CCacheBlk *next_hash,*last_hash;
  CPrt *p;
  I64 blk;
  U8 body[BLK_SIZE];
};

#define EXCEPT_LOCAL		0 // good for narrow scope user handling
#define EXCEPT_BREAK		1
#define EXCEPT_OUT_OF_MEM	2
#define EXCEPT_DIVIDE_BY_ZERO	3
#define EXCEPT_FLOATING		4
#define EXCEPT_COMPILER		5
#define EXCEPT_UNDEF_DEFINE	6
#define EXCEPT_UNDEF_EXTERN	7
#define EXCEPT_FILE		8
#define EXCEPT_ARC_XSUM		9
#define EXCEPT_PRT		10
#define EXCEPT_BLKDEV		11
#define EXCEPT_GRAPHICS		12
#define EXCEPT_SPRINT		13
#define EXCEPT_SCAN		14
#define EXCEPT_BIN_MODULE	15
#define EXCEPT_MULTICORE	16
#define EXCEPT_FUF		17
#define EXCEPT_FLAGS		18
//$LK,"EXCEPT_DEFINE","FF:::/TempleOS/Kernel/Define1a.CPP,ST_EXCEPT"$

class CExcept
{
  CExcept *next,*last;
  I64 handler_catch,handler_untry;
  I64 rsp,rbp,rflags;
  I64 rsi,rdi,r10,r11,
      r12,r13,r14,r15;
};

#define MBS_USED_SIGNATURE_VAL		'MBUs'
#define MBS_UNUSED_SIGNATURE_VAL	'MBUn'
class CMemBlk
{
  CMemBlk *next,*last;
  U32 mb_signature;
  U32 pages;
};

#define MRT_UNUSED	0
#define MRT_RAM		1
#define MRT_DEV		2

class CMemRange
{
  CMemRange *next,*last;
  U32 type,flags;
  U8 *base;
  I64 size;
};


#define _CFG_HEAP_DBG FALSE

#if _CFG_HEAP_DBG
class CMemUnusedAllocated
{
  CHeapCtrl *hc;
  U8 *caller1,*caller2;
  CMemUnusedAllocated *next;
  I64 size;
};
class CMemUsedAllocated
{
  CHeapCtrl *hc;
  U8 *caller1,*caller2;
  CMemUsedAllocated *next,*last;
  I64 size;
  U0 start;
};
#else
class CMemUnusedAllocated
{
  U0 hc;
  U0 caller1,caller2;
  CMemUnusedAllocated *next;
  I64 size;
};
class CMemUsedAllocated
{
  CHeapCtrl *hc;
  U0 caller1,caller2;
  U0 next,last;
  I64 size;
  U0 start;
};
#endif


//TempleOS does not mess with page tables after it
//identity-maps them during boot -- $LK,"SYS_INIT_MEM","MN:SYS_INIT_MEM"$.
//$LK,"PAGE_SIZE","MN:PAGE_SIZE"$ is arbitrary, and is not related to CPU hardware.
//I need some unit for memory allocation so I invented "$LK,"PAGE_SIZE","MN:PAGE_SIZE"$".
//
//All virtual addresses are identical to physical, always.
//There is no high kernel memory because RAM's physical
//addresses are low.  (All motherboards that I know of
//put RAM in low addresses of 64-bit address space.)
//It's ring-0-only so all (or none) is kernel, remember.
//
//2 Meg hardware pages are use except the first 2 Meg.
//0xA0000-0xBFFFF must be write thru.  Hardware pages
//don't matter except the caching bit.	Remember, it is ring-0-only
//and identity-mapped!	Do I need to repeat?
//Ring-0-only and 100% identity-mapped, all the time.
//
//All code is placed in just the lowest 2 Gig, so the CALL REL32
//instruction always works.  It never changes out of ring-0, remember,
//so I don't need to waste cycles on use SYSCALL or software interrupts.
//Everything just uses CALL REL32.

#define PAGE_BITS		9
#define PAGE_SIZE		(1<<PAGE_BITS)

#define HEAP_HASH_SIZE		1024
#define FREE_PAGE_HASH_SIZE	0x100

#define ADAM_STK		(PAGE_SIZE*512) //limit of like 16384*PAGE_SIZE
#define SETH_STK		(PAGE_SIZE*512)
#define INTERRUPT_STK		(PAGE_SIZE*512)
#define DFT_STK			(PAGE_SIZE*512)
#define MP_DFT_STK		(PAGE_SIZE*512)

#define ADAM_HASH_SIZE		(1<<18)
#define STD_HASH_SIZE		(1<<12)
#define MP_STD_HASH_SIZE	(1<<12)

//locked flags
#define BPlf_LOCKED		0
public class CBlkPool
//See $LK,"SYS_BP_START","FF:::/TempleOS/Kernel/KStart.CPP,SYS_BP_START"$
{
  I64 locked_flags,size;
  I64 allocated_u8s;
  CMemBlk *mem_free_lst;
  CMemBlk *mem_free_2meg_lst;
  CMemBlk *free_page_hash[FREE_PAGE_HASH_SIZE];
  CMemBlk *free_page_hash2[64-PAGE_BITS];
};

#define HCf_NON_TASK_QUE	0
//locked flags
#define HClf_LOCKED		0

#define HEAP_CTRL_SIGNATURE_VAL	'HcSV'
public class CHeapCtrl
{
  CBlkPool *bp;
  U32 hc_signature,pad;
  I64 locked_flags,flags;
  I64 used_u8s;
  CTask *mem_task;
  CMemBlk *next_mem_blk,*last_mem_blk;
  CMemUnusedAllocated *malloc_free_lst;
  CMemUsedAllocated   *next_um,*last_um;
  CMemUnusedAllocated *heap_hash[HEAP_HASH_SIZE/sizeof(U8 *)];
};

//Raster operations
#define ROP_EQU				0x0000
#define ROP_XOR				0x0100
#define ROP_COLLISION			0x0200
#define ROP_MONO			0x0300

//These are just for $LK,"ROPF_PROBABILITY_DITHER","MN:ROPF_PROBABILITY_DITHER"$
//See $LK,"GrLighting","MN:GrLighting"$().
#define ROPF_HALF_RANGE_COLOR		0x1000
#define ROPF_TWO_SIDED			0x2000

//These always go in the c1.rop of a CColorROPU32
#define ROPF_DITHER			0x40000000
#define ROPF_PROBABILITY_DITHER		0x80000000

#define ROPB_EQU			0x00
#define ROPB_XOR			0x01
#define ROPB_COLLISION			0x02
#define ROPB_MONO			0x03
#define ROPBF_HALF_RANGE_COLOR		0x10
#define ROPBF_TWO_SIDED			0x20
#define ROPBF_DITHER			0x40
#define ROPBF_PROBABILITY_DITHER	0x80

public U16 class CColorROPU16 //Don't use this, use CColorROPU32
{
  U8 color,rop;
};

public U32 class CColorROPU32
{
  CColorROPU16 c0,c1;
};
#define COLORROP_COLORS_MASK	0x00FF00FF
#define COLORROP_NO_ROP0_MASK	0xFFFF00FF
#define COLORROP_BITS		16

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
  I64 min,pos,max;
  U32 flags;
  U8 color,pad[3];
};

public class CViewAngles
{
  I64 sx,sy,sz;
  F64 ax,ay,az;
  CColorROPU32 cx,cy,cz,cbd,cbg,cfg;
};

public class CCtrl
{
  CCtrl *next,*last;
  CTask *win_task;
  I64 type,flags;

//win pixel coordinates
  I64 left,right,top,bottom;

//screen pixel coordinates (derived)
  I64 screen_left,screen_right,
     screen_top,screen_bottom;

  U8 *state;

//called on resize
  U0 (*update_derived_vals)(CCtrl *c);

  U0 (*draw_it)(CDC *dc,CCtrl *c);

  Bool (*inside_ctrl)(CCtrl *c,I64 x,I64 y); //For nonbox shapes
  U0 (*left_click)(CCtrl *c,I64 x,I64 y,Bool down);
  U0 (*right_click)(CCtrl *c,I64 x,I64 y,Bool down);
  U0 (*wheel_chg)(CCtrl *c,I64 delta);
};

#define MENU_SIGNATURE_VAL	'MnuS'
class CMenuEntry
{
  CMenuEntry *next;
  CMenuEntry *sub;
  U8 name[32];
  I64 msg_code,p1,p2;
  Bool checked,dir,pad[6];
};

class CMenu
{
  CMenu *next;
  CMenuEntry *sub;
  CTask *task;
  I64 flags;
  U32 mu_signature;
  U8 attr,checked_attr,pad[2];
};

//win_inhibit mask
#define WIF_MENU		0x001
#define WIF_CTRLS		0x002
#define WIF_IP_WHEEL		0x004
#define WIF_IP_L		0x008
#define WIF_IP_R		0x010
#define WIF_BORDER		0x020
#define WIF_DBL_CLICK		0x040
#define WIF_GRAB_SCROLL		0x080
#define WIF_DOC			0x100
#define WIF_ODE			0x200
#define WIF_ALL			(-1-WIF_DOC-WIF_ODE)

#define WIf_MENU		0
#define WIf_CTRLS		1
#define WIf_IP_WHEEL		2
#define WIf_IP_L		3
#define WIf_IP_R		4
#define WIf_BORDER		5
#define WIf_DBL_CLICK		6
#define WIf_GRAB_SCROLL		7
#define WIf_DOC			8
#define WIf_ODE			9

#define WIN_DFT_MAX_REFRESH	60.0

class CFPU
{
  U8 body[512];
};

public U32i class CRGB
{
  U8 r,g,b,reserved;
};

public U32i class CBGR
{
  U8 b,g,r,reserved;
};

public class CSnap
{
  I64 x,y,z;
  F64 dx,dy,dz;
};

#define TSF_SAME_SONG	1
#define TASK_TITLE_LEN			127

class CTaskSettings
{
  CTaskSettings *next;
  I64 left,right,top,bottom;
  U0 (*draw_it)(CTask *task,CDC *dc);
  U0 (*task_end_cb)();
  CTask *song_task,*animate_task;
  F64	win_max_refresh;
  I64	scroll_x,scroll_y,scroll_z;
  CSnap snap;
  CBGR palette4[16];
  U32	win_inhibit;;
  U8	text_attr,  title_src;
  U8	border_attr,border_src;
  U8	task_title[TASK_TITLE_LEN+1];
  Bool border;
  Bool preempt;
  Bool cursor;
  Bool scroll;
  Bool wordstat;
  U8	pad[3];
U0 pad;#assert !($$&7) //(compiler lags one token)
};

#define NUM_PROGRESS_BARS	4
#define PROGRESS_DESC_LEN	(64-8-8-1)
class CProgress
{
  I64 val,max;
  U8  desc[PROGRESS_DESC_LEN+1];
};

//Border ATTR src
#define BDS_CONST		0
#define BDS_CUR_PRT		1
#define BDS_ED_FILENAME_PRT	2

//Task title src
#define TTS_CONST		0
#define TTS_LOCKED_CONST	1 //This is not strictly enforced
#define TTS_TASK_NAME		2
#define TTS_ED_FILENAME		3
#define TTS_CUR_LEX		4

// TASK task flags
#define TASKf_KILL_TASK		0
#define TASKf_SUSPENDED		1
#define TASKf_USER		2
#define TASKf_PREEMPT		3
#define TASKf_IDLE		4
#define TASKf_INPUT_FILTER_TASK	5
#define TASKf_FILTER_INPUT	6
#define TASKf_HAS_SONG		7
#define TASKf_NOT_RAW		8
#define TASKf_DISABLE_BPTS	9
#define TASKf_AWAITING_MSG	10
#define TASKf_CURRENTLY_RUNNING	11
#define TASKf_BREAK_LOCKED	12
#define TASKf_PENDING_BREAK	13
#define TASKf_BREAK_TO_SHIFT_ESC 14
#define TASKf_INIT_EXT		15
#define TASKf_KILL_AFTER_DBG	16

//locked flags
#define TASKLf_TASK_LOCK		0

#define DISPLAYf_SHOW			0
#define DISPLAYf_HAS_CLOSE_WIN		1
#define DISPLAYf_SILENT			2
#define DISPLAYf_NO_BORDER		3
#define DISPLAYf_WIN_ON_TOP		4
#define DISPLAYf_CHILDREN_NOT_ON_TOP	5
#define DISPLAYf_HAS_BEEN_RESIZED	6

#define TASK_SIGNATURE_VAL		'TskS'
#define TASK_IN_QUE_SIGNATURE_VAL	'TskQ'
#define TASK_COMBINED_SIGNATURE_VAL	(TASK_SIGNATURE_VAL+TASK_IN_QUE_SIGNATURE_VAL<<32)

#define TASK_NAME_LEN			31
#define TASK_WALLPAPER_DATA_SIZE	128
class CTaskStk
{
  CTaskStk *next_stk;
  I64 stk_size;
  I64 stk_ptr;
  U0 stk_base;
};

public class CTask //The Fs segment register points to current CTask
{
  CTask *addr; //Self-addressed ptr
  U32 task_signature,task_in_que_signature;
  I64 task_flags,locked_flags;
  U32 display_flags,win_inhibit;
  I64 wake_jiffy;

  CHeapCtrl *code_heap,*data_heap;

  CDoc	*put_doc,*display_doc, //When double buffering, these two differ.
	*border_doc;
  I64	win_left;
  I64	win_right;
  I64	win_top;
  I64	win_bottom;

  CPrt *cur_prt;
  U8	*cur_dir;

  CTask *parent_task;
  CTask *next_task,*last_task;
  CTask *next_input_filter_task,*last_input_filter_task;
  CTask *next_sibling_task,*last_sibling_task;
  CTask *next_child_task,*last_child_task;

  //These are derived from left,top,right,bottom
  I64	win_width;
  I64	win_height;
  I64	win_pixel_left; //These are in pixels, not characters
  I64	win_pixel_right;
  I64	win_pixel_width;
  I64	win_pixel_top;
  I64	win_pixel_bottom;
  I64	win_pixel_height;
  I64	win_border_pixel_left;
  I64	win_border_pixel_right;
  I64	win_border_pixel_top;
  I64	win_border_pixel_bottom;

  I64	win_scroll_x,win_scroll_y,win_scroll_z;
  F64	win_max_refresh;
  CSnap snap;

//These must be in this order
//for $LK,"SYS_CONTEXT_SAVE","FF:::/TempleOS/Kernel/Scheduler.CPP,SYS_CONTEXT_SAVE"$ and $LK,"SYS_CONTEXT_RESTORE","FF:::/TempleOS/Kernel/Scheduler.CPP,SYS_CONTEXT_RESTORE"$
  I64 rip;
  I64 rflags;
  I64 rsp;
  I64 rsi;
  I64 rax;
  I64 rcx;
  I64 rdx;
  I64 rbx;
  I64 rbp;
  I64 rdi;
  I64 r8;
  I64 r9;
  I64 r10;
  I64 r11;
  I64 r12;
  I64 r13;
  I64 r14;
  I64 r15;

  CCPU *gs;

  CFPU *fpu_mmx;
  I64	time_slice_start;
  I64	total_time;  //Cycles
  I64	swap_cnter;

  U0	(*draw_it)(CTask *task,CDC *dc);

  U8	task_title[TASK_TITLE_LEN+1];
  U8	task_name[TASK_NAME_LEN+1];
  U8	wallpaper_data[TASK_WALLPAPER_DATA_SIZE];

  U8	title_src;
  U8	text_attr;
  U8	border_attr,border_src;
  U16	win_z_num,pad;

  CTaskStk *stk;

  CExcept *next_except,*last_except;
  I64	except_rbp;	//Stores throw routine's RBP
  I64	except_argc;
  I64	*except_argv;

  Bool	catch_except,pad[1];
  U16	answers_displayed;
  U8	answer_types[4];
  I64	answers[4];
  F64	answer_times[4];

  CBpt *bpt_lst;
  CCtrl	*next_ctrl,*last_ctrl;
  CMenu	*cur_menu;
  CTaskSettings *next_settings;
  CMathODE	*next_ode,*last_ode;
  F64	last_ode_time;
  CHashTable *hash_table;

#define MSG_NULL		0
#define MSG_CMD			1
#define MSG_KEY_DOWN		2
#define MSG_KEY_UP		3
#define MSG_IP_MOVE		4
#define MSG_IP_L_DOWN		5
#define MSG_IP_L_UP		6
#define MSG_IP_L_D_DOWN 	7
#define MSG_IP_L_D_UP		8
#define MSG_IP_R_DOWN		9
#define MSG_IP_R_UP		10
#define MSG_IP_R_D_DOWN 	11
#define MSG_IP_R_D_UP		12
#define MSG_FOCUS		13
#define MSG_RESIZE		14
#define MSG_MOVE		15

#define MSG_KEY_DOWN_UP		-2
#define MSG_IP_L_DOWN_UP	-5
#define MSG_IP_L_D_DOWN_UP	-7
#define MSG_IP_R_DOWN_UP	-9
#define MSG_IP_R_D_DOWN_UP	-11

#define MSGF_ALLOW_KEY_DESC	63

  CSrvCtrl srv_ctrl;
  CLex *next_lex,*last_lex;

  I64	win_old_left;
  I64	win_old_right;
  I64	win_old_top;
  I64	win_old_bottom;

  U0	(*task_end_cb)();
  CTask *song_task,*animate_task;
  I64	rnd_seed;
  I64	task_num,user_num;
  I64	fault_num,fault_err_code;
  CTask *dbg_task;
  CTask *popup_task;
  CWinScroll horz_scroll,vert_scroll;

  I64	user_data0,user_data1,
	user_data2,user_data3;
U0 pad;#assert !($$&7) //(compiler lags one token)
};

class CTSS
{
  U32 res1;
  I64 rsp0,rsp1,rsp2;
  I64 res2;
  I64 ist1,ist2,ist3,ist4,ist5,ist6,ist7;
  I64 res3;
  U16 res4,io_map_offset;
  U8 io_map[0x10000/8];
  I64 *st0,*st1,*st2;
  U16 tr,tr_ring3;
};

#define ans  (Fs->answers[0])
#define ans0 (Fs->answers[0])
#define ans1 (Fs->answers[1])
#define ans2 (Fs->answers[2])
#define ans3 (Fs->answers[3])

#define ansf  (Fs->answers[0](F64))
#define ansf0 (Fs->answers[0](F64))
#define ansf1 (Fs->answers[1](F64))
#define ansf2 (Fs->answers[2](F64))
#define ansf3 (Fs->answers[3](F64))

#define KBD_PORT 0x60
#define KBD_CTRL 0x64

#define CH_CTRLA		0x01
#define CH_CTRLB		0x02
#define CH_CTRLC		0x03
#define CH_CTRLD		0x04
#define CH_CTRLE		0x05
#define CH_CTRLF		0x06
#define CH_CTRLG		0x07
#define CH_CTRLH		0x08
#define CH_CTRLI		0x09
#define CH_CTRLJ		0x0A
#define CH_CTRLK		0x0B
#define CH_CTRLL		0x0C
#define CH_CTRLM		0x0D
#define CH_CTRLN		0x0E
#define CH_CTRLO		0x0F
#define CH_CTRLP		0x10
#define CH_CTRLQ		0x11
#define CH_CTRLR		0x12
#define CH_CTRLS		0x13
#define CH_CTRLT		0x14
#define CH_CTRLU		0x15
#define CH_CTRLV		0x16
#define CH_CTRLW		0x17
#define CH_CTRLX		0x18
#define CH_CTRLY		0x19
#define CH_CTRLZ		0x1A

#define CH_CURSOR		0x05
#define CH_BACKSPACE		0x08
#define CH_FORM_FEED		0x0C
#define CH_ESC			0x1B
#define CH_SHIFT_ESC		0x1C
#define CH_SHIFT_SPACE		0x1F
#define CH_SPACE		0x20

//Scan code flags
#define SCf_E0_PREFIX		7
#define SCf_KEY_UP		8
#define SCf_SHIFT		9
#define SCf_CTRL		10
#define SCf_ALT 		11
#define SCf_CAPS		12
#define SCf_NUM 		13
#define SCf_SCROLL		14
#define SCf_NEW_KEY		15
#define SCf_IP_L_DOWN		16
#define SCf_IP_R_DOWN		17
#define SCf_DELETE		18
#define SCf_INS			19
#define SCf_KEY_DESC		31
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
#define SCF_KEY_DESC	(1<<SCf_KEY_DESC)

//TempleOS places a 1 in bit 7 for
//keys with an E0 prefix.
//See $LK-A,"::/TempleOS/Doc/Char.TXT"$
//and $LK,"KbdHandler","MN:KbdHandler"$().
#define SC_ESC			0x01
#define SC_BACKSPACE		0x0E
#define SC_TAB			0x0F
#define SC_ENTER		0x1C
#define SC_SHIFT		0x2A
#define SC_CTRL			0x1D
#define SC_ALT			0x38
#define SC_CAPS			0x3A
#define SC_NUM			0x45
#define SC_SCROLL		0x46
#define SC_CURSOR_UP		0x48
#define SC_CURSOR_DOWN		0x50
#define SC_CURSOR_LEFT		0x4B
#define SC_CURSOR_RIGHT 	0x4D
#define SC_PAGE_UP		0x49
#define SC_PAGE_DOWN		0x51
#define SC_HOME			0x47
#define SC_END			0x4F
#define SC_INS			0x52
#define SC_DELETE		0x53
#define SC_F1			0x3B
#define SC_F2			0x3C
#define SC_F3			0x3D
#define SC_F4			0x3E
#define SC_F5			0x3F
#define SC_F6			0x40
#define SC_F7			0x41
#define SC_F8			0x42
#define SC_F9			0x43
#define SC_F10			0x44
#define SC_F11			0x57
#define SC_F12			0x58
#define SC_PAUSE		0x61
#define SC_GUI			0xDB
#define SC_PRTSCRN1		0xAA
#define SC_PRTSCRN2		0xB7

#define FONT_WIDTH		8
#define FONT_HEIGHT		8

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
#define COLOR_INVALID		16
#define COLOR_TRANSPARENT	0xFF
#define COLOR_MONO		0xFF

//Low 8 bits reserved for flags that go into saved bitmaps
#define DCF_COMPRESSED		1
#define DCF_NO_TRANSPARENTS	2 //Can be used to optimized $LK,"GrBlot","MN:GrBlot"$().

#define DCF_TRANSFORMATION	0x100

//See $LK,"GrSetSymmetry","MN:GrSetSymmetry"$() or $LK,"GrSetSymmetry3","MN:GrSetSymmetry3"$()
#define DCF_SYMMETRY		0x200

//Must be used with DCF_SYMMETRY set also.
//See $LK-A,"::/TempleOS/Demo/Games/BigGuns.CPP"$
#define DCF_JUST_MIRROR		0x400

#define DCF_LOCATE_NEAREST	0x800
#define DCF_DONT_DRAW		0x1000
#define DCF_ALIAS		0x2000
#define DCF_SCREEN_BITMAP	0x4000
#define DCF_FILL_NOT_COLOR	0x8000
#define DCF_RECORD_EXTENTS	0x10000
#define DCF_ON_TOP		0x20000

#define DCT_COLOR4	1 //Don't use this
#define DCT_COLOR8	3

#define DCS_SIGNATURE_VAL			'DvCS'

public class CGrSym
{
  I32 sx,sy,sz,pad;
  //Normal of symmetry plane
  I64 snx,sny,snz;
};

//z-vals less than zero are in front of screen and not drawn.
//we want to shift all Z-vals into a drawable range.
//GR_Z_ALL is set to half of the Z-range which is an I32.
#define GR_Z_ALL	(MAX_I32/2)

public class CDC
{
  U0 start_saved_area;
  I32 type,width,width_internal,height;
  I64 flags;
  I64 plane_size;
  U0 end_saved_area;

//public--change directly
  CColorROPU32 color,
	bkcolor, //Set for use with $LK,"ROP_COLLISION","MN:ROP_COLLISION"$
	color2; //Internally used for $LK,"GrFloodFill","MN:GrFloodFill"$()
  CD3I32 ls; //Light source (should be normalized to 65536).

  //dither_probability is basically a U16.
  //It is activated by $LK,"ROPF_PROBABILITY_DITHER","MN:ROPF_PROBABILITY_DITHER"$.
  //0x0000 =100% color.c0
  //0x8000 =50%  color.c0   50% color.c1
  //0x10000=100% color.c1
  U32 dither_probability, //See $LK-A,"::/TempleOS/Demo/Graphics/Shading.CPP"$.
     pad;

  CDC *brush;

//Set with $LK,"GrSetRotMat","MN:GrSetRotMat"$().  $LK,"Free","MN:Free"$() before setting.
  I64 *r;  //rotation matrix of quads decimal in lo
  I64 r_norm; //shifted 32 bits.  Used for scaling pen width

//public--change directly
  I32 x,y,z,pen_width;	//translation

//Can be changed from the default $LK,"GrTransform","MN:GrTransform"$()
  U0 (*transform)(CDC *dc,I64 *x,I64 *y,I64 *z);
//Can be changed from the default $LK,"GrLighting","MN:GrLighting"$()
  U0 (*lighting)(CDC *dc,CD3I32 *p1,CD3I32 *p2,CD3I32 *p3,CColorROPU32 color);

//Set by $LK,"GrSetSymmetry","MN:GrSetSymmetry"$() or $LK,"GrSetSymmetry3","MN:GrSetSymmetry3"$()
  CGrSym sym;

//not document num, but num in a CSprite collection
  I64 nearest_sprite_elem_num;

  I64 nearest_dist;

//not document num, but num in a CSprite collection
  I64 cur_sprite_elem_num;

  I32 cur_x,cur_y,cur_z,pad;
  I64 collision_cnt;

  //Set by $LK,"DCF_RECORD_EXTENTS","MN:DCF_RECORD_EXTENTS"$
  I64 min_x,max_x,min_y,max_y; //screen coordinates

  U32 dc_signature,pad;
  CTask *mem_task,*win_task;
  CDC *alias;
  U8 *body;

  //Set by $LK,"DCAllocDepthBuf","MN:DCAllocDepthBuf"$()
  I32 *depth_buf;
  I64 db_z; //private
};

class CRAXRBCRCXRDX
{
  I64 rax,rbx,rcx,rdx;
};

#define VGA_GRAPHIC_MEM_BASE	0xA0000
#define VGA_TEXT_MEM_BASE	0xB8000

#define VGA_ATTR_INDEX		0x03C0
#define VGA_ATTR_DATA_WRITE	0x03C0 //alias
#define VGA_ATTR_DATA_READ	0x03C1
#define VGA_MISC_OUTPUT 	0x03C2
#define VGA_SC_INDEX		0x03C4
#define VGA_SC_DATA		0x03C5
#define VGA_PALETTE_MASK	0x03C6
#define VGA_REG_READ		0x03C7
#define VGA_REG_WRITE		0x03C8
#define VGA_PALETTE_DATA	0x03C9
#define VGA_CRTC_INDEX		0x03D4
#define VGA_CRTC_DATA		0x03D5
#define VGA_INPUT_STAT		0x03DA
#define VGA_MAP_MASK		0x02

#define VGA_H_TOTAL		0x00
#define VGA_H_BLANK_START	0x02
#define VGA_H_RETRACE_START	0x04
#define VGA_V_TOTAL		0x06
#define VGA_MAX_SCAN_LINE	0x09
#define VGA_V_RETRACE_END	0x11
#define VGA_OFFSET		0x13
#define VGA_V_BLANK_START	0x15
#define VGA_MODE_CTRL		0x17

#define ARC_MAX_BITS 12
#define ARC_MAX_ENTRY (1<<ARC_MAX_BITS-1)

#define CT_NONE 	0
#define CT_7_BIT	1
#define CT_8_BIT	2
class CArcEntry
{
  CArcEntry *next;
  U16 basecode;
  U8 ch,pad;
  U32 pad;
};

public class CArcCtrl
{
  I64 src_pos;
  I64 src_size;
  I64 dst_pos;
  I64 dst_size;
  U8 *src_buf;
  U8 *dst_buf;
  I64 min_bits;
  I64 min_table_entry;
  CArcEntry *cur_entry;
  CArcEntry *next_entry;
  I64 cur_bits_in_use;
  I64 next_bits_in_use;
  U8 *stk_ptr;
  U8 *stk_base;
  I64 free_index;
  I64 free_limit;
  I64 saved_basecode;
  I64 entry_used;
  I64 last_ch;
  CArcEntry compress[ARC_MAX_ENTRY+1];
  CArcEntry *hash[ARC_MAX_ENTRY+1];
};

public class CArcCompress
{
  I64 compressed_size,
       expanded_size;
  U16 compression_type,flags;
  U0 body;
};

// Flags for StrUtil and MStrUtil
#define SUF_REM_SPACES		0x001
#define SUF_REM_CTRL_CHARS	0x002
#define SUF_REM_LEADING		0x004
#define SUF_SINGLE_SPACE	0x008
#define SUF_TO_UPPER		0x010
#define SUF_TO_LOWER		0x020
#define SUF_REM_TRAILING	0x040
#define SUF_CAP_UNDERSCORES	0x080
#define SUF_S2T			0x100
#define SUF_T2S			0x200 // Only works with MStrUtil
#define SUF_SAFE_DOLLAR		0x400
#define SUF_SCALE_INDENT	0x800

// Flags for StrFind
#define SFF_IGNORE_CASE		1
#define SFF_WHOLE_LABELS_FRONT	2
#define SFF_WHOLE_LABELS_BACK	4
#define SFG_WHOLE_LABELS	(SFF_WHOLE_LABELS_FRONT|SFF_WHOLE_LABELS_BACK)

//Flags for LstMatch
#define LMF_IGNORE_CASE		1
#define LMF_EXACT		2

#define ISO1_ATTR_DIR		2

#define _ATTR_READ_ONLY	0x01
#define _ATTR_HIDDEN		0x02
#define _ATTR_SYSTEM		0x04
#define _ATTR_VOL_ID		0x08
#define _ATTR_DIR		0x10
#define _ATTR_ARCHIVE		0x20
#define _ATTR_LONG_NAME		(_ATTR_READ_ONLY|_ATTR_HIDDEN|_ATTR_SYSTEM|_ATTR_VOL_ID)
#define _ATTR_LONG_NAME_MASK	(_ATTR_LONG_NAME|_ATTR_DIR|_ATTR_ARCHIVE)
#define _ATTR_DELETED		0x100
#define _ATTR_ENCRYPTED		0x200 // not implemented
#define _ATTR_RESIDENT		0x400
#define _ATTR_COMPRESSED	0x800
#define _ATTR_CONTIGUOUS	0x1000
#define _ATTR_FIXED		0x2000

public class CFileAccess
{
  CPrt *old_prt,*p;
  U8 *old_dir,*mask;
};

#define FFB_NEXT_BLK		MAX_U32

#define FF_WRITE		1
#define FF_NEW_FILE		2
#define FF_BUF_DIRTY		4
#define FF_NEEDS_WRITE		8
#define FF_CONTIGUOUS		16
#define FF_USE_OLD_DATETIME	32

public class CFile
{
  I64 flags;
  CDirEntry de;
  CPrt *p;
  I64 fblk_num,cluster,file_cluster_num,max_blk;
  U8 *cluster_buf;
};

//File util flags
//See $LK,"ST_FILE_UTIL_FLAGS","MN:ST_FILE_UTIL_FLAGS"$
#define FUf_RECURSE		0
#define FUf_DIFF		1
#define FUf_IGNORE		2
#define FUf_ALL			3
#define FUf_CANCEL		4
#define FUf_REPLACE		5
#define FUf_PUBLIC		6
#define FUf_MAP			7
#define FUf_JUST_DIRS		8
#define FUf_JUST_FILES		9
#define FUf_JUST_TXT	10
#define FUf_JUST_SRC	11
#define FUf_JUST_AOT	12
#define FUf_JUST_JIT	13
#define FUf_JUST_GRA	14
#define FUf_Z_OR_NOT_Z		15
#define FUf_SCAN_PARENTS	16
#define FUf_FLATTEN_TREE	17
#define FUf_WHOLE_LABELS	18
#define FUf_WHOLE_LABELS_FRONT	19
#define FUf_WHOLE_LABELS_BACK	20

#define FUF_RECURSE		0x000001 //r
#define FUF_DIFF		0x000002 //d
#define FUF_IGNORE		0x000004 //i
#define FUF_ALL			0x000008 //a
#define FUF_CANCEL		0x000010 //c
#define FUF_REPLACE		0x000020 //R
#define FUF_PUBLIC		0x000040 //p
#define FUF_MAP			0x000080 //m
#define FUF_JUST_DIRS		0x000100 //D
#define FUF_JUST_FILES		0x000200 //F
#define FUF_JUST_TXT		0x000400 //T
#define FUF_JUST_SRC		0x000800 //S
#define FUF_JUST_AOT		0x001000 //A
#define FUF_JUST_JIT		0x002000 //J
#define FUF_JUST_GRA		0x004000 //G
#define FUF_Z_OR_NOT_Z		0x008000 //Z
#define FUF_SCAN_PARENTS	0x010000 //P
#define FUF_FLATTEN_TREE	0x020000 //f
#define FUF_WHOLE_LABELS	0x040000 //l
#define FUF_WHOLE_LABELS_FRONT	0x080000 //lf
#define FUF_WHOLE_LABELS_BACK	0x100000 //lb

#define FUG_FILES_FIND	(FUF_RECURSE|FUF_JUST_DIRS|FUF_JUST_FILES|FUF_JUST_TXT|\
  FUF_JUST_SRC|FUF_JUST_AOT|FUF_JUST_JIT|FUF_JUST_GRA|FUF_FLATTEN_TREE)
#define FUG_FILE_FIND	(FUF_JUST_DIRS|FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS)

#define CPU_WALLPAPER_DATA_SIZE		128
#define CPUf_HAD_IRQ_OPPORTUNITY	0
#define CPUf_SETH_SLEEP			1

public class CCPU //The Gs segment register points to current CCPU
{
  CCPU	*addr; //Self-addressed ptr
  I64 num,cpu_flags;
  I64 startup_rip;
  I64 idle_pt_hits;
  F64 idle_factor;
  I64 total_jiffies;
  CTask *seth_task,*idle_task;
  I64 tr;   //task register
  I64 swap_cnter;
  U0 (*timer_irq_chain)(CTask *task);
  CTask *dying_task_lst;
  CTSS *tss;
  U8 wallpaper_data[CPU_WALLPAPER_DATA_SIZE]; //doubles as start-up stk
  I64 start_stk[8];
U0 pad;#assert !($$&7) //(compiler lags one token)
};

#define KDF_HAS_DESCS	1

class CKeyDev
{
  CKeyDev *next,*last;
  I64 priority,flags;
  Bool (*put_key)(I64 ch,I64 sc);
  Bool (*put_s)(U8 *st);
};

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

#define CREG_RAX	0
#define CREG_RCX	1
#define CREG_RDX	2
#define CREG_RBX	3
#define CREG_RSP	4
#define CREG_RBP	5
#define CREG_RSI	6
#define CREG_RDI	7
#define CREG_RIP	16	//Used by compiler, not really it's num
//Be careful: RBPu8, RSPu8, RSIu8, RDIu8 are 20-24
#define CREG_NONE	32	//noreg flag sets it to this
#define CREG_ALLOC	33	//reg flag sets it to this
#define CREG_UNDEF	MIN_I8

#define CALWAYS_CLOBBERED_MASK		0x003F //RAX,RCX,RDX,RBX,RBP,RSP
#define CSTK_TEMPS_MASK			0x0300 //R8,R9
#define CREG_VARS_MASK			0xCCC0 //RSI,RDI,R10,R11,R14,R15
#define CREG_NON_PTR_MASK		0x3000 //R12,R13

#define PUSH_C_REGS PUSH RAX PUSH RCX PUSH RDX PUSH RBX PUSH R8 PUSH R9
#define POP_C_REGS POP R9 POP R8 POP RBX POP RDX POP RCX POP RAX

#define PUSH_REGS PUSH RAX PUSH RCX PUSH RDX PUSH RBX PUSH RBP PUSH RSI \
PUSH RDI PUSH R8 PUSH R9 PUSH R10 PUSH R11 PUSH R12 PUSH R13 PUSH R14 PUSH R15
#define POP_REGS POP R15 POP R14 POP R13 POP R12 POP R11 POP R10 POP R9 \
POP R8 POP RDI POP RSI POP RBP POP RBX POP RDX POP RCX POP RAX

#define MAPPED_MEM_SPACE 0x2000000000 //Arbitrarily set to 128 Gig
#define PAGE_TABLE_BASE	0x100000
#define NUM_PML1	1 //First 2 Meg are 4K resolution
#define NUM_PML2	(MAPPED_MEM_SPACE>>(9+21))
#define NUM_PML3	1
#define NUM_PML4	1
#define PAGE_TABLE_SIZE (NUM_PML1+NUM_PML2+NUM_PML3+NUM_PML4)*0x1000
#define PROTECTED_LOW_PAGE_LIMIT	0x6FFF

#define ST_ERR_ST   "$$FG,LTRED$$$$BK,1$$ERROR:$$FG$$$$BK,0$$ "
#define ST_WARN_ST  "$$FG,RED$$$$BK,1$$WARNING:$$FG$$$$BK,0$$ "

class CSndData
{
  CSndData *next,*last;
  F64 freq; //Hz
  F64 time_stamp;
};

//sys_raw_flags flags
#define RWF_IN_DOLLAR	1
#define RWF_LAST_DOLLAR	2
#define RWF_SHOW_DOLLAR	4
#define RWF_SCROLL	8

//Input ptr types
#define IP_NULL		0
#define IP_MOUSE	1

#define PSECT_CACHE_SIZE	256
class CPSectCache
{
  I64 base,limit;
  F64 time_stamp;
  U8 str[1]; //PSECT_CACHE_STR_LEN
  $$=64;
};
#define PSECT_CACHE_STR_LEN	(sizeof(CPSectCache)-offset(CPSectCache.str))

