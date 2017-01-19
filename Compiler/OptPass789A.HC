I64 OptPass789A(CCmpCtrl *cc,COptReg *reg_offsets,U8 *buf,CDbgInfo **_dbg)
{/*cc->pass==7 is first time
cc->pass==8 is second time
cc->pass==9 is third time
cc->pass==9 is fourth time and repeated until size stops shrinking
size is now known
cc->pass==10 is final pass, code is placed into buf.
*/
  CIntermediateCode *tmpi,*tmpi_next;
  I64 i,cnt,num_lines=cc->max_line+1-cc->min_line,rip=0,rip2;
  U8  *ptr,saved_arg1_arg2_r[3*sizeof(CICArg)];
  CCodeMisc *lb;
  CAOT *tmpaot;
  CAOTAbsAddr *tmpa;
  CAOTImportExport *tmpie;
  CAOTHeapGlbl *tmphg;
  CAOTHeapGlblRef *tmphgr;
  CDbgInfo *dbg_info;
  CAOTCtrl *aotc=cc->aotc;
  Bool short_jmp;
  CHashClass *tmpc;
  CHashFun *tmpf;
  CHashGlblVar *tmpg;
  CExternUsage *tmpeu;

  if (_dbg) {
    *_dbg=dbg_info=CAlloc(offset(CDbgInfo.body)+sizeof(U32)*(num_lines+1));
    dbg_info->min_line=cc->min_line;
    dbg_info->max_line=cc->max_line;
    if (cc->flags&CCF_AOT_COMPILE)
      dbg_info->body[0]=aotc->rip;
    else
      dbg_info->body[0]=buf;
  } else
    dbg_info=NULL;

  if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT) &&
	Bt(&cc->saved_pass_trace,cc->pass))
    "$$BK,1$$$$LTRED$$$$IV,1$$This code gets merged together and patched.\n"
	  "$$FG$$$$IV,0$$$$BK,0$$";

  cc->last_float_op_ic=NULL;
  tmpi=&cc->coc.coc_head;
  tmpi->ic_last_start=-1;
  tmpi->ic_cnt=0;
  tmpi=tmpi->next;
  while (tmpi->ic_code) {
    tmpi_next=tmpi->next;
    if (tmpi->ic_flags&ICF_PASS_TRACE && Bt(&cc->saved_pass_trace,cc->pass))
      ICPut(cc,tmpi);
    rip2=rip;
    if (cc->flags&CCF_AOT_COMPILE)
      rip2+=aotc->rip;
    else
      rip2+=buf;
    cc->cur_ic_float_op_num=0;
    if (!(tmpi->ic_flags &ICF_CODE_FINAL)) {
      tmpi->ic_flags=tmpi->ic_flags&
	    ~(ICF_PREV_DELETED|ICF_DONT_RESTORE)|ICF_CODE_FINAL;
      if (cc->pass==7)
	cc->dont_push_float=Btr(&tmpi->ic_flags,ICf_DONT_PUSH_FLOAT0);
      MemCpy(saved_arg1_arg2_r,&tmpi->arg1,3*sizeof(CICArg));
      tmpi->ic_cnt=0;
      tmpi->ic_last_start=-1;
      if (tmpi->arg2.type.mode) {
	if (tmpi->ic_flags & ICF_ARG2_TO_F64) {
	  ICFCvt(cc,tmpi,REG_RAX,tmpi->arg2.type,
		tmpi->arg2.reg,tmpi->arg2.disp,FALSE,CN_A2,rip2);
	  tmpi->arg2.type=MDF_REG+RT_I64;
	  tmpi->arg2.reg=REG_RAX;
	  tmpi->arg2.disp=0;
	} else if (tmpi->ic_flags & ICF_ARG2_TO_INT) {
	  ICFCvt(cc,tmpi,REG_RAX,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,TRUE,CN_A2,rip2);
	  tmpi->arg2.type=MDF_REG+RT_I64;
	  tmpi->arg2.reg=REG_RAX;
	  tmpi->arg2.disp=0;
	}
      }
      if (tmpi->arg1.type.mode) {
	if (tmpi->ic_flags & ICF_ARG1_TO_F64) {
	  ICFCvt(cc,tmpi,REG_RDX,tmpi->arg1.type,
		tmpi->arg1.reg,tmpi->arg1.disp,FALSE,CN_A1,rip2);
	  tmpi->arg1.type=MDF_REG+RT_I64;
	  tmpi->arg1.reg=REG_RDX;
	  tmpi->arg1.disp=0;
	} else if (tmpi->ic_flags & ICF_ARG1_TO_INT) {
	  ICFCvt(cc,tmpi,REG_RDX,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,TRUE,CN_A1,rip2);
	  tmpi->arg1.type=MDF_REG+RT_I64;
	  tmpi->arg1.reg=REG_RDX;
	  tmpi->arg1.disp=0;
	}
      }

      switch [tmpi->ic_code] {
	start:
	  start:
	    case IC_ABS_ADDR:
	      ICU16(tmpi,0xB848);
	      ICU64(tmpi,tmpi->ic_data);
	      if (buf && cc->flags&CCF_AOT_COMPILE &&
		    !(cc->flags&(CCF_NO_ABSS|CCF_ASM_EXPRESSIONS))) {
		tmpa=CAlloc(sizeof(CAOTAbsAddr));
		tmpa->next=aotc->abss;
		tmpa->type=AAT_ADD_U64;
		aotc->abss=tmpa;
		tmpa->rip=rip2+tmpi->ic_cnt-8;
	      }
	      break;
	    case IC_HEAP_GLBL:
	      ICU16(tmpi,0xB848);
	      ICU64(tmpi,0);
	      tmphg=tmpi->ic_data;
	      if (buf && cc->flags&CCF_AOT_COMPILE &&
//TODO:is this necessary--flags?
		    !(cc->flags&(CCF_NO_ABSS|CCF_ASM_EXPRESSIONS))) {
		tmphgr=CAlloc(sizeof(CAOTHeapGlblRef));
		tmphgr->next=tmphg->references;
		tmphg->references=tmphgr;
		tmphgr->rip=rip2+tmpi->ic_cnt-8;
	      }
	      break;
	    case IC_ADDR_IMPORT:
	      ICU8(tmpi,0xB8);
	      ICU32(tmpi,0);
	      if (buf && !(cc->flags&CCF_NO_ABSS)) {
		tmpg=tmpi->ic_data;
		tmpie=CAlloc(sizeof(CAOTImportExport));
		tmpie->type=IET_IMM_U32;
		tmpie->rip=rip2+tmpi->ic_cnt-4;
		tmpie->next=tmpg->ie_lst;
		tmpg->ie_lst=tmpie;
	      }
	      ICU24(tmpi,0xC06348);
	      break;
	    case IC_RIP:
	      ICU16(tmpi,0xB848);
	      ICU64(tmpi,rip2+tmpi->ic_cnt-2);
	      if (cc->flags&CCF_AOT_COMPILE && buf &&!(cc->flags&CCF_NO_ABSS)) {
		tmpa=CAlloc(sizeof(CAOTAbsAddr));
		tmpa->next=aotc->abss;
		tmpa->type=AAT_ADD_U64;
		aotc->abss=tmpa;
		tmpa->rip=rip2+tmpi->ic_cnt-8;
	      }
	      break;
	  end:
	    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  MDF_REG+RT_I64,REG_RAX,0,rip2);
	    break;
	  case IC_BR_CARRY:
	    ICFlagBranch(tmpi,rip,0x72820F,buf);
	    break;
	  case IC_BR_NOT_CARRY:
	    ICFlagBranch(tmpi,rip,0x73830F,buf);
	    break;
	  case IC_BR_ZERO:
	    ICTestAndBranch(tmpi,rip,0x74840F,buf,rip2);
	    break;
	  case IC_BR_NOT_ZERO:
	    ICTestAndBranch(tmpi,rip,0x75850F,buf,rip2);
	    break;
	  case IC_BR_MM_ZERO:
	    ICPreIncDec(tmpi,SLASH_OP_DEC,rip2);
	    ICFlagBranch(tmpi,rip,0x74840F,buf);
	    break;
	  case IC_BR_MM_NOT_ZERO:
	    ICPreIncDec(tmpi,SLASH_OP_DEC,rip2);
	    ICFlagBranch(tmpi,rip,0x75850F,buf);
	    break;
	  case IC_BR_EQU_EQU:
	    ICCmpAndBranch(tmpi,FALSE,rip,0x74840F,0x74840F,
		  0x74840F,0x74840F,buf,rip2);
	    break;
	  case IC_BR_EQU_EQU2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x74840F,0x74840F,
		  0x74840F,0x74840F,buf,rip2);
	    break;
	  case IC_BR_NOT_EQU:
	    ICCmpAndBranch(tmpi,FALSE,rip,0x75850F,0x75850F,
		  0x75850F,0x75850F,buf,rip2);
	    break;
	  case IC_BR_NOT_EQU2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x75850F,0x75850F,
		  0x75850F,0x75850F,buf,rip2);
	    break;
	  case IC_BR_LESS:
	    if (tmpi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tmpi,	rip,0x72820F,0x77870F,buf,rip2);
	    else
	      ICCmpAndBranch(tmpi,FALSE,rip,0x72820F,0x7C8C0F,
		    0x77870F,0x7F8F0F,buf,rip2);
	    break;
	  case IC_BR_LESS2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x72820F,0x7C8C0F,
		  0x77870F,0x7F8F0F,buf,rip2);
	    break;
	  case IC_BR_GREATER_EQU:
	    if (tmpi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tmpi,	rip,0x73830F,0x76860F,buf,rip2);
	    else
	      ICCmpAndBranch(tmpi,FALSE,rip,0x73830F,0x7D8D0F,
		    0x76860F,0x7E8E0F,buf,rip2);
	    break;
	  case IC_BR_GREATER_EQU2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x73830F,0x7D8D0F,
		  0x76860F,0x7E8E0F,buf,rip2);
	    break;
	  case IC_BR_GREATER:
	    if (tmpi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tmpi,	rip,0x77870F,0x72820F,buf,rip2);
	    else
	      ICCmpAndBranch(tmpi,FALSE,rip,0x77870F,0x7F8F0F,
		    0x72820F,0x7C8C0F,buf,rip2);
	    break;
	  case IC_BR_GREATER2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x77870F,0x7F8F0F,
		  0x72820F,0x7C8C0F,buf,rip2);
	    break;
	  case IC_BR_LESS_EQU:
	    if (tmpi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tmpi,	rip,0x76860F,0x73830F,buf,rip2);
	    else
	      ICCmpAndBranch(tmpi,FALSE,rip,0x76860F,0x7E8E0F,
		    0x73830F,0x7D8D0F,buf,rip2);
	    break;
	  case IC_BR_LESS_EQU2:
	    ICCmpAndBranch(tmpi,TRUE,rip,0x76860F,0x7E8E0F,
		  0x73830F,0x7D8D0F,buf,rip2);
	    break;
	  case IC_BR_BT:
	    ICBrBitOps(tmpi,rip,0xA30F,0x20BA0F,0x72820F,buf,rip2);
	    break;
	  case IC_BR_BTS:
	    ICBrBitOps(tmpi,rip,0xAB0F,0x28BA0F,0x72820F,buf,rip2);
	    break;
	  case IC_BR_BTR:
	    ICBrBitOps(tmpi,rip,0xB30F,0x30BA0F,0x72820F,buf,rip2);
	    break;
	  case IC_BR_BTC:
	    ICBrBitOps(tmpi,rip,0xBB0F,0x38BA0F,0x72820F,buf,rip2);
	    break;
	  case IC_BR_NOT_BT:
	    ICBrBitOps(tmpi,rip,0xA30F,0x20BA0F,0x73830F,buf,rip2);
	    break;
	  case IC_BR_NOT_BTS:
	    ICBrBitOps(tmpi,rip,0xAB0F,0x28BA0F,0x73830F,buf,rip2);
	    break;
	  case IC_BR_NOT_BTR:
	    ICBrBitOps(tmpi,rip,0xB30F,0x30BA0F,0x73830F,buf,rip2);
	    break;
	  case IC_BR_NOT_BTC:
	    ICBrBitOps(tmpi,rip,0xBB0F,0x38BA0F,0x73830F,buf,rip2);
	    break;
	  case IC_BR_AND_ZERO:
	    ICAndBranch(tmpi,rip,0x74840F,buf,rip2);
	    break;
	  case IC_BR_AND_NOT_ZERO:
	    ICAndBranch(tmpi,rip,0x75850F,buf,rip2);
	    break;
	  case IC_SUB_CALL:
	    lb=OptLabelFwd(tmpi->ic_data);
	    ICU8(tmpi,0xE8);
	    ICU32(tmpi,lb->addr-(rip+5));
	    break;
	  case IC_JMP:
	    lb=OptLabelFwd(tmpi->ic_data);
	    short_jmp=ToBool(tmpi->ic_flags&ICF_SHORT_JMP);
	    if (!buf && lb->addr!=INVALID_PTR &&
		  I8_MIN+5<lb->addr-rip<I8_MAX-5)
	      short_jmp=TRUE;
	    if (short_jmp) {
	      tmpi->ic_flags|=ICF_SHORT_JMP;
	      i=lb->addr-(rip+2);
	      if (buf || i)
		ICU16(tmpi,i<<8+0xEB);
	      else
		tmpi->ic_code=IC_NOP1;
	    } else {
	      i=lb->addr-(rip+5);
	      ICU8(tmpi,0xE9);
	      ICU32(tmpi,i);
	    }
	    break;
	  case IC_LABEL:
	    lb=tmpi->ic_data;
	    lb->addr=rip;
	    if (lb->flags&CMF_POP_CMP) {
	      ICAddRSP(tmpi,-8,FALSE);
	      ICAddRSP(tmpi,8,FALSE);
	    }
	    if (lb->type==CMT_ASM_LABEL)
	      lb->addr+=lb->rip;
	    break;
	  case IC_STR_CONST:
	  case IC_GET_LABEL:
	    lb=tmpi->ic_data;
	    if (cc->flags&CCF_AOT_COMPILE)
	      i=lb->addr+aotc->rip;
	    else
	      i=lb->addr+buf;
	    ICLea(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  MDF_RIP_DISP32+RT_PTR,0,i,cc,buf,rip2);
	    break;
	  case IC_ASM:
	    tmpaot=tmpi->ic_data;
	    tmpi->ic_cnt+=tmpaot->aot_U8s;
	    if (buf) {
	      MemCpy(buf+rip,tmpaot->buf,tmpaot->aot_U8s);
	      Free(tmpaot->buf);
	      tmpaot->buf=buf;
	      tmpaot->rip=rip;
	      tmpaot->rip2=rip2;
	      if (cc->flags&CCF_AOT_COMPILE)
		CmpFixUpAOTAsm(cc,tmpaot);
	      else
		CmpFixUpJITAsm(cc,tmpaot);
	      cnt=tmpi->ic_cnt;
	      goto op789A_skip_copy;
	    }
	    break;
	  case IC_CALL:
	    i=tmpi->ic_data-(rip2+5);
	    if (!(I32_MIN<=i<=I32_MAX) && !(cc->flags&CCF_AOT_COMPILE)) {
	      ICU16(tmpi,0xBB48);
	      ICU64(tmpi,tmpi->ic_data);
	      ICU16(tmpi,0xD3FF);
	    } else {
	      ICU8(tmpi,0xE8);
	      ICU32(tmpi,i);
	    }
	    break;
	  case IC_CALL_EXTERN: //Only for static modules
	    ICU8(tmpi,0xE8);
	    ICU32(tmpi,0);
	    if (buf) {
	      tmpf=tmpi->ic_data;
	      tmpeu=CAlloc(sizeof(CExternUsage));
	      tmpeu->next=tmpf->ext_lst;
	      tmpf->ext_lst=tmpeu;
	      tmpeu->rip=rip2+1;
	    }
	    break;
	  case IC_CALL_INDIRECT2:
	    ICU16(tmpi,0xBB48);
	    if (cc->flags&CCF_AOT_COMPILE) i=rip2+tmpi->ic_cnt;
	    ICU64(tmpi,tmpi->ic_data);
	    ICU16(tmpi,0x13FF);
	    if (buf && cc->flags&CCF_AOT_COMPILE&& !(cc->flags&CCF_NO_ABSS)) {
	      tmpa=CAlloc(sizeof(CAOTAbsAddr));
	      tmpa->next=aotc->abss;
	      tmpa->type=AAT_ADD_U64;
	      aotc->abss=tmpa;
	      tmpa->rip=i;
	    }
	    break;
	  case IC_CALL_IMPORT:
	    if (GetOption(OPTf_USE_IMM64)) {
	      ICU16(tmpi,0xBB48);
	      ICU64(tmpi,0);
	      if (buf) {
		tmpf=tmpi->ic_data;
		tmpie=CAlloc(sizeof(CAOTImportExport));
		tmpie->type=IET_IMM_I64;
		tmpie->rip=rip2+tmpi->ic_cnt-8;
		tmpie->next=tmpf->ie_lst;
		tmpf->ie_lst=tmpie;
	      }
	      ICU16(tmpi,0xD3FF);
	    } else {
	      ICU8(tmpi,0xE8);
	      ICU32(tmpi,0);
	      if (buf) {
		tmpf=tmpi->ic_data;
		tmpie=CAlloc(sizeof(CAOTImportExport));
		tmpie->type=IET_REL_I32;
		tmpie->rip=rip2+tmpi->ic_cnt-4;
		tmpie->next=tmpf->ie_lst;
		tmpf->ie_lst=tmpie;
	      }
	    }
	    break;
	end:
	  tmpi->ic_flags&=~ICF_CODE_FINAL;
	  break;
	case IC_LEAVE:
	  if (cc->htc.fun) {
	    if (Bt(&cc->htc.fun->flags,Ff_INTERRUPT))
	      ICPopRegs(tmpi,REGG_CLOBBERED|cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS|REGG_STK_TMP));
	    else
	      ICPopRegs(tmpi,cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS));
	  }
	  if (tmpi->ic_data<=I16_MAX) {
	    if (tmpi->ic_data)
	      ICU8(tmpi,0xC9); //LEAVE
	    else
	      ICU8(tmpi,0x5D); //POP RBP
	  } else {
	    ICAddRSP(tmpi,tmpi->ic_data);
	    ICU8(tmpi,0x5D); //POP RBP
	  }
	  if (cc->htc.fun && Bt(&cc->htc.fun->flags,Ff_INTERRUPT)) {
	    if (Bt(&cc->htc.fun->flags,Ff_HASERRCODE))
	      ICAddRSP(tmpi,8);
	    ICU16(tmpi,0xCF48);
	  } else if (cc->htc.fun && cc->htc.fun->arg_cnt &&
		(Bt(&cc->htc.fun->flags,Ff_RET1) ||
		Bt(&cc->htc.fun->flags,Ff_ARGPOP)) &&
		!Bt(&cc->htc.fun->flags,Ff_NOARGPOP)) {
	    ICU8(tmpi,0xC2);
	    ICU16(tmpi,cc->htc.fun->arg_cnt<<3);
	  } else
	    ICU8(tmpi,0xC3);
	  break;
	case IC_RET:
	  ICU8(tmpi,0xC3);
	  break;
	case IC_FS:
	  ICZero(tmpi,REG_RAX);
	  ICU32(tmpi,0x8B4864);
	  break;
	case IC_GS:
	  ICZero(tmpi,REG_RAX);
	  ICU32(tmpi,0x8B4865);
	  break;
	case IC_MOV_FS:
	  ICZero(tmpi,REG_RAX);
	  ICU8(tmpi,0x64);
