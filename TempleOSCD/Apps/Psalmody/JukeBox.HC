CTask *JukeReward(U8 *msg)
{
  U8 *buf;
  CDoc *doc;
  CTask *res=Spawn(&SrvCmdLine,NULL,"Reward",,Fs);
  StrCpy(res->task_title,"Reward");
  res->title_src=TTS_LOCKED_CONST;

  doc=DocNew(,res);
  DocPrint(doc,"$$WW+H,1$$$$RED$$%s",msg);

  buf=MStrPrint("DocEd(0x%X);",doc);
  TaskExe(res,NULL,buf,1<<JOBf_EXIT_ON_COMPLETE|1<<JOBf_FREE_ON_COMPLETE);
  Free(buf);
  TaskWait(res);

  res->border_src =BDS_CONST;
  res->border_attr=LTGRAY<<4+DrvTextAttrGet(':')&15;
  res->text_attr  =LTGRAY<<4+BLUE;
  res->win_inhibit=WIG_NO_FOCUS_TASK_DFT;
  WinHorz(Fs->win_right+2,TEXT_COLS-2,res);
  WinVert(2,TEXT_ROWS-2,res);

  WinFocus(Fs->parent_task);
  return res;
}

CTask *SingleSong(U8 *msg,U8 *name)
{
  CTask *task=Spawn(&SrvCmdLine,NULL,name,,Fs);
  StrCpy(task->task_title,name);
  task->title_src=TTS_LOCKED_CONST;
  TaskExe(task,Fs,";",1<<JOBf_WAKE_MASTER|1<<JOBf_FREE_ON_COMPLETE);
  WinHorz(task->win_left,task->win_left+50,task);
  WinVert(2,2+8,task);
  task->win_inhibit=WIG_NO_FOCUS_TASK_DFT;
  TaskExe(task,NULL,msg,1<<JOBf_EXIT_ON_COMPLETE|1<<JOBf_FREE_ON_COMPLETE);
  DocPut(task)->max_entries=100;
  return task;
}

#define JB_RUN_LEVEL_NULL	0
#define JB_RUN_LEVEL_ONE	1
#define JB_RUN_LEVEL_TWO	2

Bool JBPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tmpde;
  I64 i;
  U8 *st;
  CDoc *doc2;
  if (!(sc&(SCF_ALT|SCF_CTRL|SCF_SHIFT))
	&& doc_ce->type_u8==DOCT_MENU_VAL && doc_ce->left_exp>=0) {
    tmpde=doc_ce->left_exp;
    if (sc.u8[0]==SC_DELETE ) {
      Beep;
      Silent;
      Del(tmpde->full_name);
      Silent(OFF);
    } else if ('0'<=ch<='9') {
      if (StrLen(doc_ce->tag)>1)
	doc_ce->tag[0]=ch;
      doc2=DocRead(tmpde->full_name);
      DocGoToLine(doc2,1);
      doc_e=doc2->cur_entry;
      if (doc_e!=doc2 && doc_e->type_u8==DOCT_TEXT && StrLen(doc_e->tag)>=3) {
	doc_e->tag[2]=ch;
	DocWrite(doc2);
      }
      DocDel(doc2);
      return TRUE;
    } else if (ch=='r') {
      if (!DocTreeFFind(tmpde->full_name,"Reward1")) {
	doc2=DocRead(tmpde->full_name);
	DocGoToLine(doc2,2);
	DocPrint(doc2,
	      "\n$$TR,\"Reward1\"$$\n"
	      "$$ID,2$$CallExtStr(\"JukeReward\",\"\"\n"
	      "$$TR,\"Reward2\"$$\n"
	      "$$ID,2$$\n"
	      "$$ID,-2$$\n"
	      ");\n$$ID,-2$$\n"
	      );
	DocWrite(doc2);
	DocDel(doc2);
      }
      if (DocTreeFFind(tmpde->full_name,"Reward1/Reward2")) {
	for (i=0;i<5;i++) {
	  st=MStrPrint("%s %s %s %s ",
		GodWordStr,GodWordStr,GodWordStr,GodWordStr);
	  DocTreeFAppend(tmpde->full_name,"Reward1/Reward2",
		"\"%s\"\n",st);
	  Free(st);
	}
	In(" ");
      }
      return TRUE;
    } else if (ch==CH_SPACE||ch==CH_ESC)
      tmpde->user_data++; //JB_RUN_LEVEL++
//<SPACE> is followed by <ESC> --> JB_RUN_LEVEL_TWO
      //Actual <ESC> just exits      --> JB_RUN_LEVEL_ONE
  }
  return FALSE;
}

