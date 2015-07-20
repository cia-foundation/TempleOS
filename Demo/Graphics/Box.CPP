CDC *dc;

U0 DrawIt(CTask *,CDC *)
{//This technique is weird, but clever.  In Box() we
//created an alias of the system $LK,"gr.dc2",A="MN:CGrGlbls"$.
  dc->pen_width=2;
  dc->color=RED;
  GrLine3(dc,-100,-100,-100, -100, 100,-100);
  GrLine3(dc,-100, 100,-100,  100, 100,-100);
  GrLine3(dc, 100, 100,-100,  100,-100,-100);
  GrLine3(dc, 100,-100,-100, -100,-100,-100);
  GrLine3(dc,-100,-100, 100, -100, 100, 100);
  GrLine3(dc,-100, 100, 100,  100, 100, 100);
  GrLine3(dc, 100, 100, 100,  100,-100, 100);
  GrLine3(dc, 100,-100, 100, -100,-100, 100);
  GrLine3(dc,-100,-100, 100, -100,-100,-100);
  GrLine3(dc,-100, 100, 100, -100, 100,-100);
  GrLine3(dc, 100, 100, 100,  100, 100,-100);
  GrLine3(dc, 100,-100, 100,  100,-100,-100);
}

U0 Box()
{
  F64 é=0,è=0,ê=0,s=1,s1=1.05;
  dc=DCAlias(gr.dc2,Fs);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  DocClear;
  Fs->draw_it=&DrawIt;
  Preempt; //We don't want matrix used while we're changing it.

  dc->x=200;
  dc->y=200;
  dc->flags|=DCF_TRANSFORMATION;
  while (!ScanChar) {
    Mat4x4IdentEqu(dc->r);
    Mat4x4RotZ(dc->r,é);
    Mat4x4RotX(dc->r,è);
    Mat4x4RotZ(dc->r,ê);
    Mat4x4Scale(dc->r,s);
    DCMat4x4Set(dc,dc->r);

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
