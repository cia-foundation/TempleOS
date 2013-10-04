F64 glbl;

F64 CompileDemo(U8 *st)
{
  I64 type;
  U8 *ex;
  CLex *lx=LexNew(st,LF_DONT_FREE_BUF);
  F64 result=0;
  Lex(lx);  //Gotta get it started
  "Compile \"%s\"\n",st;
  do {
    if (ex=LexExpression2Bin(lx,&type)) {
      if (type!=IT_F64)
	result=ToF64(Call(ex));
      else
	result=Call(ex)(F64);
      ExpressionBinDel(ex);
    }
    "result=%9.4f\n",result;
    if (lx->token==';')
      Lex(lx);
  } while (lx->token!=TK_EOF);	//end of file?
  LexDel(lx);
  return result;
}

CompileDemo("2+Cos(ã)");
CompileDemo("1;2+4");
CompileDemo("glbl=1;glbl*2;glbl/3;Sin(glbl)");

//This is just like you typed-it on
//the cmd line.
ExePrint(
"I64 i;"
"for (i=0;i<10;i++) "
"  \"%%d\n\",i;"
);
