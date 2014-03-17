









		$SP,"",BI=1,BP="::/Misc/StockSprites.TXT,TempleOSLogo"$











U0 Logo()
{
  CDC *dc=DCAlias;
  dc->flags|=DCF_TRANSFORMATION;
  DocClear;
  while (!ScanChar) {
    dc->x=Fs->pix_width>>1;
    dc->y=Fs->pix_height>>1;

//$LK,"Wrap",A="MN:Wrap"$() converts an angle to an
//equivalent angle in a range starting
//from [arg2,arg2+ã*2).
    GrIdentEqu(dc->r);
    GrRotYEqu(dc->r,Wrap(tS,-ã/2));
    GrRotXEqu(dc->r,30.0*ã/180);
    GrSetRotMat(dc,dc->r);

    Sprite3(dc,0,0,0,$IB,"<1>",BI=1$);
//See also $LK,"SpriteX3B",A="MN:SpriteX3B"$(),
//$LK,"SpriteY3B",A="MN:SpriteY3B"$(),$LK,"SpriteZ3B",A="MN:SpriteZ3B"$().

    WinMgrSync;
    DCFill(dc);
  }
  DCDel(dc);
}

Logo;
