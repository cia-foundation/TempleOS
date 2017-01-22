#define T_MAN	0
#define T_BALL	1

U0 Models()
{
  I64 i;
  Bool old_form=LBts(&(DocPut)->flags,DOCf_FORM);
  DocClear;
  "Make a CSprite model.  Then, cut-and-Paste it.\n\n";
  "$$LM,4$$";
  "\n\n$$BT,\"Man\",LE=T_MAN$$\n\n";
  "\n\n$$BT,\"Ball\",LE=T_BALL$$\n\n";
  "$$LM,0$$";
  i=DocMenu(DocPut);
  LBEqu(&(DocPut)->flags,DOCf_FORM,old_form);
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
