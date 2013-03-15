#help_index "File/CD DVD"

U0 FillU16Palindrome(CPalindromeU16 *dst,U16 w)
{
  dst->big=EndianU16(w);
  dst->little=w;
}

U0 FillU32Palindrome(CPalindromeU32 *dst,I64 d)
{
  dst->big=EndianU32(d);
  dst->little=d;
}


class CCCDUserData //Create CD
{
  I64 loc,path_entry_num,
	short_dir_blks,long_dir_blks;
};

I64 CDFileCreate2(CFile *out_file,CDirEntry *tempm,
		    CISO1DirEntry *tempi,CISO1DirEntry *tempi2,
		    I64 *_cur_blk,
		    CDirEntry *parent,Bool write)
{
  CCCDUserData *tempc;
  CDirEntry *tempm1,*tempm2;
  CFile *in_file;
  U8 *buf=MAlloc(CD_BLK_SIZE),*ptr1,*ptr2;
  CISO1DirEntry *dir_blk_buf =CAlloc(CD_BLK_SIZE*128),
		*de =dir_blk_buf, *de1,
		*dir_blk_buf2=CAlloc(CD_BLK_SIZE*128),
		*de2=dir_blk_buf2,*de12;
  I64 i,n;

  tempc=parent->user_data;

  de->len=sizeof(CISO1DirEntry)-1;
  de->ext_attr_len=0;
  FillU16Palindrome(&de->vol_seq_num,1);
  CDate2ISO1(&de->date,tempm->datetime);
  de->flags=ISO1_ATTR_DIR;
  de->name_len=1;
  de->name=0;
  de->len+=de->name_len;
  de(U8 *)+=de->len;

  de->len=sizeof(CISO1DirEntry)-1;
  de->ext_attr_len=0;
  FillU32Palindrome(&de->loc,tempc->loc);
  FillU32Palindrome(&de->size,tempc->short_dir_blks*CD_BLK_SIZE);
  FillU16Palindrome(&de->vol_seq_num,1);
  CDate2ISO1(&de->date,parent->datetime);
  de->flags=ISO1_ATTR_DIR;
  de->name_len=1;
  de->name=1;
  de->len+=de->name_len;
  de(U8 *)+=de->len;

  de2->len=sizeof(CISO1DirEntry)-1;
  de2->ext_attr_len=0;
  FillU16Palindrome(&de2->vol_seq_num,1);
  CDate2ISO1(&de2->date,tempm->datetime);
  de2->flags=ISO1_ATTR_DIR;
  de2->name_len=1;
  de2->name=0;
  de2->len=de2->len+de2->name_len;
  de2(U8 *)+=de2->len;

  de2->len=sizeof(CISO1DirEntry)-1;
  de2->ext_attr_len=0;
  FillU32Palindrome(&de2->loc,tempc->loc+tempc->short_dir_blks);
  FillU32Palindrome(&de2->size,tempc->long_dir_blks*CD_BLK_SIZE);
  FillU16Palindrome(&de2->vol_seq_num,1);
  CDate2ISO1(&de2->date,parent->datetime);
  de2->flags=ISO1_ATTR_DIR;
  de2->name_len=1;
  de2->name=1;
  de2->len+=de2->name_len;
  de2(U8 *)+=de2->len;

  tempm1=tempm->sub;
  while (tempm1) {
    tempm2=tempm1->next;
    if (!write) tempm1->user_data=CAlloc(sizeof(CCCDUserData));
    de1=de;
    de12=de2;
    if (tempm1->attr & _ATTR_DIR) {
      n=CDFileCreate2(out_file,tempm1,de,de2,_cur_blk,tempm,write);
      de(U8 *)+=sizeof(CISO1DirEntry)-1+n;
      de2(U8 *)+=sizeof(CISO1DirEntry)-1+n<<1;
    } else {
      tempc=tempm1->user_data;
      de->len=sizeof(CISO1DirEntry)-1;
      de->ext_attr_len=0;
      FillU32Palindrome(&de->loc,*_cur_blk);
      tempc->loc=*_cur_blk;
      if (write)
	"%X:%s\n",*_cur_blk,tempm1->full_name;
      FillU32Palindrome(&de->size,tempm1->size);
      FillU16Palindrome(&de->vol_seq_num,1);
      CDate2ISO1(&de->date,tempm1->datetime);
      de->flags=0;
      de->name_len=StrLen(tempm1->name);
      StrCpy(&de->name,tempm1->name);
      de->len=de->len+de->name_len;
      de(U8 *)+=de->len;

      de2->len=sizeof(CISO1DirEntry)-1;
      de2->ext_attr_len=0;
      FillU32Palindrome(&de2->loc,*_cur_blk);
      FillU32Palindrome(&de2->size,tempm1->size);
      FillU16Palindrome(&de2->vol_seq_num,1);
      CDate2ISO1(&de2->date,tempm1->datetime);
      de2->flags=0;
      de2->name_len=StrLen(tempm1->name)<<1;
      ptr1=&de2->name;
      ptr2=&tempm1->name;
      for (i=0;i<de2->name_len;i=i+2) {
	ptr1++;
	*ptr1++=*ptr2++;
      }
      de2->len+=de2->name_len;
      de2(U8 *)+=de2->len;

      in_file=FOpen(tempm1->full_name,"r");
      for (i=0;i<(FSize(in_file)+CD_BLK_SIZE-1)/CD_BLK_SIZE;i++) {
	n=4;
	if ((i+1)<<2>(FSize(in_file)+BLK_SIZE-1)>>BLK_SIZE_BITS) {
	  n=(FSize(in_file)+BLK_SIZE-1)>>BLK_SIZE_BITS&3;
	  MemSet(buf,0,CD_BLK_SIZE);
	}
	if (write) {
	  FRBlks(in_file,buf,i<<2,n);
	  FWBlks(out_file,buf,*_cur_blk<<2,n);
	}
	*_cur_blk+=1;
      }
      FClose(in_file);
    }
    if ((de1(U8 *)-dir_blk_buf(U8 *))/CD_BLK_SIZE!=
	(de(U8 *) -dir_blk_buf(U8 *))/CD_BLK_SIZE) {
      i=de1->len;
      MemCpy(buf,de1,i);
      MemSet(de1,0,i);
      de=dir_blk_buf(U8 *)+(de(U8 *)-dir_blk_buf(U8 *))/CD_BLK_SIZE*CD_BLK_SIZE;
      MemCpy(de,buf,i);
      de(U8 *)+=i;
    }
    if ((de12(U8 *)-dir_blk_buf2(U8 *))/CD_BLK_SIZE!=
	(de2(U8 *) -dir_blk_buf2(U8 *))/CD_BLK_SIZE) {
      i=de12->len;
      MemCpy(buf,de12,i);
      MemSet(de12,0,i);
      de2(U8 *)=dir_blk_buf2(U8 *)+(de2(U8 *)-dir_blk_buf2(U8 *))/CD_BLK_SIZE*CD_BLK_SIZE;
      MemCpy(de2,buf,i);
      de2(U8 *)+=i;
    }
    tempm1=tempm2;
  }

  tempc=tempm->user_data;

  tempi->len=sizeof(CISO1DirEntry)-1;
  tempi->ext_attr_len=0;
  tempi->flags=ISO1_ATTR_DIR;
  if (!tempm->name[0]) {
    tempi->name_len=1;
    tempi->name=1;
  } else {
    tempi->name_len=StrLen(tempm->name);
    StrCpy(&tempi->name,tempm->name);
  }
  tempi->len+=tempi->name_len;

  n=de(U8 *)+1-dir_blk_buf(U8 *);
  n=(n+CD_BLK_SIZE-1)/CD_BLK_SIZE;
  FillU32Palindrome(&tempi->size,n*CD_BLK_SIZE);
  FillU32Palindrome(&tempi->loc,*_cur_blk);
  tempc->short_dir_blks=n;
  tempc->loc=*_cur_blk;
  FillU32Palindrome(&dir_blk_buf->size,n*CD_BLK_SIZE);
  FillU32Palindrome(&dir_blk_buf->loc,*_cur_blk);
  FillU16Palindrome(&tempi->vol_seq_num,1);
  CDate2ISO1(&tempi->date,tempm->datetime);
  if (write)
    "%X:%s\n",*_cur_blk,tempm->full_name;
  if (write)
    FWBlks(out_file,dir_blk_buf,*_cur_blk<<2,n<<2);
  *_cur_blk+=n;

  tempi2->len=sizeof(CISO1DirEntry)-1;
  tempi2->ext_attr_len=0;
  tempi2->flags=ISO1_ATTR_DIR;
  if (!tempm->name[0]) {
    tempi2->name_len=1;
    tempi->name=1;
  } else {
    tempi2->name_len=StrLen(tempm->name)<<1;
    ptr1=&tempi2->name;
    ptr2=&tempm->name;
    for (i=0;i<tempi2->name_len;i=i+2) {
      ptr1++;
      *ptr1++=*ptr2++;
    }
  }
  tempi2->len+=tempi2->name_len;
  n=de2(U8 *)+1-dir_blk_buf2(U8 *);
  n=(n+CD_BLK_SIZE-1)/CD_BLK_SIZE;
  FillU32Palindrome(&tempi2->size,n*CD_BLK_SIZE);
  FillU32Palindrome(&tempi2->loc,*_cur_blk);
  tempc->long_dir_blks=n;
  FillU32Palindrome(&dir_blk_buf2->size,n*CD_BLK_SIZE);
  FillU32Palindrome(&dir_blk_buf2->loc,*_cur_blk);
  FillU16Palindrome(&tempi2->vol_seq_num,1);
  CDate2ISO1(&tempi2->date,tempm->datetime);
  if (write)
    "%X:%s\n",*_cur_blk,tempm->full_name;
  if (write)
    FWBlks(out_file,dir_blk_buf2,*_cur_blk<<2,n<<2);
  *_cur_blk+=n;

  Free(dir_blk_buf);
  Free(dir_blk_buf2);
  Free(buf);
  return tempi->name_len;
}

