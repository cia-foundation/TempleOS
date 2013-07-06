Bool OptEqu(I64 num,Bool val) {
  return BEqu(&Fs->last_lex->opts,num,val);
}

//Turn on compiler option
Bool OptOn(I64 num) {
  return OptEqu(num,ON);
}

//Turn off compiler option
Bool OptOff(I64 num) {
  return OptEqu(num,OFF);
}

//Get state of compiler option
Bool GetOpt(I64 num) {
  return Bt(&Fs->last_lex->opts,num);
}

//Displays intermediate code as it is being added
Bool PTrace(Bool val) {
  return OptEqu(OPTf_PARSE_TRACE,val);
}

//Displays intermediate code after passes through optimization
Bool OTrace(Bool val) {
  return OptEqu(OPTf_OPT_TRACE,val);
}

//Ctrls which optimizer passes are displayed
I64 OPassTraceMask(I64 i=0b1001111101) {
  I64 old=Fs->last_lex->pass_trace_mask;
  Fs->last_lex->pass_trace_mask=i;
  return old;
}

//Displays assembly code output
Bool CTrace(Bool val) {
  return OptEqu(OPTf_COMPILE_TRACE,val);
}

//Displays lexical analyser's output
Bool LTrace(Bool val) {
  return OptEqu(OPTf_LEX_TRACE,val);
}

//Displays text as it is being compiled
Bool Echo(Bool val) {
  return OptEqu(OPTf_ECHO,val);
}

Bool IsReplaceSyms() {
  return GetOpt(OPTf_REPLACE_SYMS);
}

U0 ImportHashTable(CHashTable *src)
{ //called from #exe {} in AOT modules
  // See $LK-A,"::/TempleOS/Kernel/AOTAdam.HPP"$.
  CLex *lx=Fs->last_lex;
  if (lx==&Fs->next_lex) {
    PutErr;
    "Not Compiling\n";
  } else {
    if (!(lx->flags&LF_EXE_BLK))
      LexExcept(lx,"ST_IMPORT_HASH_TABLE_NOT_ALLOWED");
    while (src) {
      if (lx->htc.next && lx->htc.next->glbl_hash_table)
	HashTableImport(src,lx->htc.next->glbl_hash_table);
      else if (lx->htc.glbl_hash_table)
	HashTableImport(src,lx->htc.glbl_hash_table);
      src=src->next;
    }
  }
}

I64 IncludeDepth()
{
  CLex *lx=Fs->last_lex;
  return lx->cur_lfn-&lx->lfns;
}
