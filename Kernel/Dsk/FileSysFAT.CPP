U0 CDate2Dos(U16 *t,U16 *d,CDate cdt)
{
  CDateStruct ds;
  Date2Struct(&ds,cdt+local_time_offset);
  *d=ds.day_of_mon+(ds.mon+(ds.year-1980)<<4)<<5;
  *t=ds.sec>>1+(ds.min+ds.hour<<6)<<5;
}

CDate Dos2CDate(U16 t,U16 d)
{
  CDateStruct ds;
  MemSet(&ds,0,sizeof(CDateStruct));
  ds.day_of_mon=d&0x1F; d=d>>5;
  ds.mon=d&0xF;
  ds.year=d>>4+1980;
  ds.sec=(t&0x1F)*2; t=t>>5;
  ds.min=t&0x3F;
  ds.hour=t>>6;
  return Struct2Date(&ds)-local_time_offset;
}

U0 FAT32PrtInit(CPrt *p)
{
  CFAT32Boot br32;
  Bool unlock;
  try {
    unlock=PrtLock(p);
    p->type=PT_FAT32;
    RBlks(p,&br32,p->offset,1);
    p->file_system_info_sect=p->offset+br32.file_system_info_sect;
    p->map1=p->offset+br32.reserved_sects;
    p->map2=p->map1+br32.sects_per_fat;
    p->data=p->map2+br32.sects_per_fat-2*br32.sects_per_cluster; //Starts at Cluster 2
    p->spc=br32.sects_per_cluster;
    p->root_cluster=br32.root_cluster;
    MapBufAlloc(p,1);
    Free(p->fis);
    p->fis=AMAlloc(BLK_SIZE);
    RBlks(p,p->fis,p->file_system_info_sect,1);
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
}

Bool FAT32DrvValidate(U8 drv_let)
{
  CPrt *p;
  CFAT32Boot br;
  Bool result=FALSE;
  try {
    p=Drv2Prt(drv_let);
    if (p->type==PT_FAT32 &&
	RBlks(p,&br,p->offset,1) &&
	br.signature==0xAA55 &&
	br.sects==p->size)
      result=TRUE;
  } catch
    Fs->catch_except=TRUE;
  return result;
}

U0 FAT32Fmt(U8 drv_let,Bool quick=TRUE)
{
  CFAT32Boot *br=CAlloc(BLK_SIZE);
  CFAT32FileInfoSect *fis=CAlloc(BLK_SIZE);
  CPrt *p=Drv2Prt(drv_let);
  I64 i,l;
  try {
    PrtLock(p);
    SetDrvType(drv_let,PT_FAT32);
    p->type=PT_FAT32;
    br->jump_and_nop[0]=OC_JMP_REL8;
    br->jump_and_nop[1]=offset(CFAT32Boot.code)-2;
    br->jump_and_nop[2]=OC_NOP;
    MemCpy(br->oem_name,"MSWIN4.1",8);
    br->bytes_per_sect=BLK_SIZE;
    if	  (p->size<= 500000)
      br->sects_per_cluster=1;
    else if (p->size<=2000000)
      br->sects_per_cluster=2;
    else if (p->size<=6000000)
      br->sects_per_cluster=4;
    else if (p->size<=12000000)
      br->sects_per_cluster=8;
    else if (p->size<=33000000)
      br->sects_per_cluster=16;
    else if (p->size<=67000000)
      br->sects_per_cluster=32;
    else
      br->sects_per_cluster=64;
 
    br->reserved_sects=32;
    br->copies_of_fat=2;
    br->media_desc=0xF8;
    br->sects=p->size;
    l=(br->sects/br->sects_per_cluster)>>(BLK_SIZE_BITS-2)+1;
    br->sects_per_fat=l;
    br->root_cluster=2;
    br->file_system_info_sect=1;
    br->log_drv_num=0x80;
    br->ext_signature=0x29;
    br->serial_num=RandU32;
    MemCpy(br->vol_name,"NO NAME    ",11);
    MemCpy(br->fat_name,"FAT32   ",8);
    br->signature=0xAA55;
    fis->signature1=0x41615252;
    fis->signature2=0x61417272;
    fis->free_clusters=-1;
    fis->most_recently_allocated=0;
    fis->signature3=0xAA550000;
 
    if (quick)
      i=br->reserved_sects+2*l+4*br->sects_per_cluster;
    else
      i=p->size;
    WZeroBlks(p,p->offset,i);
 
    WBlks(p,fis,p->offset+br->file_system_info_sect,1);
    WBlks(p,br,p->offset,1);
    FAT32PrtInit(p);
    ClustersAlloc(p,0,1,FALSE); //Allocate #1
    br->root_cluster=ClustersAlloc(p,0,1,FALSE);
    WBlks(p,br,p->offset,1);
    FAT32PrtInit(p);
    PrtUnlock(p);
  } catch
    PrtUnlock(p);
  Free(br);
  Free(fis);
}

Bool FATNameTo(U8 *dst,U8 *src)
{
  I64 i;
  MemSet(dst,CH_SPACE,11);
  if (!FileNameChk(src))
    return FALSE;
  if (!StrCmp(src,"..")) {
    *dst='.';
    dst[1]='.';
    return TRUE;
  } else if (!StrCmp(src,".")) {
    *dst='.';
    return TRUE;
  }
  i=0;
  while (i<8 && *src && *src!='.')
    dst[i++]=ToUpper(*src++);
  i=8;
  if (*src=='.') src++;
  while (*src)
    if (*src!='.')
      dst[i++]=ToUpper(*src++);
    else
      src++;
  return TRUE;
}

I64 FATNameXSum(U8 *src)
{
  I64 i,result=0;
  for (i=0;i<11;i++)
    if (result&1)
      result.u8[0]=0x80+result>>1+*src++;
    else
      result.u8[0]=result>>1+*src++;
  return result;
}

Bool FATFromName(U8 *dst,U8 *src)
{
  I64 i,j,k=0;
  for (j=7;j>=0 && src[j]==CH_SPACE;j--);
  for(i=0;i<=j;i++)
    dst[k++]=src[i];
  for (j=10;j>=8 && src[j]==CH_SPACE;j--);
  if (*src!='.' && j!=7)
    dst[k++]='.';
  for(i=8;i<=j;i++)
    dst[k++]=src[i];
  dst[k++]=0;
  return FileNameChk(dst);
}

U8 fat_long_name_map[13]={
  offset(CFAT32DirEntryLong.name1),
  offset(CFAT32DirEntryLong.name1)+2,
  offset(CFAT32DirEntryLong.name1)+4,
  offset(CFAT32DirEntryLong.name1)+6,
  offset(CFAT32DirEntryLong.name1)+8,
  offset(CFAT32DirEntryLong.name2),
  offset(CFAT32DirEntryLong.name2)+2,
  offset(CFAT32DirEntryLong.name2)+4,
  offset(CFAT32DirEntryLong.name2)+6,
  offset(CFAT32DirEntryLong.name2)+8,
  offset(CFAT32DirEntryLong.name2)+10,
  offset(CFAT32DirEntryLong.name3),
  offset(CFAT32DirEntryLong.name3)+2
};

Bool DirLongNameFill(CDirEntry *tempde,CFAT32DirEntryLong *de,I64 *xsum)
{
  I64 i;
  U8 *ptr=de;
  if (de->ord&0x40) {
    MemSet(tempde,0,sizeof(CDirEntry));
    *xsum=de->xsum;
  } else if (de->type || de->zero || de->xsum!=*xsum) {
    MemSet(tempde,0,sizeof(CDirEntry));
    *xsum=0;
    return FALSE;
  }
  switch (de->ord&0x3F) {
    case 1:
      for (i=0;i<13;i++)
	if (!(tempde->name[i]=ptr[fat_long_name_map[i]]))
	  return TRUE;
      break;
    case 2:
      for (i=0;i<12;i++)
	if (!(tempde->name[i+13]=ptr[fat_long_name_map[i]]))
	  return TRUE;
      break;
  }
  return TRUE;
}

Bool FAT32CDirFill(CDirEntry *tempde,CFAT32DirEntry *de)
{
  Bool result;
  if (*tempde->name)
    result=TRUE;
  else
    result=FATFromName(tempde->name,de->name);
  tempde->cluster=de->cluster_lo+de->cluster_hi<<16;
  tempde->size=de->size;
  tempde->attr=de->attr;
  tempde->datetime=Dos2CDate(de->WrtTime,de->WrtDate);
  return result;
}

Bool FAT32DirFill(CFAT32DirEntry *de,CDirEntry *tempde,I64 *_de_cnt=NULL)
{//Fill up to 3 entries and store cnt of entries.
  I64 de_cnt=0,i,l,xsum,ord;
  U8 *ptr,dname[16];
  CFAT32DirEntryLong *ld=de;
  Bool result;

  MemSet(de,0,sizeof(CFAT32DirEntry));
  result=FATNameTo(de->name,tempde->name);
  FATFromName(dname,de->name);
  if (StrCmp(dname,tempde->name)) {
    ord=0x41;
    xsum=FATNameXSum(de->name);
    if ((l=StrLen(tempde->name))>13) {
      ptr=&ld[de_cnt];
      MemSet(ptr,0,sizeof(CFAT32DirEntryLong));
      ld[de_cnt].attr=RS_ATTR_LONG_NAME;
      ld[de_cnt].xsum=xsum;
      ld[de_cnt].ord=0x42;
      for (i=13;i<l;i++)
	ptr[fat_long_name_map[i-13]]=tempde->name[i];
      i++;
      for (;i<26;i++)
	ptr[fat_long_name_map[i-13]](U16)=0xFFFF;
      ord=1;
      l=13;
      de_cnt++;
    }
    ptr=&de[de_cnt];
    MemSet(ptr,0,sizeof(CFAT32DirEntryLong));
    ld[de_cnt].attr=RS_ATTR_LONG_NAME;
    ld[de_cnt].xsum=xsum;
    ld[de_cnt].ord=ord;
    for (i=0;i<l;i++)
      ptr[fat_long_name_map[i]]=tempde->name[i];
    i++;
    for (;i<13;i++)
      ptr[fat_long_name_map[i]](U16)=0xFFFF;
    de_cnt++;
    MemSet(&de[de_cnt],0,sizeof(CFAT32DirEntry));
    result=FATNameTo(de[de_cnt].name,tempde->name);
  }
  de[de_cnt].cluster_lo=tempde->cluster.u16[0];
  de[de_cnt].cluster_hi=tempde->cluster.u16[1];
  if (!(tempde->attr&RS_ATTR_DIR))
    de[de_cnt].size=tempde->size;
  de[de_cnt].attr=tempde->attr;
  if (!tempde->datetime)
    tempde->datetime=Now;
  CDate2Dos(&de[de_cnt].WrtTime,&de[de_cnt].WrtDate,tempde->datetime);
  if (_de_cnt)
    *_de_cnt=de_cnt+1;
  return result;
}

Bool FAT32FileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$
  Bool result=FALSE,unlock;
  CFAT32DirEntry *buf;
  I64 xsum=0,attr,cur_dir_entry,entries_per_cluster;
  U8 dname[CDIR_FILENAME_LEN],ch;
  CDirEntry long_name;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  MemSet(_result,0,sizeof(CDirEntry));
  MemSet(&long_name,0,sizeof(CDirEntry));
  PrtChk(p);
  if (p->type!=PT_FAT32)
    PrintErr("Not FAT32 Partition\n");
  else if (!CFileNameTo(dname,name))
    PrintErr("Invalid FileName.\n");
  else
    try {
      unlock=PrtLock(p);
      buf=MAlloc(BLK_SIZE*p->spc);
      entries_per_cluster=p->spc<<FAT32_ENTRIES_BITS;
      RClusters(p,buf,cur_dir_cluster,1);
      cur_dir_entry=0;
      while (ch=*buf[cur_dir_entry].name) {
	attr=buf[cur_dir_entry].attr;
	if (ch!=0xE5) {
	  if (attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
	    DirLongNameFill(&long_name,&buf[cur_dir_entry],&xsum);
	  else {
	    if (!(attr&RS_ATTR_VOL_ID)) {
	      if (xsum==FATNameXSum(buf[cur_dir_entry].name))
		MemCpy(_result,&long_name,sizeof(CDirEntry));
	      else
		MemSet(_result,0,sizeof(CDirEntry));
	      if (!(fuf_flags&FUF_JUST_DIRS && !(attr & RS_ATTR_DIR)) &&
	      !(fuf_flags&FUF_JUST_FILES && attr & RS_ATTR_DIR) &&
	      FAT32CDirFill(_result,&buf[cur_dir_entry]) &&
	      !StrCmp(dname,_result->name)) {
			      result=TRUE;
		goto exit_this;
	      }
	    }
	    MemSet(&long_name,0,sizeof(CDirEntry));
	  }
	} else
	  MemSet(&long_name,0,sizeof(CDirEntry));
	if (++cur_dir_entry==entries_per_cluster) {
	  cur_dir_cluster=ClusterNumNext(p,cur_dir_cluster);
	  if (!(0<cur_dir_cluster<0x0FFFFFF8))
	    break;
	  else {
	    RClusters(p,buf,cur_dir_cluster,1);
	    cur_dir_entry=0;
	  }
	}
      }
      MemSet(_result,0,sizeof(CDirEntry));
exit_this:
      Free(buf);
      if (unlock)
	PrtUnlock(p);
    } catch
      if (unlock)
	PrtUnlock(p);
  return result;
}

U8 *FAT32FileRead(CPrt *p,U8 *cur_dir,U8 *filename,I64 *_size,I64 *_attr)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_cluster;
  PrtChk(p);
  *_size=0;
  *_attr=0;
  if (p->type!=PT_FAT32)
    PrintErr("Not FAT32 Partition\n");
  else
    try {
      PrtLock(p);
      cur_dir_cluster=Name2DirCluster(p,cur_dir);
      if (FAT32FileFind(p,cur_dir_cluster,filename,&de,FUF_JUST_FILES)) {
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.cluster;
	if (!(0<c<0x0FFFFFF8))
	  c=0x0FFFFFFF;
	else
	  c=RClustersBlks(p,buf,c,blk_cnt);
	buf[de.size]=0; //Terminate
	*_size=de.size;
	*_attr=FileAttr(de.name,de.attr);
      }
      PrtUnlock(p);
    } catch
      PrtUnlock(p);
  return buf;
}

