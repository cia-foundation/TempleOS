#help_index "Char/Define"

U0 LoadDocDefines()
{
//These are used in documents such as $LK-A,"::/TempleOS/Doc/TrivialSolutions.TXT"$
//The $LK,"CDoc","MN:CDoc"$ framework will search the sym table for define string
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
  bfh(I64)++;
  DefinePrint(
    "DD_COMPILER","%08X",bfh);
  DefinePrint(
    "DD_COMPILER2","%09,X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint(
    "DD_COMPILER_END","%08X",bfh);
  DefinePrint(
    "DD_COMPILER_END2","%09,X",bfh);

  //$LK,"DD_BOOT_HIGH_LOC","FF:::/TempleOS/Adam/Boot/BootCD.CPP.Z,DD_BOOT_HIGH_LOC"$

  DefinePrint(
    "DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrint(
    "DD_MP_VECT_END","%08X",MP_VECT_ADDR+SYS_MP_INIT_END-SYS_MP_INIT_START-1);

  DefinePrint(
    "DD_OS_NAME_VERSION","TempleOS V%0.2f",os_version);
  DefinePrint(
    "DD_TEMPLEOS_AGE","%h56D",GetDateTime-A2Date("8/1/2003"));

  //This is cut and replaced when I generate a distro.
  //See $LK,"DD_TEMPLEOS_LOC","FF:::/TempleOS/Demo/AcctExample/TSDistro.CPP,DD_TEMPLEOS_LOC"$.
  $TR-C,"LineCnt"$
$ID,4$DefinePrint("DD_TEMPLEOS_LOC","124,033");
$ID,-4$
  DefinePrint(
    "DD_PAGE_TABLE_BASE","%08X",PAGE_TABLE_BASE);
  DefinePrint(
    "DD_PAGE_TABLE_BASE2","%09,X",PAGE_TABLE_BASE);
  DefinePrint(
    "DD_PAGE_TABLE_END","%08X",PAGE_TABLE_BASE+PAGE_TABLE_SIZE-1);

  DefinePrint(
    "DD_SYS_HEAP_BASE","%08X",sys_heap_base);
  DefinePrint(
    "DD_SYS_HEAP_BASE2","%09,X",sys_heap_base);

  DefinePrint(
    "DD_SYS_HEAP_LIMIT","%08X",sys_heap_limit);
  DefinePrint(
    "DD_SYS_HEAP_LIMIT2","%09,X",sys_heap_limit);

  DefinePrint(
    "DD_MAPPED_MEM_SPACE_GIG","%d Gig",MAPPED_MEM_SPACE/0x40000000);
  DefinePrint(
    "DD_MAPPED_MEM_SPACE_END","%010X",MAPPED_MEM_SPACE-1);
  DefinePrint(
    "DD_MAPPED_MEM_SPACE_END2","%014,X",MAPPED_MEM_SPACE-1);
}

LoadDocDefines;
#help_index ""
