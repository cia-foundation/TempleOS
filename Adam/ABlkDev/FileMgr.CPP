#help_index "DolDoc/Output;StdOut/DolDoc"
U0 DirFileDoc(CDoc *doc,CDirEntry *tempde)
{
  while (tempde) {
    if (tempde->attr & RS_ATTR_DIR) {
      tempde->user_data=DocPrint(doc,"$$TR,\"%s\",U=0x%X$$",tempde->name,tempde);
      DocPrint(doc,"\n$$ID,+2$$");
      if (tempde->sub)
	DirFileDoc(doc,tempde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else {
      tempde->user_data=DocPrint(doc,"$$MU,\"%s\",U=0x%X$$",tempde->name,tempde);
      DocPrint(doc,"\n");
    }
    tempde=tempde->next;
  }
}

#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"

#define FM_NORMAL	0
#define FM_PICK_FILE	1
#define FM_PICK_DIR	2

class CFMUncollapsedLst
{
  CFMUncollapsedLst *next;
  U8 *name;
};

CFMUncollapsedLst *FMCollectUncollapsedLst(CDoc *doc)
{
  CDocEntry *doc_e=doc->root.next;
  CFMUncollapsedLst *result=NULL,*tempc;
  CDirEntry *tempde;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      if (!(doc_e->de_flags&DOCEF_CHECKED_COLLAPSED)) {
	if (tempde=doc_e->user_data) {
	  tempc=MAlloc(sizeof(CFMUncollapsedLst));
	  tempc->next=result;
	  result=tempc;
	  tempc->name=StrNew(tempde->full_name);
	}
      }
    }
    doc_e=doc_e->next;
  }
  return result;
}

