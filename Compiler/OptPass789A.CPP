I64 OptPass789A(CCmpCtrl *cc,COptReg *reg_offsets,U8 *buf,CDbgInfo **_dbg)
{/*cc->pass==7 is first time
cc->pass==8 is second time
cc->pass==9 is third time
cc->pass==9 is fourth time and repeated until size stops shrinking
size is now known
cc->pass==10 is final pass, code is placed into buf.
*/
  CIntermediateCode *tempi,*tempi_next;
  I64 i,cnt,num_lines=cc->max_line+1-cc->min_line,ip=0,ip2;
  U8  *ptr,saved_a1_a2_r[3*sizeof(CICArg)];
  CCodeMisc *lb;
  CAOT *tempaot;
  CAOTAbsAddr *tempa;
  CAOTImportExport *tempie;
  CAOTHeapGlbl *temphg;
  CAOTHeapGlblRef *temphgr;
  CDbgInfo *dbg_info;
  CAOTCtrl *aotc=cc->aotc;
  Bool short_jmp;
  CHashClass *tempc;
  CHashFun *tempf;
  CHashGlblVar *tempg;
  CExternUsage *tempeu;

  if (_dbg) {
    *_dbg=dbg_info=CAlloc(offset(CDbgInfo.body)+sizeof(U32)*(num_lines+1));
    dbg_info->min_line=cc->min_line;
    dbg_info->max_line=cc->max_line;
    if (cc->flags&CCF_AOT_COMPILE)
      dbg_info->body[0]=aotc->ip;
    else
      dbg_info->body[0]=buf;
  } else
    dbg_info=NULL;

  if (Bt(&cc->flags,CCf_PASS_TRACE_PRESENT) &&
	Bt(&cc->saved_pass_trace,cc->pass))
    "$$BK,1$$$$LTRED$$$$IV,1$$This code gets merged together and patched.\n"
	  "$$FG$$$$IV,0$$$$BK,0$$";

  cc->last_float_op_ic=NULL;
  tempi=&cc->coc.coc_head;
  tempi->ic_last_start=-1;
  tempi->ic_cnt=0;
  tempi=tempi->next;
  while (tempi->ic_code) {
    tempi_next=tempi->next;
    if (tempi->ic_flags&ICF_PASS_TRACE && Bt(&cc->saved_pass_trace,cc->pass))
      ICPut(cc,tempi);
    ip2=ip;
    if (cc->flags&CCF_AOT_COMPILE)
      ip2+=aotc->ip;
    else
      ip2+=buf;
    cc->cur_ic_float_op_num=0;
    if (!(tempi->ic_flags &ICF_CODE_FINAL)) {
      tempi->ic_flags=tempi->ic_flags&
	    ~(ICF_PREV_DELETED|ICF_DONT_RESTORE)|ICF_CODE_FINAL;
      if (cc->pass==7)
	cc->dont_push_float=Btr(&tempi->ic_flags,ICf_DONT_PUSH_FLOAT0);
      MemCpy(saved_a1_a2_r,&tempi->a1,3*sizeof(CICArg));
      tempi->ic_cnt=0;
      tempi->ic_last_start=-1;
      if (tempi->a2.type.mode) {
	if (tempi->ic_flags & ICF_A2_TO_F64) {
	  ICFCvt(cc,tempi,REG_RAX,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,FALSE,CN_A2,ip2);
	  tempi->a2.type=MDF_REG+RT_I64;
	  tempi->a2.reg=REG_RAX;
	  tempi->a2.disp=0;
	} else if (tempi->ic_flags & ICF_A2_TO_INT) {
	  ICFCvt(cc,tempi,REG_RAX,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,TRUE,CN_A2,ip2);
	  tempi->a2.type=MDF_REG+RT_I64;
	  tempi->a2.reg=REG_RAX;
	  tempi->a2.disp=0;
	}
      }
      if (tempi->a1.type.mode) {
	if (tempi->ic_flags & ICF_A1_TO_F64) {
	  ICFCvt(cc,tempi,REG_RDX,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,FALSE,CN_A1,ip2);
	  tempi->a1.type=MDF_REG+RT_I64;
	  tempi->a1.reg=REG_RDX;
	  tempi->a1.disp=0;
	} else if (tempi->ic_flags & ICF_A1_TO_INT) {
	  ICFCvt(cc,tempi,REG_RDX,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,TRUE,CN_A1,ip2);
	  tempi->a1.type=MDF_REG+RT_I64;
	  tempi->a1.reg=REG_RDX;
	  tempi->a1.disp=0;
	}
      }

      switch [tempi->ic_code] {
	start:
	  start:
	    case IC_ABS_ADDR:
	      ICU16(tempi,0xB848);
	      ICU64(tempi,tempi->ic_data);
	      if (buf && cc->flags&CCF_AOT_COMPILE &&
		    !(cc->flags&(CCF_NO_ABSS|CCF_ASM_EXPRESSIONS))) {
		tempa=CAlloc(sizeof(CAOTAbsAddr));
		tempa->next=aotc->abss;
		tempa->type=AAT_ADD_U64;
		aotc->abss=tempa;
		tempa->ip=ip2+tempi->ic_cnt-8;
	      }
	      break;
	    case IC_HEAP_GLBL:
	      ICU16(tempi,0xB848);
	      ICU64(tempi,0);
	      temphg=tempi->ic_data;
	      if (buf && cc->flags&CCF_AOT_COMPILE &&
//TODO:is this necessary--flags?
		    !(cc->flags&(CCF_NO_ABSS|CCF_ASM_EXPRESSIONS))) {
		temphgr=CAlloc(sizeof(CAOTHeapGlblRef));
		temphgr->next=temphg->references;
		temphg->references=temphgr;
		temphgr->ip=ip2+tempi->ic_cnt-8;
	      }
	      break;
	    case IC_ADDR_IMPORT:
	      ICU8(tempi,0xB8);
	      ICU32(tempi,0);
	      if (buf && !(cc->flags&CCF_NO_ABSS)) {
		tempg=tempi->ic_data;
		tempie=CAlloc(sizeof(CAOTImportExport));
		tempie->type=IET_IMM_U32;
		tempie->ip=ip2+tempi->ic_cnt-4;
		tempie->next=tempg->ie_lst;
		tempg->ie_lst=tempie;
	      }
	      ICU24(tempi,0xC06348);
	      break;
	    case IC_RIP:
	      ICU16(tempi,0xB848);
	      ICU64(tempi,ip2+tempi->ic_cnt-2);
	      if (cc->flags&CCF_AOT_COMPILE && buf &&!(cc->flags&CCF_NO_ABSS)) {
		tempa=CAlloc(sizeof(CAOTAbsAddr));
		tempa->next=aotc->abss;
		tempa->type=AAT_ADD_U64;
		aotc->abss=tempa;
		tempa->ip=ip2+tempi->ic_cnt-8;
	      }
	      break;
	  end:
	    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  MDF_REG+RT_I64,REG_RAX,0,ip2);
	    break;
	  case IC_BR_CARRY:
	    ICFlagBranch(tempi,ip,0x72820F,buf);
	    break;
	  case IC_BR_NOT_CARRY:
	    ICFlagBranch(tempi,ip,0x73830F,buf);
	    break;
	  case IC_BR_ZERO:
	    ICTestAndBranch(tempi,ip,0x74840F,buf,ip2);
	    break;
	  case IC_BR_NOT_ZERO:
	    ICTestAndBranch(tempi,ip,0x75850F,buf,ip2);
	    break;
	  case IC_BR_MM_ZERO:
	    ICPreIncDec(tempi,SLASH_OP_DEC,ip2);
	    ICFlagBranch(tempi,ip,0x74840F,buf);
	    break;
	  case IC_BR_MM_NOT_ZERO:
	    ICPreIncDec(tempi,SLASH_OP_DEC,ip2);
	    ICFlagBranch(tempi,ip,0x75850F,buf);
	    break;
	  case IC_BR_EQU_EQU:
	    ICCmpAndBranch(tempi,FALSE,ip,0x74840F,0x74840F,
		  0x74840F,0x74840F,buf,ip2);
	    break;
	  case IC_BR_EQU_EQU2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x74840F,0x74840F,
		  0x74840F,0x74840F,buf,ip2);
	    break;
	  case IC_BR_NOT_EQU:
	    ICCmpAndBranch(tempi,FALSE,ip,0x75850F,0x75850F,
		  0x75850F,0x75850F,buf,ip2);
	    break;
	  case IC_BR_NOT_EQU2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x75850F,0x75850F,
		  0x75850F,0x75850F,buf,ip2);
	    break;
	  case IC_BR_LESS:
	    if (tempi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tempi,  ip,0x72820F,0x77870F,buf,ip2);
	    else
	      ICCmpAndBranch(tempi,FALSE,ip,0x72820F,0x7C8C0F,
		    0x77870F,0x7F8F0F,buf,ip2);
	    break;
	  case IC_BR_LESS2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x72820F,0x7C8C0F,
		  0x77870F,0x7F8F0F,buf,ip2);
	    break;
	  case IC_BR_GREATER_EQU:
	    if (tempi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tempi,  ip,0x73830F,0x76860F,buf,ip2);
	    else
	      ICCmpAndBranch(tempi,FALSE,ip,0x73830F,0x7D8D0F,
		    0x76860F,0x7E8E0F,buf,ip2);
	    break;
	  case IC_BR_GREATER_EQU2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x73830F,0x7D8D0F,
		  0x76860F,0x7E8E0F,buf,ip2);
	    break;
	  case IC_BR_GREATER:
	    if (tempi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tempi,  ip,0x77870F,0x72820F,buf,ip2);
	    else
	      ICCmpAndBranch(tempi,FALSE,ip,0x77870F,0x7F8F0F,
		    0x72820F,0x7C8C0F,buf,ip2);
	    break;
	  case IC_BR_GREATER2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x77870F,0x7F8F0F,
		  0x72820F,0x7C8C0F,buf,ip2);
	    break;
	  case IC_BR_LESS_EQU:
	    if (tempi->ic_flags&ICF_USE_F64)
	      ICFCmpAndBranch(cc,tempi,  ip,0x76860F,0x73830F,buf,ip2);
	    else
	      ICCmpAndBranch(tempi,FALSE,ip,0x76860F,0x7E8E0F,
		    0x73830F,0x7D8D0F,buf,ip2);
	    break;
	  case IC_BR_LESS_EQU2:
	    ICCmpAndBranch(tempi,TRUE,ip,0x76860F,0x7E8E0F,
		  0x73830F,0x7D8D0F,buf,ip2);
	    break;
	  case IC_BR_BT:
	    ICBrBitOps(tempi,ip,0xA30F,0x20BA0F,0x72820F,buf,ip2);
	    break;
	  case IC_BR_BTS:
	    ICBrBitOps(tempi,ip,0xAB0F,0x28BA0F,0x72820F,buf,ip2);
	    break;
	  case IC_BR_BTR:
	    ICBrBitOps(tempi,ip,0xB30F,0x30BA0F,0x72820F,buf,ip2);
	    break;
	  case IC_BR_BTC:
	    ICBrBitOps(tempi,ip,0xBB0F,0x38BA0F,0x72820F,buf,ip2);
	    break;
	  case IC_BR_NOT_BT:
	    ICBrBitOps(tempi,ip,0xA30F,0x20BA0F,0x73830F,buf,ip2);
	    break;
	  case IC_BR_NOT_BTS:
	    ICBrBitOps(tempi,ip,0xAB0F,0x28BA0F,0x73830F,buf,ip2);
	    break;
	  case IC_BR_NOT_BTR:
	    ICBrBitOps(tempi,ip,0xB30F,0x30BA0F,0x73830F,buf,ip2);
	    break;
	  case IC_BR_NOT_BTC:
	    ICBrBitOps(tempi,ip,0xBB0F,0x38BA0F,0x73830F,buf,ip2);
	    break;
	  case IC_BR_AND_ZERO:
	    ICAndBranch(tempi,ip,0x74840F,buf,ip2);
	    break;
	  case IC_BR_AND_NOT_ZERO:
	    ICAndBranch(tempi,ip,0x75850F,buf,ip2);
	    break;
	  case IC_SUB_CALL:
	    lb=OptLabelFwd(tempi->ic_data);
	    ICU8(tempi,0xE8);
	    ICU32(tempi,lb->addr-(ip+5));
	    break;
	  case IC_JMP:
	    lb=OptLabelFwd(tempi->ic_data);
	    short_jmp=ToBool(tempi->ic_flags&ICF_SHORT_JMP);
	    if (!buf && lb->addr!=INVALID_PTR &&
		  MIN_I8+5<lb->addr-ip<MAX_I8-5)
	      short_jmp=TRUE;
	    if (short_jmp) {
	      tempi->ic_flags|=ICF_SHORT_JMP;
	      i=lb->addr-(ip+2);
	      if (buf || i)
		ICU16(tempi,i<<8+0xEB);
	      else
		tempi->ic_code=IC_NOP1;
	    } else {
	      i=lb->addr-(ip+5);
	      ICU8(tempi,0xE9);
	      ICU32(tempi,i);
	    }
	    break;
	  case IC_LABEL:
	    lb=tempi->ic_data;
	    lb->addr=ip;
	    if (lb->flags&CMF_POP_CMP) {
	      ICAddRSP(tempi,-8,FALSE);
	      ICAddRSP(tempi,8,FALSE);
	    }
	    if (lb->type==CMT_ASM_LABEL)
	      lb->addr+=lb->ip;
	    break;
	  case IC_STR_CONST:
	  case IC_GET_LABEL:
	    lb=tempi->ic_data;
	    if (cc->flags&CCF_AOT_COMPILE)
	      i=lb->addr+aotc->ip;
	    else
	      i=lb->addr+buf;
	    ICLea(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  MDF_RIP_DISP32+RT_PTR,0,i,cc,buf,ip2);
	    break;
	  case IC_ASM:
	    tempaot=tempi->ic_data;
	    tempi->ic_cnt+=tempaot->aot_U8s;
	    if (buf) {
	      MemCpy(buf+ip,tempaot->buf,tempaot->aot_U8s);
	      Free(tempaot->buf);
	      tempaot->buf=buf;
	      tempaot->ip=ip;
	      tempaot->ip2=ip2;
	      if (cc->flags&CCF_AOT_COMPILE)
		CmpFixUpAOTAsm(cc,tempaot);
	      else
		CmpFixUpJITAsm(cc,tempaot);
	      cnt=tempi->ic_cnt;
	      goto op789A_skip_copy;
	    }
	    break;
	  case IC_CALL:
	    i=tempi->ic_data-(ip2+5);
	    if (!(MIN_I32<=i<=MAX_I32) && !(cc->flags&CCF_AOT_COMPILE)) {
	      ICU16(tempi,0xBB48);
	      ICU64(tempi,tempi->ic_data);
	      ICU16(tempi,0xD3FF);
	    } else {
	      ICU8(tempi,0xE8);
	      ICU32(tempi,i);
	    }
	    break;
	  case IC_CALL_EXTERN: //Only for static modules
	    ICU8(tempi,0xE8);
	    ICU32(tempi,0);
	    if (buf) {
	      tempf=tempi->ic_data;
	      tempeu=CAlloc(sizeof(CExternUsage));
	      tempeu->next=tempf->ext_lst;
	      tempf->ext_lst=tempeu;
	      tempeu->ip=ip2+1;
	    }
	    break;
	  case IC_CALL_INDIRECT2:
	    ICU16(tempi,0xBB48);
	    if (cc->flags&CCF_AOT_COMPILE) i=ip2+tempi->ic_cnt;
	    ICU64(tempi,tempi->ic_data);
	    ICU16(tempi,0x13FF);
	    if (buf && cc->flags&CCF_AOT_COMPILE&& !(cc->flags&CCF_NO_ABSS)) {
	      tempa=CAlloc(sizeof(CAOTAbsAddr));
	      tempa->next=aotc->abss;
	      tempa->type=AAT_ADD_U64;
	      aotc->abss=tempa;
	      tempa->ip=i;
	    }
	    break;
	  case IC_CALL_IMPORT:
	    if (GetOption(OPTf_USE_IMM64)) {
	      ICU16(tempi,0xBB48);
	      ICU64(tempi,0);
	      if (buf) {
		tempf=tempi->ic_data;
		tempie=CAlloc(sizeof(CAOTImportExport));
		tempie->type=IET_IMM_I64;
		tempie->ip=ip2+tempi->ic_cnt-8;
		tempie->next=tempf->ie_lst;
		tempf->ie_lst=tempie;
	      }
	      ICU16(tempi,0xD3FF);
	    } else {
	      ICU8(tempi,0xE8);
	      ICU32(tempi,0);
	      if (buf) {
		tempf=tempi->ic_data;
		tempie=CAlloc(sizeof(CAOTImportExport));
		tempie->type=IET_REL_I32;
		tempie->ip=ip2+tempi->ic_cnt-4;
		tempie->next=tempf->ie_lst;
		tempf->ie_lst=tempie;
	      }
	    }
	    break;
	end:
	  tempi->ic_flags&=~ICF_CODE_FINAL;
	  break;
	case IC_LEAVE:
	  if (cc->htc.fun) {
	    if (Bt(&cc->htc.fun->flags,Ff_INTERRUPT))
	      ICPopRegs(tempi,REGG_CLOBBERED|cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS|REGG_STK_TEMP));
	    else
	      ICPopRegs(tempi,cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS));
	  }
	  if (tempi->ic_data<=MAX_I16) {
	    if (tempi->ic_data)
	      ICU8(tempi,0xC9); //LEAVE
	    else
	      ICU8(tempi,0x5D); //POP RBP
	  } else {
	    ICAddRSP(tempi,tempi->ic_data);
	    ICU8(tempi,0x5D); //POP RBP
	  }
	  if (cc->htc.fun && Bt(&cc->htc.fun->flags,Ff_INTERRUPT)) {
	    if (Bt(&cc->htc.fun->flags,Ff_HASERRCODE))
	      ICAddRSP(tempi,8);
	    ICU16(tempi,0xCF48);
	  } else if (cc->htc.fun && cc->htc.fun->arg_cnt &&
		(Bt(&cc->htc.fun->flags,Ff_RET1) ||
		Bt(&cc->htc.fun->flags,Ff_ARGPOP)) &&
		!Bt(&cc->htc.fun->flags,Ff_NOARGPOP)) {
	    ICU8(tempi,0xC2);
	    ICU16(tempi,cc->htc.fun->arg_cnt<<3);
	  } else
	    ICU8(tempi,0xC3);
	  break;
	case IC_RET:
	  ICU8(tempi,0xC3);
	  break;
	case IC_FS:
	  ICZero(tempi,REG_RAX);
	  ICU32(tempi,0x8B4864);
	  break;
	case IC_GS:
	  ICZero(tempi,REG_RAX);
	  ICU32(tempi,0x8B4865);
	  break;
	case IC_MOV_FS:
	  ICZero(tempi,REG_RAX);
	  ICU8(tempi,0x64);
