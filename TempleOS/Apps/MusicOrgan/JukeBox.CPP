CTask *SingleSong(U8 *msg,U8 *name)
{
  Bool old_preempt=Preempt(OFF);
  CTask *task=Spawn(&ServantUserCmdLine,NULL,name,Fs);
  LBts(&task->task_flags,TASKf_LOCK_TITLE);
  StrCpy(task->task_title,name);
  TaskRqstQue(task,Fs,";",1<<TASKCf_WAKE_MASTER);
  SetWinHorz(task->win_left,task->win_left+50,task);
  SetWinVert(2,2+8,task);
  Preempt(old_preempt);
  TaskRqstQue(task,NULL,msg,1<<TASKCf_EXIT_ON_COMPLETE);
  DocCur(task)->max_entries=100;
  return task;
}

#define JB_RUN_LEVEL_NULL	0
#define JB_RUN_LEVEL_ONE	1
#define JB_RUN_LEVEL_TWO	2

Bool JBPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{  //ch=ASCII; sc=scancode
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tempm;
  CDoc *s;
  if (!(sc&(SCF_ALT|SCF_CTRL|SCF_SHIFT))
    && doc_ce->type_u8==DOCT_MENU_VAL && doc_ce->left_exp>=0) {
    tempm=doc_ce->left_exp;
    if (sc.u8[0]==SC_DELETE ) {
      Beep;
      Silent(ON);
      Del(tempm->full_name);
      Silent(OFF);
    } else if ('0'<=ch<='9') {
      if (StrLen(doc_ce->tag)>1)
	doc_ce->tag[0]=ch;
      s=DocRead(tempm->full_name);
      doc_e=s->root.next;
      while (doc_e!=s && doc_e->type_u8!=DOCT_TEXT)
	doc_e=doc_e->next;
      if (doc_e!=s && doc_e->type_u8==DOCT_TEXT && StrLen(doc_e->tag)>=3) {
	doc_e->tag[2]=ch;
	DocWrite(s);
      }
      DocDel(s);
      return TRUE;
    } else if (ch==CH_SPACE||ch==CH_ESC)
      tempm->user_data++; //JB_RUN_LEVEL++
    //<SPACE BAR> is followed by <ESC> --> JB_RUN_LEVEL_TWO
    //Actual <ESC> just exits          --> JB_RUN_LEVEL_ONE
  }
  return FALSE;
}

public U0 JukeBox(U8 *dirname=NULL,U8 **_filename=NULL)
{
//_filename is for using this as a song-chooser program.
  I64 i=0,rating;
  U8 *st,*st2;
  CDirEntry *tempm,*tempm1;
  CDoc *doc=DocNew,*s;
  CDocEntry *doc_e;
  StrCpy(doc->desc,"JukeBox");
  CTask *task=NULL;
  if (_filename)
    *_filename=NULL;
  if (!dirname)
    dirname=StrNew("HOME/MusicOrgan");
  else
    dirname=StrNew(dirname);
  st=MSPrintF("%s/*.CPP.Z",dirname);
  tempm=FilesFind(st);
  Free(st);
  Free(dirname);
  doc->user_put_key=&JBPutKey;
  tempm1=tempm;
  DocPutS(doc,
  "Key: $$FG,GREEN$$Graphics $$FG,BLUE$$Words $$FG,RED$$No Nothing "
  "$$FG,BLACK$$Incomplete $$FG,CYAN$$Special$$FG$$\n\n"
  "$$FG,GREEN$$<DELETE>$$FG$$ to delete a song.\n"
      "Press a number to rate a song.\n");
  while (tempm1) {
    if (!(i++%5))
      DocPutS(doc,"\n");
    if (FileOcc("Play(",tempm1->full_name,"")) {
      st=StrNew(tempm1->name);
      FileExtRem(st);
      s=DocRead(tempm1->full_name);
      doc_e=s->root.next;
      while (doc_e!=s && doc_e->type_u8!=DOCT_TEXT)
	doc_e=doc_e->next;
      rating='0';
      if (doc_e!=s && doc_e->type_u8==DOCT_TEXT) {
	if ('0'<=doc_e->tag[2]<='9')
	  rating=doc_e->tag[2];
	if (StrStr(doc_e->tag,"incomplete"))
	  DocPutS(doc,"$$FM,BLACK$$");
	else if (StrStr(doc_e->tag,"has graphics"))
	  DocPutS(doc,"$$FM,GREEN$$");
	else if (StrStr(doc_e->tag,"has words"))
	  DocPutS(doc,"$$FM,BLUE$$");
	else if (StrStr(doc_e->tag,"special"))
	  DocPutS(doc,"$$FM,CYAN$$");
	else if (StrStr(doc_e->tag,"no nothing")) {
	  DocPutS(doc,"$$FM,RED$$");
	  if (FileOcc("\\0",tempm1->full_name,"")) {
	    s->cur_entry=doc_e->next;
	    s->cur_data_col=0;
	    DocEntryDel(s,doc_e);
	    DocPutS(s,"//0 has words\n");
	    DocWrite(s);
	  }
	}
	DocPrintF(doc,"$$MU-UL,\"%c%-8ts\",%d$$ ",rating,st,tempm1);
	tempm1->user_data=JB_RUN_LEVEL_NULL;
      }
      DocDel(s);
      Free(st);
    }
    tempm1=tempm1->next;
  }
  DocPrintF(doc,"\n$$FM,CYAN$$$$MU-UL,\"DONE\",%d$$\n",DOCM_CANCEL);
  while (TRUE) {
    if (_filename)
      tempm1=PopUpMenu(doc,DOF_INTERCEPT_TASK_END);
    else
      tempm1=PopUpMenu(doc);
    if (task)
      Kill(task);
    if (tempm1<0) break;
    st2=StrNew(tempm1->name);
    if (_filename) {
      Free(*_filename);
      *_filename=StrNew(tempm1->full_name);
    }
    if (tempm1->user_data==JB_RUN_LEVEL_ONE) break; //<ESC>
    tempm1->user_data=JB_RUN_LEVEL_NULL; //Rst from <SPACE BAR>
    FileExtRem(st2);
    st=MSPrintF("ExeFile(\"%s\");",tempm1->full_name);
    MusicSettingsRst;
    task=SingleSong(st,st2);
    Free(st2);
    Free(st);
  }
  DocDel(doc);
  CDirLstDel(tempm);
}
