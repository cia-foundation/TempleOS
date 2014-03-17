#help_index "Time/Jiffies"
//18.33333*65536Hz (SYS_TIMER_FREQ)
public extern I64 SysTimerRead();

#help_index "Task"
extern CTask *sys_winmgr_task,*sys_task_being_screen_updated;
public extern CTask *adam_task;
public extern I64 sys_num_spawned_tasks;
extern U0 TaskFinishOffDying();
public _extern _YIELD U0 Yield();
_extern _TASK_CONTEXT_RESTORE U0 TaskContextRestore();
_extern _TASK_END_NOW U0 TaskEndNow();
#help_index "Task/Misc"
public extern U0 TSSBusy(I64 tr,Bool val=OFF);
public extern U0 OverRideSingleUserSwap();
public extern U0 WinDerivedValsUpdate(CTask *task=NULL);
public extern U0 TaskDerivedValsUpdate(CTask *task=NULL,Bool update_z_buf=TRUE);
extern U0 (*fp_update_ctrls)(CTask *task);

#help_index "Time/CPU Cycles"
#help_file "::/Doc/TimeCycles"
public extern I64 time_stamp_freq,time_stamp_kHz_freq;
public extern I64 TaskTotalTime();

//This is sampled once at boot time
public extern I64 time_stamp_freq_initial;

#help_index "Time/HPET"
#help_file "::/Doc/TimeHPET"
public extern I64 HPET_freq,HPET_initial,HPET_kHz_freq;

#help_index "Time/Seconds"
public extern F64 tS(); //From SysTimerFreq.
public extern Bool Blink(F64 Hz=2.5);

#help_index "Snd"
public extern U0 Snd(F64 freq);
public extern Bool Mute(Bool val);
public extern Bool IsMute();
public extern F64 snd_freq;
public extern U0 Beep(I64 freq=500,Bool busy=FALSE);
public extern Bool snd_record;
extern CSndData snd_record_root;

#help_index "Processor"
extern U8 *sys_fault_handlers_code;
extern I64 *sys_idt;
public extern U8 *IntEntryGet(I64 irq);
public extern U8 *IntEntrySet(I64 irq,U0 (*fp_new_handler)(),I64 type=IDTET_IRQ,I64 dpl=0);
extern CPCIDev sys_pci_dev_root;
extern CMemRange sys_mem32_dev_root;

#help_index "Task/Delay;Time/CPU Cycles"
extern I64 TimeStampFreqCal();
public extern U0 BusyWait(I64 ÊS); //Loosely timed
public extern U0 PortNop(); //On the order of 1uS
#help_index "Task/Delay;Time/Jiffies"
public extern U0 SleepUntil(I64 wake_jiffy);
public extern U0 Sleep(I64 mS);
#help_index "Task"
public extern Bool IsPreempt();

#help_index "Debugging"
#help_file "::/Doc/Dbg"
extern CTask *sys_dbg_focus_task;
extern U8    *sys_dbg_msg;
extern I64   sys_dbg_msg_num;
public extern Bool IsSysDbg();
public extern Bool SysDbg(Bool val);
public extern Bool ChkPtr(U8 *ptr);
public extern Bool ChkCodePtr(U8 *ptr);
public extern Bool ChkOnStk(U8 *ptr,CTask *task=NULL);
public extern I64 UnusedStk(CTask *task=NULL);
public extern U8 *Caller(I64 num=1);
public extern U8 *TaskCaller(CTask *task=NULL,I64 num=0,Bool saved_context=FALSE);
public extern Bool SingleUser(Bool val);
public extern Bool IsSingleUser();
#help_index "Debugging/Dump"
public extern U0 D(U8 *addr,I64 cnt=0x80,Bool show_offset=TRUE);
public extern U0 Dm(U8 *addr,I64 cnt=0x80);
public extern U0 Da(U8 **addr,I64 cnt=0x10);
extern I64 *TaskRegAddr(CTask *task,I64 reg_num);
public extern U0 Dr(CTask *task=NULL);
public extern U0 StkRep(CTask *task=NULL);
public extern U0 CallerRep(U8 **rbp=NULL,CTask *task=NULL);
#help_index "Debugging;Debugging/Debugger"
public extern U0 Dbg(U8 *msg=NULL,I64 msg_num=0);
#help_index "Debugging/Debugger"
public extern Bool InDbg(Bool val);
public extern Bool IsDbg();
public extern U0 G(U8 *ip=INVALID_PTR,CTask *task=NULL);
public extern U0 G2(U8 *ip=INVALID_PTR,CTask *task=NULL);
public extern U0 S(U8 *ip=INVALID_PTR,CTask *task=NULL);
public extern U0 E(U8 *addr,I64 cnt=512,I64 ed_flags=0);
extern CBpt *BptFind(U8 *needle_addr,CTask *haystack_task=NULL,Bool rem=FALSE);
public extern Bool BptS(U8 *addr,CTask *task=NULL,Bool live=TRUE);
public extern Bool BptR(U8 *addr,CTask *task=NULL,Bool live=TRUE,Bool rem=TRUE);
public extern Bool B(U8 *addr,CTask *task=NULL,Bool live=TRUE) //Toggle breakpoint
public extern I64 B2(CTask *task=NULL,Bool live=TRUE);
#help_index "Debugging/Raw Output"
public extern Bool Raw(Bool val);
public extern Bool IsRaw();
extern U0 RawDr(CTask *task=NULL);
public extern U0 RawPutChar(I64 ch);
public extern U0 VGAFlush();
public extern U0 RawPrint(I64 mS=100,U8 *fmt,...);
public extern U0 RawD(I64 mS=100,U8 *addr,I64 cnt=0x80);
public extern U0 RawDm(I64 mS=100,U8 *addr,I64 cnt=0x80);

