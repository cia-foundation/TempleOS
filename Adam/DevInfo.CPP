#help_index "PCI;Processor;Devices;Info"

//The file was downloaded from
//http://www.pcidatabase.com/reports.php?type=tab-delimeted

#define PCI_DEV_FILE "::/Misc/PCIDevices.TXT.Z"

/****

//1) Plain text edit and remove file header and tail
//2) Text find-and-replace "=0A=" with "".
//3) Run PCIDevFileGen().

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

//$LK,"::/Misc/PCIDevices.TXT",A="PI:::/Misc/PCIDevices.TXT"$
U0 PCILookUpSingle(CDoc *doc,I64 m,I64 d,U8 **_vendor,U8 **_dev)
{
  Bool first=TRUE;
  U8 buf[8],*vendor=NULL,*dev=NULL;
  CDocEntry *doc_e=doc->root.next;
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
  CPCIDev *temppci;
  I64 w1,w2,b,d,f,timeout=32*8*2;
  CDoc *doc;
  if (dev.pci_root.next!=&dev.pci_root)
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
	  QueIns(temppci,dev.pci_root.last);
	  timeout=32*8*2;
	} else if (sys_pci_busses==256 && --timeout<=0)
	  goto lud_done;
      }
lud_done:
  DocDel(doc);
}

public U0 PCIRep()
{//Report description of PCI devices.
  CPCIDev *temppci;
  "PCI Busses:%d\n",sys_pci_busses;
  if (!FileFind(PCI_DEV_FILE)) {
    "You don't have the PCI device file.\n";
    return;
  }
  PCILookUpDevs;
  temppci=dev.pci_root.next;
  while (temppci!=&dev.pci_root) {
    "%02X:%02X:%01X %02X%02X $$GREEN$$%s $$CYAN$$%s$$FG$$\n",
	  temppci->bus,temppci->dev,temppci->fun,
	  temppci->base_code,temppci->sub_code,
	  temppci->vendor_str,temppci->dev_id_str;
    temppci=temppci->next;
  }
}

#help_index "Info;Time/CPU Cycles"
public U0 CPURep()
{//Report number of cores and clock freq.
  "%d Cores %6h?nHz\n",mp_cnt,ToF64(cnts.time_stamp_freq);
}

#help_index "Info;Memory/Info"
public U0 MemBIOSRep()
{//Report the memory ranges reported by the BIOS at boot.
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  CMemRange	*tempmr;
  "Standard Addresses\n"
	"0x000A0000-0x000BFFFF VGA\n"
	"0xFEE00000-0xFEE00FFF See $$LK,\"APIC\",A=\"MN:LAPIC_BASE\"$$\n\n"
	"32 Bit Device Mem\n";
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr=dev.mem32_root.next;
  while (tempmr!=&dev.mem32_root) {
    "%02X:0x%016X-0x%016X\n",
	  tempmr->type,tempmr->base,tempmr->base+tempmr->size-1;
    tempmr=tempmr->next;
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
