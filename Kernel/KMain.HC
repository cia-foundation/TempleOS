U0 SysGlblsInit()
{
  I64 i,j;
  CRAXRBCRCXRDX ee;

  CPUId(0x1,&ee);
  sys_cache_line_width=ee.rbx.u8[1]*8;

  sys_focus_task=Fs;
  QueInit(&sys_macro_head);

  blkdev.dft_iso_filename	=AStrNew(DFT_ISO_FILENAME);
  blkdev.dft_iso_c_filename	=AStrNew(DFT_ISO_C_FILENAME);
  blkdev.tmp_filename		=AStrNew("~/Tmp.DD.Z");
  blkdev.dvd_boot_is_good=TRUE;
  #exe {
    if (!kernel_cfg->mount_ide_auto_hd_let)
      kernel_cfg->mount_ide_auto_hd_let='C';
    if (!kernel_cfg->mount_ide_auto_cd_let)
      kernel_cfg->mount_ide_auto_cd_let='T';
    StreamPrint("blkdev.first_hd_drv_let=%d;",
	  kernel_cfg->mount_ide_auto_hd_let);
    StreamPrint("blkdev.first_dvd_drv_let=%d;",
	  kernel_cfg->mount_ide_auto_cd_let);
  }

  DbgMode(ON);
  rev_bits_table=CAlloc(256);
  set_bits_table=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++) {
      if (Bt(&i,7-j))	LBts(rev_bits_table+i,j);
      if (Bt(&i,j))	set_bits_table[i]++;
    }

  ext=CAlloc(EXT_EXTS_NUM*sizeof(U8 *));
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

  QueInit(&scrncast.snd_head);
  scrncast.t0_now=Now;
  scrncast.t0_tS=tS;
  scrncast.ona=scrncast.snd_head.ona=0;

  ProgressBarsRst;

  QueInit(&dev.pci_head);
  dev.mem64_ptr=mem_mapped_space;

  dbg.fun_seg_cache=CAlloc(FUN_SEG_CACHE_SIZE*sizeof(CFunSegCache));
  dbg.int_fault_code=IntFaultHndlrsNew;
}

