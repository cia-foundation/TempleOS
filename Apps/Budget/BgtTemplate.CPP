U0 BgtTemplatePurge(CBgtTemplate *tempt)
{
  CBgtEntry	*tempb,*tempb1;
  tempb=b_root.next;
  while (tempb!=&b_root) {
    tempb1=tempb->next;
    if (tempb->template==tempt) {
      QueRem(tempb);
      BgtEntryDel2(tempb);
      Free(tempb);
    }
    tempb=tempb1;
  }
}

U0 BgtTemplateExpand(CBgtTemplate *tempt)
{
  CDate		d,start,end;
  CDateStruct	ds;
  CBgtEntry	*tempb;

  start=MyStr2Date(tempt->start_date);
  end  =MyStr2Date(tempt->end_date);
  tempt->b.template=tempt;
  switch (tempt->type) {
    case BT_INTERVAL:
      d=start;
      while (d<=end) {
	tempb=CAlloc(sizeof(CBgtEntry));
	MemCpy(tempb,&tempt->b,sizeof(CBgtEntry));
	tempb->date=d;
	tempb->type=BE_TEMPLATE_COPY;
	BgtIns(tempb);
	d+=tempt->period*0x100000000;
      }
      break;
    case BT_MONTHLY:
      Date2Struct(&ds,start);
      while (TRUE) {
	d=Struct2Date(&ds);
	if (d<=end) {
	  tempb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tempb,&tempt->b,sizeof(CBgtEntry));
	  tempb->date=d;
	  tempb->type=BE_TEMPLATE_COPY;
	  BgtIns(tempb);
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
	  tempb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tempb,&tempt->b,sizeof(CBgtEntry));
	  tempb->date=d;
	  tempb->type=BE_TEMPLATE_COPY;
	  BgtIns(tempb);
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
	  tempb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tempb,&tempt->b,sizeof(CBgtEntry));
	  tempb->date=d;
	  tempb->type=BE_TEMPLATE_COPY;
	  BgtIns(tempb);
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
	  tempb=CAlloc(sizeof(CBgtEntry));
	  MemCpy(tempb,&tempt->b,sizeof(CBgtEntry));
	  tempb->date=d;
	  tempb->type=BE_TEMPLATE_COPY;
	  BgtIns(tempb);
	} else
	  break;
	ds.year++;
      }
      break;
  }
}

U0 CBgtTemplatesExpand()
{
  CBgtTemplate	*tempt=t_root.next;
  while (tempt!=&t_root) {
    BgtTemplateExpand(tempt);
    tempt=tempt->next;
  }
}

CBgtTemplate *BgtTemplatePmt(CBgtTemplate *dft_t=NULL)
{
  CBgtTemplate	t,*tempt;
  CBgtEntry	*tempb,*dft_b;
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
      if (tempb=BgtEntryPmt(dft_b)) {
	tempt=CAlloc(sizeof(CBgtTemplate));
	MemCpy(&tempt->start,&t.start,BT_SIZE);
	MemCpy(&tempt->b,tempb,sizeof(CBgtEntry));
	Free(tempb);
	return tempt;
      }
    } else
      break;
  return NULL;
}
