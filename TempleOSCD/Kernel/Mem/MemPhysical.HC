Bool Mem32DevIns(CMemRange *tmpmr)
{
  CMemRange *tmpmr1=dev.mem32_head.next,*tmpmr2;
  while (tmpmr1!=&dev.mem32_head) {
    if (!tmpmr1->type && tmpmr->base>=tmpmr1->base &&
	  tmpmr->base+tmpmr->size<=tmpmr1->base+tmpmr1->size) {
      if (tmpmr->base>tmpmr1->base) {
	tmpmr2=AMAlloc(sizeof(CMemRange));
	tmpmr2->type=MRT_UNUSED;
	tmpmr2->flags=0;
	tmpmr2->base=tmpmr1->base;
	tmpmr2->size=tmpmr->base-tmpmr1->base;
	QueInsRev(tmpmr2,tmpmr1);
      }
      QueInsRev(tmpmr,tmpmr1);
      tmpmr1->size=tmpmr1->base+tmpmr1->size-
	    (tmpmr->base+tmpmr->size);
      tmpmr1->base=tmpmr->base+tmpmr->size;
      if (!tmpmr1->size) {
	QueRem(tmpmr1);
	Free(tmpmr1);
      }
      return TRUE;
    }
    tmpmr1=tmpmr1->next;
  }
  return FALSE;
}

U0 Mem32DevInit()
{
  CMemRange *tmpmr;
  CMemE820 *m20=MEM_E820;

  QueInit(&dev.mem32_head);
  tmpmr=AMAlloc(sizeof(CMemRange));
  tmpmr->type=MRT_UNUSED;
  tmpmr->flags=0;
//Maybe !!! Change this to 0xF0000000 !!!
  tmpmr->base=0xE0000000;
  tmpmr->size=0x10000000;
  QueIns(tmpmr,dev.mem32_head.last);

  if (m20->type) {
    while (m20->type) {
      tmpmr=AMAlloc(sizeof(CMemRange));
      tmpmr->type=m20->type;
      tmpmr->flags=0;
      tmpmr->base=m20->base;
      tmpmr->size=m20->len;
      if (!Mem32DevIns(tmpmr))
	Free(tmpmr);
      m20++;
    }
  }
}

U8 *Mem32DevAlloc(I64 size,I64 alignment)
{//Alloc 32-bit addr space for device. (Doesn't work.) Not used.
//For this to work the BIOS E820 map must be searched for gaps in
  //the 32-bit range and the pool initialized to the gaps.
  U8 *base,*limit;
  CMemRange *tmpmr,*tmpmr1;
  while (LBts(&sys_semas[SEMA_DEV_MEM],0))
    Yield;
  tmpmr1=dev.mem32_head.next;
  while (tmpmr1!=&dev.mem32_head) {
    base=(tmpmr1->base+alignment-1)&~(alignment-1);
    limit=base+size-1;
    if (!tmpmr1->type &&
	  limit<tmpmr1->base+tmpmr1->size) {
      tmpmr=AMAlloc(sizeof(CMemRange));
      tmpmr->type=MRT_DEV;
      tmpmr->flags=0;
      tmpmr->base=base;
      tmpmr->size=size;
      if (!Mem32DevIns(tmpmr)) {
	Free(tmpmr);
	LBtr(&sys_semas[SEMA_DEV_MEM],0);
	return NULL;
      }
      LBtr(&sys_semas[SEMA_DEV_MEM],0);
      return tmpmr->base;
    }
    tmpmr1=tmpmr1->next;
  }
  LBtr(&sys_semas[SEMA_DEV_MEM],0);
  return NULL;
}

U0 Mem32DevFree(U8 *base)
{//Free 32-bit device address space.
  CMemRange *tmpmr;
  if (!base) return;
  while (LBts(&sys_semas[SEMA_DEV_MEM],0))
    Yield;
  tmpmr=dev.mem32_head.next;
  while (tmpmr!=&dev.mem32_head) {
    if (tmpmr->base==base) {
      tmpmr->type=MRT_UNUSED;
      break;
    }
    tmpmr=tmpmr->next;
  }
  LBtr(&sys_semas[SEMA_DEV_MEM],0);
}

U8 *Mem64DevAlloc(I64 *_pages1Gig)
{//Alloc 64-bit addr space for device.
  U8 *a;
  I64 i=*_pages1Gig,*pte;
  while (LBts(&sys_semas[SEMA_DEV_MEM],0))
    Yield;
  while (i--) {
    a=dev.mem64_ptr-=1<<30;
    do {
      pte=MemPageTable(a);
      *pte=*pte&~0x18 |0x11; //Uncached and present
      InvlPg(dev.mem64_ptr);
      a+=mem_page_size;
    } while (a-dev.mem64_ptr<1<<30);
  }
  LBtr(&sys_semas[SEMA_DEV_MEM],0);
  return dev.mem64_ptr;
}

U0 Mem64DevFree(U8 *base,I64 pages1Gig)
{//Free 64-bit device address space.
  if (!base) return;
  while (LBts(&sys_semas[SEMA_DEV_MEM],0))
    Yield;
  if (base==dev.mem64_ptr)
    dev.mem64_ptr+=pages1Gig*1<<30;
//else not freed
  LBtr(&sys_semas[SEMA_DEV_MEM],0);
}

U0 UncachedAliasAlloc() //Make uncached alias for 4 lowest Gig.
{
  I64 i=4,*pte;
  U8 *a;
  a=dev.uncached_alias=Mem64DevAlloc(&i);
  do {
    pte=MemPageTable(a);
    *pte=0x197+a-dev.uncached_alias;
    InvlPg(a);
    a+=mem_page_size;
  } while (a-dev.uncached_alias<1<<32);
}

I64 BIOSTotalMem()
{
  I64 r01,r20;
  U16		*m01=MEM_E801;
  CMemE820	*m20=MEM_E820;

  r01=0x100000+m01[0]<<10+m01[1]<<16;
  r20=0;
  if (m20->type) {
    while (m20->type) {
      if (m20->type==1)
	r20+=m20->len;
      m20++;
    }
  }
  return MaxI64(r01,r20);
}

I64 Scale2Mem(I64 min,I64 max,I64 limit=2*1024*1024*1024)
{//Helps pick DiskCache and RAMDisk sizes.
//Can be used in $LK,"BootHDIns",A="MN:BootHDIns"$() config scripts.
  I64 i;
  if (sys_data_bp)
    i=sys_data_bp->alloced_u8s;
  else
    i=sys_code_bp->alloced_u8s;
  if (i>=limit)
    return max;
  else
    return min+(max-min)*i/limit;
}
