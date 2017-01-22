//x must be global.
//Might as well make y global.

F64 x,y,y_last;

U0 Cartesian()
{
  U8 *st;
  I64 ch=0,h,v;
  Bool first;
  CDC *dc=DCAlias;
  do {
    DocClear;
    "Enter algebraic equation to graph.\n"
	  "Example: y=2*x\n"
	  "y=";
    if (st=GetStr) {
      if (*st) {
	h=Fs->pix_width/2;
	v=Fs->pix_height/2;
	dc->color=RED;
	GrLine(dc,h,0,h,2*v+1);
	GrLine(dc,0,v,2*h+1,v);
	first=TRUE;
	dc->color=BLACK;
	for (x=-h;x<=h;x++) {
	  y=-ExePrint("ToF64(%s);",st)(F64);
	  if (!first)
	    GrLine(dc,x-1+h,y_last+v,x+h,y+v);
	  y_last=y;
	  first=FALSE;
	}
	ch=GetChar;
      } else
	ch=CH_SHIFT_ESC;
      Free(st);
    } else
      ch=CH_SHIFT_ESC;
    DCFill;
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
  DCDel(dc);
}

Cartesian;

//For better performance, compile the expression entered by the
//user one time with $LK,"LexExpression2Bin",A="MN:LexExpression2Bin"$() and use $LK,"Call",A="MN:Call"$().
//See $LK,"::/Demo/CompileDemo.HC"$.
