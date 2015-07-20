U0 AsmResolve(CCmpCtrl *cc,CAsmUnresolvedRef *tempu,U8 *label,Bool undefined)
{
  CAOTImportExport *tempie;
  CAsmUndefHash *tempauh;
  I64 r=Call(tempu->machine_code);
  if (undefined) {
    tempauh=tempu->asm_undef_hash;
    while (tempauh) {
      if (tempauh->hash->type & HTF_UNRESOLVED) {
	tempie=CAlloc(sizeof(CAOTImportExport));
	tempie->next=tempauh->hash->ie_lst;
	tempauh->hash->ie_lst=tempie;
	tempie->ip=tempu->ip;
	tempie->aot=cc->aot;
	tempie->type=tempu->type;
      }
      tempauh=tempauh->next; //Technically, more than one won't work.
    }
  } else if (!(tempu->type&1))	{//Rel?
    r-=tempu->rel_ip;
    if (tempu->type==IET_REL_I8 && !(MIN_I8<=r<=MAX_I8) ||
	  tempu->type==IET_REL_I16 && !(MIN_I16<=r<=MAX_I16)) {
      PrintErr("branch out of range at line:%04d %s\n",
	    tempu->line_num,label);
      LexExcept(cc);
    }
    if (tempu->U8_avail && tempu->type>IET_IMM_U8 && -124<=r<=123)
      PrintWarn("could use I8 displacement at line:%04d %s %s\n",
	    tempu->line_num,cc->aotc->last_label,label);
  }
  AOTStoreCodeU8At(cc,tempu->ip,r.u8[0]);
  if (tempu->type>=IET_REL_I16) {
    AOTStoreCodeU8At(cc,tempu->ip+1,r.u8[1]);
    if (tempu->type>=IET_REL_I32) {
      AOTStoreCodeU8At(cc,tempu->ip+2,r.u8[2],r.u8[3]);
      if (tempu->type>=IET_REL_I64)
	AOTStoreCodeU8At(cc,tempu->ip+4,r.u8[4],r.u8[5],r.u8[6],r.u8[7]);
    }
  }
}

U0 AOTLocalsResolve(CCmpCtrl *cc)
{
  CAOTCtrl *aotc=cc->aotc;
  CAsmUnresolvedRef *tempu=aotc->local_unresolved,*tempu1;
  CAsmUndefHash *tempauh;
  Bool undefined;
  U8 *label=NULL;

  while (tempu) {
    undefined=FALSE;
    tempu1=tempu->next;
    tempauh=tempu->asm_undef_hash;
    while (tempauh) {
      if (tempauh->hash->type & HTF_UNRESOLVED) {
	PrintErr("undefined sym at line:%04d %s %s\n",
	      tempu->line_num,aotc->last_label,tempauh->hash->str);
	LexExcept(cc);
      }
      label=tempauh->hash->str;
      tempauh=tempauh->next;
    }
    if (!undefined)
      AsmResolve(cc,tempu,label,FALSE);
    Free(tempu->machine_code);
    LinkedLstDel(tempu->asm_undef_hash);
    Free(tempu);
    tempu=tempu1;
  }
  HashTableDel(cc->htc.local_hash_table);
  cc->htc.hash_table_lst=cc->htc.local_hash_table=HashTableNew(16);
  cc->htc.local_hash_table->next=cc->htc.glbl_hash_table;
  aotc->local_unresolved=NULL;
}

