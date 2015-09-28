#help_index "File/Internal"
I64 DirTreeSerializeSize(CDirEntry *tempde)
{
  I64 result=0;
  while (tempde) {
    result+=CDIR_SIZE+1;
    if (tempde->attr & RS_ATTR_DIR)
      result+=DirTreeSerializeSize(tempde->sub);
    tempde=tempde->next;
  }
  return result+1;
}
I64 DirTreeSerializeFill(CDirEntry *tempde,U8 *dst)
{
  I64 result=0,i;
  while (tempde) {
    *dst++=1;
    result++;
    MemCpy(dst,&tempde->start,CDIR_SIZE);
    dst+=CDIR_SIZE;
    result+=CDIR_SIZE;
    if (tempde->attr & RS_ATTR_DIR) {
      i=DirTreeSerializeFill(tempde->sub,dst);
      dst+=i;
      result+=i;
    }
    tempde=tempde->next;
  }
  *dst=0;
  return result+1;
}
public U8 *DirTreeSerialize(CDirEntry *tempde,I64 *_size=NULL)
{//Serialize tree returned from $LK,"FilesFind",A="MN:FilesFind"$() into a one contiguous U8 array.
  I64 size=DirTreeSerializeSize(tempde);
  U8 *buf=MAlloc(size);
  DirTreeSerializeFill(tempde,buf);
  if (_size) *_size=size;
  return buf;
}

U8 *DirTreeUnserialize2(U8 *src,CDirEntry **tempde)
{
  CDirEntry *tempde1;
  if (*src++) {
    tempde1=CAlloc(sizeof(CDirEntry));
    *tempde=tempde1;
    MemCpy(&tempde1->start,src,CDIR_SIZE);
    src+=CDIR_SIZE;
    if (tempde1->attr & RS_ATTR_DIR)
      src=DirTreeUnserialize2(src,&tempde1->sub);
    src=DirTreeUnserialize2(src,&tempde1->next);
  } else
    *tempde=NULL;
  return src;
}
public CDirEntry *DirTreeUnserialize(U8 *src)
{//Unserialize tree to make it like a tree returned from $LK,"FilesFind",A="MN:FilesFind"$().
  CDirEntry *tempde=NULL;
  DirTreeUnserialize2(src,&tempde);
  return tempde;
}

#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically)"

#define ROUND_DRV_TO	(63*255)
#define DRV_HEADER	63

class CPlannedDrv
{
  CPlannedDrv *next,*last;
  I64 size;
  Bool pri;
};

//Available to assist in writing scripts.
I64 drv_dsk_dsk_size;

