//ISO1 is ISO9660
//ISO2 is ISO13490
//ISO3 is ISO13346

Bool ISOPrtInit(CPrt *p,I64 blk)
{
  CBlkDev *bdev=p->bdev;
  I64 spc=bdev->blk_size>>BLK_SIZE_BITS,i=blk/spc;
  CISO1PriDesc *iso=MAlloc(bdev->blk_size);
  CISO1DirEntry *de;
  Bool unlock,result=FALSE;
  U8 buf[8];

  try {
    unlock=PrtLock(p);
    p->type=PT_ISO9660; //default
    p->spc=spc;
    p->offset=p->data=p->root_cluster=p->size=0;
    while (TRUE) {
      p->size=MaxI64(p->size,(i+1)*spc);
      RBlks(p,iso,i*spc,spc);
      buf[0](U32)=iso->id[0](U32);
      buf[4](U16)=iso->id[4](U8);
      switch (ListEntryMatch(buf,"CD001\0CDW02\0BEA01\0BOOT2\0NSR02\0NSR03\0TEA01\0",LEF_EXACT)) {
	case 0:
	  p->type=PT_ISO9660;
	  switch (iso->type) {
	    case ISO1T_SUPPLEMENTARY_DESC:
	      p->data=0;
	      de=&iso->root_dir_record;
	      p->root_cluster=de->loc.little;
	      p->size=iso->vol_space_size.little*bdev->blk_size>>BLK_SIZE_BITS;
	      result=TRUE;
	      goto ipi_done;
	    case ISO1T_TERMINATOR:
	      throw(EXCEPT_PRT,5);
	  }
	  break;
	default: //Its normal for ISO13346 to read NULL blk as terminator
	  PutDefineErr("ST_UNSUPPORTED_FILE_SYSTEM");
	  throw(EXCEPT_PRT,6);
      }
      i++;
    }
ipi_done:
    Free(iso);
    if (unlock)
      PrtUnlock(p);
  } catch {
    p->type=PT_ISO9660;
    p->spc=spc;
    p->offset=p->data=p->root_cluster=0;
    Free(iso);
    if (unlock)
      PrtUnlock(p);
  }
  return result;
}

U0 CDate2ISO1(CISO1Date *dst,CDate ltdt)
{
  CDateStruct ds;
  CDate2CDateStruct(&ds,ltdt);
  dst->year=ds.year-ISO1_BASE_YEAR;
  dst->mon=ds.mon;
  dst->day=ds.day_of_mon;
  dst->hour=ds.hour;
  dst->min=ds.min;
  dst->sec=ds.sec;
  dst->sec100=ds.sec100;
}

CDate ISO1DateStruct2CDate(CISO1Date *dt)
{
  CDateStruct ds;
  MemSet(&ds,0,sizeof(CDateStruct));
  ds.day_of_mon=dt->day;
  ds.mon=dt->mon;
  ds.year=dt->year+ISO1_BASE_YEAR;
  ds.sec100=dt->sec100;
  ds.sec=dt->sec;
  ds.min=dt->min;
  ds.hour=dt->hour;
  return CDateStruct2CDate(&ds);
}

Bool ISO1FromName(U8 *dst,U8 *src)
{
  I64 i,j,n;
  MemSet(dst,0,CDIR_FILENAME_LEN);
  n=*src++;
  if (n==1 && !*src) {
    *dst='.';
  } else if (n==1 && *src==1) {
    *dst='.';
    dst[1]='.';
  } else {
    n>>=1;
    j=0;
    for (i=0;i<n;i++) {
      src++;
      if (*src==';')
	break;
      if (Bt(filename_chars_bitmap,*src)) {
	if (j>=CDIR_FILENAME_LEN-1)
	  return FALSE;
	dst[j++]=*src++;
      } else
	return FALSE;
    }
  }
  return FileNameChk(dst);
}

Bool ISO1CDirFill(CPrt *p,CDirEntry *tempm,CISO1DirEntry *de)
{
  Bool result;
  CArcCompress *buf;
  MemSet(tempm,0,sizeof(CDirEntry));
  result=ISO1FromName(tempm->name,&de->name_len);
  tempm->cluster=de->loc.little;
  tempm->size=de->size.little;
  tempm->attr=FileAttr(tempm->name);
  if (de->flags & ISO1_ATTR_DIR)
    tempm->attr|=_ATTR_DIR;
  tempm->datetime=ISO1DateStruct2CDate(&de->date);
  if (tempm->attr & _ATTR_COMPRESSED) {
    buf=MAlloc(BLK_SIZE);
    RClustersBlks(p,buf,tempm->cluster,1);
    tempm->expanded_size=buf->expanded_size;
    Free(buf);
  } else
    tempm->expanded_size=tempm->size;
  return result;
}

