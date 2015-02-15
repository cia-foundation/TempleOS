/*  Kernel ASM symbols.
$LK,"KernelB",A="FF:::/Kernel/Kernel.PRJ,KernelB"$ Kernel.PRJ
$LK,"KernelB",A="FF:::/Compiler/Compiler.PRJ,KernelB"$ Compiler.PRJ
$LK,"KernelB",A="FF:::/Adam.CPP,KernelB"$ Adam.CPP
*/

#help_index "Bit"
#help_file "::/Doc/Bit"
public _intern IC_BSF I64 Bsf(I64 bit_field_val);//Scan fwd from lowest for 1st set. -1 if not found.
public _intern IC_BSR I64 Bsr(I64 bit_field_val);//Scan rev from highest for 1st set. -1 if not found.
public _intern IC_BT Bool Bt(U8 *bit_field,I64 bit); //Bit test.
public _intern IC_BTC Bool Btc(U8 *bit_field,I64 bit); //Bit test and complement (same as xor with 1).
public _intern IC_BTR Bool Btr(U8 *bit_field,I64 bit); //Bit test and reset to zero.
public _intern IC_BTS Bool Bts(U8 *bit_field,I64 bit); //Bit test and set to one.
public _intern IC_LBTC Bool LBtc(U8 *bit_field,I64 bit); //Locked Bit test and complement (same as xor with 1).
public _intern IC_LBTR Bool LBtr(U8 *bit_field,I64 bit); //Locked Bit test and reset to zero.
public _intern IC_LBTS Bool LBts(U8 *bit_field,I64 bit); //Locked Bit test and set to one.
public _extern _BEQU Bool BEqu(U8 *bit_field,I64 bit,Bool val);//Set bit equal to val.
public _extern _BIT_FIELD_EXTRACT_U32 U32 BFieldExtU32(U8 *bit_field,I64 bit,I64 size); //Extract U32 from bit field.
public _extern _BIT_FIELD_OR_U32 U0 BFieldOrU32(U8 *bit_field,I64 bit,U32 pattern); //Or U32 into bit field.
public _extern _LBEQU Bool LBEqu(U8 *bit_field,I64 bit,Bool val); //Locked Set bit equal to val.

#help_index "Boot"
_extern SYS_BOOT_BASE U32 sys_boot_base;
_extern SYS_BOOT_PATCH_TABLE_BASE U32 sys_boot_patch_table_base;
_extern SYS_BOOT_SRC U32 sys_boot_src;
_extern SYS_RUN_LEVEL U32 sys_run_level;

#help_index "Call"
public _extern _CALL I64 Call(U8 *s); //Call addr with no args.
public _extern _CALLEXTSTR I64 CallExtStr(/*argpop*/ U8 *name,...); //Search sym table for fun name and pass it args.
//Can have any fixed number of arguments.
public _extern _CALLIND I64 CallInd(/*argpop*/I64 (*fp_addr)(...),...); //Call with any fixed number of arguments.

#help_index "Call/System Extern Num"
public _extern SYS_EXTERN_TABLE /*argpop*/ I64 (**ext)(...); //Array of fun ptrs.  See $LK,"Extern Num Definitions",A="MN:EXT_NUM_ENTRIES"$
public _extern _CALLEXTNUM I64 CallExtNum(I64 num,...); //Call ext num with any fixed number of args.

#help_index "Char/Conversion"
public _intern IC_TOUPPER I64 ToUpper(U8 ch); //Cvt char to upper case.

#help_index "Char/Operations"
public _intern IC_STRLEN I64 StrLen(U8 *st); //String length.

#help_index "Data Types/Circular Queue"
#help_file "::/Doc/Que"
public _intern IC_QUE_INIT U0 QueInit(CQue *root); //Init queue root links.
public _intern IC_QUE_INS U0 QueIns(CQue *entry,CQue *pred);//Ins item into que after predecessor.
public _intern IC_QUE_INS_REV U0 QueInsRev(CQue *entry,CQue *succ);//Reversed insert into que.
//Ins item into que before successor.
public _intern IC_QUE_REM U0 QueRem(CQue *entry); //Remove Item from queue.

