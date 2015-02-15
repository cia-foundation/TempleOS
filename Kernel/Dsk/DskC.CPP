Bool CopySingleZ(U8 *f1,U8 *f2) //Just one file
{
  U8 *file_buf=NULL;
  I64 size,attr=0,c;
  CDirEntry de;
  if (FileFind(f1,&de,FUF_JUST_FILES)) {
    Free(de.full_name);
    file_buf=FileRead(f1,&size,&attr);
    attr=FileAttr(f2,attr);
    if (file_buf) {
      "Copying %s to %s\n",f1,f2;
      c=FileWrite(f2,file_buf,size,de.datetime,attr);
      Free(file_buf);
      return ToBool(c);
    } else
      PrintErr("File not found.\n");
  } else
    PrintErr("File not found.\n");
  return FALSE;
}

#define COPY_BUF_BLKS	0x80
Bool CopySingle(U8 *f1,U8 *f2) //Just one file
{
  U8 *absf1=FileNameAbs(f1),*absf2=FileNameAbs(f2),*buf;
  I64 cnt,n,size,attr1=FileAttr(f1),attr2=FileAttr(f2),i,j;
  CFile *in_file=NULL,*out_file=NULL;
  if (!StrCmp(absf1,absf2)) {//onto self?
    Free(absf1);
    Free(absf2);
    return FALSE;
  }
  Free(absf1);
  Free(absf2);
  if (attr1!=attr2)
    return CopySingleZ(f1,f2);
  buf=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  if (attr1 & RS_ATTR_CONTIGUOUS)
    in_file=FOpen(f1,"rc");
  else
    in_file=FOpen(f1,"r");
  if (in_file) {
    size=FSize(in_file);
    cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS;
    if (attr2 & RS_ATTR_CONTIGUOUS)
      out_file=FOpen(f2,"wc",cnt);
    else
      out_file=FOpen(f2,"w",cnt);
    if (out_file) {
      "Copying %s to %s\n",f1,f2;
      j=size;
      while (cnt>0) {
	if (cnt>COPY_BUF_BLKS) {
	  n=COPY_BUF_BLKS;
	  i=n<<BLK_SIZE_BITS;
	} else {
	  n=cnt;
	  i=j;
	}
	FRBlks(in_file, buf,FFB_NEXT_BLK,n);
	FWBlks(out_file,buf,FFB_NEXT_BLK,n);
	cnt-=n;
	j-=n<<BLK_SIZE_BITS;
      }
      out_file->flags|=FF_USE_OLD_DATETIME;
      out_file->de.datetime=in_file->de.datetime;
      out_file->de.size=size;
      out_file->de.attr=FileAttr(f2,in_file->de.attr);
      FClose(out_file);
      FClose(in_file);
      Free(buf);
      return TRUE;
    } else
      PrintErr("File not found.\n");
    FClose(in_file);
  } else
    PrintErr("File not found.\n");
  Free(buf);
  return FALSE;
}

