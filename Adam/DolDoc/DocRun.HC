#help_index "DolDoc"

public I64 DocEntryRun(CDoc *doc,CDocEntry *doc_e,
	Bool exited,I64 *_has_action=NULL)
{//Do action on final entry sel by user.
//Sometimes returns locked, sometimes unlocked
  U8 ch=doc->cmd_U8,*st;
  I64 res=DOCM_CANCEL,has_action=FALSE;
  CHashDefineStr *tmph;
  DocLock(doc);
  if (!exited) {
    if (doc_e->de_flags & DOCEF_ESC) {
      Msg(MSG_KEY_DOWN,CH_ESC,0,1<<JOBf_DONT_FILTER);
      has_action=TRUE;
      goto er_done;
    } if (doc_e->de_flags & DOCEF_QUIT) {
      Msg(MSG_KEY_DOWN,CH_SHIFT_ESC,0,1<<JOBf_DONT_FILTER);
      has_action=TRUE;
      goto er_done;
    } else if (doc_e->de_flags & DOCEF_CHECK_COLLAPSABLE) {
      doc_e->de_flags^=DOCEF_CHECKED_COLLAPSED;
      has_action=TRUE;
    }
  }
  try {
    if (ch==CH_SPACE) {
      if (doc_e->de_flags & DOCEF_LINK && doc->left_click_link) {
	res=(*doc->left_click_link)(doc,doc_e);
	has_action=TRUE;
      }
      if (doc_e->de_flags & DOCEF_LEFT_EXP) {
	res=doc_e->left_exp;
	has_action=TRUE;
	Msg(MSG_CMD,res,0,1<<JOBf_DONT_FILTER);
      }
      if (doc_e->de_flags & DOCEF_LEFT_CB && doc_e->left_cb) {
	DocUnlock(doc);
	res=(*doc_e->left_cb)(doc,doc_e);
	has_action=TRUE;
	Msg(MSG_CMD,res,0,1<<JOBf_DONT_FILTER);
      }
      if (doc_e->de_flags & DOCEF_LEFT_MACRO) {
	if (doc_e->de_flags & DOCEF_POPUP) {
	  st=StrNew(doc_e->left_macro);
	  DocUnlock(doc);
	  PopUp(st,Fs);
	  Free(st);
	} else {
	  if (doc_e->de_flags & DOCEF_LEFT_IN_STR)
	    InStr("%s",doc_e->left_macro);
	  else
	    In("%s",doc_e->left_macro);
	}
	has_action=TRUE;
      }
      if (!exited && doc_e->de_flags & DOCEF_LST &&
	    doc_e->de_flags & DOCEF_DEFINE &&
	    (tmph=HashFind(doc_e->define_str,
	    doc->win_task->hash_table,HTT_DEFINE_STR)) &&
	    (res=PopUpPickLst(tmph->data))!=DOCM_CANCEL) {
	DocDataFmt(doc,doc_e,res);
	DocDataScan(doc,doc_e);
	has_action=TRUE;
      }
    } else if (ch=='\n') {
      if (doc_e->de_flags & DOCEF_LINK && doc->right_click_link) {
	res=(*doc->right_click_link)(doc,doc_e);
	has_action=TRUE;
      }
      if (doc_e->de_flags & DOCEF_RIGHT_EXP) {
	res=doc_e->right_exp;
	has_action=TRUE;
	Msg(MSG_CMD,res,0,1<<JOBf_DONT_FILTER);
      }
      if (doc_e->de_flags & DOCEF_RIGHT_CB && doc_e->right_cb) {
	DocUnlock(doc);
	res=(*doc_e->right_cb)(doc,doc_e);
	has_action=TRUE;
	Msg(MSG_CMD,res,0,1<<JOBf_DONT_FILTER);
      }
      if (doc_e->de_flags & DOCEF_RIGHT_MACRO) {
	if (doc_e->de_flags & DOCEF_POPUP) {
	  st=StrNew(doc_e->right_macro);
	  DocUnlock(doc);
	  PopUp(st,Fs);
	  Free(st);
	} else {
	  if (doc_e->de_flags & DOCEF_RIGHT_IN_STR)
	    InStr("%s",doc_e->right_macro);
	  else
	    In("%s",doc_e->right_macro);
	}
	has_action=TRUE;
      }
    }
  }
  catch
    DocBottom(doc);
  doc->cmd_U8=CH_SPACE;
er_done:
  if (_has_action) *_has_action=has_action;
  return res;
}
