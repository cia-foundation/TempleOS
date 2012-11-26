/*
In this file, syms defined OSMain assembly
files are given C+ definitions.  This file
is included when you compile the kernel, after
the assembly files have been assembled, making
the C+ definitions available for the C+ OSMain
files.	It is also included as part of booting.

The assembly global labels are stored as HTT_EXPORT_SYS_SYM
entries in the hash table.  These symols are placed
into the OSMain binary module and added to the
sym hash table at boot before this file
gets #included during boot.

See
$LK,"Adam1b","FF:::/SparrowOS/OSMain/OSMain.PRJ.Z,Adam1b"$ for where it is included during compiling the kernel.
$LK,"Adam1b","FF:::/SparrowOS/OSMain/Adam1.CPP.Z,Adam1b"$ for where it is included during boots.
$LK,"LoadOSMain","FF:::/SparrowOS/OSMain/OSEnd.CPP.Z,LoadOSMain"$ for where the OSMain syms are added to the hash tables.

*/

#help_index "Misc;Messages"
public _extern SYS_FOCUS_TASK CTask *sys_focus_task;

#help_index "Graphics/Screen"
//This is the INT 0x10 mode number.
public _extern SYS_BIOS_GR_MODE U16 bios_gr_mode;
#help_index "PCI"
public _extern SYS_PCI_BUSSES U16 sys_pci_busses;
#help_index "Misc"
public _extern SYS_CTRL_ALT_FLAGS I64 sys_ctrl_alt_flags[1];
public _extern SYS_COMPILE_TIME CDate sys_compile_time;

#help_index "Boot"
_extern SYS_BOOT_BASE U32 sys_boot_base;
_extern SYS_BOOT_CODE U32 sys_boot_code;
_extern SYS_RUN_LEVEL U32 sys_run_level;
_extern SYS_BOOT_PATCH_TABLE_BASE U32 sys_boot_patch_table_base;

#help_index "Misc/Progress Bars"
public _extern SYS_PROGRESSES CProgress sys_progresses[NUM_PROGRESS_BARS];
public _extern SYS_PROGRESS1	   I64 progress1;
public _extern SYS_PROGRESS1_MAX  I64 progress1_max;
public _extern SYS_PROGRESS1_DESC U8  progress1_desc[PROGRESS_DESC_LEN+1];
public _extern SYS_PROGRESS2	   I64 progress2;
public _extern SYS_PROGRESS2_MAX  I64 progress2_max;
public _extern SYS_PROGRESS2_DESC U8  progress2_desc[PROGRESS_DESC_LEN+1];
public _extern SYS_PROGRESS3	   I64 progress3;
public _extern SYS_PROGRESS3_MAX  I64 progress3_max;
public _extern SYS_PROGRESS3_DESC U8  progress3_desc[PROGRESS_DESC_LEN+1];
public _extern SYS_PROGRESS4	   I64 progress4;
public _extern SYS_PROGRESS4_MAX  I64 progress4_max;
public _extern SYS_PROGRESS4_DESC U8  progress4_desc[PROGRESS_DESC_LEN+1];
 
#help_index "Bit"
#help_file "::/SparrowOS/Doc/Bit"
public _intern IC_BT Bool Bt(U8 *bit_field,I64 bit);
public _intern IC_BTS Bool Bts(U8 *bit_field,I64 bit);
public _intern IC_BTR Bool Btr(U8 *bit_field,I64 bit);
public _intern IC_BTC Bool Btc(U8 *bit_field,I64 bit);
public _intern IC_BEQU Bool BEqu(U8 *bit_field,I64 bit,Bool val);

//Locked for (for multitple CPU's)
public _intern IC_LBTS Bool LBts(U8 *bit_field,I64 bit);
public _intern IC_LBTR Bool LBtr(U8 *bit_field,I64 bit);
public _intern IC_LBTC Bool LBtc(U8 *bit_field,I64 bit);
public _intern IC_LBEQU Bool LBEqu(U8 *bit_field,I64 bit,Bool val);

//Scan I64 sized item fwd for first set bit, starting at lowest.
//Return -1 if not found.
public _intern IC_BSF I64 Bsf(I64 bit_field_val);

//Scan I64 sized item reverse for first set bit, starting at highest.
//Return -1 if not found.
public _intern IC_BSR I64 Bsr(I64 bit_field_val);

public _extern _BIT_FIELD_OR_U32 U0 BFieldOrU32(
	 U8 *bit_field,I64 bit,U32 pattern);
public _extern _BIT_FIELD_EXTRACT_U32 U32 BFieldExtU32(
	 U8 *bit_field,I64 bit,I64 size);

