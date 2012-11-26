#help_index "PCI;Processor;Device;Info"

//The file was downloaded from
//http://www.pcidatabase.com/reports.php?type=tab-delimeted

#define PCI_DEV_FILE "::/SparrowOS/Misc/PCIDevices.TXT.Z"

/****
public U0 PCIDevFileGen()
{
  Bool first=TRUE,del=FALSE,del2=FALSE,cont=FALSE;
  CDoc *doc=DocRead(PCI_DEV_FILE,DOCF_PLAIN_TEXT|DOCF_DBL_DOLLARS);
  CDocEntry *doc_e=doc->root.next,*doc_e2;
  while (doc_e!=doc) {
    doc_e2=doc_e->next;
    if (first) {
      if (doc_e->type_u8==DOCT_TEXT) {
	if (doc_e->tag[0]==';')
	  del=TRUE;
      }
      first=FALSE;
    }
    if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[StrLen(doc_e->tag)-1]=='=' &&
	doc_e2->type_u8==DOCT_NEW_LINE) {
      doc_e->tag[StrLen(doc_e->tag)-1]=CH_SPACE;
      cont=TRUE;
    }
    del2=del;
    if (doc_e->type_u8==DOCT_NEW_LINE) {
      first=TRUE;
      del2=FALSE;
      if (cont) {
	del=TRUE;
	cont=FALSE;
      }
    }
    if (del)
      DocEntryDel(doc,doc_e);
    del=del2;
    doc_e=doc_e2;
  }
  DocWrite(doc);
}
****/

//$LK,"::/SparrowOS/Misc/PCIDevices.TXT.Z","PI:::/SparrowOS/Misc/PCIDevices.TXT.Z"$
U0 PCILookUpSingle(CDoc *doc,I64 m,I64 d,U8 **_vendor,U8 **_dev)
{
  Bool first=TRUE;
  U8 buf[8],*vendor=NULL,*dev=NULL;
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (first) {
      if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[0]!=';' && StrLen(doc_e->tag)>=4) {
	buf[0]='0';
	buf[1]='x';
	buf[2]=doc_e->tag[0];
	buf[3]=doc_e->tag[1];
	buf[4]=doc_e->tag[2];
	buf[5]=doc_e->tag[3];
	buf[6]=0;
	if (A2I(buf)==m) {
	  doc_e=doc_e->next->next;
	  if (doc_e->type_u8==DOCT_TEXT) {
	    vendor=AStrNew(doc_e->tag);
	    first=FALSE;
	    break;
	  }
	}
      }
      first=FALSE;
    }
    if (doc_e->type_u8==DOCT_NEW_LINE)
      first=TRUE;
    doc_e=doc_e->next;
  }

  if (vendor) {
    while (doc_e!=doc) {
      if (first) {
	if (doc_e->type_u8==DOCT_TAB) {
	  doc_e=doc_e->next;
	  if (doc_e->type_u8==DOCT_TEXT && StrLen(doc_e->tag)>=4) {
	    buf[0]='0';
	    buf[1]='x';
	    buf[2]=doc_e->tag[0];
	    buf[3]=doc_e->tag[1];
	    buf[4]=doc_e->tag[2];
	    buf[5]=doc_e->tag[3];
	    buf[6]=0;
	    if (A2I(buf)==d) {
	      doc_e=doc_e->next->next;
	      if (doc_e->type_u8==DOCT_TEXT) {
		dev=AStrNew(doc_e->tag);
		break;
	      }
	    }
	  }
	} else
	  break;
	first=FALSE;
      }
      if (doc_e->type_u8==DOCT_NEW_LINE)
	first=TRUE;
      doc_e=doc_e->next;
    }
  }

  if (vendor)
    *_vendor=vendor;
  else
    *_vendor=AStrNew("Unknown");

  if (dev)
    *_dev=dev;
  else
    *_dev=AStrNew("Unknown");
}

