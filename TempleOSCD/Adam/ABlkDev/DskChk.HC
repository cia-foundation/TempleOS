#help_index "Info;File/Cmd Line (Typically);Cmd Line (Typically)"
Bool ChkDskConfirm(Bool *_fix,Bool *_confirm)
{
  if (*_fix && *_confirm) {
    "Fix ";
    if (!YorN)
      *_fix=FALSE;
    *_confirm=FALSE;
  }
  return *_fix;
}

I64 RedSeaChkDskLst(CDrv *dv,CDirEntry *tmpde1,
	U8 *bits,U8 *bits2,I64 size,I64 bpc)
{
  CDirEntry *tmpde2;
  I64 i,j,errs=0;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (tmpde1->attr & RS_ATTR_DIR && tmpde1->sub)
      errs+=RedSeaChkDskLst(dv,tmpde1->sub,bits,bits2,size,bpc);
    j=(tmpde1->size+bpc-1)/bpc;
    for (i=0;i<j;i++) {
      if (i+tmpde1->clus-dv->data_area>size) {
	PrintErr("Invalid Clus:%s Clus:%X\n",tmpde1->full_name,
	      i+tmpde1->clus);
	errs++;
	break;
      }
      if (LBts(bits,i+tmpde1->clus-dv->data_area)) {
	PrintErr("Dbl Alloc:%s Clus:%X\n",tmpde1->full_name,
	      i+tmpde1->clus);
	errs++;
      }
      if (!LBtr(bits2,i+tmpde1->clus-dv->data_area)) {
	PrintErr("UnAlloc:%s Clus:%X\n",tmpde1->full_name,
	      i+tmpde1->clus);
	errs++;
      }
    }
    DirEntryDel(tmpde1);
    tmpde1=tmpde2;
  }
  return errs;
}