Bool MkDir(U8 *filename)
{//Make directory. $LK,"Cd",A="MN:Cd"$() can also make directories.
  U8 *name;
  CFileAccess *fa;
  Bool result=FALSE;
  if (FileFind(filename,,FUF_JUST_DIRS))
    return FALSE;
  if (fa=FileAccessNew(filename)) {
    if (*fa->mask) {
      if (!FileNameChk(fa->mask))
	PrintErr("Invalid FileName.\n");
      else {
	"Make Directory:%s\n",filename;
	name=MStrUtil(fa->mask,SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
	switch (fa->dv->type) {
	  case DVT_REDSEA:
	    result=RedSeaMkDir(fa->dv,Fs->cur_dir,name);
	    break;
	  case DVT_FAT32:
	    result=FAT32MkDir(fa->dv,Fs->cur_dir,name);
	    break;
	  case DVT_ISO9660:
	    PrintErr("Not Writable\n");
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	Free(name);
      }
    }
    FileAccessDel(fa);
  }
  return result;
}

I64 SysDel(U8 *wild,Bool make_mask)	//this is dangerous
{
  I64 result=0;
  CFileAccess *fa;
  if (fa=FileAccessNew(wild,make_mask)) {
    switch (fa->dv->type) {
      case DVT_REDSEA:
	result=RedSeaFilesDel(fa->dv,Fs->cur_dir,fa->mask,0,TRUE,TRUE);
	break;
      case DVT_FAT32:
	result=FAT32FilesDel(fa->dv,Fs->cur_dir,fa->mask,0,TRUE,TRUE);
	break;
      case DVT_ISO9660:
	PrintErr("Not Writable\n");
	break;
      default:
	PrintErr("File System Not Supported\n");
    }
    FileAccessDel(fa);
  }
  return result;
}

U0 Fmt(I64 drv_let,Bool quick=TRUE,Bool confirm=TRUE,I64 type=DVT_FAT32)
{//Format hard drive or RAM drive.
//$BK,1$Warning:$BK,0$ Destroys all previous info.
//quick=FALSE means fill entire drive
//with zeros.
//Choose $LK,"DVT_FAT32",A="MN:DVT_FAT32"$ or $LK,"DVT_REDSEA",A="MN:DVT_REDSEA"$.
//You don't format a CD-ROM with this.
  CDrv *dv=Let2Drv(drv_let);
  Bool old_silent=IsSilent;
  if (confirm && !AreYouSure)
    return;
  if (!dv)  {
    PrintErr("Invalid Drive\n");
    return;
  }
  switch (type) {
    case DVT_REDSEA:
      Silent;
      Drv(drv_let);
      Silent(old_silent);
      RedSeaFmt(drv_let,quick);
      DskCacheInvalidate(Let2Drv(drv_let));
      Drv(drv_let);
      break;
    case DVT_FAT32:
      Silent;
      Drv(drv_let);
      Silent(old_silent);
      FAT32Fmt(drv_let,quick);
      DskCacheInvalidate(Let2Drv(drv_let));
      Drv(drv_let);
      break;
    case DVT_ISO9660:
      PrintErr("Not Writable\n");
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
}

U0 CDImageRead(U8 cd_drv_let,U8 *out_name)
{//Read entire CD/DVD image into ISO file.
  CBlkDev *bd=Let2BlkDev(cd_drv_let);
  CDrv *dv=Let2Drv(cd_drv_let);
  U8 *buf=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS),
     *out_name2=DftExt(out_name,"ISO");
  CFile *f=FOpen(out_name2,"w");
  I64 n,spc=bd->blk_size>>BLK_SIZE_BITS,blk=0,cnt,retry;
  BlkDevInit(bd);
  if (bd->type!=BDT_ATAPI)
    throw('BlkDev');
  if (!out_name)
    out_name=dsk.dft_iso_filename;
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
  CBlkDev *cd_bd;
};

U0 CDImageWriteTask(CDualBuf *d)
{
  U8 *buf;
  I64 n,blk=0,cnt=d->cnt;
  CFile *f;
  Preempt(ON);
  if (FileAttr(d->filename)&RS_ATTR_CONTIGUOUS)
    f=FOpen(d->filename,"rc");
  else
    f=FOpen(d->filename,"r");
  while (cnt>0) {
    if (cnt>COPY_BUF_BLKS)
      n=COPY_BUF_BLKS;
    else
      n=cnt;
    if (n>d->cd_bd->max_writes)
      n=d->cd_bd->max_writes;
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

U0 CDImageWrite(U8 cd_drv_let,U8 *in_name=NULL,I64 media_type=MT_CD)
{//Write CD/DVD ISO file to disk.
  CDualBuf *d=CAlloc(sizeof(CDualBuf));
  U8 *buf,*in_name2,*in_name3;
  I64 i,n,spc,blk=0,cnt;
  CDrv *dv=Let2Drv(cd_drv_let);
  CBlkDev *bd=dv->bd,*bd2;
  CTask *task;
  Bool old_preempt;
  CFile *f;

  if (!in_name)
    in_name=dsk.dft_iso_filename;
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
    bd2=bd2->lock_fwding;  //If two blkdevs on same controller, use just one lock
  if ((bd2->type==BDT_ATA || bd2->type==BDT_ATAPI) &&
       bd2->base0==bd->base0) {
    "Can't Burn CD on same ATA ctrller as File.\n\n";
    throw('BlkDev');
  }

  bd->flags|=BDF_READ_ONLY_OVERRIDE;
  BlkDevInit(bd);
  spc=bd->blk_size>>BLK_SIZE_BITS;
  if (dv->size<cnt)
    dv->size=cnt;

  d->filename=in_name2;
  d->cd_bd=bd;
  d->buf0=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  d->buf1=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  d->cnt=cnt;

  old_preempt=Preempt(OFF);
  task=Spawn(&CDImageWriteTask,d,"Write CD");

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
  Preempt(old_preempt);
  Free(d);
}