U0 PCILookUpDevs()
{
  CPCIDev *temppci;
  U16 w1,w2;
  I64 b,d,f,timeout=32*8*2;
  CDoc *doc;
  if (sys_pci_dev_root.next!=&sys_pci_dev_root)
    return;
  doc=DocRead(PCI_DEV_FILE,DOCF_PLAIN_TEXT);
  for (b=0;b<sys_pci_busses;b++)
    for (d=0;d<32;d++)
      for (f=0;f<8;f++) {
	w1=PCIReadU16(b,d,f,0);
	if (w1!=0xFFFF) {
	  temppci=ACAlloc(sizeof(CPCIDev));
	  temppci->bus=b;
	  temppci->dev=d;
	  temppci->fun=f;
	  temppci->vendor=w1;
	  temppci->dev_id=w2=PCIReadU16(b,d,f,2);
	  temppci->sub_code=PCIReadU8(b,d,f,0xA);
	  temppci->base_code=PCIReadU8(b,d,f,0xB);
	  PCILookUpSingle(doc,w1,w2,&temppci->vendor_str,&temppci->dev_id_str);
	  QueIns(temppci,sys_pci_dev_root.last);
	  timeout=32*8*2;
	} else if (sys_pci_busses==256 && --timeout<=0)
	  goto lud_done;
      }
lud_done:
  DocDel(doc);
}

public U0 PCIRep()
{
  CPCIDev *temppci;
  "PCI Busses:%d\n",sys_pci_busses;
  if (!FileFind(PCI_DEV_FILE)) {
    "You don't have the PCI device file.\n";
    return;
  }
  PCILookUpDevs;
  temppci=sys_pci_dev_root.next;
  while (temppci!=&sys_pci_dev_root) {
    "%02X:%02X:%01X %02X%02X $$FG,GREEN$$%s $$FG,CYAN$$%s$$FG$$\n",
      temppci->bus,temppci->dev,temppci->fun,
      temppci->base_code,temppci->sub_code,
      temppci->vendor_str,temppci->dev_id_str;
    temppci=temppci->next;
  }
}

#help_index "Info;Time/CPU Cycles"
public U0 CPURep()
{
  "%d Cores %6h?nHz\n",mp_cnt,ToF64(time_stamp_freq);
}

#help_index "PCI;Info;File/System/Hardware;Device"
U0 ATARepEntry(I64 base0,I64 base1,I64 unit,I64 irq,U8 *msg,CATARep **_root,I64 *num_hints)
{
  I64 type;
  base0&=-8;
  base1&=-4;
  CATARep *tempha;
  if (type=ATAProbe(base0,base1,unit)) {
    *num_hints+=1;
    "\n\n$$FG,PURPLE$$ $$BT-LE+LM+X,\"%d\",\"%d\\n\"$$$$FG$$$$LM,4$$",*num_hints,*num_hints;
    if (type==BDT_ATA)
      "$$FG,RED$$ATA   ";
    else
      "$$FG,RED$$ATAPI ";
    "$$FG,LTBLUE$$%s$$FG$$\nBase0: 0x%04X\nBase1: 0x%04X\nIrq: 0x%02X Unit:%d$$LM,0$$\n",msg,base0,base1,irq,unit;
    if (_root) {
      tempha=CAlloc(sizeof(CATARep));
      tempha->next=*_root;
      *_root=tempha;
      tempha->num=*num_hints;
      tempha->type=type;
      tempha->base0=base0;
      tempha->base1=base1;
      tempha->unit=unit;
    }
  }
}

Bool hint_ata_disable=FALSE;

