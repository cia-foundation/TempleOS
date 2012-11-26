#help_index "Doc/Output"
U0 CDirFileDoc(CDoc *doc,CDirEntry *tempm)
{
  CDocEntry *doc_e;
  while (tempm) {
    if (tempm->attr & _ATTR_DIR) {
      doc_e=DocPrintF(doc,"$$TR,\"%s\"$$",tempm->name);
      doc_e->user_data=tempm;
      tempm->user_data=doc_e;
      DocPutS(doc,"\n$$ID,+2$$");
      if (tempm->sub)
	CDirFileDoc(doc,tempm->sub);
      DocPutS(doc,"$$ID,-2$$");
    } else {
      doc_e=DocPrintF(doc,"$$MU-LE,\"%s\"$$",
	tempm->name);
      doc_e->user_data=tempm;
      tempm->user_data=doc_e;
      DocPutS(doc,"\n");
    }
    tempm=tempm->next;
  }
}

#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"

class CFMUncollapsedList
{
  CFMUncollapsedList *next;
  U8 *name;
};

CFMUncollapsedList *FMCollectUncollapsedList(CDoc *doc)
{
  CDocEntry *doc_e=doc->root.next;
  CFMUncollapsedList *result=NULL,*tempc;
  CDirEntry *tempm;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      if (!(doc_e->flags1&DOCEF1_CHECKED_COLLAPSED)) {
	if (tempm=doc_e->user_data) {
	  tempc=MAlloc(sizeof(CFMUncollapsedList));
	  tempc->next=result;
	  result=tempc;
	  tempc->name=StrNew(tempm->full_name);
	}
      }
    }
    doc_e=doc_e->next;
  }
  return result;
}

