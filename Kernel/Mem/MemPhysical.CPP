Bool Mem32DevIns(CMemRange *tempmr)
{
  CMemRange *tempmr1=dev.mem32_root.next,*tempmr2;
  while (tempmr1!=&dev.mem32_root) {
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

  QueInit(&dev.mem32_root);
  tempmr=AMAlloc(sizeof(CMemRange));
  tempmr->type=MRT_UNUSED;
  tempmr->flags=0;
//Maybe !!! Change this to 0xE0000000 !!!
  tempmr->base=0xF0000000;
  tempmr->size=0x10000000;
  QueIns(tempmr,dev.mem32_root.last);

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

Bool MemLowPagesProtect(Bool val=TRUE)
{//Mark low memory not present. Catch NULL deref.
//Returns: FALSE if any were not protected.
  U8 *a=0;
  Bool result=TRUE;
  while (a<=MEM_PROTECTED_LOW_LIMIT) {
    result&=!MemPagePresentMark(a,!val);
    a+=MemPageSize(a);
  }
  return result;
}

U0 MemPagesNotPresentMark()
{
  U8 *a,*max_physical=NULL;
  U16		*m01=SYS_MEM_E801;
  CMemE820	*m20=SYS_MEM_E820;
  MemLowPagesProtect;
  while (m20->type) {
    a=m20->base+m20->len;
    if (a>max_physical)
      max_physical=a;
    m20++;
  }
  if (max_physical>=0x1000000+m01[1]<<16) {
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
  tempmr1=dev.mem32_root.next;
  while (tempmr1!=&dev.mem32_root) {
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
  tempmr=dev.mem32_root.next;
  while (tempmr!=&dev.mem32_root) {
    if (tempmr->base==base) {
      tempmr->type=MRT_UNUSED;
      break;
    }
    tempmr=tempmr->next;
  }
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
}

U8 *Mem64DevAlloc(I64 *_pages2Meg)
{//Alloc 64-bit addr space for device.
  U8 *result;
  I64 i=*_pages2Meg,*pte;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  while (i--) {
    dev.mem64_ptr-=0x200000;
    pte=MemPageTable(dev.mem64_ptr);
    *pte=*pte&~0x18 |0x11; //Uncached and present
    InvlPg(dev.mem64_ptr);
  }
  result=dev.mem64_ptr;
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
  return result;
}

U0 Mem64DevFree(U8 *base,I64 pages2Meg)
{//Free 64-bit device address space.
  if (!base) return;
  while (LBts(&sys_semas[SYS_SEMA_DEV_MEM],0))
    Yield;
  if (base==dev.mem64_ptr)
    dev.mem64_ptr+=pages2Meg*0x200000;
//else not freed
  LBtr(&sys_semas[SYS_SEMA_DEV_MEM],0);
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
//Can be used in $LK,"MakeOSDrvBootInstall",A="MN:MakeOSDrvBootInstall"$() config scripts.
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
