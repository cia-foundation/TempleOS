U0 DskCacheInit(I64 size_in_U8s)
{
  CCacheBlk *tempc;
  I64 i,cnt;

  while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
    Yield;
  Free(dsk.cache_ctrl);
  Free(dsk.cache_base);
  Free(dsk.cache_hash_table);
  if (size_in_U8s<0x2000) {
    dsk.cache_ctrl=NULL;
    dsk.cache_base=NULL;
    dsk.cache_hash_table=NULL;
  } else {
    dsk.cache_ctrl=ACAlloc(offset(CCacheBlk.body));
    dsk.cache_base=AMAlloc(size_in_U8s);
    QueInit(dsk.cache_ctrl);

    cnt=MSize(dsk.cache_base)/sizeof(CCacheBlk);
    dsk.cache_size=cnt*BLK_SIZE;
    for (i=0;i<cnt;i++) {
      tempc=dsk.cache_base+i;
      QueIns(tempc,dsk.cache_ctrl->last_lru);
      tempc->next_hash=tempc->last_hash=tempc;
      tempc->dv=NULL;
      tempc->blk=0;
    }

    dsk.cache_hash_table=AMAlloc(DSK_CACHE_HASH_SIZE*sizeof(U8 *)*2);
    for (i=0;i<DSK_CACHE_HASH_SIZE;i++) {
      tempc=dsk.cache_hash_table(U8 *)+i*sizeof(U8 *)*2-offset(CCacheBlk.next_hash);
      tempc->next_hash=tempc->last_hash=tempc;
    }
  }
  LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
}

I64 DskCacheHash(I64 blk)
{
  I64 i=blk & (DSK_CACHE_HASH_SIZE-1);
  return dsk.cache_hash_table(U8 *)+i<<4-offset(CCacheBlk.next_hash);
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
  QueIns(tempc,dsk.cache_ctrl->last_lru);
  temp_l=DskCacheHash(tempc->blk);
  temp_n=temp_l->next_hash;
  tempc->last_hash=temp_l;
  tempc->next_hash=temp_n;
  temp_l->next_hash=temp_n->last_hash=tempc;
}

CCacheBlk *DskCacheFind(CDrv *dv,I64 blk)
{
  CCacheBlk *tempc,*tempc1=DskCacheHash(blk);
  tempc=tempc1->next_hash;
  while (tempc!=tempc1) {
    if (tempc->dv==dv && tempc->blk==blk)
      return tempc;
    tempc=tempc->next_hash;
  }
  return NULL;
}

U0 DskCacheAdd(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{
  CCacheBlk *tempc;
  if (dsk.cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    while (cnt-->0) {
      if (!(tempc=DskCacheFind(dv,blk)))
	tempc=dsk.cache_ctrl->next_lru;
      DskCacheQueRem(tempc);
      MemCpy(&tempc->body,buf,BLK_SIZE);
      tempc->dv=dv;
      tempc->blk=blk;
      DskCacheQueIns(tempc);
      blk++;
      buf+=BLK_SIZE;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}

U0 DskCacheInvalidate2(CDrv *dv)
{
  CCacheBlk *tempc,*tempc1;
  if (dsk.cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    tempc=dsk.cache_ctrl->last_lru;
    while (tempc!=dsk.cache_ctrl) {
      tempc1=tempc->last_lru;
      if (tempc->dv==dv) {
	DskCacheQueRem(tempc);
	tempc->dv=NULL;
	tempc->blk=0;
	tempc->next_hash=tempc->last_hash=tempc;
	QueIns(tempc,dsk.cache_ctrl->last_lru);
      }
      tempc=tempc1;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}

U0 RCache(CDrv *dv,U8 **_buf, I64 *_blk, I64 *_cnt)
{
  CCacheBlk *tempc;
  if (dsk.cache_base) {
    while (LBts(&sys_semas[SYS_SEMA_DSK_CACHE],0))
      Yield;
    //fetch leading blks from cache
    while (*_cnt>0) {
      if (tempc=DskCacheFind(dv,*_blk)) {
	MemCpy(*_buf,&tempc->body,BLK_SIZE);
	*_cnt-=1;
	*_buf+=BLK_SIZE;
	*_blk+=1;
      } else
	break;
    }
    //fetch trailing blks from cache
    while (*_cnt>0) {
      if (tempc=DskCacheFind(dv,*_blk+*_cnt-1)) {
	MemCpy(*_buf+(*_cnt-1)<<BLK_SIZE_BITS,&tempc->body,BLK_SIZE);
	*_cnt-=1;
      } else
	break;
    }
    LBtr(&sys_semas[SYS_SEMA_DSK_CACHE],0);
  }
}