#help_index "Graphics/Color"
extern U0 (*fp_set_std_palette)();
public extern U8 *Color2Text(U8 *buf,CColorROPU32 c);
#help_index "Graphics/Screen"
extern I64 gr_width,gr_height;

#help_index "Char"
#help_file "::/Doc/Char"
public extern U8 *gr_font;

#help_index "Char/System"
public extern Bool text_mode;
extern U0 (**fp_ctrl_alt_let_cbs)(I64 sc);
extern I64 ctrl_alt_let_in_irq_flags;
extern U8 **ctrl_alt_let_descs;
extern I64 **ctrl_alt_let_ret_addr; //addr of ret addr on stack in kbd irq
extern CSrvCmd sys_macro_root;
extern CKeyDev sys_put_key_root;
extern CTask *sys_macro_util_task;
extern I64 sys_raw_col,sys_raw_flags;
extern U8 *sys_raw_screen_image;
extern I64 text_rows,text_cols;
public extern CKeyDev *KeyDevAdd(Bool (*fp_put_key)(I64 ch,I64 sc),Bool (*fp_puts)(U8 *st),I64 priority,Bool key_descs=FALSE);
public extern U0 KeyDevRem(CKeyDev *tempp);
public extern U0 SetKeyDesc(U8 *fmt,...);
public extern U0 SetCtrlAltLetCB(U8 ch,U0 (*fp_handler)(I64 sc),U8 *desc,Bool in_irq=FALSE);
extern U8 key_desc[128];
extern U8 *key_handler;

#help_index "Char/Output"
public extern U0 Print(U8 *fmt,...);
public extern U0 PrintErr(U8 *fmt,...);
public extern U0 PrintWarn(U8 *fmt,...);
public extern U0 NewLine(I64 cnt=1);
public extern U0 Spaces(I64 cnt=1);
public extern U0 PutKey(I64 ch=0,I64 sc=0);
public extern U0 PutChars(U64 ch);
extern U0 PutS(U8 *st); //Use $LK,"Print",A="MN:Print"$()
public extern U8 *Bin2Hex(U8 *dst,I64 n;I64 width);
public extern U0 PutHex(I64 n,I64 width);
public extern Bool Silent(Bool val);
public extern Bool IsSilent();
public extern U0 GetOutOfDollar();
#help_index "Char/Conversion"
public extern I64 A2I64(U8 *st,I64 radix=10,U8 **_end_ptr=NULL);
#help_index "Input Pointer/Mouse"
public extern I64 mouse_evt_time;
extern I64 mouse_pkt_size;
#help_index "Char/Input"
public extern I64 kbd_evt_time;
extern Bool kbd_irqs_working;
public extern I64 PressAKey();
public extern Bool AreYouSure();
public extern I64 GetKey(I64 *_scancode=NULL,Bool echo=FALSE,Bool raw_cursor=FALSE);
public extern I64 GetChar(I64 *_scancode=NULL,Bool echo=TRUE,Bool raw_cursor=FALSE);
public extern Bool ScanKey(I64 *_ch=NULL,I64 *_scancode=NULL,Bool echo=FALSE);
public extern Bool YorN();
public extern I64 GetS(U8 *buf,I64 size,Bool allow_ext=TRUE);
public extern I64 KbdMouseEvtTime();

//Gets $LK,"Flags",A="MN:GETSF_SHIFT_ESC_EXIT"$
public extern U8 *MGetS(I64 flags=0);
extern U8  *(*fp_mgets)(I64 flags=0);
public extern U8 *ScanJoin(U8 *src,U8 *fmt,I64 argc,I64 *argv);
public extern U8 *Scan(U8 *src,U8 *fmt,...);

#help_index "Char/Lists"
public extern U8 *LstSub(I64 sub, U8 *lst);
public extern I64 LstMatch(U8 *needle, U8 *haystack_lst,I64 flags=0);
#help_index "Hash;Char/Lists"
public extern I64 HashLstAdd(U8 *lst,I64 type,CHashTable *table);

#help_index "Char/Operations"
public extern I64 StrOcc(U8 *src, U8 ch);
public extern U8 *StrFirstOcc(U8 *src,U8 *marker);
public extern U8 *StrLastOcc(U8 *src,U8 *marker);
public extern I64 Spaces2Tabs(U8 *dst,U8 *src);
public extern U8 *Tabs2Spaces(U8 *src);
public extern U8 *ScaleIndent(U8 *src,F64 indent_scale_factor);
public extern U8 *StrUtil(U8 *_src,I64 flags);
public extern U8 *MStrUtil(U8 *src,I64 flags,F64 indent_scale_factor=0);
public extern U8 *StrFirstRem(U8 *src,U8 *marker,U8 *dst=NULL);
public extern U8 *StrLastRem(U8 *src,U8 *marker,U8 *dst=NULL);
public extern Bool WildMatch(U8 *test_str,U8 *wild_str);
public extern U8 *StrFind(U8 *needle,U8 *haystack_str,I64 flags=0);
public extern U8 *SPrintJoin(U8 *dst,U8 *fmt,I64 argc,I64 *argv);
public extern U8 *SPrint(U8 *dst,U8 *fmt,...);
public extern U8 *CatPrint(U8 *dst,U8 *fmt,...);
public extern U8 *MPrint(U8 *fmt,...);

