#help_index "Compiler"
DefineLstLoad("ST_BIN_FILE_TYPES",
"END\0 \0"
"REL_EXPORT\0"
"LIT_EXPORT\0"
"REL_U8\0"
"LIT_U8\0"
"REL_U16\0"
"LIT_U16\0"
"REL_U32\0"
"LIT_U32\0"
"REL_U64\0"
"LIT_U64\0"
" \0 \0 \0 \0"
"MAIN\0"
"MODULE_ALIGN\0MODULE_ORG\0"
" \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0"
"ABS_ADDR\0"
"HEAP_GLBL\0"
);


public Bool BinRep(U8 *filename)
{//This reports imports, exports, etc
 //for a .BI? file.
//See $LK,"Loader Patch Table Generation","FF:::/TempleOS/Compiler/CmpMain.CPP,EIE_MODULE_ALIGNMENT"$ and $LK,"Load","MN:Load"$().
  U8 *fbuf,*ptr,etype,*sptr,*absname;
  I64 i,j,size;
  CBinFile *bfh,*bfh2;
  CHashExport *saved_hash_ptr=NULL;
  fbuf=DftExt(filename,"BIN.Z");
  if (!(bfh2=FileRead(fbuf,&size))) {
    Free(fbuf);
    return FALSE;
  }
  bfh=bfh2;

  if (ArcXSum(bfh,bfh->file_size)!=NATIVE_XSUM) {
    "Bad XSum\n\n";
    return FALSE;
  }

  absname=FileNameAbs(fbuf);
  Free(fbuf);
  fbuf=StrNew(absname);
  FileExtRem(fbuf);
  if (fbuf[1]==':' && StrLen(fbuf)>2)
    "Module Name:%s\n",fbuf+2;
  else
    "No Module Name\n";
  Free(fbuf);

  ptr=bfh(U8 *)+bfh->loader_patch_table_offset;
  while (etype=*ptr++) {
    i=*ptr(U32 *)++;
    sptr=ptr;
    ptr+=StrLen(sptr)+1;
    "%16Z:$$FG,GREEN$$%s$$FG$$:%X ",etype,"ST_BIN_FILE_TYPES",sptr,i;
    switch (etype) {
      case EIE_REL_U8...EIE_LIT_U64:
	if (*sptr)
	  saved_hash_ptr=HashFind(sptr,Fs->hash_table,HTG_ALL);
	if (!saved_hash_ptr)
	  "%S%s\n","ST_UNRESOLVED_REFERENCE",sptr;
	break;
      case EIE_ABS_ADDR:
	NewLine;
	for (j=0;j<i;j++)
	  "%X ",*ptr(U32 *)++;
	break;
      case EIE_HEAP_GLBL:
	"Size:%X ",*ptr(I64 *)++;
	for (j=0;j<i;j++)
	  "%X ",*ptr(U32 *)++;
	break;
    }
    NewLine;
  }
  Free(absname);
  Free(bfh);
  return TRUE;
}
#help_index ""
