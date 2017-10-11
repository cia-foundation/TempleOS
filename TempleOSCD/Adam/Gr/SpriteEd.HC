#help_index "Graphics/Sprite;Sprites"

CSprite *SpriteSetSettings(CDC *dc=NULL,CSprite *head,I64 elem_num,
	I64 x=0,I64 y=0,CColorROPU32 *_color=NULL,I64 *_thick=NULL,
	I64 *_xx=NULL,I64 *_yy=NULL)
{
  CSprite *res=head->next;
  I64 thick=1,xx=0,yy=0;
  CColorROPU32 color=BLACK;
  if (dc) DCRst(dc);
  while (elem_num-->0 && res!=head) {
    switch (res->type&SPG_TYPE_MASK) {
      case SPT_COLOR:
	color=res->c.color;
	if (dc) dc->color=color;
	break;
      case SPT_DITHER_COLOR:
	color=res->d.dither_color.u8[0]|
	      res->d.dither_color.u8[1]<<COLORROP_BITS|ROPF_DITHER;
	if (dc) dc->color=color;
	break;
      case SPT_THICK:
	thick=res->t.thick;
	if (dc) dc->thick=thick;
	break;
      case SPT_SHIFT:
	xx+=res->p.x1;
	yy+=res->p.y1;
	x+=res->p.x1;
	y+=res->p.y1;
	break;
      case SPT_PLANAR_SYMMETRY:
	if (dc) {
	  if (DCSymmetry3Set(dc,res->pp.x1+x,res->pp.y1+y,0,
		res->pp.x2+x,res->pp.y2+y,0,
		res->pp.x2+x,res->pp.y2+y,1))
	    dc->flags|=DCF_SYMMETRY;
	  else
	    dc->flags&=~DCF_SYMMETRY;
	}
	break;
    }
    res=res->next;
  }
  if (_color) *_color=color;
  if (_thick) *_thick=thick;
  if (_xx) *_xx=xx;
  if (_yy) *_yy=yy;
  return res;
}

Bool SpritePolyPtPlot(CSprite *head,I64 x,I64 y,I64)
{
  CSprite *tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_PT));
  tmpg->type=SPT_PT;
  tmpg->p.x1=x;
  tmpg->p.y1=y;
  QueIns(tmpg,head->last);
  return TRUE;
}

CSprite *Sprite2SpriteQue(U8 *elems)
{
  I64 s;
  CSprite *res=CAlloc(sizeof(CSprite)),
	*tmpg=elems-offset(CSprite.start),*tmpg1;
  QueInit(res);
  while (tmpg->type&SPG_TYPE_MASK) {
    tmpg1=MAlloc(SpriteElemSize(tmpg)+offset(CSprite.start));
    s=SpriteElemSize(tmpg);
    MemCpy(&tmpg1->start,&tmpg->start,s);
    QueIns(tmpg1,res->last);
    tmpg(U8 *)+=s;
  }
  return res;
}

U8 *SpriteQue2Sprite(CSprite *head,I64 *_size=NULL)
{
  I64 i,size=sprite_elem_base_sizes[SPT_END];
  CSprite *tmpg=head->next;
  U8 *res,*dst;
  while (tmpg!=head) {
    size+=SpriteElemSize(tmpg);
    tmpg=tmpg->next;
  }
  if (_size) *_size=size;
  res=dst=MAlloc(size);
  tmpg=head->next;
  while (tmpg!=head) {
    i=SpriteElemSize(tmpg);
    MemCpy(dst,&tmpg->start,i);
    dst+=i;
    tmpg=tmpg->next;
  }
  *dst=SPT_END;
  return res;
}

U0 SpriteEdUpdate(CDoc *doc,CDocEntry *doc_ce,CSprite *head)
{
  CDocBin *tmpb=doc_ce->bin_data;
  I64 size;
  Bool unlock=DocLock(doc);
  Free(tmpb->data);
  tmpb->data=SpriteQue2Sprite(head,&size);
  tmpb->size=size;
  if (unlock)
    DocUnlock(doc);
}

U0 SpriteSetOrigin(CSprite *head,I64 dx,I64 dy,I64 dz)
{
  I64 i;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tmpg=head->next;
  while (tmpg!=head) {
    if (Bt(&tmpg->type,SPf_SEL))
      switch (tmpg->type&SPG_TYPE_MASK) {
	case SPT_ARROW:
	case SPT_LINE:
	case SPT_PLANAR_SYMMETRY:
	case SPT_RECT:
	case SPT_ROTATED_RECT:
	  tmpg->pp.x2+=dx;
	  tmpg->pp.y2+=dy;
	case SPT_PT:
	case SPT_FLOOD_FILL:
	case SPT_FLOOD_FILL_NOT:
	case SPT_TEXT:
	case SPT_TEXT_BOX:
	case SPT_TEXT_DIAMOND:
	case SPT_CIRCLE:
	case SPT_BITMAP:
	case SPT_ELLIPSE:
	case SPT_POLYGON:
	  tmpg->p.x1+=dx;
	  tmpg->p.y1+=dy;
	  break;
	case SPT_POLYLINE:
	  ptr=&tmpg->nu.u;
	  for (i=0;i<tmpg->nu.num;i++) {
	    ptr[i<<1]+=dx;
	    ptr[i<<1+1]+=dy;
	  }
	  break;
	case SPT_POLYPT:
	  tmpg->npu.x+=dx;
	  tmpg->npu.y+=dy;
	  break;
	case SPT_BSPLINE2:
	case SPT_BSPLINE3:
	case SPT_BSPLINE2_CLOSED:
	case SPT_BSPLINE3_CLOSED:
	  p=&tmpg->nu.u;
	  for (i=0;i<tmpg->nu.num;i++,p++) {
	    p->x+=dx;
	    p->y+=dy;
	    p->z+=dz;
	  }
	  break;
	case SPT_MESH:
	  p=&tmpg->mu.u;
	  for (i=0;i<tmpg->mu.vertex_cnt;i++,p++) {
	    p->x+=dx;
	    p->y+=dy;
	    p->z+=dz;
	  }
	  break;
	case SPT_SHIFTABLE_MESH:
	  tmpg->pmu.x+=dx;
	  tmpg->pmu.y+=dy;
	  tmpg->pmu.z+=dz;
	  break;
      }
    tmpg=tmpg->next;
  }
}

