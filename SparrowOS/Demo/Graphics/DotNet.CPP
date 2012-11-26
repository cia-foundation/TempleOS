U0 DotNet()
{
  I64 i;
  CDC *dc=DCAlias;
  DocClear;  //Clear text
  dc->color=RED;
  for (i=0;i<480;i+=20)
    GrLine(dc,i,0,0,480-i,3);
  DCDel(dc);
  PressAKey;
  DCFill;
}

DotNet;  //Run when #included
