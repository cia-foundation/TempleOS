U0 SysGlblsInit()
{
  I64 i,j;
  U8 *m;
  CRAXRBCRCXRDX ee;

  CPUId(0x1,&ee);
  sys_cache_line_width=ee.rbx.u8[1]*8;

  sys_focus_task=Fs;
  QueInit(&sys_macro_root);

  blkdev.dft_iso_filename="::/Temp/CDDVD.ISO";
  blkdev.temp_filename="::/Home/Temp.TXT.Z";
  blkdev.dvd_boot_is_good=TRUE;
  #exe {
    if (!kernel_cfg->auto_mount_ide_hd_let)
      kernel_cfg->auto_mount_ide_hd_let='C';
    if (!kernel_cfg->auto_mount_ide_cd_let)
      kernel_cfg->auto_mount_ide_cd_let='T';
    StreamPrint("blkdev.first_hd_drv_let=%d;",
	  kernel_cfg->auto_mount_ide_hd_let);
    StreamPrint("blkdev.first_dvd_drv_let=%d;",
	  kernel_cfg->auto_mount_ide_cd_let);
  }

  InDbg(ON);
  reverse_bits_table	=CAlloc(256);
  set_bits_table	=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++) {
      if (Bt(&i,7-j))	LBts(reverse_bits_table+i,j);
      if (Bt(&i,j))	set_bits_table[i]++;
    }

  ext=CAlloc(EXT_NUM_ENTRIES*sizeof(U8 *));
  fp_getstr2=&SysGetStr2;
  KeyDevInit;

  #exe {
    StreamPrint("blkdev.boot_drv_let='%C';",kernel_cfg->boot_drv_let);
    StreamPrint("#exe{Option(OPTf_WARN_PAREN,OFF);}");
    StreamPrint("DskCacheInit(%s);",kernel_cfg->dsk_cache_size_exp);
    StreamPrint("#exe{Option(OPTf_WARN_PAREN,ON);}");
  };

  pow10_I64=CAlloc(sizeof(F64)*(308+308+1));
  for (i=-308;i<309;i++)
    pow10_I64[i+309]=Pow10(i);

  QueInit(&snd.record_root);
  snd.record_root.time=tS;

  ProgressBarsRst;

  //$AN,"Independent HeapCtrl Example",A="Independent HeapCtrl Example"$
  i=1; //Rqst 1 blk, we might get more.
  m=Mem2MegUncachedAlloc(&i,sys_code_bp);
  dev.uncached_heap=HeapCtrlBPInit(m,i*0x200000/0x200);
  QueInit(&dev.pci_root);
  dev.mem64_ptr=MEM_MAPPED_SPACE;

  dbg.psect_cache=CAlloc(PSECT_CACHE_SIZE*sizeof(CPSectCache));
  dbg.int_fault_code=IntFaultHandlersNew;
}

U0 SysGrInit()
{
  text.font=sys_font_8x8;
  if (!Bt(&sys_run_level,RLf_VGA)) { //if text mode
    text.cols=80;
    text.rows=25;
    MemSet(VGAM_TEXT,0,text.rows*text.cols<<1);
  } else { //if 640x480 16 color
    text.cols=GR_WIDTH/FONT_WIDTH;
    text.rows=GR_HEIGHT/FONT_HEIGHT;
    OutU8(VGAP_INDEX,VGAR_MAP_MASK);
    OutU8(VGAP_DATA,0x0F);
    MemSet(VGAM_GRAPHICS,0,GR_WIDTH*GR_HEIGHT>>3);
    text.raw_screen_image=CAlloc(GR_WIDTH*GR_HEIGHT/8);
  }
}

U0 TimersInit()
{
  I64 i,*_q;
  U32 *_d;

  OutU8(0x43,0x34);
  OutU8(0x40,SYS_TIMER0_PERIOD);
  OutU8(0x40,SYS_TIMER0_PERIOD>>8);

  //High Precision Event Timer
  if (PCIReadU16(0,31,0,0)==0x8086) {//Intel?
//D31 F0, cfg 0xF0=RCBA of PCI-LPC Bridge
    _d=PCIReadU32(0,31,0,0xF0)(U8 *)&~0x3FFF+0x3404; //HPET cfg
//7	enable
    //1:0 HPET is at 0xFED00000,0xFED01000, 0xFED02000 or 0xFED03000.
    *_d=*_d&3|0x80;
  }

  _q=HPET_GCAP_ID;
  i=*_q; //i.u32[1]= period in femtoS (10e-15)
  if (100000<i.u32[1]<1000000000) {
    cnts.HPET_freq    =1000000000000000/i.u32[1];
    cnts.HPET_kHz_freq=1000000000000/i.u32[1];
    _q=HPET_GEN_CONF;
    *_q|=1; //Enable counting
    cnts.HPET_initial=HPET;
  } else {
    cnts.HPET_freq=0;
    cnts.HPET_kHz_freq=0;
    cnts.HPET_initial=0;
  }
}

