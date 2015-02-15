#help_index "Training"

public U0 PressKeyHelp(CTask *task=NULL)
{//Jump to key handlers src for key presses.
  I64 a1=0,a2=0;
  Bool old_preempt=Preempt(OFF);
  CTask *old_hash=Fs->hash_table;
  if (!task) task=Fs;

/*
This feature is a burden in code.  It would be nice to lst key
cmds for all applications, but it makes the code too ugly.
*/
task=Fs;

  Fs->hash_table=task->hash_table;
  "Press keys to show handler code.\n"
  "<SHIFT-ESC> to exit.\n"
  "Do not press <CTRL-ALT-keys>.\n";
  while (a1!=CH_ESC && a1!=CH_SHIFT_ESC) {
    GetMsg(&a1,&a2,1<<MSG_KEY_DOWN);
    *keydev.desc=0;
    keydev.handler=NULL;
    if (task==Fs)
      PutKey(a1,a2|SCF_KEY_DESC);
    else if (TaskValidate(task)) {
      PostMsg(task,MSG_KEY_DOWN,a1,a2|SCF_KEY_DESC);
      while (!keydev.handler)
	Yield;
    }
    if (*keydev.desc)
      E(keydev.handler,512);
  }
  Fs->hash_table=old_hash;
  Preempt(old_preempt);
}

U0 KeyMapCtrlAltFamily()
{
  I64 i,a2;
  U8 *st,*st2;

  a2=SC_DELETE+SCF_CTRL+SCF_ALT;
  st2=ScanCode2KeyName(a2);
  st=SrcEdLink(&Reboot,256);
  "%20s $$LK,\"%s\",A=\"%s\"$$\n",st2,"Sys/Reboot",st;
  Free(st);
  Free(st2);

  a2=SC_ESC+SCF_CTRL+SCF_ALT;
  st2=ScanCode2KeyName(a2);
  st=SrcEdLink(&User,256);
  "%20s $$LK,\"%s\",A=\"%s\"$$\n",st2,"Sys/Spawn New User Task",st;
  Free(st);
  Free(st2);

  a2=SC_TAB+SCF_CTRL+SCF_ALT;
  st2=ScanCode2KeyName(a2);
  st=SrcEdLink(&WinToTop,256);
  "%20s $$LK,\"%s\",A=\"%s\"$$\n",st2,"Sys/Switch Focus to Next Task",st;
  Free(st);
  Free(st2);

  for (i=0;i<26;i++) {
    if (keydev.fp_ctrl_alt_let_cbs[i] &&
	keydev.ctrl_alt_let_descs[i]) {
      a2=Char2ScanCode(i+'a')+SCF_CTRL+SCF_ALT;
      st2=ScanCode2KeyName(a2);
      st=SrcEdLink(keydev.fp_ctrl_alt_let_cbs[i],256);
      "%20s $$LK,\"%s\",A=\"%s\"$$\n",st2,keydev.ctrl_alt_let_descs[i],st;
      Free(st);
      Free(st2);
    }
  }
}

I64 KMCompare(U8 *e1,U8 *e2)
{
  I64 i,ch1,ch2;
  U8 *ee1,*ee2;
  Bool e1_shift=FALSE,e2_shift=FALSE;
  if (e1) {
    if (e2) {
      while (*e1==CH_SPACE)
	e1++;
      while (*e2==CH_SPACE)
	e2++;
      while (*e1 && *e1==*e2) {
	e1++;
	e2++;
      }
      if (!StrNCmp(e1,"SHIFT",5)) {
	e1+=6;
	e1_shift=TRUE;
      } else if (!StrNCmp(e1,"     ",5))
	e1+=6;
      if (!StrNCmp(e2,"SHIFT",5)) {
	e2+=6;
	e2_shift=TRUE;
      } else if (!StrNCmp(e2,"     ",5))
	e2+=6;

      ee1=e1;
      while (*ee1 && *ee1!='$$')
	ee1++;
      ch1=*ee1;
      *ee1=0;

      ee2=e2;
      while (*ee2 && *ee2!='$$')
	ee2++;
      ch2=*ee2;
      *ee2=0;

      i=StrCmp(e1,e2);

      *ee1=ch1;
      *ee2=ch2;

      if (i)
	return i;
      else
	return e1_shift-e2_shift;
    } else
      return 1;
  } else {
    if (e2)
      return -1;
    else
      return 0;
  }
}

U0 KeyMapFamily2(U8 **entries,U8 *family,CTask *task,I64 scf)
{
  I64 i,a2,a1;
  U8 *st,*st2;
  CTask *old_hash=Fs->hash_table;
  Fs->hash_table=task->hash_table;
  for (i=0;i<256;i++) {
    a2=scf|i|SCF_KEY_DESC;
    a1=ScanCode2Char(a2);
    *keydev.desc=0;
    keydev.handler=NULL;
    if (task==Fs)
      PutKey(a1,a2);
    else if (TaskValidate(task)) {
      PostMsg(task,MSG_KEY_DOWN,a1,a2);
      while (!keydev.handler)
	Yield;
    }
    if (*keydev.desc && StrNCmp(keydev.desc,"Char/",5)) {
      st=ScanCode2KeyName(a2&~(SCF_CTRL|SCF_ALT|SCF_SHIFT));
      st2=MStrPrint("%s %s",family,st);
      Free(st);
      st=SrcEdLink(keydev.handler,256);
      entries[i]=MStrPrint("%20s $$LK,\"%s\",A=\"%s\"$$\n",
	  st2,keydev.desc,st);
      Free(st);
      Free(st2);
    }
  }
  Fs->hash_table=old_hash;
}

U0 KeyMapFamily(U8 *family,CTask *task,I64 scf)
{
  I64 i;
  U8 **entries=CAlloc(2*256*sizeof(U8 *)),st1[32],st2[32];
  StrPrint(st1,"%s      ",family);
  StrPrint(st2,"%s SHIFT",family);
  KeyMapFamily2(entries    ,st1,task,scf);
  KeyMapFamily2(entries+256,st2,task,scf+SCF_SHIFT);
  QSortI64(entries,2*256,&KMCompare);
  for (i=0;i<2*256;i++)
    if (entries[i]) {
      "%s",entries[i];
      Free(entries[i]);
    }
  Free(entries);
}

public U0 KeyMap(CTask *task=NULL)
{//Report desc of all keys.
  if (!task) task=Fs;
/*
This feature is a burden in code.  It would be nice to lst key
cmds for all applications, but it makes the code too ugly.
*/
task=Fs;

  DocMax;
  KeyMapFamily("",task,0);
  KeyMapFamily("CTRL",task,SCF_CTRL);
  KeyMapFamily("ALT",task,SCF_ALT);
  KeyMapCtrlAltFamily;
  "\n$$MA-X+PU,T=\"Click for help on key pressed\",LM=\"PressKeyHelp(0x%X);\"$$\n",task;
}

public U0 TipOfDay(U8 *tip_file="::/Doc/Tips.TXT")
{//Print random tip-of-day from ::/Doc/Tips.TXT.
  I64 i=RandU16;
  CDoc *doc=DocRead(tip_file),*doc2=DocNew;
  CDocEntry *doc_e=doc->root.next;
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
