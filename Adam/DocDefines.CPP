#help_index "Define;Char/Define"

U0 LoadDocDefines()
{//These are used in documents such as $LK,"::/Doc/TrivialSolutions.TXT"$
//The $LK,"CDoc",A="MN:CDoc"$ framework will search the sym table for define string
//consts if you specify that.
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);

  DefinePrint(
    "DD_KERNEL","%08X",bfh);
  DefinePrint(
    "DD_KERNEL2","%09,X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint(
    "DD_KERNEL_END","%08X",bfh);
  DefinePrint(
    "DD_KERNEL_END2","%09,X",bfh);

  DefinePrint(
    "DD_PROTECTED_LOW","%08X",MEM_PROTECTED_LOW_LIMIT);
  DefinePrint(
    "DD_PROTECTED_LOW2","%09,X",MEM_PROTECTED_LOW_LIMIT);

  //$LK,"DD_BOOT_HIGH_LOC",A="FF:::/Adam/Boot/BootCD.CPP,DD_BOOT_HIGH_LOC"$

  DefinePrint(
    "DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrint(
    "DD_MP_VECT_END","%08X",MP_VECT_ADDR+SYS_MP_INIT_END-SYS_MP_INIT_START-1);

  DefinePrint(
    "DD_OS_NAME_VERSION","TempleOS V%0.2f",os_version);
  DefinePrint(
    "DD_TEMPLEOS_AGE","%0.1f",(Now-Str2Date("8/1/2003"))/ToF64(1<<32)/CDATE_YEAR_DAYS);

  //This is cut and replaced when I generate a distro.
  //See $LK,"DD_TEMPLEOS_LOC",A="FF:::/Demo/AcctExample/TSDistro.CPP,DD_TEMPLEOS_LOC"$.
  $TR-C,"LineRep"$
$ID,4$DefinePrint("DD_TEMPLEOS_LOC","121,504");
DefinePrint("DD_TEMPLEOS_LOC_OFFICIAL","81,303");
$ID,-4$
  DefinePrint(
    "DD_FIXED_AREA_BASE","%08X",SYS_FIXED_AREA);
  DefinePrint(
    "DD_FIXED_AREA_BASE2","%09,X",SYS_FIXED_AREA);
  DefinePrint(
    "DD_FIXED_AREA_END","%08X",SYS_FIXED_AREA+sizeof(CSysFixedArea)-1);

  DefinePrint(
    "DD_SYS_HEAP_BASE","%08X",sys_heap_base);
  DefinePrint(
    "DD_SYS_HEAP_BASE2","%09,X",sys_heap_base);

  DefinePrint(
    "DD_SYS_HEAP_LIMIT","%08X",sys_heap_limit);
  DefinePrint(
    "DD_SYS_HEAP_LIMIT2","%09,X",sys_heap_limit);

  DefinePrint(
    "DD_MAPPED_MEM_SPACE_GIG","%d Gig",MEM_MAPPED_SPACE/0x40000000);
  DefinePrint(
    "DD_MAPPED_MEM_SPACE_END","%010X",MEM_MAPPED_SPACE-1);
  DefinePrint(
    "DD_MAPPED_MEM_SPACE_END2","%014,X",MEM_MAPPED_SPACE-1);

  DefinePrint(
    "DD_JIFFY_HZ","%d Hz",JIFFY_FREQ);
}

LoadDocDefines;
