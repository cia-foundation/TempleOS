U0 AsmResolve(CCmpCtrl *cc,CAsmUnresolvedRef *tmpu,U8 *label,Bool undefined)
{
  CAOTImportExport *tmpie;
  CAsmUndefHash *tmpauh;
  I64 res=Call(tmpu->machine_code);
  if (undefined) {
    tmpauh=tmpu->asm_undef_hash;
    while (tmpauh) {
      if (tmpauh->hash->type & HTF_UNRESOLVED) {
	tmpie=CAlloc(sizeof(CAOTImportExport));
	tmpie->next=tmpauh->hash->ie_lst;
	tmpauh->hash->ie_lst=tmpie;
	tmpie->rip=tmpu->rip;
	tmpie->aot=cc->aot;
	tmpie->type=tmpu->type;
      }
      tmpauh=tmpauh->next; //Technically, more than one won't work.
    }
  } else if (!(tmpu->type&IEF_IMM_NOT_REL)) {
    res-=tmpu->rel_rip;
    if (tmpu->type==IET_REL_I8 && !(I8_MIN<=res<=I8_MAX) ||
	  tmpu->type==IET_REL_I16 && !(I16_MIN<=res<=I16_MAX)) {
      PrintErr("Branch out of range at line:%04d %s\n",
	    tmpu->line_num,label);
      LexExcept(cc);
    }
    if (tmpu->U8_avail && tmpu->type>IET_IMM_U8 && -124<=res<=123)
      PrintWarn("could use I8 displacement at line:%04d %s %s\n",
	    tmpu->line_num,cc->aotc->last_label,label);
  }
  AOTStoreCodeU8At(cc,tmpu->rip,res.u8[0]);
  if (tmpu->type>=IET_REL_I16) {
    AOTStoreCodeU8At(cc,tmpu->rip+1,res.u8[1]);
    if (tmpu->type>=IET_REL_I32) {
      AOTStoreCodeU8At(cc,tmpu->rip+2,res.u8[2],res.u8[3]);
      if (tmpu->type>=IET_REL_I64)
	AOTStoreCodeU8At(cc,tmpu->rip+4,res.u8[4],res.u8[5],
	      res.u8[6],res.u8[7]);
    }
  }
}

U0 AOTLocalsResolve(CCmpCtrl *cc)
{
  CAOTCtrl *aotc=cc->aotc;
  CAsmUnresolvedRef *tmpu=aotc->local_unresolved,*tmpu1;
  CAsmUndefHash *tmpauh;
  Bool undefined;
  U8 *label=NULL;

  while (tmpu) {
    undefined=FALSE;
    tmpu1=tmpu->next;
    tmpauh=tmpu->asm_undef_hash;
    while (tmpauh) {
      if (tmpauh->hash->type & HTF_UNRESOLVED) {
	PrintErr("Undefined sym at line:%04d %s %s\n",
	      tmpu->line_num,aotc->last_label,tmpauh->hash->str);
	LexExcept(cc);
      }
      label=tmpauh->hash->str;
      tmpauh=tmpauh->next;
    }
    if (!undefined)
      AsmResolve(cc,tmpu,label,FALSE);
    Free(tmpu->machine_code);
    LinkedLstDel(tmpu->asm_undef_hash);
    Free(tmpu);
    tmpu=tmpu1;
  }
  HashTableDel(cc->htc.local_hash_table);
  cc->htc.hash_table_lst=cc->htc.local_hash_table=HashTableNew(16);
  cc->htc.local_hash_table->next=cc->htc.glbl_hash_table;
  aotc->local_unresolved=NULL;
}