#help_index "Char/Define"
#help_file "::/Doc/Define"
public extern CHashDefineStr *DefineLoad(U8 *name,U8 *st);
public extern U0 DefinePrint(U8 *name,U8 *src,...);
public extern U8 *Define(U8 *name);
#help_index "Char/Define;Char/Lists"
public extern CHashDefineStr *DefineLstLoad(U8 *name,U8 *st);
public extern U8 *DefineSub(I64 sub,U8 *name);
public extern I64 DefineCnt(U8 *name);
public extern I64 DefineMatch(U8 *needle,U8 *haystack_lst_name,I64 flags=0);
#help_index "Hash;Char/Define;Char/Lists"
public extern I64 HashDefineLstAdd(U8 *name,I64 type,CHashTable *table);

#help_index "Memory"
#help_file "::/Doc/Mem"
public extern I64 MemPageSize(U8 *a)
public extern Bool MemPagePresentMark(U8 *a,Bool val);
public extern Bool MemLowPagesProtect(Bool val=TRUE);
public extern U8 *MemBlks2MegAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL);
public extern U8 *MemBlksUncachedAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL);
public extern U8 *MemBlksWriteThroughAlloc(I64 *_pages2Meg,CBlkPool *bp=NULL);
public extern U0 MemBlks2MegFree(CMemBlk *m,I64 pages2Meg,CBlkPool *bp=NULL);
public extern U8 *MemBlksIndependentAlloc(I64 *_pages512,CBlkPool *bp=NULL);
public extern I64 *MemPageTable(U8 *a);
public extern U0 MemBlksNonTaskFree(CMemBlk *m,I64 pages512,CBlkPool *bp=NULL);
public _extern _MALLOC U8 *MAlloc(I64 size,CTask *mem_task=NULL);
public _extern _FREE U0 Free(U8 *addr);
public _extern _MSIZE I64 MSize(U8 *src);	//size of heap object
public _extern _MSIZE2 I64 MSize2(U8 *src); //Internal size
public _extern _MHEAP_CTRL CHeapCtrl *MHeapCtrl(U8 *src);
public extern U8 *CAlloc(I64 size,CTask *mem_task=NULL);
public extern U8 *MAllocIdentical(U8 *src,CTask *mem_task=NULL);
public extern U8 *MAllocAligned(I64 size,I64 alignment,CTask *mem_task=NULL,I64 misalignment=0);
public extern U8 *CAllocAligned(I64 size,I64 alignment,CTask *mem_task=NULL,I64 misalignment=0);
public extern I64 Scale2Mem(I64 min,I64 max,I64 limit=2*1024*1024*1024);
#help_index "Memory/Heap Ctrl"
public extern U0 BlkPoolAdd(CBlkPool *bp,CMemBlk *m,I64 pages512);
public extern U0 BlkPoolInit(CBlkPool *bp,I64 pages512);
public extern CHeapCtrl *HeapCtrlInit(CHeapCtrl *hc=NULL,CTask *task=NULL,CBlkPool *bp);
public extern U0 HeapCtrlDel(CHeapCtrl *hc);
public extern CHeapCtrl *HeapCtrlBPInit(CBlkPool *bp,I64 pages512);
#help_index "Memory/Adam Heap"
public extern U8 *AMAlloc(I64 size);
public extern U8 *AMAllocIdentical(U8 *src);
public extern U8 *ACAlloc(I64 size);

#help_index "Char/Operations;Memory"
public extern U8 *StrNew(U8 *buf,CTask *mem_task=NULL);
#help_index "Char/Operations;Memory/Adam Heap"
public extern U8 *AStrNew(U8 *buf);

#help_index "Device"
public extern U8 *Mem32DevAlloc(I64 size,I64 alignment);
public extern U8 *Mem64DevAlloc(I64 *_pages2Meg);
public extern U0 Mem32DevFree(U8 *base);
public extern U0 Mem64DevFree(U8 *base,I64 pages2Meg);
public extern CHeapCtrl *sys_uncached_heap;

#help_index "Task"
public extern U0 Exit();
public extern U0 Kill(CTask *task,Bool wait=TRUE);
public extern Bool Suspend(CTask *task=NULL,Bool state=TRUE);
public extern Bool IsSuspended(CTask *task=NULL);
public extern Bool TaskValidate(CTask *task);
public extern U0 TaskQueIns(CTask *task,CTask *pred=NULL);
public extern U0 TaskQueRem(CTask *task);
public extern U0 TaskWait(CTask *task=NULL);
public extern U0 UserCmdLine();
public extern CTask *SpawnUser(U8 *fmt=NULL,...);
extern U0 TaskFinishOff(CTask *task);
#help_index "Task;MultiCore"
public extern CTask *Spawn(U0 (*fp_addr)(U8 *data),U8 *data=NULL,
	U8 *task_name=NULL,I64 target_cpu=-1,	//-1 means current CPU
	CTask *parent=NULL, //NULL means adam
	I64 stk_size=0,I64 flags=1<<SVCf_ADD_TO_QUE);

#help_index "Task/Srv"
public extern U0 SrvCmdDel(CSrvCmd *tempc);
public extern U0 TaskRstAwaitingMsg(CTask *task=NULL);
public extern CSrvCmd *TaskExe(CTask *srv,CTask *master,U8 *data,I64 flags);
public extern I64 SrvCmdsHandler(CTask *task=NULL);
public extern I64 SrvUser(U8 *fmt,...);
extern U0 SrvUserCmdLine();
public extern U0 SrvTaskCont();
public extern I64 Adam(U8 *fmt,...);
public extern U0 AdamLog(U8 *fmt,...);
public extern U0 AdamErr(U8 *fmt,...);
#help_index "Task/Text & Msgs"
public extern CSrvCmd *TaskText(CTask *srv,CTask *master,
	U8 *data,I64 flags);

#help_index "Task/Text & Msgs;Messages"
public extern CSrvCmd *TaskMsg(CTask *srv,CTask *master,
		     I64 msg_code,I64 p1,I64 p2,I64 flags);
