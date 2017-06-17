#help_index "DolDoc/Output;StdOut/DolDoc"
U0 DirFileDoc(CDoc *doc,CDirEntry *tmpde)
{
  while (tmpde) {
    if (tmpde->attr & RS_ATTR_DIR) {
      tmpde->user_data=DocPrint(doc,"$$TR,\"%s\",U=0x%X$$",tmpde->name,tmpde);
      DocPrint(doc,"\n$$ID,+2$$");
      if (tmpde->sub)
	DirFileDoc(doc,tmpde->sub);
      DocPrint(doc,"$$ID,-2$$");
    } else {
      tmpde->user_data=DocPrint(doc,"$$MU,\"%s\",U=0x%X$$",tmpde->name,tmpde);
      DocPrint(doc,"\n");
    }
    tmpde=tmpde->next;
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
  CDocEntry *doc_e=doc->head.next;
  CFMUncollapsedLst *res=NULL,*tmpc;
  CDirEntry *tmpde;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      if (!(doc_e->de_flags&DOCEF_CHECKED_COLLAPSED)) {
	if (tmpde=doc_e->user_data) {
	  tmpc=MAlloc(sizeof(CFMUncollapsedLst));
	  tmpc->next=res;
	  res=tmpc;
	  tmpc->name=StrNew(tmpde->full_name);
	}
      }
    }
    doc_e=doc_e->next;
  }
  return res;
}

U0 FMMarkUncollapsed(CDoc *doc,CFMUncollapsedLst *tmpc,
	U8 *cur_entry,U8 *next_entry)
{
  CDocEntry *doc_e=doc->head.next;
  CFMUncollapsedLst *tmpc1;
  CDirEntry *tmpde;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_TREE) {
      tmpde=doc_e->user_data;
      tmpc1=tmpc;
      while (tmpc1) {
	if (!StrCmp(tmpc1->name,tmpde->full_name)) {
	  doc_e->de_flags&=~DOCEF_CHECKED_COLLAPSED;
	  break;
	}
	tmpc1=tmpc1->next;
      }
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tmpde->full_name,StrLen(tmpde->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tmpde->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,
		tmpde->full_name,StrLen(tmpde->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tmpde->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    } else if (doc_e->type_u8==DOCT_MENU_VAL) {
      tmpde=doc_e->user_data;
      if (cur_entry) {
	if (!StrNCmp(cur_entry,tmpde->full_name,StrLen(tmpde->full_name))) {
	  doc->cur_entry=doc_e;
	  if (StrLen(tmpde->full_name)==StrLen(cur_entry))
	    cur_entry=NULL;
	} else if (next_entry) {
	  if (!StrNCmp(next_entry,
		tmpde->full_name,StrLen(tmpde->full_name))) {
	    doc->cur_entry=doc_e;
	    if (StrLen(tmpde->full_name)==StrLen(next_entry))
	      cur_entry=NULL;
	  }
	}
      }
    }
    doc_e=doc_e->next;
  }
}

U0 FMDelUncollapsedLst(CFMUncollapsedLst *tmpc)
{
  CFMUncollapsedLst *tmpc1;
  while (tmpc) {
    tmpc1=tmpc->next;
    Free(tmpc->name);
    Free(tmpc);
    tmpc=tmpc1;
  }
}

CDirEntry *FMRebuildDocDrv(U8 drv_let,CDoc *doc,CDirEntry **_head,Bool init)
{
  CDirEntry *tmpde,*tmpde1;
  U8 *st;
  tmpde=CAlloc(sizeof(CDirEntry));
  tmpde->full_name=MStrPrint("%C:/",drv_let);
  tmpde->attr=RS_ATTR_DIR;
  st=MStrPrint("%c:/*",drv_let);
  if (init)
    tmpde->sub=tmpde1=FilesFind(st,FUF_RECURSE);
  else
    tmpde1=NULL;
  Free(st);
  tmpde->user_data=DocPrint(doc,"$$TR,\"%s\",U=0x%X$$",tmpde->full_name,tmpde);
  tmpde->next=*_head;
  *_head=tmpde;
  DocPrint(doc,"\n$$ID,+2$$");
  DocBottom(doc);
  if (init) {
    DirFileDoc(doc,tmpde1);
    while (tmpde1) {
      tmpde1->parent=tmpde;
      tmpde1=tmpde1->next;
    }
  }
  DocPrint(doc,"$$ID,-2$$");
  return tmpde;
}