U0 AOTGlblsResolve(CCmpCtrl *cc,CAOT *tempaot)
{
  CAOTCtrl *aotc=cc->aotc;
  CHashFun *tempf;
  CAsmUnresolvedRef *tempu=aotc->glbl_unresolved,*tempu1;
  I64 i,j;
  CAOTImportExport *tempie,*tempie1;
  CAsmUndefHash *tempauh;
  CHashExport *tempex;
  U8 *label;
  Bool undefined;
  CExternUsage *tempeu,*tempeu8;

  while (tempu) {
    label=NULL;
    undefined=FALSE;
    tempu1=tempu->next;
    tempauh=tempu->asm_undef_hash;
    while (tempauh) {
      if (tempauh->hash->type & HTF_UNRESOLVED) {
	tempex=tempauh->hash;
	if (tempex->type & HTT_EXPORT_SYS_SYM &&
	      tempex->type & HTF_UNRESOLVED &&
	      !(tempex->type & HTF_IMPORT) &&
	      (tempf=HashFind(tempex->str,cc->htc.hash_table_lst,HTT_FUN)) &&
	      !Bt(&tempf->flags,Cf_EXTERN)) {
	  tempex->val=tempf->executable_addr;
	  tempex->type&=~HTF_UNRESOLVED;
	  label=tempauh->hash->str;
	} else {
	  if (!(tempex->type & HTF_IMPORT)) {
	    if (cc->htc.local_var_lst) {
	      tempex->type|=HTF_GOTO_LABEL;
	      tempex->use_cnt++;
	    } else {
	      PrintErr("undefined sym at line:%04d %s\n",
		    tempu->line_num,tempex->str);
	      LexExcept(cc);
	    }
	  } else if (undefined) {
	    PrintErr("Two imports in same expression "
		  "not allowed at line:%04d %s\n",
		  tempu->line_num,tempex->str);
	    LexExcept(cc);
	  }
	  undefined=TRUE;
	}
      } else
	label=tempauh->hash->str;
      tempauh=tempauh->next;
    }
    AsmResolve(cc,tempu,label,undefined);
    Free(tempu->machine_code);
    LinkedLstDel(tempu->asm_undef_hash);
    Free(tempu);
    tempu=tempu1;
  }

  for (i=0;i<=cc->htc.glbl_hash_table->mask;i++) {
    tempex=cc->htc.glbl_hash_table->body[i];
    while (tempex) {
      if (tempex->type & (HTF_IMPORT|HTF_GOTO_LABEL)) {
	if (tempex->use_cnt && (tempie=tempex->ie_lst)) {
	  if (tempex->type&HTF_GOTO_LABEL)
	    tempie->flags|=IEF_GOTO_LABEL;
	  if (tempex->import_name)
	    tempie->str=StrNew(tempex->import_name);
	  else
	    tempie->str=StrNew(tempex->str);
	  do {
	    tempie1=tempie->next;
	    QueIns(tempie,tempaot->last_ie);
	  } while (tempie=tempie1);
	  tempex->ie_lst=NULL;
	}
      } else if (tempex->type & (HTF_EXPORT|HTF_RESOLVE)) {
	if (tempex->type & HTF_UNRESOLVED) {
	  PrintErr("undefined sym at %s\n",tempex->str);
	  LexExcept(cc);
	}
	if (tempex->type & HTF_RESOLVE) {
	  tempf=tempex;
	  tempeu=tempf->ext_lst;
	  while (tempeu) {
	    tempeu8=tempeu->next;
	    j=tempf->executable_addr-(tempeu->ip+4);
	    AOTStoreCodeU8At(cc,tempeu->ip,j.u8[0],j.u8[1],j.u8[2],j.u8[3]);
	    Free(tempeu);
	    tempeu=tempeu8;
	  }
	}
	if (tempex->type & HTF_EXPORT) {
	  tempie=CAlloc(sizeof(CAOTImportExport));
	  tempie->type=IET_REL32_EXPORT;
	  if (tempex->type & HTT_FUN)
	    tempie->ip=tempf->executable_addr;
	  else if (tempex->type & HTT_GLBL_VAR)
	    tempie->ip=tempex(CHashGlblVar *)->data_addr_ip;
	  else
	    tempie->ip=tempex->val;
	  tempie->aot=cc->aot;
	  if (tempex->type & HTF_IMM)
	    tempie->type++;
	  tempie->str=StrNew(tempex->str);
	  tempie->src_link=StrNew(tempex->src_link);
	  QueIns(tempie,tempaot->last_ie);
	}
      }
      tempex=tempex->next;
    }
  }
}

U0 AsmUnresolvedAdd(CCmpCtrl *cc,U8 *machine_code,I64 type,I64 ip,I64 rel_ip,
	CAsmUndefHash *local_asm_undef_hash,CAsmUndefHash *glbl_asm_undef_hash,
	I64 line_num,Bool U8_avail)
{
  CAsmUnresolvedRef *tempu=MAlloc(sizeof(CAsmUnresolvedRef));
  tempu->machine_code=machine_code;
  tempu->type=type;
  tempu->ip=ip;
  tempu->rel_ip=rel_ip;
  tempu->aot=cc->aot;
  tempu->imm_flag=FALSE;
  tempu->line_num=line_num;
  tempu->U8_avail=U8_avail;
  tempu->str=NULL;
  if (local_asm_undef_hash) {
    tempu->asm_undef_hash=local_asm_undef_hash;
    tempu->next=cc->aotc->local_unresolved;
    cc->aotc->local_unresolved=tempu;
  } else {
    tempu->asm_undef_hash=glbl_asm_undef_hash;
    tempu->next=cc->aotc->glbl_unresolved;
    cc->aotc->glbl_unresolved=tempu;
    if (glbl_asm_undef_hash->hash->type & HTF_IMPORT) {
      tempu->str=StrNew(glbl_asm_undef_hash->hash->str);
      if (glbl_asm_undef_hash->hash->type & HTF_IMM)
	tempu->imm_flag=TRUE;
    }
  }
}
