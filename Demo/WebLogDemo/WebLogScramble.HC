U8 key[4][256];

U0 InitKey()
{
  I64 i,j;
  for (i=0;i<4;i++) {
    for (j=0;j<256;j++)
      key[i][j]=j;
    for (j=0;j<255;j++)
      SwapU8(&key[i][j],&key[i][j+RandU32%(256-j)]);
  }
}

U0 LogScrambleFile(U8 *name)
{
  CDoc *doc=DocRead(name,DOCF_PLAIN_TEXT|DOCF_NO_CURSOR);
  CDocEntry *doc_e=doc->head.next;
  I64 i;
  U32 num;
  U8 *st,*st2;
  "Scrambling: %s\n",name;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TEXT) {
      st=MAlloc(StrLen(doc_e->tag)+1);
      st2=MAlloc(StrLen(doc_e->tag)+1);
      StrFirstRem(doc_e->tag," ",st);

      for (i=3;i>=0;i--) {
	StrFirstRem(st,".",st2);
	num.u8[i]=key[i][Str2I64(st2)&255];
      }

      Free(st);
      Free(st2);

      st=MStrPrint("%d.%d.%d.%d %s",num.u8[3],num.u8[2],
	    num.u8[1],num.u8[0],doc_e->tag);
      Free(doc_e->tag);
      doc_e->tag=st;
    }
    doc_e=doc_e->next;
  }
  DocWrite(doc);
  DocDel(doc);
}
U0 WebLogScramble(U8 *files_find_mask)
{
  CDirEntry *tmpde=FilesFind(files_find_mask,FUF_RECURSE|FUF_JUST_FILES),
	*tmpde1=tmpde;
  InitKey;
  while (tmpde) {
    LogScrambleFile(tmpde->full_name);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

U0 LogScrambleDemo()
{
  U8 *dirname;
  Cd(__DIR__);
  dirname=DirCur;
  ST_WARN_ST "This will scramble all .log files in '%s'.\n\n",dirname;
  Free(dirname);
  if (AreYouSure)
    WebLogScramble("*.log*");
}

LogScrambleDemo;
