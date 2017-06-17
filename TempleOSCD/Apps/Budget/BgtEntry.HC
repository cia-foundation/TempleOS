U0 BgtEntryDel2(CBgtEntry *tmpb)
{
  if (tmpb->type!=BE_TEMPLATE_COPY) {
    Free(tmpb->credit);
    Free(tmpb->debit);
    Free(tmpb->desc);
  }
}

CBgtEntry *BgtEntryCopy(CBgtEntry *tmpb,Bool periodic_copy)
{
  CBgtEntry *res=MAlloc(sizeof(CBgtEntry));
  MemCpy(res,tmpb,sizeof(CBgtEntry));
  if (periodic_copy) {
    res->credit=StrNew(tmpb->credit);
    res->debit =StrNew(tmpb->debit);
    res->desc  =StrNew(tmpb->desc);
  }
  return res;
}

U0 BgtEntryDel(CBgtEntry *tmpb)
{
  BgtEntryDel2(tmpb);
  Free(tmpb);
}

U0 BgtIns(CBgtEntry *tmpb)
{
  CBgtEntry	*tmpb1=b_head.next;
  while (tmpb1!=&b_head && tmpb1->date<tmpb->date)
    tmpb1=tmpb1->next;
  QueIns(tmpb,tmpb1->last);
}

class CBgtEntryForm
{
  U8	date[512]	format "$$DA-P,A=\"Date              :%s\"$$\n";
  F64	amount		format "Amount            $$$$$$DA,A=\"%10.2f\"$$\n";
  U8	credit[512]	format "$$DA-P,A=\"Credit (from) Acct:%s\"$$\n";
  U8	debit [512]	format "$$DA-P,A=\"Debit  (to)   Acct:%s\"$$\n";
  U8	desc  [512]	format "$$DA-P,A=\"Desc              :%s\"$$\n";
};

CBgtEntry *BgtEntryPmt(CBgtEntry *dft=NULL)
{
  CBgtEntryForm b;
  CBgtEntry *tmpb;
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
	tmpb=CAlloc(sizeof(CBgtEntry));
	tmpb->date  =Str2Date(b.date);
	tmpb->amount=b.amount;
	tmpb->credit=StrNew(b.credit);
	tmpb->debit =StrNew(b.debit);
	tmpb->desc  =StrNew(b.desc);
	tmpb->type  =BE_NORMAL;
	StrFileAdd(tmpb->credit,&accts_table_strs,accts_table);
	StrFileAdd(tmpb->debit,&accts_table_strs,accts_table);
	return tmpb;
      }
    } else
      return NULL;
}
