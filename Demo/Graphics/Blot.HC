CDC *dc=DCAlias;
CDC *img1=DCNew(64,64);
CDC *img2=DCNew(512,512);

img1->color=COLOR_MONO;
img1->thick=3;
GrCircle(img1,30,30,20);
GrFloodFill3(img1,30,30,0,TRUE);
GrLine3(img1,60,0,0,0,60,0);

DCFill(img2);
img2->color=RED;
img2->brush=img1;
GrCircle(img2,256-30,256-30,128,67);
img2->color=BLUE;
img2->thick=6;
img2->brush=NULL;
GrLine3(img2,128,128,0,256+128,256+128,0);
GrLine3(img2,256+128,128,0,128,256+128,0);

img2->color=YELLOW;
GrRect(img2,256-20,256-20,40,40);

I64 i;
for (i=0;i<256;i+=4) {
  Refresh;
  DCFill;
  GrBlot(dc,i-256,i-256,img2);
  Sleep(10);
}
U8 *tmpg=DC2Sprite(img1);
Sprite(tmpg);
Free(tmpg);
"%h9c",'\n';

DCDel(img1);
DCDel(img2);
DCDel(dc);

PressAKey;
DCFill;