CSprite *SpriteTransformCircle(I64 *r,CSprite *tmpg)
{
  I64 x,y,z;
  F64 m1,arg1,m2,radius=tmpg->pr.radius<<16;
  CSprite *tmpg1=CAlloc(SpriteElemQuedBaseSize(SPT_ELLIPSE));
  tmpg1->type=SPT_ELLIPSE;

  x=tmpg->pr.x1; y=tmpg->pr.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tmpg1->pwha.x1=x;
  tmpg1->pwha.y1=y;

  x=radius; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&arg1,x,y);

  x=0; y=radius; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  m2=Sqrt(x*x+y*y);

  tmpg1->pwha.width =ToI64(m1)/0x10000;
  tmpg1->pwha.height=ToI64(m2)/0x10000;
  tmpg1->pwha.angle=-arg1;

  tmpg1->type|=tmpg->type&SPF_SEL;
  return tmpg1;
}

CSprite *SpriteTransformEllipse(I64 *r,CSprite *tmpg)
{
  I64 x,y,z;
  F64 m1,arg1,m2,arg2,s,c,x_radius=tmpg->pwha.width<<16,
	y_radius=tmpg->pwha.height<<16;
  CSprite *tmpg1=CAlloc(SpriteElemQuedBaseSize(tmpg->type&SPG_TYPE_MASK));
  tmpg1->type=tmpg->type;
  if (tmpg->type&SPG_TYPE_MASK==SPT_POLYGON)
    tmpg1->pwhas.sides=tmpg->pwhas.sides;

  x=tmpg->pwha.x1; y=tmpg->pwha.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tmpg1->pwha.x1=x;
  tmpg1->pwha.y1=y;

  c=Cos(-tmpg->pwha.angle);
  s=Sin(-tmpg->pwha.angle);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&arg1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m2,&arg2,x,y);
  m2*=Abs(Sin(arg2-arg1));

  tmpg1->pwha.width=ToI64(m1)/0x10000;
  if (tmpg1->pwha.width<1) tmpg1->pwha.width=1;
  tmpg1->pwha.height=ToI64(m2)/0x10000;
  if (tmpg1->pwha.height<1) tmpg1->pwha.height=1;
  tmpg1->pwha.angle=-arg1;

  tmpg1->type|=tmpg->type&SPF_SEL;
  return tmpg1;
}

CSprite *SpriteTransformRect(I64 *r,CSprite *tmpg,F64 é)
{
  I64 x,y,z,w,h;
  F64 m1,arg1,m2,arg2,s,c,
	x_radius=(tmpg->pp.x2-tmpg->pp.x1)<<16,
	y_radius=(tmpg->pp.y2-tmpg->pp.y1)<<16;
  CSprite *tmpg1=CAlloc(SpriteElemQuedBaseSize(SPT_ROTATED_RECT));
  tmpg1->type=SPT_ROTATED_RECT;

  x=tmpg->pp.x1; y=tmpg->pp.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tmpg1->ppa.x1=x;
  tmpg1->ppa.y1=y;

  c=Cos(-é);
  s=Sin(-é);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&arg1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m2,&arg2,x,y);
  m2*=Abs(Sin(arg2-arg1));

  w=ToI64(m1)/0x10000;
  if (w<1) w=1;
  h=ToI64(m2)/0x10000;
  if (h<1) h=1;
  tmpg1->ppa.x2=tmpg1->ppa.x1+w;
  tmpg1->ppa.y2=tmpg1->ppa.y1+h;
  tmpg1->ppa.angle=-arg1;

  tmpg1->type|=tmpg->type&SPF_SEL;
  return tmpg1;
}

