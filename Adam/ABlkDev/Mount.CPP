#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically);"
CDoc *BlkDevPmt(U8 boot_drv_let=0,Bool make_free=FALSE,Bool repartition=FALSE)
{
  I64 num_hints,drv_let,type,unit;
  U8 blks_buf[STR_LEN],addr_buf[STR_LEN],base0_buf[STR_LEN],base1_buf[STR_LEN];
  CATARep *root=NULL,*tempha;
  CDoc *doc;
  "\n****** Mount Drives ******\n"
	"$$GREEN$$A$$FG$$-$$GREEN$$B$$FG$$ are RAM drives.\n"
	"$$GREEN$$C$$FG$$-$$GREEN$$S$$FG$$ are hard drives.\n"
	"$$GREEN$$T$$FG$$-$$GREEN$$Z$$FG$$ are CD/DVD drives.\n"
	"\nDrive Letter ($$PURPLE$$<ENTER>$$FG$$ to exit):";
  drv_let=ToUpper(GetChar);
  '\n';
  if ('A'<=drv_let<='Z') {
    doc=DocNew;
    unit=0;
    if ('A'<=drv_let<='B') {
      type=BDT_RAM;
      "Addr of RAM disk ($$PURPLE$$<ENTER>$$FG$$ to MAlloc):";
      GetS(addr_buf,STR_LEN);
      "Blks of 512 bytes:";
      GetS(blks_buf,STR_LEN);
    } else {
      num_hints=ATARep(,,&root);
      if ('C'<=drv_let<='S')
	type=BDT_ATA;
      else
	type=BDT_ATAPI;
      if (type==BDT_ATAPI && boot_drv_let)
	"<ENTER> to use booted CD/DVD\n";
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
    }
    DocPrint(doc,"bd=BlkDevNextFree(%d,%d);bd->unit=%d;\n",drv_let,type,unit);
    if (type==BDT_RAM) {
      if (!*addr_buf) StrCpy(addr_buf,"0");
      DocPrint(doc,"bd->RAM_dsk=%s;bd->max_blk=(%s)-1;\n",addr_buf,blks_buf);
    }
    if (type==BDT_ATA || type==BDT_ATAPI) {
      if (type==BDT_ATAPI && !*base0_buf) {
	DocPrint(doc,"GetBaseUnit(bd);\n");
	if (drv_let==boot_drv_let)
	  make_free=TRUE;
      } else
	DocPrint(doc,"bd->base0=%s;bd->base1=%s;\n",
	      base0_buf,base1_buf);
    }
    if (type==BDT_ATA && repartition) {
      "\nReformat WHOLE drive!";
      if (YorN) {
	DocPrint(doc,"if (!BlkDevAdd(bd,TRUE,TRUE))"
	      "\"\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n\";"
	      "else PrtDsk(bd->first_drv_let);\n");
	return doc;
      }
    }
    DocPrint(doc,"if (!BlkDevAdd(bd,FALSE,%d))"
	  "\"\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n\";\n",make_free);
    return doc;
  } else
    return NULL;
}

public U0 Mount(Bool repartition=FALSE)
{//Mount a drive.
  CDoc *doc;
  ExePrint("CBlkDev *bd;"); //Leaks
  do {
    DrvRep;
    if (doc=BlkDevPmt(,TRUE,repartition)) {
      ExeDoc(doc);
      DocDel(doc);
    }
  } while (doc);
}
