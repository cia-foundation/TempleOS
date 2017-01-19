#help_index "PCI;Processor;Devices;Info"

//The file was downloaded from
//http://www.pcidatabase.com/reports.php?type=tab-delimeted

#define PCI_DEV_FILE "::/Misc/PCIDevices.DD.Z"

/****
//1) Download http://www.pcidatabase.com/reports.php?type=tab-delimeted
//2) Rename to ::/Misc/PCIDevices.DD.Z
//3) ToDolDoc("::/Misc/PCIDevices.DD.Z");
//4) Edit and remove file header and tail
//5) Text find-and-replace "=0A=" with "". (Doesn't seem necessary anmore.)
//6) Run PCIDevFileGen(). (Doesn't seem necessary anmore.)

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

#help_index "Info;Memory/Info"
public U0 MemBIOSRep()
{//Report the memory ranges reported by the BIOS at boot.
  U16		*m01=MEM_E801;
  CMemE820	*m20=MEM_E820;
  CMemRange	*tmpmr;
  "Standard Addresses\n"
	"000A0000-000BFFFF VGA\n"
	"FEE00000-FEE00FFF See $$LK,\"APIC\",A=\"MN:LAPIC_BASE\"$$\n\n"
	"32 Bit Device Mem\n";
  while (LBts(&sys_semas[SEMA_DEV_MEM],0))
    Yield;
  tmpmr=dev.mem32_head.next;
  while (tmpmr!=&dev.mem32_head) {
    "%02X:%016X-%016X\n",
	  tmpmr->type,tmpmr->base,tmpmr->base+tmpmr->size-1;
    tmpmr=tmpmr->next;
  }
  LBtr(&sys_semas[SEMA_DEV_MEM],0);

  "\nBIOS Memory Report 15:E801\n"
	"01:0000000000000000-%016X\n",0x100000+m01[0]<<10-1;
  "01:0000000001000000-%016X\n",SYS_16MEG_AREA_LIMIT+m01[1]<<16-1;

  if (m20->type) {
    '\n';
    "BIOS Memory Report 15:E820\n";
    while (m20->type) {
      "%02X:%016X-%016X\n",m20->type,m20->base,m20->base+m20->len-1;
      m20++;
    }
  }
}

public U0 MemPageRep()
{//Page Table Report.
  "MAPPED\t  :%010X with ",mem_mapped_space;
  if (Bt(&mem_page_size,30))
    "$$RED$$1GIG$$FG$$ pages\n";
  else
    "$$RED$$2MEG$$FG$$ pages\n";
  "PML2\t  :%010X 2MEG  :%08X\n",
	*MEM_PML2(U64 *),*MEM_2MEG_NUM(U64 *);
  "PML3\t  :%010X 1GIG  :%08X\n",
	*MEM_PML3(U64 *),*MEM_1GIG_NUM(U64 *);
  "PML4\t  :%010X 512GIG:%08X\n",
	*MEM_PML4(U64 *),*MEM_512GIG_NUM(U64 *);
  "FIXED_AREA:%010X\n",SYS_FIXED_AREA;
  "HEAP_BASE :%010X\nHEAP_LIMIT:%010X\n",mem_heap_base,mem_heap_limit;
}
