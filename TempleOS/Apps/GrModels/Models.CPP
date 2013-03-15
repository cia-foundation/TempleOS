#define T_MAN	0
#define T_BALL	1

U0 Models()
{
  I64 i;
  Bool old_form=LBts(&(DocCur)->flags,DOCf_FORM);
  DocClear;
  "Make a CSprite model.  Then, cut-and-Paste it.\n\n";
  "$$LM,4$$";
  "\n\n$$BT,\"Man\",T_MAN$$\n\n";
  "\n\n$$BT,\"Ball\",T_BALL$$\n\n";
  "$$LM,0$$";
  i=DocMenuDo(DocCur);
  LBEqu(&(DocCur)->flags,DOCf_FORM,old_form);
  DocBottom;
  switch (i) {
    case T_MAN:
      ManGen;
      break;
    case T_BALL:
      BallGen;
      break;
  }
}
