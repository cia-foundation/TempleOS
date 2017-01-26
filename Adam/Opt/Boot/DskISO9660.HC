#help_index "File/CD DVD"

class CCDVDUserData //Create DVD
{
  I64 loc,path_entry_num,
	short_dir_blks,long_dir_blks;
};

I64 DVDFileCreate2(CFile *out_file,CDirEntry *tmpde,CISO1DirEntry *tmpi,
	CISO1DirEntry *tmpi2,I64 *_cur_blk,CDirEntry *parent,Bool write,
	U8 *stage2_filename,I64 *_stage2_blk)
{
  CCDVDUserData *tmpc;
  CDirEntry *tmpde1,*tmpde2;
  CFile *in_file;
  U8 *buf=MAlloc(DVD_BLK_SIZE),*ptr1,*ptr2;
  CISO1DirEntry *dir_blk_buf =CAlloc(DVD_BLK_SIZE*128),
	*de =dir_blk_buf, *de1,
	*dir_blk_buf2=CAlloc(DVD_BLK_SIZE*128),
	*de2=dir_blk_buf2,*de12;
  I64 i,n;

  tmpc=parent->user_data;

  de->len=sizeof(CISO1DirEntry)-1;
  de->ext_attr_len=0;
  FillU16Palindrome(&de->vol_seq_num,1);
  Date2ISO1(&de->date,tmpde->datetime);
  de->flags=ISO1_ATTR_DIR;
  de->name_len=1;
  de->name=0;
  de->len+=de->name_len;
  de(U8 *)+=de->len;

  de->len=sizeof(CISO1DirEntry)-1;
  de->ext_attr_len=0;
  FillU32Palindrome(&de->loc,tmpc->loc);
  FillU32Palindrome(&de->size,tmpc->short_dir_blks*DVD_BLK_SIZE);
  FillU16Palindrome(&de->vol_seq_num,1);
  Date2ISO1(&de->date,parent->datetime);
  de->flags=ISO1_ATTR_DIR;
  de->name_len=1;
  de->name=1;
  de->len+=de->name_len;
  de(U8 *)+=de->len;

  de2->len=sizeof(CISO1DirEntry)-1;
  de2->ext_attr_len=0;
  FillU16Palindrome(&de2->vol_seq_num,1);
  Date2ISO1(&de2->date,tmpde->datetime);
  de2->flags=ISO1_ATTR_DIR;
  de2->name_len=1;
  de2->name=0;
  de2->len+=de2->name_len;
  de2(U8 *)+=de2->len;

  de2->len=sizeof(CISO1DirEntry)-1;
  de2->ext_attr_len=0;
  FillU32Palindrome(&de2->loc,tmpc->loc+tmpc->short_dir_blks);
  FillU32Palindrome(&de2->size,tmpc->long_dir_blks*DVD_BLK_SIZE);
  FillU16Palindrome(&de2->vol_seq_num,1);
  Date2ISO1(&de2->date,parent->datetime);
  de2->flags=ISO1_ATTR_DIR;
  de2->name_len=1;
  de2->name=1;
  de2->len+=de2->name_len;
  de2(U8 *)+=de2->len;

  tmpde1=tmpde->sub;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (!write) tmpde1->user_data=CAlloc(sizeof(CCDVDUserData));
    de1=de;
    de12=de2;
    if (tmpde1->attr & RS_ATTR_DIR) {
      n=DVDFileCreate2(out_file,tmpde1,de,de2,_cur_blk,
	    tmpde,write,stage2_filename,_stage2_blk);
      de(U8 *)+=sizeof(CISO1DirEntry)-1+n;
      de2(U8 *)+=sizeof(CISO1DirEntry)-1+n<<1;
    } else {
      tmpc=tmpde1->user_data;
      de->len=sizeof(CISO1DirEntry)-1;
      de->ext_attr_len=0;
      FillU32Palindrome(&de->loc,*_cur_blk);
      tmpc->loc=*_cur_blk;
      if (write) {
	if (stage2_filename && !StrCmp(tmpde1->full_name,stage2_filename)) {
	  "$$RED$$!!! Boot Stage 2 !!!$$FG$$\n";
	  if (_stage2_blk) *_stage2_blk=*_cur_blk;
	}
	"%X:%s\n",*_cur_blk,tmpde1->full_name;
      }
      FillU32Palindrome(&de->size,tmpde1->size);
      FillU16Palindrome(&de->vol_seq_num,1);
      Date2ISO1(&de->date,tmpde1->datetime);
      de->flags=0;
      de->name_len=StrLen(tmpde1->name);
      StrCpy(&de->name,tmpde1->name);
      de->len=de->len+de->name_len;
      de(U8 *)+=de->len;

      de2->len=sizeof(CISO1DirEntry)-1;
      de2->ext_attr_len=0;
      FillU32Palindrome(&de2->loc,*_cur_blk);
      FillU32Palindrome(&de2->size,tmpde1->size);
      FillU16Palindrome(&de2->vol_seq_num,1);
      Date2ISO1(&de2->date,tmpde1->datetime);
      de2->flags=0;
      de2->name_len=StrLen(tmpde1->name)<<1;
      ptr1=&de2->name;
      ptr2=&tmpde1->name;
      for (i=0;i<de2->name_len;i=i+2) {
	ptr1++;
	*ptr1++=*ptr2++;
      }
      de2->len+=de2->name_len;
      de2(U8 *)+=de2->len;

      in_file=FOpen(tmpde1->full_name,"r");
      for (i=0;i<(FSize(in_file)+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;i++) {
	n=4;
	if ((i+1)<<2>(FSize(in_file)+BLK_SIZE-1)>>BLK_SIZE_BITS) {
	  n=(FSize(in_file)+BLK_SIZE-1)>>BLK_SIZE_BITS&3;
	  MemSet(buf,0,DVD_BLK_SIZE);
	}
	if (write) {
	  FBlkRead(in_file,buf,i<<2,n);
	  FBlkWrite(out_file,buf,*_cur_blk<<2,n);
	}
	*_cur_blk+=1;
      }
      FClose(in_file);
    }
    if ((de1(U8 *)-dir_blk_buf(U8 *))/DVD_BLK_SIZE!=
	  (de(U8 *) -dir_blk_buf(U8 *))/DVD_BLK_SIZE) {
      i=de1->len;
      MemCpy(buf,de1,i);
      MemSet(de1,0,i);
      de=dir_blk_buf(U8 *)+(de(U8 *)
      -dir_blk_buf(U8 *))/DVD_BLK_SIZE*DVD_BLK_SIZE;
      MemCpy(de,buf,i);
      de(U8 *)+=i;
    }
    if ((de12(U8 *)-dir_blk_buf2(U8 *))/DVD_BLK_SIZE!=
	  (de2(U8 *) -dir_blk_buf2(U8 *))/DVD_BLK_SIZE) {
      i=de12->len;
      MemCpy(buf,de12,i);
      MemSet(de12,0,i);
      de2(U8 *)=dir_blk_buf2(U8 *)+(de2(U8 *)
      -dir_blk_buf2(U8 *))/DVD_BLK_SIZE*DVD_BLK_SIZE;
      MemCpy(de2,buf,i);
      de2(U8 *)+=i;
    }
    tmpde1=tmpde2;
  }

  tmpc=tmpde->user_data;

  tmpi->len=sizeof(CISO1DirEntry)-1;
  tmpi->ext_attr_len=0;
  tmpi->flags=ISO1_ATTR_DIR;
  if (!tmpde->name[0]) {
    tmpi->name_len=1;
    tmpi->name=1;
  } else {
    tmpi->name_len=StrLen(tmpde->name);
    StrCpy(&tmpi->name,tmpde->name);
  }
  tmpi->len+=tmpi->name_len;

  n=de(U8 *)+1-dir_blk_buf(U8 *);
  n=(n+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;
  FillU32Palindrome(&tmpi->size,n*DVD_BLK_SIZE);
  FillU32Palindrome(&tmpi->loc,*_cur_blk);
  tmpc->short_dir_blks=n;
  tmpc->loc=*_cur_blk;
  FillU32Palindrome(&dir_blk_buf->size,n*DVD_BLK_SIZE);
  FillU32Palindrome(&dir_blk_buf->loc,*_cur_blk);
  FillU16Palindrome(&tmpi->vol_seq_num,1);
  Date2ISO1(&tmpi->date,tmpde->datetime);
  if (write)
    "%X:%s\n",*_cur_blk,tmpde->full_name;
  if (write)
    FBlkWrite(out_file,dir_blk_buf,*_cur_blk<<2,n<<2);
  *_cur_blk+=n;

  tmpi2->len=sizeof(CISO1DirEntry)-1;
  tmpi2->ext_attr_len=0;
  tmpi2->flags=ISO1_ATTR_DIR;
  if (!tmpde->name[0]) {
    tmpi2->name_len=1;
    tmpi->name=1;
  } else {
    tmpi2->name_len=StrLen(tmpde->name)<<1;
    ptr1=&tmpi2->name;
    ptr2=&tmpde->name;
    for (i=0;i<tmpi2->name_len;i=i+2) {
      ptr1++;
      *ptr1++=*ptr2++;
    }
  }
  tmpi2->len+=tmpi2->name_len;
  n=de2(U8 *)+1-dir_blk_buf2(U8 *);
  n=(n+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;
  FillU32Palindrome(&tmpi2->size,n*DVD_BLK_SIZE);
  FillU32Palindrome(&tmpi2->loc,*_cur_blk);
  tmpc->long_dir_blks=n;
  FillU32Palindrome(&dir_blk_buf2->size,n*DVD_BLK_SIZE);
  FillU32Palindrome(&dir_blk_buf2->loc,*_cur_blk);
  FillU16Palindrome(&tmpi2->vol_seq_num,1);
  Date2ISO1(&tmpi2->date,tmpde->datetime);
  if (write)
    "%X:%s\n",*_cur_blk,tmpde->full_name;
  if (write)
    FBlkWrite(out_file,dir_blk_buf2,*_cur_blk<<2,n<<2);
  *_cur_blk+=n;

  Free(dir_blk_buf);
  Free(dir_blk_buf2);
  Free(buf);
  return tmpi->name_len;
}

I64 DVDTableLen(CDirEntry *tmpde,I64 *size1,I64 *size2,I64 cur_depth)
{//Returns depth
  CDirEntry *tmpde1=tmpde->sub;
  I64 max_depth=cur_depth,i;
  while (tmpde1) {
    if (tmpde1->attr & RS_ATTR_DIR) {
      *size1+=sizeof(CISO1PathEntry)-2+ (StrLen(tmpde1->name)+1)&-0x2;
      *size2+=sizeof(CISO1PathEntry)-2+StrLen(tmpde1->name)<<1;
      i=DVDTableLen(tmpde1,size1,size2,cur_depth+1);
      if (i>max_depth) max_depth=i;
    }
    tmpde1=tmpde1->next;
  }
  return max_depth;
}

U0 DVDFillPathTable(CDirEntry *tmpde,
  CISO1PathEntry **_itabbuf,CISO1PathEntry **_itabbuf2,
  I64 parent_entry_num,Bool big_endian,I64 *first_free,
  I64 cur_level,I64 output_level)
{
  U8 *ptr1,*ptr2;
  I64 i;
  CISO1PathEntry *tabbuf=*_itabbuf,*tabbuf2=*_itabbuf2;
  CDirEntry *tmpde1=tmpde->sub,*tmpde2;
  CCDVDUserData *tmpc;

  if (cur_level==output_level) {
    while (tmpde1) {
      if (tmpde1->attr & RS_ATTR_DIR) {
	tmpc=tmpde1->user_data;
	tmpc->path_entry_num=*first_free;
	tabbuf->name_len=StrLen(tmpde1->name);
	if (big_endian) {
	  tabbuf->blk=EndianU32(tmpc->loc);
	  tabbuf->parent_entry_num=EndianU16(parent_entry_num);
	} else {
	  tabbuf->blk=tmpc->loc;
	  tabbuf->parent_entry_num=parent_entry_num;
	}
	StrCpy(&tabbuf->name,tmpde1->name);

	tabbuf(U8 *)+=sizeof(CISO1PathEntry)-2+
	      (StrLen(tmpde1->name)+1) & -0x2;

	tabbuf2->name_len=StrLen(tmpde1->name)<<1;
	if (big_endian) {
	  tabbuf2->blk=EndianU32(tmpc->loc+tmpc->short_dir_blks);
	  tabbuf2->parent_entry_num=EndianU16(parent_entry_num);
	} else {
	  tabbuf2->blk=tmpc->loc+tmpc->short_dir_blks;
	  tabbuf2->parent_entry_num=parent_entry_num;
	}
	ptr1=&tabbuf2->name;
	ptr2=&tmpde1->name;
	for (i=0;i<tabbuf2->name_len;i=i+2) {
	  ptr1++;
	  *ptr1++=*ptr2++;
	}
	tabbuf2(U8 *)+=sizeof(CISO1PathEntry)-2+
	      StrLen(tmpde1->name)<<1;
	*first_free+=1;
      }
      tmpde1=tmpde1->next;
    }
    *_itabbuf=tabbuf;
    *_itabbuf2=tabbuf2;
  }
  tmpde1=tmpde->sub;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (tmpde1->attr & RS_ATTR_DIR) {
      tmpc=tmpde1->user_data;
      DVDFillPathTable(tmpde1,_itabbuf,_itabbuf2,tmpc->path_entry_num,
	    big_endian,first_free,cur_level+1,output_level);
    }
    tmpde1=tmpde2;
  }
}

public I64 ISO9660ISO(U8 *_filename=NULL,U8 *src_files_find_mask,
	U8 *fu_flags=NULL,U8 *_stage2_filename=NULL)
{//See $LK,"::/Misc/DoDistro.HC"$
//Use "C:/Distro/*" if you want all files in the C:/Distro directory.
  //Default flags are "+r" recurse.
  CISO1PriDesc *iso_pri=CAlloc(DVD_BLK_SIZE),
	*iso_boot=CAlloc(DVD_BLK_SIZE),
	*iso_sup=CAlloc(DVD_BLK_SIZE),
	*iso_term=CAlloc(DVD_BLK_SIZE);
  CDirEntry *headdir=CAlloc(sizeof(CDirEntry));
  I64 i,j,stage2_blk=(20<<2+1<<2+DVD_BOOT_LOADER_SIZE/BLK_SIZE)>>2,
	stage2_size,cur_blk=0,tabsize,tabsize2,first_free,max_depth,fuf_flags=0;
  U32 *d;
  CElTorito *et=CAlloc(DVD_BLK_SIZE);
  U8 *filename,*stage2_filename,
	*stage1_buf=CAlloc(DVD_BOOT_LOADER_SIZE),
	*zero_buf=CAlloc(DVD_BLK_SIZE);
  CISO1PathEntry *tabbuf=NULL,*tabbuf2=NULL,*itabbuf,*itabbuf2;
  CFile *out_file=NULL;
  CISO1DirEntry *tmpi;
  CCDVDUserData *tmpc;

  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);

  if (!_filename)
    _filename=blkdev.dft_iso_filename;
  filename=ExtDft(_filename,"ISO");

  if (_stage2_filename)
    stage2_filename=FileNameAbs(_stage2_filename);
  else
    stage2_filename=NULL;

  headdir->attr=RS_ATTR_DIR;
  headdir->sub=FilesFind(src_files_find_mask,fuf_flags);
  headdir->datetime=Now;
  headdir->user_data=CAlloc(sizeof(CCDVDUserData));
  tmpc=headdir->user_data;
  tmpc->path_entry_num=1;

  cur_blk=20<<2>>2;
  if (stage2_filename) //preboot and bootloader
    cur_blk+=1+DVD_BOOT_LOADER_SIZE/DVD_BLK_SIZE;
  DVDFileCreate2(out_file,headdir,&iso_pri->root_dir_record,
	&iso_sup->root_dir_record,&cur_blk,headdir,FALSE,
	stage2_filename,&stage2_blk);
  tabsize=sizeof(CISO1PathEntry);
  tabsize2=sizeof(CISO1PathEntry);
  max_depth=DVDTableLen(headdir,&tabsize,&tabsize2,1);
  FillU32Palindrome(&iso_pri->path_table_size,tabsize);
  FillU32Palindrome(&iso_sup->path_table_size,tabsize2);
  tabsize=(tabsize+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;
  cur_blk+=tabsize<<1;
  tabsize2=(tabsize2+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;
  cur_blk+=tabsize2<<1;

  if (FileAttr(filename) & RS_ATTR_CONTIGUOUS)
    out_file=FOpen(filename,"wc",cur_blk<<2);
  else
    out_file=FOpen(filename,"w",cur_blk<<2);
  cur_blk=0;
  if (!out_file)
    goto cf_done;

  while (cur_blk<20<<2>>2)
    FBlkWrite(out_file,zero_buf,cur_blk++<<2,4);

  iso_pri->type=ISO1T_PRI_VOL_DESC;
  StrCpy(iso_pri->id,"CD001");
  iso_pri->version=1;
  FillU16Palindrome(&iso_pri->vol_set_size,1);
  FillU16Palindrome(&iso_pri->vol_seq_num,1);
  FillU16Palindrome(&iso_pri->log_block_size,DVD_BLK_SIZE);
  iso_pri->file_structure_version=1;

  iso_sup->type=ISO1T_SUPPLEMENTARY_DESC;
  StrCpy(iso_sup->id,"CD001");
  iso_sup->version=1;
  FillU16Palindrome(&iso_sup->vol_set_size,1);
  FillU16Palindrome(&iso_sup->vol_seq_num,1);
  FillU16Palindrome(&iso_sup->log_block_size,DVD_BLK_SIZE);
  iso_sup->file_structure_version=1;

  iso_boot->type=ISO1T_BOOT_RECORD;
  StrCpy(iso_boot->id,"CD001");
  iso_boot->version=1;
  StrCpy(iso_boot(U8 *)+7,"EL TORITO SPECIFICATION");

  cur_blk=20<<2>>2;

  if (stage2_filename) {
    d=iso_boot(U8 *)+0x47;
    *d=cur_blk;
    et->w[0]=1;
    StrCpy(&et->w[2],"TempleOS");
    et->w[15]=0xAA55;
    j=0;
    for (i=0;i<16;i++) //Checksum
      j+=et->w[i];
    et->w[14]=-j;
    et->bootable=0x88;
    et->media_type=0;//0=no emu 2=1.44meg 4=hard drive
    et->sect_cnt=4;  //5 seems like the limit, 4 is safer
    et->load_rba=cur_blk+1;
    "%X: Pre Boot Blk\n",cur_blk;
    FBlkWrite(out_file,et,cur_blk++<<2,4);
    "%X: Boot Stage 1\n",cur_blk;
    cur_blk+=DVD_BOOT_LOADER_SIZE/DVD_BLK_SIZE;
  }

  DVDFileCreate2(out_file,headdir,&iso_pri->root_dir_record,
	&iso_sup->root_dir_record,&cur_blk,headdir,TRUE,
	stage2_filename,&stage2_blk);

  tabbuf=CAlloc(tabsize*DVD_BLK_SIZE);
  iso_pri->type_l_path_table=cur_blk;
  tabbuf->name_len=2;  //Fill-in adam entry
  tmpi=&iso_pri->root_dir_record;
  tabbuf->blk=tmpi->loc.little;
  tabbuf->parent_entry_num=1;
  tabbuf2=CAlloc(tabsize2*DVD_BLK_SIZE);
  iso_sup->type_l_path_table=cur_blk+tabsize;
  tabbuf2->name_len=2;	//Fill-in adam entry
  tmpi=&iso_sup->root_dir_record;
  tabbuf2->blk=tmpi->loc.little;
  tabbuf2->parent_entry_num=1;
  itabbuf=tabbuf+1;
  itabbuf2=tabbuf2+1;
  first_free=2;
  for (i=1;i<=max_depth;i++)
    DVDFillPathTable(headdir,&itabbuf,&itabbuf2,1,FALSE,&first_free,1,i);
  "%X: Path Table 0\n",cur_blk;
  FBlkWrite(out_file,tabbuf,cur_blk<<2,tabsize<<2);
  cur_blk+=tabsize;
  "%X: Path Table 1\n",cur_blk;
  FBlkWrite(out_file,tabbuf2,cur_blk<<2,tabsize2<<2);
  cur_blk+=tabsize2;

  MemSet(tabbuf,0,tabsize*DVD_BLK_SIZE);
  iso_pri->type_m_path_table=EndianU32(cur_blk);
  tabbuf->name_len=2;  //Fill-in adam entry
  tmpi=&iso_pri->root_dir_record;
  tabbuf->blk=tmpi->loc.big;
  tabbuf->parent_entry_num=EndianU16(1);
  MemSet(tabbuf2,0,tabsize2*DVD_BLK_SIZE);
  iso_sup->type_m_path_table=EndianU32(cur_blk+tabsize);
  tabbuf2->name_len=2;	//Fill-in adam entry
  tmpi=&iso_sup->root_dir_record;
  tabbuf2->blk=tmpi->loc.big;
  tabbuf2->parent_entry_num=EndianU16(1);
  itabbuf=tabbuf+1;
  itabbuf2=tabbuf2+1;
  first_free=2;
  for (i=1;i<=max_depth;i++)
    DVDFillPathTable(headdir,&itabbuf,&itabbuf2,1,TRUE,&first_free,1,i);
  "%X: Path Table 2\n",cur_blk;
  FBlkWrite(out_file,tabbuf,cur_blk<<2,tabsize<<2);
  cur_blk+=tabsize;
  "%X: Path Table 3\n",cur_blk;
  FBlkWrite(out_file,tabbuf2,cur_blk<<2,tabsize2<<2);
  cur_blk+=tabsize2;

  DirTreeDel2(headdir);
  FillU32Palindrome(&iso_pri->vol_space_size,cur_blk);
  FillU32Palindrome(&iso_sup->vol_space_size,cur_blk);
  FBlkWrite(out_file,iso_pri,16<<2,4);

  iso_term->type=ISO1T_TERMINATOR;
  StrCpy(iso_term->id,"CD001");
  iso_term->version=1;
  if (stage2_filename) {
    FBlkWrite(out_file,iso_boot,17<<2,4);
    FBlkWrite(out_file,iso_sup,18<<2,4);
    FBlkWrite(out_file,iso_term,19<<2,4);
    stage2_size=(Size(stage2_filename,"+s")+DVD_BLK_SIZE-1)/DVD_BLK_SIZE;
    MemCpy(stage1_buf,BDVD_START,BDVD_END-BDVD_START);
    *(BDVD_BLK_CNT-BDVD_START+stage1_buf)(U16 *)=stage2_size;
    *(BDVD_BLK_LO -BDVD_START+stage1_buf)(U32 *)=stage2_blk;
    "$$RED$$!!! Boot Stage 2 !!! %X-%X$$FG$$\n",
	  stage2_blk,stage2_blk+stage2_size-1;
    FBlkWrite(out_file,stage1_buf,
	  20<<2+1<<2,DVD_BOOT_LOADER_SIZE/BLK_SIZE);
  } else {
    FBlkWrite(out_file,iso_sup,17<<2,4);
    FBlkWrite(out_file,iso_term,18<<2,4);
  }

  cf_done:
  FClose(out_file);
  Free(tabbuf);
  Free(tabbuf2);
  Free(stage2_filename);
  Free(filename);
  Free(zero_buf);
  Free(stage1_buf);
  Free(et);
  Free(iso_pri);
  Free(iso_boot);
  Free(iso_sup);
  Free(iso_term);
  return cur_blk;
}
