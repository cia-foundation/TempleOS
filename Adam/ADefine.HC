#help_index "Define;Char/Define"

U0 LoadDocDefines()
{
  CBinFile *bfh=mem_boot_base-sizeof(CBinFile);

  DefinePrint("DD_OS_NAME_VERSION","TempleOS V%0.2f",sys_os_version);
  DefinePrint("DD_TEMPLEOS_AGE","%0.1f",
	(Now-Str2Date("8/1/2003"))/ToF64(1<<32)/CDATE_YEAR_DAYS);

  DefinePrint("DD_KERNEL","%08X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("DD_KERNEL_END","%08X",bfh);

  //$LK,"DD_BOOT_HIGH_LOC_DVD",A="FF:::/Adam/Opt/Boot/BootDVD.HC,DD_BOOT_HIGH_LOC_DVD"$

$TR,"LineRep"$
$ID,2$DefinePrint("DD_TEMPLEOS_LOC","80,849");
$ID,-2$
  DefinePrint("DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrint("DD_MP_VECT_END","%08X",
	MP_VECT_ADDR+COREAP_16BIT_INIT_END-COREAP_16BIT_INIT-1);

  DefinePrint("DD_SYS_FIXED_AREA_BASE","%08X",SYS_FIXED_AREA);
  DefinePrint("DD_SYS_FIXED_AREA_END","%08X",
	SYS_FIXED_AREA+sizeof(CSysFixedArea)-1);
  DefinePrint("DD_MEM_HEAP_BASE","%08X",mem_heap_base);
  DefinePrint("DD_MEM_HEAP_LIMIT","%08X",mem_heap_limit);
  DefinePrint("DD_MEM_MIN_MEG","%dMeg",MEM_MIN_MEG);
  DefinePrint("DD_UNCACHED_ALIAS","%010X",dev.uncached_alias);
  DefinePrint("DD_PHYSICAL_SPACE_END","%010X",
	mem_physical_space-1);
  DefinePrint("DD_MAPPED_SPACE_END","%010X",
	mem_mapped_space-1);
  DefinePrint("DD_JIFFY_HZ","%dHz",JIFFY_FREQ);
}

LoadDocDefines;
