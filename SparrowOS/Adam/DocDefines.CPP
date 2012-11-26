#help_index "Char/Define"

U0 LoadDocDefines()
{
//These are used in documents such as $LK,"::/SparrowOS/Doc/TrivialSolutions.TXT.Z","FI:::/SparrowOS/Doc/TrivialSolutions.TXT.Z"$
//The $LK,"CDoc","MN:CDoc"$ framework will search the sym table for define string
//constants if you specify that.
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);

  DefinePrintF(
    "DD_OSMAIN","%08X",bfh);
  DefinePrintF(
    "DD_OSMAIN2","%09,X",bfh);
  bfh(I64)+=bfh->file_size-1;
  DefinePrintF(
    "DD_OSMAIN_END","%08X",bfh);
  DefinePrintF(
    "DD_OSMAIN_END2","%09,X",bfh);

  DefinePrintF(
    "DD_MP_VECT","%08X",MP_VECT_ADDR);
  DefinePrintF(
    "DD_MP_VECT_END","%08X",MP_VECT_ADDR+SYS_MP_INIT_END-SYS_MP_INIT_START-1);


  DefinePrintF(
    "DD_SPARROWOS_AGE","%h56D",GetCurTimeCDate-A2CDate("8/1/2003"));

  //This is cut and replaced when I generate a distribution.
  //See $LK,"DD_SPARROWOS_LOC","FF:::/SparrowOS/Demo/AcctExample/TSDistribute.CPP.Z,DD_SPARROWOS_LOC"$.
  $TR-C,"LineCnt"$
$ID,4$DefinePrintF("DD_SPARROWOS_LOC","137,965");
$ID,-4$
  DefinePrintF(
    "DD_PAGE_TABLE_BASE","%08X",PAGE_TABLE_BASE);
  DefinePrintF(
    "DD_PAGE_TABLE_BASE2","%09,X",PAGE_TABLE_BASE);
  DefinePrintF(
    "DD_PAGE_TABLE_END","%08X",PAGE_TABLE_BASE+PAGE_TABLE_SIZE-1);

  DefinePrintF(
    "DD_SYS_HEAP_BASE","%08X",sys_heap_base);
  DefinePrintF(
    "DD_SYS_HEAP_BASE2","%09,X",sys_heap_base);

  DefinePrintF(
    "DD_MAPPED_MEM_SPACE_GIG","%d Gig",MAPPED_MEM_SPACE/0x40000000);
  DefinePrintF(
    "DD_MAPPED_MEM_SPACE_END","%010X",MAPPED_MEM_SPACE-1);
  DefinePrintF(
    "DD_MAPPED_MEM_SPACE_END2","%014,X",MAPPED_MEM_SPACE-1);
}

LoadDocDefines;
#help_index ""