public U0 PrtDsk(U8 drv_let=0)
{//Partition the disk containing partition drv_let.
//drv_let=0 means add new drive that
  //is not already mounted.
  CBlkDev *bd;
  CPlannedDrv root,*temppp;
  CMasterBoot mbr;
  Bool pri=TRUE;
  I64 ext_base,drv_let2,pri_cnt=0,
	i,start_offset,offset,remaining;
  "This command does not play well\n"
	"with other operating systems.\n"
	"You really should use another\n"
	"operating system's partitioner.\n"
	"If you use this, it may, in fact,\n"
	"make your hard drive impossible\n"
	"to repartition with other operating\n"
	"until you set block zero to zero\n"
	"with $$LK,\"MasterBootZero\",\"MN:MasterBootZero\"$$()\n\n\n"
	"Continue";
  if (!YorN) return;

  if (drv_let)
    try
	    Let2BlkDev(drv_let);
    catch {
      drv_let=0;
      Fs->catch_except=TRUE;
    }

  if (!drv_let) {
    Mount(TRUE);
    return;
  }

  bd=Let2BlkDev(drv_let);
  if (!bd || bd->type!=BDT_ATA)
    return;

  drv_dsk_dsk_size=bd->max_blk+1;

  QueInit(&root);

  drv_let2=bd->first_drv_let;
  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO);
  while (FloorU64(remaining,ROUND_DRV_TO)>=ROUND_DRV_TO) {
    temppp=MAlloc(sizeof(CPlannedDrv));
    do {
      "$$RED$$Partition %C$$FG$$\n",drv_let2;
      temppp->pri=FALSE;
      if (pri) {
	"Primary Partition";
	if (YorN) {
	  pri_cnt++;
	  temppp->pri=TRUE;
	  if (pri_cnt==3)
	    pri=FALSE;
	} else
	  pri=FALSE;
      }
      "Blocks Remaining:%d (0x%X)\n",
	    remaining-DRV_HEADER,remaining-DRV_HEADER;
      temppp->size=CeilU64(GetI64("Size in Blocks  :",
	    remaining-DRV_HEADER)+DRV_HEADER,ROUND_DRV_TO);
    } while (!(ROUND_DRV_TO<=temppp->size<=FloorU64(remaining,ROUND_DRV_TO)));
    QueIns(temppp,root.last);
    remaining-=temppp->size;
    drv_let2++;
  }

  "\n\n!!! Repartition Drive !!!\n\n";
  temppp=root.next;
  drv_let2=bd->first_drv_let;
  while (temppp!=&root) {
    "Drive %C:%08X ",drv_let2,temppp->size;
    if (temppp->pri)
      "Primary\n";
    else
      "Logical\n";
    temppp=temppp->next;
    drv_let2++;
  }
  if (!AreYouSure)
    goto pd_done;

  remaining=FloorU64(bd->max_blk+1,ROUND_DRV_TO)-ROUND_DRV_TO;
  temppp=root.next;
  MemSet(&mbr,0,BLK_SIZE);
  mbr.signature=0xAA55;
  offset=0;
  for (i=0;i<pri_cnt;i++) {
    mbr.p[i].active=0x80;
    mbr.p[i].start_head=0;
    mbr.p[i].start_cyl=0x101;
    mbr.p[i].type=1; //Will get set different.
    mbr.p[i].end_head=0xFE;
    mbr.p[i].end_cyl=0xFFFF;
    mbr.p[i].offset=DRV_HEADER+offset;
    mbr.p[i].size=temppp->size-DRV_HEADER;
    offset+=temppp->size;
    remaining-=temppp->size;
    temppp=temppp->next;
  }
  if (!i) i++;
  if (temppp!=&root) {
    mbr.p[i].active=0x80;
    mbr.p[i].start_head=0;
    mbr.p[i].start_cyl=0x101;
    mbr.p[i].type=0xF;
    mbr.p[i].end_head=0xFE;
    mbr.p[i].end_cyl=0xFFFF;
    mbr.p[i].offset=offset;
    mbr.p[i].size=remaining;
    ext_base=offset;
  }
  ATAWriteBlks(bd,&mbr,0,1);

  while (temppp!=&root) {
    start_offset=offset;
    MemSet(&mbr,0,BLK_SIZE);
    mbr.signature=0xAA55;

    mbr.p[0].active=0x80;
    mbr.p[0].start_head=1;
    mbr.p[0].start_cyl=0x101;
    mbr.p[0].type=1; //Will get set different.
    mbr.p[0].end_head=0xFE;
    mbr.p[0].end_cyl=0xFFFF;
    mbr.p[0].offset=DRV_HEADER;
    mbr.p[0].size=temppp->size-DRV_HEADER;
    offset+=temppp->size;
    temppp=temppp->next;
    if (temppp!=&root) {
      mbr.p[1].active=0x80;
      mbr.p[1].start_head=0;
      mbr.p[1].start_cyl=0x101;
      mbr.p[1].type=5;
      mbr.p[1].end_head=0xFE;
      mbr.p[1].end_cyl=0xFFFF;
      mbr.p[1].offset=offset-ext_base;
      mbr.p[1].size=temppp->size;
    }
    ATAWriteBlks(bd,&mbr,start_offset,1);
  }

  bd->flags&=~(BDF_INITIALIZED | BDF_INIT_IN_PROGRESS);
  BlkDevAdd(bd,FALSE,TRUE);
  for (i=bd->first_drv_let;i<drv_let2;i++)
    Fmt(i,,FALSE);

    pd_done:
  while (root.next!=&root) {
    temppp=root.next;
    QueRem(temppp);
    Free(temppp);
  }
}

#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically)"

