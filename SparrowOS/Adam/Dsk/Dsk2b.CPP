#help_index "File/Program Routines/Utils"

I64 CDirTreeFlattenSize(CDirEntry *tempm)
{
  I64 result=0;
  while (tempm) {
    result+=CDIR_SIZE+1;
    if (tempm->attr & _ATTR_DIR)
      result+=CDirTreeFlattenSize(tempm->sub);
    tempm=tempm->next;
  }
  return result+1;
}

I64 CDirTreeFlattenFill(CDirEntry *tempm,U8 *dst)
{
  I64 result=0,i;
  while (tempm) {
    *dst++=1;
    result++;
    i=CDIR_SIZE;
    MemCpy(dst,&tempm->start,i);
    dst+=i;
    result+=i;
    if (tempm->attr & _ATTR_DIR) {
      i=CDirTreeFlattenFill(tempm->sub,dst);
      dst+=i;
      result+=i;
    }
    tempm=tempm->next;
  }
  *dst=0;
  return result+1;
}

public U8 *CDirTreeFlatten(CDirEntry *tempm,I64 *_size=NULL)
{
  I64 size=CDirTreeFlattenSize(tempm);
  U8 *buf=MAlloc(size);
  CDirTreeFlattenFill(tempm,buf);
  if (_size) *_size=size;
  return buf;
}

U8 *CDirTreeDoUnflatten(U8 *src,CDirEntry **tempm)
{
  I64 i;
  CDirEntry *tempm1;
  if (*src++) {
    tempm1=CAlloc(sizeof(CDirEntry));
    *tempm=tempm1;
    i=CDIR_SIZE;
    MemCpy(&tempm1->start,src,i);
    src+=i;
    if (tempm1->attr & _ATTR_DIR)
      src=CDirTreeDoUnflatten(src,&tempm1->sub);
    src=CDirTreeDoUnflatten(src,&tempm1->next);
  } else
    *tempm=NULL;
  return src;
}

public CDirEntry *CDirTreeUnflatten(U8 *src)
{
  CDirEntry *tempm=NULL;
  CDirTreeDoUnflatten(src,&tempm);
  return tempm;
}

#help_index "Install;File/Install Related"
#help_file "::/SparrowOS/Doc/Install"

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
  I64 ext_base,num_prt,pri_cnt=0,
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

  num_prt=bdev->drv_let_base;
  remaining=FloorU64(bdev->max_blk+1,ROUND_PRT_TO);
  while (FloorU64(remaining,ROUND_PRT_TO)>=ROUND_PRT_TO) {
    temppp=MAlloc(sizeof(CPlannedPrt));
    do {
      "$$FG,RED$$Partition %C$$FG$$\n",num_prt;
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
    num_prt++;
  }

  "\n\n!!! Repartition Drive !!!\n\n";
  temppp=root.next;
  num_prt=bdev->drv_let_base;
  while (temppp!=&root) {
    "Drive %C:%08X %s\n",num_prt,temppp->size,
      temppp->pri ? "Primary":"Logical";
    temppp=temppp->next;
    num_prt++;
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
    mbr.p[i].type=1; //Will get set to MBR_PT_NATIVE
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
    mbr.p[0].type=1; //Will get set to MBR_PT_NATIVE
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
  BlkDevAdd(bdev,FALSE);
  for (i=bdev->drv_let_base;i<num_prt;i++)
    Fmt(i,TRUE,FALSE);

pd_done:
  while (root.next!=&root) {
    temppp=root.next;
    QueRem(temppp);
    Free(temppp);
  }
}


#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically)"

U0 NativeFSDrvView(U8 drv_let=0)
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
    case PT_NATIVE:
      NativeFSDrvView(drv_let);
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
  CBlkDev *bdev=Drv2BlkDevFwded(drv_let);
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
  dc->color=ROP_EQU+BLACK;
  GrCircle(dc,h>>1,v>>1,radius);

  j=1;
  for (i=0;i<NUM_PRTS;i++) {
    p=&sys_prts[i];
    if (bdev==p->bdev && p->type) {
      sect_start=-(p->offset*2*ã/(bdev->max_blk+1));
      sect_end	=-((p->offset+p->size)*2*ã/(bdev->max_blk+1));
      dc->color=ROP_EQU+BLACK;
      GrLine(dc,h>>1,v>>1,
	h>>1+radius*Cos(sect_start),
	v>>1+radius*Sin(sect_start));
      GrLine(dc,h>>1,v>>1,
	h>>1+radius*Cos(sect_end),
	v>>1+radius*Sin(sect_end));

      dc->color=ROP_EQU+j;
      GrPrintF(dc,0,v-FONT_HEIGHT*j,"%C %-8Z",Prt2Drv(p),
	p->type.u16[0],"ST_PRT_TYPES");
      GrFloodFill(dc,
	h>>1+(radius-4)*Cos((sect_start+sect_end)/2),
	v>>1+(radius-4)*Sin((sect_start+sect_end)/2),FALSE);
      j++;
    }
  }

  GetChar(NULL,FALSE);
  SettingsPop;
  DCFill;
  DCDel(dc);
}