public extern U0 PostMsg(CTask *task,I64 msg_code,I64 p1,I64 p2,I64 flags=0);
public extern U0 SendMsg(CTask *task,I64 msg_code,I64 p1,I64 p2,I64 flags=0);
public extern U0 Msg(I64 msg_code,I64 p1,I64 p2,I64 flags=0);

#help_index "Exceptions"
//Exceptions are thrown through the following function call.
public extern U0 throw(I64 ch=0,Bool no_log=FALSE);
//The following are used internally and should not be called directly
extern U0 SysTry(U8 *start_label,U8 *skip_label);
extern U0 SysUntry();
public extern U0 ThrowBreak(CTask *task=NULL);
public extern Bool BreakLock(CTask *task=NULL);
public extern Bool BreakUnlock(CTask *task=NULL);
public extern U0 PutExcept(Bool catch_it=TRUE);

#help_index "Compression/Piece by Piece"
public extern U0 ArcCompressBuf(CArcCtrl *c);
public extern U0 ArcFinishCompression(CArcCtrl *c);
public extern U0 ArcExpandBuf(CArcCtrl *c);
public extern CArcCtrl *ArcCtrlNew(Bool expand,Bool seven_bit);
public extern U0 ArcCtrlDel(CArcCtrl *c);
#help_index "Compression"
public extern CArcCompress *CompressBuf(U8 *src,I64 size,I64 flags=0,CTask *mem_task=NULL);
public extern U8 *ExpandBuf(CArcCompress *r,CTask *mem_task=NULL);

#help_index "AutoFile"
#help_file "::/Doc/AutoFile"
#help_index "Task/Text & Msgs;AutoFile"
public extern U0 Auto(U8 *fmt,...);
public extern U0 AutoStr(U8 *fmt,...);
public extern U0 AutoFile(U8 *filename);
public extern U0 XTalk(CTask *task,U8 *fmt,...);
public extern U0 XTalkWait(CTask *task,U8 *fmt,...);
public extern U0 XTalkStr(CTask *task,U8 *fmt,...);
public extern U0 XTalkStrWait(CTask *task,U8 *fmt,...);

#help_index "Misc/Progress Bars"
public extern U0 ProgressBarsRst();

#help_index "Math"
public extern U16 RandU16(); //With timestamp
public extern U32 RandU32();
public extern U64 RandU64();
public extern I16 RandI16();
public extern I32 RandI32();
public extern I64 RandI64();
public extern F64 Rand();
public extern U16 Rand2U16(); //Without timestamp
public extern U32 Rand2U32();
public extern U64 Rand2U64();
public extern I16 Rand2I16();
public extern I32 Rand2I32();
public extern I64 Rand2I64();
public extern F64 Rand2();
public extern U0  Randomize(I64 seed=0);
public extern U64 FloorU64(U64 num,U64 to);
public extern U64 CeilU64(U64 num,U64 to);
public extern I64 RoundI64(I64 num,I64 to);
public extern I64 FloorI64(I64 num,I64 to);
public extern I64 CeilI64(I64 num,I64 to);

#help_index "Misc"
public extern U0 QSortI64(I64 *base,I64 num, I64 (*fp_compare)(I64 e1,I64 e2));
public extern U0 QSort(U8 *base,I64 num, I64 width,I64 (*fp_compare)(U8 *e1,U8 *e2));

public extern U16 EndianU16(U16 d);
public extern U32 EndianU32(U32 d);
public extern U64 EndianU64(U64 d);
extern I64 sys_rnd_seed;

#help_index "Bit"
public extern I64 BCnt(I64 d);
public extern U8 *reverse_bits_table,*set_bits_table;

#help_index "Doc/Clipboard"
public extern CDoc *sys_clipboard_doc;

#help_index "Time/Date;Date"
public extern U16 mon_start_days1[12];
public extern U16 mon_start_days2[12];
public extern I64 YearStartDate(I64 year);
public extern I64 FirstDayOfMon(I64 i);
public extern I64 LastDayOfMon(I64 i);
public extern I64 FirstDayOfYear(I64 i);
public extern I64 LastDayOfYear(I64 i);
public extern I64 DayOfWeek(I64 i);

#help_index "Time/Date/CDate;Date/CDate"
#help_file "::/Doc/TimeDate"
public extern CDate Struct2Date(CDateStruct *_ds);
public extern U0 Date2Struct(CDateStruct *_ds,CDate cdt);
extern U0 Date2ISO1(CISO1Date *dst,CDate cdt);
public extern CDate local_time_offset;
public extern CDate Now();
public extern I64 Hour12 (CDate cdt=MIN_I64);
public extern I64 Hour24 (CDate cdt=MIN_I64);
public extern I64 Minute (CDate cdt=MIN_I64);
public extern I64 Second (CDate cdt=MIN_I64);
public extern I64 Day	 (CDate cdt=MIN_I64);
public extern I64 WeekDay(CDate cdt=MIN_I64);
public extern I64 Month  (CDate cdt=MIN_I64);
public extern I64 Year	 (CDate cdt=MIN_I64);
extern U8 *MPrintTime(CDate cdt);
extern U8 *MPrintDate(CDate cdt);

#help_index "File/System"
extern U8 sys_temp_filename[136];
extern I64 sys_dsk_reads,sys_dsk_writes;
extern CCacheBlk *sys_dsk_cache_base,*sys_dsk_cache_ctrl;
extern CCacheBlk **sys_dsk_cache_hash_table;
extern I64 sys_dsk_cache_size;
extern U0 DskCacheInit(I64 size_in_U8s);
public extern U0 DskCacheInvalidate(CPrt *p);
public extern CBlkDev *sys_blkdevs;
public extern CPrt *sys_prts;
extern CPrt *drv_to_prt[32];

