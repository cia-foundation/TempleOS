#help_index "Graphics/Sprite;Sprites"
/*
$LK,"CSprite",A="MN:CSprite"$s are stored as a sequence of var
length operations with a 1-byte $LK,"type",A="MN:SPT_PT"$ leading
each operation.  They are stored, one after another,
in a chunk of memory terminated by a $LK,"zero",A="MN:SPT_END"$.
$LK,"Sprite3",A="MN:Sprite3"$() shows how the $LK,"CSprite",A="MN:CSprite"$ unions are used.

$LK,"SpriteElemSize",A="MN:SpriteElemSize"$() will return the size of a single
element, while $LK,"SpriteSize",A="MN:SpriteSize"$() will return the size
of an entire list.  Look at $LK,"sprite_elem_base_sizes",A="MN:sprite_elem_base_sizes"$.

See $MA-X+PU,"::/Apps/GrModels",LM="Cd(\"::/Apps/GrModels\");Dir;View;\n"$ for an example of
making CSprite by hand.  It uses $LK,"SPT_MESH",A="MN:SPT_MESH"$,
one of the most complicated.
*/

public U0 Sprite3(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems,Bool just_one=FALSE)
{//Plot a sprite into a CDC.
  CSprite *tempg=elems-offset(CSprite.start);
  I64 i,j,k,x1,y1,z1,x2,y2,
	cur_sprite_elem_num=0,*old_r,*r2,
	old_flags=dc->flags,
	old_pen_width=dc->pen_width;
  I32 *ptr;
  CColorROPU32 old_color=dc->color;
  CDC *img;
  CD3I32 *p;
  CGrSym old_sym;
  MemCpy(&old_sym,&dc->sym,sizeof(CGrSym));
  if (dc->flags & DCF_LOCATE_NEAREST) {
    dc->nearest_sprite_elem_num=0;
    dc->dist=dc->nearest_dist=MAX_I64;
  }
  while (tempg->type) {
    switch (tempg->type) {
      case SPT_COLOR:
	dc->color=dc->color&~(COLORROP_COLORS_MASK|ROPF_DITHER)|tempg->c.color;
	break;
      case SPT_DITHER_COLOR:
	dc->color=dc->color&~COLORROP_COLORS_MASK|
	      tempg->d.dither_color.u8[0]|
	      tempg->d.dither_color.u8[1]<<COLORROP_BITS|ROPF_DITHER;
	break;
      case SPT_WIDTH:
	dc->pen_width=tempg->w.width;
	DCPenWidthScale(dc);
	break;
      case SPT_TRANSFORM_ON:
	if (!(dc->flags&DCF_TRANSFORMATION)) {
	  x-=dc->x;
	  y-=dc->y;
	  z-=dc->z;
	}
	dc->flags|=DCF_TRANSFORMATION;
	break;
      case SPT_TRANSFORM_OFF:
	if (dc->flags&DCF_TRANSFORMATION) {
	  x+=dc->x;
	  y+=dc->y;
	  z+=dc->z;
	}
	dc->flags&=~DCF_TRANSFORMATION;
	break;
      case SPT_PT:
	GrPlot3(dc,tempg->p.x1+x,tempg->p.y1+y,z);
	break;
      case SPT_TEXT:
	GrPrint3(dc,tempg->ps.x1+x,tempg->ps.y1+y,z,"%s",tempg->ps.st);
	break;
      case SPT_TEXT_BOX:
	GrTextBox3(dc,tempg->ps.x1+x,tempg->ps.y1+y,z,tempg->ps.st);
	break;
      case SPT_TEXT_DIAMOND:
	GrTextDiamond3(dc,tempg->ps.x1+x,tempg->ps.y1+y,z,tempg->ps.st);
	break;
      case SPT_FLOOD_FILL:
	GrFloodFill3(dc,tempg->p.x1+x,tempg->p.y1+y,z,FALSE);
	break;
      case SPT_FLOOD_FILL_NOT:
	i=dc->color;
	dc->color=dc->color.c0;
	GrFloodFill3(dc,tempg->p.x1+x,tempg->p.y1+y,z,TRUE);
	dc->color=i;
	break;
      case SPT_SHIFT:
	x+=tempg->p.x1;
	y+=tempg->p.y1;
	break;
      case SPT_LINE:
	GrLine3(dc,tempg->pp.x1+x,tempg->pp.y1+y,z,
	      tempg->pp.x2+x,tempg->pp.y2+y,z);
	break;
      case SPT_ARROW:
	GrArrow3(dc,tempg->pp.x1+x,tempg->pp.y1+y,z,
	      tempg->pp.x2+x,tempg->pp.y2+y,z);
	break;
      case SPT_PLANAR_SYMMETRY:
	if (DCSymmetry3Set(dc,tempg->pp.x1+x,tempg->pp.y1+y,z,
	      tempg->pp.x2+x,tempg->pp.y2+y,z,
	      tempg->pp.x2+x,tempg->pp.y2+y,z+1))
	  dc->flags|=DCF_SYMMETRY;
	else
	  dc->flags&=~DCF_SYMMETRY;
	break;
      case SPT_BITMAP:
	img=CAlloc(sizeof(CDC));
	img->width=tempg->pwhu.width;
	img->width_internal=(tempg->pwhu.width+7)&~7;
	img->height=tempg->pwhu.height;
	img->body=&tempg->pwhu.u;
	img->dc_signature=DCS_SIGNATURE_VAL;
	GrBlot3(dc,tempg->pwhu.x1+x,tempg->pwhu.y1+y,z,img);
	Free(img);
	break;
      case SPT_RECT:
	GrRect3(dc,tempg->pp.x1+x,tempg->pp.y1+y,z,
	      tempg->pp.x2-tempg->pp.x1,tempg->pp.y2-tempg->pp.y1);
	break;
      case SPT_ROTATED_RECT:
	x1=tempg->ppa.x1+x;
	y1=tempg->ppa.y1+y;
	z1=z;
	Mat4x4MulXYZ(dc->r,&x1,&y1,&z1);
	old_r=dc->r;
	dc->flags|=DCF_TRANSFORMATION;
	r2=Mat4x4IdentNew;
	Mat4x4RotZ(r2,-tempg->ppa.angle);
	Mat4x4TranslationEqu(r2,x1,y1,z1);
	DCMat4x4Set(dc,Mat4x4MulMat4x4New(old_r,r2));
	GrRect3(dc,0,0,0,
	      tempg->ppa.x2-tempg->ppa.x1,tempg->ppa.y2-tempg->ppa.y1);
	Free(dc->r);
	Free(r2);
	DCMat4x4Set(dc,old_r);
	dc->flags=dc->flags&~DCF_TRANSFORMATION|old_flags;
	break;
      case SPT_CIRCLE:
	GrCircle3(dc,tempg->pr.x1+x,tempg->pr.y1+y,z,tempg->pr.radius);
	break;
      case SPT_ELLIPSE:
	GrEllipse3(dc,tempg->pwha.x1+x,tempg->pwha.y1+y,z,tempg->pwha.width,
	      tempg->pwha.height,tempg->pwha.angle);
	break;
      case SPT_POLYGON:
	GrRegPoly3(dc,tempg->pwhas.x1+x,tempg->pwhas.y1+y,z,tempg->pwhas.width,
	      tempg->pwhas.height,tempg->pwhas.sides,tempg->pwhas.angle);
	break;
      case SPT_POLYLINE:
	ptr=&tempg->nu.u;
	x1=ptr[0];
	y1=ptr[1];
	for (i=1;i<tempg->nu.num;i++) {
	  x2=ptr[i<<1];
	  y2=ptr[i<<1+1];
	  GrLine3(dc,x1+x,y1+y,z,x2+x,y2+y,z);
	  x1=x2;y1=y2;
	}
	break;
      case SPT_POLYPT:
	x1=tempg->npu.x;
	y1=tempg->npu.y;
	ptr=&tempg->npu.u;
	k=tempg->npu.num*3;
	GrPlot3(dc,x1+x,y1+y,z);
	for (i=0;i<k;i+=3) {
	  j=BFieldExtU32(ptr,i,3);
	  x1+=gr_x_offsets[j];
	  y1+=gr_y_offsets[j];
	  GrPlot3(dc,x1+x,y1+y,z);
	}
	break;
      sub_switch_start:
	p=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++,p++) {
	  p->x+=x;
	  p->y+=y;
	  p->z+=z;
	}
	case SPT_BSPLINE2:
	  Gr2BSpline3(dc,&tempg->nu.u,tempg->nu.num,FALSE);
	  break;
	case SPT_BSPLINE3:
	  Gr3BSpline3(dc,&tempg->nu.u,tempg->nu.num,FALSE);
	  break;
	case SPT_BSPLINE2_CLOSED:
	  Gr2BSpline3(dc,&tempg->nu.u,tempg->nu.num,TRUE);
	  break;
	case SPT_BSPLINE3_CLOSED:
	  Gr3BSpline3(dc,&tempg->nu.u,tempg->nu.num,TRUE);
	  break;
      sub_switch_end:
	p=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++,p++) {
	  p->x-=x;
	  p->y-=y;
	  p->z-=z;
	}
	break;
      case SPT_MESH:
	p=&tempg->mu.u;
	for (i=0;i<tempg->mu.vertex_cnt;i++,p++) {
	  p->x+=x;
	  p->y+=y;
	  p->z+=z;
	}
	Gr3Mesh(dc,tempg->mu.vertex_cnt,&tempg->mu.u,tempg->mu.tri_cnt,
	      (&tempg->mu.u)(U8 *)+sizeof(CD3I32)*tempg->mu.vertex_cnt);
	p=&tempg->mu.u;
	for (i=0;i<tempg->mu.vertex_cnt;i++,p++) {
	  p->x-=x;
	  p->y-=y;
	  p->z-=z;
	}
	break;
      case SPT_SHIFTABLE_MESH:
	if (dc->flags&DCF_TRANSFORMATION) {
	  dc->x+=tempg->pmu.x;
	  dc->y+=tempg->pmu.y;
	  dc->z+=tempg->pmu.z;
	  x1=x;
	  y1=y;
	  z1=z;
	} else {
	  x1=tempg->pmu.x+x;
	  y1=tempg->pmu.y+y;
	  z1=tempg->pmu.z+z;
	}
	p=&tempg->pmu.u;
	for (i=0;i<tempg->pmu.vertex_cnt;i++,p++) {
	  p->x+=x1;
	  p->y+=y1;
	  p->z+=z1;
	}
	Gr3Mesh(dc,tempg->pmu.vertex_cnt,&tempg->pmu.u,tempg->pmu.tri_cnt,
	      (&tempg->pmu.u)(U8 *)+sizeof(CD3I32)*tempg->pmu.vertex_cnt);
	p=&tempg->pmu.u;
	for (i=0;i<tempg->pmu.vertex_cnt;i++,p++) {
	  p->x-=x1;
	  p->y-=y1;
	  p->z-=z1;
	}
	if (dc->flags&DCF_TRANSFORMATION) {
	  dc->x-=tempg->pmu.x;
	  dc->y-=tempg->pmu.y;
	  dc->z-=tempg->pmu.z;
	}
	break;
    }
    if (dc->flags & DCF_LOCATE_NEAREST && dc->dist<dc->nearest_dist) {
      dc->nearest_dist=dc->dist;
      dc->nearest_sprite_elem_num=cur_sprite_elem_num;
    }
    cur_sprite_elem_num++;
    if (just_one) break;
    tempg(U8 *)+=SpriteElemSize(tempg);
  }
  MemCpy(&dc->sym,&old_sym,sizeof(CGrSym));
  dc->color=old_color;
  dc->pen_width=old_pen_width;
  dc->flags=dc->flags&~(DCF_SYMMETRY|DCF_TRANSFORMATION) |
	old_flags&(DCF_SYMMETRY|DCF_TRANSFORMATION);
}