Bool FAT32Cd(U8 *name,I64 cur_dir_cluster)
{
  CDirEntry de;
  if (Fs->cur_prt->type!=PT_FAT32)
    PrintErr("Not FAT32 Partition\n");
  else if (FAT32FileFind(Fs->cur_prt,cur_dir_cluster,name,&de,FUF_JUST_DIRS))
    return TRUE;
  else {
    "%s ",name;
    PrintErr("File not found.\n");
  }
  return FALSE;
}

U0 FAT32FreeClusters(CPrt *p,I64 c)
{
  I64 next,saved_c=c;
  Bool unlock,unlock_break;
  PrtChk(p);
  if (!(0<c<0x0FFFFFF8)) return;
  if (p->type!=PT_FAT32)
    PrintErr("Not FAT32 Partition\n");
  else
    try {
      unlock_break=BreakLock;
      unlock=PrtLock(p);
      CleanMAPBuf(p);
      do {
	SetFSBuf(p,c,0);
	next=p->cur_map_blk[c&(BLK_SIZE/4-1)];
	p->cur_map_blk[c&(BLK_SIZE/4-1)]=0;
	LBts(p->buffered_map_dirty_bits,0);
	c=next;
      } while (0<c<0x0FFFFFF8);
      CleanMAPBuf(p,0);
 
      c=saved_c;
      do {
	SetFSBuf(p,c,1);
	next=p->cur_map_blk[c&(BLK_SIZE/4-1)];
	p->cur_map_blk[c&(BLK_SIZE/4-1)]=0;
	LBts(p->buffered_map_dirty_bits,0);
	c=next;
      } while (0<c<0x0FFFFFF8);
      CleanMAPBuf(p,1);
      if (unlock)
	PrtUnlock(p);
      if (unlock_break)
	BreakUnlock;
    } catch {
      if (unlock)
	PrtUnlock(p);
      if (unlock_break)
	BreakUnlock;
    }
}

