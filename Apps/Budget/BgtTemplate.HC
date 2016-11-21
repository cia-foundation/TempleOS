U0 BgtTemplatePurge(CBgtTemplate *tmpt)
{
  CBgtEntry	*tmpb,*tmpb1;
  tmpb=b_head.next;
  while (tmpb!=&b_head) {
    tmpb1=tmpb->next;
    if (tmpb->template==tmpt) {
      QueRem(tmpb);
      BgtEntryDel2(tmpb);
      Free(tmpb);
    }
    tmpb=tmpb1;
  }
}

U0 BgtTemplateDel(CBgtTemplate *tmpt)
{
  BgtTemplatePurge(tmpt);
  BgtEntryDel2(&tmpt->b);
  Free(tmpt);
}

U0 BgtTemplateExpand(CBgtTemplate *tmpt,Bool periodic_copy=FALSE)
{
  CDate		d,start,end;
  CDateStruct	ds;
  CBgtEntry	*tmpb;
  Bool		first=TRUE;

  start=MyStr2Date(tmpt->start_date);
  end  =MyStr2Date(tmpt->end_date);
  tmpt->b.template=tmpt;
  switch (tmpt->type) {
    case BT_INTERVAL:
      d=start;
      while (d<=end) {
	if (!first || !periodic_copy) {
	  tmpb=BgtEntryCopy(&tmpt->b,periodic_copy);
	  tmpb->date=d;
	  if (periodic_copy)
	    tmpb->template=NULL;
	  else
	    tmpb->type=BE_TEMPLATE_COPY;
	  BgtIns(tmpb);
	}
	d+=tmpt->period*0x100000000;
	first=FALSE;
      }
      break;
    case BT_MONTHLY:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  if (!first || !periodic_copy) {
	    tmpb=BgtEntryCopy(&tmpt->b,periodic_copy);
	    tmpb->date=d;
	    if (periodic_copy)
	      tmpb->template=NULL;
	    else
	      tmpb->type=BE_TEMPLATE_COPY;
	    BgtIns(tmpb);
	  }
	} else
	  break;
	if (++ds.mon>12) {
	  ds.mon=1;
	  ds.year++;
	}
	first=FALSE;
      }
      break;
    case BT_BIMONTHLY:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  if (!first || !periodic_copy) {
	    tmpb=BgtEntryCopy(&tmpt->b,periodic_copy);
	    tmpb->date=d;
	    if (periodic_copy)
	      tmpb->template=NULL;
	    else
	      tmpb->type=BE_TEMPLATE_COPY;
	    BgtIns(tmpb);
	  }
	} else
	  break;
	ds.mon+=2;
	if (ds.mon>12) {
	  ds.mon-=12;
	  ds.year++;
	}
	first=FALSE;
      }
      break;
    case BT_SEMIANNUAL:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  if (!first || !periodic_copy) {
	    tmpb=BgtEntryCopy(&tmpt->b,periodic_copy);
	    tmpb->date=d;
	    if (periodic_copy)
	      tmpb->template=NULL;
	    else
	      tmpb->type=BE_TEMPLATE_COPY;
	    BgtIns(tmpb);
	  }
	} else
	  break;
	ds.mon+=6;
	if (ds.mon>12) {
	  ds.mon-=12;
	  ds.year++;
	}
	first=FALSE;
      }
      break;
    case BT_ANNUAL:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  if (!first || !periodic_copy) {
	    tmpb=BgtEntryCopy(&tmpt->b,periodic_copy);
	    tmpb->date=d;
	    if (periodic_copy)
	      tmpb->template=NULL;
	    else
	      tmpb->type=BE_TEMPLATE_COPY;
	    BgtIns(tmpb);
	  }
	} else
	  break;
	ds.year++;
	first=FALSE;
      }
      break;
  }
}

U0 CBgtTemplatesExpand()
{
  CBgtTemplate	*tmpt=t_head.next;
  while (tmpt!=&t_head) {
    BgtTemplateExpand(tmpt);
    tmpt=tmpt->next;
  }
}

CBgtTemplate *BgtTemplatePmt(CBgtTemplate *dft_t=NULL,CBgtEntry *dft_b=NULL)
{
  CBgtTemplate	t,*tmpt;
  CBgtEntry	*tmpb;
  MemSet(&t,0,sizeof(CBgtTemplate));
  if (dft_t) {
    MemCpy(&t.start,&dft_t->start,BT_SIZE);
    dft_b=&dft_t->b;
  } else {
    t.type=BT_INTERVAL;
    if (dft_b)
      StrPrint(&t.start_date,"%D",dft_b->date);
    else
      StrCpy(&t.start_date,"[");
    StrCpy(&t.end_date,"]");
  }
  while (TRUE)
    if (PopUpForm(&t) && ((t.type==BT_INTERVAL && t.period>0) ||
	  t.type>BT_INTERVAL)) {
      if (tmpb=BgtEntryPmt(dft_b)) {
	tmpt=CAlloc(sizeof(CBgtTemplate));
	MemCpy(&tmpt->start,&t.start,BT_SIZE);
	MemCpy(&tmpt->b,tmpb,sizeof(CBgtEntry));
	Free(tmpb);
	return tmpt;
      }
    } else
      break;
  return NULL;
}
