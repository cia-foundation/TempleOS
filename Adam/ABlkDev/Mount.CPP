#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically);"
I64 Mount2(U8 boot_drv_let,CDoc *_doc,Bool repartition)
{//If _doc, called by $LK,"::/Kernel/KCfg.CPP"$ else called by $LK,"Mount",A="MN:Mount"$().
  I64 cnt,res=0,num_hints,drv_let,type,unit;
  U8 blks_buf[STR_LEN],addr_buf[STR_LEN],base0_buf[STR_LEN],base1_buf[STR_LEN];
  CATARep *root=NULL,*tempha;
  Bool one_drv,make_free;
  CDoc *doc;
  boot_drv_let=ToUpper(boot_drv_let);
  do {
    cnt=0;
    if (!_doc)
      DrvRep;
    "\n****** Mount Drives ******\n"
	  "$$GREEN$$A$$FG$$-$$GREEN$$B$$FG$$ are RAM drives.\n"
	  "$$GREEN$$C$$FG$$-$$GREEN$$L$$FG$$ are ATA hard drives.\n"
	  "$$GREEN$$M$$FG$$-$$GREEN$$S$$FG$$ are file drives.\n"
	  "$$GREEN$$T$$FG$$-$$GREEN$$Z$$FG$$ are ATAPI CD/DVD drives.\n"
	  "\nDrive Letter ($$PURPLE$$<ENTER>$$FG$$ to exit):";
    drv_let=ToUpper(GetChar);
    '\n';
    if (type=Let2BlkDevType(drv_let)) {
      one_drv=FALSE;
      if (_doc) { //Called by $LK,"::/Kernel/KCfg.CPP"$
	doc=_doc;
	make_free=FALSE;
      } else { //Called by $LK,"Mount",A="MN:Mount"$()
	doc=DocNew;
	DocPrint(doc,"CBlkDev *bd;\n");
	make_free=TRUE;
      }
      unit=0;
      switch (type) {
	case BDT_RAM:
	  "Addr of RAM disk ($$PURPLE$$<ENTER>$$FG$$ to MAlloc):";
	  GetS(addr_buf,STR_LEN);
	case BDT_FILE:
	  "Blks of 512 bytes:";
	  GetS(blks_buf,STR_LEN);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  num_hints=ATARep(,,&root);
	  if (type==BDT_ATAPI && boot_drv_let)
	    "<ENTER> to use booted CD/DVD\n"; //Only $LK,"::/Kernel/KCfg.CPP"$
	  do {
	    if (num_hints)
	      "Enter dev number or\nport with $$PURPLE$$0x$$FG$$ prefix.\n"
		    "I/O Port Base0:\n";
	    else
	      "Include $$PURPLE$$0x$$FG$$ prefix.\nI/O Port Base0:\n";
	    GetS(base0_buf,STR_LEN);
	  } while (!Str2I64(base0_buf) && (type!=BDT_ATAPI || !boot_drv_let));
	  if (1<=Str2I64(base0_buf)<=num_hints) {
	    tempha=ATARepFind(root,Str2I64(base0_buf));
	    StrPrint(base0_buf,"0x%X",tempha->base0);
	    StrPrint(base1_buf,"0x%X",tempha->base1);
	    unit=tempha->unit;
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
	  LinkedLstDel(root);
	  break;
      }
      DocPrint(doc,"bd=BlkDevNextFree(\'%C\',%d);bd->unit=%d;\n",
	    drv_let,type,unit);
      switch (type) {
	case BDT_RAM:
	  if (!*addr_buf) StrCpy(addr_buf,"0");
	  DocPrint(doc,"bd->RAM_dsk=%s;\n",addr_buf);
	case BDT_FILE:
	  if (!*blks_buf) StrCpy(blks_buf,"0");
	  DocPrint(doc,"bd->max_blk=(%s)-1;\n",blks_buf);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  if (type==BDT_ATAPI && !*base0_buf) {
	    DocPrint(doc,"GetBaseUnit(bd);\n"); //Only $LK,"::/Kernel/KCfg.CPP"$
	    if (drv_let==boot_drv_let)
	      make_free=TRUE;
	  } else
	    DocPrint(doc,"bd->base0=%s;bd->base1=%s;\n",base0_buf,base1_buf);
	  if (type==BDT_ATA && repartition) {
	    "\nReformat WHOLE drive!";
	    one_drv=YorN;
	  }
	  break;
      }
      DocPrint(doc,"BlkDevAdd(bd,%d,%d);\n",one_drv,make_free);
      if (_doc) //Called by $LK,"::/Kernel/KCfg.CPP"$
	cnt++;
      else { //Called by $LK,"Mount",A="MN:Mount"$()
	if ((cnt=ExeDoc(doc)) && one_drv)
	  PrtDsk(drv_let);
	DocDel(doc);
      }
    }
    res+=cnt;
  } while (cnt || !res && _doc); //At least 1 if Called by $LK,"::/Kernel/KCfg.CPP"$
  return res;
}

public I64 Mount(Bool repartition=FALSE)
{//Mount drives.
  return Mount2(0,NULL,repartition);
}