CSprite *SpriteTransformBitMap(I64 *r,CSprite *tmpg)
{
  CDC *img,*dc3;
  U8 *elems;
  I64 x,y,z,minx,maxx,miny,maxy,minz,maxz;
  CSprite *tmpg1;

  x=tmpg->pwhu.x1; y=tmpg->pwhu.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  minx=maxx=x;
  miny=maxy=y;
  minz=maxz=z;

  x=tmpg->pwhu.x1; y=tmpg->pwhu.y1+tmpg->pwhu.height; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<miny) miny=y;
  if (y>maxy) maxy=y;
  if (z<minz) minz=z;
  if (z>maxz) maxz=z;

  x=tmpg->pwhu.x1+tmpg->pwhu.width; y=tmpg->pwhu.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<miny) miny=y;
  if (y>maxy) maxy=y;
  if (z<minz) minz=z;
  if (z>maxz) maxz=z;

  x=tmpg->pwhu.x1+tmpg->pwhu.width; y=tmpg->pwhu.y1+tmpg->pwhu.height; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<miny) miny=y;
  if (y>maxy) maxy=y;
  if (z<minz) minz=z;
  if (z>maxz) maxz=z;

  dc3=DCNew(maxx-minx+1,maxy-miny+1);

  img=CAlloc(sizeof(CDC));
  img->width=tmpg->pwhu.width;
  img->width_internal=(tmpg->pwhu.width+7)&~7;
  img->height=tmpg->pwhu.height;
  img->body=&tmpg->pwhu.u;
  img->dc_signature=DCS_SIGNATURE_VAL;

  dc3->color=TRANSPARENT;
  GrRect(dc3,0,0,maxx-minx+1,maxy-miny+1);

  Free(dc3->r);
  DCMat4x4Set(dc3,r);
  dc3->flags|=DCF_TRANSFORMATION;

  dc3->x=tmpg->pwhu.x1-minx;
  dc3->y=tmpg->pwhu.y1-miny;
  dc3->z=-minz;
  GrBlot3(dc3,0,0,0,img);
  Free(img);

  elems=DC2Sprite(dc3);
  dc3->r=NULL;
  DCDel(dc3);
  tmpg1=CAlloc(offset(CSprite.start)+MSize(elems));
  MemCpy(tmpg1(U8 *)+offset(CSprite.start),elems,MSize(elems));
  tmpg1->type=tmpg->type;

  x=tmpg->pwhu.x1; y=tmpg->pwhu.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tmpg1->pwhu.x1=x;
  tmpg1->pwhu.y1=y;

  return tmpg1;
}

