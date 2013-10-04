CTrace(ON);
U0 EdCursorLeft(I64 sc=MIN_I64)
{
  CDocEntry *doc_ce;
  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
}
