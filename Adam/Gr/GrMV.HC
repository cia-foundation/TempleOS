#help_index "Graphics/Device Contexts;Graphics/GR Files"

#define MV_FILE_MAX	0x007FF000

public I64 GR2MV(U8 *print_fmt="VID%03d.MV",
	U8 *files_find_mask,U8 *fu_flags=NULL)
{/*Merge GR lst to MV file.
"+d" will delete GR list files.
*/
  I64 fuf_flags=0,file_num=0,size;
  CDirEntry *tmpde,*tmpde1;
  U8 *base,*dst,*src,*st;
  CDC *dc,*dc_base=DCNew(GR_WIDTH,GR_HEIGHT);
  CDate cdt;
  Bool old_silent;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND);
  if (progress1_max=LinkedLstCnt(tmpde1)) {
    dst=base=MAlloc(MV_FILE_MAX);
    while (tmpde) {
      dc=GRRead(tmpde->full_name);
      GrBlot(dc_base,dc->x0,dc->y0,dc);
      if (dst+GR_FILE_MAX>base+MV_FILE_MAX) {
	st=MStrPrint(print_fmt,file_num++);
	FileWrite(st,base,dst-base);
	Free(st);
	dst=base;

	cdt=dc->cdt;
	dc=DCCopy(dc_base);
	dc->cdt=cdt;
      }
      src=DCSave(dc,&size);
      DCDel(dc);
      MemCpy(dst,src,size);
      dst+=size;
      Free(src);
      if (fuf_flags&FUF_DEL) {
	old_silent=Silent;
	Del(tmpde->full_name);
	Silent(old_silent);
      }
      progress1++;
      tmpde=tmpde->next;
    }
    if (dst>base) {
      st=MStrPrint(print_fmt,file_num++);
      FileWrite(st,base,dst-base);
      Free(st);
    }
    Free(base);
  }
  progress1=progress1_max=0;
  DirTreeDel(tmpde1);
  DCDel(dc_base);
  return file_num;
}
