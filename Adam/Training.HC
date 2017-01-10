#help_index "Help System"

U8 *KeyMapKeyMStrPrint(I64 sc,U0 (*fp_hndlr)(I64 sc),
	U8 *desc,CTask *task=NULL)
{
  I64 i=9,k,c;
  U8 *st,*st2,*res,*ptr;
  CHashTable *old_hash=Fs->hash_table;
  st=ScanCode2KeyName(sc);
  if (sc&SCF_CTRL)	i+=5;
  if (sc&SCF_ALT)	i+=4;
  if (sc&(SCF_SHIFT|SCF_NO_SHIFT)) i+=6;
  if (TaskValidate(task))
    Fs->hash_table=task->hash_table;
  st2=SrcEdLink(fp_hndlr,256);
  Fs->hash_table=old_hash;

  k=*desc(U32 *);
  if (k=='Edit')	c=BLUE;
  else if (k=='Dol ')	c=GREEN;
  else if (k=='Cmd ')	c=RED;
  else			c=BLACK;

  res=MStrPrint("%-*s $$FG,%d$$$$TX+UL+L+PU,\"%$$Q\",A=\"%s\"$$$$FG$$\n",
	i,st,c,desc,st2);
  Free(st);
  Free(st2);

  ptr=res;
  while (*ptr) {
    if (*ptr==CH_SPACE)
      *ptr=CH_SHIFT_SPACE;
    ptr++;
  }

  return res;
}

U0 KeyMapKeyPrint(I64 sc,U0 (*fp_hndlr)(I64 sc),U8 *desc,CTask *task=NULL)
{
  U8 *st=KeyMapKeyMStrPrint(sc,fp_hndlr,desc,task);
  "%s",st;
  Free(st);
}

U0 KeyMapCtrlAltFamily(Bool no_shift,Bool shift)
{
  I64 i,no_shift_f;
  if (no_shift && shift)
    no_shift_f=SCF_NO_SHIFT;
  else
    no_shift_f=0;
  if (no_shift) KeyMapKeyPrint(SC_DELETE+SCF_CTRL+SCF_ALT+no_shift_f,
	  &Reboot,"Cmd /Reboot");
  if (no_shift) KeyMapKeyPrint(SC_ESC+SCF_CTRL+SCF_ALT+no_shift_f,
	  &User,"Cmd /Terminal Window");
  if (no_shift) KeyMapKeyPrint(SC_TAB+SCF_CTRL+SCF_ALT+no_shift_f,
	  &WinToTop,"Cmd /Next Focus Task");

  for (i=0;i<26;i++)
    if (keydev.fp_ctrl_alt_cbs[i]) {
      if (no_shift && keydev.ctrl_alt_no_shift_descs[i])
	KeyMapKeyPrint(Char2ScanCode(i+'a')+SCF_CTRL+SCF_ALT+no_shift_f,
	      keydev.fp_ctrl_alt_cbs[i],keydev.ctrl_alt_no_shift_descs[i]);
      if (shift && keydev.ctrl_alt_shift_descs[i])
	KeyMapKeyPrint(Char2ScanCode(i+'a')+SCF_CTRL+SCF_ALT+SCF_SHIFT,
	      keydev.fp_ctrl_alt_cbs[i],keydev.ctrl_alt_shift_descs[i]);
    }
}

U0 KMComparePrepare(U8 *buf,I64 *src)
{
  I64 i,*dst=buf;
  U8 *ptr;
  if (src) {
    *dst++=*src++;
    *dst++=*src++;
    *dst++=*src++;
    *dst++=*src++;
    *dst(U8 *)=0;
    if (ptr=StrMatch("SHIFT",buf)) {
      for (i=0;i<5;i++)
	ptr[i]=CH_SHIFT_SPACE;
      if (ptr=StrMatch("$$",buf))
	*ptr=255;
    }
  } else
    *buf=0;
}

I64 KMCompare(U8 *e1,U8 *e2)
{
  U8 buf1[STR_LEN],buf2[STR_LEN];
  KMComparePrepare(buf1,e1);
  KMComparePrepare(buf2,e2);
  return StrCmp(buf1,buf2);
}

U0 KeyMapFamily2(U8 **entries,CTask *task,I64 scf)
{
  I64 i,arg1,arg2;
  for (i=0;i<256;i++) {
    arg2=scf|i|SCF_KEY_DESC;
    arg1=ScanCode2Char(arg2);
    *keydev.desc=0;
    keydev.hndlr=NULL;
    if (TaskValidate(task) && !Bt(&task->win_inhibit,WIf_SELF_KEY_DESC)) {
      if (task==Fs)
	PutKey(arg1,arg2);
      else
	PostMsg(task,MSG_KEY_DOWN,arg1,arg2);
      Refresh(0,TRUE);
      Sleep(1); //Open loop because might be no response.  TODO: Drops msgs.
    }
    if (*keydev.desc && StrNCmp(keydev.desc,"Char  /",7))
      entries[i]=KeyMapKeyMStrPrint(arg2,keydev.hndlr,keydev.desc,task);
  }
}

U0 KeyMapFamily(CTask *task,I64 scf,Bool no_shift,Bool shift)
{
  I64 i,cnt=0;
  U8 **entries=CAlloc(2*256*sizeof(U8 *)),**ptr=entries;
  if (no_shift) {
    if (shift)
      KeyMapFamily2(ptr,task,scf+SCF_NO_SHIFT);
    else
      KeyMapFamily2(ptr,task,scf);
    ptr+=256;
    cnt+=256;
  }
  if (shift) {
    KeyMapFamily2(ptr,task,scf+SCF_SHIFT);
    ptr+=256;
    cnt+=256;
  }
  QSortI64(entries,cnt,&KMCompare);
  for (i=0;i<cnt;i++)
    if (entries[i]) {
      "%s",entries[i];
      Free(entries[i]);
    }
  Free(entries);
}

public U0 KeyMap(CTask *task=NULL)
{//Report desc of all keys.
  Bool old_key_desc;
  if (!task) task=Fs;
  old_key_desc=LBtr(&task->win_inhibit,WIf_SELF_KEY_DESC);
  DocMax;
  KeyMapFamily(task,0,TRUE,TRUE);
  KeyMapFamily(task,SCF_CTRL,TRUE,TRUE);
  KeyMapFamily(task,SCF_ALT,TRUE,TRUE);
  KeyMapCtrlAltFamily(TRUE,TRUE);
  LBEqu(&task->win_inhibit,WIf_SELF_KEY_DESC,old_key_desc);
  "\nKeyMap Completed.\n";
}

#help_index "Help System/Training"
public U0 TipOfDay(U8 *tip_file="::/Doc/Tips.DD")
{//Print random tip-of-day from ::/Doc/Tips.DD.
  I64 i=RandU16;
  CDoc *doc=DocRead(tip_file),*doc2=DocNew;
  CDocEntry *doc_e=doc->head.next;
  "$$WW,1$$\n";
  while (TRUE) {
    if (doc_e->type_u8==DOCT_TEXT && *doc_e->tag=='*')
      if (!i--) break;
    doc_e=doc_e->next;
  }
  if (doc_e->type_u8==DOCT_TEXT && *doc_e->tag=='*') {
    while (doc_e!=doc) {
      if (doc_e->type_u8!=DOCT_ERROR)
	DocInsEntry(doc2,DocEntryCopy(doc2,doc_e));
      doc_e=doc_e->next;
      if (doc_e->type_u8==DOCT_TEXT && *doc_e->tag=='*')
	break;
    }
  }
  DocInsDoc(DocPut,doc2);
  DocDel(doc2);
  DocDel(doc);
}
