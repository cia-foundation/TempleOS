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

U0 RedSeaISO9660Stage1(U8 *iso_filename,U8 *stage2_filename)
{
  CDirEntry de;
  CFile *out_file=NULL;
  U8	*stage1_buf=CAlloc(DVD_BOOT_LOADER_SIZE);
  if (FileFind(stage2_filename,&de) && (out_file=FOpen(iso_filename,"wc+"))) {
    MemCpy(stage1_buf,BDVD_START,BDVD_END-BDVD_START);
    *(BDVD_BLK_LO	-BDVD_START+stage1_buf)(U32 *)=de.clus>>2;
    *(BDVD_BLK_CNT	-BDVD_START+stage1_buf)(U16 *)=
	  (de.size+DVD_BLK_SIZE-1)>>(BLK_SIZE_BITS+2);
    *(BDVD_SHIFT_BLKS	-BDVD_START+stage1_buf)(U16 *)=de.clus&3;
    if (de.clus&3)
      *(BDVD_BLK_CNT	-BDVD_START+stage1_buf)(U16 *)+=1;
    FBlkWrite(out_file,stage1_buf,
	  20<<2+1<<2,DVD_BOOT_LOADER_SIZE/BLK_SIZE);
    FClose(out_file);
  }
  Free(stage1_buf);
}

U0 RedSeaISO9660(U8 *iso_filename,U8 drv_let)
{
  CDrv	*dv=Let2Drv(drv_let);
  CISOPriDesc *iso_pri=CAlloc(DVD_BLK_SIZE),
	*iso_boot=CAlloc(DVD_BLK_SIZE),
	*iso_sup=CAlloc(DVD_BLK_SIZE),
	*iso_term=CAlloc(DVD_BLK_SIZE);
  I64 iso_size=0,i,j;
  U32 *d;
  CElTorito *et=CAlloc(DVD_BLK_SIZE);
  U8	*zero_buf=CAlloc(DVD_BLK_SIZE);
  CFile *out_file=NULL;

  if (out_file=FOpen(iso_filename,"wc+")) {
    iso_size=FSize(out_file)/DVD_BLK_SIZE;
    for (i=0;i<dv->bd->drv_offset;i+=4)
      FBlkWrite(out_file,zero_buf,i,4);

    iso_pri->type=ISOT_PRI_VOL_DESC;
    StrCpy(iso_pri->id,"CD001");
    iso_pri->version=1;
    FillU16Palindrome(&iso_pri->vol_set_size,1);
    FillU16Palindrome(&iso_pri->vol_seq_num,1);
    FillU16Palindrome(&iso_pri->log_block_size,DVD_BLK_SIZE);
    FillU32Palindrome(&iso_pri->vol_space_size,iso_size);
    FillU32Palindrome(&iso_pri->root_dir_record,dv->root_clus);
    iso_pri->file_structure_version=1;
    StrCpy(iso_pri->publisher_id,"TempleOS RedSea");

    MemCpy(iso_sup,iso_pri,DVD_BLK_SIZE);
    iso_sup->type=ISOT_SUPPLEMENTARY_DESC;

    iso_boot->type=ISOT_BOOT_RECORD;
    StrCpy(iso_boot->id,"CD001");
    iso_boot->version=1;
    StrCpy(iso_boot(U8 *)+7,"EL TORITO SPECIFICATION");

    FBlkWrite(out_file,iso_pri,16<<2,4);
    iso_term->type=ISOT_TERMINATOR;
    StrCpy(iso_term->id,"CD001");
    iso_term->version=1;

    d=iso_boot(U8 *)+0x47;
    *d=20<<2>>2;
    FBlkWrite(out_file,iso_boot,17<<2,4);

    FBlkWrite(out_file,iso_sup,18<<2,4);
    FBlkWrite(out_file,iso_term,19<<2,4);

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
    et->load_rba=20<<2>>2+1;
    FBlkWrite(out_file,et,20<<2,4);
    FClose(out_file);
  }
  Free(zero_buf);
  Free(et);
  Free(iso_pri);
  Free(iso_boot);
  Free(iso_sup);
  Free(iso_term);
}

