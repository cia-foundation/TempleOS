U0 DskCacheInit(I64 size_in_U8s)
{
  CCacheBlk *tempc;
  I64 i,cnt;

  while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
    Yield;
  Free(sys_dsk_cache_ctrl);
  Free(sys_dsk_cache_base);
  Free(sys_dsk_cache_hash_table);
  if (size_in_U8s<0x2000) {
    sys_dsk_cache_ctrl=NULL;
    sys_dsk_cache_base=NULL;
    sys_dsk_cache_hash_table=NULL;
  } else {
    sys_dsk_cache_ctrl=ACAlloc(offset(CCacheBlk.body));
    sys_dsk_cache_base=AMAlloc(size_in_U8s);
    QueInit(sys_dsk_cache_ctrl);

    cnt=MSize(sys_dsk_cache_base)/sizeof(CCacheBlk);
    sys_dsk_cache_size=cnt*BLK_SIZE;
    for (i=0;i<cnt;i++) {
      tempc=sys_dsk_cache_base+i;
      QueIns(tempc,sys_dsk_cache_ctrl->last_lru);
      tempc->next_hash=tempc->last_hash=tempc;
      tempc->p=NULL;
      tempc->blk=0;
    }

    sys_dsk_cache_hash_table=AMAlloc(DSK_CACHE_HASH_SIZE*sizeof(U8 *)*2);
    for (i=0;i<DSK_CACHE_HASH_SIZE;i++) {
      tempc=sys_dsk_cache_hash_table(U8 *)+i*sizeof(U8 *)*2-offset(CCacheBlk.next_hash);
      tempc->next_hash=tempc->last_hash=tempc;
    }
  }
  LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
}

I64 DskCacheHash(I64 blk)
{
  I64 i=blk & (DSK_CACHE_HASH_SIZE-1);
  return sys_dsk_cache_hash_table(U8 *)+i<<4-offset(CCacheBlk.next_hash);
}

U0 DskCacheQueRem(CCacheBlk *tempc)
{
  QueRem(tempc);
  tempc->next_hash->last_hash=tempc->last_hash;
  tempc->last_hash->next_hash=tempc->next_hash;
}

U0 DskCacheQueIns(CCacheBlk *tempc)
{
  CCacheBlk *temp_n,*temp_l;
  QueIns(tempc,sys_dsk_cache_ctrl->last_lru);
  temp_l=DskCacheHash(tempc->blk);
  temp_n=temp_l->next_hash;
  tempc->last_hash=temp_l;
  tempc->next_hash=temp_n;
  temp_l->next_hash=temp_n->last_hash=tempc;
}

CCacheBlk *DskCacheFind(CPrt *p,I64 blk)
{
  CCacheBlk *tempc,*tempc1=DskCacheHash(blk);
  tempc=tempc1->next_hash;
  while (tempc!=tempc1) {
    if (tempc->p==p && tempc->blk==blk)
      return tempc;
    tempc=tempc->next_hash;
  }
  return NULL;
}

U0 DskCacheAdd(CPrt *p,U8 *buf, I64 blk, I64 cnt)
{
  CCacheBlk *tempc;
  if (sys_dsk_cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    while (cnt-->0) {
      if (!(tempc=DskCacheFind(p,blk)))
	tempc=sys_dsk_cache_ctrl->next_lru;
      DskCacheQueRem(tempc);
      MemCpy(&tempc->body,buf,BLK_SIZE);
      tempc->p=p;
      tempc->blk=blk;
      DskCacheQueIns(tempc);
      blk++;
      buf+=BLK_SIZE;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}

U0 DskCacheInvalidate2(CPrt *p)
{
  CCacheBlk *tempc,*tempc1;
  if (sys_dsk_cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    tempc=sys_dsk_cache_ctrl->last_lru;
    while (tempc!=sys_dsk_cache_ctrl) {
      tempc1=tempc->last_lru;
      if (tempc->p==p) {
	DskCacheQueRem(tempc);
	tempc->p=NULL;
	tempc->blk=0;
	tempc->next_hash=tempc->last_hash=tempc;
	QueIns(tempc,sys_dsk_cache_ctrl->last_lru);
      }
      tempc=tempc1;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}

U0 RCache(CPrt *p,U8 **_buf, I64 *_blk, I64 *_cnt)
{
  CCacheBlk *tempc;
  if (sys_dsk_cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    //fetch leading blks from cache
    while (*_cnt>0) {
      if (tempc=DskCacheFind(p,*_blk)) {
	MemCpy(*_buf,&tempc->body,BLK_SIZE);
	*_cnt-=1;
	*_buf+=BLK_SIZE;
	*_blk+=1;
      } else
	break;
    }
    //fetch trailing blks from cache
    while (*_cnt>0) {
      if (tempc=DskCacheFind(p,*_blk+*_cnt-1)) {
	MemCpy(*_buf+(*_cnt-1)<<BLK_SIZE_BITS,&tempc->body,BLK_SIZE);
	*_cnt-=1;
      } else
	break;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}

Bool RAMRBlks(CPrt *p,U8 *buf,I64 blk,I64 cnt)
{
  CBlkDev *bdev=p->bdev;
  BlkDevChk(bdev);
  MemCpy(buf,bdev->RAM_dsk+blk<<BLK_SIZE_BITS,cnt<<BLK_SIZE_BITS);
  return TRUE;
}

Bool RAMWBlks(CPrt *p,U8 *buf,I64 blk,I64 cnt)
{
  CBlkDev *bdev=p->bdev;
  BlkDevChk(bdev);
  MemCpy(bdev->RAM_dsk+blk<<BLK_SIZE_BITS,buf,cnt<<BLK_SIZE_BITS);
  return TRUE;
}
