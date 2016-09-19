U0 MyRead(U8 *filename)
{
  CCmpCtrl *cc=CmpCtrlNew(MStrPrint("#include \"%s\"",filename));
  while (Lex(cc)) {
    "%03d:",cc->token;
    switch (cc->token) {
	case TK_IDENT:		"IDENT:%s",cc->cur_str;	break;
	case TK_STR:		"STR:%s",cc->cur_str;	break;
	case TK_CHAR_CONST:	"CHAR:%d",cc->cur_i64;	break;
	case TK_I64:		"I64:%d",cc->cur_i64;	break;
	case TK_F64:		"F64:%d",cc->cur_f64;	break;
    }
    '\n';
  }
  CmpCtrlDel(cc);
}
