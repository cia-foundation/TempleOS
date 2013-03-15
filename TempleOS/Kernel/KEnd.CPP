U0 SysGlblsInit()
{
  I64 i,j;
  U8 *m;
  CRAXRBCRCXRDX ee;

  CPUId(0x1,&ee);
  sys_cache_line_width=ee.rbx.u8[1]*8;
  QueInit(&sys_pci_dev_root);
  sys_rnd_seed = 93186752^GetTimeStamp;
  sys_focus_task=Fs;
  QueInit(&sys_macro_root);
  sys_dft_iso_name="::/Temp/CDDVD.ISO";
  MemSet(mp_apic_ids,0,MP_MAX_PROCESSORS*sizeof(U8));

  LBts(&adam_task->task_flags,TASKf_INIT_EXT);//So dbgger doesn't call $LK,"TaskInitExt","MN:TaskInitExt"$().
  InDbg(ON);

  reverse_bits_table=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      if (Bt(&i,7-j))
	LBts(reverse_bits_table+i,j);
  set_bits_table=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      if (Bt(&i,j))
	set_bits_table[i]++;

  ext=CAlloc(EXT_TABLE_SIZE*sizeof(U8 *));
  fp_mgets=&SysMGetS;

  fp_ctrl_alt_let_cbs=CAlloc(26*sizeof(U8 *));
  ctrl_alt_let_descs=CAlloc(26*sizeof(U8 *));;
  ctrl_alt_let_in_irq_flags=0;

  psect_cache=CAlloc(PSECT_CACHE_SIZE*sizeof(CPSectCache));
  psect_cache_index=0;

  StrCpy(sys_temp_filename,"HOME/Temp.TXT.Z");
  MemSet(&sys_put_key_root,0,sizeof(CKeyDev));
  QueInit(&sys_put_key_root);
  KeyDevAdd(&AKDInputFilterPutKey,&AKDInputFilterPutS,0x40000000,FALSE);
  KeyDevAdd(&AKDRawDisplayPutKey,&AKDRawDisplayPutS,0x60000000,FALSE);
  #exe {
    if (*kernel_cfg->acct_name)
      StreamPrintF("sys_acct=StrNew(\"%C:/TempleOS/Accts/%s\");",kernel_cfg->dft_drv,kernel_cfg->acct_name);
    else
      StreamPrintF("sys_acct=StrNew(\"%C:/TempleOS/Accts\");",kernel_cfg->dft_drv);
    StreamPutS("#exe{OptOff(OPTf_WARN_PAREN);}");
    StreamPrintF("DskCacheInit(%s);",kernel_cfg->dsk_cache_size_exp);
    StreamPutS("#exe{OptOn(OPTf_WARN_PAREN);}");
  };
  pow10_I64=CAlloc(sizeof(F64)*(308+308+1));
  for (i=-308;i<309;i++)
    pow10_I64[i+309]=Pow10(i);

//$AN,"Independent Heap Ctrl Example","Independent Heap Ctrl Example"$
  i=1; //Rqst 1 blk, we might get more.
  m=MemBlksUncachedAlloc(&i,sys_code_bp);
  sys_uncached_heap=HeapCtrlIndependentInit(m,i*0x200000/0x200);
  ProgressBarsRst;
  date_fmt_map=MAlloc(DF_NUM);
  time_fmt_map=MAlloc(TF_NUM);
  DateTimeFmtMapsRst;
  sys_fault_handlers_code=IntFaultHandlersNew;
}

U0 SysGrInit()
{
//See $LK,"SYS_BIOS_GR_MODE","FF:D:/TempleOS/Kernel/KStart.CPP,SYS_BIOS_GR_MODE:2"$
//See $LK-A,"::/TempleOS/Adam/Gr/GrCfg.CPP"$
  gr_font=sys_font_8x8;
  if (bios_gr_mode==0x12 || bios_gr_mode==0x6A) {
    text_mode=FALSE;

/*
Mode must be multiple of 32 wide
$LK,"GrUpdateLine32","MN:GrUpdateLine32"$().
*/

    if (bios_gr_mode==0x6A) {
      gr_width =800;
      gr_height=600;
    } else {
      gr_width =640;
      gr_height=480;
    }
    text_cols=gr_width/FONT_WIDTH;
    text_rows=gr_height/FONT_HEIGHT;
    OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
    OutU8(VGA_SC_DATA,0x0F);
    MemSet(VGA_GRAPHIC_MEM_BASE,0,gr_width*gr_height>>3);
    sys_dc_screen_image=CAlloc(gr_width*gr_height/8);
  } else {
    text_mode=TRUE;
    gr_width =640;
    gr_height=480;
    text_cols=80;
    text_rows=25;
    MemSet(VGA_TEXT_MEM_BASE,0,text_rows*text_cols<<1);
  }
}