public U0 JukeBox(U8 *dirname="~/Psalmody",U8 **_filename=NULL)
{//_filename is for using this as a song-chooser program.
  I64 i=0,rating;
  U8 *st,*st2;
  CDirEntry *tmpde,*tmpde1;
  CDoc *doc=DocNew,*s;
  CDocEntry *doc_e;
  CTask *task=NULL;
  if (_filename)
    *_filename=NULL;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  try {
    dirname=StrNew(dirname);
    st=MStrPrint("%s/*.HC.Z",dirname);
    tmpde=tmpde1=FilesFind(st);
    Free(st);
    Free(dirname);
    doc->user_put_key=&JBPutKey;
    DocPrint(doc,
	  "Key: $$GREEN$$Graphics $$BLUE$$Words $$RED$$No Nothing "
	  "$$BLACK$$Incomplete $$CYAN$$Special$$FG$$\n\n"
	  "$$GREEN$$<DEL>$$FG$$\tto delete a song.\n"
	  "'$$GREEN$$0$$FG$$'-'$$GREEN$$9$$FG$$'\tto rate a song.\n"
	  "'$$GREEN$$r$$FG$$'\tto get your reward from God.\n");
    while (tmpde) {
      if (!(i++%5))
	DocPrint(doc,"\n");
      if (FileOcc("Play(",tmpde->full_name,"")) {
	st=StrNew(tmpde->name);
	FileExtRem(st);
	s=DocRead(tmpde->full_name);
	doc_e=s->head.next;
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
	    if (FileOcc("\\0",tmpde->full_name,"")) {
	      s->cur_entry=doc_e->next;
	      s->cur_col=0;
	      DocEntryDel(s,doc_e);
	      DocPrint(s,"//0 has words\n");
	      DocWrite(s);
	    }
	  }
	  DocPrint(doc,"$$MU-UL,\"%c%-8ts\",LE=%d$$ ",rating,st,tmpde);
	  tmpde->user_data=JB_RUN_LEVEL_NULL;
	}
	DocDel(s);
	Free(st);
      }
      tmpde=tmpde->next;
    }
    DocPrint(doc,"\n$$CYAN$$$$MU-UL,\"DONE\",LE=%d$$\n",DOCM_CANCEL);
    while (TRUE) {
      if (_filename)
	tmpde=PopUpMenu(doc,DOF_INTERCEPT_TASK_END);
      else
	tmpde=PopUpMenu(doc);
      if (task)
	Kill(task);
      if (tmpde<=0) break;
      st2=StrNew(tmpde->name);
      if (_filename) {
	Free(*_filename);
	*_filename=StrNew(tmpde->full_name);
      }
      if (tmpde->user_data==JB_RUN_LEVEL_ONE) break; //<ESC>
      tmpde->user_data=JB_RUN_LEVEL_NULL; //Rst from <SPACE>
      FileExtRem(st2);
      st=MStrPrint("ExeFile(\"%s\");",tmpde->full_name);
      MusicSettingsRst;
      task=SingleSong(st,st2);
      Free(st2);
      Free(st);
    }
    DocDel(doc);
    DirTreeDel(tmpde1);
  } catch
    PutExcept;
  SettingsPop;
}