U0 SysGrInit()
{
  text.font=sys_font_std;
  text.aux_font=sys_font_cyrillic;
  text.vga_alias	=dev.uncached_alias+VGAM_GRAPHICS;
  text.vga_text_alias	=dev.uncached_alias+VGAM_TEXT;
  if (!Bt(&sys_run_level,RLf_VGA)) { //if text mode
    text.cols=80;
    text.rows=25;
    MemSet(text.vga_text_alias,0,text.rows*text.cols<<1);
    text.border_chars[2] (I64)='ÄÍ³ºÚÉ¿»';
    text.border_chars[10](U32)='ÀÈÙ¼';
  } else { //if 640x480 16 color
    text.cols=GR_WIDTH/FONT_WIDTH;
    text.rows=GR_HEIGHT/FONT_HEIGHT;
    OutU8(VGAP_IDX,VGAR_MAP_MASK);
    OutU8(VGAP_DATA,0x0F);
    MemSet(text.vga_alias,0,GR_WIDTH*GR_HEIGHT>>3);
    text.raw_scrn_image=CAlloc(GR_WIDTH*GR_HEIGHT/8);
    text.border_chars[2] (I64)=0x0908070605040302;
    text.border_chars[10](U32)=0x0D0C0B0A;
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

  _q=dev.uncached_alias+HPET_GCAP_ID;
  i=*_q; //i.u32[1]= period in femtoS (10e-15)
  if (100000<i.u32[1]<1000000000) {
    cnts.HPET_freq    =1000000000000000/i.u32[1];
    cnts.HPET_kHz_freq=1000000000000/i.u32[1];
    _q=dev.uncached_alias+HPET_GEN_CONF;
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
  CLI
  if (mp_cnt>1)
    MPHalt;
  *0x472(U16 *)=0;
  OutU8(0x70,0x8F);
  OutU8(0x71,0x00);
  OutU8(0x70,0x00);
  OutU8(0x92,InU8(0x92)|1);
  SysHlt;
}

U0 KMain()
{//Continued from $LK,"KStart64.HC",A="FF:::/Kernel/KStart64.HC,I32 &KMain"$
  CBlkDev *bd;
  OutU8(0x61,InU8(0x61)&~3); //Snd;
  adam_task=Fs;
  BlkPoolsInit;
  SysGlblsInit;
  Mem32DevInit;
  UncachedAliasAlloc;
  LoadKernel;
  SysGrInit;
  StrCpy(Fs->task_name,"Adam Task CPU00");
  StrCpy(Fs->task_title,Fs->task_name);
  Fs->title_src=TTS_TASK_NAME;
  Fs->win_right=text.cols-2;
  Fs->win_top++;
  TaskDerivedValsUpdate;

  SysDefinesLoad;
  Core0Init;
  IntInit1;

  //Before this point use $LK,"Snd",A="MN:Snd"$() and $LK,"Busy",A="MN:Busy"$()
  //to debug.  After this point, use $LK,"RawPrint",A="MN:RawPrint"$()
  LBts(&sys_run_level,RLf_RAW);
  "TempleOS V%5.3f\t%D %T\n\n",
	sys_os_version,sys_compile_time,sys_compile_time;

  TimersInit;
  if (BIOSTotalMem<ToI64(0.95*MEM_MIN_MEG*0x100000))
    RawPrint(4000,"!!! Requires $TX,"512Meg",D="DD_MEM_MIN_MEG"$ of RAM Memory !!!");

  IntsInit;
  "Enable IRQ's\n";
  SetRFlags(RFLAGG_NORMAL);
  Busy(2000);
  IntInit2;
  LBts(&sys_run_level,RLf_INTERRUPTS);

  TimeCal;
  BlkDevsInitAll;
  "DskChg(':');\n";
  DskChg(':');
  #exe {
    StreamPrint("HomeSet(\"%s\");\n"
	  "blkdev.ins_base0=%d;blkdev.ins_base1=%d;blkdev.ins_unit =%d;\n",
	  kernel_cfg->home_dir,
	  blkdev.ins_base0,blkdev.ins_base1,blkdev.ins_unit);
  }
  Gs->idle_task->cur_dv=blkdev.let_to_drv[*blkdev.home_dir-'A'];
  DrvRep;
  if (blkdev.dvd_boot_is_good) {
    bd=Let2BlkDev(':');
    if (bd->type==BDT_ATAPI) {
      blkdev.ins_base0=bd->base0;
      blkdev.ins_base1=bd->base1;
      blkdev.ins_unit =bd->unit;
    }
  }
  LBts(&sys_run_level,RLf_BLKDEV);

  #exe {
    if (!kernel_cfg->opts[CFG_NO_MP])
      StreamPrint("\"MultiCore Start\\n\\n\";"
	    "Core0StartMP;"
	    "LBts(&sys_run_level,RLf_MP);");
  };

  KbdMsInit;
  MsInit;
  KbdInit;
  Spawn(&MsHardDrvrInstall);

  "Loading Compiler\n";
  Cd("/Compiler");
  Load("Compiler",LDF_SILENT);
  LBts(&sys_run_level,RLf_COMPILER);

  DbgMode(OFF);
  cnts.time_stamp_freq_initial=TimeCal;
  Cd("/");
  try ExeFile("StartOS"); //Continues $LK,"/StartOS.HC",A="FL:/StartOS.HC,1"$
  catch {
    Raw(ON);
    Silent(OFF);
    GetOutOfDollar;
    PutExcept;
    Dbg;
  }

  LBts(&sys_run_level,RLf_ADAM_SERVER);
  SrvTaskCont; //Never to return
}

asm {	ALIGN	16,OC_NOP
SYS_KERNEL_END::
#exe {
  if (kernel_cfg->opts[CFG_DBG_DISTRO])
    StreamPrint("DU8 0x%X-(SYS_KERNEL_END-SYS_KERNEL+"
	"BOOT_RAM_BASE+sizeof(CBinFile)) DUP (0);"
	"BINFILE \"%s\";",kernel_cfg->dbg_distro_start,
			  kernel_cfg->dbg_distro_file);
};
}