I64 RedSeaISOPass1(CDirEntry *tmpde)
{
  I64 dir_entry_cnt=3+LinkedLstCnt(tmpde),res=0;
  while (tmpde) {
    if (tmpde->attr & RS_ATTR_DIR) {
      if (tmpde->sub)
	res+=RedSeaISOPass1(tmpde->sub);
      else
	res+=BLK_SIZE; //Empty dir
    } else
      res+=CeilU64(tmpde->size,BLK_SIZE);
    tmpde=tmpde->next;
  }
  res+=CeilU64(dir_entry_cnt<<6,BLK_SIZE); //Size in bytes
#assert CDIR_SIZE==64
  return res;
}
public I64 RedSeaISO(U8 *_iso_filename=NULL,U8 *_src_dir,
	U8 *_stage2_filename=NULL)
{//See $LK,"::/Misc/DoDistro.HC"$. Must be ISO.C
  I64 i,res,root_cnt,root_dir_blks,bitmap_blks,bitmap_blks1;
  CDirEntry *tmpde;
  U8 buf[STR_LEN],*iso_filename,*src_dir,*stage2_filename;
  CDrv *dv=DrvMakeFreeSlot(DrvNextFreeLet('Q')); //First $LK,"BDT_ISO_FILE_WRITE",A="MN:BDT_ISO_FILE_WRITE"$
  CBlkDev *bd=BlkDevNextFreeSlot(dv->drv_let,BDT_ISO_FILE_WRITE);

  if (!IsDir(_src_dir))
    PrintErr("'%s' is not a dir.\n",_src_dir);
  else {
    if (!_iso_filename)
      _iso_filename=blkdev.dft_iso_c_filename;
    iso_filename=ExtChg(_iso_filename,"ISO.C");
    src_dir=DirNameAbs(_src_dir);
    if (_stage2_filename) {
      stage2_filename=FileNameAbs(_stage2_filename);
      *stage2_filename=dv->drv_let;
      i=StrLen(src_dir);
      if (i!=3)	//If not root
	i++;	//Skip slash
      StrCpy(stage2_filename+3,stage2_filename+i);
    } else
      stage2_filename=NULL;
    tmpde=FilesFind(src_dir,FUF_RECURSE);
    root_cnt=LinkedLstCnt(tmpde)+3;
    root_dir_blks=CeilU64(root_cnt<<6,BLK_SIZE)>>BLK_SIZE_BITS;
    if (res=RedSeaISOPass1(tmpde)>>BLK_SIZE_BITS) {
      bd->drv_offset=19<<2+(DVD_BLK_SIZE*2+DVD_BOOT_LOADER_SIZE)/BLK_SIZE;
      bitmap_blks=1;
      do {
	bitmap_blks1=bitmap_blks;
	bitmap_blks=(res+bitmap_blks+BLK_SIZE<<3-1)/BLK_SIZE<<3;
      } while (bitmap_blks!=bitmap_blks1);

      bd->max_blk=CeilI64(bd->drv_offset+1+bitmap_blks+res,4);
      bd->max_blk--; //Inclusive.
      bd->file_dsk_name=AStrNew(iso_filename);
      bd->init_root_dir_blks=root_dir_blks;
      BlkDevAdd(bd,,TRUE,TRUE);
      StrPrint(buf,"%C:/",dv->drv_let);
      CopyTree(src_dir,buf,TRUE);
      RedSeaISO9660Stage1(iso_filename,stage2_filename);
      DrvDel(dv);
      BlkDevDel(bd);
    }
    Free(stage2_filename);
    Free(src_dir);
    Free(iso_filename);
  }
  return res;
}