//It's ugly to use ic_class here
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		MDF_DISP+CmpRawType(tempi->ic_class),REG_RAX,tempi->ic_data,
		ip2);
	  break;
	case IC_MOV_GS:
	  ICZero(tempi,REG_RAX);
	  ICU8(tempi,0x65);
//It's ugly to use ic_class here
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		MDF_DISP+CmpRawType(tempi->ic_class),REG_RAX,tempi->ic_data,
		ip2);
	  break;
	case IC_HOLYC_TYPECAST:
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_COM:
	  ICUnaries(tempi,SLASH_OP_NOT,ip2);
	  break;
	case IC_NOT:
	  ICNot(tempi,ip2);
	  break;
	case IC_UNARY_MINUS:
	  if (tempi->r.type.raw_type==RT_F64)
	    ICFUnaryMinus(cc,tempi,buf,ip2);
	  else
	    ICUnaries(tempi,SLASH_OP_NEG,ip2);
	  break;
	case IC_ADDR:
	case IC_MOV:
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_DEREF:
	  ICDeref(tempi,ip2);
	  break;
	case IC_DEREF_PP:
	  ICDerefPostIncDec(tempi,SLASH_OP_INC,ip2);
	  break;
	case IC_DEREF_MM:
	  ICDerefPostIncDec(tempi,SLASH_OP_DEC,ip2);
	  break;
	case IC__PP:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICPostIncDec(tempi,SLASH_OP_INC,ip2);
	  else
	    ICFPostIncDec(cc,tempi,CMP_TEMPLATE_INC,ip2);
	  break;
	case IC__MM:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICPostIncDec(tempi,SLASH_OP_DEC,ip2);
	  else
	    ICFPostIncDec(cc,tempi,CMP_TEMPLATE_DEC,ip2);
	  break;
	case IC_PP_:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICPreIncDec(tempi,SLASH_OP_INC,ip2);
	  else
	    ICFPreIncDec(cc,tempi,CMP_TEMPLATE_INC,ip2);
	  break;
	case IC_MM_:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICPreIncDec(tempi,SLASH_OP_DEC,ip2);
	  else
	    ICFPreIncDec(cc,tempi,CMP_TEMPLATE_DEC,ip2);
	  break;
	case IC_LEA:
	  ICLea(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,cc,buf,ip2);
	  break;
	case IC_POWER:
	  ICFPow(cc,tempi,buf,ip2);
	  break;
	case IC_SHL:
	  ICShift(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,ip2);
	  break;
	case IC_SHR:
	  ICShift(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,ip2);
	  break;
	case IC_MUL:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICMul(tempi,ip2);
	  else
	    ICFMul(cc,tempi,buf,ip2);
	  break;
	case IC_DIV:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICDiv(tempi,ip2);
	  else
	    ICFDiv(cc,tempi,buf,ip2);
	  break;
	case IC_MOD:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICMod(tempi,ip2);
	  else
	    ICFMod(cc,tempi,ip2);
	  break;
	case IC_AND:
	  ICAddEct(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x23,ip2);
	  break;
	case IC_OR:
	  ICAddEct(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x0B,ip2);
	  break;
	case IC_XOR:
	  ICAddEct(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x33,ip2);
	  break;
	case IC_ADD:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICAddEct(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,0x03,ip2);
	  else
	    ICFAdd(cc,tempi,buf,ip2);
	  break;
	case IC_SUB:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICSub(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	  else
	    ICFSub(cc,tempi,buf,ip2);
	  break;
	case IC_EQU_EQU:
	  ICCmp(tempi,0x75,0x75,ip2);
	  break;
	case IC_NOT_EQU:
	  ICCmp(tempi,0x74,0x74,ip2);
	  break;
	case IC_LESS:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICCmp(tempi,0x73,0x7D,ip2);
	  else
	    ICFCmp(cc,tempi,CMP_TEMPLATE_LESS,ip2);
	  break;
	case IC_GREATER_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICCmp(tempi,0x72,0x7C,ip2);
	  else
	    ICFCmp(cc,tempi,CMP_TEMPLATE_GREATER_EQU,ip2);
	  break;
	case IC_GREATER:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICCmp(tempi,0x76,0x7E,ip2);
	  else
	    ICFCmp(cc,tempi,CMP_TEMPLATE_GREATER,ip2);
	  break;
	case IC_LESS_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICCmp(tempi,0x77,0x7F,ip2);
	  else
	    ICFCmp(cc,tempi,CMP_TEMPLATE_LESS_EQU,ip2);
	  break;
	case IC_AND_AND:
	  ICAndAnd(tempi,ip2);
	  break;
	case IC_OR_OR:
	  ICOrOr(tempi,ip2);
	  break;
	case IC_XOR_XOR:
	  ICXorXor(tempi,ip2);
	  break;
	case IC_ASSIGN:
	  ICAssign(tempi,ip2);
	  break;
	case IC_ASSIGN_PP:
	  ICAssignPostIncDec(tempi,SLASH_OP_INC,ip2);
	  break;
	case IC_ASSIGN_MM:
	  ICAssignPostIncDec(tempi,SLASH_OP_DEC,ip2);
	  break;
	case IC_SHL_EQU:
	  ICShiftEqu(tempi,tempi->a1_type_pointed_to,
		tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,ip2);
	  break;
	case IC_SHR_EQU:
	  ICShiftEqu(tempi,tempi->a1_type_pointed_to,
		tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,ip2);
	  break;
	case IC_MUL_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICMulEqu(tempi,ip2);
	  else
	    ICFOpEqu(cc,tempi,SLASH_OP_FMUL,buf,ip2);
	  break;
	case IC_DIV_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICDivEqu(tempi,FALSE,ip2);
	  else
	    ICFOpEqu(cc,tempi,SLASH_OP_FDIV,buf,ip2);
	  break;
	case IC_MOD_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICDivEqu(tempi,TRUE,ip2);
	  else
	    ICFModEqu(cc,tempi,ip2);
	  break;
	case IC_AND_EQU:
	  ICAndEqu(tempi,ip2);
	  break;
	case IC_OR_EQU:
	  ICOrEqu(tempi,ip2);
	  break;
	case IC_XOR_EQU:
	  ICXorEqu(tempi,ip2);
	  break;
	case IC_ADD_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICAddSubEctEqu(tempi,tempi->a1_type_pointed_to,
		  tempi->r.type,tempi->r.reg,tempi->r.disp,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		  0x010000000003,ip2);
	  else
	    ICFOpEqu(cc,tempi,SLASH_OP_FADD,buf,ip2);
	  break;
	case IC_SUB_EQU:
	  if (tempi->ic_flags&ICF_USE_INT)
	    ICAddSubEctEqu(tempi,tempi->a1_type_pointed_to,
		  tempi->r.type,tempi->r.reg,tempi->r.disp,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,
		  0x29000000052B,ip2);
	  else
	    ICFOpEqu(cc,tempi,SLASH_OP_FSUB,buf,ip2);
	  break;
	case IC_SHL_CONST:
	  ICShift(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		MDF_IMM+RT_I64,0,tempi->ic_data,
		0xE0D1E0D3E0C1,0xE0D1E0D3E0C1,ip2);
	  break;
	case IC_SHR_CONST:
	  ICShift(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,
		MDF_IMM+RT_I64,0,tempi->ic_data,
		0xE8D1E8D3E8C1,0xF8D1F8D3F8C1,ip2);
	  break;
	case IC_ADD_CONST:
	  ICAddSubEctImm(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,tempi->ic_data,
		0x0003,ip2);
	  break;
	case IC_SUB_CONST:
	  ICAddSubEctImm(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,tempi->ic_data,
		0x052B,ip2);
	  break;
	case IC_ENTER:
	  ICU32(tempi,0xEC8B4855);
	  if (tempi->ic_data)
	    ICAddRSP(tempi,-tempi->ic_data,FALSE);
	  if (cc->htc.fun) {
	    if (Bt(&cc->htc.fun->flags,Ff_INTERRUPT))
	      ICPushRegs(tempi,REGG_CLOBBERED|cc->htc.fun->used_reg_mask&
		    (REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS|REGG_STK_TEMP));
	    else {
	      if (sys_var_init_flag && i)
		ICLocalVarInit(tempi);
	      ICPushRegs(tempi,cc->htc.fun->used_reg_mask
		    &(REGG_LOCAL_VARS|REGG_LOCAL_NON_PTR_VARS));
	    }
	    for (i=0;i<NUM_REGS;i++)
	      if (reg_offsets[i]>0 && reg_offsets[i].offset!=MAX_I64) {
		tempc=OptClassFwd(reg_offsets[i].m->member_class);
		ICMov(tempi,MDF_REG+RT_I64,i,0,MDF_DISP+tempc->raw_type,
		      REG_RBP,reg_offsets[i].offset,ip2);
	      }
	  }
	  break;
	case IC_ADD_RSP:
	  ICAddRSP(tempi,tempi->ic_data);
	  break;
	case IC_CALL_INDIRECT:
	  if (MIN_I8<=tempi->ic_data<=MAX_I8) {
	    ICU24(tempi,0x2454FF);  //CALL disp[RSP]
	    ICU8(tempi,tempi->ic_data);
	  } else {
	    ICU24(tempi,0x2494FF);  //CALL disp[RSP]
	    ICU32(tempi,tempi->ic_data);
	  }
	  break;
	case IC_PUSH:
	  ICPush(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_POP:
	  ICU8(tempi,0x58);
	  break;
	case IC_INVLPG:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU24(tempi,0x38010F);
	  break;
	case IC_CLFLUSH:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU24(tempi,0x38AE0F);
	  break;
	case IC_GET_RFLAGS:
	  ICU8(tempi,0x9C);
	  ICPop(tempi,MDF_REG+RT_I64,REG_RAX,0,ip2);
	  break;
	case IC_CARRY:
	  ICU24(tempi,0xC0920F); //SETC AL
	  ICU24(tempi,0x01E083); //AND EAX,1
	  break;
	case IC_RDTSC:
	  ICCopyTemplate(cc,tempi,CMP_TEMPLATE_RDTSC,TRUE,FALSE,FALSE,CN_MAIN);
	  break;
	case IC_SET_RFLAGS:
	  ICPush(tempi,tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU8(tempi,0x9D);
	  break;
	case IC_GET_RBP:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_REG+RT_I64,REG_RBP,0,ip2);
	  break;
	case IC_SET_RBP:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RBP,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_GET_RSP:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		MDF_REG+RT_I64,REG_RSP,0,ip2);
	  break;
	case IC_SET_RSP:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RSP,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_RETURN_VAL:
	case IC_SET_RAX:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  break;
	case IC_RETURN_VAL2:
	case IC_GET_RAX:
	  break;
	case IC_BT:
	  ICBitOps(tempi,&tempi->a1,&tempi->a2,tempi->next,
		0xA30F,0x20BA0F,ip2);
	  break;
	case IC_BTS:
	case IC_LBTS:
	  ICBitOps(tempi,&tempi->a1,&tempi->a2,tempi->next,
		0xAB0F,0x28BA0F,ip2);
	  break;
	case IC_BTR:
	case IC_LBTR:
	  ICBitOps(tempi,&tempi->a1,&tempi->a2,tempi->next,
		0xB30F,0x30BA0F,ip2);
	  break;
	case IC_BTC:
	case IC_LBTC:
	  ICBitOps(tempi,&tempi->a1,&tempi->a2,tempi->next,
		0xBB0F,0x38BA0F,ip2);
	  break;
	case IC_BSF:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU32(tempi,0xC0BC0F48);
	  ICU16(tempi,0x0375);
	  ICU24(tempi,0xD0F748);
	  break;
	case IC_BSR:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU32(tempi,0xC0BD0F48);
	  ICU16(tempi,0x0375);
	  ICU24(tempi,0xD0F748);
	  break;
	case IC_SIGN_I64:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICCopyTemplate(cc,tempi,CMP_TEMPLATE_SIGN_I64,
		TRUE,FALSE,FALSE,CN_MAIN);
	  break;
	case IC_TOUPPER:
	  ICToUpper(tempi,ip2);
	  break;
	case IC_TO_I64:
	  ICToI64(cc,tempi,ip2);
	  break;
	case IC_TO_F64:
	  ICToF64(cc,tempi,ip2);
	  break;
	case IC_TO_BOOL:
	  ICToBool(cc,tempi,ip2);
	  break;
	case IC_SQR:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_SQR,ip2);
	  break;
	case IC_ABS:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_ABS,ip2);
	  break;
	case IC_SQRT:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_SQRT,ip2);
	  break;
	case IC_SIN:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_SIN,ip2);
	  break;
	case IC_COS:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_COS,ip2);
	  break;
	case IC_TAN:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_TAN,ip2);
	  break;
	case IC_ATAN:
	  ICFTemplateFun(cc,tempi,CMP_TEMPLATE_ATAN,ip2);
	  break;
	case IC_ABS_I64:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICU24(tempi,0xC08548);
	  ICU16(tempi,0x0379);
	  ICU24(tempi,0xD8F748);
	  break;
	case IC_MIN_I64:
	  ICMinMax(tempi,0x4F,ip2);
	  break;
	case IC_MAX_I64:
	  ICMinMax(tempi,0x4C,ip2);
	  break;
	case IC_MIN_U64:
	  ICMinMax(tempi,0x47,ip2);
	  break;
	case IC_MAX_U64:
	  ICMinMax(tempi,0x42,ip2);
	  break;
	case IC_MOD_U64:
	  ICModU64(tempi,ip2);
	  break;
	case IC_SQR_I64:
	  ICSqr(tempi,SLASH_OP_IMUL,ip2);
	  break;
	case IC_SQR_U64:
	  ICSqr(tempi,SLASH_OP_MUL,ip2);
	  break;
	case IC_SWAP_U8:
	case IC_SWAP_U16:
	case IC_SWAP_U32:
	case IC_SWAP_I64:
	  ICSwap(tempi,ip2);
	  break;
	case IC_QUE_INIT:
	  ICQueInit(tempi,ip2);
	  break;
	case IC_QUE_INS:
	  ICQueIns(tempi,ip2);
	  break;
	case IC_QUE_INS_REV:
	  ICQueInsRev(tempi,ip2);
	  break;
	case IC_QUE_REM:
	  ICQueRem(tempi,ip2);
	  break;
	case IC_STRLEN:
	  ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	  ICCopyTemplate(cc,tempi,CMP_TEMPLATE_STRLEN,TRUE,FALSE,FALSE,CN_MAIN);
	  break;
	case IC_IN_U32:
	  if (tempi->a1.type&MDF_IMM) {
	    ICU16(tempi,0xC033);
	    if (tempi->a1.disp<=MAX_U8)
	      ICU16(tempi,0xE5+tempi->a1.disp<<8);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a1.disp<<16);
	      ICU8(tempi,0xED);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU16(tempi,0xC033);
	    ICU8(tempi,0xED);
	  }
	  break;
	case IC_IN_U16:
	  if (tempi->a1.type&MDF_IMM) {
	    ICU16(tempi,0xC033);
	    if (tempi->a1.disp<=MAX_U8)
	      ICU24(tempi,0xE500+OC_OP_SIZE_PREFIX+tempi->a1.disp<<16);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a1.disp<<16);
	      ICU16(tempi,0xED00+OC_OP_SIZE_PREFIX);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU16(tempi,0xC033);
	    ICU16(tempi,0xED00+OC_OP_SIZE_PREFIX);
	  }
	  break;
	case IC_IN_U8:
	  if (tempi->a1.type&MDF_IMM) {
	    ICU16(tempi,0xC033);
	    if (tempi->a1.disp<=MAX_U8)
	      ICU16(tempi,0xE4+tempi->a1.disp<<8);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a1.disp<<16);
	      ICU8(tempi,0xEC);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU16(tempi,0xC033);
	    ICU8(tempi,0xEC);
	  }
	  break;
	case IC_OUT_U32:
	  if (tempi->a2.type&MDF_IMM) {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    if (tempi->a2.disp<=MAX_U8)
	      ICU16(tempi,0xE7+tempi->a2.disp<<8);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a2.disp<<16);
	      ICU8(tempi,0xEF);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU8(tempi,0xEF);
	  }
	  break;
	case IC_OUT_U16:
	  if (tempi->a2.type&MDF_IMM) {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    if (tempi->a2.disp<=MAX_U8)
	      ICU24(tempi,0xE700+OC_OP_SIZE_PREFIX+tempi->a2.disp<<16);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a2.disp<<16);
	      ICU16(tempi,0xEF00+OC_OP_SIZE_PREFIX);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU16(tempi,0xEF00+OC_OP_SIZE_PREFIX);
	  }
	  break;
	case IC_OUT_U8:
	  if (tempi->a2.type&MDF_IMM) {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    if (tempi->a2.disp<=MAX_U8)
	      ICU16(tempi,0xE6+tempi->a2.disp<<8);
	    else {
	      ICU32(tempi,0xBA00+OC_OP_SIZE_PREFIX+tempi->a2.disp<<16);
	      ICU8(tempi,0xEE);
	    }
	  } else {
	    ICMov(tempi,MDF_REG+RT_I64,REG_RDX,0,
		  tempi->a2.type,tempi->a2.reg,tempi->a2.disp,ip2);
	    ICMov(tempi,MDF_REG+RT_I64,REG_RAX,0,
		  tempi->a1.type,tempi->a1.reg,tempi->a1.disp,ip2);
	    ICU8(tempi,0xEE);
	  }
	  break;
	case IC_NOBOUND_SWITCH:
	  ICSwitch(tempi,ip,TRUE,cc,buf,ip2);
	  break;
	case IC_SWITCH:
	  ICSwitch(tempi,ip,FALSE,cc,buf,ip2);
	  break;
	case IC_NOP1:
	case IC_NOP2:
	  OptFree(tempi);
	  goto op789A_next;
	case IC_CALL_START:
	case IC_PUSH_REGS:
	  ICPushRegs(tempi,tempi->ic_data);
	  break;
	case IC_CALL_END:
	  ICPopRegs(tempi,tempi->ic_data);
	  if (tempi->r.type.mode)
	    ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  MDF_REG+RT_I64,REG_RAX,0,ip2);
	  break;
	case IC_POP_REGS:
	  ICPopRegs(tempi,tempi->ic_data);
	  break;
	case IC_PUSH_CMP:
	case IC_CALL_END2:
	case IC_END:
	case IC_ADD_RSP1:
	  break;
	default:
	  "Pass:%d Missing IC handler\n",cc->pass;
	  ICPut(cc,tempi);
	  LexExcept(cc,"Compiler Optimization Error at ");
      }
      if (tempi->r.type.mode) {
	if (tempi->ic_flags & ICF_R_TO_F64) {
	  if (tempi->ic_code==IC_PUSH_CMP) {
	    ICU24(tempi,0x242CDF); //FILD U64 [RSP]
	    ICU24(tempi,0x241CDD); //FSTP U64 [RSP]
	  } else {
	    ICFCvt(cc,tempi,REG_RAX,tempi->r.type,tempi->r.reg,tempi->r.disp,
		  FALSE,CN_R,ip2);
	    if (!Bt(&tempi->ic_flags,ICf_DONT_POP_FLOAT0+
		  cc->cur_ic_float_op_num-1))
	      ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		    MDF_REG+RT_I64,REG_RAX,0,ip2);
	  }
	} else if (tempi->ic_flags & ICF_R_TO_INT) {
	  ICFCvt(cc,tempi,REG_RAX,tempi->r.type,tempi->r.reg,tempi->r.disp,
		TRUE,CN_R,ip2);
	  ICMov(tempi,tempi->r.type,tempi->r.reg,tempi->r.disp,
		MDF_REG+RT_I64,REG_RAX,0,ip2);
	}
      }
    }
    cnt=tempi->ic_cnt;
    if (tempi->ic_flags&ICF_DEL_PREV_INS) {
      if (cc->pass>8)
	cnt=tempi->ic_last_start;
      tempi->ic_flags&=~ICF_DEL_PREV_INS;
    }
    if (cnt && buf)
      MemCpy(buf+ip,tempi->ic_body,cnt);