#help_index "I/O;Processor/IO Port"
public _intern IC_IN_U16 I64 InU16(I64 port); //Read U16 from I/O port.
public _intern IC_IN_U32 I64 InU32(I64 port); //Read U32 from I/O port.
public _intern IC_IN_U8 I64 InU8(I64 port); //Read U8 from I/O port.
public _intern IC_OUT_U16 U0 OutU16(I64 port, I64 val); //Write U16 to I/O port.
public _intern IC_OUT_U32 U0 OutU32(I64 port, I64 val); //Write U32 to I/O port.
public _intern IC_OUT_U8  U0 OutU8(I64 port, I64 val); //Write U8 to I/O port.
public _extern _REP_IN_U16 U0 RepInU16(U8 *buf,I64 cnt,I64 port); //Repeated read U16 from I/O port.
public _extern _REP_IN_U32 U0 RepInU32(U8 *buf,I64 cnt,I64 port); //Repeated read U32 from I/O port.
public _extern _REP_IN_U8  U0 RepInU8(U8 *buf,I64 cnt,I64 port); //Repeated read U8 from I/O port.
public _extern _REP_OUT_U16 U0 RepOutU16(U8 *buf,I64 cnt,I64 port); //Repeated write U16 to I/O port.
public _extern _REP_OUT_U32 U0 RepOutU32(U8 *buf,I64 cnt,I64 port); //Repeated write U32 to I/O port.
public _extern _REP_OUT_U8  U0 RepOutU8(U8 *buf,I64 cnt,I64 port); //Repeated write U8 to I/O port.

#help_index "Math"
public _intern IC_ATAN F64 ATan(F64 d); //Arc Tan (Inverse Tan).
public _intern IC_ABS F64 Abs(F64 d); //Absolute F64.
public _intern IC_ABS_I64 I64 AbsI64(I64 i); //Absolute I64.
public _intern IC_COS F64 Cos(F64 d); //Cosine.
public _intern IC_MAX_I64 I64 MaxI64(I64 n1,I64 n2); //Max of two I64s.
public _intern IC_MAX_U64 U64 MaxU64(U64 n1,U64 n2); //Max of two U64s.
public _intern IC_MIN_I64 I64 MinI64(I64 n1,I64 n2); //Min of two I64s.
public _intern IC_MIN_U64 U64 MinU64(U64 n1,U64 n2); //Min of two U64s.
public _intern IC_MOD_U64 U64 ModU64(U64 *q,U64 d); //Divide and return U64 Modulo.
public _intern IC_SIGN_I64 I64 SignI64(I64 i); //Sign of I64: -1,0,1.
public _intern IC_SIN F64 Sin(F64 d); //Sine.
public _intern IC_SQR F64 Sqr(F64 d); //Square of F64.
public _intern IC_SQR_I64  I64 SqrI64(I64 i); //Square of I64.
public _intern IC_SQR_U64  U64 SqrU64(U64 i); //Square of U64.
public _intern IC_SQRT F64 Sqrt(F64 d); //Square root of F64.
public _intern IC_SWAP_I64 U0 SwapI64(I64 *n1,I64 *n2); //Swap two I64s. Not atomic.
public _intern IC_SWAP_U16 U0 SwapU16(U16 *n1,U16 *n2); //Swap two U16s. Not atomic.
public _intern IC_SWAP_U32 U0 SwapU32(U32 *n1,U32 *n2); //Swap two U32s. Not atomic.
public _intern IC_SWAP_U8  U0 SwapU8(U8 *n1,U8 *n2); //Swap two U8s. Not atomic.
public _intern IC_TAN F64 Tan(F64 d); //Tangent.
public _intern IC_TO_BOOL Bool ToBool(I64 i); //Convert to Boolean.
public _intern IC_TO_F64 F64 ToF64(I64 i); //Convert to F64.
public _intern IC_TO_I64 I64 ToI64(F64 d);  //Convert to I64. Truncates.
public _extern _ARG F64 Arg(F64 x,F64 y); //Polar coordinate angle.
public _extern _CEIL F64 Ceil(F64 d); //Ceiling of F64.
public _extern _CLAMP_I64 I64 ClampI64(I64 dv,I64 lo,I64 hi); //Clamp to I64 [] range.
public _extern _CLAMP_U64 U64 ClampU64(U64 dv,U64 lo,U64 hi); //Clamp to U64 [] range.
public _extern _EXP F64 Exp(F64 d); //Exponential function.
_extern _FCLEX U0 Fclex();
_extern _FLDCW U0 Fldcw(U16 w);
public _extern _FLOOR F64 Floor(F64 d); //Floor of F64.
_extern _FSTCW U16 Fstcw();
_extern _FSTSW U16 Fstsw();
_extern _FXRSTOR U0 Fxrstor(CFPU *fpu_mmx);
_extern _FXSAVE  U0 Fxsave(CFPU *fpu_mmx);
public _extern _LN  F64 Ln(F64 d); //Logarithm.
public _extern _LOG10 F64 Log10(F64 d); //Log base 10.
public _extern _LOG2 F64 Log2(F64 d); //Log base 2.
public _extern _POW F64 Pow(F64 base,F64 power); //F64 base to a power.
public _extern _POW10 F64 Pow10(F64 d); //Ten to the dth power.
public _extern _ROUND F64 Round(F64 d); //Round F64 to whole number.
public _extern _SIGN F64 Sign(F64 d); //Sign of F64: -1,0,1
public _extern _TRUNC F64 Trunc(F64 d); //Truncate F64.

