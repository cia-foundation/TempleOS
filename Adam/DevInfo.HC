#help_index "PCI;Processor;Devices;Info"

//The file was downloaded from
//http://www.pcidatabase.com/reports.php?type=tab-delimeted

#define PCI_DEV_FILE "::/Misc/PCIDevices.DD.Z"

/****
//1) Plain text edit and remove file header and tail
//2) Text find-and-replace "=0A=" with "".
//3) Run PCIDevFileGen().

public U0 PCIDevFileGen()
{
  Bool first=TRUE,del=FALSE,del2=FALSE,cont=FALSE;
  CDoc *doc=DocRead(PCI_DEV_FILE,
	DOCF_PLAIN_TEXT|DOCF_DBL_DOLLARS|DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->head.next,*doc_e2;
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

//$LK,"::/Misc/PCIDevices.DD",A="PI:::/Misc/PCIDevices.DD"$
U0 PCILookUpSingle(CDoc *doc,I64 m,I64 d,U8 **_vendor,U8 **_dev)
{
  Bool first=TRUE;
  U8 buf[8],*vendor=NULL,*dev=NULL;
  CDocEntry *doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (first) {
      if (doc_e->type_u8==DOCT_TEXT && doc_e->tag[0]!=';' &&
	    StrLen(doc_e->tag)>=4) {
	buf[0](U16)='0x';
	buf[2](U32)=doc_e->tag(U32 *)[0];
	buf[6]=0;
	if (Str2I64(buf)==m) {
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
	    buf[0](U16)='0x';
	    buf[2](U32)=doc_e->tag(U32 *)[0];
	    buf[6]=0;
	    if (Str2I64(buf)==d) {
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
  CPCIDev *tmppci;
  I64 w1,w2,b,d,f,timeout=32*8*2;
  CDoc *doc;
  if (dev.pci_head.next!=&dev.pci_head)
    return;
  doc=DocRead(PCI_DEV_FILE,DOCF_PLAIN_TEXT|DOCF_NO_CURSOR);
  for (b=0;b<sys_pci_busses;b++)
    for (d=0;d<32;d++)
      for (f=0;f<8;f++) {
	w1=PCIReadU16(b,d,f,0);
	if (w1!=0xFFFF) {
	  tmppci=ACAlloc(sizeof(CPCIDev));
	  tmppci->bus=b;
	  tmppci->dev=d;
	  tmppci->fun=f;
	  tmppci->vendor=w1;
	  tmppci->dev_id=w2=PCIReadU16(b,d,f,2);
	  tmppci->sub_code=PCIReadU8(b,d,f,0xA);
	  tmppci->base_code=PCIReadU8(b,d,f,0xB);
	  PCILookUpSingle(doc,w1,w2,&tmppci->vendor_str,&tmppci->dev_id_str);
	  QueIns(tmppci,dev.pci_head.last);
	  timeout=32*8*2;
	} else if (sys_pci_busses==256 && --timeout<=0)
	  goto lud_done;
      }
lud_done:
  DocDel(doc);
}

public U0 PCIRep()
{//Report description of PCI devices.
  CPCIDev *tmppci;
  "PCI Busses:%d\n",sys_pci_busses;
  if (!FileFind(PCI_DEV_FILE)) {
    "You don't have the PCI device file.\n";
    return;
  }
  PCILookUpDevs;
  tmppci=dev.pci_head.next;
  while (tmppci!=&dev.pci_head) {
    "%02X:%02X:%01X %02X%02X $$GREEN$$%s $$CYAN$$%s$$FG$$\n",
	  tmppci->bus,tmppci->dev,tmppci->fun,
	  tmppci->base_code,tmppci->sub_code,
	  tmppci->vendor_str,tmppci->dev_id_str;
    tmppci=tmppci->next;
  }
}

#help_index "Info;Time/CPU Cycles"
class CCPURep
{
  Bool mp_start,mp_end;
  I64 mp_not_done_flags,
      **swaps,
      **cycles;
};

U0 MPCPURep(CCPURep *cr)
{
  I64 swaps=0,cycles_0,cycles_f;
  while (!cr->mp_start)
    Yield;
  cycles_0=GetTSC;
  while (!cr->mp_end) {
    swaps++;
    Yield;
  }
  cycles_f=GetTSC;
  cr->swaps[Gs->num]=swaps;
  cr->cycles[Gs->num]=cycles_f-cycles_0;
  LBtr(&cr->mp_not_done_flags,Gs->num);
}

public U0 CPURep(Bool full=FALSE)
{//Report number of cores and clock freq.
  I64 i,total_swaps,total_cycles;
  F64 t0,tf;
  CCPURep cr;

  if (!full)
    "%d Cores %6h?nHz\n",mp_cnt,ToF64(cnts.time_stamp_freq);
  else {
    cr.swaps=CAlloc(sizeof(I64)*mp_cnt);
    cr.cycles=CAlloc(sizeof(I64)*mp_cnt);
    cr.mp_start=cr.mp_end=FALSE;
    cr.mp_not_done_flags=1<<mp_cnt-1;
    for (i=0;i<mp_cnt;i++)
      Spawn(&MPCPURep,&cr,NULL,i);

    t0=tS;
    cr.mp_start=TRUE;
    Sleep(2000);
    cr.mp_end=TRUE;
    while (cr.mp_not_done_flags)
      Yield;
    tf=tS;

    "\nCPU: %d Cores %6h?nHz\n",mp_cnt,ToF64(cnts.time_stamp_freq);
    "\n         Context\n"
	  "Core     Swaps/s         Cycles\n"
	  "---- ------------- -----------------\n";
    total_swaps=0;
    total_cycles=0;
    for (i=0;i<mp_cnt;i++) {
      "%4d %13,d %17,d\n",i,
	    ToI64(cr.swaps[i]/(tf-t0)),ToI64(cr.cycles[i]/(tf-t0));
      total_swaps+=cr.swaps[i];
      total_cycles+=cr.cycles[i];
    }
    "---- ------------- -----------------\n";
    "%4d %13,d %17,d\n",i,
	  ToI64(total_swaps/(tf-t0)),ToI64(total_cycles/(tf-t0));
    " Avg %13,d %17,d\n\n",
	  ToI64(total_swaps/(tf-t0)/i),ToI64(total_cycles/(tf-t0)/i);
    "Avg Cycles/Swap: %12.6f\n",ToF64(total_cycles)/total_swaps;
    "Avg Time/Swap  : %12.6fns\n\n",(tf-t0)*1000000000.0*i/total_swaps;
    Free(cr.swaps);
    Free(cr.cycles);
  }
}

#help_index "Info;Memory/Info"
public U0 MemBIOSRep()
{//Report the memory ranges reported by the BIOS at boot.
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  CMemRange	*tmpmr;
  "Standard Addresses\n"
	"0x000A0000-0x000BFFFF VGA\n"
	"0xFEE00000-0xFEE00FFF See $$LK,\"APIC\",A=\"MN:LAPIC_BASE\"$$\n\n"
	"32 Bit Device Mem\n";
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tmpmr=dev.mem32_head.next;
  while (tmpmr!=&dev.mem32_head) {
    "%02X:0x%016X-0x%016X\n",
	  tmpmr->type,tmpmr->base,tmpmr->base+tmpmr->size-1;
    tmpmr=tmpmr->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);

  "\nBIOS Memory Report 15:E801\n"
	"01:0x0000000000000000-0x%016X\n",0x100000+m01[0]<<10-1;
  "01:0x0000000001000000-0x%016X\n",0x1000000+m01[1]<<16-1;

  if (m20->type) {
    '\n';
    "BIOS Memory Report 15:E820\n";
    while (m20->type) {
      "%02X:0x%016X-0x%016X\n",m20->type,m20->base,m20->base+m20->len-1;
      m20++;
    }
  }

}