I64 RedSeaChkDsk(U8 drv_let,Bool *_fix,Bool *_confirm)
{
  I64 i,j,bpc,size,errs=0;
  CDrv *dv=Let2Drv(drv_let),*old_dv=Fs->cur_dv;
  U8 *files_find_mask=MStrPrint("%c:/*",Drv2Let(dv)),
	*old_dir=StrNew(Fs->cur_dir),
	*bits,*bits2;
  CDirEntry *ptr,*ptr2;

  Drv(drv_let);
  "Scanning...\n";
  size=(dv->size-(dv->data_area-dv->drv_offset))/dv->spc;
  bpc=dv->spc<<BLK_SIZE_BITS;
  bits=CAlloc((size+7)>>3);
  bits2=CAlloc((size+7)>>3+BLK_SIZE);
  BlkRead(dv,bits2,dv->fat1,((size+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS);

  //Get Root Dir size
  ptr2=MAlloc(bpc);
  BlkRead(dv,ptr2,dv->root_clus,1);
  ptr=ptr2(U8 *)-offset(CDirEntry.start);
  j=(ptr->size+bpc-1)/bpc;
  Free(ptr2);

  for (i=0;i<j;i++) {
    if (i+dv->root_clus-dv->data_area>size) {
      PrintErr("Invalid Clus: RootDir Clus:%X\n",i+dv->root_clus);
      errs++;
      break;
    }
    if (LBts(bits,i+dv->root_clus-dv->data_area)) {
      PrintErr("Dbl Alloc: RootDir Clus:%X\n",i+dv->root_clus);
      errs++;
    }
    if (!LBtr(bits2,i+dv->root_clus-dv->data_area)) {
      PrintErr("UnAlloc: RootDir Clus:%X\n",i+dv->root_clus);
      errs++;
    }
  }

  errs+=RedSeaChkDskLst(dv,FilesFind(files_find_mask,FUF_RECURSE),
	bits,bits2,size,bpc);
  for (i=1;i<size;i++)
    if (Bt(bits2,i)) {
      PrintWarn("Shouldn't Alloc Clus:%0X\n",i+dv->data_area);
      errs++;
      if (ChkDskConfirm(_fix,_confirm))
	RedSeaFreeClus(dv,i+dv->data_area,1);
    }

  Free(files_find_mask);
  Free(bits);
  Free(bits2);
  Drv(Drv2Let(old_dv));
  Cd(old_dir);
  Free(old_dir);
  return errs;
}

I64 FAT32ChkDskLst(CDrv *dv,CDirEntry *tmpde1,
	U8 *bits,U32 *bits2,I64 size,I64 bpc)
{
  CDirEntry *tmpde2;
  I64 i,c,errs=0;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (tmpde1->attr & RS_ATTR_DIR && tmpde1->sub)
      errs+=FAT32ChkDskLst(dv,tmpde1->sub,bits,bits2,size,bpc);
    i=0;
    c=tmpde1->clus;
    while (0<c<0x0FFFFFF8) {
      if (c>size) {
	PrintErr("Invalid Clus:%s Clus:%X\n",tmpde1->full_name,c);
	errs++;
	break;
      }
      if (LBts(bits,c)) {
	PrintErr("Dbl Alloc:%s Clus:%X\n",tmpde1->full_name,c);
	errs++;
      }
      if (!bits2[c]) {
	PrintErr("UnAlloc:%s Clus:%X\n",tmpde1->full_name,c);
	errs++;
      } else
	bits2[c]=0;
      c=ClusNumNext(dv,c);
      i++;
    }
    if (!(tmpde1->attr & RS_ATTR_DIR)) {
      i*=bpc;
      if (tmpde1->size>i) {
	PrintErr("Alloced File Too Short:%s\n",tmpde1->full_name);
	errs++;
      }
      if (i>tmpde1->size+bpc-1) {
	PrintWarn("Alloced File Too Long:%s\n",tmpde1->full_name);
	errs++;
      }
    }
    DirEntryDel(tmpde1);
    tmpde1=tmpde2;
  }
  return errs;
}

I64 FAT32ChkDsk(U8 drv_let,Bool *_fix,Bool *_confirm)
{
  I64 i,bpc,size,c,errs=0;
  CDrv *dv=Let2Drv(drv_let),*old_dv=Fs->cur_dv;
  U8 *files_find_mask=MStrPrint("%c:/*",Drv2Let(dv)),
	*old_dir=StrNew(Fs->cur_dir),
	*bits;
  U32 *bits2;
  Drv(drv_let);
  "Scanning...\n";
  size=(dv->size-(dv->data_area-dv->drv_offset))/dv->spc;
  bpc=dv->spc<<BLK_SIZE_BITS;
  bits=CAlloc((size+7)>>3);
  bits2=CAlloc(size*4+BLK_SIZE);
  BlkRead(dv,bits2,dv->fat1,(size*4+BLK_SIZE-1)>>BLK_SIZE_BITS);

  c=dv->root_clus;
  while (0<c<0x0FFFFFF8) {
    if (c>size) {
      PrintErr("Invalid Clus: RootDir Clus:%X\n",c);
      errs++;
      break;
    }
    if (LBts(bits,c)) {
      PrintErr("Dbl Alloc: RootDir Clus:%X\n",c);
      errs++;
    }
    if (!bits2[c]) {
      PrintErr("UnAlloc: RootDir Clus:%X\n",c);
      errs++;
    } else
      bits2[c]=0;
    c=ClusNumNext(dv,c);
  }

  errs+=FAT32ChkDskLst(dv,FilesFind(files_find_mask,FUF_RECURSE),
	bits,bits2,size,bpc);

  bits2[1]=0; //See $LK,"FAT32Fmt",A="MN:FAT32Fmt"$()
  for (i=1;i<size;i++)
    if (bits2[i]) {
      PrintWarn("Shouldn't Alloc Clus:%0X\n",i);
      errs++;
      if (ChkDskConfirm(_fix,_confirm))
	FAT32FreeClus(dv,i);
    }
  Free(files_find_mask);
  Free(bits);
  Free(bits2);
  Drv(Drv2Let(old_dv));
  Cd(old_dir);
  Free(old_dir);
  return errs;
}

public I64 DskChk(U8 drv_let=0,Bool fix=FALSE,Bool confirm=TRUE)
{//Check disk for allocation errors and, optionally, fix.
//You probably want to reformat and reinstall.
  I64 errs=0;
  CDrv *dv=Let2Drv(drv_let);
  switch (dv->fs_type) {
    case FSt_REDSEA:
      errs=RedSeaChkDsk(drv_let,&fix,&confirm);
      break;
    case FSt_FAT32:
      errs=FAT32ChkDsk(drv_let,&fix,&confirm);
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
  if (errs) {
    if (fix)
      "It might be a little better.  ";
    "Copy files to another partition or CD/DVD, "
	  "reformat, and copy back.  "
	  "Or, copy from a back-up.\n";
  }
  return errs;
}

U0 RedSeaDrvView(U8 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  I64 lohi,c1,i,x,y,l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
	s=dv->size+dv->drv_offset-dv->data_area;
  U8 *bitmap;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  WinBorder(ON);
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=((s+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    BlkRead(dv,bitmap,dv->fat1,i);
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
  I64 lohi,c1,i,x,y,l=(GR_HEIGHT-3*FONT_HEIGHT)*(GR_WIDTH-FONT_WIDTH<<1),
	s=(dv->size+dv->spc-1)/dv->spc-(2+dv->data_area-dv->drv_offset);
  U32 *bitmap;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  WinBorder(ON);
  DocCursor;
  DocClear;
  DCFill;
  try {
    i=(s*4+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    BlkRead(dv,bitmap,dv->fat1,i);
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
  switch (dv->fs_type) {
    case FSt_REDSEA:
      RedSeaDrvView(drv_let);
      break;
    case FSt_FAT32:
      FAT32DrvView(drv_let);
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
  Drv(Drv2Let(old_dv));
}

public U0 DskView(U8 drv_let=0)
{//Disk view. Pie chart of partition sizes.
  I64 i,j,attr,
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
  for (i=0;i<DRVS_NUM;i++) {
    dv=&blkdev.drvs[i];
    if (bd==dv->bd && dv->fs_type) {
      sect_start=-(dv->drv_offset*2*ã/(bd->max_blk+1));
      sect_end	=-((dv->drv_offset+dv->size)*2*ã/(bd->max_blk+1));
      dc->color=BLACK;
      GrLine(dc,h>>1,v>>1,
	    h>>1+radius*Cos(sect_start),
	    v>>1+radius*Sin(sect_start));
      GrLine(dc,h>>1,v>>1,
	    h>>1+radius*Cos(sect_end),
	    v>>1+radius*Sin(sect_end));

      attr=DrvTextAttrGet(Drv2Let(dv));
      dc->color=attr&15;
      GrPrint(dc,0,v-FONT_HEIGHT*j,"%C %-8Z",Drv2Let(dv),
	    dv->fs_type,"ST_DRV_TYPES");
      dc->color.c1=attr>>4;
      dc->color|=ROPF_DITHER;
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

I64 RedSeaUnusedDrvSpace(U8 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  I64 res=0,i,l;
  U8 *bitmap;
  try {
    l=dv->size+dv->drv_offset-dv->data_area;
    i=((l+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    BlkRead(dv,bitmap,dv->fat1,i);
    for (i=0;i<l;i++)
      if (!Bt(bitmap,i))
	res++;
    Free(bitmap);
  } catch
    DrvUnlock(dv);
  return res*BLK_SIZE*dv->spc;
}
I64 FAT32UnusedDrvSpace(U8 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  I64 res=0,i,l;
  U32 *bitmap;
  try {
    l=(dv->size+dv->spc-1)/dv->spc-(2+dv->data_area-dv->drv_offset);
    i=(l*4+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    BlkRead(dv,bitmap,dv->fat1,i);
    for (i=0;i<l;i++)
      if (!bitmap[i])
	res++;
    Free(bitmap);
  } catch
    DrvUnlock(dv);
  return res*BLK_SIZE*dv->spc;
}
public I64 DrvUnused(U8 drv_let=0)
{//Returns unused size in bytes.
  CDrv *dv=Let2Drv(drv_let),*old_dv=Fs->cur_dv;
  U8 *old_dir=StrNew(Fs->cur_dir);
  I64 res=0;
  Drv(drv_let);
  switch (dv->fs_type) {
    case FSt_REDSEA:
      res=RedSeaUnusedDrvSpace(drv_let);
      break;
    case FSt_FAT32:
      res=FAT32UnusedDrvSpace(drv_let);
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
  Drv(Drv2Let(old_dv));
  Cd(old_dir);
  Free(old_dir);
  return res;
}
