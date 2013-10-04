#help_index "File/Program Routines/Utils"

I64 DirTreeFlattenSize(CDirEntry *tempde)
{
  I64 result=0;
  while (tempde) {
    result+=CDIR_SIZE+1;
    if (tempde->attr & _ATTR_DIR)
      result+=DirTreeFlattenSize(tempde->sub);
    tempde=tempde->next;
  }
  return result+1;
}

I64 DirTreeFlattenFill(CDirEntry *tempde,U8 *dst)
{
  I64 result=0,i;
  while (tempde) {
    *dst++=1;
    result++;
    i=CDIR_SIZE;
    MemCpy(dst,&tempde->start,i);
    dst+=i;
    result+=i;
    if (tempde->attr & _ATTR_DIR) {
      i=DirTreeFlattenFill(tempde->sub,dst);
      dst+=i;
      result+=i;
    }
    tempde=tempde->next;
  }
  *dst=0;
  return result+1;
}

public U8 *DirTreeFlatten(CDirEntry *tempde,I64 *_size=NULL)
{
  I64 size=DirTreeFlattenSize(tempde);
  U8 *buf=MAlloc(size);
  DirTreeFlattenFill(tempde,buf);
  if (_size) *_size=size;
  return buf;
}

U8 *DirTreeDoUnflatten(U8 *src,CDirEntry **tempde)
{
  I64 i;
  CDirEntry *tempde1;
  if (*src++) {
    tempde1=CAlloc(sizeof(CDirEntry));
    *tempde=tempde1;
    i=CDIR_SIZE;
    MemCpy(&tempde1->start,src,i);
    src+=i;
    if (tempde1->attr & _ATTR_DIR)
      src=DirTreeDoUnflatten(src,&tempde1->sub);
    src=DirTreeDoUnflatten(src,&tempde1->next);
  } else
    *tempde=NULL;
  return src;
}

public CDirEntry *DirTreeUnflatten(U8 *src)
{
  CDirEntry *tempde=NULL;
  DirTreeDoUnflatten(src,&tempde);
  return tempde;
}

#help_index "Install;File/Cmd Line (Typically);Cmd Line (Typically)"

#define ROUND_PRT_TO	(63*255)
#define PRT_HEADER	63

class CPlannedPrt
{
  CPlannedPrt *next,*last;
  I64 size;
  Bool pri;
};

//This is available to assist in writing scripts.
I64 prt_dsk_dsk_size;

