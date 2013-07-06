CDC *dc;

U0 DrawIt(CTask *,CDC *)
{
  dc->pen_width=2;
  dc->color=RED;
  GrLine3(dc,-100,-100,-100, -100, 100,-100);
  GrLine3(dc,-100, 100,-100,	100, 100,-100);
  GrLine3(dc, 100, 100,-100,	100,-100,-100);
  GrLine3(dc, 100,-100,-100, -100,-100,-100);
  GrLine3(dc,-100,-100, 100, -100, 100, 100);
  GrLine3(dc,-100, 100, 100,	100, 100, 100);
  GrLine3(dc, 100, 100, 100,	100,-100, 100);
  GrLine3(dc, 100,-100, 100, -100,-100, 100);
  GrLine3(dc,-100,-100, 100, -100,-100,-100);
  GrLine3(dc,-100, 100, 100, -100, 100,-100);
  GrLine3(dc, 100, 100, 100,	100, 100,-100);
  GrLine3(dc, 100,-100, 100,	100,-100,-100);
}

U0 Box()
{
  F64 é=0,è=0,ê=0,s=1,s1=1.05;
  dc=DCAlias(gr_dc2,Fs);

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  DocClear;
  Fs->draw_it=&DrawIt;
  Preempt(OFF); //We don't want matrix used while we're changing it.

  dc->x=200;
  dc->y=200;
  dc->flags|=DCF_TRANSFORMATION;
  while (!ScanChar) {
    GrIdentEqu(dc->r);
    GrRotZEqu(dc->r,é);
    GrRotXEqu(dc->r,è);
    GrRotZEqu(dc->r,ê);
    GrScaleMatEqu(dc->r,s);
    GrSetRotMat(dc,dc->r);

    Sleep(20);
    é+=2*ã/70;
    è+=2*ã/90;
    ê+=2*ã/110;
    s*=s1;
    if ( !(0.2<s<1.4) ) s1=1/s1;
  }
  SettingsPop;
  DCDel(dc);
}

Box;
