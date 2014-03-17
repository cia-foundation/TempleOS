I64 type_weights[OT_NUM_TYPES]={30,30,15,30,3,1,1,1};

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
  Obj *o_sort[O_NUM_OBJS],*tempo;

  GrIdentEqu(r);
  GrRotYEqu(r,-é);
  GrRotXEqu(r,77*ã/180);

  //Sun
  x=vx; y=0; z=1000000+vz;
  GrRot(r,&x,&y,&z);
  s=100.0/(AbsI64(z)+50);
  if (y<0) {
    dc->color=BROWN;
    GrCircle(dc,s*x+cx,15,15);
    dc->color=YELLOW;
    GrFloodFill(dc,s*x+cx,15);
  }

  for (i=0;i<O_NUM_OBJS;i++) {
    tempo=&objs[i];
    o_sort[i]=tempo;
    tempo->x1=tempo->x+vx; tempo->y1=tempo->y; tempo->z1=tempo->z+vz;
    GrRot(r,&tempo->x1,&tempo->y1,&tempo->z1);
    if (tempo->type==OT_SHEEP || tempo->type==OT_GOAT1 || tempo->type==OT_GOAT2) {
      dx=100*tempo->dx; dy=100*tempo->dy; dz=100*tempo->dz;
      GrRot(r,&dx,&dy,&dz);
      if (dx<0)
	tempo->sym=TRUE;
      else
	tempo->sym=FALSE;
    }
  }
  QSortI64(o_sort,O_NUM_OBJS,&ObjCompare);

  for (i=0;i<O_NUM_OBJS;i++) {
    tempo=o_sort[i];
    if (tempo->z1<0) break;
    s=100.0/(AbsI64(tempo->z1)+50);
    GrIdentEqu(dc->r);
    GrScaleEqu(dc->r,s*2);
    GrSetRotMat(dc,dc->r);
    dc->flags|=DCF_TRANSFORMATION;
    if (tempo->sym) {
      dc->flags|=DCF_SYMMETRY|DCF_JUST_MIRROR;
      GrSetSymmetry(dc,s*tempo->x1+cx,s*tempo->y1+h,s*tempo->x1+cx,s*tempo->y1+h+10);
    }
    Sprite3B(dc,s*tempo->x1+cx,s*tempo->y1+h,0,imgs[tempo->type]);
    dc->flags&=~(DCF_SYMMETRY|DCF_JUST_MIRROR);
    if (o_sort[i]==&objs[O_BURNING_BUSH]) { //draw burning bush
      dc->x=s*tempo->x1+cx;
      dc->y=s*tempo->y1+h;
      dc->z=0;

      dc->pen_width=1;
      dc->color=win_updates&15;
      for (j=0;j<45;j++) {
	m=Sqr(Rand); a=2*ã*Rand;
	P2R(&x1,&y1,m,a);
	m=Sqr(Rand); a=2*ã*Rand;
	P2R(&x2,&y2,m,a);
	GrLine3(dc,20*x1,20*y1-25,0,20*x2,20*y2-25,0);
      }

      if (SqrI64(s*tempo->x1)+SqrI64(s*tempo->y1)<300*300)
	horeb_done=TRUE;
    }
  }

  if (Blink) {
    dc->color=LTRED;
    GrPrint(dc,cx-21*FONT_WIDTH/2,cy-FONT_HEIGHT/2,"Find the Burning Bush.");
  }
}
