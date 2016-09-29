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

U0 BgtTemplateExpand(CBgtTemplate *tmpt)
{
  CDate		d,start,end;
  CDateStruct	ds;
  CBgtEntry	*tmpb;

  start=MyStr2Date(tmpt->start_date);
  end  =MyStr2Date(tmpt->end_date);
  tmpt->b.template=tmpt;
  switch (tmpt->type) {
    case BT_INTERVAL:
      d=start;
      while (d<=end) {
	tmpb=CAlloc(sizeof(CBgtEntry));
	MemCpy(tmpb,&tmpt->b,sizeof(CBgtEntry));
	tmpb->date=d;
	tmpb->type=BE_TEMPLATE_COPY;
	BgtIns(tmpb);
	d+=tmpt->period*0x100000000;
      }
      break;
    case BT_MONTHLY:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  tmpb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tmpb,&tmpt->b,sizeof(CBgtEntry));
	  tmpb->date=d;
	  tmpb->type=BE_TEMPLATE_COPY;
	  BgtIns(tmpb);
	} else
	  break;
	if (++ds.mon>11) {
	  ds.mon=0;
	  ds.year++;
	}
      }
      break;
    case BT_BIMONTHLY:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  tmpb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tmpb,&tmpt->b,sizeof(CBgtEntry));
	  tmpb->date=d;
	  tmpb->type=BE_TEMPLATE_COPY;
	  BgtIns(tmpb);
	} else
	  break;
	ds.mon+=2;
	if (ds.mon>11) {
	  ds.mon-=12;
	  ds.year++;
	}
      }
      break;
    case BT_SEMIANNUAL:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  tmpb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tmpb,&tmpt->b,sizeof(CBgtEntry));
	  tmpb->date=d;
	  tmpb->type=BE_TEMPLATE_COPY;
	  BgtIns(tmpb);
	} else
	  break;
	ds.mon+=6;
	if (ds.mon>11) {
	  ds.mon-=12;
	  ds.year++;
	}
      }
      break;
    case BT_ANNUAL:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  tmpb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tmpb,&tmpt->b,sizeof(CBgtEntry));
	  tmpb->date=d;
	  tmpb->type=BE_TEMPLATE_COPY;
	  BgtIns(tmpb);
	} else
	  break;
	ds.year++;
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

CBgtTemplate *BgtTemplatePmt(CBgtTemplate *dft_t=NULL)
{
  CBgtTemplate	t,*tmpt;
  CBgtEntry	*tmpb,*dft_b;
  MemSet(&t,0,sizeof(CBgtTemplate));
  if (dft_t) {
    MemCpy(&t.start,&dft_t->start,BT_SIZE);
    dft_b=&dft_t->b;
  } else {
    t.type=BT_INTERVAL;
    StrCpy(&t.start_date,"[");
    StrCpy(&t.end_date,"]");
    dft_b=NULL;
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