U0 FMMarkUncollapsed(CDoc *doc,CFMUncollapsedLst *tempc,
	U8 *cur_entry,U8 *next_entry)
{
  CDocEntry *doc_e=doc->root.next;
  CFMUncollapsedLst *tempc1;
  CDirEntry *tempde;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      tempde=doc_e->user_data;
      tempc1=tempc;
      while (tempc1) {
	if (!StrCmp(tempc1->name,tempde->full_name)) {
	  doc_e->de_flags&=~DOCEF_CHECKED_COLLAPSED;
	  break;
	}
	tempc1=tempc1->next;
      }
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tempde->full_name,StrLen(tempde->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tempde->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,
		tempde->full_name,StrLen(tempde->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tempde->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    } else if (doc_e->type_u8==DOCT_MENU_VAL) {
      tempde=doc_e->user_data;
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tempde->full_name,StrLen(tempde->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tempde->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,
		tempde->full_name,StrLen(tempde->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tempde->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    }
    doc_e=doc_e->next;
  }
}

U0 FMDelUncollapsedLst(CFMUncollapsedLst *tempc)
{
  CFMUncollapsedLst *tempc1;
  while (tempc) {
    tempc1=tempc->next;
    Free(tempc->name);
    Free(tempc);
    tempc=tempc1;
  }
}

CDirEntry *FMRebuildDocDrv(U8 drv_let,CDoc *doc,CDirEntry **_root,Bool init)
{
  CDirEntry *tempde,*tempde1;
  U8 *st;
  tempde=CAlloc(sizeof(CDirEntry));
  tempde->full_name=MStrPrint("%C:/",drv_let);
  tempde->attr=RS_ATTR_DIR;
  st=MStrPrint("%c:/*",drv_let);
  if (init)
    tempde->sub=tempde1=FilesFind(st,FUF_RECURSE);
  else
    tempde1=NULL;
  Free(st);
  tempde->user_data=DocPrint(doc,"$$TR,\"%s\",U=0x%X$$",tempde->full_name,tempde);
  tempde->next=*_root;
  *_root=tempde;
  DocPrint(doc,"\n$$ID,+2$$");
  DocBottom(doc);
  if (init) {
    DirFileDoc(doc,tempde1);
    while (tempde1) {
      tempde1->parent=tempde;
      tempde1=tempde1->next;
    }
  }
  DocPrint(doc,"$$ID,-2$$");
  return tempde;
}

U0 FMRebuildDoc(CDoc **_doc,CDirEntry **_root,I64 mode)
{
  CDrv *dv;
  I64 i;
  CDoc *doc=*_doc,*doc2=sys_clipboard_doc,*parent_doc;
  CFMUncollapsedLst *tempc=NULL;
  U8 *cur_entry=NULL,*next_entry=NULL;
  CDocEntry *doc_ce;
  CDirEntry *tempde,*tempde1,*cur_tree_entry;
  if (!doc)
    parent_doc=DocPut;
  else {
    parent_doc=doc->parent_doc;
    Fs->put_doc=Fs->display_doc=NULL;
    DocUnlock(doc);
    WinMgrSync;
    DocLock(doc);
    cur_tree_entry=NULL;
    doc_ce=doc->cur_entry;
    if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
      cur_tree_entry=doc_ce->user_data;
    if (cur_tree_entry)
      cur_entry=StrNew(cur_tree_entry->full_name);
    tempde=NULL;
    if (doc_ce!=doc)
      doc_ce=doc_ce->next;
    while (doc_ce!=doc) {
      if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
	tempde=doc_ce->user_data;
      else
	tempde=NULL;
      if (tempde) {
	tempde1=tempde->parent;
	while (tempde1) {
	  if (tempde1==cur_tree_entry) {
	    tempde=NULL;
	    break;
	  } else
	    tempde1=tempde1->parent;
	}
	if (tempde)
	  break;
      }
      doc_ce=doc_ce->next;
    }
    if (tempde)
      next_entry=StrNew(tempde->full_name);

    tempc=FMCollectUncollapsedLst(doc);
    DocDel(doc);
  }
  if (*_root) {
    DirTreeDel(*_root);
    *_root=NULL;
  }
  doc=DocNew;
  doc->desc='FileMan';
  doc->parent_doc=parent_doc;
  doc->flags|=DOCF_FORM;
  switch (mode) {
    case FM_NORMAL:
      DocPrint(doc,"$$PURPLE$$File Manager\n\n"
	    "$$LK,\"Click for Help\",A=\"FI:::/Doc/FileMgr.TXT\"$$\n\n");
      break;
    case FM_PICK_FILE:
      DocPrint(doc,"$$PURPLE$$Pick file and press <ESC>\n\n");
      doc->flags|=DOCF_MIN_SIZE;
      break;
    case FM_PICK_DIR:
      DocPrint(doc,"$$PURPLE$$Pick directory and press <ESC>\n\n");
      doc->flags|=DOCF_MIN_SIZE;
      break;
  }
  DocPrint(doc,"$$LTBLUE$$");
  for (i=0;i<NUM_DRVS;i++) {
    dv=&blkdev.drvs[i];
    if (dv->fs_type==FSt_REDSEA||dv->fs_type==FSt_FAT32)
      FMRebuildDocDrv(Drv2Let(dv),doc,_root,TRUE);
    else if (dv->fs_type==FSt_ISO9660) {
      if (dv->bd->flags&BDF_INITIALIZED)
	tempde=FMRebuildDocDrv(Drv2Let(dv),doc,_root,TRUE);
      else {
	tempde=FMRebuildDocDrv(Drv2Let(dv),doc,_root,FALSE);
	tempde->flags|=DEF_NOT_INITIALIZED;
      }
      tempde->flags|=DEF_REMOVABLE;
    }
  }
  DocTop(doc);
  FMMarkUncollapsed(doc,tempc,cur_entry,next_entry);
  DocCenter(doc);
  DocRst(doc2,TRUE);
  FMDelUncollapsedLst(tempc);
  Free(cur_entry);
  Free(next_entry);
  *_doc=doc;
  DocLock(doc);
  Fs->put_doc=Fs->display_doc=doc;
}

U0 FMRename(CDoc *doc)
{
  CEdFileName fn;
  CDocEntry *doc_e=doc->cur_entry;
  CDirEntry *tempde=NULL,*parent;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tempde=doc_e->user_data;
    if (parent=tempde->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tempde->name);
      if (DocForm(&fn)) {
	Silent;
	Move(tempde->name,fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tempde=doc_e->user_data;
    if (parent=tempde->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tempde->name);
      if (DocForm(&fn)) {
	if (StrCmp(tempde->name,fn.name)) {
	  Silent;
	  if (CopyTree(tempde->name,fn.name))
	    DelTree(tempde->name);
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
  CDirEntry *tempde=NULL,*parent;
  *fn.name=0;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tempde=doc_e->user_data;
    if (parent=tempde->parent) {
      Cd(parent->full_name);
      if (DocForm(&fn)) {
	Silent;
	MkDir(fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tempde=doc_e->user_data;
    Cd(tempde->full_name);
    if (DocForm(&fn)) {
      Silent;
      MkDir(fn.name);
      Silent(OFF);
    }
  }
}

U0 FMDelete(CDoc *doc)
{
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempde;
  if (doc_ce->type_u8==DOCT_MENU_VAL) {
    tempde=doc_ce->user_data;
    Silent;
    st=MStrPrint("Delete: %s",tempde->full_name);
    if (PopUpCancelOk(st))
      Del(tempde->full_name);
    Free(st);
    Silent(OFF);
  } else if (doc_ce->type_u8==DOCT_TREE) {
    tempde=doc_ce->user_data;
    Silent;
    st=MStrPrint("Delete: %s",tempde->full_name);
    if (PopUpCancelOk(st))
      DelTree(tempde->full_name);
    Free(st);
    Silent(OFF);
  }
}

U0 FMMount(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempde;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempde=doc_ce->user_data;
  else
    tempde=NULL;
  if (tempde) {
    while (tempde->parent)
      tempde=tempde->parent;
    Silent;
    ChgDsk(*tempde->full_name);
    Silent(OFF);
  }
}

U0 FMFmtDrv(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempde;
  U8 *st=NULL;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempde=doc_ce->user_data;
  else
    tempde=NULL;
  if (tempde) {
    while (tempde->parent)
      tempde=tempde->parent;
    st=MStrPrint("Format Drive '%c'?\nAre You Sure?\n",*tempde->full_name);
    if (PopUpCancelOk(st)) {
      Silent;
      Fmt(*tempde->full_name,,FALSE);
      Silent(OFF);
    }
  }
  Free(st);
}

U0 FMMakeISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempde;
  U8 *st;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempde=doc_ce->user_data;
  else
    tempde=NULL;
  if (tempde) {
    if (doc_ce->type_u8==DOCT_MENU_VAL)
      tempde=tempde->parent;
    if (tempde && *tempde->full_name) {
      Silent;
      if (tempde->full_name[StrLen(tempde->full_name)-1]=='/')
	st=MStrPrint("%s*",tempde->full_name);
      else
	st=MStrPrint("%s/*",tempde->full_name);
      ISOFileCreate(,st);
      Free(st);
      Silent(OFF);
    }
  }
}

U0 FMBurnISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tempde;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tempde=doc_ce->user_data;
  else
    tempde=NULL;
  if (tempde) {
    while (tempde->parent)
      tempde=tempde->parent;
    Silent;
    DVDImageWrite(*tempde->full_name);
    Silent(OFF);
  }
}

U0 FMCopy(CDoc *doc)
{
  CDoc *doc2=sys_clipboard_doc;
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tempde,*tempde1=NULL,*tempde2;
  Bool unlock_doc2=DocLock(doc2);
  doc_e=doc2->root.next;

  tempde1=doc_ce->user_data;
  if (doc_ce->type_u8==DOCT_MENU_VAL)
    tempde1=tempde1->parent;
  else if (doc_ce->type_u8!=DOCT_TREE)
    tempde1=NULL;
  if (tempde1) {
    while (doc_e!=doc2) {
      if (doc_e->type_u8==DOCT_MENU_VAL) {
	tempde=doc_e->user_data;
	tempde->flags|=DEF_PROCESSED;
	tempde2=tempde->parent;
	if (!tempde2 || !(tempde2->flags&DEF_PROCESSED)) {
	  Silent;
	  Copy(tempde->full_name,tempde1->full_name);
	  Silent(OFF);
	}
      } else if (doc_e->type_u8==DOCT_TREE) {
	tempde=doc_e->user_data;
	tempde->flags|=DEF_PROCESSED;
	tempde2=tempde->parent;
	if (!tempde2 || !(tempde2->flags&DEF_PROCESSED)) {
	  Silent;
	  if (*tempde1->name)
	    st=MStrPrint("%s/%s",tempde1->full_name,tempde->name);
	  else
	    st=MStrPrint("%s%s",tempde1->full_name,tempde->name);
	  CopyTree(tempde->full_name,st);
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

I64 PopUpFMRight(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"$$CM+LX,1,1 $$$$BT,\"INCLUDE               \",LE=FMR_INCLUDE$$"
	"$$CM+LX,27,0$$$$BT,\"ADAM INCLUDE          \",LE=FMR_ADAM_INCLUDE$$"
	"$$CM+LX,1,3 $$$$BT,\"DELETE                \",LE=FMR_DELETE$$"
	"$$CM+LX,27,0$$$$BT,\"RENAME                \",LE=FMR_RENAME$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE DIRECTORY        \",LE=FMR_MKDIR$$"
	"$$CM+LX,27,0$$$$BT,\"PLAIN-TEXT EDIT       \",LE=FMR_PLAIN$$"
	"$$CM+LX,1,3 $$$$BT,\"PASTE CLIPBOARD FILES \",LE=FMR_PASTE$$"
	"$$CM+LX,27,0$$$$BT,\"CHANGE DISK(MOUNT IT) \",LE=FMR_CHG_DSK$$"
	"$$CM+LX,1,3 $$$$BT,\"FORMAT                \",LE=FMR_FORMAT$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE ISO (CD/DVD) FILE\",LE=FMR_MAKE_ISO$$"
	"$$CM+LX,27,0$$$$BT,\"BURN ISO (CD/DVD) FILE\",LE=FMR_BURN_ISO$$"
	"$$CM+LX,1,3 $$$$BT,\"HELP                  \",LE=FMR_HELP$$"
	"$$CM+LX,27,0$$$$BT,\"CANCEL                \",LE=DOCM_CANCEL$$\n");
  if (footer) DocPrint(doc,"%s",footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 FMRightClick()
{
  switch (PopUpFMRight) {
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
    GrPrint(dc,ip.pos.x,ip.pos.y,"%s",fm_ip_str);
  }
  (*fp_old_final_screen_update)(dc);
}

public U8 *FileMgr(I64 mode=FM_NORMAL,CTask *mem_task=NULL)
{//File manager. Also, used to choose files and dirs.
  CDirEntry *root=NULL,*tempde,*tempde1,*tempde2;
  I64 sc,ch,a1,a2,msg_code;
  CDoc *doc=NULL,*old_put_doc=DocPut,*old_display_doc=DocDisplay;
  U8 *result=NULL,*st,*st2,*old_cur_dir=CurDir;
  CDocEntry *doc_ce=NULL,*doc_e;
  Bool okay;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  fp_old_final_screen_update=gr.fp_final_screen_update;
  MenuFilePush("::/Doc/FileMgrPullDown.TXT");
  FMRebuildDoc(&doc,&root,mode);
  if (tempde1=Cd2DirEntry(root,old_cur_dir))
    doc->cur_entry=tempde1->user_data;
  while (tempde1) {
    if (tempde1->attr&RS_ATTR_DIR)
      tempde1->user_data(CDocEntry *)->de_flags&=~DOCEF_CHECKED_COLLAPSED;
    tempde1=tempde1->parent;
  }
  do {
    DocUnlock(doc);
    do msg_code=GetMsg(&a1,&a2,
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
	gr.fp_final_screen_update=&FMFinalScreenUpdate;
	break;
      case MSG_IP_L_UP:
	if (doc_ce) {
	  gr.fp_final_screen_update=fp_old_final_screen_update;
	  if (WinCursorPosSet(Fs,a1+Fs->pix_left+Fs->scroll_x,
		a2+Fs->pix_top+Fs->scroll_y,TRUE)) {
	    doc_e=doc->cur_entry;
	    if (doc_e!=doc_ce) {
	      st2=NULL;
	      if (doc_e->type_u8==DOCT_MENU_VAL) {
		tempde1=doc_e->user_data;
		if (tempde1=tempde1->parent)
		  st2=StrNew(tempde1->full_name);
	      } else if (doc_e->type_u8==DOCT_TREE) {
		tempde1=doc_e->user_data;
		st2=StrNew(tempde1->full_name);
	      }
	      if (st2 && doc_ce->type_u8==DOCT_MENU_VAL) {
		tempde=doc_ce->user_data;
		Silent;
		Move(tempde->full_name,st2);
		Silent(OFF);
		FMRebuildDoc(&doc,&root,mode);
	      } else if (st2 && doc_ce->type_u8==DOCT_TREE) {
		tempde=doc_ce->user_data;
		okay=TRUE;
		tempde2=tempde1;
		while (tempde2) {
		  if (tempde2!=tempde)
		    tempde2=tempde2->parent;
		  else {
		    okay=FALSE;
		    break;
		  }
		}
		if (okay) {
		  if (*tempde1->name)
		    st=MStrPrint("%s/%s",tempde1->full_name,tempde->name);
		  else
		    st=MStrPrint("%s%s",tempde1->full_name,tempde->name);
		  if (StrCmp(tempde->full_name,st)) {
		    Silent;
		    CopyTree(tempde->full_name,st);
		    DelTree(tempde->full_name);
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
	ch=a1; sc=a2;
	if (sc.u8[0]==SC_DELETE && !(sc&(SCF_SHIFT|SCF_CTRL)))
	  ch=CH_CTRLY;
	if (ch && sc&SCF_ALT) goto fm_regular_key;
	switch (ch) {
	  case '\n':
	    DocUnlock(doc);
	    FMRightClick;
	    DocLock(doc);
	    break;
	  sub_switch_start:
	    DocUnlock(doc);
	    case CH_CTRLV:
	      FMCopy(doc);
	      break;
	    case 'r':
	      FMRename(doc);
	      break;
	    case 'd':
	      FMMkDir(doc);
	      break;
	    case CH_CTRLY:
	      FMDelete(doc);
	      break;
	    case 'm':
	      FMMount(doc);
	      break;
	    case 'M':
	      FMMakeISO(doc);
	      break;
	    case 'B':
	      FMBurnISO(doc);
	      break;
	    case 'f':
	      FMFmtDrv(doc);
	      break;
	  sub_switch_end:
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
	      goto fm_regular_key;
	    break;
	  case CH_SHIFT_SPACE:
	    if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
	      DocUnlock(doc);
	      Plain(doc->cur_entry->user_data(CDirEntry *)->full_name);
	      DocLock(doc);
	    } else
	      goto fm_regular_key;
	    break;
	  case CH_ESC:
	    doc_ce=doc->cur_entry;
	    tempde=doc_ce->user_data;
	    if (mode==FM_PICK_FILE && doc_ce->type_u8==DOCT_MENU_VAL)
	      result=StrNew(tempde->full_name,mem_task);
	    else if (mode==FM_PICK_DIR) {
	      if (doc_ce->type_u8==DOCT_TREE)
		result=StrNew(tempde->full_name,mem_task);
	      else if (doc_ce->type_u8==DOCT_MENU_VAL &&
		    (tempde=tempde->parent))
		result=StrNew(tempde->full_name,mem_task);
	    }
	    break;
	  default:
	    if (sc.u8[0]==SC_INS && sc&SCF_SHIFT && !(sc&SCF_CTRL)) {
	      FMCopy(doc);
	      FMRebuildDoc(&doc,&root,mode);
	    } else if (sc.u8[0]==SC_F5) {
	      if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
		tempde=doc->cur_entry->user_data;
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  AdamFile(tempde->full_name);
		else
		  PopUpFile(tempde->full_name);
		DocLock(doc);
	      }
	    } else {
fm_regular_key:
	      DocUnlock(doc);
	      PutKey(ch,sc);
	      DocLock(doc);
	    }
	}
	break;
    }
  } while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  gr.fp_final_screen_update=fp_old_final_screen_update;
  Fs->put_doc	 =old_put_doc;
  Fs->display_doc=old_display_doc;
  SettingsPop;
  DocDel(doc);
  DirTreeDel(root);
  Cd(old_cur_dir);
  Free(old_cur_dir);
  if (mode!=FM_NORMAL && !result)
    result=StrNew("",mem_task);
  MenuPop;
  return result;
}