U0 AOTGlblsResolve(CCmpCtrl *cc,CAOT *tmpaot)
{
  CAOTCtrl *aotc=cc->aotc;
  CHashFun *tmpf;
  CAsmUnresolvedRef *tmpu=aotc->glbl_unresolved,*tmpu1;
  I64 i,j;
  CAOTImportExport *tmpie,*tmpie1;
  CAsmUndefHash *tmpauh;
  CHashExport *tmpex;
  U8 *label;
  Bool undefined;
  CExternUsage *tmpeu,*tmpeu8;

  while (tmpu) {
    label=NULL;
    undefined=FALSE;
    tmpu1=tmpu->next;
    tmpauh=tmpu->asm_undef_hash;
    while (tmpauh) {
      if (tmpauh->hash->type & HTF_UNRESOLVED) {
	tmpex=tmpauh->hash;
	if (tmpex->type & HTT_EXPORT_SYS_SYM&& tmpex->type & HTF_UNRESOLVED &&
	      !(tmpex->type & HTF_IMPORT) &&
	      (tmpf=HashFind(tmpex->str,cc->htc.hash_table_lst,HTT_FUN)) &&
	      !Bt(&tmpf->flags,Cf_EXTERN)) {
	  tmpex->val=tmpf->exe_addr;
	  tmpex->type&=~HTF_UNRESOLVED;
	  label=tmpauh->hash->str;
	} else {
	  if (!(tmpex->type & HTF_IMPORT)) {
	    if (cc->htc.local_var_lst) {
	      tmpex->type|=HTF_GOTO_LABEL;
	      tmpex->use_cnt++;
	    } else {
	      PrintErr("Undefined sym at line:%04d %s\n",
		    tmpu->line_num,tmpex->str);
	      LexExcept(cc);
	    }
	  } else if (undefined) {
	    PrintErr("Two imports in same expression "
		  "not allowed at line:%04d %s\n",
		  tmpu->line_num,tmpex->str);
	    LexExcept(cc);
	  }
	  undefined=TRUE;
	}
      } else
	label=tmpauh->hash->str;
      tmpauh=tmpauh->next;
    }
    AsmResolve(cc,tmpu,label,undefined);
    Free(tmpu->machine_code);
    LinkedLstDel(tmpu->asm_undef_hash);
    Free(tmpu);
    tmpu=tmpu1;
  }

  for (i=0;i<=cc->htc.glbl_hash_table->mask;i++) {
    tmpex=cc->htc.glbl_hash_table->body[i];
    while (tmpex) {
      if (tmpex->type & (HTF_IMPORT|HTF_GOTO_LABEL)) {
	if (tmpex->use_cnt && (tmpie=tmpex->ie_lst)) {
	  if (tmpex->type&HTF_GOTO_LABEL)
	    tmpie->flags|=IEF_GOTO_LABEL;
	  if (tmpex->import_name)
	    tmpie->str=StrNew(tmpex->import_name);
	  else
	    tmpie->str=StrNew(tmpex->str);
	  do {
	    tmpie1=tmpie->next;
	    QueIns(tmpie,tmpaot->last_ie);
	  } while (tmpie=tmpie1);
	  tmpex->ie_lst=NULL;
	}
      } else if (tmpex->type & (HTF_EXPORT|HTF_RESOLVE)) {
	if (tmpex->type & HTF_UNRESOLVED) {
	  PrintErr("Undefined sym at %s\n",tmpex->str);
	  LexExcept(cc);
	}
	if (tmpex->type & HTF_RESOLVE) {
	  tmpf=tmpex;
	  tmpeu=tmpf->ext_lst;
	  while (tmpeu) {
	    tmpeu8=tmpeu->next;
	    j=tmpf->exe_addr-(tmpeu->rip+4);
	    AOTStoreCodeU8At(cc,tmpeu->rip,j.u8[0],j.u8[1],j.u8[2],j.u8[3]);
	    Free(tmpeu);
	    tmpeu=tmpeu8;
	  }
	}
	if (tmpex->type & HTF_EXPORT) {
	  tmpie=CAlloc(sizeof(CAOTImportExport));
	  tmpie->type=IET_REL32_EXPORT;
	  if (tmpex->type & HTT_FUN)
	    tmpie->rip=tmpf->exe_addr;
	  else if (tmpex->type & HTT_GLBL_VAR)
	    tmpie->rip=tmpex(CHashGlblVar *)->data_addr_rip;
	  else
	    tmpie->rip=tmpex->val;
	  tmpie->aot=cc->aot;
	  if (tmpex->type & HTF_IMM)
	    tmpie->type++;
	  tmpie->str=StrNew(tmpex->str);
	  tmpie->src_link=StrNew(tmpex->src_link);
	  QueIns(tmpie,tmpaot->last_ie);
	}
      }
      tmpex=tmpex->next;
    }
  }
}

U0 AsmUnresolvedAdd(CCmpCtrl *cc,U8 *machine_code,I64 type,I64 rip,I64 rel_rip,
	CAsmUndefHash *local_asm_undef_hash,CAsmUndefHash *glbl_asm_undef_hash,
	I64 line_num,Bool U8_avail)
{
  CAsmUnresolvedRef *tmpu=MAlloc(sizeof(CAsmUnresolvedRef));
  tmpu->machine_code=machine_code;
  tmpu->type=type;
  tmpu->rip=rip;
  tmpu->rel_rip=rel_rip;
  tmpu->aot=cc->aot;
  tmpu->imm_flag=FALSE;
  tmpu->line_num=line_num;
  tmpu->U8_avail=U8_avail;
  tmpu->str=NULL;
  if (local_asm_undef_hash) {
    tmpu->asm_undef_hash=local_asm_undef_hash;
    tmpu->next=cc->aotc->local_unresolved;
    cc->aotc->local_unresolved=tmpu;
  } else {
    tmpu->asm_undef_hash=glbl_asm_undef_hash;
    tmpu->next=cc->aotc->glbl_unresolved;
    cc->aotc->glbl_unresolved=tmpu;
    if (glbl_asm_undef_hash->hash->type & HTF_IMPORT) {
      tmpu->str=StrNew(glbl_asm_undef_hash->hash->str);
      if (glbl_asm_undef_hash->hash->type & HTF_IMM)
	tmpu->imm_flag=TRUE;
    }
  }
}
