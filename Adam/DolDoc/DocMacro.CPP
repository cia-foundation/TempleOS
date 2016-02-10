#help_index "DolDoc/Misc"

U8 captured_macro_name[STR_LEN];
StrCpy(captured_macro_name,"Test");

I64 sys_macro_repeat_n=1;

U0 SysMacroStripKey(CSrvCmd *macro_root,I64 a1,I64 a2)
{
  CSrvCmd *tempc,*tempc1;
  tempc=macro_root->next;
  while (tempc!=macro_root) {
    tempc1=tempc->next;
    if (tempc->cmd_code==SVCT_MSG &&
	  (tempc->msg_code==MSG_KEY_DOWN || tempc->msg_code==MSG_KEY_UP ||
	  tempc->msg_code==MSG_KEY_DOWN_UP) &&
	  a1 && tempc->aux1==a1 || !a1 && tempc->aux2==a2) {
      QueRem(tempc);
      SrvCmdDel(tempc);
    }
    tempc=tempc1;
  }
}

#define MT_NULL	0
#define MT_MSG	1
#define MT_CHAR	2

class CMacroTemp
{
  CMacroTemp *next,*last;
  I64 type;
  U8 buf[STR_LEN];
};

CMacroTemp *Cmd2MT(CSrvCmd *tempc)
{
  U8 buf[8];
  CMacroTemp *tempmt=CAlloc(sizeof(CMacroTemp));
  if (Bt(chars_bmp_macro,tempc->aux1) && tempc->msg_code==MSG_KEY_DOWN) {
    tempmt->type=MT_CHAR;
    buf[0]=tempc->aux1;
    buf[1]=0;
    StrPrint(tempmt->buf,"%Q",buf);
  } else {
    tempmt->type=MT_MSG;
    StrPrint(tempmt->buf,"Msg(0x%X,0x%X,0x%X);",
	  tempc->msg_code,tempc->aux1,tempc->aux2);
  }
  return tempmt;
}

U8 *SysMacro2Str(CSrvCmd *macro_root)
{
  CSrvCmd *tempc;
  I64 cnt=1; //terminating zero
  U8 *ptr,*m;
  CMacroTemp *tempmt,*tempmt1,root;
  LBtr(&sys_semas[SYS_SEMA_RECORD_MACRO],0);

  QueInit(&root);
  root.type=MT_NULL;
  tempc=macro_root->next;
  while (tempc!=macro_root) {
    tempmt=Cmd2MT(tempc);
    QueIns(tempmt,root.last);
    cnt+=StrLen(tempmt->buf);
    if (tempmt->type==MT_CHAR) {
      if (tempmt->last->type!=MT_CHAR)
	cnt+=StrLen("\"");
      if (tempmt->next->type!=MT_CHAR)
	cnt+=StrLen("\";");
    }
    tempc=tempc->next;
  }

  m=MAlloc(cnt);
  ptr=m;

  tempmt=root.next;
  while (tempmt!=&root) {
    tempmt1=tempmt->next;
    if (tempmt->type==MT_MSG) {
      StrCpy(ptr, tempmt->buf);
      ptr+=StrLen(tempmt->buf);
    } else {
      if (tempmt->last->type!=MT_CHAR) {
	StrCpy(ptr, "\"");
	ptr+=StrLen("\"");
      }
      StrCpy(ptr,tempmt->buf);
      ptr+=StrLen(tempmt->buf);
      if (tempmt->next->type!=MT_CHAR) {
	StrCpy(ptr, "\";");
	ptr+=StrLen("\";");
      }
    }
    Free(tempmt);
    tempmt=tempmt1;
  }
  *ptr=0;
  return m;
}

U0 PlaySysMacro(I64 n=1)
{
  CTask *task=sys_focus_task;
  U8 *m;
  if (TaskValidate(task)) {
    LBtr(&sys_semas[SYS_SEMA_RECORD_MACRO],0);
    m=SysMacro2Str(&sys_macro_root);
    while (n-- && TaskValidate(task)) {
      if (task==Fs)
	AutoStr("%s",m);
      else
	XTalkStrWait(task,"%s",m);
    }
    Free(m);
  }
}

