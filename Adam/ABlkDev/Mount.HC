#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically);"
U8 Mount2(U8 boot_drv_let,CDoc *_doc,Bool _caller_is_prtdsk)
{//If _doc, called by $LK,"::/Kernel/KCfg.HC"$ else called by $LK,"Mount",A="MN:Mount"$().
  I64 cnt,total=0,num_hints,drv_let,type,unit,prt_num;
  U8 blks_buf[STR_LEN],addr_buf[STR_LEN],base0_buf[STR_LEN],base1_buf[STR_LEN],
	*filename=NULL,*filename2=NULL,res=0;
  CATARep *head=NULL,*tmpha;
  Bool whole_drv,make_free;
  CDoc *doc;
  if (boot_drv_let)
    boot_drv_let=Let2Let(boot_drv_let);
  do {
    cnt=0;
    if (!_doc)
      DrvRep;
    "\n****** Mount Drives ******\n"
	  "$$GREEN$$A$$FG$$-$$GREEN$$B$$FG$$ are RAM drives.\n"
	  "$$GREEN$$C$$FG$$-$$GREEN$$L$$FG$$ are ATA hard drives.\n"
	  "$$GREEN$$M$$FG$$-$$GREEN$$P$$FG$$ are ISO file read drives.\n"
	  "$$GREEN$$Q$$FG$$-$$GREEN$$S$$FG$$ are ISO file write drives.\n"
	  "$$GREEN$$T$$FG$$-$$GREEN$$Z$$FG$$ are ATAPI CD/DVD drives.\n"
	  "\nDrive Letter ($$PURPLE$$<ENTER>$$FG$$ to exit):";
    drv_let=Let2Let(GetChar);
    '\n';
    if (type=Let2BlkDevType(drv_let)) {
      whole_drv=FALSE;
      if (_doc) { //Called by $LK,"::/Kernel/KCfg.HC"$
	doc=_doc;
	make_free=FALSE;
      } else { //Called by $LK,"Mount",A="MN:Mount"$()
	doc=DocNew;
	DocPrint(doc,"CBlkDev *bd;\n");
	make_free=TRUE;
      }
      unit=0;
      prt_num=I64_MIN;
      switch (type) {
	case BDT_RAM:
	  "Addr of RAM disk ($$PURPLE$$<ENTER>$$FG$$ to MAlloc):";
	  GetS(addr_buf,STR_LEN);
	case BDT_ISO_FILE_WRITE:
	  "Blks of 512 bytes:";
	  GetS(blks_buf,STR_LEN);
	  break;
	case BDT_ISO_FILE_READ:
	  filename=GetStr("File Name:");
	  break;
	case BDT_ATA:
	  prt_num=GetI64("Partition Num (Default=All):",prt_num);
	case BDT_ATAPI:
	  num_hints=ATARep(,,&head);
	  if (type==BDT_ATAPI && boot_drv_let)
	    "<ENTER> to use booted CD/DVD\n"; //Only $LK,"::/Kernel/KCfg.HC"$
	  do {
	    if (num_hints)
	      "Enter dev number or\nport with $$PURPLE$$0x$$FG$$ prefix.\n"
		    "I/O Port Base0:\n";
	    else
	      "Include $$PURPLE$$0x$$FG$$ prefix.\nI/O Port Base0:\n";
	    GetS(base0_buf,STR_LEN);
	  } while (!Str2I64(base0_buf) && (type!=BDT_ATAPI || !boot_drv_let));
	  if (1<=Str2I64(base0_buf)<=num_hints) {
	    tmpha=ATARepFind(head,Str2I64(base0_buf));
	    StrPrint(base0_buf,"0x%X",tmpha->base0);
	    StrPrint(base1_buf,"0x%X",tmpha->base1);
	    unit=tmpha->unit;
	  } else if (type!=BDT_ATAPI || *base0_buf) {
	    if (type==BDT_ATAPI)
	      StrCpy(base1_buf,"0");
	    else
	      do {
		"I/O Port Base1:\n";
		GetS(base1_buf,STR_LEN);
	      } while (!Str2I64(base1_buf));
	    do {
	      "\t$$PURPLE$$0$$FG$$=Master\n\t$$PURPLE$$1$$FG$$=Slave\nUnit:";
	      unit=GetChar-'0';
	    } while (!(0<=unit<=1));
	    '\n';
	  }
	  LinkedLstDel(head);
	  break;
      }
      DocPrint(doc,"bd=BlkDevNextFreeSlot(\'%C\',%d);bd->unit=%d;\n",
	    drv_let,type,unit);
      switch (type) {
	case BDT_RAM:
	  if (!*addr_buf) StrCpy(addr_buf,"0");
	  DocPrint(doc,"bd->RAM_dsk=%s;\n",addr_buf);
	case BDT_ISO_FILE_WRITE:
	  if (!*blks_buf) StrCpy(blks_buf,"0");
	  DocPrint(doc,"bd->max_blk=(%s)-1;\n",blks_buf);
	  DocPrint(doc,"bd->drv_offset=19<<2+"
		"(DVD_BLK_SIZE*2+DVD_BOOT_LOADER_SIZE)/BLK_SIZE;\n");
	  break;
	case BDT_ISO_FILE_READ:
	  filename2=FileNameAbs(filename);
	  DocPrint(doc,"bd->file_dsk_name=AStrNew(\"%s\");\n",filename2);
	  DocPrint(doc,"bd->drv_offset=19<<2+"
		"(DVD_BLK_SIZE*2+DVD_BOOT_LOADER_SIZE)/BLK_SIZE;\n");
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  if (type==BDT_ATAPI && !*base0_buf) {
	    DocPrint(doc,"GetBaseUnit(bd);\n"); //Only $LK,"::/Kernel/KCfg.HC"$
	    if (drv_let==boot_drv_let)
	      make_free=TRUE;
	  } else
	    DocPrint(doc,"bd->base0=%s;bd->base1=%s;\n",base0_buf,base1_buf);
	  if (type==BDT_ATA && _caller_is_prtdsk) {
	    "\nReformat WHOLE drive!";
	    whole_drv=YorN;
	  }
	  break;
      }
      DocPrint(doc,"BlkDevAdd(bd,%d,%d,%d);\n",prt_num,whole_drv,make_free);
      if (_doc) //Called by $LK,"::/Kernel/KCfg.HC"$
	cnt++;
      else { //Called by $LK,"Mount",A="MN:Mount"$()
	if ((cnt=ExeDoc(doc)) && whole_drv) {
	  if (_caller_is_prtdsk) {
	    res=drv_let;
	    DskPrt(drv_let,1.0); //First mount whole drive.
	  } else
	    DskPrt(drv_let);
	}
	DocDel(doc);
      }
    }
    total+=cnt;
  } while (cnt && !_caller_is_prtdsk ||
	!total && _doc); //At least 1 if Called by $LK,"::/Kernel/KCfg.HC"$
  Free(filename);
  Free(filename2);
  return res;
}

