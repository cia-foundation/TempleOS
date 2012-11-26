U0 Main()
{
  I64 x1=0,y1=0,x2=0,y2=0,i=1,c=0;
  I64 vx1=0,vy1=0,vx2=0,vy2=0;
  CDC *dc=DCAlias;
  while (!ScanChar) {
    if (!--i) {
      i=1000;
      dc->color=c++&15;
    }
    GrLine(dc,x1,y1,x2,y2);
    vx1=LimitI64(vx1+SignI64(RandI16),-5,5);
    vy1=LimitI64(vy1+SignI64(RandI16),-5,5);
    vx2=LimitI64(vx2+SignI64(RandI16),-5,5);
    vy2=LimitI64(vy2+SignI64(RandI16),-5,5);
    x1=LimitI64(x1+vx1,0,Fs->win_pixel_width-1);
    y1=LimitI64(y1+vy1,0,Fs->win_pixel_height-1);
    x2=LimitI64(x2+vx2,0,Fs->win_pixel_width-1);
    y2=LimitI64(y2+vy2,0,Fs->win_pixel_height-1);
    Sleep(1); //Take this out and see what happens!
  }
  DCFill(dc);
  DCDel(dc);
}

Main;