#help_index "Circular Queue"
#help_file "::/SparrowOS/Doc/Que"
public _intern IC_QUE_INIT U0 QueInit(CQue *root);
//Ins item into que after predicessor.
public _intern IC_QUE_INS U0 QueIns(CQue *entry,CQue *pred);
//Reversed insert into que.  Ins
//item into que before successor.
public _intern IC_QUE_INS_REV U0 QueInsRev(CQue *entry,CQue *succ);
//Remove item from que.
public _intern IC_QUE_REM U0 QueRem(CQue *entry);

#help_index "Task"
public _intern IC_PREEMPT Bool Preempt(Bool val);

#help_index "I/O;Processor/IO Port"
public _intern IC_INU8	I64 InU8(I64 port);
public _intern IC_INU16 I64 InU16(I64 port);
public _intern IC_INU32 I64 InU32(I64 port);
public _intern IC_OUTU8  U0 OutU8(I64 port, I64 val);
public _intern IC_OUTU16 U0 OutU16(I64 port, I64 val);
public _intern IC_OUTU32 U0 OutU32(I64 port, I64 val);
public _extern _REP_IN_U8  U0 RepInU8(U8 *buf,I64 cnt,I64 port);
public _extern _REP_IN_U16 U0 RepInU16(U8 *buf,I64 cnt,I64 port);
public _extern _REP_IN_U32 U0 RepInU32(U8 *buf,I64 cnt,I64 port);
public _extern _REP_OUT_U8  U0 RepOutU8(U8 *buf,I64 cnt,I64 port);
public _extern _REP_OUT_U16 U0 RepOutU16(U8 *buf,I64 cnt,I64 port);
public _extern _REP_OUT_U32 U0 RepOutU32(U8 *buf,I64 cnt,I64 port);

#help_index "Memory"

//Only goes fwd.
public _intern IC_MEMCPY    U0	MemCpy(U8 *dst,U8 *src,I64 cnt);
public _intern IC_MEMSET    U8 *MemSet(U8 *dst,I64 val,I64 cnt);
public _intern IC_MEMSET    U8 *MemSetU8(U8 *dst,I64 val,I64 U8cnt);
public _intern IC_MEMSET_U16 U16 *MemSetU16(U8 *dst,I64 val,I64 U16cnt);
public _intern IC_MEMSET_U32 U32 *MemSetU32(U8 *dst,I64 val,I64 U32cnt);
public _intern IC_MEMSET_U64 U64 *MemSetU64(U8 *dst,I64 val,I64 U64cnt);
public _extern SYS_CODE_BP CBlkPool *sys_code_bp;
public _extern SYS_DATA_BP CBlkPool *sys_data_bp;
public _extern SYS_HEAP_BASE U32 sys_heap_base;
public _extern SYS_HEAP_LIMIT U32 sys_heap_limit;

_extern SYS_HEAP_INIT_FLAG	U8 sys_heap_init_flag;
_extern SYS_HEAP_INIT_VAL	U8 sys_heap_init_val;
_extern SYS_MEM_INIT_FLAG	U8 sys_mem_init_flag;
_extern SYS_MEM_INIT_VAL	U8 sys_mem_init_val;
_extern SYS_VAR_INIT_FLAG	U8 sys_var_init_flag;
_extern SYS_VAR_INIT_VAL	U8 sys_var_init_val;
_extern SYS_HEAP_DBG_FLAG	U8 sys_heap_dbg;

#help_index "Char/Conversion"
public _intern IC_TOUPPER I64 ToUpper(U8 ch);
#help_index "Char/Operations"
public _intern IC_STRLEN I64 StrLen(U8 *st);

#help_index "Processor"
public _extern _CPUID U0 CPUId(U32 rax,CRAXRBCRCXRDX *result);
public _intern IC_SET_RAX U0 SetRAX(I64 d);
public _intern IC_GET_RAX I64 GetRAX();
public _intern IC_CLI U0 Cli();
public _intern IC_STI U0 Sti();
public _intern IC_PUSHFD U0 PushFD();
public _intern IC_POPFD U0 PopFD();
public _intern IC_HLT U0 Hlt();
public _extern _SYS_HLT U0 SysHlt(); //Loops

//This instruction hints to the processor
//that a spin-loop is taking place.
public _intern IC_PAUSE U0 Pause();

