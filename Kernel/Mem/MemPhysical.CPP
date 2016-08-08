Bool Mem32DevIns(CMemRange *tempmr)
{
  CMemRange *tempmr1=dev.mem32_head.next,*tempmr2;
  while (tempmr1!=&dev.mem32_head) {
    if (!tempmr1->type && tempmr->base>=tempmr1->base &&
	  tempmr->base+tempmr->size<=tempmr1->base+tempmr1->size) {
      if (tempmr->base>tempmr1->base) {
	tempmr2=AMAlloc(sizeof(CMemRange));
	tempmr2->type=MRT_UNUSED;
	tempmr2->flags=0;
	tempmr2->base=tempmr1->base;
	tempmr2->size=tempmr->base-tempmr1->base;
	QueInsRev(tempmr2,tempmr1);
      }
      QueInsRev(tempmr,tempmr1);
      tempmr1->size=tempmr1->base+tempmr1->size-
	    (tempmr->base+tempmr->size);
      tempmr1->base=tempmr->base+tempmr->size;
      if (!tempmr1->size) {
	QueRem(tempmr1);
	Free(tempmr1);
      }
      return TRUE;
    }
    tempmr1=tempmr1->next;
  }
  return FALSE;
}

U0 Mem32DevInit()
{
  CMemRange *tempmr;
  CMemE820 *m20=SYS_MEM_E820;

  QueInit(&dev.mem32_head);
  tempmr=AMAlloc(sizeof(CMemRange));
  tempmr->type=MRT_UNUSED;
  tempmr->flags=0;
//Maybe !!! Change this to 0xF0000000 !!!
  tempmr->base=0xE0000000;
  tempmr->size=0x10000000;
  QueIns(tempmr,dev.mem32_head.last);

  if (m20->type) {
    while (m20->type) {
      tempmr=AMAlloc(sizeof(CMemRange));
      tempmr->type=m20->type;
      tempmr->flags=0;
      tempmr->base=m20->base;
      tempmr->size=m20->len;
      if (!Mem32DevIns(tempmr))
	Free(tempmr);
      m20++;
    }
  }
}

U0 MemPagesNotPresentMark()
{
  U8 *a,*max_physical=NULL;
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  while (m20->type) {
    a=m20->base+m20->len;
    if (a>max_physical)
      max_physical=a;
    m20++;
  }
  if (max_physical>=0x1000000+m01[1]<<16) {
    if (sys_gig_pages)
      a=(max_physical+0x3FFFFFFF)&~0x3FFFFFFF;
    else
      a=(max_physical+0x1FFFFF)&~0x1FFFFF;
    while (a<MEM_MAPPED_SPACE) {
      MemPagePresentMark(a,FALSE);
      a+=MemPageSize(a);
    }
  }
}

U8 *Mem32DevAlloc(I64 size,I64 alignment)
{//Alloc 32-bit addr space for device. (Doesn't work.) Not used.
//For this to work the BIOS E820 map must be searched for gaps in
  //the 32-bit range and the pool initialized to the gaps.
  U8 *base,*limit;
  CMemRange *tempmr,*tempmr1;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr1=dev.mem32_head.next;
  while (tempmr1!=&dev.mem32_head) {
    base=(tempmr1->base+alignment-1)&~(alignment-1);
    limit=base+size-1;
    if (!tempmr1->type &&
	  limit<tempmr1->base+tempmr1->size) {
      tempmr=AMAlloc(sizeof(CMemRange));
      tempmr->type=MRT_DEV;
      tempmr->flags=0;
      tempmr->base=base;
      tempmr->size=size;
      if (!Mem32DevIns(tempmr)) {
	Free(tempmr);
	LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
	return NULL;
      }
      LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
      return tempmr->base;
    }
    tempmr1=tempmr1->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
  return NULL;
}

U0 Mem32DevFree(U8 *base)
{//Free 32-bit device address space.
  CMemRange *tempmr;
  if (!base) return;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  tempmr=dev.mem32_head.next;
  while (tempmr!=&dev.mem32_head) {
    if (tempmr->base==base) {
      tempmr->type=MRT_UNUSED;
      break;
    }
    tempmr=tempmr->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
}

U8 *Mem64DevAlloc(I64 *_pages1Gig)
{//Alloc 64-bit addr space for device.
  U8 *a;
  I64 i=*_pages1Gig,*pte;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  while (i--) {
    a=dev.mem64_ptr-=1<<30;
    do {
      pte=MemPageTable(a);
      *pte=*pte&~0x18 |0x11; //Uncached and present
      InvlPg(dev.mem64_ptr);
      a+=MemPageSize(a);
    } while (a-dev.mem64_ptr<1<<30);
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
  return dev.mem64_ptr;
}

U0 Mem64DevFree(U8 *base,I64 pages1Gig)
{//Free 64-bit device address space.
  if (!base) return;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  if (base==dev.mem64_ptr)
    dev.mem64_ptr+=pages1Gig*1<<30;
//else not freed
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
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
    a+=MemPageSize(a);
  } while (a-dev.uncached_alias<1<<32);
}

I64 BIOSTotalMem()
{
  I64 r01,r20;
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;

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
