U0 DskCacheInit(I64 size_in_U8s)
{
  CCacheBlk *tmpc;
  I64 i,cnt;

  while (LBts(&sys_semas[SEMA_DSK_CACHE],0))
    Yield;
  Free(blkdev.cache_ctrl);
  Free(blkdev.cache_base);
  Free(blkdev.cache_hash_table);
  if (size_in_U8s<0x2000) {
    blkdev.cache_ctrl=NULL;
    blkdev.cache_base=NULL;
    blkdev.cache_hash_table=NULL;
  } else {
    blkdev.cache_ctrl=ACAlloc(offset(CCacheBlk.body));
    blkdev.cache_base=AMAlloc(size_in_U8s);
    QueInit(blkdev.cache_ctrl);

    cnt=MSize(blkdev.cache_base)/sizeof(CCacheBlk);
    blkdev.cache_size=cnt*BLK_SIZE;
    for (i=0;i<cnt;i++) {
      tmpc=blkdev.cache_base+i;
      QueIns(tmpc,blkdev.cache_ctrl->last_lru);
      tmpc->next_hash=tmpc->last_hash=tmpc;
      tmpc->dv=NULL;
      tmpc->blk=0;
    }

    blkdev.cache_hash_table=AMAlloc(DSK_CACHE_HASH_SIZE*sizeof(U8 *)*2);
    for (i=0;i<DSK_CACHE_HASH_SIZE;i++) {
      tmpc=blkdev.cache_hash_table(U8 *)+i*sizeof(U8 *)*2
	    -offset(CCacheBlk.next_hash);
      tmpc->next_hash=tmpc->last_hash=tmpc;
    }
  }
  LBtr(&sys_semas[SEMA_DSK_CACHE],0);
}

I64 DskCacheHash(I64 blk)
{
  I64 i=blk & (DSK_CACHE_HASH_SIZE-1);
  return blkdev.cache_hash_table(U8 *)+i<<4-offset(CCacheBlk.next_hash);
}

U0 DskCacheQueRem(CCacheBlk *tmpc)
{
  QueRem(tmpc);
  tmpc->next_hash->last_hash=tmpc->last_hash;
  tmpc->last_hash->next_hash=tmpc->next_hash;
}

U0 DskCacheQueIns(CCacheBlk *tmpc)
{
  CCacheBlk *tmp_n,*tmp_l;
  QueIns(tmpc,blkdev.cache_ctrl->last_lru);
  tmp_l=DskCacheHash(tmpc->blk);
  tmp_n=tmp_l->next_hash;
  tmpc->last_hash=tmp_l;
  tmpc->next_hash=tmp_n;
  tmp_l->next_hash=tmp_n->last_hash=tmpc;
}

CCacheBlk *DskCacheFind(CDrv *dv,I64 blk)
{
  CCacheBlk *tmpc,*tmpc1=DskCacheHash(blk);
  tmpc=tmpc1->next_hash;
  while (tmpc!=tmpc1) {
    if (tmpc->dv==dv && tmpc->blk==blk)
      return tmpc;
    tmpc=tmpc->next_hash;
  }
  return NULL;
}

U0 DskCacheAdd(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{
  CCacheBlk *tmpc;
  if (blkdev.cache_base) {
    while (LBts(&sys_semas[SEMA_DSK_CACHE],0))
      Yield;
    while (cnt-->0) {
      if (!(tmpc=DskCacheFind(dv,blk)))
	tmpc=blkdev.cache_ctrl->next_lru;
      DskCacheQueRem(tmpc);
      MemCpy(&tmpc->body,buf,BLK_SIZE);
      tmpc->dv=dv;
      tmpc->blk=blk;
      DskCacheQueIns(tmpc);
      blk++;
      buf+=BLK_SIZE;
    }
    LBtr(&sys_semas[SEMA_DSK_CACHE],0);
  }
}

U0 DskCacheInvalidate2(CDrv *dv)
{
  CCacheBlk *tmpc,*tmpc1;
  if (blkdev.cache_base) {
    while (LBts(&sys_semas[SEMA_DSK_CACHE],0))
      Yield;
    tmpc=blkdev.cache_ctrl->last_lru;
    while (tmpc!=blkdev.cache_ctrl) {
      tmpc1=tmpc->last_lru;
      if (tmpc->dv==dv) {
	DskCacheQueRem(tmpc);
	tmpc->dv=NULL;
	tmpc->blk=0;
	tmpc->next_hash=tmpc->last_hash=tmpc;
	QueIns(tmpc,blkdev.cache_ctrl->last_lru);
      }
      tmpc=tmpc1;
    }
    LBtr(&sys_semas[SEMA_DSK_CACHE],0);
  }
}

U0 RCache(CDrv *dv,U8 **_buf, I64 *_blk, I64 *_cnt)
{
  CCacheBlk *tmpc;
  if (blkdev.cache_base) {
    while (LBts(&sys_semas[SEMA_DSK_CACHE],0))
      Yield;
//fetch leading blks from cache
    while (*_cnt>0) {
      if (tmpc=DskCacheFind(dv,*_blk)) {
	MemCpy(*_buf,&tmpc->body,BLK_SIZE);
	*_cnt-=1;
	*_buf+=BLK_SIZE;
	*_blk+=1;
      } else
	break;
    }
//fetch trailing blks from cache
    while (*_cnt>0) {
      if (tmpc=DskCacheFind(dv,*_blk+*_cnt-1)) {
	MemCpy(*_buf+(*_cnt-1)<<BLK_SIZE_BITS,&tmpc->body,BLK_SIZE);
	*_cnt-=1;
      } else
	break;
    }
    LBtr(&sys_semas[SEMA_DSK_CACHE],0);
  }
}
