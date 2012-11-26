#help_index "Compiler/Lex"
#help_file "::/SparrowOS/Doc/Lex"
public extern CLex *LexNew(U8 *buf,I64 flags,U8 *filename=NULL);
public extern U0 LexDel(CLex *lx);
public extern I64 LexSize(CLex *lx);
public extern U0 SetSrcFileInfo(CLex *lx,CHashSrcSym *h);
public extern U0 LexPutToken(CLex *lx);
public extern I64 LexGetChar(CLex *lx);
public extern U0 LexSkipEol(CLex *lx);
public extern U0 LexPush(CLex *lx);
public extern U0 LexPop(CLex *lx);
//public extern U0 LexSubStkPtr(CLex *lx);
public extern I64 Lex(CLex *lx);
public extern U8 *LexExtStr(CLex *lx,I64 *_size=NULL);
public extern U0 LexPutLine(CLex *lx,U8 *start);
public extern U0 LexPutPos(CLex *lx);
public extern U0 LexPutWarn(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL);
public extern U0 LexExcept(CLex *lx,U8 *name=NULL,CHashFun *fun_class=NULL);
public extern U8 *CmpExpression(CLex *lx,I64 *type=NULL);
public extern U8 *CmpStatement(CLex *lx,I64 *type);
public extern I64 GetExpression(CLex *lx,U8 **ex); //FALSE=no err
public extern I64 Expression(CLex *lx);
public extern U0 ExpressionDel(U8 *code);
public extern I64 MemberMetaData(U8 *st,CMemberList *ml);
extern U0 MemberListDel(CHashClass *c);
public extern CMemberListMeta *MemberMetaFind(U8 *st,CMemberList *ml);
public extern CMemberList *MemberFind(U8 *st,CHashClass *c);
public extern CMemberList *MemberFind2(CHashClass *pattern,CHashClass *tempc);
public extern U0 LexIncludeDoc(CLex *lx,U8 *filename,CDoc *_doc);

#help_index "Compiler/Directive"
#help_file "::/SparrowOS/Doc/Directives"
public extern U0 StreamPutS(U8 *st);
public extern U0 StreamPrintF(U8 *src,...);
public extern Bool PTrace(Bool val);
public extern Bool OTrace(Bool val);
public extern I64 OPassTraceMask(I64 i=0b1001111101);
public extern Bool OptEqu(I64 num,Bool val);
public extern Bool OptOn(I64 num);
public extern Bool OptOff(I64 num);
public extern Bool GetOpt(I64 num);
public extern Bool CTrace(Bool val);
public extern Bool LTrace(Bool val);
public extern Bool Echo(Bool val);
public extern Bool IsReplaceSyms();
public extern U0 ImportHashTable(CHashTable *tempht);
public extern I64 IncludeDepth();

#help_index "Hash"
extern CHashSrcSym *HashEntryImportCopy(CHashSrcSym *temph);
extern U0 HashTableImport(CHashTable *src,CHashTable *dst);
public extern I64 HashEntrySize(CHashSrcSym *temph);
public extern I64 HashEntrySize2(CHashSrcSym *temph);
public extern I64 HashTableSize2(CHashTable *table);


#help_index "Compiler;Cmd Line (Typically)"
public extern I64 Cmp(U8 *filename,U8 *map_name=NULL,U8 *out_name=NULL,U8 mapfile_drv=0);

#help_index "Compiler"
public extern I64 ExePutS(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE);
public extern I64 ExePrintF(U8 *fmt,...);
public extern I64 ExeFile(U8 *name,Bool just_load=FALSE);
public extern I64 ExePutS2(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE);
public extern I64 ExePrintF2(U8 *fmt,...);
public extern I64 ExeFile2(U8 *name,Bool just_load=FALSE);
public extern I64 ExePutS3(U8 *buf,U8 *filename=NULL,Bool just_load=FALSE);
public extern I64 ExePrintF3(U8 *fmt,...);
public extern I64 ExeFile3(U8 *name,Bool just_load=FALSE);
extern CHashTable *asm_hash;
extern U0 CmpInit(Bool first);
extern I64 compiled_lines,compiled_funs,compiled_bytes;



//The following is for benchmarking
extern Bool cmp_do_not_write_to_dsk;

#help_index "Debugging"
public extern U0 Ui(U8 *buf,U8 **_ip,I64 seg_size=64,I64 *branch=NULL);
public extern U8 *U(U8 *ip,I64 cnt=20,I64 seg_size=64);
public extern I64 Un(U8 *ip,I64 cnt=0x80,I64 seg_size=64);

#help_index ""