#define DEF2_PROCESSED		1
#define DEF2_NOT_INITIALIZED	2

U0 FMRebuildDoc(CDoc **_doc,CDirEntry **_head,I64 mode)
{
  CDrv *dv;
  I64 i;
  CDoc *doc=*_doc,*doc2=sys_clip_doc,*parent_doc;
  CFMUncollapsedLst *tmpc=NULL;
  U8 *cur_entry=NULL,*next_entry=NULL;
  CDocEntry *doc_ce;
  CDirEntry *tmpde,*tmpde1,*cur_tree_entry;
  if (!doc)
    parent_doc=DocPut;
  else {
    parent_doc=doc->parent_doc;
    Fs->put_doc=Fs->display_doc=NULL;
    DocUnlock(doc);
    Refresh;
    DocLock(doc);
    cur_tree_entry=NULL;
    doc_ce=doc->cur_entry;
    if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
      cur_tree_entry=doc_ce->user_data;
    if (cur_tree_entry)
      cur_entry=StrNew(cur_tree_entry->full_name);
    tmpde=NULL;
    if (doc_ce!=doc)
      doc_ce=doc_ce->next;
    while (doc_ce!=doc) {
      if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
	tmpde=doc_ce->user_data;
      else
	tmpde=NULL;
      if (tmpde) {
	tmpde1=tmpde->parent;
	while (tmpde1) {
	  if (tmpde1==cur_tree_entry) {
	    tmpde=NULL;
	    break;
	  } else
	    tmpde1=tmpde1->parent;
	}
	if (tmpde)
	  break;
      }
      doc_ce=doc_ce->next;
    }
    if (tmpde)
      next_entry=StrNew(tmpde->full_name);

    tmpc=FMCollectUncollapsedLst(doc);
    DocDel(doc);
  }
  if (*_head) {
    DirTreeDel(*_head);
    *_head=NULL;
  }
  doc=DocNew;
  doc->desc='FileMan';
  doc->parent_doc=parent_doc;
  doc->flags|=DOCF_FORM;
  switch (mode) {
    case FM_NORMAL:
      DocPrint(doc,"$$PURPLE$$File Manager\n\n"
	    "$$LK,\"Click for Help\",A=\"FI:::/Doc/FileMgr.DD\"$$\n\n");
      break;
    case FM_PICK_FILE:
      DocPrint(doc,"$$PURPLE$$Pick file and press <ESC>\n\n");
      doc->flags|=DOCF_SIZE_MIN;
      break;
    case FM_PICK_DIR:
      DocPrint(doc,"$$PURPLE$$Pick directory and press <ESC>\n\n");
      doc->flags|=DOCF_SIZE_MIN;
      break;
  }
  DocPrint(doc,"$$LTBLUE$$");
  for (i=0;i<DRVS_NUM;i++) {
    dv=&blkdev.drvs[i];
    if (dv->bd->type==BDT_ATAPI) {
      if (dv->bd->flags&BDF_INITIALIZED)
	tmpde=FMRebuildDocDrv(Drv2Let(dv),doc,_head,TRUE);
      else {
	tmpde=FMRebuildDocDrv(Drv2Let(dv),doc,_head,FALSE);
	tmpde->user_data2|=DEF2_NOT_INITIALIZED;
      }
    } else if (dv->fs_type==FSt_REDSEA || dv->fs_type==FSt_FAT32)
      FMRebuildDocDrv(Drv2Let(dv),doc,_head,TRUE);
  }
  DocTop(doc);
  FMMarkUncollapsed(doc,tmpc,cur_entry,next_entry);
  DocCenter(doc);
  DocRst(doc2,TRUE);
  FMDelUncollapsedLst(tmpc);
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
  CDirEntry *tmpde=NULL,*parent;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tmpde=doc_e->user_data;
    if (parent=tmpde->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tmpde->name);
      if (DocForm(&fn)) {
	Silent;
	Move(tmpde->name,fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tmpde=doc_e->user_data;
    if (parent=tmpde->parent) {
      Cd(parent->full_name);
      StrCpy(fn.name,tmpde->name);
      if (DocForm(&fn)) {
	if (StrCmp(tmpde->name,fn.name)) {
	  Silent;
	  if (CopyTree(tmpde->name,fn.name))
	    DelTree(tmpde->name);
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
  CDirEntry *tmpde=NULL,*parent;
  *fn.name=0;
  if (doc_e->type_u8==DOCT_MENU_VAL) {
    tmpde=doc_e->user_data;
    if (parent=tmpde->parent) {
      Cd(parent->full_name);
      if (DocForm(&fn)) {
	Silent;
	DirMk(fn.name);
	Silent(OFF);
      }
    }
  } else if (doc_e->type_u8==DOCT_TREE) {
    tmpde=doc_e->user_data;
    Cd(tmpde->full_name);
    if (DocForm(&fn)) {
      Silent;
      DirMk(fn.name);
      Silent(OFF);
    }
  }
}

U0 FMDelete(CDoc *doc)
{
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  if (doc_ce->type_u8==DOCT_MENU_VAL) {
    tmpde=doc_ce->user_data;
    Silent;
    st=MStrPrint("Delete: %s",tmpde->full_name);
    if (PopUpCancelOk(st))
      Del(tmpde->full_name);
    Free(st);
    Silent(OFF);
  } else if (doc_ce->type_u8==DOCT_TREE) {
    tmpde=doc_ce->user_data;
    Silent;
    st=MStrPrint("Delete: %s",tmpde->full_name);
    if (PopUpCancelOk(st))
      DelTree(tmpde->full_name);
    Free(st);
    Silent(OFF);
  }
}

U0 FMChgDsk(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde=doc_ce->user_data;
  else
    tmpde=NULL;
  if (tmpde) {
    while (tmpde->parent)
      tmpde=tmpde->parent;
    Silent;
    DskChg(*tmpde->full_name);
    Silent(OFF);
  }
}

U0 FMMountISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  if (doc_ce->type_u8==DOCT_MENU_VAL && (tmpde=doc_ce->user_data))
    MountFile(tmpde->full_name);
}

U0 FMUnmount(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  I64 drv_let;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde=doc_ce->user_data;
  else
    tmpde=NULL;
  if (tmpde) {
    while (tmpde->parent)
      tmpde=tmpde->parent;
    drv_let=*tmpde->full_name;
    if (Let2BlkDev(drv_let)!=Let2BlkDev(':'))
      Unmount(drv_let);
  }
}

U0 FMFmtDrv(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  U8 *st=NULL;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde=doc_ce->user_data;
  else
    tmpde=NULL;
  if (tmpde) {
    while (tmpde->parent)
      tmpde=tmpde->parent;
    st=MStrPrint("Format Drive '%c'?\nAre You Sure?\n",*tmpde->full_name);
    if (PopUpCancelOk(st)) {
      Silent;
      Fmt(*tmpde->full_name,,FALSE);
      Silent(OFF);
    }
  }
  Free(st);
}

U0 FMMakeISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde=doc_ce->user_data;
  else
    tmpde=NULL;
  if (tmpde) {
    if (doc_ce->type_u8==DOCT_MENU_VAL)
      tmpde=tmpde->parent;
    if (tmpde && *tmpde->full_name) {
      Silent;
      RedSeaISO(,tmpde->full_name);
      Silent(OFF);
    }
  }
}

U0 FMBurnISO(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  CDirEntry *tmpde;
  if (doc_ce->type_u8==DOCT_TREE || doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde=doc_ce->user_data;
  else
    tmpde=NULL;
  if (tmpde) {
    while (tmpde->parent)
      tmpde=tmpde->parent;
    Silent;
    DVDImageWrite(*tmpde->full_name);
    Silent(OFF);
  }
}

U0 FMCopy(CDoc *doc)
{
  CDoc *doc2=sys_clip_doc;
  U8 *st;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  CDirEntry *tmpde,*tmpde1=NULL,*tmpde2;
  Bool unlock_doc2=DocLock(doc2);
  doc_e=doc2->head.next;

  tmpde1=doc_ce->user_data;
  if (doc_ce->type_u8==DOCT_MENU_VAL)
    tmpde1=tmpde1->parent;
  else if (doc_ce->type_u8!=DOCT_TREE)
    tmpde1=NULL;
  if (tmpde1) {
    while (doc_e!=doc2) {
      if (doc_e->type_u8==DOCT_MENU_VAL) {
	tmpde=doc_e->user_data;
	tmpde->user_data2|=DEF2_PROCESSED;
	tmpde2=tmpde->parent;
	if (!tmpde2 || !(tmpde2->user_data2&DEF2_PROCESSED)) {
	  Silent;
	  Copy(tmpde->full_name,tmpde1->full_name);
	  Silent(OFF);
	}
      } else if (doc_e->type_u8==DOCT_TREE) {
	tmpde=doc_e->user_data;
	tmpde->user_data2|=DEF2_PROCESSED;
	tmpde2=tmpde->parent;
	if (!tmpde2 || !(tmpde2->user_data2&DEF2_PROCESSED)) {
	  Silent;
	  if (*tmpde1->name)
	    st=MStrPrint("%s/%s",tmpde1->full_name,tmpde->name);
	  else
	    st=MStrPrint("%s%s",tmpde1->full_name,tmpde->name);
	  CopyTree(tmpde->full_name,st);
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
#define FMR_MOUNT_REDSEA_ISO_C	9
#define FMR_UNMOUNT		10
#define FMR_MAKE_REDSEA_ISO_C	11
#define FMR_BURN_ISO		12
#define FMR_HELP		13

I64 PopUpFMRight(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"$$CM+LX,1,1 $$$$BT,\"INCLUDE                 \",LE=FMR_INCLUDE$$"
	"$$CM+LX,29,0$$$$BT,\"ADAM INCLUDE            \",LE=FMR_ADAM_INCLUDE$$"
	"$$CM+LX,1,3 $$$$BT,\"DELETE                  \",LE=FMR_DELETE$$"
	"$$CM+LX,29,0$$$$BT,\"RENAME                  \",LE=FMR_RENAME$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE DIRECTORY          \",LE=FMR_MKDIR$$"
	"$$CM+LX,29,0$$$$BT,\"PLAIN-TEXT EDIT         \",LE=FMR_PLAIN$$"
	"$$CM+LX,1,3 $$$$BT,\"PASTE CLIP FILES        \",LE=FMR_PASTE$$"
	"$$CM+LX,29,0$$$$BT,\"CHANGE DISK(MOUNT IT)   \",LE=FMR_CHG_DSK$$"
	"$$CM+LX,1,3 $$$$BT,\"FORMAT                  \",LE=FMR_FORMAT$$"
	"$$CM+LX,1,3 $$$$BT,\"MOUNT ISO.C FILE        \","
	"LE=FMR_MOUNT_REDSEA_ISO_C$$"
	"$$CM+LX,29,0$$$$BT,\"UNMOUNT                 \",LE=FMR_UNMOUNT$$"
	"$$CM+LX,1,3 $$$$BT,\"MAKE ISO.C (CD/DVD) FILE\",LE=FMR_MAKE_REDSEA_ISO_C$$"
	"$$CM+LX,29,0$$$$BT,\"BURN ISO (CD/DVD) FILE  \",LE=FMR_BURN_ISO$$"
	"$$CM+LX,1,3 $$$$BT,\"HELP                    \",LE=FMR_HELP$$"
	"$$CM+LX,29,0$$$$BT,\"CANCEL                  \",LE=DOCM_CANCEL$$\n");
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
      Msg(MSG_KEY_DOWN,'c',0);
      break;
    case FMR_FORMAT:
      Msg(MSG_KEY_DOWN,'f',0);
      break;
    case FMR_MOUNT_REDSEA_ISO_C:
      Msg(MSG_KEY_DOWN,'i',0);
      break;
    case FMR_UNMOUNT:
      Msg(MSG_KEY_DOWN,'u',0);
      break;
    case FMR_MAKE_REDSEA_ISO_C:
      Msg(MSG_KEY_DOWN,'m',0);
      break;
    case FMR_BURN_ISO:
      Msg(MSG_KEY_DOWN,'B',0);
      break;
    case FMR_HELP:
      Msg(MSG_KEY_DOWN,CH_CTRLM,0x43200000432);
      break;
  }
}

U8 *fm_ms_str=NULL;
U0 (*fp_old_final_scrn_update)(CDC *dc);

U0 FMFinalScrnUpdate(CDC *dc)
{
  if (fm_ms_str) {
    dc->color=LTRED;
    GrPrint(dc,ms.pos.x,ms.pos.y,"%s",fm_ms_str);
  }
  (*fp_old_final_scrn_update)(dc);
}

public U8 *FileMgr(I64 mode=FM_NORMAL,CTask *mem_task=NULL)
{//File manager. Also, used to choose files and dirs.
  CDirEntry *head=NULL,*tmpde,*tmpde1,*tmpde2;
  I64 sc,ch,arg1,arg2,msg_code;
  CDoc *doc=NULL,*old_put_doc=DocPut,*old_display_doc=DocDisplay;
  U8 *res=NULL,*st,*st2,*old_cur_dir=DirCur;
  CDocEntry *doc_ce=NULL,*doc_e;
  Bool okay;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  fp_old_final_scrn_update=gr.fp_final_scrn_update;
  MenuFilePush("::/Doc/FileMgrPullDown.DD");
  FMRebuildDoc(&doc,&head,mode);
  if (tmpde1=Cd2DirEntry(head,old_cur_dir))
    doc->cur_entry=tmpde1->user_data;
  while (tmpde1) {
    if (tmpde1->attr&RS_ATTR_DIR)
      tmpde1->user_data(CDocEntry *)->de_flags&=~DOCEF_CHECKED_COLLAPSED;
    tmpde1=tmpde1->parent;
  }
  do {
    DocUnlock(doc);
    do msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|1<<MSG_MS_R_UP);
    while (Fs!=sys_focus_task);
    DocLock(doc);
    switch (msg_code) {
      case MSG_MS_R_UP:
	DocUnlock(doc);
	FMRightClick;
	DocLock(doc);
	break;
      case MSG_MS_L_DOWN:
	doc_ce=doc->cur_entry;
	fm_ms_str=doc_ce->tag;
	gr.fp_final_scrn_update=&FMFinalScrnUpdate;
	break;
      case MSG_MS_L_UP:
	if (doc_ce) {
	  gr.fp_final_scrn_update=fp_old_final_scrn_update;
	  if (WinCursorPosSet(Fs,arg1+Fs->pix_left+Fs->scroll_x,
		arg2+Fs->pix_top+Fs->scroll_y,TRUE)) {
	    doc_e=doc->cur_entry;
	    if (doc_e!=doc_ce) {
	      st2=NULL;
	      if (doc_e->type_u8==DOCT_MENU_VAL) {
		tmpde1=doc_e->user_data;
		if (tmpde1=tmpde1->parent)
		  st2=StrNew(tmpde1->full_name);
	      } else if (doc_e->type_u8==DOCT_TREE) {
		tmpde1=doc_e->user_data;
		st2=StrNew(tmpde1->full_name);
	      }
	      if (st2 && doc_ce->type_u8==DOCT_MENU_VAL) {
		tmpde=doc_ce->user_data;
		Silent;
		Move(tmpde->full_name,st2);
		Silent(OFF);
		FMRebuildDoc(&doc,&head,mode);
	      } else if (st2 && doc_ce->type_u8==DOCT_TREE) {
		tmpde=doc_ce->user_data;
		okay=TRUE;
		tmpde2=tmpde1;
		while (tmpde2) {
		  if (tmpde2!=tmpde)
		    tmpde2=tmpde2->parent;
		  else {
		    okay=FALSE;
		    break;
		  }
		}
		if (okay) {
		  if (*tmpde1->name)
		    st=MStrPrint("%s/%s",tmpde1->full_name,tmpde->name);
		  else
		    st=MStrPrint("%s%s",tmpde1->full_name,tmpde->name);
		  if (StrCmp(tmpde->full_name,st)) {
		    Silent;
		    CopyTree(tmpde->full_name,st);
		    DelTree(tmpde->full_name);
		    Silent(OFF);
		    FMRebuildDoc(&doc,&head,mode);
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
	ch=arg1; sc=arg2;
	if (sc.u8[0]==SC_DELETE && !(sc&(SCF_SHIFT|SCF_CTRL)))
	  ch=CH_CTRLY;
	if (ch && sc&SCF_ALT) goto fm_regular_key;
	switch (ch) {
	  case '\n':
	    DocUnlock(doc);
	    FMRightClick;
	    DocLock(doc);
	    break;
	  start:
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
	    case 'c':
	      FMChgDsk(doc);
	      break;
	    case 'i':
	      FMMountISO(doc);
	      break;
	    case 'u':
	      FMUnmount(doc);
	      break;
	    case 'm':
	      FMMakeISO(doc);
	      break;
	    case 'B':
	      FMBurnISO(doc);
	      break;
	    case 'f':
	      FMFmtDrv(doc);
	      break;
	  end:
	    FMRebuildDoc(&doc,&head,mode);
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
	    tmpde=doc_ce->user_data;
	    if (mode==FM_PICK_FILE && doc_ce->type_u8==DOCT_MENU_VAL)
	      res=StrNew(tmpde->full_name,mem_task);
	    else if (mode==FM_PICK_DIR) {
	      if (doc_ce->type_u8==DOCT_TREE)
		res=StrNew(tmpde->full_name,mem_task);
	      else if (doc_ce->type_u8==DOCT_MENU_VAL &&
		    (tmpde=tmpde->parent))
		res=StrNew(tmpde->full_name,mem_task);
	    }
	    break;
	  default:
	    if (sc.u8[0]==SC_INS && sc&SCF_SHIFT && !(sc&SCF_CTRL)) {
	      FMCopy(doc);
	      FMRebuildDoc(&doc,&head,mode);
	    } else if (sc.u8[0]==SC_F5) {
	      if (doc->cur_entry->type_u8==DOCT_MENU_VAL) {
		tmpde=doc->cur_entry->user_data;
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  AdamFile(tmpde->full_name);
		else
		  PopUpFile(tmpde->full_name);
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
  gr.fp_final_scrn_update=fp_old_final_scrn_update;
  Fs->put_doc	 =old_put_doc;
  Fs->display_doc=old_display_doc;
  SettingsPop;
  DocDel(doc);
  DirTreeDel(head);
  Cd(old_cur_dir);
  Free(old_cur_dir);
  if (mode!=FM_NORMAL && !res)
    res=StrNew("",mem_task);
  MenuPop;
  return res;
}
