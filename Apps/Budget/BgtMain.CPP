extern U0 BgtRegen();

Bool BgtPutKey(CDoc *doc,U8 *,I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code
  no_warn sc;
  CBgtEntry *tempb,*tempb1;
  CBgtTemplate *tempt,*tempt1;
  CDocEntry *doc_ce;
  U8 *st;
  switch (ch) {
    case '\n':
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL) {
	tempb=doc_ce->user_data;
	if (tempt=tempb->template) {
	  if (tempt1=BgtTemplatePmt(tempt)) {
	    QueRem(tempt);
	    BgtTemplatePurge(tempt);
	    BgtEntryDel2(&tempt->b);
	    Free(tempt);
	    QueIns(tempt1,t_head.last);
	    BgtTemplateExpand(tempt1);
	    BgtRegen;
	  }
	} else {
	  if (tempb1=BgtEntryPmt(tempb)) {
	    QueRem(tempb);
	    BgtEntryDel(tempb);
	    BgtIns(tempb1);
	    BgtRegen;
	  }
	}
      }
      return TRUE;
    case CH_CTRLY:
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL) {
	tempb=doc_ce->user_data;
	if (tempt=tempb->template) {
	  QueRem(tempt);
	  BgtTemplatePurge(tempt);
	  BgtEntryDel2(&tempt->b);
	  Free(tempt);
	} else {
	  QueRem(tempb);
	  BgtEntryDel(tempb);
	}
	BgtRegen;
      }
      return TRUE;
    case 'a':
      PopUpOk(	"Set the name and color of your accounts.\n"
	    "To delete accounts, manually edit\n"
	    "$$GREEN$$~/Budget/Accts.TXT.Z$$FG$$.");
      if (PopUpEd(BGT_ACCTS_FILE,Fs)) {
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
      if (tempb1=BgtEntryPmt) {
	BgtIns(tempb1);
	BgtRegen;
      }
      return TRUE;
    case 't':
      if (tempt1=BgtTemplatePmt) {
	QueIns(tempt1,t_head.last);
	BgtTemplateExpand(tempt1);
	BgtRegen;
      }
      return TRUE;
    case 'c':
      if ((doc_ce=doc->cur_entry) && doc_ce!=doc &&
	    doc_ce->type_u8==DOCT_MENU_VAL)
	tempb=doc_ce->user_data;
      else
	tempb=NULL;
      if (tempb1=BgtEntryPmt(tempb)) {
	BgtIns(tempb1);
	BgtRegen;
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
  CBgtEntry *tempb=b_head.next,*tempb_ce;
  doc=DocNew;
  doc->flags|=DOCF_FORM;
  while (tempb!=&b_head) {
    if (!StrCmp(view_acct,tempb->credit))
      balance-=tempb->amount;
    if (!StrCmp(view_acct,tempb->debit))
      balance+=tempb->amount;
    c=BgtAcctColor(tempb->credit);
    if (c!=color) {
      color=c;
      DocPrint(doc,"$$FG,%d$$",color);
    }
    tempb->doc_e=DocPrint(doc,
	  "$$MU-UL,\"%D %8ts %8ts:%8.2f %8.2f:%$$Q\",U=0x%X$$\n",
	  tempb->date,tempb->credit,tempb->debit,balance,
	  tempb->amount,tempb->desc,tempb);
    tempb=tempb->next;
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
      while (doc_ce->type_u8!=DOCT_MENU_VAL || !(tempb_ce=doc_ce->user_data)) {
	doc_ce=doc_ce->next;
	if (doc_ce==pdoc) goto br_cont;
      }
      tempb=b_head.next;
      while (tempb!=&b_head) {
	if (tempb==tempb_ce) {
	  doc->cur_entry=tempb->doc_e;
	  doc->cur_col=0;
	  DocCenter(doc);
	  goto br_cont;
	}
	tempb=tempb->next;
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

U0 Budget()
{
  CDoc *pdoc,*ddoc,*old_put,*old_display;
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