#help_index "Math/CD3"
#help_file "::/Doc/D3"
public _extern _D3_ADD CD3 *D3Add(CD3 *d1,CD3 *d2,CD3 *sum); //d1+d2 -->sum
public _extern _D3_ADD_EQU CD3 *D3AddEqu(CD3 *sum,CD3 *d); //sum+=d
public _extern _D3_COPY CD3 *D3Copy(CD3 *dst,CD3 *src); //dst=src
public _extern _D3_CROSS CD3 *D3Cross(CD3 *d1,CD3 *d2,CD3 *prod); //d1 x d2 -->prod
public _extern _D3_DIST F64 D3Dist(CD3 *d1,CD3 *d2); //Dist between d1 & d2
public _extern _D3_DIST_SQR F64 D3DistSqr(CD3 *d1,CD3 *d2); //Dist Squared between d1 & d2
public _extern _D3_DIV CD3 *D3Div(F64 s,CD3 *d,CD3 *quot); //1/s * d -->quot
public _extern _D3_DIV_EQU CD3 *D3DivEqu(CD3 *quot,F64 s); //quot *= 1/s
public _extern _D3_DOT F64 D3Dot(CD3 *d1,CD3 *d2); //d1 dot d2
public _extern _D3_EQU CD3 *D3Equ(CD3 *dst,F64 x,F64 y,F64 z); // dst=x,y,z
public _extern _D3_MUL CD3 *D3Mul(F64 s,CD3 *d,CD3 *prod); //s * d -->prod
public _extern _D3_MUL_EQU CD3 *D3MulEqu(CD3 *prod,F64 s); //prod *= s
public _extern _D3_NORM F64 D3Norm(CD3 *d); //Norm d
public _extern _D3_NORM_SQR F64 D3NormSqr(CD3 *d); //Norm Squared d
public _extern _D3_SUB CD3 *D3Sub(CD3 *d1,CD3 *d2,CD3 *diff); //d1-d2 -->diff
public _extern _D3_UNIT CD3 *D3Unit(CD3 *d); //Convert to unit vect
public _extern _D3_SUB_EQU CD3 *D3SubEqu(CD3 *diff,CD3 *d); //diff-=d
public _extern _D3_ZERO CD3 *D3Zero(CD3 *dst); //Convert to zero

