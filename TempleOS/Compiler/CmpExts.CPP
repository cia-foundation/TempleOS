import U0 DocLoad(CDoc *doc,U8 *src2,I64 size);
import CDoc *DocNew(CTask *task=NULL);
import U0 DocDel(CDoc *doc);
import CDocEntry *DocPutLine(CDoc *doc,CDocEntry *doc_e);
import I64 DocSize(CDoc *doc);
import U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e);
import U0 Who(U8 *flags=NULL,CHashTable *h=NULL,
  U8 *outname=NULL,U8 mapfile_drv=0,U8 *index=NULL,CDoc *_doc=NULL);

extern U0 ExpressionBinDel(U8 *ex);
extern I64 LexExpression(CLex *lx);
extern Bool IsLexExpression2Bin(CLex *lx,U8 **_ex);
extern I64 PrsUnaryTerm(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,
    I64 *max_prec,I64 *unary_pre_prec,I64 *paren_prec);
extern I64 PrsUnaryModifier(CLex *lx,CPrsStk *ps,CMemberLst **_local_var,CArrayDim **_tempad,I64 *unary_post_prec);
extern U0 LexExcept(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL);
extern CExe *CmpJoin(CLex *lx,CHashFun *locals,Bool first_lex,Bool asm_blk=FALSE,U8 *map_name=NULL,U8 mapfile_drv=0);
extern U0 CmpFixUpJITAsm(CLex *lx,CExe *tempx,I64 ip2);
extern U0 CmpFixUpAOTAsm(CLex *lx,CExe *tempx,I64 ip2);
extern CHashClass *PrsClass(CLex *lx,I64 keyword,I64 cps_flags,Bool is_extern);
extern CHashClass *PrsType(CLex *lx,CHashClass **tempc1,
  I64 *_mode,CMemberLst *tempm,U8 **_ident,CHashFun **_fun_ptr,
  CHashExport **_tempex,CArrayDim *tempad,I64 cps_flags);
extern U0 PrsVarInit2(CLex *lx,U8 **_dst,CHashClass *tempc,CArrayDim *tempad,U8 *data_addr_ip,U8 **_base,Bool data_heap,I64 pass);
extern CHashFun *PrsFunJoin(CLex *lx,
	CHashClass *temp_return,U8 *name,I64 cps_flags);
extern Bool PrsStatement(CLex *lx,CHashFun *locals,I64 try_cnt=0,
  CLexCodeMisc *lb_break=NULL,Bool saved_prs_semicolon=TRUE);
extern CIntermediateCode *OptLag1(CIntermediateCode *tempi);
extern U0 OptSetNOP1(CIntermediateCode *tempi);
extern CHashClass *OptClassFwd(CHashClass *tempc);
extern CLexCodeMisc *OptLabelFwd(CLexCodeMisc *lb);
extern CIntermediateCode *OptPass012(CLex *lx,CHashFun *fun_class);
extern U0 OptPass3(CLex *lx,CHashFun *fun_class,COptReg *register_offsets);
extern U0 OptPass4(CLex *lx,COptReg *register_offsets,I64 *_type);
extern U0 OptPass5(CLex *lx);
extern U0 OptPass6(CLex *lx,CHashFun *fun_class);
extern I64 OptPass789(CLex *lx,CHashFun *fun_class,COptReg *register_offsets,U8 *buf,CDbgInfo **_dbg);
extern I64 PrsKeyWord(CLex *lx);
extern I64 HashEntrySize2(CHashSrcSym *temph);
extern U0 LexPutWarn(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL);
extern CHashSrcSym *HashEntryImportCopy(CHashSrcSym *temph);
extern U0 HashTableImport(CHashTable *src,CHashTable *dst);

_extern CMP_TEMPLATES U32 cmp_templates[1];
_extern CMP_TEMPLATES_DONT_POP U32 cmp_templates_dont_pop[1];
_extern CMP_TEMPLATES_DONT_PUSH U32 cmp_templates_dont_push[1];
_extern CMP_TEMPLATES_DONT_PUSH_POP U32 cmp_templates_dont_push_pop[1];
_extern CMP_TEMPLATES_DONT_PUSH2 U32 cmp_templates_dont_push2[1];


