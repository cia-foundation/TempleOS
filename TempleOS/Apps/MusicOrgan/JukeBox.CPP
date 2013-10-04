CTask *SingleSong(U8 *msg,U8 *name)
{
  Bool old_preempt=Preempt(OFF);
  CTask *task=Spawn(&SrvUserCmdLine,NULL,name,,Fs);
  StrCpy(task->task_title,name);
  task->title_src=TTS_LOCKED_CONST;
  TaskExeStrQue(task,Fs,";",1<<SVCf_WAKE_MASTER|1<<SVCf_FREE_ON_COMPLETE);
  SetWinHorz(task->win_left,task->win_left+50,task);
  SetWinVert(2,2+8,task);
  Preempt(old_preempt);
  TaskExeStrQue(task,NULL,msg,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
  DocPut(task)->max_entries=100;
  return task;
}

#define JB_RUN_LEVEL_NULL	0
#define JB_RUN_LEVEL_ONE	1
#define JB_RUN_LEVEL_TWO	2

Bool JBPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{  //ch=ASCII; sc=scancode
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tempde;
  CDoc *s;
  if (!(sc&(SCF_ALT|SCF_CTRL|SCF_SHIFT))
    && doc_ce->type_u8==DOCT_MENU_VAL && doc_ce->left_exp>=0) {
    tempde=doc_ce->left_exp;
    if (sc.u8[0]==SC_DELETE ) {
      Beep;
      Silent(ON);
      Del(tempde->full_name);
      Silent(OFF);
    } else if ('0'<=ch<='9') {
      if (StrLen(doc_ce->tag)>1)
	doc_ce->tag[0]=ch;
      s=DocRead(tempde->full_name);
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
      tempde->user_data++; //JB_RUN_LEVEL++
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
  CDirEntry *tempde,*tempde1;
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
  st=MPrint("%s/*.CPP.Z",dirname);
  tempde=FilesFind(st);
  Free(st);
  Free(dirname);
  doc->user_put_key=&JBPutKey;
  tempde1=tempde;
  DocPrint(doc,
  "Key: $$FG,GREEN$$Graphics $$FG,BLUE$$Words $$FG,RED$$No Nothing "
  "$$FG,BLACK$$Incomplete $$FG,CYAN$$Special$$FG$$\n\n"
  "$$FG,GREEN$$<DELETE>$$FG$$ to delete a song.\n"
      "Press a number to rate a song.\n");
  while (tempde1) {
    if (!(i++%5))
      DocPrint(doc,"\n");
    if (FileOcc("Play(",tempde1->full_name,"")) {
      st=StrNew(tempde1->name);
      FileExtRem(st);
      s=DocRead(tempde1->full_name);
      doc_e=s->root.next;
      while (doc_e!=s && doc_e->type_u8!=DOCT_TEXT)
	doc_e=doc_e->next;
      rating='0';
      if (doc_e!=s && doc_e->type_u8==DOCT_TEXT) {
	if ('0'<=doc_e->tag[2]<='9')
	  rating=doc_e->tag[2];
	if (StrMatch("incomplete",doc_e->tag))
	  DocPrint(doc,"$$FG,BLACK$$");
	else if (StrMatch("has graphics",doc_e->tag))
	  DocPrint(doc,"$$FG,GREEN$$");
	else if (StrMatch("has words",doc_e->tag))
	  DocPrint(doc,"$$FG,BLUE$$");
	else if (StrMatch("special",doc_e->tag))
	  DocPrint(doc,"$$FG,CYAN$$");
	else if (StrMatch("no nothing",doc_e->tag)) {
	  DocPrint(doc,"$$FG,RED$$");
	  if (FileOcc("\\0",tempde1->full_name,"")) {
	    s->cur_entry=doc_e->next;
	    s->cur_data_col=0;
	    DocEntryDel(s,doc_e);
	    DocPrint(s,"//0 has words\n");
	    DocWrite(s);
	  }
	}
	DocPrint(doc,"$$MU-UL,\"%c%-8ts\",%d$$ ",rating,st,tempde1);
	tempde1->user_data=JB_RUN_LEVEL_NULL;
      }
      DocDel(s);
      Free(st);
    }
    tempde1=tempde1->next;
  }
  DocPrint(doc,"\n$$FG,CYAN$$$$MU-UL,\"DONE\",%d$$\n",DOCM_CANCEL);
  while (TRUE) {
    if (_filename)
      tempde1=PopUpMenu(doc,DOF_INTERCEPT_TASK_END);
    else
      tempde1=PopUpMenu(doc);
    if (task)
      Kill(task);
    if (tempde1<0) break;
    st2=StrNew(tempde1->name);
    if (_filename) {
      Free(*_filename);
      *_filename=StrNew(tempde1->full_name);
    }
    if (tempde1->user_data==JB_RUN_LEVEL_ONE) break; //<ESC>
    tempde1->user_data=JB_RUN_LEVEL_NULL; //Rst from <SPACE BAR>
    FileExtRem(st2);
    st=MPrint("ExeFile(\"%s\");",tempde1->full_name);
    MusicSettingsRst;
    task=SingleSong(st,st2);
    Free(st2);
    Free(st);
  }
  DocDel(doc);
  DirLstDel(tempde);
}