#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically);File/Install Related;Install"
public U0 BlkDevRep()
{
  I64 i;
  for (i=0;i<num_blkdevs;i++)
    ClassRep("CBlkDev",&sys_blkdevs[i]);
}

#help_index "Install;File/Install Related"
#define SC_BLK_CNT	2048
public I64 NonNativeScan(I64 drv_let,U32 signature)
{
// See $LK,"::/SparrowOS/Doc/TSZ.TXT.Z","FI:::/SparrowOS/Doc/TSZ.TXT.Z"$
  U8 *buf=CAlloc(BLK_SIZE*SC_BLK_CNT),*b;
  U32 *d;
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  CPrt *p=Drv2Prt(drv_let);
  I64 *qq,blk=p->offset,result=0,start=0,i,j,n,to_go,size,
      contiguous=0,new_drv_let;
  Bool good,proceed=FALSE;
  try {
    while (!result) {
      '-';
      n=p->size+p->offset-blk;
      if (n>SC_BLK_CNT)
	n=SC_BLK_CNT;
      if (!n)
	break;
      RBlks(p,buf,blk,1);
      d=buf;
      qq=d;
      if (d[127]==signature) {
	b=d;
	good=TRUE;
	for (j=16;j<BLK_SIZE-4;j++)
	  if (b[j]!=j.u8[0]) {
	    good=FALSE;
	    break;
	  }
	if (good) {
	  result=blk;
	  break;
	}
      }
      blk+=n;
    }
    if (result) {
      blk=result-(SC_BLK_CNT-1);
      if (blk<0) blk=0;
      result=0;
      n=p->size+p->offset-blk;
      if (n>SC_BLK_CNT)
	n=SC_BLK_CNT;
      RBlks(p,buf,blk,n);
      for (i=0;i<n;i++) {
	d=buf+BLK_SIZE*i;
	qq=d;
	if (d[127]==signature && !qq[0]) {
	  b=d;
	  good=TRUE;
	  for (j=16;j<BLK_SIZE-4;j++)
	    if (b[j]!=j.u8[0]) {
	      good=FALSE;
	      break;
	    }
	  if (good) {
	    result=blk+i;
	    size=qq[1];
	    break;
	  }
	}
      }
    }
    if (result) {
      start=result;
      "\nStart  :0x%X Size:0x%X\n",result,size;
      to_go=size;
      blk=result;
      while (to_go && result) {
	'+';
	n=to_go;
	if (n>SC_BLK_CNT)
	  n=SC_BLK_CNT;
	RBlks(p,buf,blk,n);
	for (i=0;i<n;i++) {
	  d=buf+BLK_SIZE*i;
	  qq=d;
	  if (d[127]==signature && qq[0]==blk-result+i &&
	  qq[1]==size) {
	    b=d;
	    for (j=16;j<BLK_SIZE-4;j++)
	      if (b[j]!=j.u8[0]) {
		result=0;
		break;
	      }
	    if (result)
	      contiguous++;
	    else
	      break;
	  } else {
	    result=0;
	    break;
	  }
	}
	to_go-=n;
	blk+=n;
      }
    }
    "Start: 0x%X\n",result;
    if (result) {
      "\nSuccess\n";
      proceed=TRUE;
    } else {
      "\nFailed, but found 0x%X contiguous.\n",contiguous;
      if (contiguous) {
	"Proceed Anyway ";
	proceed=YorN;
      }
    }
  } catch
    Fs->catch_except=TRUE;
 
  Cd("HOME");
  Free(buf);
 
  if (proceed) {
    "New Drive Letter:";
    new_drv_let=ToUpper(GetChar);
    NewLine;
    bdev=&sys_blkdevs[num_blkdevs++];
    bdev->drv_let_base=new_drv_let;
    bdev->blk_size=BLK_SIZE;
    bdev->min_blk=start;
    bdev->max_blk=start+contiguous-1;
    bdev->non_native_drv=drv_let;
    bdev->non_native_offset=start;
    bdev->type=BDT_NON_NATIVE_FILE;
    bdev->flags=BDF_FMT;
    if (!BlkDevAdd(bdev,FALSE))
      PutDefine("ST_FAILED");
    Drv(new_drv_let);
    "File Blk: 0x%X\n",start;
    result=start;
  } else
    result=0;
  return result;
}
#help_index ""
