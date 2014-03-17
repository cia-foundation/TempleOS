#help_index "Compiler"
DefineLstLoad("ST_BIN_FILE_TYPES",
"END\0 \0"
"REL_EXPORT\0"
"IMM_EXPORT\0"
"REL_I8\0"
"IMM_U8\0"
"REL_I16\0"
"IMM_U16\0"
"REL_I32\0"
"IMM_U32\0"
"REL_I64\0"
"IMM_U64\0"
" \0 \0 \0 \0"
"MAIN\0"
"MODULE_ALIGN\0MODULE_ORG\0"
" \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0"
"ABS_ADDR\0"
"HEAP_GLBL\0"
);


public Bool BinRep(U8 *filename)
{//This reports imports, exports, etc
 //for a .BI* file.
//See $LK,"Loader Patch Table Generation",A="FF:::/Compiler/CmpMain.CPP,EIT_MODULE_ALIGNMENT"$ and $LK,"Load",A="MN:Load"$().
  U8 *fbuf,*ptr,*sptr,*absname;
  I64 i,j,size,etype;
  CBinFile *bfh;
  CHashExport *saved_hash_ptr=NULL;
  fbuf=DftExt(filename,"BIN.Z");
  if (!(bfh=FileRead(fbuf,&size))) {
    Free(fbuf);
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
    "%16Z:$$GREEN$$%s$$FG$$:%X ",etype,"ST_BIN_FILE_TYPES",sptr,i;
    switch (etype) {
      case EIT_REL_I8...EIT_IMM_U64:
	if (*sptr)
	  saved_hash_ptr=HashFind(sptr,Fs->hash_table,HTG_ALL);
	if (!saved_hash_ptr)
	  "Unresolved Reference:%s\n",sptr;
	break;
      case EIT_ABS_ADDR:
	NewLine;
	for (j=0;j<i;j++)
	  "%X ",*ptr(U32 *)++;
	break;
      case EIT_HEAP_GLBL:
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
