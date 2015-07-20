U0 BgtAcctsUpdate()
{
  CBgtEntry	*tempb;
  CBgtTemplate	*tempt;
  tempb=b_root.next;
  while (tempb!=&b_root) {
    if (tempb->type!=BE_TEMPLATE_COPY) {
      tempb->credit_index=StrFileAdd(tempb->credit,
	    &accts_table_strs,accts_table);
      tempb->debit_index =StrFileAdd(tempb->debit,
	    &accts_table_strs,accts_table);
    }
    tempb=tempb->next;
  }
  tempt=t_root.next;
  while (tempt!=&t_root) {
    tempt->b.credit_index=StrFileAdd(tempt->b.credit,
	  &accts_table_strs,accts_table);
    tempt->b.debit_index =StrFileAdd(tempt->b.debit,
	  &accts_table_strs,accts_table);
    tempt=tempt->next;
  }
}

U0 BgtAcctsWrite()
{
  BgtAcctsUpdate;
  StrFileWrite(BGT_ACCTS_FILE,accts_table,TRUE);
}

U0 BgtAcctsRead()
{
  I64	i,max_num;
  U8	*colors,**s=StrFileRead(BGT_ACCTS_FILE,&max_num,&colors,TRUE);
  StrFileDel(accts_table);
  accts_table=HashTableNew(512);
  accts_table_strs=0;
  for (i=0;i<=max_num;i++)
    if (s[i])
      StrFileAdd(s[i],&accts_table_strs,accts_table,colors[i]);
  StrFileArrDel(s,max_num);
  Free(colors);
}

I64 BgtAcctColor(U8 *st)
{
  CHashGeneric *temph;
  if (temph=HashFind(st,accts_table,SFT_GENERIC))
    return temph->user_data1;
  else
    return BLACK;
}

U8 *BgtPopUpAcct(U8 *header=NULL,U8 *dft=NULL)
{
  I64 i;
  U8 *result;
  CDoc *doc=DocNew;
  CDocEntry *doc_e,*doc_dft=NULL;
  CHashGeneric *temph;

  if (header)
    DocPrint(doc,"%s",header);

  for (i=0;i<=accts_table->mask;i++) {
    temph=accts_table->body[i];
    while (temph) {
      doc_e=DocPrint(doc,"$$FG,%d$$$$MU-UL,\"%s\",LE=0x%X$$\n",
	    temph->user_data1,temph->str,temph->str);
      if (dft && !StrCmp(dft,temph->str))
	doc_dft=doc_e;
      temph=temph->next;
    }
  }

  if (doc_dft) {
    doc->cur_entry=doc_dft;
    doc->cur_col=0;
  }
  result=PopUpMenu(doc,DOF_DONT_HOME);
  DocDel(doc);
  return result;
}