//It's ugly to use ic_class here
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		MDF_DISP+CmpRawType(tmpi->ic_class),REG_RAX,tmpi->ic_data,
		rip2);
	  break;
	case IC_MOV_GS:
	  ICZero(tmpi,REG_RAX);
	  ICU8(tmpi,0x65);
//It's ugly to use ic_class here
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		MDF_DISP+CmpRawType(tmpi->ic_class),REG_RAX,tmpi->ic_data,
		rip2);
	  break;
	case IC_HOLYC_TYPECAST:
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_COM:
	  ICUnaries(tmpi,SLASH_OP_NOT,rip2);
	  break;
	case IC_NOT:
	  ICNot(tmpi,rip2);
	  break;
	case IC_UNARY_MINUS:
	  if (tmpi->res.type.raw_type==RT_F64)
	    ICFUnaryMinus(cc,tmpi,buf,rip2);
	  else
	    ICUnaries(tmpi,SLASH_OP_NEG,rip2);
	  break;
	case IC_ADDR:
	case IC_MOV:
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_DEREF:
	  ICDeref(tmpi,rip2);
	  break;
	case IC_DEREF_PP:
	  ICDerefPostIncDec(tmpi,SLASH_OP_INC,rip2);
	  break;
	case IC_DEREF_MM:
	  ICDerefPostIncDec(tmpi,SLASH_OP_DEC,rip2);
	  break;
	case IC__PP:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICPostIncDec(tmpi,SLASH_OP_INC,rip2);
	  else
	    ICFPostIncDec(cc,tmpi,CMP_TEMPLATE_INC,rip2);
	  break;
	case IC__MM:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICPostIncDec(tmpi,SLASH_OP_DEC,rip2);
	  else
	    ICFPostIncDec(cc,tmpi,CMP_TEMPLATE_DEC,rip2);
	  break;
	case IC_PP_:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICPreIncDec(tmpi,SLASH_OP_INC,rip2);
	  else
	    ICFPreIncDec(cc,tmpi,CMP_TEMPLATE_INC,rip2);
	  break;
	case IC_MM_:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICPreIncDec(tmpi,SLASH_OP_DEC,rip2);
	  else
	    ICFPreIncDec(cc,tmpi,CMP_TEMPLATE_DEC,rip2);
	  break;
	case IC_LEA:
	  ICLea(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,cc,buf,rip2);
	  break;
	case IC_POWER:
	  ICFPow(cc,tmpi,buf,rip2);
	  break;
	case IC_SHL:
	  ICShift(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,rip2);
	  break;
	case IC_SHR:
	  ICShift(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,rip2);
	  break;
	case IC_MUL:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICMul(tmpi,rip2);
	  else
	    ICFMul(cc,tmpi,buf,rip2);
	  break;
	case IC_DIV:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICDiv(tmpi,rip2);
	  else
	    ICFDiv(cc,tmpi,buf,rip2);
	  break;
	case IC_MOD:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICMod(tmpi,rip2);
	  else
	    ICFMod(cc,tmpi,rip2);
	  break;
	case IC_AND:
	  ICAddEct(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x23,rip2);
	  break;
	case IC_OR:
	  ICAddEct(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x0B,rip2);
	  break;
	case IC_XOR:
	  ICAddEct(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x33,rip2);
	  break;
	case IC_ADD:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICAddEct(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,0x03,rip2);
	  else
	    ICFAdd(cc,tmpi,buf,rip2);
	  break;
	case IC_SUB:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICSub(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
	  else
	    ICFSub(cc,tmpi,buf,rip2);
	  break;
	case IC_EQU_EQU:
	  ICCmp(tmpi,0x75,0x75,rip2);
	  break;
	case IC_NOT_EQU:
	  ICCmp(tmpi,0x74,0x74,rip2);
	  break;
	case IC_LESS:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICCmp(tmpi,0x73,0x7D,rip2);
	  else
	    ICFCmp(cc,tmpi,CMP_TEMPLATE_LESS,rip2);
	  break;
	case IC_GREATER_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICCmp(tmpi,0x72,0x7C,rip2);
	  else
	    ICFCmp(cc,tmpi,CMP_TEMPLATE_GREATER_EQU,rip2);
	  break;
	case IC_GREATER:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICCmp(tmpi,0x76,0x7E,rip2);
	  else
	    ICFCmp(cc,tmpi,CMP_TEMPLATE_GREATER,rip2);
	  break;
	case IC_LESS_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICCmp(tmpi,0x77,0x7F,rip2);
	  else
	    ICFCmp(cc,tmpi,CMP_TEMPLATE_LESS_EQU,rip2);
	  break;
	case IC_AND_AND:
	  ICAndAnd(tmpi,rip2);
	  break;
	case IC_OR_OR:
	  ICOrOr(tmpi,rip2);
	  break;
	case IC_XOR_XOR:
	  ICXorXor(tmpi,rip2);
	  break;
	case IC_ASSIGN:
	  ICAssign(tmpi,rip2);
	  break;
	case IC_ASSIGN_PP:
	  ICAssignPostIncDec(tmpi,SLASH_OP_INC,rip2);
	  break;
	case IC_ASSIGN_MM:
	  ICAssignPostIncDec(tmpi,SLASH_OP_DEC,rip2);
	  break;
	case IC_SHL_EQU:
	  ICShiftEqu(tmpi,tmpi->arg1_type_pointed_to,
		tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,rip2);
	  break;
	case IC_SHR_EQU:
	  ICShiftEqu(tmpi,tmpi->arg1_type_pointed_to,
		tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,rip2);
	  break;
	case IC_MUL_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICMulEqu(tmpi,rip2);
	  else
	    ICFOpEqu(cc,tmpi,SLASH_OP_FMUL,buf,rip2);
	  break;
	case IC_DIV_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICDivEqu(tmpi,FALSE,rip2);
	  else
	    ICFOpEqu(cc,tmpi,SLASH_OP_FDIV,buf,rip2);
	  break;
	case IC_MOD_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICDivEqu(tmpi,TRUE,rip2);
	  else
	    ICFModEqu(cc,tmpi,rip2);
	  break;
	case IC_AND_EQU:
	  ICAndEqu(tmpi,rip2);
	  break;
	case IC_OR_EQU:
	  ICOrEqu(tmpi,rip2);
	  break;
	case IC_XOR_EQU:
	  ICXorEqu(tmpi,rip2);
	  break;
	case IC_ADD_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICAddSubEctEqu(tmpi,tmpi->arg1_type_pointed_to,
		  tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		  0x010000000003,rip2);
	  else
	    ICFOpEqu(cc,tmpi,SLASH_OP_FADD,buf,rip2);
	  break;
	case IC_SUB_EQU:
	  if (tmpi->ic_flags&ICF_USE_INT)
	    ICAddSubEctEqu(tmpi,tmpi->arg1_type_pointed_to,
		  tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,
		  0x29000000052B,rip2);
	  else
	    ICFOpEqu(cc,tmpi,SLASH_OP_FSUB,buf,rip2);
	  break;
	case IC_SHL_CONST:
	  ICShift(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		MDF_IMM+RT_I64,0,tmpi->ic_data,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,rip2);
	  break;
	case IC_SHR_CONST:
	  ICShift(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,
		MDF_IMM+RT_I64,0,tmpi->ic_data,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,rip2);
	  break;
	case IC_ADD_CONST:
	  ICAddSubEctImm(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,tmpi->ic_data,
		0x0003,rip2);
	  break;
	case IC_SUB_CONST:
	  ICAddSubEctImm(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,tmpi->ic_data,
		0x052B,rip2);
	  break;
	case IC_ENTER:
	  ICU32(tmpi,0xEC8B4855);
	  if (tmpi->ic_data)
	    ICAddRSP(tmpi,-tmpi->ic_data,FALSE);
	  if (cc->htc.fun) {
	    if (Bt(&cc->htc.fun->flags,Ff_INTERRUPT))
	      ICPushRegs(tmpi,REGG_CLOBBERED|cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS|REGG_STK_TMP));
	    else {
	      if (sys_var_init_flag && i)
		ICLocalVarInit(tmpi);
	      ICPushRegs(tmpi,cc->htc.fun->used_reg_mask
		    &(REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS));
	    }
	    for (i=0;i<REG_REGS_NUM;i++)
	      if (reg_offsets[i]>0 && reg_offsets[i].offset!=I64_MAX) {
		tmpc=OptClassFwd(reg_offsets[i].m->member_class);
		ICMov(tmpi,MDF_REG+RT_I64,i,0,MDF_DISP+tmpc->raw_type,
		      REG_RBP,reg_offsets[i].offset,rip2);
	      }
	  }
	  break;
	case IC_ADD_RSP:
	  ICAddRSP(tmpi,tmpi->ic_data);
	  break;
	case IC_CALL_INDIRECT:
	  if (I8_MIN<=tmpi->ic_data<=I8_MAX) {
	    ICU24(tmpi,0x2454FF);  //CALL disp[RSP]
	    ICU8(tmpi,tmpi->ic_data);
	  } else {
	    ICU24(tmpi,0x2494FF);  //CALL disp[RSP]
	    ICU32(tmpi,tmpi->ic_data);
	  }
	  break;
	case IC_PUSH:
	  ICPush(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_POP:
	  ICU8(tmpi,0x58);
	  break;
	case IC_INVLPG:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU24(tmpi,0x38010F);
	  break;
	case IC_CLFLUSH:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU24(tmpi,0x38AE0F);
	  break;
	case IC_GET_RFLAGS:
	  ICU8(tmpi,0x9C);
	  ICPop(tmpi,MDF_REG+RT_I64,REG_RAX,0,rip2);
	  break;
	case IC_CARRY:
	  ICU24(tmpi,0xC0920F); //SETC AL
	  ICU24(tmpi,0x01E083); //AND EAX,1
	  break;
	case IC_RDTSC:
	  ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_RDTSC,TRUE,FALSE,FALSE,CN_INST);
	  break;
	case IC_SET_RFLAGS:
	  ICPush(tmpi,tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU8(tmpi,0x9D);
	  break;
	case IC_GET_RBP:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_REG+RT_I64,REG_RBP,0,rip2);
	  break;
	case IC_SET_RBP:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RBP,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_GET_RSP:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_REG+RT_I64,REG_RSP,0,rip2);
	  break;
	case IC_SET_RSP:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RSP,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_RETURN_VAL:
	case IC_SET_RAX:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  break;
	case IC_RETURN_VAL2:
	case IC_GET_RAX:
	  break;
	case IC_BT:
	  ICBitOps(tmpi,&tmpi->arg1,&tmpi->arg2,tmpi->next,
		0xA30F,0x20BA0F,rip2);
	  break;
	case IC_BTS:
	case IC_LBTS:
	  ICBitOps(tmpi,&tmpi->arg1,&tmpi->arg2,tmpi->next,
		0xAB0F,0x28BA0F,rip2);
	  break;
	case IC_BTR:
	case IC_LBTR:
	  ICBitOps(tmpi,&tmpi->arg1,&tmpi->arg2,tmpi->next,
		0xB30F,0x30BA0F,rip2);
	  break;
	case IC_BTC:
	case IC_LBTC:
	  ICBitOps(tmpi,&tmpi->arg1,&tmpi->arg2,tmpi->next,
		0xBB0F,0x38BA0F,rip2);
	  break;
	case IC_BSF:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU32(tmpi,0xC0BC0F48);
	  ICU16(tmpi,0x0375);
	  ICU24(tmpi,0xD0F748);
	  break;
	case IC_BSR:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU32(tmpi,0xC0BD0F48);
	  ICU16(tmpi,0x0375);
	  ICU24(tmpi,0xD0F748);
	  break;
	case IC_SIGN_I64:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_SIGN_I64,
		TRUE,FALSE,FALSE,CN_INST);
	  break;
	case IC_TOUPPER:
	  ICToUpper(tmpi,rip2);
	  break;
	case IC_TO_I64:
	  ICToI64(cc,tmpi,rip2);
	  break;
	case IC_TO_F64:
	  ICToF64(cc,tmpi,rip2);
	  break;
	case IC_TO_BOOL:
	  ICToBool(cc,tmpi,rip2);
	  break;
	case IC_SQR:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_SQR,rip2);
	  break;
	case IC_ABS:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_ABS,rip2);
	  break;
	case IC_SQRT:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_SQRT,rip2);
	  break;
	case IC_SIN:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_SIN,rip2);
	  break;
	case IC_COS:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_COS,rip2);
	  break;
	case IC_TAN:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_TAN,rip2);
	  break;
	case IC_ATAN:
	  ICFTemplateFun(cc,tmpi,CMP_TEMPLATE_ATAN,rip2);
	  break;
	case IC_ABS_I64:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICU24(tmpi,0xC08548);
	  ICU16(tmpi,0x0379);
	  ICU24(tmpi,0xD8F748);
	  break;
	case IC_MIN_I64:
	  ICMinMax(tmpi,0x4F,rip2);
	  break;
	case IC_MAX_I64:
	  ICMinMax(tmpi,0x4C,rip2);
	  break;
	case IC_MIN_U64:
	  ICMinMax(tmpi,0x47,rip2);
	  break;
	case IC_MAX_U64:
	  ICMinMax(tmpi,0x42,rip2);
	  break;
	case IC_MOD_U64:
	  ICModU64(tmpi,rip2);
	  break;
	case IC_SQR_I64:
	  ICSqr(tmpi,SLASH_OP_IMUL,rip2);
	  break;
	case IC_SQR_U64:
	  ICSqr(tmpi,SLASH_OP_MUL,rip2);
	  break;
	case IC_SWAP_U8:
	case IC_SWAP_U16:
	case IC_SWAP_U32:
	case IC_SWAP_I64:
	  ICSwap(tmpi,rip2);
	  break;
	case IC_QUE_INIT:
	  ICQueInit(tmpi,rip2);
	  break;
	case IC_QUE_INS:
	  ICQueIns(tmpi,rip2);
	  break;
	case IC_QUE_INS_REV:
	  ICQueInsRev(tmpi,rip2);
	  break;
	case IC_QUE_REM:
	  ICQueRem(tmpi,rip2);
	  break;
	case IC_STRLEN:
	  ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	  ICCopyTemplate(cc,tmpi,CMP_TEMPLATE_STRLEN,TRUE,FALSE,FALSE,CN_INST);
	  break;
	case IC_IN_U32:
	  if (tmpi->arg1.type&MDF_IMM) {
	    ICU16(tmpi,0xC033);
	    if (tmpi->arg1.disp<=U8_MAX)
	      ICU16(tmpi,0xE5+tmpi->arg1.disp<<8);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg1.disp<<16);
	      ICU8(tmpi,0xED);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU16(tmpi,0xC033);
	    ICU8(tmpi,0xED);
	  }
	  break;
	case IC_IN_U16:
	  if (tmpi->arg1.type&MDF_IMM) {
	    ICU16(tmpi,0xC033);
	    if (tmpi->arg1.disp<=U8_MAX)
	      ICU24(tmpi,0xE500+OC_OP_SIZE_PREFIX+tmpi->arg1.disp<<16);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg1.disp<<16);
	      ICU16(tmpi,0xED00+OC_OP_SIZE_PREFIX);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU16(tmpi,0xC033);
	    ICU16(tmpi,0xED00+OC_OP_SIZE_PREFIX);
	  }
	  break;
	case IC_IN_U8:
	  if (tmpi->arg1.type&MDF_IMM) {
	    ICU16(tmpi,0xC033);
	    if (tmpi->arg1.disp<=U8_MAX)
	      ICU16(tmpi,0xE4+tmpi->arg1.disp<<8);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg1.disp<<16);
	      ICU8(tmpi,0xEC);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU16(tmpi,0xC033);
	    ICU8(tmpi,0xEC);
	  }
	  break;
	case IC_OUT_U32:
	  if (tmpi->arg2.type&MDF_IMM) {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    if (tmpi->arg2.disp<=U8_MAX)
	      ICU16(tmpi,0xE7+tmpi->arg2.disp<<8);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg2.disp<<16);
	      ICU8(tmpi,0xEF);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU8(tmpi,0xEF);
	  }
	  break;
	case IC_OUT_U16:
	  if (tmpi->arg2.type&MDF_IMM) {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    if (tmpi->arg2.disp<=U8_MAX)
	      ICU24(tmpi,0xE700+OC_OP_SIZE_PREFIX+tmpi->arg2.disp<<16);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg2.disp<<16);
	      ICU16(tmpi,0xEF00+OC_OP_SIZE_PREFIX);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU16(tmpi,0xEF00+OC_OP_SIZE_PREFIX);
	  }
	  break;
	case IC_OUT_U8:
	  if (tmpi->arg2.type&MDF_IMM) {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    if (tmpi->arg2.disp<=U8_MAX)
	      ICU16(tmpi,0xE6+tmpi->arg2.disp<<8);
	    else {
	      ICU32(tmpi,0xBA00+OC_OP_SIZE_PREFIX+tmpi->arg2.disp<<16);
	      ICU8(tmpi,0xEE);
	    }
	  } else {
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RDX,0,
		  tmpi->arg2.type,tmpi->arg2.reg,tmpi->arg2.disp,rip2);
	    ICMov(tmpi,MDF_REG+RT_I64,REG_RAX,0,
		  tmpi->arg1.type,tmpi->arg1.reg,tmpi->arg1.disp,rip2);
	    ICU8(tmpi,0xEE);
	  }
	  break;
	case IC_NOBOUND_SWITCH:
	  ICSwitch(tmpi,rip,TRUE,cc,buf,rip2);
	  break;
	case IC_SWITCH:
	  ICSwitch(tmpi,rip,FALSE,cc,buf,rip2);
	  break;
	case IC_NOP1:
	case IC_NOP2:
	  OptFree(tmpi);
	  goto op789A_next;
	case IC_CALL_START:
	case IC_PUSH_REGS:
	  ICPushRegs(tmpi,tmpi->ic_data);
	  break;
	case IC_CALL_END:
	  ICPopRegs(tmpi,tmpi->ic_data);
	  if (tmpi->res.type.mode)
	    ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  MDF_REG+RT_I64,REG_RAX,0,rip2);
	  break;
	case IC_POP_REGS:
	  ICPopRegs(tmpi,tmpi->ic_data);
	  break;
	case IC_PUSH_CMP:
	case IC_CALL_END2:
	case IC_END:
	case IC_ADD_RSP1:
	  break;
	default:
	  "Pass:%d Missing IC hndlr\n",cc->pass;
	  ICPut(cc,tmpi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
      if (tmpi->res.type.mode) {
	if (tmpi->ic_flags & ICF_RES_TO_F64) {
	  if (tmpi->ic_code==IC_PUSH_CMP) {
	    ICU24(tmpi,0x242CDF); //FILD U64 [RSP]
	    ICU24(tmpi,0x241CDD); //FSTP U64 [RSP]
	  } else {
	    ICFCvt(cc,tmpi,REG_RAX,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		  FALSE,CN_RES,rip2);
	    if (!Bt(&tmpi->ic_flags,ICf_DONT_POP_FLOAT0+
		  cc->cur_ic_float_op_num-1))
	      ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		    MDF_REG+RT_I64,REG_RAX,0,rip2);
	  }
	} else if (tmpi->ic_flags & ICF_RES_TO_INT) {
	  ICFCvt(cc,tmpi,REG_RAX,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		TRUE,CN_RES,rip2);
	  ICMov(tmpi,tmpi->res.type,tmpi->res.reg,tmpi->res.disp,
		MDF_REG+RT_I64,REG_RAX,0,rip2);
	}
      }
    }
    cnt=tmpi->ic_cnt;
    if (tmpi->ic_flags&ICF_DEL_PREV_INS) {
      if (cc->pass>8)
	cnt=tmpi->ic_last_start;
      tmpi->ic_flags&=~ICF_DEL_PREV_INS;
    }
    if (cnt && buf)
      MemCpy(buf+rip,tmpi->ic_body,cnt);