I64 FAT32AllocClusters(CPrt *p,I64 c,I64 cnt)
{
  Bool wrap_around=FALSE,unlock,unlock_break;
  I64 first=INVALID_CLUSTER,j,l;
 
  if (cnt<=0) return 0x0FFFFFFF;
  try {
    unlock_break=BreakLock;
    unlock=PrtLock(p);
    l=(p->size+p->offset-p->data)/p->spc-1;
    j=p->fis->most_recently_allocated;
    while (cnt-->0) {
      while (TRUE) {
	j++;
	if (j<1) j=1;
	if (j>=l) {
	  if (wrap_around)
	    Dbg;
	  j=1;
	  wrap_around=TRUE;
	}
	SetFSBuf(p,j);
	if (!p->cur_map_blk[j&(BLK_SIZE/4-1)])
	  break;
      }
      if (!(0<first<0x0FFFFFF8))
	first=j;
      if (0<c<l) {
	SetFSBuf(p,c);
	p->cur_map_blk[c&(BLK_SIZE/4-1)]=j;
	LBts(p->buffered_map_dirty_bits,0);
      }
      c=j;
    }
 
    if (0<c<l) {
      SetFSBuf(p,c);
      p->cur_map_blk[c&(BLK_SIZE/4-1)]=0x0FFFFFFF;
      LBts(p->buffered_map_dirty_bits,0);
    }
    CleanMAPBuf(p);
 
    p->fis->most_recently_allocated=j;
    p->fis->free_clusters=-1;
    WBlks(p,p->fis,p->file_system_info_sect,1);
  } catch {
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  if (unlock)
    PrtUnlock(p);
  if (unlock_break)
    BreakUnlock;
  return first;
}

I64 FAT32AllocContiguousClusters(CPrt *p,I64 cnt)
{
  I64 i,first=1;
  Bool cont,unlock,unlock_break;
 
  if (cnt<=0) return 0x0FFFFFFF;
  try {
    unlock_break=BreakLock;
    unlock=PrtLock(p);
    while (TRUE) {
      first++;
      i=0;
      cont=TRUE;
      while (cont && i<cnt) {
	if ((first+i+1)*p->spc+p->data>p->size+p->offset)
	  Dbg;
	SetFSBuf(p,first+i);
	if (p->cur_map_blk[(first+i)&(BLK_SIZE/4-1)])
	  cont=FALSE;
	else
	  i++;
      }
      if (!cont)
	first=first+i;
      else {
	CleanMAPBuf(p);
 
	for (i=0;i<cnt;i++) {
	  SetFSBuf(p,first+i,0);
	  if (i+1==cnt)
	    p->cur_map_blk[(first+i)&(BLK_SIZE/4-1)]=0x0FFFFFFF;
	  else
	    p->cur_map_blk[(first+i)&(BLK_SIZE/4-1)]=first+i+1;
	  LBts(p->buffered_map_dirty_bits,0);
	}
	CleanMAPBuf(p,0);
 
	for (i=0;i<cnt;i++) {
	  SetFSBuf(p,first+i,1);
	  if (i+1==cnt)
	    p->cur_map_blk[(first+i)&(BLK_SIZE/4-1)]=0x0FFFFFFF;
	  else
	    p->cur_map_blk[(first+i)&(BLK_SIZE/4-1)]=first+i+1;
	  LBts(p->buffered_map_dirty_bits,0);
	}
	CleanMAPBuf(p,1);
	break;
      }
    }
  } catch {
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  if (unlock)
    PrtUnlock(p);
  if (unlock_break)
    BreakUnlock;
  return first;
}

Bool FAT32DirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempde,Bool free_old_chain)
{
//See $LK,"::/Doc/CutCorners.TXT"$.
  CFAT32DirEntry *buf,*last_buf,*temp_buf,de[3];
  I64 i,attr,avail_cnt,de_cnt,c,
      cur_dir_entry,entries_per_cluster,
      cur_dir_cluster,xsum=0,last_dir_cluster=INVALID_CLUSTER;
  U8 ch;
  Bool written=FALSE,unlock,unlock_break;
  CDirEntry long_name;
  FAT32DirFill(&de,tempde,&de_cnt);
  MemSet(&long_name,0,sizeof(CDirEntry));
  try {
    unlock_break=BreakLock;
    unlock=PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,cur_dir);
    buf     =MAlloc(BLK_SIZE*p->spc);
    last_buf=CAlloc(BLK_SIZE*p->spc);
    entries_per_cluster=p->spc<<FAT32_ENTRIES_BITS;
    RClusters(p,buf,cur_dir_cluster,1);
    cur_dir_entry=0;
    while (ch=*buf[cur_dir_entry].name) {
      attr=buf[cur_dir_entry].attr;
      if (ch!=0xE5 && attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
	DirLongNameFill(&long_name,&buf[cur_dir_entry],&xsum);
      else {
	avail_cnt=FAT32_ENTRIES_PER_BLK-cur_dir_entry & (FAT32_ENTRIES_PER_BLK-1);
	for (i=0;i<avail_cnt;i++)
	  if (*buf[cur_dir_entry+i].name!=0xE5) {
	    if (*buf[cur_dir_entry+i].name)
	      avail_cnt=i;
	    break;
	  }
	if (ch==0xE5 && !written && avail_cnt>=de_cnt) {
	  MemCpy(&buf[cur_dir_entry],&de,de_cnt*sizeof(CFAT32DirEntry));
	  WBlks(p,&buf[cur_dir_entry & -FAT32_ENTRIES_PER_BLK],
	      p->data+cur_dir_cluster*p->spc
	      +cur_dir_entry>>FAT32_ENTRIES_BITS,1);
	  cur_dir_entry+=de_cnt-1; //gets inc'ed
	  written=TRUE;
	} else if (ch!=0xE5 && !(attr&RS_ATTR_VOL_ID)) {
	  if (xsum!=FATNameXSum(buf[cur_dir_entry].name))
	    MemSet(&long_name,0,sizeof(CDirEntry));
	  if (!*long_name.name)
	    FATFromName(long_name.name,buf[cur_dir_entry].name);
	    //Del old entry with same name
	  if (!StrCmp(long_name.name,tempde->name)) {
	    if (free_old_chain)
	      FAT32FreeClusters(p,buf[cur_dir_entry].cluster_lo+
		  buf[cur_dir_entry].cluster_hi<<16);
	    if (!written) {
	      MemCpy(&buf[cur_dir_entry],&de[de_cnt-1],sizeof(CFAT32DirEntry));
	      WBlks(p,&buf[cur_dir_entry & -FAT32_ENTRIES_PER_BLK],
		  p->data+cur_dir_cluster*p->spc
		  +cur_dir_entry>>FAT32_ENTRIES_BITS,1);
	      written=TRUE;
	    } else {
	      *buf[cur_dir_entry].name=0xE5;
	      i=1;
	      while (i<=cur_dir_entry &&
		  buf[cur_dir_entry-i].attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
		*buf[cur_dir_entry-i++].name=0xE5;
	      i--;
	      WBlks(p,&buf[(cur_dir_entry-i)&-FAT32_ENTRIES_PER_BLK],
		  p->data+cur_dir_cluster*p->spc
		  +(cur_dir_entry-i)>>FAT32_ENTRIES_BITS,
		  (i+FAT32_ENTRIES_PER_BLK)>>FAT32_ENTRIES_BITS);
	      if (i==cur_dir_entry && 0<last_dir_cluster<0x0FFFFFF8) {
		i=1;
		while (i<=entries_per_cluster &&
		    last_buf[entries_per_cluster-i].attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
		  *last_buf[entries_per_cluster-i++].name=0xE5;
		if (--i>0)
		  WBlks(p,&buf[(entries_per_cluster-i)&-FAT32_ENTRIES_PER_BLK],
		      p->data+last_dir_cluster*p->spc
		      +(entries_per_cluster-i)>>FAT32_ENTRIES_BITS,
		      (i+FAT32_ENTRIES_PER_BLK-1)>>FAT32_ENTRIES_BITS);
	      }
	    }
	    break;
	  }
	}
	MemSet(&long_name,0,sizeof(CDirEntry));
      }
      if (++cur_dir_entry==entries_per_cluster) {
	last_dir_cluster=cur_dir_cluster;
	temp_buf=buf; buf=last_buf; last_buf=temp_buf;
	c=ClusterNumNext(p,cur_dir_cluster);
	if (!(0<c<0x0FFFFFF8)) {
	  c=ClustersAlloc(p,cur_dir_cluster,1,FALSE);
	  MemSet(buf,0,BLK_SIZE*p->spc);
	  WClusters(p,buf,c,1);
	} else
	  RClusters(p,buf,c,1);
	cur_dir_cluster=c;
	cur_dir_entry=0;
      }
    }
    if (!written) {
      avail_cnt=FAT32_ENTRIES_PER_BLK-cur_dir_entry & (FAT32_ENTRIES_PER_BLK-1);
      if (avail_cnt<de_cnt) {
	for (i=0;i<avail_cnt;i++)
	  *buf[cur_dir_entry+i].name=0xE5;
	WBlks(p,&buf[cur_dir_entry &-FAT32_ENTRIES_PER_BLK],
	    p->data+cur_dir_cluster*p->spc
	    +cur_dir_entry>>FAT32_ENTRIES_BITS,1);
	cur_dir_entry+=avail_cnt;
	if (cur_dir_entry==entries_per_cluster) {
	  last_dir_cluster=cur_dir_cluster;
	  temp_buf=buf; buf=last_buf; last_buf=temp_buf;
	  cur_dir_cluster=ClustersAlloc(p,cur_dir_cluster,1);
	  cur_dir_entry=0;
	  MemSet(buf,0,BLK_SIZE*p->spc);
	  WClusters(p,buf,cur_dir_cluster,1);
	}
      }
      MemCpy(&buf[cur_dir_entry],&de,de_cnt*sizeof(CFAT32DirEntry));
      WBlks(p,&buf[cur_dir_entry &-FAT32_ENTRIES_PER_BLK],
	  p->data+cur_dir_cluster*p->spc+
	  cur_dir_entry>>FAT32_ENTRIES_BITS,1);
      cur_dir_entry+=de_cnt;
      if (cur_dir_entry==entries_per_cluster) {
	cur_dir_cluster=ClustersAlloc(p,cur_dir_cluster,1);
	MemSet(buf,0,BLK_SIZE*p->spc);
	WClusters(p,buf,cur_dir_cluster,1);
      } else {
	MemSet(&buf[cur_dir_entry],0,sizeof(CFAT32DirEntry));
	WBlks(p,&buf[cur_dir_entry &-FAT32_ENTRIES_PER_BLK],
	    p->data+cur_dir_cluster*p->spc
	    +cur_dir_entry>>FAT32_ENTRIES_BITS,1);
      }
    }
    Free(last_buf);
    Free(buf);
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  } catch {
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  return FALSE;
}

I64 FAT32FilesDel(CPrt *p,U8 *cur_dir,U8 *files_find_mask,I64 fuf_flags,
		     Bool del_dir,Bool print_msg)
{
  CFAT32DirEntry *buf,*last_buf,*temp_buf;
  I64 i,result=0,attr,xsum=0,last_dir_cluster=INVALID_CLUSTER,
    cur_dir_entry,entries_per_cluster,cur_dir_cluster;
  U8 ch;
  Bool unlock_break;
  CDirEntry long_name;
  MemSet(&long_name,0,sizeof(CDirEntry));
  try {
    unlock_break=BreakLock;
    PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,cur_dir);
    buf     =MAlloc(BLK_SIZE*p->spc);
    last_buf=CAlloc(BLK_SIZE*p->spc);
    entries_per_cluster=p->spc<<FAT32_ENTRIES_BITS;
    RClusters(p,buf,cur_dir_cluster,1);
    cur_dir_entry=0;
    while (ch=*buf[cur_dir_entry].name) {
      attr=buf[cur_dir_entry].attr;
      if (ch!=0xE5 && ch!='.') {
	if (attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
	  DirLongNameFill(&long_name,&buf[cur_dir_entry],&xsum);
	else {
	  if (!(attr & RS_ATTR_VOL_ID) &&
	  (del_dir || !(attr & RS_ATTR_DIR))) {
	    if (xsum!=FATNameXSum(buf[cur_dir_entry].name))
	      MemSet(&long_name,0,sizeof(CDirEntry));
	    if (!*long_name.name)
	      FATFromName(long_name.name,buf[cur_dir_entry].name);
	    if (FilesFindMatch(long_name.name,files_find_mask,fuf_flags)) {
	      if (!(attr & RS_ATTR_DIR)) result++;
	      if (print_msg)
		"Del %s\n",long_name.name;
	      *buf[cur_dir_entry].name=0xE5;
	      i=1;
	      while (i<=cur_dir_entry &&
	      buf[cur_dir_entry-i].attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
		*buf[cur_dir_entry-i++].name=0xE5;
	      i--;
	      WBlks(p,&buf[(cur_dir_entry-i)&-FAT32_ENTRIES_PER_BLK],
	      p->data+cur_dir_cluster*p->spc
	      +(cur_dir_entry-i)>>FAT32_ENTRIES_BITS,
	      (i+FAT32_ENTRIES_PER_BLK)>>FAT32_ENTRIES_BITS);
	      if (i==cur_dir_entry && last_dir_cluster!=INVALID_CLUSTER) {
		i=1;
		while (i<=entries_per_cluster &&
		last_buf[entries_per_cluster-i].attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
		  *last_buf[entries_per_cluster-i++].name=0xE5;
		if (--i>0)
		  WBlks(p,&buf[(entries_per_cluster-i)&-FAT32_ENTRIES_PER_BLK],
		  p->data+last_dir_cluster*p->spc
		  +(entries_per_cluster-i)>>FAT32_ENTRIES_BITS,
		  (i+FAT32_ENTRIES_PER_BLK-1)>>FAT32_ENTRIES_BITS);
	      }
	      FAT32FreeClusters(p,buf[cur_dir_entry].cluster_lo+
	      buf[cur_dir_entry].cluster_hi<<16);
	    }
	  }
	  MemSet(&long_name,0,sizeof(CDirEntry));
	}
      } else
	MemSet(&long_name,0,sizeof(CDirEntry));
      if (++cur_dir_entry==entries_per_cluster) {
	last_dir_cluster=cur_dir_cluster;
	cur_dir_cluster=ClusterNumNext(p,cur_dir_cluster,1);
	temp_buf=buf; buf=last_buf; last_buf=temp_buf;
	RClusters(p,buf,cur_dir_cluster,1);
	cur_dir_entry=0;
      }
    }
    Free(buf);
    Free(last_buf);
    PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  } catch {
    PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  return result;
}

I64 FAT32FileWrite(CPrt *p,U8 *cur_dir,U8 *name,U8 *buf,I64 size,
	CDate cdt,I64 attr)
{
  CDirEntry de;
  I64 c=0,blk_cnt;
  Bool contiguous;
  MemSet(&de,0,sizeof(CDirEntry));
  if (size<0) size=0;
  if (p->type!=PT_FAT32)
    PrintErr("Not FAT32 Partition\n");
  else if (!CFileNameTo(de.name,name))
    PrintErr("Invalid FileName.\n");
  else {
    FAT32FilesDel(p,cur_dir,de.name,0,FALSE,FALSE);
    if (attr & RS_ATTR_CONTIGUOUS)
      contiguous=TRUE;
    else
      contiguous=FALSE;
    de.size=size;
    if (blk_cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS)
      c=ClustersAlloc(p,0,(blk_cnt+p->spc-1)/p->spc,contiguous);
    else
      c=0x0FFFFFFF;
    de.cluster=c;
    de.attr=attr;
    de.datetime=cdt;
    if (blk_cnt)
      WClustersBlks(p,buf,c,blk_cnt);
    FAT32DirNew(p,cur_dir,&de,TRUE);
  }
  return c;
}

CDirEntry *FAT32FilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL,I64 *_dir_size=NULL)
{
  CPrt *p=Fs->cur_prt;
  CFAT32DirEntry *buf;
  I64 attr,xsum=0,dir_size=0,sub_dir_size,
  cur_dir_cluster,cur_dir_entry,entries_per_cluster;
  U8 ch;
  CDirEntry *result=NULL,*tempde,long_name;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');
  try {
    MemSet(&long_name,0,sizeof(CDirEntry));
    PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,Fs->cur_dir);
    buf=MAlloc(BLK_SIZE*p->spc);
    entries_per_cluster=p->spc<<FAT32_ENTRIES_BITS;
    RClusters(Fs->cur_prt,buf,cur_dir_cluster,1);
    dir_size+=Fs->cur_prt->spc*BLK_SIZE;
    cur_dir_entry=0;
    while (ch=*buf[cur_dir_entry].name) {
      attr=buf[cur_dir_entry].attr;
      if (ch!=0xE5) {
	if (attr&RS_ATTR_LONG_NAME_MASK==RS_ATTR_LONG_NAME)
	  DirLongNameFill(&long_name,&buf[cur_dir_entry],&xsum);
	else {
	  if (!(attr&RS_ATTR_VOL_ID)) {
	    tempde=MAlloc(sizeof(CDirEntry));
	    if (xsum==FATNameXSum(buf[cur_dir_entry].name))
	      MemCpy(tempde,&long_name,sizeof(CDirEntry));
	    else
	      MemSet(tempde,0,sizeof(CDirEntry));
	    if (FAT32CDirFill(tempde,&buf[cur_dir_entry])) {
	      tempde->parent=parent;
	      if (Bt(&fuf_flags,FUf_RECURSE) && attr&RS_ATTR_DIR && *tempde->name!='.') {
		tempde->next=result;
		result=tempde;
		tempde->full_name=DirNameAbs(tempde->name);
		PrtUnlock(p);
		if (Cd(tempde->name)) {
		  tempde->sub=FAT32FilesFind(files_find_mask,fuf_flags,tempde,&sub_dir_size);
		  tempde->size=sub_dir_size;
		  Cd("..");
		}
		PrtLock(p);
	      } else if ((attr&RS_ATTR_DIR || !Bt(&fuf_flags,FUf_JUST_DIRS)) &&
	      !(Bt(&fuf_flags,FUf_RECURSE) && *tempde->name=='.' && attr&RS_ATTR_DIR) &&
		FilesFindMatch(tempde->name,files_find_mask,fuf_flags)) {
		tempde->next=result;
		result=tempde;
		tempde->full_name=FileNameAbs(tempde->name);
	      } else
		DirEntryDel(tempde);
	    } else
	      DirEntryDel(tempde);
	  }
	  MemSet(&long_name,0,sizeof(CDirEntry));
	}
      } else
	MemSet(&long_name,0,sizeof(CDirEntry));
      if (++cur_dir_entry==entries_per_cluster) {
	cur_dir_cluster=ClusterNumNext(p,cur_dir_cluster);
	if (cur_dir_cluster==INVALID_CLUSTER)
	  break;
	else {
	  RClusters(p,buf,cur_dir_cluster,1);
	  dir_size+=Fs->cur_prt->spc*BLK_SIZE;
	  cur_dir_entry=0;
	}
      }
    }
    Free(buf);
    PrtUnlock(p);
  } catch
    PrtUnlock(p);
  if (_dir_size)
    *_dir_size=dir_size;
  return result;
}

Bool FAT32MkDir(CPrt *p,U8 *cur_dir,U8 *name)
{
  I64 size=p->spc<<BLK_SIZE_BITS,c,
      cur_dir_cluster=Name2DirCluster(p,cur_dir);
  U8 *buf=CAlloc(size);
  CDirEntry d_native;
  CFAT32DirEntry *dFAT=buf;
  Bool unlock_break;
 
  try {
    unlock_break=BreakLock;
    c=FileWrite(name,buf,size,0,RS_ATTR_DIR,FALSE);
    MemSet(&d_native,0,sizeof(CDirEntry));
    d_native.attr=RS_ATTR_DIR;
    *d_native.name='.';
    d_native.name[1]=0;
    d_native.cluster=c;
    d_native.size=0;
    d_native.datetime=Now;
    FAT32DirFill(dFAT,&d_native);
    dFAT++;
 
    MemSet(&d_native,0,sizeof(CDirEntry));
    d_native.attr=RS_ATTR_DIR;
    *d_native.name='.';
    d_native.name[1]='.';
    d_native.name[2]=0;
    d_native.cluster=cur_dir_cluster;
    d_native.size=0;
    d_native.datetime=Now;
    FAT32DirFill(dFAT,&d_native);
    WClusters(p,buf,c,1);
    Free(buf);
    if (unlock_break)
      BreakUnlock;
  } catch
    if (unlock_break)
      BreakUnlock;
  return TRUE;
}