U0 SpriteTransformQue(CSprite *head,I64 *r)
{
  I64 i,j,k,num,x,y,z,x1,y1,z1,x2,y2,z2,x3,y3,z3;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tmpg=head->next,head2,*tmpg1,*tmpg2,*tmpg3;
  while (tmpg!=head) {
    if (Bt(&tmpg->type,SPf_SEL))
      switch (tmpg->type&SPG_TYPE_MASK) {
	case SPT_THICK:
	  tmpg->t.thick*=Sqrt(Mat4x4NormSqr65536(r))/65536;
	  if (tmpg->t.thick<0) tmpg->t.thick=0;
	  break;
	case SPT_PLANAR_SYMMETRY:
	case SPT_ARROW:
	case SPT_LINE:
	  x=tmpg->pp.x2; y=tmpg->pp.y2; z=0;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  tmpg->pp.x2=x;
	  tmpg->pp.y2=y;
	case SPT_PT:
	case SPT_FLOOD_FILL:
	case SPT_FLOOD_FILL_NOT:
	case SPT_TEXT:
	case SPT_TEXT_BOX:
	case SPT_TEXT_DIAMOND:
	  x=tmpg->p.x1; y=tmpg->p.y1; z=0;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  tmpg->p.x1=x;
	  tmpg->p.y1=y;
	  break;
	case SPT_BITMAP:
	  tmpg1=SpriteTransformBitMap(r,tmpg);
	  QueIns(tmpg1,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg1;
	  break;
	case SPT_ROTATED_RECT:
	  tmpg1=SpriteTransformRect(r,tmpg,tmpg->ppa.angle);
	  QueIns(tmpg1,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg1;
	  break;
	case SPT_RECT:
	  tmpg1=SpriteTransformRect(r,tmpg,0);
	  QueIns(tmpg1,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg1;
	  break;
	case SPT_CIRCLE:
	  tmpg1=SpriteTransformCircle(r,tmpg);
	  QueIns(tmpg1,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg1;
	  break;
	case SPT_ELLIPSE:
	case SPT_POLYGON:
	  tmpg1=SpriteTransformEllipse(r,tmpg);
	  QueIns(tmpg1,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg1;
	  break;
	case SPT_POLYLINE:
	  ptr=&tmpg->nu.u;
	  for (i=0;i<tmpg->nu.num;i++) {
	    x=ptr[i<<1]; y=ptr[i<<1+1]; z=0;
	    Mat4x4MulXYZ(r,&x,&y,&z);
	    ptr[i<<1]=x;
	    ptr[i<<1+1]=y;
	  }
	  break;
	case SPT_POLYPT:
	  QueInit(&head2);
	  x=tmpg->npu.x; y=tmpg->npu.y; z=0;
	  x1=x; y1=y; z1=z;  //unrotated cur coordinates
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  ptr=&tmpg->npu.u;
	  k=tmpg->npu.num*3;
	  x2=x; y2=y; z2=z;  //rotated start coordinates
	  x3=x; y3=y; z3=z;  //lag 1 rotated coordinates
	  for (i=0;i<k;i+=3) {
	    j=BFieldExtU32(ptr,i,3);
	    x1+=gr_x_offsets[j];
	    y1+=gr_y_offsets[j];
	    x=x1; y=y1; z=z1;
	    Mat4x4MulXYZ(r,&x,&y,&z);
	    Line(&head2,x3-x2,y3-y2,0,x-x2,y-y2,0,&SpritePolyPtPlot);
	    x3=x; y3=y; z3=z;
	  }

	  num=0;
	  tmpg1=head2.next;
	  x3=0; y3=0; z3=0;
	  while (tmpg1!=&head2) {
	    tmpg2=tmpg1->next;
	    if (tmpg1->p.x1==x3 && tmpg1->p.y1==y3) {
	      QueRem(tmpg1);
	      Free(tmpg1);
	    } else {
	      num++;
	      x3=tmpg1->p.x1;
	      y3=tmpg1->p.y1;
	    }
	    tmpg1=tmpg2;
	  }

	  tmpg3=CAlloc(SpriteElemQuedBaseSize(SPT_POLYPT)+(num*3+7)>>3);
	  tmpg3->npu.x=x2;
	  tmpg3->npu.y=y2;
	  ptr=&tmpg3->npu.u;
	  x3=0;y3=0; z3=0;
	  i=0;
	  tmpg1=head2.next;
	  while (tmpg1!=&head2) {
	    tmpg2=tmpg1->next;
	    BFieldOrU32(ptr,i,
		  polypt_map[SignI64(tmpg1->p.x1-x3)+1+
		  3*(SignI64(tmpg1->p.y1-y3)+1)]);
	    i+=3;
	    x3=tmpg1->p.x1;y3=tmpg1->p.y1;
	    QueRem(tmpg1);
	    Free(tmpg1);
	    tmpg1=tmpg2;
	  }
	  tmpg3->type=SPT_POLYPT|tmpg->type&SPF_SEL;
	  tmpg3->npu.num=num;
	  QueIns(tmpg3,tmpg);
	  QueRem(tmpg);
	  Free(tmpg);
	  tmpg=tmpg3;
	  break;
	case SPT_BSPLINE2:
	case SPT_BSPLINE3:
	case SPT_BSPLINE2_CLOSED:
	case SPT_BSPLINE3_CLOSED:
	  p=&tmpg->nu.u;
	  for (i=0;i<tmpg->nu.num;i++,p++) {
	    x=p->x; y=p->y; z=p->z;
	    Mat4x4MulXYZ(r,&x,&y,&z);
	    p->x=x;
	    p->y=y;
	    p->z=z;
	  }
	  break;
	case SPT_SHIFTABLE_MESH:
	  x=tmpg->pmu.x; y=tmpg->pmu.y; z=tmpg->pmu.z;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  tmpg->pmu.x=x;
	  tmpg->pmu.y=y;
	  tmpg->pmu.z=z;
	  p=&tmpg->pmu.u;
	  for (i=0;i<tmpg->pmu.vertex_cnt;i++,p++) {
	    x=p->x; y=p->y; z=p->z;
	    Mat4x4MulXYZ(r,&x,&y,&z);
	    p->x=x;
	    p->y=y;
	    p->z=z;
	  }
	  break;
	case SPT_MESH:
	  p=&tmpg->mu.u;
	  for (i=0;i<tmpg->mu.vertex_cnt;i++,p++) {
	    x=p->x; y=p->y; z=p->z;
	    Mat4x4MulXYZ(r,&x,&y,&z);
	    p->x=x;
	    p->y=y;
	    p->z=z;
	  }
	  break;
      }
    tmpg=tmpg->next;
  }
}

I64 SpriteQueSelCnt(CSprite *head,Bool val=TRUE)
{
  I64 res=0;
  CSprite *tmpg=head->next;
  val=ToBool(val);
  while (tmpg!=head) {
    if (Bt(&tmpg->type,SPf_SEL)==val)
      res++;
    tmpg=tmpg->next;
  }
  return res;
}

I64 SpriteQueSelAll(CSprite *head,Bool val=TRUE)
{
  I64 res=0;
  CSprite *tmpg=head->next;
  while (tmpg!=head) {
    BEqu(&tmpg->type,SPf_SEL,val);
    res++;
    tmpg=tmpg->next;
  }
  return res;
}

Bool SpriteEdText(CSprite **_head,I64 *_cur_elem_num)
{
  Bool res;
  CSprite *head=*_head;
  U8 *elems=SpriteQue2Sprite(head);
  CDoc *doc=DocNew,*doc2,*old_put=DocPut;
  StrPrint(doc->filename.name,"AI:0x%X",doc);
  DocPrint(doc,"//$$PURPLE$$$$TX+CX,\"Sprite Edit as Text\"$$$$FG$$\n"
	"//$$LK+PU+CX,\"Click for Help\","
	"A=\"FI:::/Doc/SpriteEdText.DD.Z\"$$\n\n");
  Sprite2Code(doc,elems);
  Free(elems);
  while (TRUE) {
    if (res=PopUpPrint("DocEd(0x%X,0x%X);",doc,0)) {
      Fs->put_doc=doc2=DocNew;
      "$$WW,1$$";
      if (elems=Code2Sprite(doc)) {
	DocDel(doc2);
	Fs->put_doc=old_put;
	QueDel(head);
	Free(head);
	head=Sprite2SpriteQue(elems);
	Free(elems);
	*_cur_elem_num=QueCnt(head); //TODO: Might want to improve this.
	break;
      } else {
	PopUpPrint("DocEd(0x%X,0x%X);",doc2,0);
	DocDel(doc2);
	Fs->put_doc=old_put;
      }
    } else
      break;
  }
  DocDel(doc);
  if (_head) *_head=head;
  return res;
}

#define SPED_SEL_UNSEL_ALL	0
#define SPED_SEL		2
#define SPED_SEL_RECTS		3
#define SPED_UNSEL		4
#define SPED_UNSEL_RECTS	5
#define SPED_SHIFT_PTS		6
#define SPED_SHIFT_RECTS	7
#define SPED_SHIFT_SEL		8
#define SPED_TRANSFORM_SEL	9
#define SPED_SET_ORIGIN		10
#define SPED_SHIFT_SUB_ORIGIN	11
#define SPED_TEXT_ED		12
#define SPED_INS_CLIP		13
#define SPED_MAIN_MENU		14
#define SPED_EXIT		15

U0 GrInit3()
{
  DefineLstLoad("ST_SPRITE_ED_MENU","Select/Unselect All\0 \0Select\0"
	"Select Rects\0Unselect\0Unselect Rects\0Shift Points\0Shift Rects\0"
	"Shift Selected\0Transform Selected\0Set Origin\0"
	"Insert Shift SubOrigin\0Edit as Text\0Insert Clip\0Main Menu\0");
}
GrInit3;

I64 PopUpSpriteEd(CSprite **_head,I64 *_cur_elem_num)
{
  U8 *st;
  CTask *pu_task;
  I64 res;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite Edit Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteEd.DD.Z\"$$\n\n"
	"$$LTBLUE$$$$MU-UL,\"Select/Unselect All\",LE=SPED_SEL_UNSEL_ALL$$\n"
	"$$MU-UL,\"Select Elems\",LE=SPED_SEL$$\n"
	"$$MU-UL,\"Select Elems with Rects\",LE=SPED_SEL_RECTS$$\n"
	"$$MU-UL,\"Unsel Elems\",LE=SPED_UNSEL$$\n"
	"$$MU-UL,\"Unsel Elems with Rects\",LE=SPED_UNSEL_RECTS$$\n\n"
	"$$MU-UL,\"Shift Points\",LE=SPED_SHIFT_PTS$$\n"
	"$$MU-UL,\"Shift Points with Rects\",LE=SPED_SHIFT_RECTS$$\n"
	"$$MU-UL,\"Shift Selected Elems\",LE=SPED_SHIFT_SEL$$\n"
	"$$MU-UL,\"Transform Selected Elems\",LE=SPED_TRANSFORM_SEL$$\n\n"
	"$$MU-UL,\"Set Origin\",LE=SPED_SET_ORIGIN$$\n"
	"$$MU-UL,\"Insert Shift SubOrigin\",LE=SPED_SHIFT_SUB_ORIGIN$$\n\n"
	"$$MU-UL,\"Edit as Text\",LE=SPED_TEXT_ED$$\n"
	"$$MU-UL,\"Insert Clip Sprite's\",LE=SPED_INS_CLIP$$\n\n"
	"$$PURPLE$$$$MU-UL,\"+] Sprite Main Menu\",LE=SPED_MAIN_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=SPED_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$");
  st=MStrPrint("SpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_head,_cur_elem_num);
  PopUp(st,NULL,&pu_task);
  Free(st);
  res=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_head=SpriteSideBar2SpriteQue(DocPut(pu_task),*_head,_cur_elem_num);
    Kill(pu_task);
  }
  DocDel(doc);
  return res;
}

#define SPEDT_SIMPLE_PT		0
#define SPEDT_WIDTH_HEIGHT	1

#define SPEDF_SEL		1

class CEdSprite
{
  CEdSprite *next,*last;
  CSprite *g;
  I32 type,num,flags,xx,yy,zz;
  I32 *x,*y,*z,*w,*h;
};

CEdSprite *EdSpriteNew(I64 type,CSprite *tmpg)
{
  CEdSprite *res=CAlloc(sizeof(CEdSprite));
  res->g=tmpg;
  if (tmpg->type&SPF_SEL)
    res->flags|=SPEDF_SEL;
  res->type=type;
  return res;
}

U0 SpritePtQueNew(U8 *elems,I64 x,I64 y,CEdSprite *head)
{
  I64 i,num=0;
  I32 *ptr;
  CD3I32 *p;
  CEdSprite *tmpes;
  CSprite *tmpg=elems-offset(CSprite.start);
  QueInit(head);
  while (tmpg->type&SPG_TYPE_MASK) {
    switch (tmpg->type&SPG_TYPE_MASK) {
      case SPT_ELLIPSE:
      case SPT_POLYGON:
	tmpes=EdSpriteNew(SPEDT_WIDTH_HEIGHT,tmpg);
	tmpes->xx=x;
	tmpes->yy=y;
	tmpes->x=&tmpg->pwha.x1;
	tmpes->y=&tmpg->pwha.y1;
	tmpes->w=&tmpg->pwha.width;
	tmpes->h=&tmpg->pwha.height;
	tmpes->num=num;
	QueIns(tmpes,head->last);
	goto pq_x1_y1;
      case SPT_RECT:
      case SPT_ROTATED_RECT:
      case SPT_LINE:
      case SPT_ARROW:
      case SPT_PLANAR_SYMMETRY:
	tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	tmpes->xx=x;
	tmpes->yy=y;
	tmpes->x=&tmpg->pp.x2;
	tmpes->y=&tmpg->pp.y2;
	tmpes->num=num;
	QueIns(tmpes,head->last);
      case SPT_TEXT:
      case SPT_TEXT_BOX:
      case SPT_TEXT_DIAMOND:
      case SPT_PT:
      case SPT_BITMAP:
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
      case SPT_CIRCLE:
pq_x1_y1:
	tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	tmpes->xx=x;
	tmpes->yy=y;
	tmpes->x=&tmpg->p.x1;
	tmpes->y=&tmpg->p.y1;
	tmpes->num=num;
	QueIns(tmpes,head->last);
	break;
      case SPT_SHIFT:
	x+=tmpg->p.x1;
	y+=tmpg->p.y1;
	break;
      case SPT_POLYLINE:
	ptr=&tmpg->nu.u;
	for (i=0;i<tmpg->nu.num;i++) {
	  tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	  tmpes->xx=x;
	  tmpes->yy=y;
	  tmpes->x=&ptr[i<<1];
	  tmpes->y=&ptr[i<<1+1];
	  tmpes->num=num;
	  QueIns(tmpes,head->last);
	}
	break;
      case SPT_POLYPT:
	tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	tmpes->xx=x;
	tmpes->yy=y;
	tmpes->x=&tmpg->npu.x;
	tmpes->y=&tmpg->npu.y;
	tmpes->num=num;
	QueIns(tmpes,head->last);
	break;
      case SPT_BSPLINE2:
      case SPT_BSPLINE3:
      case SPT_BSPLINE2_CLOSED:
      case SPT_BSPLINE3_CLOSED:
	p=&tmpg->nu.u;
	for (i=0;i<tmpg->nu.num;i++) {
	  tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	  tmpes->xx=x;
	  tmpes->yy=y;
	  tmpes->x=&p[i].x;
	  tmpes->y=&p[i].y;
	  tmpes->z=&p[i].z;
	  tmpes->num=num;
	  QueIns(tmpes,head->last);
	}
	break;
      case SPT_MESH:
	break;
      case SPT_SHIFTABLE_MESH:
	tmpes=EdSpriteNew(SPEDT_SIMPLE_PT,tmpg);
	tmpes->xx=x;
	tmpes->yy=y;
	tmpes->x=&tmpg->pmu.x;
	tmpes->y=&tmpg->pmu.y;
	tmpes->z=&tmpg->pmu.z;
	tmpes->num=num;
	QueIns(tmpes,head->last);
	break;
    }
    tmpg(U8 *)+=SpriteElemSize(tmpg);
    num++;
  }
}

U0 SpriteCtrlPtsDraw(CDC *dc,CEdSprite *head)
{
  I64 x,y;
  CEdSprite *tmpes;
  Refresh;
  DCFill(dc);
  if (Blink(20)) {
    tmpes=head->next;
    while (tmpes!=head) {
      switch (tmpes->type) {
	case SPEDT_SIMPLE_PT:
	  x=*tmpes->x+tmpes->xx;
	  y=*tmpes->y+tmpes->yy;
	  break;
	case SPEDT_WIDTH_HEIGHT:
	  x=*tmpes->w+*tmpes->x+tmpes->xx;
	  y=*tmpes->h+*tmpes->y+tmpes->yy;
	  break;
      }
      if (tmpes->flags&SPEDF_SEL)
	dc->color=RED;
      else
	dc->color=BLACK;
      GrRect(dc,x-2,y-2,4,4);
      dc->color=WHITE;
      GrRect(dc,x-1,y-1,2,2);
      tmpes=tmpes->next;
    }
  }
}

U0 SpriteCtrlPtsMove(CEdSprite *head,I64 dx,I64 dy)
{
  CEdSprite *tmpes;
  tmpes=head->next;
  while (tmpes!=head) {
    if (tmpes->flags&SPEDF_SEL)
      switch (tmpes->type) {
	case SPEDT_SIMPLE_PT:
	  if (tmpes->x) *tmpes->x+=dx;
	  if (tmpes->y) *tmpes->y+=dy;
	  break;
	case SPEDT_WIDTH_HEIGHT:
	  if (tmpes->w) *tmpes->w+=dx;
	  if (tmpes->h) *tmpes->h+=dy;
	  break;
      }
    tmpes=tmpes->next;
  }
}

Bool SpriteSelUnselShiftPts(U8 *elems,I64 x,I64 y,I64 *_cur_elem_num,I64 mode)
{
  I64 msg_code,arg1,arg2,xx,yy,xx2,yy2,dd,best_dd,cur_elem_num;
  Bool res=TRUE;
  CDC *dc=DCAlias;
  CEdSprite head,*tmpes,*best_es;

  SpritePtQueNew(elems,x,y,&head);
  cur_elem_num=0;
  if (head.next!=&head) {
    while (TRUE) {
      SpriteCtrlPtsDraw(dc,&head); //has Refresh
      switch (msg_code=ScanMsg(&arg1,&arg2,
	    1<<MSG_MS_R_UP|1<<MSG_MS_L_DOWN|1<<MSG_KEY_DOWN)) {
	case MSG_MS_L_DOWN:
	  switch (mode) {
	    case SPED_SEL:
	    case SPED_UNSEL:
	    case SPED_SHIFT_PTS:
	      xx=arg1; yy=arg2;
	      best_dd=I64_MAX;
	      tmpes=head.next;
	      while (tmpes!=&head) {
		switch (tmpes->type) {
		  case SPEDT_SIMPLE_PT:
		    dd=SqrI64(*tmpes->x+tmpes->xx-xx)+
			  SqrI64(*tmpes->y+tmpes->yy-yy);
		    break;
		  case SPEDT_WIDTH_HEIGHT:
		    dd=SqrI64(*tmpes->x+*tmpes->w+tmpes->xx-xx)+
			  SqrI64(*tmpes->y+*tmpes->h+tmpes->yy-yy);
		    break;
		}
		if (dd<best_dd) {
		  best_dd=dd;
		  best_es=tmpes;
		}
		tmpes=tmpes->next;
	      }
	      cur_elem_num=best_es->num;
	      if (mode!=SPED_UNSEL) {
		best_es->flags|=SPEDF_SEL;
		best_es->g->type|=SPF_SEL;
	      } else {
		best_es->flags&=~SPEDF_SEL;
		best_es->g->type&=~SPF_SEL;
	      }
	      break;
	    start:
	      xx2=xx=arg1; yy2=yy=arg2;
	      while (TRUE) {
		SpriteCtrlPtsDraw(dc,&head);
		dc->color=ROPF_DITHER+WHITE<<16+RED;
		GrBorder(dc,xx,yy,xx2,yy2);
		if (msg_code=ScanMsg(&arg1,&arg2,
		      1<<MSG_MS_MOVE|1<<MSG_MS_L_UP)) {
		  if (msg_code==MSG_MS_MOVE) {
		    xx2=arg1; yy2=arg2;
		  } else
		    break;
		}
	      }
	      if (xx2<xx) SwapI64(&xx,&xx2);
	      if (yy2<yy) SwapI64(&yy,&yy2);
	      tmpes=head.next;
	      while (tmpes!=&head) {
		switch (tmpes->type) {
		  case SPEDT_SIMPLE_PT:
		    if (xx<=*tmpes->x+tmpes->xx<=xx2 &&
			  yy<=*tmpes->y+tmpes->yy<=yy2) {
		      if (mode!=SPED_UNSEL_RECTS) {
			tmpes->flags|=SPEDF_SEL;
			tmpes->g->type|=SPF_SEL;
		      } else {
			tmpes->flags&=~SPEDF_SEL;
			tmpes->g->type&=~SPF_SEL;
		      }
		    }
		    break;
		  case SPEDT_WIDTH_HEIGHT:
		    if (xx<=*tmpes->x+*tmpes->w+tmpes->xx<=xx2 &&
			  yy<=*tmpes->y+*tmpes->h+tmpes->yy<=yy2) {
		      if (mode!=SPED_UNSEL_RECTS) {
			tmpes->flags|=SPEDF_SEL;
			tmpes->g->type|=SPF_SEL;
		      } else {
			tmpes->flags&=~SPEDF_SEL;
			tmpes->g->type&=~SPF_SEL;
		      }
		    }
		    break;
		}
		tmpes=tmpes->next;
	      }
	      case SPED_SEL_RECTS:
	      case SPED_UNSEL_RECTS:
		break;
	      case SPED_SHIFT_RECTS:
		do {
		  SpriteCtrlPtsDraw(dc,&head);
		  msg_code=ScanMsg(&arg1,&arg2,
			1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN);
		  if (msg_code==MSG_KEY_DOWN) goto gs_key;
		} while (msg_code!=MSG_MS_L_DOWN);
		xx=arg1;yy=arg2;
		break;
	    end:
	  }
	  switch (mode) {
	    case SPED_SHIFT_PTS:
	    case SPED_SHIFT_RECTS:
	      do {
		SpriteCtrlPtsDraw(dc,&head);
		if (msg_code=ScanMsg(&arg1,&arg2,
		      1<<MSG_MS_MOVE|1<<MSG_MS_L_UP)) {
		  SpriteCtrlPtsMove(&head,arg1-xx,arg2-yy);
		  xx=arg1;yy=arg2;
		}
	      } while (msg_code!=MSG_MS_L_UP);
	      tmpes=head.next;
	      while (tmpes!=&head) {
		tmpes->flags&=~SPEDF_SEL;
		tmpes->g->type&=~SPF_SEL;
		tmpes=tmpes->next;
	      }
	      break;
	  }
	  break;
	case MSG_KEY_DOWN:
gs_key:
	  switch (arg1.u8[0]) {
	    case CH_SHIFT_ESC:
	      res=FALSE;
	    case CH_ESC:
	      GetMsg(&arg1,&arg2,1<<MSG_KEY_UP);
	      goto gs_done;
	    case 'p':
	    case 'P':
	      mode&=~1;
	      break;
	    case 'r':
	    case 'R':
	      mode|=1;
	      break;
	  }
	  break;
	case MSG_MS_R_UP:
	  goto gs_done;
      }
    }
gs_done:
    QueDel(&head,TRUE);
  }
  DCFill(dc);
  DCDel(dc);
  if (_cur_elem_num && res)
    *_cur_elem_num=cur_elem_num;
  return res;
}

I64 SpriteEd(CDoc *doc,CDocEntry *doc_ce,I64 x,I64 y,
	CSprite **_head,I64 *_cur_elem_num)
{
  CDocEntry *doc_e2;
  CDocBin *tmpb;
  Bool unlock;
  I64 i,r[16],msg_code,arg1,arg2,xx,yy,
	old_de_flags;
  CSprite *head2,*next,*last,*tmpg,*insert_pt;

  old_de_flags=doc_ce->de_flags;
  tmpb=doc_ce->bin_data;
  DocUnlock(doc);
  SpriteQueSelAll(*_head,FALSE);
  do {
    if (winmgr.fps<10)
      doc_ce->de_flags|=DOCEF_DONT_DRAW;
    StrCpy(Fs->task_title,"Sprite Edit Menu");
    i=PopUpSpriteEd(_head,_cur_elem_num);
    SpriteEdUpdate(doc,doc_ce,*_head);
    if (0<=i<SPED_EXIT) {
      StrCpy(Fs->task_title,DefineSub(i,"ST_SPRITE_ED_MENU"));
      switch (i) {
	case SPED_SEL_UNSEL_ALL:
	  if (!SpriteQueSelCnt(*_head))
	    SpriteQueSelAll(*_head);
	  else
	    SpriteQueSelAll(*_head,FALSE);
	  break;
	case SPED_SET_ORIGIN:
	  SpriteQueSelAll(*_head);
	  doc_ce->de_flags=old_de_flags;
	  GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP);
	  SpriteSetOrigin(*_head,x-arg1,y-arg2,0);
	  SpriteEdUpdate(doc,doc_ce,*_head);
	  SpriteQueSelAll(*_head,FALSE);
	  break;
	case SPED_SHIFT_SEL:
	  if (!SpriteQueSelCnt(*_head))
	    SpriteQueSelAll(*_head);
	  doc_ce->de_flags=old_de_flags;
	  GetMsg(&arg1,&arg2,1<<MSG_MS_L_DOWN);
	  xx=arg1; yy=arg2;
	  do {
	    msg_code=GetMsg(&arg1,&arg2,
		  1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	    SpriteSetOrigin(*_head,arg1-xx,arg2-yy,0);
	    xx=arg1; yy=arg2;
	    SpriteEdUpdate(doc,doc_ce,*_head);
	  } while (msg_code!=MSG_MS_L_UP);
	  if (!SpriteQueSelCnt(*_head,FALSE))
	    SpriteQueSelAll(*_head,FALSE);
	  break;
	case SPED_SEL:
	case SPED_SEL_RECTS:
	case SPED_UNSEL:
	case SPED_UNSEL_RECTS:
	case SPED_SHIFT_PTS:
	case SPED_SHIFT_RECTS:
	  RegOneTimePopUp(ARf_CSPRITE_PTS_RECTANGLES,
		"You can switch between points\n"
		"and rectangles with '$$GREEN$$p$$FG$$' and '$$GREEN$$r$$FG$$'.\n"
		"Press '$$GREEN$$r$$FG$$' after one rectangle\n"
		"to OR another rectangle.\n");
	  doc_ce->de_flags=old_de_flags;
	  if (SpriteSelUnselShiftPts(tmpb->data,x,y,_cur_elem_num,i)) {
	    QueDel(*_head);
	    Free(*_head);
	    *_head=Sprite2SpriteQue(tmpb->data);
	  } else
	    SpriteEdUpdate(doc,doc_ce,*_head);
	  break;
	case SPED_TRANSFORM_SEL:
	  if (!SpriteQueSelCnt(*_head))
	    SpriteQueSelAll(*_head);
	  if (PopUpTransform(r)) {
	    SpriteTransformQue(*_head,r);
	    SpriteEdUpdate(doc,doc_ce,*_head);
	  }
	  if (!SpriteQueSelCnt(*_head,FALSE))
	    SpriteQueSelAll(*_head,FALSE);
	  break;
	case SPED_SHIFT_SUB_ORIGIN:
	  doc_ce->de_flags=old_de_flags;
	  insert_pt=SpriteSetSettings(,*_head,*_cur_elem_num);
	  tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_SHIFT));
	  tmpg->type=SPT_SHIFT;
	  tmpg->p.x1=0;
	  tmpg->p.y1=0;
	  QueIns(tmpg,insert_pt->last);
	  GetMsg(&arg1,&arg2,1<<MSG_MS_L_DOWN);
	  xx=arg1; yy=arg2;
	  do {
	    msg_code=GetMsg(&arg1,&arg2,
		  1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	    tmpg->p.x1=arg1-xx;
	    tmpg->p.y1=arg2-yy;
	    SpriteEdUpdate(doc,doc_ce,*_head);
	  } while (msg_code!=MSG_MS_L_UP);
	  *_cur_elem_num+=1;
	  break;
	case SPED_INS_CLIP:
	  RegOneTimePopUp(ARf_CSPRITE_INS_CLIP,
		"You will probably want to shift around\n"
		"the location of element groups.  Use\n"
		"'Insert shift sub-origin' after picking the\n"
		"element to insert before.  Or,\n"
		"use 'shift points'.\n");
	  insert_pt=SpriteSetSettings(,*_head,*_cur_elem_num);
	  unlock=DocLock(sys_clip_doc);
	  doc_e2=sys_clip_doc->head.next;
	  while (doc_e2!=sys_clip_doc) {
	    if (doc_e2->type_u8==DOCT_SPRITE) {
	      head2=Sprite2SpriteQue(doc_e2->bin_data->data);
	      if (head2->next!=head2) {
		tmpg=head2->next;
		while (tmpg!=head2) {
		  *_cur_elem_num+=1;
		  tmpg=tmpg->next;
		}
		next=head2->next;
		last=head2->last;
		insert_pt->last->next=next;
		next->last=insert_pt->last;
		insert_pt->last=last;
		last->next=insert_pt;
	      }
	      Free(head2);
	    }
	    doc_e2=doc_e2->next;
	  }
	  if (unlock)
	    DocUnlock(sys_clip_doc);
	  SpriteEdUpdate(doc,doc_ce,*_head);
	  break;
	case SPED_TEXT_ED:
	  if (SpriteEdText(_head,_cur_elem_num))
	    SpriteEdUpdate(doc,doc_ce,*_head);
	  break;
      }
    }
  } while (i!=DOCM_CANCEL && i!=SPED_EXIT && i!=SPED_MAIN_MENU);
  doc_ce->de_flags=old_de_flags;

  switch (i) {
    case DOCM_CANCEL:	return SPE_ABORT;
    case SPED_EXIT:	return SPE_EXIT;
    case SPED_MAIN_MENU:	return SPE_CONT;
  }
}

#help_index "Graphics/Sprite;Sprites;Graphics/Math/3D Transformation"
public U8 *SpriteTransform(U8 *elems,I64 *r)
{//Rotate Sprite using 4x4 matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  U8 *res;
  CSprite *head=Sprite2SpriteQue(elems);
  SpriteQueSelAll(head);
  SpriteTransformQue(head,r);
  res=SpriteQue2Sprite(head);
  QueDel(head);
  Free(head);
  return res;
}