public U0 Sprite3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems)
{//Plot a sprite into a CDC, post transform xyz translation.
  I64 old_x=dc->x,old_y=dc->y,old_z=dc->z,
	old_flags=dc->flags&DCF_TRANSFORMATION;
  dc->x=x; dc->y=y; dc->z=z;
  dc->flags|=DCF_TRANSFORMATION;
  Sprite3(dc,0,0,0,elems);
  dc->x=old_x; dc->y=old_y; dc->z=old_z;
  dc->flags=dc->flags&~DCF_TRANSFORMATION|old_flags;
}

public U0 SpriteMat3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems,I64 *m)
{//Plot rotated by matrix.
  I64 r[16],*old_r=dc->r,old_m[16],
	old_flags=dc->flags&DCF_TRANSFORMATION;
  MemCpy(old_m,m,16*sizeof(I64));
  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4TranslationAdd(m,x,y,z);
  dc->r=Mat4x4MulMat4x4Equ(r,old_r,m);
  Sprite3(dc,0,0,0,elems);
  dc->r=old_r;
  MemCpy(m,old_m,16*sizeof(I64));
  dc->flags=dc->flags&~DCF_TRANSFORMATION|old_flags;
}

public U0 SpriteX3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems,F64 è=0)
{//Plot rotated around X axis.
  I64 r[16];
  Mat4x4IdentEqu(r);
  Mat4x4RotX(r,è);
  SpriteMat3B(dc,x,y,z,elems,r);
}

