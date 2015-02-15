U0 BgtEntryDel2(CBgtEntry *tempb)
{
  if (tempb->type!=BE_TEMPLATE_COPY) {
    Free(tempb->credit);
    Free(tempb->debit);
    Free(tempb->desc);
  }
}

U0 BgtEntryDel(CBgtEntry *tempb)
{
  BgtEntryDel2(tempb);
  Free(tempb);
}

U0 BgtIns(CBgtEntry *tempb)
{
  CBgtEntry	*tempb1=b_root.next;
  while (tempb1!=&b_root && tempb1->date<tempb->date)
    tempb1=tempb1->next;
  QueIns(tempb,tempb1->last);
}

class CBgtEntryForm
{
  U8	date[512]	fmtstr "$$DA-P,511,A=\"Date       :%s\"$$\n";
  F64	amount		fmtstr "Amount     $$$$$$DA,511,A=\"%6.2f\"$$\n";
  U8	credit[512]	fmtstr "$$DA-P,511,A=\"Credit Acct:%s\"$$\n";
  U8	debit [512]	fmtstr "$$DA-P,511,A=\"Debit  Acct:%s\"$$\n";
  U8	desc  [512]	fmtstr "$$DA-P,511,A=\"Desc       :%s\"$$\n";
};

CBgtEntry *BgtEntryPmt(CBgtEntry *dft=NULL)
{
  CBgtEntryForm b;
  CBgtEntry *tempb;
  U8 *st;
  MemSet(&b,0,sizeof(CBgtEntryForm));
  StrCpy(&b.date,"*");
  if (dft) {
    StrPrint(b.date,"%D",dft->date);
    b.amount=dft->amount;
    StrCpy(b.credit,dft->credit);
    StrCpy(b.debit ,dft->debit);
    StrCpy(b.desc  ,dft->desc);
  }
  while (TRUE)
    if (PopUpForm(&b)) {
      if (!*b.credit) {
	st=BgtPopUpAcct("Credit Acct\n\n");
	if (st!=DOCM_CANCEL)
	  StrCpy(b.credit,st);
      } else if (!*b.debit) {
	st=BgtPopUpAcct("Debit Acct\n\n");
	if (st!=DOCM_CANCEL)
	  StrCpy(b.debit,st);
      } else {
	tempb=CAlloc(sizeof(CBgtEntry));
	tempb->date  =Str2Date(b.date);
	tempb->amount=b.amount;
	tempb->credit=StrNew(b.credit);
	tempb->debit =StrNew(b.debit);
	tempb->desc  =StrNew(b.desc);
	tempb->type  =BE_NORMAL;
	StrFileAdd(tempb->credit,&accts_table_strs,accts_table);
	StrFileAdd(tempb->debit,&accts_table_strs,accts_table);
	return tempb;
      }
    } else
      return NULL;
}