op789A_skip_copy:
    if (dbg_info && cc->min_line<=tempi->ic_line<=cc->max_line) {
      i=tempi->ic_line-cc->min_line;
      if (!dbg_info->body[i])
	dbg_info->body[i]=ip2;
    }
    if (tempi->ic_flags&ICF_PASS_TRACE &&
	  Bt(&cc->saved_pass_trace,cc->pass) && cnt) {
      "$$RED$$";
      if (buf)
	Un(buf+ip,cnt,64);
      else
	Un(tempi->ic_body,cnt,64);
      "$$FG$$";
    }
    if (!(tempi->ic_flags&(ICF_CODE_FINAL|ICF_DONT_RESTORE)))
      MemCpy(&tempi->a1,saved_a1_a2_r,3*sizeof(CICArg));
    ip+=cnt;
    if (tempi->ic_cnt>=IC_BODY_SIZE && tempi->ic_code!=IC_ASM)
      throw('Compiler');
op789A_next:
    tempi=tempi_next;
  }

  lb=cc->coc.coc_next_misc;
  while (lb!=&cc->coc.coc_next_misc) {
    switch (lb->type) {
      case CMT_STR_CONST:
	lb->addr=ip;
	if (buf)
	  MemCpy(buf+ip,lb->str,lb->st_len);
	ip+=lb->st_len;
	break;
      case CMT_JMP_TABLE:
	lb->addr=ip;
	ptr=buf+lb->addr;
	if (lb->flags&(CMF_I8_JMP_TABLE|CMF_U8_JMP_TABLE)) {
	  if (buf)
	    for (i=0;i<lb->range;i++)
	      *ptr++=lb->jmp_table[i]->addr-lb->begin->addr;
	  ip+=lb->range;
	} else if (lb->flags&(CMF_I16_JMP_TABLE|CMF_U16_JMP_TABLE)) {
	  if (buf)
	    for (i=0;i<lb->range;i++)
	      *ptr(U16 *)++=lb->jmp_table[i]->addr-lb->begin->addr;
	  ip+=lb->range<<1;
	} else {
	  if (buf)
	    for (i=0;i<lb->range;i++) {
	      if (cc->flags&CCF_AOT_COMPILE && !(cc->flags&CCF_NO_ABSS)) {
		tempa=CAlloc(sizeof(CAOTAbsAddr));
		tempa->next=aotc->abss;
		tempa->type=AAT_ADD_U32;
		aotc->abss=tempa;
		tempa->ip=aotc->ip+lb->addr+i<<2;
		*ptr(U32 *)++=lb->jmp_table[i]->addr+aotc->ip;
	      } else
		*ptr(U32 *)++=lb->jmp_table[i]->addr+buf;
	    }
	  ip+=lb->range<<2;
	}
	break;
      case CMT_FLOAT_CONSTS:
	lb->addr=ip;
	if (buf)
	  MemCpy(buf+lb->addr,lb->float_consts,lb->num_consts*sizeof(F64));
	ip+=lb->num_consts*sizeof(F64);
	break;
    }
    lb=lb->next;
  }
  if (dbg_info) {
    if (cc->flags&CCF_AOT_COMPILE)
      dbg_info->body[num_lines]=ip+aotc->ip;
    else
      dbg_info->body[num_lines]=ip+buf;
  }
  return ip;
}