#help_index "Memory"
public _extern _MEMCMP I64 MemCmp(U8 *ptr1,U8 *ptr2,I64 cnt); //Compare chunk of memory.
public _extern _MEMCPY U8 *MemCpy(U8 *dst,U8 *src,I64 cnt); //Copy chunk of memory. Only goes fwd.
public _extern _MEMSET U8 *MemSet(U8 *dst,I64 val,I64 cnt); //Set chunk of U8s to value.
public _extern _MEMSET_I64 I64 *MemSetI64(I64 *dst,I64 val,I64 I64cnt); //Set chunk of I64s to value.
public _extern _MEMSET_U16 U16 *MemSetU16(U16 *dst,I64 val,I64 U16cnt); //Set chunk of U16s to value.
public _extern _MEMSET_U32 U32 *MemSetU32(U32 *dst,I64 val,I64 U32cnt); //Set chunk of U32s to value.
public _extern _MEMSET U8 *MemSetU8(U8 *dst,I64 val,I64 U8cnt); //Set chunk of U8s to value.
_extern SYS_HEAP_DBG_FLAG	U8 sys_heap_dbg;
_extern SYS_HEAP_INIT_FLAG	U8 sys_heap_init_flag;
_extern SYS_HEAP_INIT_VAL	U8 sys_heap_init_val;
_extern SYS_MEM_INIT_FLAG	U8 sys_mem_init_flag;
_extern SYS_MEM_INIT_VAL	U8 sys_mem_init_val;
_extern SYS_VAR_INIT_VAL	U8 sys_var_init_val;
_extern SYS_VAR_INIT_FLAG	U8 sys_var_init_flag;

#help_index "Memory/BlkPool"
public _extern SYS_CODE_BP CBlkPool *sys_code_bp; //System's BlkPool for code.
public _extern SYS_DATA_BP CBlkPool *sys_data_bp; //System's BlkPool for data.

#help_index "Memory/Info"
_extern SYS_HEAP_BASE	U8 *sys_heap_base;
_extern SYS_HEAP_LIMIT	U8 *sys_heap_limit;

#help_index "Misc"
public _extern SYS_COMPILE_TIME CDate sys_compile_time; //Date and time kernel was compiled.
_extern SYS_CTRL_ALT_FLAGS I64 sys_ctrl_alt_flags[1]; //Should be semiphores?

#help_index "Misc/Progress Bars"
public _extern SYS_PROGRESS1 I64 progress1; //Current progress 1.
public _extern SYS_PROGRESS1_DESC U8  progress1_desc[PROGRESS_DESC_LEN+1]; //Progress 1 desc.
public _extern SYS_PROGRESS1_MAX I64 progress1_max; //Max progress 1.
public _extern SYS_PROGRESS2 I64 progress2; //Current progress 2.
public _extern SYS_PROGRESS2_DESC U8  progress2_desc[PROGRESS_DESC_LEN+1]; //Progress 2 desc.
public _extern SYS_PROGRESS2_MAX I64 progress2_max; //Max progress 2.
public _extern SYS_PROGRESS3 I64 progress3; //Current progress 3.
public _extern SYS_PROGRESS3_DESC U8  progress3_desc[PROGRESS_DESC_LEN+1]; //Progress 3 desc.
public _extern SYS_PROGRESS3_MAX I64 progress3_max; //Max progress 3.
public _extern SYS_PROGRESS4 I64 progress4; //Current progress 4.
public _extern SYS_PROGRESS4_DESC U8  progress4_desc[PROGRESS_DESC_LEN+1]; //Progress 4 desc.
public _extern SYS_PROGRESS4_MAX I64 progress4_max; //Max progress 4.
public _extern SYS_PROGRESSES CProgress sys_progresses[NUM_PROGRESS_BARS]; //All progress bars.

#help_index "Misc;Messages"
public _extern SYS_FOCUS_TASK CTask *sys_focus_task; //Current focus task.

#help_index "MultiCore"
#help_file "::/Doc/MultiCore"
public _extern SYS_CPU_STRUCTS CCPU *cpu_structs; //$LK,"Gs",A="MN:Gs"$ points to cur $LK,"CCPU",A="MN:CCPU"$.
public _extern SYS_MP_CNT I64 mp_cnt; //Count of cores
_extern SYS_MP_CNT_INITIAL I64 mp_cnt_initial; //only used during $LK,"BSPStartMP",A="MN:BSPStartMP"$
_extern SYS_MP_CNT_LOCK I64 mp_cnt_lock;
_extern SYS_SEMAS CSema sys_semas[NUM_SYS_SEMAS];

