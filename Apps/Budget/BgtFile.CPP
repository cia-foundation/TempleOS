U0 BgtDataRead()
{
  CBgtEntry	*tempb;
  CBgtTemplate	*tempt;
  I64		i,cnt,size;
  U8		*b,*ptr;
  I64		max_num;
  U8		**s=StrFileRead(BGT_STRINGS_FILE,&max_num);

  MemSet(&b_root,0,sizeof(CBgtEntry));
  QueInit(&b_root);
  b_root.date=Now;
  MemSet(&t_root,0,sizeof(CBgtTemplate));
  QueInit(&t_root);
  t_root.b.date=Now;

  if (ptr=b=FileRead(BGT_DATA_FILE,&size)) {
    cnt=*ptr(I64 *)++;
    for (i=0;i<cnt;i++) {
      tempb=CAlloc(sizeof(CBgtEntry));
      MemCpy(&tempb->start,ptr,BE_SIZE);
      tempb->credit=StrNew(s[tempb->credit_index]);
      tempb->debit =StrNew(s[tempb->debit_index]);
      tempb->desc  =StrNew(s[tempb->desc_index]);
      QueIns(tempb,b_root.last);
      ptr+=BE_SIZE;
    }

    cnt=*ptr(I64 *)++;
    for (i=0;i<cnt;i++) {
      tempt=CAlloc(sizeof(CBgtTemplate));
      MemCpy(&tempt->start,ptr,BT_SIZE);
      ptr+=BT_SIZE;
      MemCpy(&tempt->b.start,ptr,BE_SIZE);
      ptr+=BE_SIZE;
      tempt->b.credit=StrNew(s[tempt->b.credit_index]);
      tempt->b.debit =StrNew(s[tempt->b.debit_index]);
      tempt->b.desc  =StrNew(s[tempt->b.desc_index]);
      QueIns(tempt,t_root.last);
    }
  }

  StrFileArrDel(s,max_num);
  Free(b);

  BgtAcctsUpdate;
}

U0 BgtDataWrite()
{
  I64 i,num=0,size,cnt1,cnt2;
  CHashTable	*table=HashTableNew(1024);
  CBgtEntry	*tempb;
  CBgtTemplate	*tempt;
  CHashGeneric *temph;
  U8 *buf,*ptr;

  for (i=0;i<=accts_table->mask;i++) {
    temph=accts_table->body[i];
    while (temph) {
      StrFileAdd(temph->str,&num,table); //Cosmetics -- make accts appear first.
      temph=temph->next;
    }
  }

  tempb=b_root.next;
  cnt1=0;
  while (tempb!=&b_root) {
    if (tempb->type!=BE_TEMPLATE_COPY) {
      tempb->credit_index	=StrFileAdd(tempb->credit,&num,table);
      tempb->debit_index	=StrFileAdd(tempb->debit,&num,table);
      tempb->desc_index		=StrFileAdd(tempb->desc,&num,table);
      cnt1++;
    }
    tempb=tempb->next;
  }

  tempt=t_root.next;
  cnt2=0;
  while (tempt!=&t_root) {
    tempt->b.credit_index	=StrFileAdd(tempt->b.credit,&num,table);
    tempt->b.debit_index	=StrFileAdd(tempt->b.debit,&num,table);
    tempt->b.desc_index		=StrFileAdd(tempt->b.desc,&num,table);
    cnt2++;
    tempt=tempt->next;
  }
  StrFileWrite(BGT_STRINGS_FILE,table);
  StrFileDel(table);

  size=sizeof(I64)*2+cnt1*BE_SIZE+cnt2*(BT_SIZE+BE_SIZE);
  buf=ptr=MAlloc(size);

  MemCpy(ptr,&cnt1,sizeof(I64));
  ptr+=sizeof(I64);
  tempb=b_root.next;
  while (tempb!=&b_root) {
    if (tempb->type!=BE_TEMPLATE_COPY) {
      MemCpy(ptr,&tempb->start,BE_SIZE);
      ptr+=BE_SIZE;
    }
    tempb=tempb->next;
  }

  MemCpy(ptr,&cnt2,sizeof(I64));
  ptr+=sizeof(I64);
  tempt=t_root.next;
  while (tempt!=&t_root) {
    MemCpy(ptr,&tempt->start,BT_SIZE);
    ptr+=BT_SIZE;
    MemCpy(ptr,&tempt->b.start,BE_SIZE);
    ptr+=BE_SIZE;
    tempt=tempt->next;
  }

  FileWrite(BGT_DATA_FILE,buf,size);
  Free(buf);
}

U0 BgtDel()
{
  CBgtEntry	*tempb,*tempb1;
  CBgtTemplate	*tempt,*tempt1;
  tempb=b_root.next;
  while (tempb!=&b_root) {
    tempb1=tempb->next;
    BgtEntryDel2(tempb);
    Free(tempb);
    tempb=tempb1;
  }
  tempt=t_root.next;
  while (tempt!=&t_root) {
    tempt1=tempt->next;
    BgtEntryDel2(&tempt->b);
    Free(tempt);
    tempt=tempt1;
  }
  StrFileDel(accts_table);
  accts_table=NULL;
  accts_table_strs=0;
}