public U0 SpriteY3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems,F64 ê=0)
{//Plot rotated around Y axis.
  I64 r[16];
  Mat4x4IdentEqu(r);
  Mat4x4RotY(r,ê);
  SpriteMat3B(dc,x,y,z,elems,r);
}

public U0 SpriteZ3B(CDC *dc=gr.dc,I64 x,I64 y,I64 z,U8 *elems,F64 é=0)
{//Plot rotated around Z axis.
  I64 r[16];
  Mat4x4IdentEqu(r);
  Mat4x4RotZ(r,é);
  SpriteMat3B(dc,x,y,z,elems,r);
}

public U0 SpriteExtents(U8 *elems,I64 *min_x=NULL,I64 *max_x=NULL,
				   I64 *min_y=NULL,I64 *max_y=NULL)
{//Ignores flood fills.
  CDC *dc=DCNew(MAX_I32,MAX_I32,Fs,TRUE);
  DCExtentsInit(dc);
  Sprite3(dc,MAX_I32/2,MAX_I32/2,MAX_I32/2,elems);
  if (dc->min_x<=dc->max_x) {
    dc->min_x-=MAX_I32/2;
    dc->max_x-=MAX_I32/2;
  }
  if (dc->min_y<=dc->max_y) {
    dc->min_y-=MAX_I32/2;
    dc->max_y-=MAX_I32/2;
  }
  if (min_x) *min_x=dc->min_x;
  if (max_x) *max_x=dc->max_x;
  if (min_y) *min_y=dc->min_y;
  if (max_y) *max_y=dc->max_y;
  DCDel(dc);
}