U0 Reboot()
{//Hardware reset.
  U16 *_w=0x472;
  CLI
  if (mp_cnt>1)
    MPHalt;
  MemPagePresentMark(_w,TRUE);
  *_w=0;
  OutU8(0x70,0x8F);
  OutU8(0x71,0x00);
  OutU8(0x70,0x00);
  OutU8(0x92,InU8(0x92)|1);
  SysHlt;
}

U0 KernelMain()
{//Continued from $LK,"Kernel",A="FF:::/Kernel/KStart.CPP,I32 &Kernel"$
  OutU8(0x61,InU8(0x61)&~3); //Snd(0);
  adam_task=Fs;
  HeapsInit;
  LBts(&sys_run_level,RLf_FULL_HEAP);
  SysGlblsInit;
  Mem32DevInit;
  MemPagesNotPresentMark;
  LoadKernel;
  SysGrInit;
  StrCpy(Fs->task_name,"Adam Task CPU#0");
  StrCpy(Fs->task_title,Fs->task_name);
  Fs->title_src=TTS_TASK_NAME;
  Fs->win_right=text.cols-2;
  Fs->win_top++;
  TaskDerivedValsUpdate;

  SysDefinesLoad;
  BSPInit;
  IntInit1;

  //Before this point use $LK,"Snd",A="MN:Snd"$() and $LK,"BusyWait",A="MN:BusyWait"$()
  //to debug.  After this point, use $LK,"RawPrint",A="MN:RawPrint"$()
  LBts(&sys_run_level,RLf_RAW);
  TimersInit;
  if (BIOSTotalMem<ToI64(0.95*MEM_MIN_MEG*0x100000))
    RawPrint(4000,"!!! Requires $TX,"512 Meg",D="DD_MEM_MIN_MEG"$ of RAM Memory !!!");

  IntsInit;
  "Enable IRQ's\n";
  SetRFlags(RFLAGG_NORMAL);
  BusyWait(2000);
  IntInit2;
  LBts(&sys_run_level,RLf_INTERRUPTS);

  TimeStampFreqCal;

  "Before BlkDevsInitAll\n";
  BlkDevsInitAll;
  "After  BlkDevsInitAll\n";

  #exe {
    if (kernel_cfg->dvd_ports_pmt)
      StreamPrint(
	    "if (blkdev.boot_rescue_drv_pmt) \"%%S\",\"ST_DVD_PORTS_PMT2\";");
  };

  ChgDsk(':');
  Gs->idle_task->cur_dv=blkdev.let_to_drv[ToUpper(blkdev.boot_drv_let)-'A'];
  LBts(&sys_run_level,RLf_BLKDEV);

  #exe {
    if (!kernel_cfg->opts[CFG_NO_MP])
      StreamPrint("\"MultiCore Start\\n\\n\";"
	    "BSPStartMP;"
	    "LBts(&sys_run_level,RLf_MP);");
  };

  KbdMouseVarsInit;
  IPVarsInit;
  KbdInit;
  if (mp_cnt<2)
    Spawn(&MouseDriverInstall);
  else
    Spawn(&MouseDriverInstall,,,1);

  "Loading Compiler\n";
  Cd("/Compiler");
  Load("Compiler",LDF_SILENT);
  LBts(&sys_run_level,RLf_COMPILER);

  InDbg(OFF);
  cnts.time_stamp_freq_initial=TimeStampFreqCal;
  Cd("/");
  try ExeFile("StartOS"); //Continues $LK,"/StartOS.CPP",A="FL:/StartOS.CPP,1"$
  catch {
    Raw(ON);
    PutExcept;
    Dbg;
  }

  LBts(&sys_run_level,RLf_ADAM_SERVER);
  SrvTaskCont; //Never to return
}

asm {	ALIGN	16,OC_NOP
SYS_KERNEL_END::
#exe {
  if (kernel_cfg->opts[CFG_BOOT_RESCUE_DRV])
    StreamPrint("DU8 0x%X-(SYS_KERNEL_END-SYS_KERNEL+"
	"BOOT_RAM_BASE+sizeof(CBinFile)) DUP (0);"
	"BINFILE \"%s\";",kernel_cfg->boot_rescue_drv_start,
			  kernel_cfg->boot_rescue_drv_file);
};
}
