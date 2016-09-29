I64 type_weights[OT_NUM_TYPES]={0,30,30,15,30,3,1,1,1};

I64 ObjCompare(Obj *o1,Obj *o2)
{
  return o2->z1-o1->z1;
}

U0 DrawHoreb(CTask *task,CDC *dc)
{
  I64 i,j,x,y,z,dx,dy,dz,
	w=task->pix_width,
	h=task->pix_height,r[16],
	cx=w>>1,cy=h>>1;
  F64 s,m,a,x1,y1,x2,y2;
  Obj *o_sort[O_NUM_OBJS+O_NUM_PEBBLES],*tmpo;

  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,-é);
  Mat4x4RotX(r,77*ã/180);

  //Sun
  x=vx; y=0; z=1000000+vz;
  Mat4x4MulXYZ(r,&x,&y,&z);
  s=100.0/(AbsI64(z)+50);
  if (y<0) {
    dc->color=BROWN;
    GrCircle(dc,s*x+cx,15,15);
    dc->color=YELLOW;
    GrFloodFill(dc,s*x+cx,15);
  }

  for (i=0;i<O_NUM_OBJS+O_NUM_PEBBLES;i++) {
    tmpo=&objs[i];
    o_sort[i]=tmpo;
    tmpo->x1=tmpo->x+vx; tmpo->y1=tmpo->y; tmpo->z1=tmpo->z+vz;
    Mat4x4MulXYZ(r,&tmpo->x1,&tmpo->y1,&tmpo->z1);
    if (tmpo->type==OT_SHEEP ||
	  tmpo->type==OT_GOAT1 || tmpo->type==OT_GOAT2) {
      dx=100*tmpo->dx; dy=100*tmpo->dy; dz=100*tmpo->dz;
      Mat4x4MulXYZ(r,&dx,&dy,&dz);
      if (dx<0)
	tmpo->sym=TRUE;
      else
	tmpo->sym=FALSE;
    }
  }
  QSortI64(o_sort,O_NUM_OBJS+O_NUM_PEBBLES,&ObjCompare);

  for (i=0;i<O_NUM_OBJS+O_NUM_PEBBLES;i++) {
    tmpo=o_sort[i];
    if (tmpo->z1<0) break;
    s=100.0/(AbsI64(tmpo->z1)+50);
    if (tmpo->type) {
      Mat4x4IdentEqu(dc->r);
      Mat4x4Scale(dc->r,s*2);
      DCMat4x4Set(dc,dc->r);
      dc->flags|=DCF_TRANSFORMATION;
      if (tmpo->sym) {
	dc->flags|=DCF_SYMMETRY|DCF_JUST_MIRROR;
	DCSymmetrySet(dc,s*tmpo->x1+cx,s*tmpo->y1+h,
	      s*tmpo->x1+cx,s*tmpo->y1+h+10);
      }
      Sprite3B(dc,s*tmpo->x1+cx,s*tmpo->y1+h,0,imgs[tmpo->type]);
    } else {
      dc->flags&=~DCF_TRANSFORMATION;
      dc->color=pebble_colors[tmpo->num&3];
      GrPlot3(dc,s*tmpo->x1+cx,s*tmpo->y1+h,tmpo->z1);
    }
    dc->flags&=~(DCF_SYMMETRY|DCF_JUST_MIRROR);
    if (o_sort[i]==&objs[O_BURNING_BUSH]) {//draw burning bush
      dc->x=s*tmpo->x1+cx;
      dc->y=s*tmpo->y1+h;
      dc->z=0;

      dc->pen_width=1;
      dc->color=winmgr.updates&15;
      for (j=0;j<45;j++) {
	m=Sqr(Rand); a=2*ã*Rand;
	P2R(&x1,&y1,m,a);
	m=Sqr(Rand); a=2*ã*Rand;
	P2R(&x2,&y2,m,a);
	GrLine3(dc,20*x1,20*y1-25,0,20*x2,20*y2-25,0);
      }

      if (SqrI64(s*tmpo->x1)+SqrI64(s*tmpo->y1)<300*300)
	horeb_done=TRUE;
    }
  }

  if (Blink) {
    dc->color=LTRED;
    GrPrint(dc,cx-21*FONT_WIDTH/2,cy-FONT_HEIGHT/2,"Find the Burning Bush.");
  }
}
