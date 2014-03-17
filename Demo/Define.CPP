/*
See $LK,"Define",A="HI:Char/Define"$.
*/

//These might be in the Adam task.
DefineLoad("ST_YES_NO","(Yes or No)");
DefineLoad("ST_EXIT","Exit");
DefineLstLoad("ST_NO_YES_LST","No\0Yes\0");


//New strings might be loaded for each user
//to override.

"Spanish";
if (YorN) {
  DefineLoad("ST_YES_NO","(Si or No)");
  DefineLoad("ST_EXIT","Salida");
  DefineLstLoad("ST_NO_YES_LST","No\0Si\0");
}


Bool Quit()
{
  Bool result;
  U8 *st;
  I64 i;
  while (TRUE) {
    "%S %S?","ST_EXIT","ST_YES_NO";
    st=MGetS;
    i=DefineMatch(st,"ST_NO_YES_LST",LMF_IGNORE_CASE);
    if (i>=0) {
      result=i;
      break;
    }
    Free(st);
  }
  return result;
}

Quit;