public U8 Mount(Bool caller_is_prtdsk=FALSE)
{//Mount drives. Called from $LK,"DskPrt(Mount)",A="FF:::/Adam/ABlkDev/DskPrt.HC,Mount("$.
  return Mount2(0,NULL,caller_is_prtdsk);
}

public U0 Unmount(U8 drv_let=0)
{//Unmount drive(s).
  BlkDevDel(Let2BlkDev(drv_let));
}

public U8 MountFile(U8 *filename)
{//Mount ISO.C file.
  U8 *filename2=ExtDft(filename,"ISO.C"),*filename3=FileNameAbs(filename2);
  CDrv *dv=DrvMakeFreeSlot(DrvNextFreeLet('M')); //First $LK,"BDT_ISO_FILE_READ",A="MN:BDT_ISO_FILE_READ"$
  CBlkDev *bd=BlkDevNextFreeSlot(dv->drv_let,BDT_ISO_FILE_READ);
  bd->drv_offset=19<<2+(DVD_BLK_SIZE*2+DVD_BOOT_LOADER_SIZE)/BLK_SIZE;
  bd->file_dsk_name=AStrNew(filename3);
  BlkDevAdd(bd,,TRUE,TRUE);
  Free(filename3);
  Free(filename2);
  return dv->drv_let;
}