U0 ATARepExitAllApplications()
{
  I64 ch;
  PutDefine("ST_GOING_TO_PROBE_HARDWARE");
  PutDefine("ST_EXIT_ALL_APPLICATIONS");
  PutDefine("ST_PRESS_A_KEY");
  "Press '$$FG,PURPLE$$p$$FG$$' to probe or '$$FG,PURPLE$$s$$FG$$' to skip.\n";
  ch=ToUpper(GetChar(NULL,FALSE));
  if (ch=='S')
    hint_ata_disable=TRUE;
  else
    hint_ata_disable=FALSE;
}

public I64 ATARep(Bool pmt=TRUE,CATARep **_root=NULL)
{
  U32 d1,d2;
  U8 d3;
  I64 i,unlock_flags=0,num_hints=0;
#assert NUM_BLKDEVS<=64
  Bool old_preempt;

  if (_root) *_root=NULL;

  if (pmt)
    ATARepExitAllApplications;
  if (hint_ata_disable)
    return 0;

  old_preempt=Preempt(OFF);
  for (i=0;i<num_blkdevs;i++)
    BEqu(&unlock_flags,i,
      BlkDevLock(&sys_blkdevs[i]));

  if (PCIReadU16(0,31,1,0)==0x8086) {
    d1=PCIReadU32(0,31,1,0x10);
    d2=PCIReadU32(0,31,1,0x14);
    d3=PCIReadU8(0,31,1,0x3C);
    if (d1&1 && d2&1) {
      ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
      ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);
    }
    d1=PCIReadU32(0,31,1,0x18);
    d2=PCIReadU32(0,31,1,0x1C);
    if (d1&1 && d2&1) {
      ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
      ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
    }
  } else {
    d1=0x1F0;
    d2=0x3F6;
    d3=14;
    ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);

    d1=0x170;
    d2=0x376;
    d3=15;
    ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
  }
  if (PCIReadU16(0,31,2,0)==0x8086) {
    d1=PCIReadU32(0,31,2,0x10);
    d2=PCIReadU32(0,31,2,0x14);
    d3=PCIReadU8(0,31,2,0x3C);
    if (d1&1 && d2&1) {
      ATARepEntry(d1,d2,0,d3,"Primary SATA",_root,&num_hints);
      ATARepEntry(d1,d2,1,d3,"Primary SATA",_root,&num_hints);
    }
    d1=PCIReadU32(0,31,2,0x18);
    d2=PCIReadU32(0,31,2,0x1C);
    if (d1&1 && d2&1) {
      ATARepEntry(d1,d2,0,d3,"Secondary SATA",_root,&num_hints);
      ATARepEntry(d1,d2,1,d3,"Secondary SATA",_root,&num_hints);
    }
  }
  NewLine(2);
  for (i=0;i<num_blkdevs;i++)
    if (Bt(&unlock_flags,i))
     BlkDevUnlock(&sys_blkdevs[i]);
  Preempt(old_preempt);
  return num_hints;
}

#help_index "Info;Memory/Info"
public U0 MemBIOSRep()
{
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  CMemRange	*tempmr;
	"Standard Addresses\n"
	"0x000A0000-0x000BFFFF VGA\n"
	"0xFEE00000-0xFEE00FFF See $$LK,\"APIC\",\"MN:LAPIC_BASE\"$$\n\n"
	"32 Bit Device Mem\n";
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr=sys_mem32_dev_root.next;
  while (tempmr!=&sys_mem32_dev_root) {
    "%02X:0x%016X-0x%016X\n",tempmr->type,tempmr->base,tempmr->base+tempmr->size-1;
    tempmr=tempmr->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);

  "\nBIOS Memory Report 15:E801\n"
  "01:0x0000000000000000-0x%016X\n",0x100000+m01[0]<<10-1;
  "01:0x0000000001000000-0x%016X\n",0x1000000+m01[1]<<16-1;

  if (m20->type) {
    NewLine;
    "BIOS Memory Report 15:E820\n";
    while (m20->type) {
      "%02X:0x%016X-0x%016X\n",m20->type,m20->base,m20->base+m20->len-1;
      m20++;
    }
  }

}

#help_index ""