U0 EdInsCapturedMacro()
{
  U8 *st=SysMacro2Str(&sys_macro_root);
  if (sys_focus_task) {
    XTalk(sys_focus_task,"$$MA+LA,T=\"%s\",LM=\"%Q\"$$",
	  captured_macro_name,st);
    Free(st);
  }
}

#define SM_RECORD	0
#define SM_INS		1
#define SM_PLAY		2
#define SM_REPEAT_N	3
#define SM_STOP		4

I64 PopUpMacroMenu()
{
  I64 result=0;
  U8 buf[STR_LEN];
  CSrvCmd *tempc;
  CDoc *doc=DocNew;
  CDocEntry *doc_e=DocPrint(doc,"$$DA-P,LEN=STR_LEN-1,A=\"Name:%%s\"$$");
  doc_e->data=captured_macro_name;
  DocDataFmt(doc,doc_e);

  doc_e=DocPrint(doc,"\n$$DA,A=\"Repeat N:%%d\"$$");
  doc_e->data=&sys_macro_repeat_n;
  DocDataFmt(doc,doc_e);

  DocPrint(doc,"\n"
	"$$CM+LX,1,3$$$$BT,\"RECORD\",LE=SM_RECORD$$"
	"$$CM+LX,17,0$$$$BT,\"INSERT\",LE=SM_INS$$"
	"$$CM+LX,1,3$$$$BT,\"PLAY\",LE=SM_PLAY$$"
	"$$CM+LX,17,0$$$$BT,\"REPEAT N\",LE=SM_REPEAT_N$$"
	"$$CM+LX,1,3$$$$BT,\"STOP\",LE=SM_STOP$$"
	"$$CM+LX,17,0$$$$BT,\"CANCEL\",LE=DOCM_CANCEL$$"
	"\n\n\n$$GREEN$$SHIFT-F2$$FG$$ will play macro.\n");
  doc->flags|=DOCF_MIN_SIZE | DOCF_FORM;
  StrPrint(buf,"DocMenu(%d);",doc);
  sys_macro_task=Spawn(&SrvCmdLine,NULL,"Macro Popup",,Fs);
  Fs->popup_task=sys_macro_task;
  LBts(&sys_macro_task->display_flags,DISPLAYf_WIN_ON_TOP);
  tempc=TaskExe(sys_macro_task,Fs,buf,
	1<<SVCf_WAKE_MASTER|1<<SVCf_FOCUS_MASTER);
  JobResultScan(tempc,&result);
  Fs->popup_task=NULL;
  Kill(sys_macro_task);
  sys_macro_task=NULL;
  DocDataScan(doc,doc_e);
  DocDel(doc);
  return result;
}

U0 MacroTask(I64)
{
  I64 i;
  StrCpy(captured_macro_name,"Click Here");
  sys_macro_repeat_n=1;
  do {
    i=PopUpMacroMenu;
    WinRefocus(sys_focus_task);
    switch (i) {
      case SM_RECORD:
	LBtr(&sys_semas[SYS_SEMA_RECORD_MACRO],0);
	QueDel(&sys_macro_root,TRUE);
	LBts(&sys_semas[SYS_SEMA_RECORD_MACRO],0);
	break;
      case SM_PLAY:
	PlaySysMacro;
	break;
      case SM_REPEAT_N:
	PlaySysMacro(sys_macro_repeat_n);
	break;
      case SM_STOP:
	LBtr(&sys_semas[SYS_SEMA_RECORD_MACRO],0);
	break;
      case SM_INS:
	LBtr(&sys_semas[SYS_SEMA_RECORD_MACRO],0);
	EdInsCapturedMacro;
	break;
    }
  } while (i>=0);
}

U0 EdMacroUtil()
{
  if (!sys_macro_task)
    Spawn(&MacroTask,NULL,"Macro");
}