public CDC *Sprite2DC(U8 *elems)
{//Convert sprite to device context.
  CDC *result;
  I64 min_x,max_x,min_y,max_y;
  SpriteExtents(elems,&min_x,&max_x,&min_y,&max_y);
  result=DCNew(max_x-min_x+1,max_y-min_y+1);
  Sprite3(result,-min_x,-min_y,0,elems);
  return result;
}

public U8 *SpriteInterpolate(U8 *elems0,U8 *elems1,F64 t)
{//The two CSprite should be identical except for points shifted around.
//t ranges from 0.0 to 1.0.
  I64 i,t1=GR_SCALE*t,t0=GR_SCALE-t1;
  I32 *ptr0,*ptr1,*ptrr;
  CD3I32 *p0,*p1,*pr;
  U8 *result;
  CSprite *tempg0=elems0-offset(CSprite.start),
	*tempg1=elems1-offset(CSprite.start),*tempgr;
  if (t<0.5) {
    i=SpriteSize(elems0),
	  result=MAlloc(i);
    MemCpy(result,elems0,i);
  } else {
    i=SpriteSize(elems1),
	  result=MAlloc(i);
    MemCpy(result,elems1,i);
  }
  tempgr=result-offset(CSprite.start);
  while (tempg0->type) {
    if (tempg0->type!=tempg1->type)
      throw('Graphics');
    switch (tempg0->type) {
      case SPT_ROTATED_RECT:
	tempgr->ppa.angle=(tempg0->ppa.angle*t0+tempg1->ppa.angle*t1)/GR_SCALE;
      case SPT_RECT:
      case SPT_LINE:
      case SPT_ARROW:
      case SPT_PLANAR_SYMMETRY:
	tempgr->pp.x2=(tempg0->pp.x2*t0+tempg1->pp.x2*t1)>>32;
	tempgr->pp.y2=(tempg0->pp.y2*t0+tempg1->pp.y2*t1)>>32;
      case SPT_TEXT:
      case SPT_TEXT_BOX:
      case SPT_TEXT_DIAMOND:
      case SPT_PT:
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
      case SPT_SHIFT:
	tempgr->p.x1=(tempg0->p.x1*t0+tempg1->p.x1*t1)>>32;
	tempgr->p.y1=(tempg0->p.y1*t0+tempg1->p.y1*t1)>>32;
	break;
      case SPT_CIRCLE:
	tempgr->pr.radius=(tempg0->pr.radius*t0+tempg1->pr.radius*t1)>>32;
	tempgr->pr.x1=(tempg0->pr.x1*t0+tempg1->pr.x1*t1)>>32;
	tempgr->pr.y1=(tempg0->pr.y1*t0+tempg1->pr.y1*t1)>>32;
	break;
      case SPT_ELLIPSE:
      case SPT_POLYGON:
	tempgr->pwha.x1=(tempg0->pwha.x1*t0+tempg1->pwha.x1*t1)>>32;
	tempgr->pwha.y1=(tempg0->pwha.y1*t0+tempg1->pwha.y1*t1)>>32;
	tempgr->pwha.width =(tempg0->pwha.width *t0+tempg1->pwha.width*t1)>>32;
	tempgr->pwha.height=(tempg0->pwha.height*t0+tempg1->pwha.height*t1)>>32;
	break;
      case SPT_BITMAP:
	tempgr->pwhu.x1=(tempg0->pwhu.x1*t0+tempg1->pwhu.x1*t1)>>32;
	tempgr->pwhu.y1=(tempg0->pwhu.y1*t0+tempg1->pwhu.y1*t1)>>32;
	break;
      case SPT_POLYLINE:
	ptr0=&tempg0->nu.u;
	ptr1=&tempg1->nu.u;
	ptrr=&tempgr->nu.u;
	for (i=0;i<tempg0->nu.num;i++) {
	  ptrr[i<<1]=(ptr0[i<<1]*t0+ptr1[i<<1]*t1)>>32;
	  ptrr[i<<1+1]=(ptr0[i<<1+1]*t0+ptr1[i<<1+1]*t1)>>32;
	}
	break;
      case SPT_POLYPT:
	tempgr->npu.x=(tempg0->npu.x*t0+tempg1->npu.x*t1)>>32;
	tempgr->npu.y=(tempg0->npu.y*t0+tempg1->npu.y*t1)>>32;
	break;
      case SPT_BSPLINE2:
      case SPT_BSPLINE3:
      case SPT_BSPLINE2_CLOSED:
      case SPT_BSPLINE3_CLOSED:
	p0=&tempg0->nu.u;
	p1=&tempg1->nu.u;
	pr=&tempgr->nu.u;
	for (i=0;i<tempg0->nu.num;i++) {
	  pr[i].x=(p0[i].x*t0+p1[i].x*t1)>>32;
	  pr[i].y=(p0[i].y*t0+p1[i].y*t1)>>32;
	  pr[i].z=(p0[i].z*t0+p1[i].z*t1)>>32;
	}
	break;
      case SPT_MESH:
	p0=&tempg0->mu.u;
	p1=&tempg1->mu.u;
	pr=&tempgr->mu.u;
	for (i=0;i<tempg0->mu.vertex_cnt;i++) {
	  pr[i].x=(p0[i].x*t0+p1[i].x*t1)>>32;
	  pr[i].y=(p0[i].y*t0+p1[i].y*t1)>>32;
	  pr[i].z=(p0[i].z*t0+p1[i].z*t1)>>32;
	}
	break;
      case SPT_SHIFTABLE_MESH:
	p0=&tempg0->pmu.u;
	p1=&tempg1->pmu.u;
	pr=&tempgr->pmu.u;
	for (i=0;i<tempg0->pmu.vertex_cnt;i++) {
	  pr[i].x=(p0[i].x*t0+p1[i].x*t1)>>32;
	  pr[i].y=(p0[i].y*t0+p1[i].y*t1)>>32;
	  pr[i].z=(p0[i].z*t0+p1[i].z*t1)>>32;
	}
	break;
    }
    tempg0(U8 *)+=SpriteElemSize(tempg0);
    tempg1(U8 *)+=SpriteElemSize(tempg1);
    tempgr(U8 *)+=SpriteElemSize(tempgr);
  }
  return result;
}