#help_index "PCI;Info;File/System;Device"
public extern I64 ATARep(Bool pmt=TRUE,Bool just_ide=FALSE,CATARep **_root=NULL);

#help_index "File/System"
public extern CATARep *ATAIDDrvs(CATARep *root,U8 _sys_acct_drv,CATARep **_ata_drv,CATARep **_atapi_drv);
extern CBlkDev *ATAMount(U8 first_drv_let,I64 type,I64 base0,I64 base1,I64 unit);
public extern I64 sys_auto_mount_ide_cnt;
public extern I64 AutoMountIDE();
extern I64 ATAProbe(I64 base0,I64 base1,I64 unit);
extern Bool ATAPIStartStop(CBlkDev *bdev,F64 timeout,Bool start);
extern I64 BlkDevAdd(CBlkDev *bdev,Bool one_prt,Bool make_free);
extern U0 ATAReadBlks(CBlkDev *bdev,U8 *buf, I64 blk, I64 cnt);
extern U0 ATAWriteBlks(CBlkDev *bdev,U8 *buf, I64 blk, I64 cnt);

#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"
#help_file "::/Doc/CmdLine"
public extern U0 ChgDsk(U8 drv_let=0);
public extern Bool Drv(U8 drv_let);
public extern Bool Cd(U8 *dirname,Bool make_dirs=FALSE);
public extern Bool MapDrv(U8 drv_let,CPrt *p);
public extern I64 Dir(U8 *wild="*",Bool full=FALSE);
public extern I64 Del(U8 *wild,Bool print_msg=TRUE);
public extern Bool MkDir(U8 *filename);

#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically);Install"
public extern U0 PrtRep();

#help_index "File/Program Routines/Low Level"
extern CDirEntry *SysFilesFind(U8 *files_find_mask,I64 fuf_flags);
extern I64 SysDel(U8 *wild,Bool make_mask);	//this is dangerous
extern Bool CopySingle(U8 *f1,U8 *f2); //Just one file
extern U0 CleanMAPBuf(CPrt *p);
extern U0 FAT32FreeClusters(CPrt *p,I64 c);
extern U0 RedSeaFSFreeClusters(CPrt *p,I64 c,I64 cnt);
public extern Bool BlkDevLock(CBlkDev *bdev);
public extern Bool PrtLock(CPrt *p);
public extern Bool BlkDevUnlock(CBlkDev *bdev,Bool rst=FALSE);
public extern Bool PrtUnlock(CPrt *p,Bool rst=FALSE);
public extern U0 BlkDevsRelease();
public extern CPrt *PrtMakeFree(U8 drv_let);
public extern CPrt *PrtNextFree(U8 first_drv_let='C');
public extern CBlkDev *BlkDevNextFree(U8 first_drv_let,I64 type);
public extern U0 PrtsRelease();
public extern I64 ClusterNumNext(CPrt *p,I64 c,I64 cnt=1);
public extern I64 Cluster2Blk(CPrt *p,I64 c);
public extern I64 RClustersBlks(CPrt *p,U8 *buf,I64 c,I64 blks);
public extern I64 RClusters(CPrt *p,U8 *buf,I64 c,I64 cnt);
public extern I64 WClustersBlks(CPrt *p,U8 *buf,I64 c,I64 blks);
public extern I64 WClusters(CPrt *p,U8 *buf,I64 c,I64 cnt);
public extern I64 ClustersAlloc(CPrt *p,I64 c=0,I64 cnt=1,Bool contiguous=FALSE);
public extern Bool RBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
public extern Bool WBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt);
public extern U0 WZeroBlks(CPrt *p,I64 blk,I64 cnt);
public extern U0 PrtChk(CPrt *p);
public extern U0 BlkDevChk(CBlkDev *bdev);
public extern U8 Prt2Drv(CPrt *p=NULL);
public extern CPrt *Drv2Prt(U8 drv_let=0);
public extern CBlkDev *Drv2BlkDev(U8 drv_let=0);
extern I64 Name2DirCluster(CPrt *p,U8 *dirname);
extern I64 Name2ParentDirCluster(CPrt *p,U8 *dirname);
public extern U8 *DrvSerialNum(U8 drv_let=0);
public extern U8 *DrvModelNum(U8 drv_let=0);

#help_index "File/Program Routines/CFile"
public extern CFile *FOpen(U8 *filename,U8 *flags,I64 cnt=0);
public extern I64 FSize(CFile *f);
public extern U0 FClose(CFile *f);
public extern Bool FRBlks(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1);
public extern Bool FWBlks(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1);

#help_index "File/CD DVD"
public extern U0 CDImageRead(U8 cd_drv_let,U8 *out_name);
public extern U0 CDImageWrite(U8 cd_drv_let,U8 *in_name=NULL,I64 media_type=MT_CD);
public extern U8 *sys_dft_iso_name;

#help_index "File/FileNames"
public extern U8 *sys_acct;
public extern Bool FilesFindMatch(U8 *test_filename,U8 *files_find_mask,I64 fuf_flags=0);
public extern Bool IsDotZ(U8 *filename);
public extern Bool IsDotC(U8 *filename);
public extern U8 *FileExtDot(U8 *src);
public extern U8 *FileExtRem(U8 *src,U8 *dst=NULL);
public extern U0 PutFileLink(U8 *filename,I64 line=0,Bool plain_text=FALSE);
public extern U0 PutDirLink(U8 *dirname);
public extern U8 *CurDir(CTask *task=NULL,CTask *mem_task=NULL);
public extern U8 *DirNameAbs(U8 *dirname);
public extern U8 *DirFile(U8 *dirname,U8 *name=NULL,U8 *_extension=NULL);
public extern U8 *ToggleZorNotZ(U8 *name);
public extern U8 *FileNameAbs(U8 *filename,I64 fuf_flags=0);
public extern Bool FileNameChk(U8 *filename);
public extern U8 *ChgExt(U8 *filename,U8 *extension);
public extern U8 *DftExt(U8 *filename,U8 *extension);
public extern Bool IsDir(U8 *dir_name);
public extern Bool IsDrvWritable(U8 drv_let=0);
extern CDirEntry *Cd2DirEntry(CDirEntry *tempde,U8 *abs_name);
public extern U0 ToFileLine(U8 *_fl_file_line,U8 **_filename,I64 *_linenum);

