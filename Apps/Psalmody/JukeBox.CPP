CTask *SingleSong(U8 *msg,U8 *name)
{
  CTask *task=Spawn(&SrvCmdLine,NULL,name,,Fs);
  StrCpy(task->task_title,name);
  task->title_src=TTS_LOCKED_CONST;
  TaskExe(task,Fs,";",1<<SVCf_WAKE_MASTER|1<<SVCf_FREE_ON_COMPLETE);
  WinHorz(task->win_left,task->win_left+50,task);
  WinVert(2,2+8,task);
  TaskExe(task,NULL,msg,1<<SVCf_EXIT_ON_COMPLETE|1<<SVCf_FREE_ON_COMPLETE);
  DocPut(task)->max_entries=100;
  return task;
}

#define JB_RUN_LEVEL_NULL	0
#define JB_RUN_LEVEL_ONE	1
#define JB_RUN_LEVEL_TWO	2

Bool JBPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tempde;
  CDoc *s;
  if (!(sc&(SCF_ALT|SCF_CTRL|SCF_SHIFT))
	&& doc_ce->type_u8==DOCT_MENU_VAL && doc_ce->left_exp>=0) {
    tempde=doc_ce->left_exp;
    if (sc.u8[0]==SC_DELETE ) {
      Beep;
      Silent;
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
//<SPACE> is followed by <ESC> --> JB_RUN_LEVEL_TWO
      //Actual <ESC> just exits          --> JB_RUN_LEVEL_ONE
  }
  return FALSE;
}

public U0 JukeBox(U8 *dirname="::/Home/Psalmody",U8 **_filename=NULL)
{//_filename is for using this as a song-chooser program.
  I64 i=0,rating;
  U8 *st,*st2;
  CDirEntry *tempde,*tempde1;
  CDoc *doc=DocNew,*s;
  CDocEntry *doc_e;
  CTask *task=NULL;
  if (_filename)
    *_filename=NULL;
  dirname=StrNew(dirname);
  st=MStrPrint("%s/*.CPP.Z",dirname);
  tempde=tempde1=FilesFind(st);
  Free(st);
  Free(dirname);
  doc->user_put_key=&JBPutKey;
  DocPrint(doc,
	"Key: $$GREEN$$Graphics $$BLUE$$Words $$RED$$No Nothing "
	"$$BLACK$$Incomplete $$CYAN$$Special$$FG$$\n\n"
	"$$GREEN$$<DELETE>$$FG$$ to delete a song.\n"
	"Press a number to rate a song.\n");
  while (tempde) {
    if (!(i++%5))
      DocPrint(doc,"\n");
    if (FileOcc("Play(",tempde->full_name,"")) {
      st=StrNew(tempde->name);
      FileExtRem(st);
      s=DocRead(tempde->full_name);
      doc_e=s->root.next;
      while (doc_e!=s && doc_e->type_u8!=DOCT_TEXT)
	doc_e=doc_e->next;
      rating='0';
      if (doc_e!=s && doc_e->type_u8==DOCT_TEXT) {
	if ('0'<=doc_e->tag[2]<='9')
	  rating=doc_e->tag[2];
	if (StrMatch("incomplete",doc_e->tag))
	  DocPrint(doc,"$$BLACK$$");
	else if (StrMatch("has graphics",doc_e->tag))
	  DocPrint(doc,"$$GREEN$$");
	else if (StrMatch("has words",doc_e->tag))
	  DocPrint(doc,"$$BLUE$$");
	else if (StrMatch("special",doc_e->tag))
	  DocPrint(doc,"$$CYAN$$");
	else if (StrMatch("no nothing",doc_e->tag)) {
	  DocPrint(doc,"$$RED$$");
	  if (FileOcc("\\0",tempde->full_name,"")) {
	    s->cur_entry=doc_e->next;
	    s->cur_col=0;
	    DocEntryDel(s,doc_e);
	    DocPrint(s,"//0 has words\n");
	    DocWrite(s);
	  }
	}
	DocPrint(doc,"$$MU-UL,\"%c%-8ts\",LE=%d$$ ",rating,st,tempde);
	tempde->user_data=JB_RUN_LEVEL_NULL;
      }
      DocDel(s);
      Free(st);
    }
    tempde=tempde->next;
  }
  DocPrint(doc,"\n$$CYAN$$$$MU-UL,\"DONE\",LE=%d$$\n",DOCM_CANCEL);
  while (TRUE) {
    if (_filename)
      tempde=PopUpMenu(doc,DOF_INTERCEPT_TASK_END);
    else
      tempde=PopUpMenu(doc);
    if (task)
      Kill(task);
    if (tempde<=0) break;
    st2=StrNew(tempde->name);
    if (_filename) {
      Free(*_filename);
      *_filename=StrNew(tempde->full_name);
    }
    if (tempde->user_data==JB_RUN_LEVEL_ONE) break; //<ESC>
    tempde->user_data=JB_RUN_LEVEL_NULL; //Rst from <SPACE>
    FileExtRem(st2);
    st=MStrPrint("ExeFile(\"%s\");",tempde->full_name);
    MusicSettingsRst;
    task=SingleSong(st,st2);
    Free(st2);
    Free(st);
  }
  DocDel(doc);
  DirTreeDel(tempde1);
}
