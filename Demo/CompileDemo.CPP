F64 glbl;

F64 CompileDemo(U8 *st)
{
  I64 type;
  U8 *machine_code;
  CCmpCtrl *cc=CmpCtrlNew(st,CCF_DONT_FREE_BUF);
  F64 res=0;
  Lex(cc);  //Gotta get it started
  "Compile \"%s\"\n",st;
  do {
    if (machine_code=LexExpression2Bin(cc,&type)) {
      if (type!=RT_F64)
	res=ToF64(Call(machine_code));
      else
	res=Call(machine_code)(F64);
      Free(machine_code);
    }
    "res=%9.4f\n",res;
    if (cc->token==';')
      Lex(cc);
  } while (cc->token!=TK_EOF);	//end of file?
  CmpCtrlDel(cc);
  return res;
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
