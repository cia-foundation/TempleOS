#help_index "Define;Char/Define"

U0 LoadDocDefines()
{
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);

  DefinePrint("DD_OS_NAME_VERSION","TempleOS V%0.2f",sys_os_version);
  DefinePrint("DD_TEMPLEOS_AGE","%0.1f",
	(Now-Str2Date("8/1/2003"))/ToF64(1<<32)/CDATE_YEAR_DAYS);

  //This is cut and replaced when I generate a distro.
  //See $LK,"DD_TEMPLEOS_LOC",A="FF:::/Demo/AcctExample/TOSDistro.HC,DD_TEMPLEOS_LOC"$.
  $TR-C,"LineRep"$
  $ID,4$DefinePrint("DD_TEMPLEOS_LOC","120,770");
DefinePrint("DD_TEMPLEOS_LOC_OFFICIAL","81,303");
$ID,-4$
  DefinePrint("DD_KERNEL","%08X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("DD_KERNEL_END","%08X",bfh);

  //$LK,"DD_BOOT_HIGH_LOC_DVD",A="FF:::/Adam/Opt/Boot/BootDVD.HC,DD_BOOT_HIGH_LOC_DVD"$

  DefinePrint("DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrint("DD_MP_VECT_END","%08X",
	MP_VECT_ADDR+COREAP_16BIT_INIT_END-COREAP_16BIT_INIT-1);

  DefinePrint("DD_SYS_FIXED_AREA_BASE","%08X",SYS_FIXED_AREA);
  DefinePrint("DD_SYS_FIXED_AREA_END","%08X",
	SYS_FIXED_AREA+sizeof(CSysFixedArea)-1);
  DefinePrint("DD_SYS_HEAP_BASE","%08X",sys_heap_base);
  DefinePrint("DD_SYS_HEAP_LIMIT","%08X",sys_heap_limit);
  DefinePrint("DD_MEM_MIN_MEG","%dMeg",MEM_MIN_MEG);
  DefinePrint("DD_UNCACHED_ALIAS","%010X",dev.uncached_alias);
  DefinePrint("DD_MEM_MAPPED_SPACE_GIG","%dGig",MEM_MAPPED_SPACE/0x40000000);
  DefinePrint("DD_MEM_MAPPED_SPACE_END","%010X",MEM_MAPPED_SPACE-1);
  DefinePrint("DD_JIFFY_HZ","%dHz",JIFFY_FREQ);
}

LoadDocDefines;