public _intern IC_POP I64 Pop();
public _intern IC_PUSH U0 Push(I64 d);
public _intern IC_CLD U0 Cld();
public _intern IC_STD U0 Std();
public _intern IC_GET_FLAGS I64 GetFlags();
public _intern IC_SET_FLAGS U0 SetFlags(I64 d);
public _intern IC_GET_STK_PTR U8 *GetStkPtr();
public _intern IC_SET_STK_PTR U0 SetStkPtr(U8 *d);
public _intern IC_GET_BASE_PTR U8 *GetBasePtr();
public _intern IC_SET_BASE_PTR U0 SetBasePtr(U8 *d);
public _extern SYS_CACHE_LINE_WIDTH I64 sys_cache_line_width;
_extern SYS_GDTTAB U8 sys_gdttab[GDT_SIZE];
public _extern _XCHG_U64 U64 XchgU64(U64 *dst,U64 d);
public _extern _XCHG_U32 U64 XchgU32(U32 *dst,U32 d);
public _extern _XCHG_U16 U64 XchgU16(U16 *dst,U16 d);
public _extern _XCHG_U8  U64 XchgU8 (U8  *dst,U8 d);
public _extern _LXCHG_U64 U64 LXchgU64(U64 *dst,U64 d);
public _extern _LXCHG_U32 U64 LXchgU32(U32 *dst,U32 d);
public _extern _LXCHG_U16 U64 LXchgU16(U16 *dst,U16 d);
public _extern _LXCHG_U8  U64 LXchgU8(U8 *dst,U8 d);

//Model Specific REgisters
//See $LK,"MSRs","MN:IA32_EFER"$
public _extern _SET_MSR U0 SetMSR(I64 model_specific_reg,I64 val);

#help_index "Processor/Cache;Memory/Cache"
public _intern IC_INVLPG U0 InvlPg(U8 *a);
//Flush cache line.
public _intern IC_CLFLUSH U0 CLFlush(U8 *a);

#help_index "MultiCore"
#help_file "::/SparrowOS/Doc/MultiCore"
_extern _SET_GS_BASE U0 SetGs(CCPU *c);
public _extern SYS_MP_CNT I64 mp_cnt;
_extern SYS_MP_CNT_INITIAL I64 mp_cnt_initial; //only used during $LK,"MPStart","MN:MPStart"$
_extern SYS_MP_CNT_LOCK I64 mp_cnt_lock;
_extern SYS_TEMP_CPU0_STRUCT CCPU sys_temp_cpu0_struct;
public _extern SYS_CPU_STRUCTS	 CCPU *cpu_structs;
_extern SYS_SEMAS CSema sys_semas[NUM_SYS_SEMAS];

#help_index "MultiCore;Processor"
//This pts to the current CPU structure.
//It returns the val in the GS segment register.
public _intern IC_GS CCPU *Gs();

#help_index "Time/CPU Cycles"
public _intern IC_RDTSC I64 GetTimeStamp();

#help_index "Time/HPET"
public _intern IC_HPET I64 HPET();

#help_index "Time/Jiffies"
#help_file "::/SparrowOS/Doc/TimeJiffy"
_extern SYS_TIMER I64 sys_timer;
public _extern SYS_JIFFIES I64 sys_jiffies;

#help_index "Math"
public _intern IC_TO_I64 I64 ToI64(F64 d);  //This truncates
public _intern IC_TO_F64 F64 ToF64(I64 i);
public _intern IC_TO_BOOL Bool ToBool(I64 i);
public _intern IC_ABS_I64 I64 AbsI64(I64 i);
public _intern IC_LIMIT_I64 I64 LimitI64(I64 p,I64 lo,I64 hi);
public _intern IC_LIMIT_U64 U64 LimitU64(U64 p,U64 lo,U64 hi);
public _intern IC_SIGN_I64 I64 SignI64(I64 i);
public _intern IC_SQR_I64  I64 SqrI64(I64 i);
public _intern IC_SQR_U64  U64 SqrU64(U64 i);
public _intern IC_SWAP_U8  U0 SwapU8(U8 *n1,U8 *n2);
public _intern IC_SWAP_U16 U0 SwapU16(U16 *n1,U16 *n2);
public _intern IC_SWAP_U32 U0 SwapU32(U32 *n1,U32 *n2);
public _intern IC_SWAP_U64 U0 SwapU64(U64 *n1,U64 *n2);
public _intern IC_MIN_I64 I64 MinI64(I64 n1,I64 n2);
public _intern IC_MAX_I64 I64 MaxI64(I64 n1,I64 n2);
public _intern IC_MIN_U64 U64 MinU64(U64 n1,U64 n2);
public _intern IC_MAX_U64 U64 MaxU64(U64 n1,U64 n2);
public _intern IC_MOD_U64 U64 ModU64(U64 *q,U64 d);
public _intern IC_SQR F64 Sqr(F64 d);
public _intern IC_ABS F64 Abs(F64 d);
public _intern IC_SQRT F64 Sqrt(F64 d);
public _intern IC_SIN F64 Sin(F64 d);
public _intern IC_COS F64 Cos(F64 d);
public _intern IC_TAN F64 Tan(F64 d);
public _intern IC_ATAN F64 ATan(F64 d);

