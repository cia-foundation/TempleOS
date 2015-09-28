U0 DVDImageRead(U8 dvd_drv_let,U8 *out_name)
{//Read entire CD/DVD image into ISO file.
  CBlkDev *bd=Let2BlkDev(dvd_drv_let);
  CDrv *dv=Let2Drv(dvd_drv_let);
  U8 *buf=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS),
	*out_name2=DftExt(out_name,"ISO");
  CFile *f=FOpen(out_name2,"w");
  I64 n,spc=bd->blk_size>>BLK_SIZE_BITS,blk=0,cnt,retry;
  BlkDevInit(bd);
  if (bd->type!=BDT_ATAPI)
    throw('BlkDev');
  if (!out_name)
    out_name=blkdev.dft_iso_filename;
  cnt=CeilU64(dv->size,spc);
  while (cnt>0) {
    if (cnt>COPY_BUF_BLKS)
      n=COPY_BUF_BLKS;
    else
      n=cnt;
    if (n>bd->max_reads)
      n=bd->max_reads;

    retry=4;
    while (--retry)
      if (ATAPIReadBlks2(bd,tS+7.0+0.004*n/spc,buf,blk/spc,n/spc,TRUE))
//n is 0x800 if max_reads.  Up to 8 additional seconds
	break;

    if (!retry)
      ATAPIReadBlks2(bd,0,buf,blk/spc,n/spc,TRUE);

    FWBlks(f,buf,blk,n);
    cnt-=n;
    blk+=n;
  }
  FClose(f);
  Free(buf);
  Free(out_name2);
}

class CDualBuf
{
  U8 *buf0,*buf1;
  I64 in_buf,out_buf,cnt;
  U8 *filename;
  CBlkDev *dvd_bd;
};

U0 DVDImageWriteTask(CDualBuf *d)
{
  U8 *buf;
  I64 n,blk=0,cnt=d->cnt;
  CFile *f;
  if (FileAttr(d->filename)&RS_ATTR_CONTIGUOUS)
    f=FOpen(d->filename,"rc");
  else
    f=FOpen(d->filename,"r");
  while (cnt>0) {
    if (cnt>COPY_BUF_BLKS)
      n=COPY_BUF_BLKS;
    else
      n=cnt;
    if (n>d->dvd_bd->max_writes)
      n=d->dvd_bd->max_writes;
    if (d->in_buf&1)
      buf=d->buf1;
    else
      buf=d->buf0;
    while (d->in_buf>d->out_buf+1)
      Yield;
    FRBlks(f,buf,blk,n);
    d->in_buf++;
    cnt-=n;
    blk+=n;
  }
  FClose(f);
}

U0 DVDImageWrite(U8 dvd_drv_let,U8 *in_name=NULL,I64 media_type=MT_DVD)
{//Write CD/DVD ISO file to disk.
  CDualBuf *d=CAlloc(sizeof(CDualBuf));
  U8 *buf,*in_name2,*in_name3;
  I64 i,n,spc,blk=0,cnt;
  CDrv *dv=Let2Drv(dvd_drv_let);
  CBlkDev *bd=dv->bd,*bd2;
  CTask *task;
  CFile *f;

  if (!in_name)
    in_name=blkdev.dft_iso_filename;
  in_name3=DftExt(in_name,"ISO");
  in_name2=FileNameAbs(in_name3);
  f=FOpen(in_name2,"r");
  if (!f) {
    Free(d);
    return;
  }
  cnt=(FSize(f)+BLK_SIZE-1)>>BLK_SIZE_BITS;
  FClose(f);
  if (bd->type!=BDT_ATAPI)
    throw('BlkDev');
  bd2=Let2BlkDev(*in_name2);
  while (bd2->lock_fwding)
    bd2=bd2->lock_fwding;  //If two blkdevs on same controller, use one lock
  if ((bd2->type==BDT_ATA || bd2->type==BDT_ATAPI) &&
	bd2->base0==bd->base0) {
    PrintErr("Can't burn CD/DVD on same ATA controller as file.\n\n");
    throw('BlkDev');
  }

  bd->flags|=BDF_READ_ONLY_OVERRIDE;
  BlkDevInit(bd);
  spc=bd->blk_size>>BLK_SIZE_BITS;
  if (dv->size<cnt)
    dv->size=cnt;

  d->filename=in_name2;
  d->dvd_bd=bd;
  d->buf0=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  d->buf1=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  d->cnt=cnt;

  task=Spawn(&DVDImageWriteTask,d,"Write CD/DVD");
  while (d->in_buf<=d->out_buf)
    Yield;

  BlkDevLock(bd);
  ATAPIWaitReady(bd,0);

  progress1=0; progress1_max=cnt;
  StrCpy(progress1_desc,"Writing");
  while (cnt>0) {
    if (cnt>COPY_BUF_BLKS)
      n=COPY_BUF_BLKS;
    else
      n=cnt;
    if (n>bd->max_writes)
      n=bd->max_writes;
    if (d->out_buf&1)
      buf=d->buf1;
    else
      buf=d->buf0;
    while (d->in_buf<=d->out_buf)
      Yield;
    ATAPIWriteBlks(bd,buf,blk/spc,(n+spc-1)/spc);
    d->out_buf++;
    cnt-=n;
    blk+=n;
    progress1+=n;
  }
  ATAPISync(bd);

  progress1=0; progress1_max=2;
  StrCpy(progress1_desc,"Closing");
  for (i=0;i<2;i++) {
    ATAPIClose(bd,0x100,i); //Close tracks
    progress1++;
  }

  ATAPISync(bd);

  ATAPIClose(bd,0x200); //close disk
  ATAPISync(bd);
  if (media_type==MT_DVD) {
    ATAPIClose(bd,0x300);
    ATAPISync(bd);
  }

  *progress1_desc=0;
  progress1=progress1_max=0;

  bd->flags&=~BDF_READ_ONLY_OVERRIDE;
  BlkDevUnlock(bd);
  Free(d->buf0);
  Free(d->buf1);
  Free(in_name2);
  Free(in_name3);
  Free(d);
}