op789A_skip_copy:
    if (dbg_info && cc->min_line<=tmpi->ic_line<=cc->max_line) {
      i=tmpi->ic_line-cc->min_line;
      if (!dbg_info->body[i])
	dbg_info->body[i]=rip2;
    }
    if (tmpi->ic_flags&ICF_PASS_TRACE &&
	  Bt(&cc->saved_pass_trace,cc->pass) && cnt) {
      "$$RED$$";
      if (buf)
	Un(buf+rip,cnt,64);
      else
	Un(tmpi->ic_body,cnt,64);
      "$$FG$$";
    }
    if (!(tmpi->ic_flags&(ICF_CODE_FINAL|ICF_DONT_RESTORE)))
      MemCpy(&tmpi->arg1,saved_arg1_arg2_r,3*sizeof(CICArg));
    rip+=cnt;
    if (tmpi->ic_cnt>=IC_BODY_SIZE && tmpi->ic_code!=IC_ASM)
      throw('Compiler');
op789A_next:
    tmpi=tmpi_next;
  }

  lb=cc->coc.coc_next_misc;
  while (lb!=&cc->coc.coc_next_misc) {
    switch (lb->type) {
      case CMT_STR_CONST:
	lb->addr=rip;
	if (buf)
	  MemCpy(buf+rip,lb->str,lb->st_len);
	rip+=lb->st_len;
	break;
      case CMT_JMP_TABLE:
	lb->addr=rip;
	ptr=buf+lb->addr;
	if (lb->flags&(CMF_I8_JMP_TABLE|CMF_U8_JMP_TABLE)) {
	  if (buf)
	    for (i=0;i<lb->range;i++)
	      *ptr++=lb->jmp_table[i]->addr-lb->begin->addr;
	  rip+=lb->range;
	} else if (lb->flags&(CMF_I16_JMP_TABLE|CMF_U16_JMP_TABLE)) {
	  if (buf)
	    for (i=0;i<lb->range;i++)
	      *ptr(U16 *)++=lb->jmp_table[i]->addr-lb->begin->addr;
	  rip+=lb->range<<1;
	} else {
	  if (buf)
	    for (i=0;i<lb->range;i++) {
	      if (cc->flags&CCF_AOT_COMPILE && !(cc->flags&CCF_NO_ABSS)) {
		tmpa=CAlloc(sizeof(CAOTAbsAddr));
		tmpa->next=aotc->abss;
		tmpa->type=AAT_ADD_U32;
		aotc->abss=tmpa;
		tmpa->rip=aotc->rip+lb->addr+i<<2;
		*ptr(U32 *)++=lb->jmp_table[i]->addr+aotc->rip;
	      } else
		*ptr(U32 *)++=lb->jmp_table[i]->addr+buf;
	    }
	  rip+=lb->range<<2;
	}
	break;
      case CMT_FLOAT_CONSTS:
	lb->addr=rip;
	if (buf)
	  MemCpy(buf+lb->addr,lb->float_consts,lb->num_consts*sizeof(F64));
	rip+=lb->num_consts*sizeof(F64);
	break;
    }
    lb=lb->next;
  }
  if (dbg_info) {
    if (cc->flags&CCF_AOT_COMPILE)
      dbg_info->body[num_lines]=rip+aotc->rip;
    else
      dbg_info->body[num_lines]=rip+buf;
  }
  return rip;
}