I64 CDTableLen(CDirEntry *tempm,I64 *size1,I64 *size2,I64 cur_depth)
//returns depth
{
  CDirEntry *tempm1=tempm->sub;
  I64 max_depth=cur_depth,i;
  while (tempm1) {
    if (tempm1->attr & _ATTR_DIR) {
      *size1+=sizeof(CISO1PathEntry)-2+ (StrLen(tempm1->name)+1)&-0x2;
      *size2+=sizeof(CISO1PathEntry)-2+StrLen(tempm1->name)<<1;
      i=CDTableLen(tempm1,size1,size2,cur_depth+1);
      if (i>max_depth) max_depth=i;
    }
    tempm1=tempm1->next;
  }
  return max_depth;
}

U0 CDFillPathTable(CDirEntry *tempm,
  CISO1PathEntry **_itabbuf,CISO1PathEntry **_itabbuf2,
  I64 parent_entry_num,Bool big_endian,I64 *first_free,I64 cur_level,I64 output_level)
{
  U8 *ptr1,*ptr2;
  I64 i;
  CISO1PathEntry *tabbuf =*_itabbuf,
		 *tabbuf2=*_itabbuf2;
  CDirEntry *tempm1=tempm->sub,*tempm2;
  CCCDUserData *tempc;
 
  if (cur_level==output_level) {
    while (tempm1) {
      if (tempm1->attr & _ATTR_DIR) {
	tempc=tempm1->user_data;
	tempc->path_entry_num=*first_free;
	tabbuf->name_len=StrLen(tempm1->name);
	if (big_endian) {
	  tabbuf->blk=EndianU32(tempc->loc);
	  tabbuf->parent_entry_num=EndianU16(parent_entry_num);
	} else {
	  tabbuf->blk=tempc->loc;
	  tabbuf->parent_entry_num=parent_entry_num;
	}
	StrCpy(&tabbuf->name,tempm1->name);

	tabbuf(U8 *)+=sizeof(CISO1PathEntry)-2+
	  (StrLen(tempm1->name)+1) & -0x2;

	tabbuf2->name_len=StrLen(tempm1->name)<<1;
	if (big_endian) {
	  tabbuf2->blk=EndianU32(tempc->loc+tempc->short_dir_blks);
	  tabbuf2->parent_entry_num=EndianU16(parent_entry_num);
	} else {
	  tabbuf2->blk=tempc->loc+tempc->short_dir_blks;
	  tabbuf2->parent_entry_num=parent_entry_num;
	}
	ptr1=&tabbuf2->name;
	ptr2=&tempm1->name;
	for (i=0;i<tabbuf2->name_len;i=i+2) {
	  ptr1++;
	  *ptr1++=*ptr2++;
	}
	tabbuf2(U8 *)+=sizeof(CISO1PathEntry)-2+
	  StrLen(tempm1->name)<<1;
	*first_free+=1;
      }
      tempm1=tempm1->next;
    }
    *_itabbuf=tabbuf;
    *_itabbuf2=tabbuf2;
  }
  tempm1=tempm->sub;
  while (tempm1) {
    tempm2=tempm1->next;
    if (tempm1->attr & _ATTR_DIR) {
      tempc=tempm1->user_data;
      CDFillPathTable(tempm1,_itabbuf,_itabbuf2,tempc->path_entry_num,
	 big_endian,first_free,cur_level+1,output_level);
    }
    tempm1=tempm2;
  }
}

