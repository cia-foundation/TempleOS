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

I64 RedSeaChkDskLst(CDrv *dv,CDirEntry *tempde1,
	U8 *bits,U8 *bits2,I64 size,I64 bpc)
{
  CDirEntry *tempde2;
  I64 i,j,errs=0;
  while (tempde1) {
    tempde2=tempde1->next;
    if (tempde1->attr & RS_ATTR_DIR && tempde1->sub)
      errs+=RedSeaChkDskLst(dv,tempde1->sub,bits,bits2,size,bpc);
    j=(tempde1->size+bpc-1)/bpc;
    for (i=0;i<j;i++) {
      if (i+tempde1->cluster-dv->data_area>size) {
	PrintErr("Invalid Cluster:%s Cluster:%X\n",tempde1->full_name,
	      i+tempde1->cluster);
	errs++;
	break;
      }
      if (LBts(bits,i+tempde1->cluster-dv->data_area)) {
	PrintErr("Dbl Alloc:%s Cluster:%X\n",tempde1->full_name,
	      i+tempde1->cluster);
	errs++;
      }
      if (!LBtr(bits2,i+tempde1->cluster-dv->data_area)) {
	PrintErr("UnAlloc:%s Cluster:%X\n",tempde1->full_name,
	      i+tempde1->cluster);
	errs++;
      }
    }
    DirEntryDel(tempde1);
    tempde1=tempde2;
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
  size=(dv->size-(dv->data_area-dv->base_offset))/dv->spc;
  bpc=dv->spc<<BLK_SIZE_BITS;
  bits=CAlloc((size+7)>>3);
  bits2=CAlloc((size+7)>>3+BLK_SIZE);
  RBlks(dv,bits2,dv->fat1,((size+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS);

  //Get Root Dir size
  ptr2=MAlloc(bpc);
  RBlks(dv,ptr2,dv->root_cluster,1);
  ptr=ptr2(U8 *)-offset(CDirEntry.start);
  j=(ptr->size+bpc-1)/bpc;
  Free(ptr2);

  for (i=0;i<j;i++) {
    if (i+dv->root_cluster-dv->data_area>size) {
      PrintErr("Invalid Cluster: RootDir Cluster:%X\n",i+dv->root_cluster);
      errs++;
      break;
    }
    if (LBts(bits,i+dv->root_cluster-dv->data_area)) {
      PrintErr("Dbl Alloc: RootDir Cluster:%X\n",i+dv->root_cluster);
      errs++;
    }
    if (!LBtr(bits2,i+dv->root_cluster-dv->data_area)) {
      PrintErr("UnAlloc: RootDir Cluster:%X\n",i+dv->root_cluster);
      errs++;
    }
  }

  errs+=RedSeaChkDskLst(dv,FilesFind(files_find_mask,FUF_RECURSE),
	bits,bits2,size,bpc);
  for (i=1;i<size;i++)
    if (Bt(bits2,i)) {
      PrintWarn("Shouldn't Alloc Cluster:%0X\n",i+dv->data_area);
      errs++;
      if (ChkDskConfirm(_fix,_confirm))
	RedSeaFreeClusters(dv,i+dv->data_area,1);
    }

  Free(files_find_mask);
  Free(bits);
  Free(bits2);
  Drv(Drv2Let(old_dv));
  Cd(old_dir);
  Free(old_dir);
  return errs;
}

I64 FAT32ChkDskLst(CDrv *dv,CDirEntry *tempde1,
	U8 *bits,U32 *bits2,I64 size,I64 bpc)
{
  CDirEntry *tempde2;
  I64 i,c,errs=0;
  while (tempde1) {
    tempde2=tempde1->next;
    if (tempde1->attr & RS_ATTR_DIR && tempde1->sub)
      errs+=FAT32ChkDskLst(dv,tempde1->sub,bits,bits2,size,bpc);
    i=0;
    c=tempde1->cluster;
    while (0<c<0x0FFFFFF8) {
      if (c>size) {
	PrintErr("Invalid Cluster:%s Cluster:%X\n",tempde1->full_name,c);
	errs++;
	break;
      }
      if (LBts(bits,c)) {
	PrintErr("Dbl Alloc:%s Cluster:%X\n",tempde1->full_name,c);
	errs++;
      }
      if (!bits2[c]) {
	PrintErr("UnAlloc:%s Cluster:%X\n",tempde1->full_name,c);
	errs++;
      } else
	bits2[c]=0;
      c=ClusterNumNext(dv,c);
      i++;
    }
    if (!(tempde1->attr & RS_ATTR_DIR)) {
      i*=bpc;
      if (tempde1->size>i) {
	PrintErr("Alloced File Too Short:%s\n",tempde1->full_name);
	errs++;
      }
      if (i>tempde1->size+bpc-1) {
	PrintWarn("Alloced File Too Long:%s\n",tempde1->full_name);
	errs++;
      }
    }
    DirEntryDel(tempde1);
    tempde1=tempde2;
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
  size=(dv->size-(dv->data_area-dv->base_offset))/dv->spc;
  bpc=dv->spc<<BLK_SIZE_BITS;
  bits=CAlloc((size+7)>>3);
  bits2=CAlloc(size*4+BLK_SIZE);
  RBlks(dv,bits2,dv->fat1,(size*4+BLK_SIZE-1)>>BLK_SIZE_BITS);

  c=dv->root_cluster;
  while (0<c<0x0FFFFFF8) {
    if (c>size) {
      PrintErr("Invalid Cluster: RootDir Cluster:%X\n",c);
      errs++;
      break;
    }
    if (LBts(bits,c)) {
      PrintErr("Dbl Alloc: RootDir Cluster:%X\n",c);
      errs++;
    }
    if (!bits2[c]) {
      PrintErr("UnAlloc: RootDir Cluster:%X\n",c);
      errs++;
    } else
      bits2[c]=0;
    c=ClusterNumNext(dv,c);
  }

  errs+=FAT32ChkDskLst(dv,FilesFind(files_find_mask,FUF_RECURSE),
	bits,bits2,size,bpc);

  bits2[1]=0; //See $LK,"FAT32Fmt",A="MN:FAT32Fmt"$()
  for (i=1;i<size;i++)
    if (bits2[i]) {
      PrintWarn("Shouldn't Alloc Cluster:%0X\n",i);
      errs++;
      if (ChkDskConfirm(_fix,_confirm))
	FAT32FreeClusters(dv,i);
    }
  Free(files_find_mask);
  Free(bits);
  Free(bits2);
  Drv(Drv2Let(old_dv));
  Cd(old_dir);
  Free(old_dir);
  return errs;
}

public I64 ChkDsk(U8 drv_let=0,Bool fix=FALSE,Bool confirm=TRUE)
{//Check disk for allocation errors and, optionally, fix.
//You probably want to reformat and reinstall.
  I64 errs=0;
  CDrv *dv=Let2Drv(drv_let);
  switch (dv->type) {
    case DVT_REDSEA:
      errs=RedSeaChkDsk(drv_let,&fix,&confirm);
      break;
    case DVT_FAT32:
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