#help_index "Graphics/Sprite;DolDoc/Output;StdOut/DolDoc"
public CDocEntry *DocSprite(CDoc *doc=NULL,U8 *elems,U8 *fmt=NULL)
{//Put a sprite into a document.  You can, optionally, supply a fmt string
//for $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.TXT"$ cmd with a %d for the bin_num.
  I64 size;
  U8 *st;
  Bool unlock;
  CDocEntry *doc_e;
  CDocBin *tempb;
  if (!doc && !(doc=DocPut)) return NULL;
  unlock=DocLock(doc);
  size=SpriteSize(elems);
  tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
  tempb->size=size;
  tempb->data=MAlloc(size,doc->mem_task);
  MemCpy(tempb->data,elems,size);
  tempb->num=doc->cur_bin_num;
  tempb->use_cnt=1;
  QueIns(tempb,doc->bin_root.last);
  if (fmt)
    st=MStrPrint(fmt,doc->cur_bin_num++);
  else
    st=MStrPrint("$$SP,\"\",BI=%d$$",doc->cur_bin_num++);
  doc_e=DocPrint(doc,"%s",st);
  Free(st);
  doc_e->bin_data=tempb;
  if (doc_e && doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
    tempb->tag=StrNew(doc_e->tag,doc->mem_task);
  if (unlock)
    DocUnlock(doc);
  return doc_e;
}

public CDocEntry *Sprite(U8 *elems,U8 *fmt=NULL)
{//Put sprite to the command-line, $LK,"DocPut",A="MN:DocPut"$.
//If you set fmt, then include dollars ("$$SP ...$$") and leave %d for num.
  CDoc *doc;
  if (doc=DocPut)
    return DocSprite(doc,elems,fmt);
  return NULL;
}