public U0 PrtDsk(U8 drv_let=0)
//Partition Disk containing partition drv_let.
//or drv_let=0 means add new drv that is not already mounted.
{
  CBlkDev *bdev;
  CPlannedPrt root,*temppp;
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
      Drv2BlkDev(drv_let);
    catch {
      drv_let=0;
      Fs->catch_except=TRUE;
    }

  if (!drv_let) {
    Mount(TRUE);
    return;
  }

  bdev=Drv2BlkDev(drv_let);
  if (!bdev || bdev->type!=BDT_ATA)
    return;

  prt_dsk_dsk_size=bdev->max_blk+1;

  QueInit(&root);

  drv_let2=bdev->first_drv_let;
  remaining=FloorU64(bdev->max_blk+1,ROUND_PRT_TO);
  while (FloorU64(remaining,ROUND_PRT_TO)>=ROUND_PRT_TO) {
    temppp=MAlloc(sizeof(CPlannedPrt));
    do {
      "$$FG,RED$$Partition %C$$FG$$\n",drv_let2;
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
	remaining-PRT_HEADER,remaining-PRT_HEADER;
      temppp->size=CeilU64(PmtI64("Size in Blocks  :",
	remaining-PRT_HEADER)+PRT_HEADER,ROUND_PRT_TO);
    } while (!(ROUND_PRT_TO<=temppp->size<=FloorU64(remaining,ROUND_PRT_TO)));
    QueIns(temppp,root.last);
    remaining-=temppp->size;
    drv_let2++;
  }

  "\n\n!!! Repartition Drive !!!\n\n";
  temppp=root.next;
  drv_let2=bdev->first_drv_let;
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

  remaining=FloorU64(bdev->max_blk+1,ROUND_PRT_TO)-ROUND_PRT_TO;
  temppp=root.next;
  MemSet(&mbr,0,BLK_SIZE);
  mbr.signature=0xAA55;
  offset=0;
  for (i=0;i<pri_cnt;i++) {
    mbr.p[i].active=0x80;
    mbr.p[i].start_head=0;
    mbr.p[i].start_cyl=0x101;
    mbr.p[i].type=1; //Will get set to MBR_PT_REDSEA
    mbr.p[i].end_head=0xFE;
    mbr.p[i].end_cyl=0xFFFF;
    mbr.p[i].offset=PRT_HEADER+offset;
    mbr.p[i].size=temppp->size-PRT_HEADER;
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
  ATAWriteBlks(bdev,&mbr,0,1);

  while (temppp!=&root) {
    start_offset=offset;
    MemSet(&mbr,0,BLK_SIZE);
    mbr.signature=0xAA55;

    mbr.p[0].active=0x80;
    mbr.p[0].start_head=1;
    mbr.p[0].start_cyl=0x101;
    mbr.p[0].type=1; //Will get set to MBR_PT_REDSEA
    mbr.p[0].end_head=0xFE;
    mbr.p[0].end_cyl=0xFFFF;
    mbr.p[0].offset=PRT_HEADER;
    mbr.p[0].size=temppp->size-PRT_HEADER;
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
    ATAWriteBlks(bdev,&mbr,start_offset,1);
  }

  bdev->flags&=~(BDF_INITIALIZED | BDF_INIT_IN_PROGRESS);
  BlkDevAdd(bdev,FALSE,TRUE);
  for (i=bdev->first_drv_let;i<drv_let2;i++)
    Fmt(i,TRUE,FALSE);

pd_done:
  while (root.next!=&root) {
    temppp=root.next;
    QueRem(temppp);
    Free(temppp);
  }
}


#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically)"

U0 RedSeaFSDrvView(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  I64 lohi,c1,i,x,y,l,s;
  U8 *bitmap;
  CDC *dc=DCAlias;

  l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
  s=(p->size+p->spc-1)/p->spc;
 

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinMax;
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=((s+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(p,bitmap,p->map1,i);
    i=0;
    for (y=0;y<GR_HEIGHT-3*FONT_HEIGHT;y++) {
      if (ScanKey)
	break;
      for (x=0;x<GR_WIDTH-FONT_WIDTH<<1;x++) {
	lohi=i*s;
	c1=lohi/l;
	if (Bt(bitmap,c1))
	  dc->color=ROP_XOR+BLUE^COLOR_TRANSPARENT;
	else
	  dc->color=ROP_XOR+WHITE^COLOR_TRANSPARENT;
	GrPlot(dc,x,y);
	i++;
      }
    }
    Free(bitmap);
  } catch
    PrtUnlock(p);
  GetChar;
 
  SettingsPop;
  DCFill;
  DCDel(dc);
}

U0 FAT32DrvView(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  I64 lohi,c1,i,x,y,l,s;
  U32 *bitmap;
  CDC *dc=DCAlias;
  l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
  s=(p->size+p->spc-1)/p->spc;
 

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinMax;
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=(s*4+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(p,bitmap,p->map1,i);
    i=0;
    for (y=0;y<GR_HEIGHT-3*FONT_HEIGHT;y++) {
      if (ScanKey)
	break;
      for (x=0;x<GR_WIDTH-FONT_WIDTH<<1;x++) {
	lohi=i*s;
	c1=lohi/l;
	if (bitmap[c1])
	  dc->color=ROP_XOR+BLUE^COLOR_TRANSPARENT;
	else
	  dc->color=ROP_XOR+WHITE^COLOR_TRANSPARENT;
	GrPlot(dc,x,y);
	i++;
      }
    }
    Free(bitmap);
  } catch
    PrtUnlock(p);
  GetChar;
 
  SettingsPop;
  DCFill;
  DCDel(dc);
}

public U0 DrvView(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let),*p_old=Fs->cur_prt;
  Drv(drv_let);
  switch (p->type) {
    case PT_REDSEA:
      RedSeaFSDrvView(drv_let);
      break;
    case PT_FAT32:
      FAT32DrvView(drv_let);
      break;
    default:
      PutDefineErr("ST_UNSUPPORTED_FILE_SYSTEM");
  }
  Drv(Prt2Drv(p_old));
}

public U0 DskView(U8 drv_let=0)
{
  I64 i,j,
    h=Fs->win_pixel_width,
    v=Fs->win_pixel_height,
    radius;
  CPrt *p;
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  CDC *dc=DCAlias;
  F64 sect_start,sect_end;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
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
  for (i=0;i<NUM_PRTS;i++) {
    p=&sys_prts[i];
    if (bdev==p->bdev && p->type) {
      sect_start=-(p->offset*2*ã/(bdev->max_blk+1));
      sect_end	=-((p->offset+p->size)*2*ã/(bdev->max_blk+1));
      dc->color=BLACK;
      GrLine(dc,h>>1,v>>1,
	h>>1+radius*Cos(sect_start),
	v>>1+radius*Sin(sect_start));
      GrLine(dc,h>>1,v>>1,
	h>>1+radius*Cos(sect_end),
	v>>1+radius*Sin(sect_end));

      dc->color=j;
      GrPrint(dc,0,v-FONT_HEIGHT*j,"%C %-8Z",Prt2Drv(p),
	p->type.u16[0],"ST_PRT_TYPES");
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

#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically);Install"
public U0 BlkDevRep()
{
  I64 i;
  for (i=0;i<NUM_BLKDEVS;i++)
    if (sys_blkdevs[i].bd_signature==BD_SIGNATURE_VAL)
      ClassRep(&sys_blkdevs[i]);
}
#help_index ""