class CElTorito
{
  U16 w[16];
  U8 bootable; //88=bootable 00=not bootable
  U8 media_type; //0=no emulation 4=hard disk
  U16 load_seg; //0000->07C0
  U8 sys_type;
  U8 zero;
  U16 sect_cnt;
  U32 load_rba; //start addr of virtual disk
  U8 zero2[20];
};

public I64 CDFileCreate(U8 *filename=NULL,U8 *src_files_find_mask,U8 *fu_flags=NULL,
		  U8 *bootfile=NULL,I64 emulation=0)
//See $LK-A,"::/TempleOS/Demo/MakeDistro.CPP.Z"$

//NULL filename uses sys_dft_iso_name.
//See the above sample code
//0=no emu 2=1.44meg 4=hard drive

//Use "C:/*" if you want all files on the C: drive.
//Default flags are "+r" recurse.

{
  CISO1PriDesc *iso =CAlloc(CD_BLK_SIZE),
		       *iso1=CAlloc(CD_BLK_SIZE),
		       *iso2=CAlloc(CD_BLK_SIZE),
		       *iso3=CAlloc(CD_BLK_SIZE);
  CDirEntry *rootdir=CAlloc(sizeof(CDirEntry));
  I64 i,j,
      cur_blk=0,bootfile_cd_blks=0,tabsize,tabsize2,first_free,max_depth,
      fuf_flags=0;
  U32 *d;
  CElTorito *et=CAlloc(CD_BLK_SIZE);
  U8 *filename2,
     *zero_buf=CAlloc(CD_BLK_SIZE),
     *bootfile_buf=NULL;
  CISO1PathEntry *tabbuf=NULL,*tabbuf2=NULL,*itabbuf,*itabbuf2;
  Bool okay=TRUE;
  CFile *out_file=NULL;
  CISO1DirEntry *tempi;
  CCCDUserData *tempc;

  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  GetFFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);

  if (!filename)
    filename=sys_dft_iso_name;
  filename2=DftExt(filename,"ISO");

  if (bootfile) {
    bootfile_buf=FileRead(bootfile,&bootfile_cd_blks);
    if (bootfile_cd_blks>CD_BLK_SIZE*4) {
      ST_ERR_ST "Boot files limited to 0x2000 bytes\n";
      goto done1;
    }
    bootfile_cd_blks=(bootfile_cd_blks+CD_BLK_SIZE-1)/CD_BLK_SIZE;
  }
  rootdir->attr=_ATTR_DIR;
  rootdir->sub=FilesFind(src_files_find_mask,fuf_flags);
  rootdir->datetime=GetCurTimeCDate;
  rootdir->user_data=CAlloc(sizeof(CCCDUserData));
  tempc=rootdir->user_data;
  tempc->path_entry_num=1;

  if (okay) {
    cur_blk=CD_FILE_OFFSET>>2;
    if (bootfile_buf)
      //preboot and bootloader
      cur_blk+=1+bootfile_cd_blks;
    CDFileCreate2(out_file,rootdir,
	&iso->root_dir_record,
	&iso2->root_dir_record,
	&cur_blk,rootdir,FALSE);
    tabsize=sizeof(CISO1PathEntry);
    tabsize2=sizeof(CISO1PathEntry);
    max_depth=CDTableLen(rootdir,&tabsize,&tabsize2,1);
    FillU32Palindrome(&iso->path_table_size,tabsize);
    FillU32Palindrome(&iso2->path_table_size,tabsize2);
    tabsize=(tabsize+CD_BLK_SIZE-1)/CD_BLK_SIZE;
    cur_blk+=tabsize<<1;
    tabsize2=(tabsize2+CD_BLK_SIZE-1)/CD_BLK_SIZE;
    cur_blk+=tabsize2<<1;
  }
  if (okay) {
    if (FileAttr(filename2) & _ATTR_CONTIGUOUS)
      out_file=FOpen(filename2,"wc",cur_blk<<2);
    else
      out_file=FOpen(filename2,"w",cur_blk<<2);
    if (!out_file)
      okay=FALSE;
  }
  if (okay) {
    cur_blk=0;
    while (cur_blk<CD_FILE_OFFSET>>2) {
      FWBlks(out_file,zero_buf,cur_blk<<2,4);
      cur_blk++;
    }
    iso->type=ISO1T_PRI_VOL_DESC;
    StrCpy(iso->id,"CD001");
    iso->version=1;
    FillU16Palindrome(&iso->vol_set_size,1);
    FillU16Palindrome(&iso->vol_seq_num,1);
    FillU16Palindrome(&iso->log_block_size,CD_BLK_SIZE);
    iso->file_structure_version=1;

    iso2->type=ISO1T_SUPPLEMENTARY_DESC;
    StrCpy(iso2->id,"CD001");
    iso2->version=1;
    FillU16Palindrome(&iso2->vol_set_size,1);
    FillU16Palindrome(&iso2->vol_seq_num,1);
    FillU16Palindrome(&iso2->log_block_size,CD_BLK_SIZE);
    iso2->file_structure_version=1;

    iso1->type=ISO1T_BOOT_RECORD;
    StrCpy(iso1->id,"CD001");
    iso1->version=1;
    StrCpy(iso1(U8 *)+7,"EL TORITO SPECIFICATION");

    cur_blk=CD_FILE_OFFSET>>2;

    if (bootfile_buf) {
      d=iso1(U8 *)+0x47;
      *d=cur_blk;
      et->w[0]=1;
      StrCpy(&et->w[2],"TempleOS");
      et->w[15]=0xAA55;
      j=0;
      for (i=0;i<16;i++) //Checksum
	j+=et->w[i];
      et->w[14]=-j;
      if (bootfile_buf)
	et->bootable=0x88;
      //0=no emu 2=1.44meg 4=hard drive
      et->media_type=emulation;
      et->sect_cnt=4;  //5 seems like the limit, 4 is safer
      et->load_rba=cur_blk+1;
//$AN,"CDBOOT","CDBOOT"$ See $LK,"CDBootFileCreate","MN:CDBootFileCreate"$
//See $LK,"BootCdProbe","MN:BootCdProbe"$().
      "%X: Pre Boot Blk\n",cur_blk;
      FWBlks(out_file,et,cur_blk<<2,4);
      cur_blk++;

      "%X: Boot Loader Blks\n",cur_blk;
      FWBlks(out_file,bootfile_buf,cur_blk<<2,bootfile_cd_blks<<2);
      cur_blk+=bootfile_cd_blks;
    }

    CDFileCreate2(out_file,rootdir,
	&iso->root_dir_record,
	&iso2->root_dir_record,
	&cur_blk,rootdir,TRUE);

    tabbuf=CAlloc(tabsize*CD_BLK_SIZE);
    iso->type_l_path_table=cur_blk;
    tabbuf->name_len=2;  //Fill-in adam entry
    tempi=&iso->root_dir_record;
    tabbuf->blk=tempi->loc.little;
    tabbuf->parent_entry_num=1;
    tabbuf2=CAlloc(tabsize2*CD_BLK_SIZE);
    iso2->type_l_path_table=cur_blk+tabsize;
    tabbuf2->name_len=2;  //Fill-in adam entry
    tempi=&iso2->root_dir_record;
    tabbuf2->blk=tempi->loc.little;
    tabbuf2->parent_entry_num=1;
    itabbuf=tabbuf+1;
    itabbuf2=tabbuf2+1;
    first_free=2;
    for (i=1;i<=max_depth;i++)
      CDFillPathTable(rootdir,&itabbuf,&itabbuf2,
	1,FALSE,&first_free,1,i);
    "%X: Path Table 0\n",cur_blk;
    FWBlks(out_file,tabbuf,cur_blk<<2,tabsize<<2);
    cur_blk+=tabsize;
    "%X: Path Table 1\n",cur_blk;
    FWBlks(out_file,tabbuf2,cur_blk<<2,tabsize2<<2);
    cur_blk+=tabsize2;


    MemSet(tabbuf,0,tabsize*CD_BLK_SIZE);
    iso->type_m_path_table=EndianU32(cur_blk);
    tabbuf->name_len=2;  //Fill-in adam entry
    tempi=&iso->root_dir_record;
    tabbuf->blk=tempi->loc.big;
    tabbuf->parent_entry_num=EndianU16(1);
    MemSet(tabbuf2,0,tabsize2*CD_BLK_SIZE);
    iso2->type_m_path_table=EndianU32(cur_blk+tabsize);
    tabbuf2->name_len=2;  //Fill-in adam entry
    tempi=&iso2->root_dir_record;
    tabbuf2->blk=tempi->loc.big;
    tabbuf2->parent_entry_num=EndianU16(1);
    itabbuf=tabbuf+1;
    itabbuf2=tabbuf2+1;
    first_free=2;
    for (i=1;i<=max_depth;i++)
      CDFillPathTable(rootdir,&itabbuf,&itabbuf2,
	1,TRUE,&first_free,1,i);
    "%X: Path Table 2\n",cur_blk;
    FWBlks(out_file,tabbuf,cur_blk<<2,tabsize<<2);
    cur_blk+=tabsize;
    "%X: Path Table 3\n",cur_blk;
    FWBlks(out_file,tabbuf2,cur_blk<<2,tabsize2<<2);
    cur_blk+=tabsize2;

    CDirLstDel2(rootdir);
    FillU32Palindrome(&iso->vol_space_size,cur_blk);
    FillU32Palindrome(&iso2->vol_space_size,cur_blk);
    FWBlks(out_file,iso,16<<2,4);

    iso3->type=ISO1T_TERMINATOR;
    StrCpy(iso3->id,"CD001");
    iso3->version=1;
    if (bootfile_buf) {
      FWBlks(out_file,iso1,17<<2,4);
      FWBlks(out_file,iso2,18<<2,4);
      FWBlks(out_file,iso3,19<<2,4);
    } else {
      FWBlks(out_file,iso2,17<<2,4);
      FWBlks(out_file,iso3,18<<2,4);
    }
  } else
    cur_blk=0;

  FClose(out_file);
  Free(tabbuf);
  Free(tabbuf2);
done1:
  Free(bootfile_buf);
  Free(filename2);
  Free(et);
  Free(zero_buf);
  Free(iso);
  Free(iso1);
  Free(iso2);
  Free(iso3);
  return cur_blk;
}
#help_index ""
