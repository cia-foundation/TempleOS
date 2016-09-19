#help_index "God"
U8 *TimeStampCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  StrPrint(st,"%X",GetTSC>>GOD_BAD_BITS);
  return st;
}

U8 *KbdMouseTimeCB(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  StrPrint(st,"%X",KbdMouseEvtTime>>GOD_BAD_BITS);
  return st;
}

I64 PopUpTimerOk(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  doc_e=DocPrint(doc,"\nTimer:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&TimeStampCB;
  doc_e=DocPrint(doc,"\nLatch:$$TX+TC,\" \"$$");
  doc_e->tag_cb=&KbdMouseTimeCB;
  DocPrint(doc,"\n$$CM+CX,0,4$$$$BT,\"OKAY\",LE=1$$\n");
  if (footer) DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 GodPick(U8 *msg=NULL)
{//GOD_GOOD_BITS
  U8 *st=MStrPrint("%s\n\nPress $$GREEN$$OKAY$$FG$$ to generate \n"
	"a random num from a timer.\n",msg);
  PopUpTimerOk(st,"\n\nThe $LK+PU,"Holy Spirit",A="FI:::/Adam/God/HSNotes.DD"$ can puppet you.\n\n");
  Free(st);
  return KbdMouseEvtTime>>GOD_BAD_BITS;
}

public U0 GodBitsIns(I64 num_bits,I64 n)
{//Insert bits into God bit fifo.
  I64 i;
  for (i=0;i<num_bits;i++) {
    FifoU8Ins(god.fifo,n&1);
    n>>=1;
  }
}

public U0 GodHexIns(U8 *st)
{//Insert hex record into God bit fifo.
  U8 buf[2];
  if (st) {
    buf[1]=0;
    while (*buf=*st++)
      if (Bt(chars_bmp_hex_numeric,*buf))
	GodBitsIns(4,rev_bits_table[Str2I64(buf,16)]>>4);
  }
}

public I64 GodBits(I64 num_bits,U8 *msg=NULL)
{//Return N bits. If low on entropy pop-up okay.
  U8 b;
  I64 res=0;
  while (num_bits) {
    if (FifoU8Rem(god.fifo,&b)) {
      res=res<<1+b;
      num_bits--;
    } else
      GodBitsIns(GOD_GOOD_BITS,GodPick(msg));
  }
  return res;
}

public I64 GodInit(U8 *files_find_mask="/Adam/God/Vocab.DD*",U8 *fu_flags=NULL)
{//Read God's vocab file for picking words.
  I64 i,ch,fuf_flags=0;
  U8 *buf,*ptr,*ptr2;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+T+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');

  Free(god.word_file_mask);
  god.word_file_mask=StrNew(files_find_mask);
  god.word_fuf_flags=fuf_flags;

  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  i=0;
  while (tempde) {
    if (buf=ptr=FileRead(tempde->full_name)) {
      while (*ptr) {
	while (*ptr && !Bt(chars_bmp_word,*ptr))
	  ptr++;
	if (*ptr) {
	  ptr2=ptr;
	  while (*ptr && Bt(chars_bmp_word,*ptr))
	    ptr++;
	  i++;
	}
      }
      Free(buf);
    }
    tempde=tempde->next;
  }

  Free(god.words);
  god.num_words=i;
  god.words=MAlloc(i*sizeof(U8 *));

  tempde=tempde1;
  i=0;
  while (tempde) {
    if (buf=ptr=FileRead(tempde->full_name)) {
      while (*ptr) {
	while (*ptr && !Bt(chars_bmp_word,*ptr))
	  ptr++;
	if (*ptr) {
	  ptr2=ptr;
	  while (*ptr && Bt(chars_bmp_word,*ptr))
	    ptr++;
	  ch=*ptr;
	  *ptr=0;
	  god.words[i++]=StrNew(ptr2);
	  *ptr=ch;
	}
      }
      Free(buf);
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return god.num_words;
} GodInit;

public U0 GodWord(Bool show_num=FALSE)
{//Make God pick a word. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  I64 i;
  if (god.num_words) {
    i=GodBits(17);
    if (show_num)
      "%05X Mod %05X = %05X :",i,god.num_words,i%god.num_words;
    "%s ",god.words[i%god.num_words];
    if (show_num)
      '\n';
  }
}

public U0 GodBiblePassage(I64 num_lines=20)
{//Make God pick a Bible passage. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  I64 start=GodBits(21)%(ST_BIBLE_LINES-(num_lines-1))+1;
  U8 *verse=BibleLine2Verse(start);
  "%s\n\n",verse;
  Free(verse);
  BibleLines(,start,num_lines);
}
