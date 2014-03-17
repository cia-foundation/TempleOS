I64 x=0,y=0;

U0 DrawMap(CTask *,CDC *dc)
{
  static I64 last_x;
  static Bool last_left;
  if (x<last_x)
    last_left=TRUE;
  else if (x>last_x)
    last_left=FALSE;
  dc->pen_width=1;
  if (last_left)
    Sprite3(dc,x,y,0,left_imgs [ToI64(6.0*tS)&3]);
  else
    Sprite3(dc,x,y,0,right_imgs[ToI64(6.0*tS)&3]);
  last_x=x;
}

#define A1_MIN	0.02
#define A1_MAX	0.05
#define A2_MIN	0.15
#define A2_MAX	0.30

U0 AEMap()
{
  I64 i=0,
      cx=Fs->pix_width>>1,
      cy=Fs->pix_height>>1,
      c_min=MinI64(cx,cy),
      *r=NULL,
      x1=0,y1=0,z1=0,
      x_last=x1,y_last=y1;
  F64 a,a1=(A1_MAX+A1_MIN)/2,a2=(A2_MAX+A2_MIN)/2,a2_total=a2,
      x_scale=ToF64(cx)/c_min,y_scale=ToF64(cy)/c_min;
  CDC *dc=DCAlias;

  SettingsPush(Fs,TSF_SAME_SONG); //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=YELLOW<<4+BLUE;
  DocClear;
  "$$BG,YELLOW$$"; NewLine(TEXT_ROWS);
  BibleVerse(,"Exodus,16:35",3);
  Fs->draw_it=&DrawMap;
  try {
    while (!ScanChar) {
      dc->color=BLACK;
      a1=Clamp(Wrap(a1+(A1_MAX+A1_MIN)/5.0*(Rand-0.5)),-A1_MAX,A1_MAX);
      a=a1;
      if ( 0.0<=a<=A1_MIN) a=A1_MIN;
      if (-A1_MIN<=a<=0.0) a=-A1_MIN;
      Free(r);
      r=GrRotZNew(a);
      GrRot(r,&x1,&y1,&z1);

      a2=Clamp(Wrap(a2+(A2_MAX+A2_MIN)/5.0*(Rand-0.5)),-A2_MAX,A2_MAX);
      a=a2;
      if ( 0.0<=a<=A2_MIN) a=A2_MIN;
      if (-A2_MIN<=a<=0.0) a=-A2_MIN;
      a2_total+=a;

      x1=ClampI64(x1+6.0*Cos(a2_total),-c_min+10,c_min-10);
      y1=ClampI64(y1+6.0*Sin(a2_total),-c_min+10,c_min-10);

      x=x_scale*x1+cx; y=y_scale*y1+cy;
      if (i++&1)
	GrLine(dc,x_last,y_last,x,y);
      x_last=x; y_last=y;
      Sleep(15);
    }
  } catch
    Fs->catch_except=TRUE;
  DocClear;
  SettingsPop(Fs,TSF_SAME_SONG);
  Free(r);
  DCFill(dc);
  DCDel(dc);
}
