//"ls" is light source.

U0 Lighting(CDC *dc,CD3I32 *ls,CD3I32 *poly,I64 color)
{//color is a color from 0-7
  CD3I32 v1,v2;
  I64 *r=dc->r,i,vn_x,vn_y,vn_z;
  F64 d;

  v1.x=poly[0].x-poly[1].x;
  v1.y=poly[0].y-poly[1].y;
  v1.z=poly[0].z-poly[1].z;

  v2.x=poly[2].x-poly[1].x;
  v2.y=poly[2].y-poly[1].y;
  v2.z=poly[2].z-poly[1].z;

  //V1 and V2 are vects along two sides
  //of the polygon joined at point[1].

  vn_x=v1.y*v2.z-v1.z*v2.y;
  vn_y=v1.z*v2.x-v1.x*v2.z;
  vn_z=v1.x*v2.y-v1.y*v2.x;

  if (d=Sqrt(SqrI64(vn_x)+SqrI64(vn_y)+SqrI64(vn_z)))
    d=1<<16/d;
  vn_x*=d;
  vn_y*=d;
  vn_z*=d;
//Vn is the cross product of V1 and V3
  //which means it is perpendicular.  It
  //is the normal vect to the surface.
  //It has been scaled to length 65536.
  Mat4x4MulXYZ(r,&vn_x,&vn_y,&vn_z);
  i=(vn_x*ls->x+vn_y*ls->y+vn_z*ls->z)>>16;
//The dot product of the light source
  //vect and the surface normal
  //gives an illumination number.

  //TempleOS will generate a random U16
  //and compare to dither_probability_u16 and
  //will pick from two colors.
  //Probability dithering does not work with thick>1 at this time.
  if (i<0) {
    dc->color=ROPF_PROBABILITY_DITHER+BLACK<<16+color;
    dc->dither_probability_u16=-i;
  } else {
    dc->color=ROPF_PROBABILITY_DITHER+(color^8)<<16+color;
    dc->dither_probability_u16=i;
  }
}

#define RINGS	8
#define FACES	32
#define SLOP	0.03 //Gaps appear without this.

U0 DrawIt(CTask *task,CDC *dc)
{
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  F64 tt=0.5*(Sin(ã*2*(tS%10.0)/10.0)+2.0),
	é,é2,è,è2,radius,d;
  I64 i,j,cx=task->pix_width/2,cy=task->pix_height/2;
  CD3I32 poly[3],ls;

  dc->flags|=DCF_TRANSFORMATION;
  DCDepthBufAlloc(dc);

  Mat4x4IdentEqu(dc->r);
  Mat4x4RotZ(dc->r,s->az);
  Mat4x4RotY(dc->r,s->ay);
  Mat4x4RotX(dc->r,s->ax+ã);
  Mat4x4Scale(dc->r,tt);
  DCMat4x4Set(dc,dc->r);

  ls.x=-(ms.pos.x-task->pix_left-task->scroll_x-cx);
  ls.y=-(ms.pos.y-task->pix_top-task->scroll_y-cy);
  ls.z=GR_WIDTH/8;
  d=1<<16/D3I32Norm(&ls);
  ls.x*=d;
  ls.y*=d;
  ls.z*=d;

  dc->x=cx;
  dc->y=cy;
  dc->z=MaxI64(cx,cy);
  radius =MinI64(cx,cy)/2;

  for (i=0;i<RINGS;i++) {
    è =    i*ã/2/RINGS;
    è2=(i+1)*ã/2/RINGS+SLOP;
    for (j=0;j<FACES;j++) {
      é =j*2*ã/FACES;
      é2=(j+1)*2*ã/FACES+SLOP;

      //Upper half
      poly[0].x=radius*Cos(è)*Cos(é);
      poly[0].y=radius*Cos(è)*Sin(é);
      poly[0].z=radius*Sin(è);
      poly[1].x=radius*Cos(è)*Cos(é2);
      poly[1].y=radius*Cos(è)*Sin(é2);
      poly[1].z=radius*Sin(è);
      poly[2].x=radius*Cos(è2)*Cos(é+2*ã/FACES/2);
      poly[2].y=radius*Cos(è2)*Sin(é+2*ã/FACES/2);
      poly[2].z=radius*Sin(è2);
      Lighting(dc,&ls,poly,BLUE);
      GrFillPoly3(dc,3,poly);

      poly[2].x=radius*Cos(è2)*Cos(é +2*ã/FACES/2);
      poly[2].y=radius*Cos(è2)*Sin(é +2*ã/FACES/2);
      poly[2].z=radius*Sin(è2);
      poly[1].x=radius*Cos(è2)*Cos(é2+2*ã/FACES/2);
      poly[1].y=radius*Cos(è2)*Sin(é2+2*ã/FACES/2);
      poly[1].z=radius*Sin(è2);
      poly[0].x=radius*Cos(è)*Cos(é2);
      poly[0].y=radius*Cos(è)*Sin(é2);
      poly[0].z=radius*Sin(è);
      Lighting(dc,&ls,poly,BLUE);
      GrFillPoly3(dc,3,poly);

      //Lower half
      poly[2].x=radius*Cos(è)*Cos(é);
      poly[2].y=radius*Cos(è)*Sin(é);
      poly[2].z=-radius*Sin(è);
      poly[1].x=radius*Cos(è)*Cos(é2);
      poly[1].y=radius*Cos(è)*Sin(é2);
      poly[1].z=-radius*Sin(è);
      poly[0].x=radius*Cos(è2)*Cos(é+2*ã/FACES/2);
      poly[0].y=radius*Cos(è2)*Sin(é+2*ã/FACES/2);
      poly[0].z=-radius*Sin(è2);
      Lighting(dc,&ls,poly,RED);
      GrFillPoly3(dc,3,poly);

      poly[0].x=radius*Cos(è2)*Cos(é +2*ã/FACES/2);
      poly[0].y=radius*Cos(è2)*Sin(é +2*ã/FACES/2);
      poly[0].z=-radius*Sin(è2);
      poly[1].x=radius*Cos(è2)*Cos(é2+2*ã/FACES/2);
      poly[1].y=radius*Cos(è2)*Sin(é2+2*ã/FACES/2);
      poly[1].z=-radius*Sin(è2);
      poly[2].x=radius*Cos(è)*Cos(é2);
      poly[2].y=radius*Cos(è)*Sin(é2);
      poly[2].z=-radius*Sin(è);
      Lighting(dc,&ls,poly,RED);
      GrFillPoly3(dc,3,poly);
    }
  }
}

//See $LK,"::/Demo/Graphics/SpritePlot3D.HC"$.
//for a CSprite example.

//See $LK,"SpriteMeshEd",A="MN:SpriteMeshEd"$() for a fancy example.

U0 Main()
{
  CCtrl *c=ViewAnglesNew;
  CViewAngles *s=c->state;
  s->sx=2*VIEWANGLES_SNAP;
  s->sy=7*VIEWANGLES_SNAP;
  s->sz=6*VIEWANGLES_SNAP;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    "\n\nMove mouse to change light source.\n\n";
    PressAKey;
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
  ViewAnglesDel;
}

Main;
