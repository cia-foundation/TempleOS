F64 MySin(F64 d)
{ //Sin() does not have an address.
  return Sin(d);
}

F64 MyCos(F64 d)
{ //Cos() does not have an address.
  return Cos(d);
}

U0 FunDraw1(CDC *dc,F64 (*f)(F64 t))
{
  I64 i,w=Fs->pix_width,h=Fs->pix_height;
  F64 scale=h/5.0;
  for (i=-w/2;i<=w/2;i++)
    GrLine(dc,w/2+i,h/2-scale*f(i/scale*2*ã),
	  w/2+i+1,h/2-scale*f((i+1)/scale*2*ã));
}

U0 FunDraw2(CDC *dc,F64 (*f)(F64 t,F64 p),F64 T)
{
  I64 i,w=Fs->pix_width,h=Fs->pix_height;
  F64 scale=h/5.0;
  for (i=-w/2;i<=w/2;i++)
    GrLine(dc,w/2+i,h/2-scale*f(i/scale,T),w/2+i+1,h/2-scale*f((i+1)/scale,T));
}

U0 Init(CDC *dc)
{
  I64 i,w=Fs->pix_width,h=Fs->pix_height;
  F64 scale=h/5.0;
  DocClear;
  DCFill(dc);
  dc->color=BLACK;
  GrLine(dc,0,h/2,w-1,h/2);
  GrLine(dc,w/2,0,w/2,h-1);
  for (i=-2;i<=2;i++) {
    dc->color=BLACK;
    GrLine(dc,w/2-3,h/2-scale*i,w/2+3,h/2-scale*i);
    if (i) {
      dc->color=DKGRAY;
      GrPrint(dc,w/2+5,h/2-scale*i-FONT_HEIGHT/2,"%5.1f",ToF64(i));
    }
  }
  for (i=-4;i<=4;i++) {
    dc->color=BLACK;
    GrLine(dc,w/2+scale*i,h/2-3,w/2+scale*i,h/2+3);
    if (i) {
      dc->color=DKGRAY;
      GrPrint(dc,
	    w/2+scale*i-7*FONT_WIDTH/2,h/2+5+FONT_HEIGHT,"%5.1f*T",ToF64(i));
    }
  }
}

U0 SndMathDemo()
{
  CDC *dc=DCAlias;

  Init(dc);
  dc->color=RED;
  FunDraw1(dc,&MySin);
  "$$RED$$Sin$$FG$$\n";
  PressAKey;
  dc->color=BLUE;
  FunDraw2(dc,&Saw,1.0);
  "$$BLUE$$Saw$$FG$$\n";
  PressAKey;
  dc->color=GREEN;
  FunDraw2(dc,&FullSaw,1.0);
  "$$GREEN$$FullSaw$$FG$$\n";
  PressAKey;

  Init(dc);
  dc->color=RED;
  FunDraw1(dc,&MyCos);
  "$$RED$$Cos$$FG$$\n";
  PressAKey;
  dc->color=BLUE;
  FunDraw2(dc,&Caw,1.0);
  "$$BLUE$$Caw$$FG$$\n";
  PressAKey;
  dc->color=GREEN;
  FunDraw2(dc,&FullCaw,1.0);
  "$$GREEN$$FullCaw$$FG$$\n";
  PressAKey;

  Init(dc);
  dc->color=RED;
  FunDraw1(dc,&MySin);
  "$$RED$$Sin$$FG$$\n";
  PressAKey;
  dc->color=BLUE;
  FunDraw2(dc,&Tri,1.0);
  "$$BLUE$$Tri$$FG$$\n";
  PressAKey;
  dc->color=GREEN;
  FunDraw2(dc,&FullTri,1.0);
  "$$GREEN$$FullTri$$FG$$\n";
  PressAKey;

  DCFill(dc);
  DCDel(dc);
}

SndMathDemo;
