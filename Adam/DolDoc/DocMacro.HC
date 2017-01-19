#help_index "DolDoc/Misc"

U8 captured_macro_name[STR_LEN];
StrCpy(captured_macro_name,"Test");

I64 sys_macro_repeat_n=1;

U0 SysMacroStripKey(CJob *macro_head,I64 arg1,I64 arg2)
{
  CJob *tmpc,*tmpc1;
  tmpc=macro_head->next;
  while (tmpc!=macro_head) {
    tmpc1=tmpc->next;
    if (tmpc->job_code==JOBT_MSG &&
	  (tmpc->msg_code==MSG_KEY_DOWN || tmpc->msg_code==MSG_KEY_UP ||
	  tmpc->msg_code==MSG_KEY_DOWN_UP) &&
	  arg1 && tmpc->aux1==arg1 || !arg1 && tmpc->aux2==arg2) {
      QueRem(tmpc);
      JobDel(tmpc);
    }
    tmpc=tmpc1;
  }
}

#define MT_NULL	0
#define MT_MSG	1
#define MT_CHAR	2

class CMacroTmp
{
  CMacroTmp *next,*last;
  I64 type;
  U8 buf[STR_LEN];
};

CMacroTmp *Cmd2MT(CJob *tmpc)
{
  U8 buf[8];
  CMacroTmp *tmpmt=CAlloc(sizeof(CMacroTmp));
  if (Bt(char_bmp_macro,tmpc->aux1) && tmpc->msg_code==MSG_KEY_DOWN) {
    tmpmt->type=MT_CHAR;
    buf[0]=tmpc->aux1;
    buf[1]=0;
    StrPrint(tmpmt->buf,"%Q",buf);
  } else {
    tmpmt->type=MT_MSG;
    StrPrint(tmpmt->buf,"Msg(0x%X,0x%X,0x%X);",
	  tmpc->msg_code,tmpc->aux1,tmpc->aux2);
  }
  return tmpmt;
}

U8 *SysMacro2Str(CJob *macro_head)
{
  CJob *tmpc;
  I64 cnt=1; //terminating zero
  U8 *ptr,*m;
  CMacroTmp *tmpmt,*tmpmt1,head;
  LBtr(&sys_semas[SEMA_RECORD_MACRO],0);

  QueInit(&head);
  head.type=MT_NULL;
  tmpc=macro_head->next;
  while (tmpc!=macro_head) {
    tmpmt=Cmd2MT(tmpc);
    QueIns(tmpmt,head.last);
    cnt+=StrLen(tmpmt->buf);
    if (tmpmt->type==MT_CHAR) {
      if (tmpmt->last->type!=MT_CHAR)
	cnt+=StrLen("\"");
      if (tmpmt->next->type!=MT_CHAR)
	cnt+=StrLen("\";");
    }
    tmpc=tmpc->next;
  }

  m=MAlloc(cnt);
  ptr=m;

  tmpmt=head.next;
  while (tmpmt!=&head) {
    tmpmt1=tmpmt->next;
    if (tmpmt->type==MT_MSG) {
      StrCpy(ptr, tmpmt->buf);
      ptr+=StrLen(tmpmt->buf);
    } else {
      if (tmpmt->last->type!=MT_CHAR) {
	StrCpy(ptr, "\"");
	ptr+=StrLen("\"");
      }
      StrCpy(ptr,tmpmt->buf);
      ptr+=StrLen(tmpmt->buf);
      if (tmpmt->next->type!=MT_CHAR) {
	StrCpy(ptr, "\";");
	ptr+=StrLen("\";");
      }
    }
    Free(tmpmt);
    tmpmt=tmpmt1;
  }
  *ptr=0;
  return m;
}

U0 PlaySysMacro(I64 n=1)
{
  CTask *task=sys_focus_task;
  U8 *m;
  if (TaskValidate(task)) {
    LBtr(&sys_semas[SEMA_RECORD_MACRO],0);
    m=SysMacro2Str(&sys_macro_head);
    while (n-- && TaskValidate(task)) {
      if (task==Fs)
	InStr("%s",m);
      else
	XTalkStrWait(task,"%s",m);
    }
    Free(m);
  }
}

U0 EdInsCapturedMacro()
{
  U8 *st=SysMacro2Str(&sys_macro_head);
  if (sys_focus_task) {
    XTalk(sys_focus_task,"$$MA+LIS,T=\"%s\",LM=\"%$$Q\"$$",
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
  I64 res=0;
  U8 buf[STR_LEN];
  CJob *tmpc;
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
  doc->flags|=DOCF_SIZE_MIN | DOCF_FORM;
  StrPrint(buf,"DocMenu(%d);",doc);
  sys_macro_task=Spawn(&SrvCmdLine,NULL,"Macro Popup",,Fs);
  Fs->popup_task=sys_macro_task;
  LBts(&sys_macro_task->display_flags,DISPLAYf_WIN_ON_TOP);
  tmpc=TaskExe(sys_macro_task,Fs,buf,
	1<<JOBf_WAKE_MASTER|1<<JOBf_FOCUS_MASTER);
  JobResScan(tmpc,&res);
  Fs->popup_task=NULL;
  Kill(sys_macro_task);
  sys_macro_task=NULL;
  DocDataScan(doc,doc_e);
  DocDel(doc);
  return res;
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
	LBtr(&sys_semas[SEMA_RECORD_MACRO],0);
	QueDel(&sys_macro_head,TRUE);
	LBts(&sys_semas[SEMA_RECORD_MACRO],0);
	break;
      case SM_PLAY:
	PlaySysMacro;
	break;
      case SM_REPEAT_N:
	PlaySysMacro(sys_macro_repeat_n);
	break;
      case SM_STOP:
	LBtr(&sys_semas[SEMA_RECORD_MACRO],0);
	break;
      case SM_INS:
	LBtr(&sys_semas[SEMA_RECORD_MACRO],0);
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
