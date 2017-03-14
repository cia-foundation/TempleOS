U0 BgtAcctsUpdate()
{
  CBgtEntry	*tmpb;
  CBgtTemplate	*tmpt;
  tmpb=b_head.next;
  while (tmpb!=&b_head) {
    if (tmpb->type!=BE_TEMPLATE_COPY) {
      tmpb->credit_idx=StrFileAdd(tmpb->credit,
	    &accts_table_strs,accts_table);
      tmpb->debit_idx =StrFileAdd(tmpb->debit,
	    &accts_table_strs,accts_table);
    }
    tmpb=tmpb->next;
  }
  tmpt=t_head.next;
  while (tmpt!=&t_head) {
    tmpt->b.credit_idx=StrFileAdd(tmpt->b.credit,
	  &accts_table_strs,accts_table);
    tmpt->b.debit_idx =StrFileAdd(tmpt->b.debit,
	  &accts_table_strs,accts_table);
    tmpt=tmpt->next;
  }
}

U0 BgtAcctsWrite()
{
  BgtAcctsUpdate;
  StrFileWrite(bgt_accts_file,accts_table,TRUE);
}

U0 BgtAcctsRead()
{
  I64	i,max_num;
  U8	*colors,**s=StrFileRead(bgt_accts_file,&max_num,&colors,TRUE);
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
  CHashGeneric *tmph;
  if (tmph=HashFind(st,accts_table,SFT_GENERIC))
    return tmph->user_data1;
  else
    return BLACK;
}

U8 *BgtPopUpAcct(U8 *header=NULL,U8 *dft=NULL)
{
  I64 i;
  U8 *res;
  CDoc *doc=DocNew;
  CDocEntry *doc_e,*doc_dft=NULL;
  CHashGeneric *tmph;

  if (header)
    DocPrint(doc,"%s",header);

  for (i=0;i<=accts_table->mask;i++) {
    tmph=accts_table->body[i];
    while (tmph) {
      doc_e=DocPrint(doc,"$$FG,%d$$$$MU-UL,\"%s\",LE=0x%X$$\n",
	    tmph->user_data1,tmph->str,tmph->str);
      if (dft && !StrCmp(dft,tmph->str))
	doc_dft=doc_e;
      tmph=tmph->next;
    }
  }

  if (doc_dft) {
    doc->cur_entry=doc_dft;
    doc->cur_col=0;
  }
  res=PopUpMenu(doc,DOF_DONT_HOME);
  DocDel(doc);
  return res;
}
