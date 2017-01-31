extern U0 BgtRegen();

Bool BgtPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code
  no_warn sc;
  CBgtEntry *tmpb,*tmpb1;
  CBgtTemplate *tmpt,*tmpt1;
  CDocEntry *doc_ce;
  U8 *st;
  switch (ch) {
    case '\n':
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL) {
	tmpb=doc_ce->user_data;
	if (tmpt=tmpb->template) {
	  if (tmpt1=BgtTemplatePmt(tmpt)) {
	    QueRem(tmpt);
	    BgtTemplatePurge(tmpt);
	    BgtEntryDel2(&tmpt->b);
	    Free(tmpt);
	    QueIns(tmpt1,t_head.last);
	    BgtTemplateExpand(tmpt1);
	    BgtRegen;
	  }
	} else {
	  if (tmpb1=BgtEntryPmt(tmpb)) {
	    QueRem(tmpb);
	    BgtEntryDel(tmpb);
	    BgtIns(tmpb1);
	    BgtRegen;
	  }
	}
      }
      return TRUE;
    case CH_CTRLY:
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL) {
	tmpb=doc_ce->user_data;
	if (tmpt=tmpb->template) {
	  QueRem(tmpt);
	  BgtTemplateDel(tmpt);
	} else {
	  QueRem(tmpb);
	  BgtEntryDel(tmpb);
	}
	BgtRegen;
      }
      return TRUE;
    case 'a':
      PopUpOk(	"Set the name and color of your accounts.\n"
	    "To delete accounts, manually edit\n"
	    "$$GREEN$$~/Budget/Accts.DD.Z$$FG$$.");
      if (PopUpEd(bgt_accts_file,Fs)) {
	BgtAcctsRead;
	BgtRegen;
      }
      return TRUE;
    case 'v':
      if ((st=BgtPopUpAcct("View Acct\n\n",view_acct))>=0) {
	StrCpy(view_acct,st);
	BgtRegen;
      }
      return TRUE;
    case 'n':
      if (tmpb1=BgtEntryPmt) {
	BgtIns(tmpb1);
	BgtRegen;
      }
      return TRUE;
    case 't':
      if (tmpt1=BgtTemplatePmt) {
	QueIns(tmpt1,t_head.last);
	BgtTemplateExpand(tmpt1);
	BgtRegen;
      }
      return TRUE;
    case 'c':
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL)
	tmpb=doc_ce->user_data;
      else
	tmpb=NULL;
      if (tmpb1=BgtEntryPmt(tmpb)) {
	BgtIns(tmpb1);
	BgtRegen;
      }
      return TRUE;
    case 'p':
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL) {
	tmpb=doc_ce->user_data;
	if (tmpt1=BgtTemplatePmt(,tmpb)) {
	  BgtTemplateExpand(tmpt1,TRUE);
	  BgtTemplateDel(tmpt1);
	  BgtRegen;
	}
      }
      return TRUE;
  }
  return FALSE;
}

U0 BgtRegen()
{
  I64 timeout_jiffy,c,color=COLOR_INVALID;
  F64 balance=0;
  CDoc *doc,*pdoc,*ddoc;
  CDocEntry *doc_ce;
  CBgtEntry *tmpb=b_head.next,*tmpb_ce;
  doc=DocNew;
  doc->flags|=DOCF_FORM;
  while (tmpb!=&b_head) {
    if (!StrCmp(view_acct,tmpb->credit))
      balance-=tmpb->amount;
    if (!StrCmp(view_acct,tmpb->debit))
      balance+=tmpb->amount;
    c=BgtAcctColor(tmpb->credit);
    if (c!=color) {
      color=c;
      DocPrint(doc,"$$FG,%d$$",color);
    }
    tmpb->doc_e=DocPrint(doc,
	  "$$MU-UL,\"%D %8ts %8ts:%8.2f %8.2f:%$$Q\",U=0x%X$$\n",
	  tmpb->date,tmpb->credit,tmpb->debit,balance,
	  tmpb->amount,tmpb->desc,tmpb);
    tmpb=tmpb->next;
  }
  DocRecalc(doc);

  if (pdoc=Fs->put_doc) {
    DocLock(pdoc);
//Now, we want to preserve old position in doc, using ugly brute force.
    //It's tricky -- can't use old line num because of editor filters.

    //The price we pay for using the standard document editor is this kludge.
    //When I originally wrote my budget program, I did not have separate budget
    //and line entries, so we never had to resync.

    doc_ce=pdoc->cur_entry;
    timeout_jiffy=cnts.jiffies+JIFFY_FREQ; //Max one second.
    while (doc_ce!=pdoc && cnts.jiffies<timeout_jiffy) {
      while (doc_ce->type_u8!=DOCT_MENU_VAL || !(tmpb_ce=doc_ce->user_data)) {
	doc_ce=doc_ce->next;
	if (doc_ce==pdoc) goto br_cont;
      }
      tmpb=b_head.next;
      while (tmpb!=&b_head) {
	if (tmpb==tmpb_ce) {
	  doc->cur_entry=tmpb->doc_e;
	  doc->cur_col=0;
	  DocCenter(doc);
	  goto br_cont;
	}
	tmpb=tmpb->next;
      }
      doc_ce=doc_ce->next;
    }
  }

  br_cont:
  ddoc=Fs->display_doc;
  Fs->put_doc	 =doc;
  Fs->display_doc=doc;
  DocDel(pdoc);
  if (pdoc!=ddoc)
    DocDel(ddoc);
  doc->user_put_key=&BgtPutKey;
}

U0 Budget(U8 *dirname="~/Budget")
{
  CDoc *pdoc,*ddoc,*old_put,*old_display;

  Cd(dirname);
  bgt_string_file	=FileNameAbs("Strs.DD.Z");
  bgt_accts_file	=FileNameAbs("Accts.DD.Z");
  bgt_data_file		=FileNameAbs("Bgt.DATA.Z");

  BgtAcctsRead;
  BgtDataRead;
  CBgtTemplatesExpand;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  NewEntry(,'n');"
	"  CopyEntry(,'c');"
	"  PeriodicEntry(,'p');"
	"  EditEntry(,'\n');"
	"  DeleteEntry(,CH_CTRLY);"
	"  NewTemplate(,'t');"
	"  AcctsFile(,'a');"
	"}"
	"View {"
	"  ViewAcct(,'v');"
	"}"
	);
  StrCpy(view_acct,"BANK");
  DocMax;
  old_put	 =Fs->put_doc;
  old_display	 =Fs->display_doc;
  Fs->put_doc	 =NULL;
  Fs->display_doc=NULL;
  BgtRegen;
  try
    if (View) {
      BgtDataWrite;
      BgtAcctsWrite;
    }
  catch
    PutExcept;

  pdoc=Fs->put_doc;
  ddoc=Fs->display_doc;
  Fs->put_doc	 =old_put;
  Fs->display_doc=old_display;
  DocDel(pdoc);
  if (pdoc!=ddoc)
    DocDel(ddoc);

  SettingsPop;
  BgtDel;
  MenuPop;
}