#help_index "File/Program Routines/Utils"
public extern U0 ScanFlags(U8 *f,U8 *lst,U8 *st);
public extern U8 *SPrintFlags(U8 *dst,U8 *lst,I64 flags);
public extern U0 FileAccessDel(CFileAccess *fa);
public extern CFileAccess *FileAccessNew(U8 *mask,Bool make_mask=FALSE,Bool make_dirs=FALSE,Bool no_mask=FALSE);
extern I64 FileAttr(U8 *name,I64 old_attr=0);
public extern U0 DirEntryDel(CDirEntry *tempde);
public extern U0 DirEntryDel2(CDirEntry *tempde);
public extern U0 DirLstDel(CDirEntry *tempde);
public extern U0 DirLstDel2(CDirEntry *tempde);
public extern Bool DirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempde,Bool free_old_chain=TRUE);

#help_index "File/Program Routines"
public extern CDirEntry *FilesFind(U8 *files_find_mask,I64 fuf_flags=0);
public extern Bool FileFind(U8 *filename,CDirEntry *_de=NULL,I64 fuf_flags=0);
public extern U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL,Bool raw=FALSE);
public extern I64 FileWrite(U8 *filename,U8 *fbuf,
	I64 size,CDate cdt=0,I64 attr=0,Bool raw=FALSE);

#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically);"
public extern U0 Mount(Bool repartition=FALSE);
public extern U0 Fmt(U8 drv_let,Bool quick=TRUE,Bool confirm=TRUE,I64 type=PT_FAT32);

#help_index "Install"
#help_file "::/Doc/Install"
extern CATARep *ATARepFind(CATARep *haystack_root,I64 needle_num);
public extern Bool DrvEnable(U8 drv_let,Bool val);
public extern U8 sys_first_hd_drv_let,sys_first_cd_drv_let;
extern Bool boot_drv_pmt,cdrom_boot_is_good;

#help_index "Hash"
#help_file "::/Doc/Hash"
public _extern _HASH_STR I64 HashStr(U8 *st);
public _extern _HASH_FIND CHash *HashFind(U8 *needle_str,CHashTable *haystack_table,I64 mask,I64 instance=1);
public _extern _HASH_SINGLE_TABLE_FIND CHash *
	HashSingleTableFind(U8 *needle_str,CHashTable *haystack_table,I64 mask,I64 instance=1);
public _extern _HASH_BUCKET_FIND CHash **
	HashBucketFind(U8 *needle_str,CHashTable *haystack_table);
public _extern _HASH_ADD U0 HashAdd(CHash *temph,CHashTable *table);
public _extern _HASH_ADD_AFTER U0 HashAddAfter(CHash *temph,CHash *pred,CHashTable *table);
public _extern _HASH_REM_DEL Bool HashRemDel(CHash *temph,CHashTable *table,
					I64 instance=1);//instance must match temph's
public extern CHashGeneric *HashPublic(U8 *st,I64 mask,Bool val=TRUE);
public extern CHashGeneric *HashGenericAdd(U8 *name,I64 type,I64 val=0,I64 u8=0,I64 u16=0,CTask *task=NULL);
public extern I64 HashTypeNum(CHash *temph);
public extern I64 HashVal(CHash *temph);
public extern CHashTable *HashTableNew(I64 size,CTask *mem_task=NULL);
public extern U0 HashDel(CHashSrcSym *temph);
public extern U0 HashTableDel(CHashTable *table);
public extern I64 HashTablePurge(CHashTable *tempht);
extern U0 SysSymImportsResolve(U8 *sptr,I64 ld_flags=0);

#help_index "Compiler/Lex"
public extern U0 SetSrcFileInfo(CLex *lx,CHashSrcSym *h);

#help_index "Hash/Frame"
#help_file "::/Doc/Frame"
public extern I64 FramePtr(U8 *name,CTask *task=NULL);
public extern CHashGeneric *FramePtrAdd(U8 *name,I64 val,CTask *task=NULL);
public extern I64 FramePtrSet(U8 *name,I64 val,CTask *task=NULL);
public extern I64 FramePtrDel(U8 *name,CTask *task=NULL);

#help_index "Compiler;Cmd Line (Typically)"
public extern U8 *Load(U8 *filename,I64 ld_flags=0,
	CBinFile *bfh_addr=INVALID_PTR); //INVALID_PTR means don't care what load addr.

#help_index "Boot"
#help_file "::/Doc/Boot"
public extern U0 Reboot();

#help_index "MultiCore;Task/Srv"
public extern CSrvCmd *JobQue(I64 (*fp_addr)(U8 *data),U8 *data=NULL,
	I64 target_cpu=1,I64 flags=1<<SVCf_FREE_ON_COMPLETE,
	I64 cmd_code=SVCT_CALL,U8 *aux_str=NULL,I64 aux1=0,I64 aux2=0);