U0 FMMarkUncollapsed(CDoc *doc,CFMUncollapsedList *tempc,U8 *cur_entry,U8 *next_entry)
{
  CDocEntry *doc_e=doc->root.next;
  CFMUncollapsedList *tempc1;
  CDirEntry *tempm;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      tempm=doc_e->user_data;
      tempc1=tempc;
      while (tempc1) {
	if (!StrCmp(tempc1->name,tempm->full_name)) {
	  doc_e->flags1&=~DOCEF1_CHECKED_COLLAPSED;
	  break;
	}
	tempc1=tempc1->next;
      }
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tempm->full_name,StrLen(tempm->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tempm->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,tempm->full_name,StrLen(tempm->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tempm->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    } else if (doc_e->type_u8==DOCT_MENU_VAL) {
      tempm=doc_e->user_data;
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tempm->full_name,StrLen(tempm->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tempm->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,tempm->full_name,StrLen(tempm->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tempm->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    }
    doc_e=doc_e->next;
  }
}

U0 FMDelUncollapsedList(CFMUncollapsedList *tempc)
{
  CFMUncollapsedList *tempc1;
  while (tempc) {
    tempc1=tempc->next;
    Free(tempc->name);
    Free(tempc);
    tempc=tempc1;
  }
}

CDirEntry *FMRebuildDocDrv(U8 drv_let,CDoc *doc,CDirEntry **_root,Bool init)
{
  CDocEntry *doc_e;
  CDirEntry *tempm,*tempm1;
  U8 *st;
  tempm=CAlloc(sizeof(CDirEntry));
  tempm->full_name=MSPrintF("%C:/",drv_let);
  tempm->attr=_ATTR_DIR;
  st=MSPrintF("%c:/*",drv_let);
  if (init)
    tempm->sub=tempm1=FilesFind(st,1<<FUf_RECURSE);
  else
    tempm1=NULL;
  Free(st);
  doc_e=DocPrintF(doc,"$$TR,\"%s\"$$",tempm->full_name);
  doc_e->user_data=tempm;
  tempm->user_data=doc_e;
  tempm->next=*_root;
  *_root=tempm;
  DocPutS(doc,"\n$$ID,+2$$");
  DocBottom(doc);
  if (init) {
    CDirFileDoc(doc,tempm1);
    while (tempm1) {
      tempm1->parent=tempm;
      tempm1=tempm1->next;
    }
  }
  DocPutS(doc,"$$ID,-2$$");
  return tempm;
}

U0 FMRebuildDoc(CDoc **_doc,CDirEntry **_root,I64 mode)
{
  CPrt *p;
  I64 i;
  CDoc *doc=*_doc,*doc2=sys_clipboard_doc,*parent_doc;
  CFMUncollapsedList *tempc=NULL;
  U8 *cur_entry=NULL,*next_entry=NULL;
  CDocEntry *doc_ce;
  CDirEntry *tempm,*tempm1,*cur_tree_entry;
  if (!doc)
    parent_doc=DocCur;
  else {
    parent_doc=doc->parent_doc;
    DocQueRem(doc);
    DocUnlock(doc);
    WinMgrSync;
    DocLock(doc);
    cur_tree_entry=NULL;
    doc_ce=doc->cur_entry;
    if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
      cur_tree_entry=doc_ce->user_data;
    if (cur_tree_entry)
      cur_entry=StrNew(cur_tree_entry->full_name);
    tempm=NULL;
    if (doc_ce!=doc)
      doc_ce=doc_ce->next;
    while (doc_ce!=doc) {
      if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
	tempm=doc_ce->user_data;
      else
	tempm=NULL;
      if (tempm) {
	tempm1=tempm->parent;
	while (tempm1) {
	  if (tempm1==cur_tree_entry) {
	    tempm=NULL;
	    break;
	  } else
	    tempm1=tempm1->parent;
	}
	if (tempm)
	  break;
      }
      doc_ce=doc_ce->next;
    }
    if (tempm)
      next_entry=StrNew(tempm->full_name);

    tempc=FMCollectUncollapsedList(doc);
    DocDel(doc);
  }
  if (*_root) {
    CDirListDel(*_root);
    *_root=NULL;
  }
  doc=DocNew;
  StrCpy(doc->desc,"FileMan");
  doc->parent_doc=parent_doc;
  doc->flags|=DOCF_FORM;
  switch (mode) {
    case FM_NORMAL:
      DocPutS(doc,"$$FG,PURPLE$$File Manager$$FG$$\n\n$$LK,\"Click for Help\",\"FI:::/SparrowOS/Doc/FileMgr.TXT.Z\"$$\n\n");
      break;
    case FM_PICK_FILE:
      DocPutS(doc,"$$FG,PURPLE$$Pick file and press <ESC>$$FG$$\n\n");
      doc->flags|=DOCF_MIN_SIZE;
      break;
    case FM_PICK_DIR:
      DocPutS(doc,"$$FG,PURPLE$$Pick directory and press <ESC>$$FG$$\n\n");
      doc->flags|=DOCF_MIN_SIZE;
      break;
  }
  for (i=0;i<NUM_PRTS;i++) {
    p=&sys_prts[i];
    if (p->type==PT_NATIVE||p->type==PT_FAT32)
      FMRebuildDocDrv(Prt2Drv(p),doc,_root,TRUE);
    else if (p->type==PT_ISO9660) {
      if (p->bdev->flags&BDF_INITIALIZED)
	tempm=FMRebuildDocDrv(Prt2Drv(p),doc,_root,TRUE);
      else {
	tempm=FMRebuildDocDrv(Prt2Drv(p),doc,_root,FALSE);
	tempm->flags|=DEF_NOT_INITIALIZED;
      }
      tempm->flags|=DEF_REMOVABLE;
    }
  }
  DocHome(doc);
  FMMarkUncollapsed(doc,tempc,cur_entry,next_entry);
  DocCenter(doc);
  DocRst(doc2,TRUE);
  FMDelUncollapsedList(tempc);
  Free(cur_entry);
  Free(next_entry);
  *_doc=doc;
  DocLock(doc);
  DocQueRem(DocCur);
  DocQueIns(doc,Fs->last_doc);
}

U0 FMRename(CDoc *doc)
{
  CEdFileName fn;
  CDocEntry *doc_e=doc->cur_entry;
  CDirEntry *tempm=NULL,*parent;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tempm=doc_e->user_data;
    if (parent=tempm->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tempm->name);
      if (DocFormDo(fn.name,"CEdFileName")) {
	Silent(ON);
	Move(tempm->name,fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tempm=doc_e->user_data;
    if (parent=tempm->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tempm->name);
      if (DocFormDo(fn.name,"CEdFileName")) {
	if (StrCmp(tempm->name,fn.name)) {
	  Silent(ON);
	  if (CopyTree(tempm->name,fn.name))
	    DelTree(tempm->name);
	  Silent(OFF);
	}
      }
    }
  }
}

U0 FMMkDir(CDoc *doc)
{
  CEdFileName fn;
  CDocEntry *doc_e=doc->cur_entry;
  CDirEntry *tempm=NULL,*parent;
  *fn.name=0;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tempm=doc_e->user_data;
    if (parent=tempm->parent) {
      Cd(parent->full_name);
      if (DocFormDo(fn.name,"CEdFileName")) {
	Silent(ON);
	MkDir(fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tempm=doc_e->user_data;
    Cd(tempm->full_name);
    if (DocFormDo(fn.name,"CEdFileName")) {
      Silent(ON);
      MkDir(fn.name);
      Silent(OFF);
    }
  }
}

U0 FMDelete(CDoc *doc)
{
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempm;
  if (doc_ce->type_u8==DOCT_MENU_VAL) {
    tempm=doc_ce->user_data;
    Silent(ON);
    st=MSPrintF("Delete: %s",tempm->full_name);
    if (PopUpCancelOk(st))
      Del(tempm->full_name);
    Free(st);
    Silent(OFF);
  } else if (doc_ce->type_u8==DOCT_TREE) {
    tempm=doc_ce->user_data;
    Silent(ON);
    st=MSPrintF("Delete: %s",tempm->full_name);
    if (PopUpCancelOk(st))
      DelTree(tempm->full_name);
    Free(st);
    Silent(OFF);
  }
}

U0 FMMount(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempm;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempm=doc_ce->user_data;
  else
    tempm=NULL;
  if (tempm) {
    while (tempm->parent)
      tempm=tempm->parent;
    Silent(ON);
    ChgDsk(*tempm->full_name);
    Silent(OFF);
  }
}

U0 FMFmtDrv(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempm;
  U8 *st=NULL;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempm=doc_ce->user_data;
  else
    tempm=NULL;
  if (tempm) {
    while (tempm->parent)
      tempm=tempm->parent;
    st=MSPrintF("Format Drive '%c'?\nAre You Sure?\n",*tempm->full_name);
    if (PopUpCancelOk(st)) {
      Silent(ON);
      Fmt(*tempm->full_name,TRUE,FALSE);
      Silent(OFF);
    }
  }
  Free(st);
}

U0 FMMakeISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempm;
  U8 *st;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempm=doc_ce->user_data;
  else
    tempm=NULL;
  if (tempm) {
    if (doc_ce->type_u8==DOCT_MENU_VAL)
      tempm=tempm->parent;
    if (tempm && *tempm->full_name) {
      Silent(ON);
      if (tempm->full_name[StrLen(tempm->full_name)-1]=='/')
	st=MSPrintF("%s*",tempm->full_name);
      else
	st=MSPrintF("%s/*",tempm->full_name);
      CDFileCreate(,st);
      Free(st);
      Silent(OFF);
    }
  }
}

U0 FMBurnISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempm;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempm=doc_ce->user_data;
  else
    tempm=NULL;
  if (tempm) {
    while (tempm->parent)
      tempm=tempm->parent;
    Silent(ON);
    CDImageWrite(*tempm->full_name);
    Silent(OFF);
  }
}

U0 FMCopy(CDoc *doc)
{
  CDoc *doc2=sys_clipboard_doc;
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tempm,*tempm1=NULL,*tempm2;
  Bool unlock_doc2=DocLock(doc2);
  doc_e=doc2->root.next;

  tempm1=doc_ce->user_data;
  if (doc_ce->type_u8==DOCT_MENU_VAL)
    tempm1=tempm1->parent;
  else if (doc_ce->type_u8!=DOCT_TREE)
    tempm1=NULL;
  if (tempm1) {
    while (doc_e!=doc2) {
      if (doc_e->type_u8==DOCT_MENU_VAL) {
	tempm=doc_e->user_data;
	tempm->flags|=DEF_PROCESSED;
	tempm2=tempm->parent;
	if (!tempm2 || !(tempm2->flags&DEF_PROCESSED)) {
	  Silent(ON);
	  Copy(tempm->full_name,tempm1->full_name);
	  Silent(OFF);
	}
      } else if (doc_e->type_u8==DOCT_TREE) {
	tempm=doc_e->user_data;
	tempm->flags|=DEF_PROCESSED;
	tempm2=tempm->parent;
	if (!tempm2 || !(tempm2->flags&DEF_PROCESSED)) {
	  Silent(ON);
	  if (*tempm1->name)
	    st=MSPrintF("%s/%s",tempm1->full_name,tempm->name);
	  else
	    st=MSPrintF("%s%s",tempm1->full_name,tempm->name);
	  CopyTree(tempm->full_name,st);
	  Free(st);
	  Silent(OFF);
	}
      }
      doc_e=doc_e->next;
    }
  }
  if (unlock_doc2)
    DocUnlock(doc2);
}

#define FMR_INCLUDE		0
#define FMR_ADAM_INCLUDE	1
#define FMR_DELETE		2
#define FMR_RENAME		3
#define FMR_MKDIR		4
#define FMR_PLAIN		5
#define FMR_PASTE		6
#define FMR_CHG_DSK		7
#define FMR_FORMAT		8
#define FMR_MAKE_ISO		9
#define FMR_BURN_ISO		10
#define FMR_HELP		11

I64 PopUpCancelPasteDel(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPutS(doc,header);
  DocPutS(doc,"$$CM+LX,1,1 $$$$BT,\"INCLUDE               \",FMR_INCLUDE$$"
	"$$CM+LX,27,0$$$$BT,\"ADAM INCLUDE          \",FMR_ADAM_INCLUDE$$"
	"$$CM+LX,1,3 $$$$BT,\"DELETE                \",FMR_DELETE$$"
	"$$CM+LX,27,0$$$$BT,\"RENAME                \",FMR_RENAME$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE DIRECTORY        \",FMR_MKDIR$$"
	"$$CM+LX,27,0$$$$BT,\"PLAIN-TEXT EDIT       \",FMR_PLAIN$$"
	"$$CM+LX,1,3 $$$$BT,\"PASTE CLIPBOARD FILES \",FMR_PASTE$$"
	"$$CM+LX,27,0$$$$BT,\"CHANGE DISK(MOUNT IT) \",FMR_CHG_DSK$$"
	"$$CM+LX,1,3 $$$$BT,\"FORMAT                \",FMR_FORMAT$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE ISO (CD-ROM) FILE\",FMR_MAKE_ISO$$"
	"$$CM+LX,27,0$$$$BT,\"BURN ISO (CD-ROM) FILE\",FMR_BURN_ISO$$"
	"$$CM+LX,1,3 $$$$BT,\"HELP                  \",FMR_HELP$$"
	"$$CM+LX,27,0$$$$BT,\"CANCEL                \",DOCM_CANCEL$$\n");
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 FMRightClick()
{
  switch (PopUpCancelPasteDel) {
    case FMR_INCLUDE:
      Msg(MSG_KEY_DOWN,0,0x3F0000003F);
      break;
    case FMR_ADAM_INCLUDE:
      Msg(MSG_KEY_DOWN,0,0x23F0000023F);
      break;
    case FMR_DELETE:
      Msg(MSG_KEY_DOWN,CH_CTRLY,0);
      break;
    case FMR_RENAME:
      Msg(MSG_KEY_DOWN,'r',0);
      break;
    case FMR_MKDIR:
      Msg(MSG_KEY_DOWN,'d',0);
      break;
    case FMR_PLAIN:
      Msg(MSG_KEY_DOWN,CH_SHIFT_SPACE,0);
      break;
    case FMR_PASTE:
      Msg(MSG_KEY_DOWN,0,SC_INS+SCF_SHIFT);
      break;
    case FMR_CHG_DSK:
      Msg(MSG_KEY_DOWN,'m',0);
      break;
    case FMR_FORMAT:
      Msg(MSG_KEY_DOWN,'f',0);
      break;
    case FMR_MAKE_ISO:
      Msg(MSG_KEY_DOWN,'M',0);
      break;
    case FMR_BURN_ISO:
      Msg(MSG_KEY_DOWN,'B',0);
      break;
    case FMR_HELP:
      Msg(MSG_KEY_DOWN,CH_CTRLM,0x43200000432);
      break;
  }
}


U8 *fm_ip_str=NULL;
U0 (*fp_old_final_screen_update)(CDC *dc);

U0 FMFinalScreenUpdate(CDC *dc)
{
  if (fm_ip_str) {
    dc->color=LTRED;
    GrPutS(dc,ipx,ipy,fm_ip_str);
  }
  (*fp_old_final_screen_update)(dc);
}

public U8 *FileMgr(I64 mode=FM_NORMAL,CTask *mem_task=NULL)
{
  CDirEntry *root=NULL,*tempm,*tempm1,*tempm2;
  I64 sc,ch,p1,p2,msg_code;
  CDoc *doc=NULL,*old_doc=DocCur;
  U8 *result=NULL,*st,*st2,*old_cur_dir=CurDir;
  CDocEntry *doc_ce=NULL,*doc_e;
  Bool okay;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  fp_old_final_screen_update=fp_final_screen_update;
  MenuFilePush("::/SparrowOS/Doc/FileMgrPullDown.TXT.Z");
  FMRebuildDoc(&doc,&root,mode);
  if (tempm1=CdDir(root,old_cur_dir))
    doc->cur_entry=tempm1->user_data;
  while (tempm1) {
    if (tempm1->attr&_ATTR_DIR)
      tempm1->user_data(CDocEntry *)->flags1&=~DOCEF1_CHECKED_COLLAPSED;
    tempm1=tempm1->parent;
  }
  do {
    DocUnlock(doc);
    do msg_code=GetMsg(&p1,&p2,
	1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|
	1<<MSG_IP_R_UP);
    while (Fs!=sys_focus_task);
    DocLock(doc);
    switch (msg_code) {
      case MSG_IP_R_UP:
	DocUnlock(doc);
	FMRightClick;
	DocLock(doc);
	break;
      case MSG_IP_L_DOWN:
	doc_ce=doc->cur_entry;
	fm_ip_str=doc_ce->tag;
	fp_final_screen_update=&FMFinalScreenUpdate;
	break;
      case MSG_IP_L_UP:
	if (doc_ce) {
	  fp_final_screen_update=fp_old_final_screen_update;
	  if (SetCursorPos(Fs,p1/FONT_WIDTH+Fs->win_left+Fs->win_scroll_x/FONT_WIDTH,p2/FONT_HEIGHT+Fs->win_top+Fs->win_scroll_y/FONT_HEIGHT,TRUE)) {
	    doc_e=doc->cur_entry;
	    if (doc_e!=doc_ce) {
	      st2=NULL;
	      if (doc_e->type_u8==DOCT_MENU_VAL) {
		tempm1=doc_e->user_data;
		if (tempm1=tempm1->parent)
		  st2=StrNew(tempm1->full_name);
	      } else if (doc_e->type_u8==DOCT_TREE) {
		tempm1=doc_e->user_data;
		st2=StrNew(tempm1->full_name);
	      }
	      if (st2 && doc_ce->type_u8==DOCT_MENU_VAL) {
		tempm=doc_ce->user_data;
		Silent(ON);
		Move(tempm->full_name,st2);
		Silent(OFF);
		FMRebuildDoc(&doc,&root,mode);
	      } else if (st2 && doc_ce->type_u8==DOCT_TREE) {
		tempm=doc_ce->user_data;
		okay=TRUE;
		tempm2=tempm1;
		while (tempm2) {
		  if (tempm2!=tempm)
		    tempm2=tempm2->parent;
		  else {
		    okay=FALSE;
		    break;
		  }
		}
		if (okay) {
		  if (*tempm1->name)
		    st=MSPrintF("%s/%s",tempm1->full_name,tempm->name);
		  else
		    st=MSPrintF("%s%s",tempm1->full_name,tempm->name);
		  if (StrCmp(tempm->full_name,st)) {
		    Silent(ON);
		    CopyTree(tempm->full_name,st);
		    DelTree(tempm->full_name);
		    Silent(OFF);
		    FMRebuildDoc(&doc,&root,mode);
		  }
		  Free(st);
		}
	      }
	      Free(st2);
	      FlushMsgs;
	    } else
	      if (doc_e->type_u8==DOCT_MENU_VAL) {
		DocUnlock(doc);
		Ed(doc_e->user_data(CDirEntry *)->full_name);
		DocLock(doc);
	      }
	    doc_ce=NULL;
	  }
	}
	break;
      case MSG_KEY_DOWN:
	doc_ce=NULL;
	ch=p1; sc=p2;
	if (sc.u8[0]==SC_DELETE && !(sc&(SCF_SHIFT|SCF_CTRL)))
	  ch=CH_CTRLY;
	if (ch && sc&SCF_ALT) goto regular_key;
	switch (ch) {
	  case CH_NEW_LINE:
	    DocUnlock(doc);
	    FMRightClick;
	    DocLock(doc);
	    break;
	  case CH_CTRLV:
	    FMCopy(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'r':
	    DocUnlock(doc);
	    FMRename(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'd':
	    DocUnlock(doc);
	    FMMkDir(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case CH_CTRLY:
	    DocUnlock(doc);
	    FMDelete(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'm':
	    DocUnlock(doc);
	    FMMount(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'M':
	    DocUnlock(doc);
	    FMMakeISO(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'B':
	    DocUnlock(doc);
	    FMBurnISO(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case 'f':
	    DocUnlock(doc);
	    FMFmtDrv(doc);
	    FMRebuildDoc(&doc,&root,mode);
	    break;
	  case CH_SHIFT_ESC:
	    break;
	  case CH_SPACE:
	    if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
	      DocUnlock(doc);
	      Ed(doc->cur_entry->user_data(CDirEntry *)->full_name);
	      DocLock(doc);
	    } else
	      goto regular_key;
	    break;
	  case CH_SHIFT_SPACE:
	    if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
	      DocUnlock(doc);
	      Plain(doc->cur_entry->user_data(CDirEntry *)->full_name);
	      DocLock(doc);
	    } else
	      goto regular_key;
	    break;
	  case CH_ESC:
	    doc_ce=doc->cur_entry;
	    tempm=doc_ce->user_data;
	    if (mode==FM_PICK_FILE && doc_ce->type_u8==DOCT_MENU_VAL)
	      result=StrNew(tempm->full_name,mem_task);
	    else if (mode==FM_PICK_DIR) {
	      if (doc_ce->type_u8==DOCT_TREE)
		result=StrNew(tempm->full_name,mem_task);
	      else if (doc_ce->type_u8==DOCT_MENU_VAL && (tempm=tempm->parent))
		result=StrNew(tempm->full_name,mem_task);
	    }
	    break;
	  default:
	    if (sc.u8[0]==SC_INS && sc&SCF_SHIFT && !(sc&SCF_CTRL)) {
	      FMCopy(doc);
	      FMRebuildDoc(&doc,&root,mode);
	    } else if (sc.u8[0]==SC_F5) {
	      if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
		tempm=doc->cur_entry->user_data;
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  AdamFile(tempm->full_name);
		else
		  PopUpFile(tempm->full_name);
		DocLock(doc);
	      }
	    } else {
regular_key:
	      DocUnlock(doc);
	      PutKey(ch,sc);
	      DocLock(doc);
	    }
	}
	break;
    }
  } while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  fp_final_screen_update=fp_old_final_screen_update;
  DocQueRem(doc);
  DocQueIns(old_doc,Fs->last_doc);
  SettingsPop;
  DocDel(doc);
  CDirListDel(root);
  Cd(old_cur_dir);
  Free(old_cur_dir);
  if (mode!=FM_NORMAL && !result)
    result=StrNew("",mem_task);
  MenuPop;
  return result;
}
#help_index ""