public _extern _SIGN F64 Sign(F64 d);
public _extern _ARG F64 Arg(F64 x,F64 y);
public _extern _ROUND F64 Round(F64 d);
public _extern _TRUNC F64 Trunc(F64 d);
public _extern _FLOOR F64 Floor(F64 d);
public _extern _CEIL F64 Ceil(F64 d);
public _extern _POW F64 Pow(F64 base,F64 power);
public _extern _POW10 F64 Pow10(F64 d);
public _extern _LOG2 F64 Log2(F64 d);
public _extern _LOG10 F64 Log10(F64 d);
public _extern _LN  F64 Ln(F64 d);
public _extern _EXP F64 Exp(F64 d);
_extern _FLDCW U0 Fldcw(U16 w);
_extern _FCLEX U0 Fclex();
_extern _FSTCW U16 Fstcw();
_extern _FSTSW U16 Fstsw();
_extern _FXSAVE  U0 Fxsave(CFPU *fpu_mmx);
_extern _FXRSTOR U0 Fxrstor(CFPU *fpu_mmx);
_extern SYS_INIT_FPU_MMX CFPU sys_init_fpu_mmx;

#help_index "Math/CD3;ODE/CD3"
#help_file "::/SparrowOS/Doc/D3"
public _extern _D3_EQU CD3 *D3Equ(CD3 *dst,F64 x,F64 y,F64 z);
public _extern _D3_ZERO CD3 *D3Zero(CD3 *dst);
public _extern _D3_COPY CD3 *D3Copy(CD3 *dst,CD3 *src);
public _extern _D3_NORM F64 D3Norm(CD3 *d);
public _extern _D3_SQR_NORM F64 D3SqrNorm(CD3 *d);
public _extern _D3_ADD CD3 *D3Add(CD3 *d1,CD3 *d2,CD3 *sum);
public _extern _D3_SUB CD3 *D3Sub(CD3 *d1,CD3 *d2,CD3 *diff);
public _extern _D3_MUL CD3 *D3Mul(F64 c,CD3 *d,CD3 *prod);
public _extern _D3_DIV CD3 *D3Div(F64 c,CD3 *d,CD3 *quot);
public _extern _D3_ADD_EQU CD3 *D3AddEqu(CD3 *sum,CD3 *d);
public _extern _D3_SUB_EQU CD3 *D3SubEqu(CD3 *diff,CD3 *d);
public _extern _D3_MUL_EQU CD3 *D3MulEqu(CD3 *prod,F64 c);
public _extern _D3_DIV_EQU CD3 *D3DivEqu(CD3 *quot,F64 c);
public _extern _D3_DIST F64 D3Dist(CD3 *d1,CD3 *d2);
public _extern _D3_SQR_DIST F64 D3SqrDist(CD3 *d1,CD3 *d2);
//Dot Product
public _extern _D3_DOT F64 D3Dot(CD3 *d1,CD3 *d2);
//Cross Product
public _extern _D3_CROSS CD3 *D3Cross(CD3 *d1,CD3 *d2,CD3 *prod);
//Convert to unit vect
public _extern _D3_UNIT CD3 *D3Unit(CD3 *d);


#help_index "Task;Processor"
//This pts to the current task record.
//It returns the val in the FS segment register.
public _intern IC_FS CTask *Fs();

#help_index "Call"
public _extern _CALL I64 Call(U8 *s);

//fp_add can have any fixed number of arguments.
public _extern _CALLIND I64 CallInd(/*argpop*/I64 (*fp_add)(...),...);

//This is just an array of ptrs to funs.
//See $LK,"Extern Num Definitions","MN:EXT_TABLE_SIZE"$
public _extern SYS_EXTERN_TABLE /*argpop*/ I64 (**ext)(...);
//Can have any fixed number of arguments.
public _extern _CALLEXTNUM I64 CallExtNum(I64 num,...);

//Search sym table for fun name and pass it args.
//Can have any fixed number of arguments.
public _extern _CALLEXTSTR I64 CallExtStr(/*argpop*/ U8 *name,...);

#help_index ""