#help_index "MultiCore;Processor"
public _intern IC_GS CCPU *Gs(); //GS points to current $LK,"CCPU",A="MN:CCPU"$.

#help_index "PCI"
public _extern SYS_PCI_BUSSES U16 sys_pci_busses; //Number of PCI busses.

#help_index "Processor"
public _intern IC_CARRY I64 Carry(); //See $LK,"::/Demo/Carry.CPP"$
public _intern IC_GET_RAX I64 GetRAX(); //Get RAX register value.
public _intern IC_GET_RBP U8 *GetRBP(); //Get RBP register value.
public _intern IC_GET_RFLAGS I64 GetRFlags(); //Get RFlags register value.
public _intern IC_GET_RSP U8 *GetRSP(); //Get RSP register value.
public _intern IC_POP I64 Pop(); //Pop value from stk.
public _intern IC_PUSH U0 Push(I64 d); //Push value on stk.
public _intern IC_SET_RAX U0 SetRAX(I64 d); //Set RAX register value.
public _intern IC_SET_RBP U0 SetRBP(U8 *d); //Set RBP register value.
public _intern IC_SET_RFLAGS U0 SetRFlags(I64 d); //Set RFlags register value.
public _intern IC_SET_RSP U0 SetRSP(U8 *d); //Set RSP register value.
public _extern _CPUID U0 CPUId(U32 rax,CRAXRBCRCXRDX *result); //Do CPUID instruction.
public _extern _LXCHG_I64 I64 LXchgI64(I64 *dst,I64 d); //Locked eXchange I64s.
public _extern _LXCHG_U16 U64 LXchgU16(U16 *dst,U16 d); //Locked eXchange U16s.
public _extern _LXCHG_U32 U64 LXchgU32(U32 *dst,U32 d); //Locked eXchange U32s.
public _extern _LXCHG_U8  U64 LXchgU8(U8 *dst,U8 d); //Locked eXchange U8s.
public _extern _SET_MSR U0 SetMSR(I64 model_specific_reg,I64 val); //Model Specific Reg See $LK,"MSRs",A="MN:IA32_EFER"$.
public _extern _SYS_HLT U0 SysHlt(); //Loops doing HLT instruction.
public _extern _XCHG_I64 I64 XchgI64(I64 *dst,I64 d); //eXchange I64s.
public _extern _XCHG_U16 U64 XchgU16(U16 *dst,U16 d); //eXchange U16s.
public _extern _XCHG_U32 U64 XchgU32(U32 *dst,U32 d); //eXchange U32s.
public _extern _XCHG_U8  U64 XchgU8(U8 *dst,U8 d); //eXchange U8s.
public _extern SYS_CACHE_LINE_WIDTH I64 sys_cache_line_width; //CPU's Cache line width.
_extern SYS_GDT CGDT sys_gdt;

#help_index "Processor/Cache;Memory/Cache"
public _intern IC_CLFLUSH U0 CLFlush(U8 *a); //Flush cache line instruction.

#help_index "Processor/Page Tables;Memory/Page Tables"
public _intern IC_INVLPG U0 InvlPg(U8 *a); //Invalidate page at addr instruction.

#help_index "Task"
public _intern IC_PREEMPT Bool Preempt(Bool val); //Turn mutli-tasking preemption OFF/ON.

#help_index "Task;Processor"
public _intern IC_FS CTask *Fs(); //FS seg reg points to the current $LK,"CTask",A="MN:CTask"$.

#help_index "Time/CPU Cycles"
public _intern IC_RDTSC I64 GetTSC(); //Get time stamp counter.

#help_index "Time/HPET"
#help_file "::/Doc/TimeHPET"
public _intern IC_HPET I64 HPET(); //Get high precision event timer.

#help_index ""