#help_index "MultiCore"
extern U8 mp_apic_ids[MP_MAX_PROCESSORS];
extern CMPCrash *mp_crash;
public extern U0 MPInt(U8 num,I64 cpu_num=1);
public extern U0 MPIntAll(U8 num);
public extern U0 MPWake(I64 target_cpu);
extern U0 MPSethServerTask();
extern CTask *SpawnQue(U0 (*fp_addr)(U8 *data),U8 *data=NULL,
	U8 *task_name=NULL,I64 target_cpu,
	CTask *parent=NULL, //NULL means adam
	I64 stk_size=0,I64 flags=1<<SVCf_ADD_TO_QUE);
public extern U0 MPNMInt();
public extern U0 MPStart();
#help_index "Task/Srv;MultiCore"
public extern Bool ScanResult(CSrvCmd *rqst=NULL,I64 *_result=NULL);
public extern I64 GetResult(CSrvCmd *rqst=NULL);
#help_index "MultiCore;Boot"
public extern U0 MPHalt();

#help_index "Math"
public extern F64 Clamp(F64 num,F64 lo,F64 hi);
public extern F64 Min(F64 n1,F64 n2);
public extern F64 Max(F64 n1,F64 n2);
public extern F64 Pow10I64(I64 i);

#help_index "PCI"
public extern U8 PCIReadU8(I64 bus,I64 dev,I64 fun,I64 rg);
public extern U16 PCIReadU16(I64 bus,I64 dev,I64 fun,I64 rg);
public extern U32 PCIReadU32(I64 bus,I64 dev,I64 fun,I64 rg);
public extern U0 PCIWriteU8(I64 bus,I64 dev,I64 fun,I64 rg,I64 val);
public extern U0 PCIWriteU16(I64 bus,I64 dev,I64 fun,I64 rg,I64 val);
public extern U0 PCIWriteU32(I64 bus,I64 dev,I64 fun,I64 rg,I64 val);
public extern I64 PCIClassFind(I64 class_code,I64 n);

//These are far call routines in the BIOS
public _extern SYS_BIOS32_SERVICE_DIR U32 sys_BIOS32_service_dir;
public _extern SYS_PCI_SERVICES U32 sys_pci_services;

#help_index "PCI;Call"
public _extern C32_EAX U32 c32_eax;
public _extern C32_EBX U32 c32_ebx;
public _extern C32_ECX U32 c32_ecx;
public _extern C32_EDX U32 c32_edx;
public _extern C32_ESI U32 c32_esi;
public _extern C32_EDI U32 c32_edi;
public _extern C32_EFLAGS U32 c32_eflags;
//Not reentrant
public _extern _FAR_CALL32 Bool FarCall32(U0 (*fp_addr)());

#help_index "Call"
//fp_addr can have any fixed number of arguments.
public argpop extern I64 CallStkGrow(I64 stk_size_threshold,I64 stk_size,/*argpop*/I64 (*fp_addr)(...),...);

#help_index "Data Types/Circular Queue"
public extern U0 QueDel(CQue *root,Bool querem=FALSE);
public extern CQue *QueCopy(CQue *root,CTask *mem_task=NULL);
public extern I64 QueCnt(CQue *root);
public extern I64 QueSize(CQue *root);

#help_index "Data Types"
public extern U0 LinkedLstDel(U8 **_lst);
public extern U8 *LinkedLstCopy(U8 **_lst,CTask *mem_task=NULL);
public extern I64 LinkedLstCnt(U8 **_lst);
public extern I64 LinkedLstSize(U8 **_lst);

public extern CQueVectU8 *QueVectU8New(I64 min_index=0);
public extern U0 QueVectU8Put(CQueVectU8 *v,I64 index,U8 ch);
public extern U0 QueVectU8Del(CQueVectU8 *v);
public extern I64 QueVectU8Get(CQueVectU8 *v,I64 index);

public extern CFifoU8 *FifoU8New(I64 size,CTask *mem_task=NULL);
public extern U0 FifoU8Del(CFifoU8 *f);
public extern Bool FifoU8Ins(CFifoU8 *f,U8 b);
public extern Bool FifoU8Rem(CFifoU8 *f,U8 *_b);
public extern Bool FifoU8Peek(CFifoU8 *f,U8 *_b);
public extern U0 FifoU8Flush(CFifoU8 *f);
public extern I64 FifoU8Cnt(CFifoU8 *f);
public extern CFifoI64 *FifoI64New(I64 size,CTask *mem_task=NULL);
public extern U0 FifoI64Del(CFifoI64 *f);
public extern Bool FifoI64Ins(CFifoI64 *f,I64 q);
public extern Bool FifoI64Rem(CFifoI64 *f,I64 *_q);
public extern Bool FifoI64Peek(CFifoI64 *f,I64 *_q);
public extern U0 FifoI64Flush(CFifoI64 *f);
public extern I64 FifoI64Cnt(CFifoI64 *f);

#help_index "Char/Operations"
public _extern _STRCMP I64 StrCmp(U8 *st1,U8 *st2);
public _extern _STRICMP I64 StrICmp(U8 *st1,U8 *st2);
public _extern _STRNCMP I64 StrNCmp(U8 *st1,U8 *st2,I64 n);
public _extern _STRNICMP I64 StrNICmp(U8 *st1,U8 *st2,I64 n);
public _extern _STRMATCH U8 *StrMatch(U8 *needle,U8 *haystack_str);
public _extern _STRIMATCH U8 *StrIMatch(U8 *needle,U8 *haystack_str);
public _extern _STRCPY U0 StrCpy(U8 *dst,U8 *src);

#help_index "Char/Input"
public extern I64 sys_key_cnt;
public extern U0 SetTypeMatic(U8 delay);
extern U0 KbdInit();
extern I64 KbdMsgsQue();
extern U0 KbdMouseVarsInit();
extern Bool MouseDriverInstall();
extern U0 KbdMouseHandler(Bool poll_kbd,Bool poll_mouse);
public extern U32 key_down_bitmap[8],key_down_bitmap2[8];