Bool ISO1FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,
		     Bool dir_only,Bool file_only)
{
  CISO1DirEntry *isoptr,*buf;
  U8 dname[CDIR_FILENAME_LEN];
  Bool result=FALSE,unlock;
  I64 i;
 
  PrtChk(p);
  if (p->type!=PT_ISO9660)
    PutDefineErr("ST_NOT_ISO9660");
  else {
    try {
      unlock=PrtLock(p);
      isoptr=MAlloc(p->spc<<BLK_SIZE_BITS);
      RClusters(Fs->cur_prt,isoptr,cur_dir_cluster,1);
      if (isoptr->name_len==1 && !isoptr->name) { //curdir
	i=(isoptr->size.little+p->spc<<BLK_SIZE_BITS-1)/p->spc<<BLK_SIZE_BITS;
	buf=MAlloc(p->spc<<BLK_SIZE_BITS*i);
	RClusters(Fs->cur_prt,buf,cur_dir_cluster,i);
	Free(isoptr);
      } else {
	buf=isoptr;
	i=1;
      }
      i*=p->spc<<BLK_SIZE_BITS;
      isoptr=buf;
      while (i>0) {
	if (!isoptr->len) {
	  isoptr(U8 *)++;
	  i--;
	} else {
	  ISO1FromName(dname,&isoptr->name_len);
	  if (*dname) {
	    if (!StrCmp(name,dname)) {
	      result=ISO1CDirFill(p,_result,isoptr);
	      if (result &&
	      !(dir_only && !(_result->attr & _ATTR_DIR)) &&
	      !(file_only && _result->attr & _ATTR_DIR) )
		goto exit_this;
	      else
		result=FALSE;
	    }
	  }
	  i-=isoptr->len;
	  isoptr(U8 *)+=isoptr->len;
	}
      }
exit_this:
      Free(buf);
      if (unlock)
	PrtUnlock(p);
    } catch
      if (unlock)
	PrtUnlock(p);
  }
  return result;
}

U8 *ISO1FileRead(CPrt *p,U8 *cur_dir,U8 *filename,I64 *_size,
  I64 *_attr,Bool raw,CArcCompress **_ac2)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_cluster;
  CArcCompress *ac;
 
  PrtChk(p);
  if (_size) *_size=0;
  if (_ac2)  *_ac2=NULL;
  if (_attr) *_attr=0;
  if (p->type!=PT_ISO9660)
    PutDefineErr("ST_NOT_ISO9660");
  else {
    try {
      PrtLock(p);
      cur_dir_cluster=Name2DirCluster(p,cur_dir);
      if (ISO1FileFind(p,cur_dir_cluster,filename,&de,FALSE,TRUE)) {
	if (_attr) *_attr=de.attr;
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.cluster;
	c=RClustersBlks(p,buf,c,blk_cnt);
	if (FileAttr(de.name)&_ATTR_COMPRESSED && !raw) {
	  ac=buf;
	  if (_attr) *_attr=FileAttr(de.name,*_attr);
	  if (_size) *_size=ac->expanded_size;
	  if (_ac2) {
	    *_ac2=ac;
	    buf=ExpandBuf(ac);
	  } else {
	    buf=ExpandBuf(ac);
	    Free(ac);
	  }
	} else {
	  buf[de.size]=0; //Terminate
	  if (_size) *_size=de.size;
	}
	if (_attr) *_attr=FileAttr(de.name,*_attr);
      }
      PrtUnlock(p);
    } catch
      PrtUnlock(p);
  }
  return buf;
}

Bool ISO1Cd(U8 *name,I64 cur_dir_cluster)
{
  CDirEntry de;
  if (Fs->cur_prt->type!=PT_ISO9660)
    PutDefineErr("ST_NOT_ISO9660");
  else {
    if (ISO1FileFind(Fs->cur_prt,cur_dir_cluster,name,&de,TRUE,FALSE))
      return TRUE;
    else {
      "%s ",name;
      PutDefineErr("ST_FILE_NOT_FOUND");
    }
  }
  return FALSE;
}

CDirEntry *ISO1FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL)
{
  CPrt *p=Fs->cur_prt;
  CISO1DirEntry *buf,*buf2,*isoptr;
  I64 i,cur_dir_cluster=Name2DirCluster(p,Fs->cur_dir);
  CDirEntry *result=NULL,*tempm;
  isoptr=MAlloc(p->spc<<BLK_SIZE_BITS);
  RClusters(Fs->cur_prt,isoptr,cur_dir_cluster,1);
  if (isoptr->name_len==1 && !isoptr->name) { //curdir
    i=(isoptr->size.little+p->spc<<BLK_SIZE_BITS-1)/p->spc<<BLK_SIZE_BITS;
    buf=MAlloc(p->spc<<BLK_SIZE_BITS*i);
    RClusters(Fs->cur_prt,buf,cur_dir_cluster,i);
    Free(isoptr);
  } else {
    buf=isoptr;
    i=1;
  }
  buf2=buf;
  i*=p->spc<<BLK_SIZE_BITS;
  while (i>0) {
    if (!buf->len) {
      buf(U8 *)++;
      i--;
    } else {
      tempm=MAlloc(sizeof(CDirEntry));
      if (ISO1CDirFill(Fs->cur_prt,tempm,buf)) {
	tempm->parent=parent;
	if (Bt(&fuf_flags,FUf_RECURSE) && tempm->attr&_ATTR_DIR && *tempm->name!='.') {
	  tempm->next=result;
	  result=tempm;
	  tempm->full_name=DirNameAbs(tempm->name);
	  if (Cd(tempm->name)) {
	    tempm->sub=ISO1FilesFind(files_find_mask,fuf_flags,tempm);
	    Cd("..");
	  }
	} else if ((tempm->attr & _ATTR_DIR || !Bt(&fuf_flags,FUf_JUST_DIRS)) &&
	!(Bt(&fuf_flags,FUf_RECURSE) && *tempm->name=='.' && tempm->attr&_ATTR_DIR) &&
	   FilesFindMatch(files_find_mask,tempm->name,fuf_flags)) {
	  tempm->next=result;
	  result=tempm;
	  tempm->full_name=FileNameAbs(tempm->name);
	} else
	  CDirEntryDel(tempm);
      } else
	CDirEntryDel(tempm);
      i-=buf->len;
      buf(U8 *)+=buf->len;
    }
  }
  Free(buf2);
  return result;
}