U0 TimersInit()
{
  I64 i,*qq;
  U32 *_d;

  OutU8(0x43,0x34);
  OutU8(0x40,SYS_TIMER0_PERIOD);
  OutU8(0x40,SYS_TIMER0_PERIOD>>8);

// High Precision Event Timer

  if (PCIReadU16(0,31,0,0)==0x8086) { //Intel?
    //D31 F0, cfg 0xF0=RCBA of PCI-LPC Bridge
    _d=PCIReadU32(0,31,0,0xF0)(U8 *)&~0x3FFF+0x3404; //HPET cfg
  //7	enable
  //1:0 HPET is at 0xFED00000,0xFED01000, 0xFED02000 or 0xFED03000.
    *_d=*_d&3|0x80;
  }

  qq=HPET_GCAP_ID;
  i=*qq; //i.u32[1]= period in femtoS (10e-15)
  if (100000<i.u32[1]<1000000000) {
    HPET_freq	 =1000000000000000/i.u32[1];
    HPET_kHz_freq=1000000000000/i.u32[1];
    qq=HPET_GEN_CONF;
    *qq|=1; //Enable counting
  } else {
    HPET_freq=0;
    HPET_kHz_freq=0;
  }
}

U0 Reboot()
{
  U16 *d=0x472;
  Cli;
  if (mp_cnt>1)
    MPHalt;
  MemPagePresentMark(d,TRUE);
  *d=0;
  OutU8(0x70,0x8F);
  OutU8(0x71,0x00);
  OutU8(0x70,0x00);
  OutU8(0x92,InU8(0x92)|1);
  SysHlt;
}

U0 Kernel()
{ //Continued from $LK,"Kernel","FF:::/TempleOS/Kernel/KStart.CPP,I32 &Kernel"$
  adam_task=Fs;
  HeapsInit;
  LBts(&sys_run_level,RLf_FULL_HEAP);
  Fs->hash_table=HashTableNew(ADAM_HASH_SIZE,Fs);

#exe {StreamPrintF("SysDeterministic(%d);",kernel_cfg->opts[CFG_DETERMINISTIC]);};
  SysGlblsInit;

  Mem32DevInit;
  MemPagesNotPresentMark;
  LoadKernel;
  SysGrInit;
  StrCpy(Fs->task_name,"Adam Task CPU#0");
  StrCpy(Fs->task_title,Fs->task_name);
  Fs->text_attr=WHITE<<4+PURPLE;
  Fs->border_attr=WHITE<<4+LTPURPLE;
  Fs->win_right=text_cols-2;
  Fs->win_top++;
  TaskDerivedValsUpdate;

  LoadSysStrs;
  MPInit;
  IntInit1;

//Before this point use $LK,"Snd","MN:Snd"$() and $LK,"BusyWait","MN:BusyWait"$()
//to debug.  After this point, use $LK,"DbgI64","MN:DbgI64"$()
  LBts(&sys_run_level,RLf_RAW);
  if (BIOSTotalMem<250*0x100000)
    Dbg("Requires 256 Meg of RAM Memory");

  TimersInit;
  IntsInit;

  "Enable IRQ's\n";
  SetFlags(SYS_NORMAL_RFLAGS);
  BusyWait(2000);
  IntInit2;
  LBts(&sys_run_level,RLf_INTERRUPTS);
  TimeStampFreqCal;
  if (!IsSysDeterministic)
    Preempt(ON);

#exe {
  if (!kernel_cfg->opts[CFG_NO_MP])
    StreamPutS("\"MultiCore Start\\n\\n\";"
	       "MPStart;");
};
  LBts(&sys_run_level,RLf_MP);

  KbdMouseVarsInit;
  IPVarsInit;
  KbdInit;

  if (IsSysDeterministic)
    MouseDriverInstall;
  else if (mp_cnt<2)
    Spawn(&MouseDriverInstall);
  else
    MPSpawn(&MouseDriverInstall);

  "BlkDevsInitAll\n";
  BlkDevsInitAll;
  "BlkDevsInitAll Done\n";
#exe {
  if (kernel_cfg->cdrom_ports_pmt)
    StreamPutS("if (boot_drv_pmt) PutDefine(\"ST_CDROM_PORTS_PMT2\");");
};

  DrvDftSelect;
  LBts(&sys_run_level,RLf_DSK);

  "Loading Compiler\n";
  Cd("/TempleOS/Compiler");
  Load("Compiler",LDF_SILENT);

  time_stamp_freq_initial=TimeStampFreqCal;
  InDbg(OFF);
  LBts(&sys_run_level,RLf_COMPILER);

  Cd("/TempleOS/Kernel");
  try
    ExeFile("Adam1"); //Continues $LK,"/TempleOS/Kernel/Adam1.CPP","FL:/TempleOS/Kernel/Adam1.CPP,1"$
  catch {
    CatchAll;
    Dbg;
  }

  LBts(&sys_run_level,RLf_ADAM_SERVER);
  ServantTaskCont; //Never to return
}

asm {	ALIGN	16,OC_NOP
SYS_KERNEL_END::}