#help_index "Char/BitMaps"
public extern U32 chars_bitmap_alpha[16];
public extern U32 chars_bitmap_alpha_numeric[16];
public extern U32 chars_bitmap_alpha_numeric_no_at[16];
public extern U32 chars_bitmap_word[16];
public extern U32 chars_bitmap_dec_numeric[16];
public extern U32 chars_bitmap_hex_numeric[16];
public extern U32 chars_bitmap_white_space[16];
public extern U32 chars_bitmap_non_eol_white_space[16];
public extern U32 chars_bitmap_zero_cr_nl_cursor[16];
public extern U32 chars_bitmap_zero_tab_ff_cr_nl_cursor[16];
public extern U32 chars_bitmap_zero_tab_ff_cr_nl_cursor_dollar[16];
public extern U32 chars_bitmap_getkey[16];
public extern U32 chars_bitmap_printable[16];
public extern U32 chars_bitmap_displayable[16];
public extern U32 chars_bitmap_safe_dollar[16];
public extern U32 chars_bitmap_filename[16];
public extern U32 chars_bitmap_non_eol[16];

#help_index "Char/Conversion"
public extern I64 A2ScanCode(I64 ch,I64 sc_flags=0);
public extern U8 ScanCode2A(I64 sc);
public extern U8 *A2KeyName(I64 ch,Bool include_ctrl=TRUE);
public extern U8 *ScanCode2KeyName(I64 sc);

#help_index "Input Pointer/Mouse"
public extern F64 mxx_scale,myy_scale,mzz_scale;

//These are the coordinates before
//they get scaled.
public extern I64 mxx_prescale,myy_prescale,mzz_prescale;
public extern I64 mxx,myy,mzz;
public extern F64 mouse_speed;
extern I64 old_mxx,old_myy,old_mzz;
public extern Bool mouse_buttons[5];
extern Bool mouse_installed;
extern I64  mouse_install_attempts,
	    mouse_last_evt_time;
extern Bool mouse_has_wheel;
extern Bool mouse_has_ext_buttons;
extern Bool mouse_evt;
extern Bool mouse_install_attempt_in_progress;
extern CFifoI64 *scan_code_fifo;
extern I64 sys_last_down_scan_code;
extern Bool kbdmouse_rst;
extern Bool mouse_irqs_working;

#help_index "Input Pointer"
#help_file "::/Doc/InputPointer"
public extern I64 ipx,ipy,ipz,ip_throttle,ip_throttle_step;
public extern I64 ipx_offset,ipy_offset,ipz_offset;
public extern F64 ipx_scale,ipy_scale,ipz_scale;
public extern F64 ip_speed;
public extern I64 ip_dev;
public extern I64 iptx,ipty;
public extern Bool ip_lb,ip_cb,ip_rb;
extern Bool ip_lb_last,ip_cb_last,ip_rb_last;
public extern Bool ip_has_wheel;
extern F64 ip_last_move_time;
public extern CGrid ip_grid;
public extern U0 GridInit();
public extern U0 SetIP(I64 x=MAX_I64,I64 y=MAX_I64,I64 z=MAX_I64,I64 l=MAX_I64,I64 r=MAX_I64,I64 c=MAX_I64);
#help_index "Input Pointer/Displayed Image"
public extern Bool ip_on;  //Show or hide cursor
extern U0 IPVarsUpdate(I64 x,I64 y,I64 z,Bool l,Bool c,Bool r);
extern U0 IPVarsInit();
public extern I64 ipx_presnap,ipy_presnap,ipz_presnap;

#help_index "Messages"
extern U0 InputFilterTask();
public extern I64 ScanMsg(I64 *_p1=NULL,I64 *_p2=NULL,I64 mask=~1,CTask *task=NULL);
public extern I64 FlushMsgs(CTask *task=NULL);
public extern I64 GetMsg(I64 *_p1=NULL,I64 *_p2=NULL,I64 mask=~1,CTask *task=NULL);
public extern I64 ScanChar();

#help_index "Misc;Messages"
public extern I64 sys_cur_scan_code;

#help_index "Doc/Task"
extern CDoc *(*fp_doc_put)(CTask *task=NULL);

#help_index "Char/Conversion"
public extern F64 A2F64(U8 *src,U8 **_end_ptr=NULL);

#help_index "Char/Conversion;Time/Date/CDate;Date/CDate"
public extern CDate A2Date(U8 *src);

#help_index "Debugging/PSect;Hash"
public extern CHash *PSectFind(U8 *addr,I64 *_offset);
extern CPSectCache *psect_cache;
extern I64 psect_cache_index;
#help_index "Debugging/PSect"
public extern I64 SrcLineNum(U8 *addr,I64 cnt=1);
public extern U8 *SrcFileName(U8 *addr,I64 cnt=1,CTask *mem_task=NULL);
public extern U8 *SrcEdLink(U8 *addr,I64 cnt=1,CTask *mem_task=NULL);
public extern Bool PutSrcLink(U8 *addr,I64 cnt=1,U8 *buf=NULL);

#help_index "Debugging;Debugging/Debugger"
public extern U0 EdLite(U8 *filename,I64 num=1);
extern U8 *sys_fix_file_line;
public extern U0 SetFix(U8 *filename,I64 line);
extern U0 EdLiteFileLine(U8 *fl_file_line);
public extern U0 Fix();
#help_index "Debugging/Debugger;Doc/Cmd Line (Typically);Cmd Line (Typically);Training"
public extern U0 Man(U8 *st);

#help_index "Task/Srv;Compiler"
public extern U0 UserTaskCont();
public extern I64 ExeCmdLine(CLex *lx);

#help_index "Training;Debugging/Debugger"
public extern U0 Help();

#help_index ""