U0 RedSeaDrvView(U8 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  I64 lohi,c1,i,x,y,l,s;
  U8 *bitmap;
  CDC *dc=DCAlias;

  l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
	s=(dv->size+dv->spc-1)/dv->spc;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=((s+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(dv,bitmap,dv->fat1,i);
    i=0;
    for (y=0;y<GR_HEIGHT-3*FONT_HEIGHT;y++) {
      if (ScanKey)
	break;
      for (x=0;x<GR_WIDTH-FONT_WIDTH<<1;x++) {
	lohi=i*s;
	c1=lohi/l;
	if (Bt(bitmap,c1))
	  dc->color=ROP_XOR+BLUE^TRANSPARENT;
	else
	  dc->color=ROP_XOR+WHITE^TRANSPARENT;
	GrPlot(dc,x,y);
	i++;
      }
    }
    Free(bitmap);
  } catch
    DrvUnlock(dv);
  GetChar;

  SettingsPop;
  DCFill;
  DCDel(dc);
}
U0 FAT32DrvView(U8 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  I64 lohi,c1,i,x,y,l,s;
  U32 *bitmap;
  CDC *dc=DCAlias;
  l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
	s=(dv->size+dv->spc-1)/dv->spc;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=(s*4+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(dv,bitmap,dv->fat1,i);
    i=0;
    for (y=0;y<GR_HEIGHT-3*FONT_HEIGHT;y++) {
      if (ScanKey)
	break;
      for (x=0;x<GR_WIDTH-FONT_WIDTH<<1;x++) {
	lohi=i*s;
	c1=lohi/l;
	if (bitmap[c1])
	  dc->color=ROP_XOR+BLUE^TRANSPARENT;
	else
	  dc->color=ROP_XOR+WHITE^TRANSPARENT;
	GrPlot(dc,x,y);
	i++;
      }
    }
    Free(bitmap);
  } catch
    DrvUnlock(dv);
  GetChar;

  SettingsPop;
  DCFill;
  DCDel(dc);
}
public U0 DrvView(U8 drv_let=0)
{//Drive view. Graph the allocation map's fragmentation.
  CDrv *dv=Let2Drv(drv_let),*old_dv=Fs->cur_dv;
  Drv(drv_let);
  switch (dv->type) {
    case DVT_REDSEA:
      RedSeaDrvView(drv_let);
      break;
    case DVT_FAT32:
      FAT32DrvView(drv_let);
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
  Drv(Drv2Let(old_dv));
}

public U0 DskView(U8 drv_let=0)
{//Disk view. Pie chart of partition sizes.
  I64 i,j,
	h=Fs->pix_width,
	v=Fs->pix_height,
	radius;
  CDrv *dv;
  CBlkDev *bd=Let2BlkDev(drv_let);
  CDC *dc=DCAlias;
  F64 sect_start,sect_end;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  DocCursor;
  DocClear;
  DCFill;
  if (h<v)
    radius=0.4*h;
  else
    radius=0.4*v;
  dc->color=BLACK;
  GrCircle(dc,h>>1,v>>1,radius);

  j=1;
  for (i=0;i<NUM_DRVS;i++) {
    dv=&blkdev.drvs[i];
    if (bd==dv->bd && dv->type) {
      sect_start=-(dv->base_offset*2*ã/(bd->max_blk+1));
      sect_end	=-((dv->base_offset+dv->size)*2*ã/(bd->max_blk+1));
      dc->color=BLACK;
      GrLine(dc,h>>1,v>>1,
	    h>>1+radius*Cos(sect_start),
	    v>>1+radius*Sin(sect_start));
      GrLine(dc,h>>1,v>>1,
	    h>>1+radius*Cos(sect_end),
	    v>>1+radius*Sin(sect_end));

      dc->color=j;
      GrPrint(dc,0,v-FONT_HEIGHT*j,"%C %-8Z",Drv2Let(dv),
	    dv->type.u16[0],"ST_DRV_TYPES");
      GrFloodFill(dc,
	    h>>1+(radius-4)*Cos((sect_start+sect_end)/2),
	    v>>1+(radius-4)*Sin((sect_start+sect_end)/2),FALSE);
      j++;
    }
  }

  GetChar(,FALSE);
  SettingsPop;
  DCFill;
  DCDel(dc);
}

#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"

public I64 Zip(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{//Compress files by moving to .Z filename.
  U8 *st;
  CDirEntry *tempde,*tempde1;
  I64 result=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (!IsDotZ(tempde->full_name)) {
      st=MStrPrint("%s.Z",tempde->full_name);
      result+=Move(tempde->full_name,st);
      Free(st);
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return result;
}

public I64 Unzip(U8 *files_find_mask="*.Z",U8 *fu_flags=NULL)
{//Uncompress files by moving to not .Z filename.
//You don't have to do this for normal operation.
  //It automatically unzips ".Z" files.
  U8 *st;
  CDirEntry *tempde,*tempde1;
  I64 result=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (IsDotZ(tempde->full_name)) {
      st=StrNew(tempde->full_name);
      StrLastRem(st,".");
      result+=Move(tempde->full_name,st);
      Free(st);
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return result;
}
