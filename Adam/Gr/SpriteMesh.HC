#help_index "Graphics/Mesh"
#define MESH_WORKSPACE_SIZE	4000

#define VF_SEL		1
#define VF_COPIED	2
#define VF_IGNORE	4

class CMeshEdVertex
{
  CMeshEdVertex *next,*last,*copy;

  U0 start;
  CD3I32 p; //World coordinates of the point.
  U0 end;
  CD3I32 p0,
	pt; //Transformed coordinates.	(Scrn)
  I32 num,flags;
};

#define TF_SEL	1
#define TF_COPIED	2

class CMeshEdTri
{
  CMeshEdTri *next,*last;

  U0 start;
  CMeshTri mt;
  U0 end;

  I32 cpu_num,flags; //Draw different tris with different cores.
  CMeshEdVertex *t[3];
};

class CMeshFrame
{
  I64 ms_z,thickness; //Mouse Z-coordinate
  I64 ed_mode,cx,cy;
  CColorROPU32 cur_color;
  Bool grid_on,flip_y,sel_rect,vertex_on,closed,pad[3];
  I64 mp_not_done_flags; //Used for multiprocessing signaling.
  F64 view_scale;
  CDC *dc;
  I32 *depth_buf;
  I64 *w2s,*s2w; //Scrn-to-world and world-to-scrn transform matrices.
  I64 vertex_cnt,tri_cnt; //Set by MeshSize
  CMeshEdVertex vertex_head,*cur_vertex,*chain_pred;
  CMeshEdTri	tri_head,*cur_tri;
  I64 x1,y1,x2,y2,cur_snap;
};

CMeshEdVertex *MeshVertexNew(CMeshFrame *e,I64 x,I64 y,I64 z)
{
  CMeshEdVertex *tmpv=CAlloc(sizeof(CMeshEdVertex));
  tmpv->p.x=x;
  tmpv->p.y=y;
  tmpv->p.z=z;
  QueIns(tmpv,e->vertex_head.last);
  return tmpv;
}

CMeshEdTri *MeshTriNew(CMeshFrame *e,CColorROPU32 color,
	CMeshEdVertex *v1,CMeshEdVertex *v2,CMeshEdVertex *v3)
{
  static I64 cpu_num=0;
  CMeshEdTri *tmpt=CAlloc(sizeof(CMeshEdTri));
  tmpt->cpu_num=cpu_num++%mp_cnt;
  tmpt->mt.color=color;
  tmpt->t[0]=v1;
  tmpt->t[1]=v2;
  tmpt->t[2]=v3;
  QueIns(tmpt,e->tri_head.last);
  return tmpt;
}

CMeshEdVertex *MeshVertexFindScrPt(CMeshFrame *e,I64 x,I64 y)
{//Scrn coordinates
  CMeshEdVertex *res=NULL,*tmpv=e->vertex_head.next;
  I64 dd,dz,best_dd=I64_MAX,best_dz=I64_MAX;
  while (tmpv!=&e->vertex_head) {
    if (!(tmpv->flags&VF_IGNORE)) {
      dd=SqrI64(x-tmpv->pt.x)+SqrI64(y-tmpv->pt.y);
      dz=AbsI64(e->ms_z-tmpv->p.z);
      if (dd<best_dd || dd==best_dd && dz<best_dz) {
	res=tmpv;
	best_dd=dd;
	best_dz=dz;
      }
    }
    tmpv=tmpv->next;
  }
  return res;
}

CMeshEdVertex *MeshVertexFindNum(CMeshFrame *haystack_e,I64 needle_num)
{
  CMeshEdVertex *tmpv=haystack_e->vertex_head.next;
  while (tmpv!=&haystack_e->vertex_head) {
    if (tmpv->num==needle_num)
      return tmpv;
    tmpv=tmpv->next;
  }
  return NULL;
}

U0 MeshTriDel(CMeshFrame *e,CMeshEdTri *tmpt)
{
  if (tmpt) {
    if (tmpt==e->cur_tri)
      e->cur_tri=NULL;
    QueRem(tmpt);
    Free(tmpt);
  }
}

U0 MeshVertexDel(CMeshFrame *e,CMeshEdVertex *tmpv)
{
  I64 i;
  CMeshEdTri *tmpt,*tmpt1;
  if (tmpv) {
    tmpt=e->tri_head.next;
    while (tmpt!=&e->tri_head) {
      tmpt1=tmpt->next;
      for (i=0;i<3;i++)
	if (tmpt->t[i]==tmpv)
	  break;
      if (i<3)
	MeshTriDel(e,tmpt);
      tmpt=tmpt1;
    }
    if (tmpv==e->cur_vertex)
      e->cur_vertex=NULL;
    if (tmpv==e->chain_pred)
      e->chain_pred=NULL;
    QueRem(tmpv);
    Free(tmpv);
  }
}

U0 MeshFence(CMeshFrame *e)
{
  CMeshEdVertex *tmpv,*tmpv1,*tmpv_last=NULL,*tmpv1_last=NULL,
	*start=e->chain_pred->next,*end=e->vertex_head.last;
  tmpv=start;
  while (TRUE) {
    tmpv1=MeshVertexNew(e,tmpv->p.x,tmpv->p.y,tmpv->p.z+e->thickness);
    if (tmpv_last) {
      MeshTriNew(e,e->cur_color,tmpv_last,tmpv,tmpv1);
      MeshTriNew(e,e->cur_color,tmpv1,tmpv1_last,tmpv_last);
    }
    tmpv_last=tmpv;
    tmpv1_last=tmpv1;
    if (tmpv==end)
      break;
    tmpv=tmpv->next;
  }
  if (e->closed && tmpv_last) {
    MeshTriNew(e,e->cur_color,tmpv_last,start,end->next);
    MeshTriNew(e,e->cur_color,end->next,tmpv1_last,tmpv_last);
  }
}

U0 MeshPolygon(CMeshFrame *e,CMeshEdVertex *start,CMeshEdVertex *end,Bool rev)
{
  CMeshEdVertex *tmpv,*tmpv1;
  if (start!=end) {
    tmpv=start;
    tmpv1=tmpv->next;
    while (tmpv1!=end) {
      if (rev)
	MeshTriNew(e,e->cur_color,tmpv1,tmpv,end);
      else
	MeshTriNew(e,e->cur_color,tmpv,tmpv1,end);
      tmpv=tmpv->next;
      tmpv1=tmpv1->next;
    }
  }
}

U0 MeshPrism(CMeshFrame *e)
{
  CMeshEdVertex *start=e->chain_pred->next,*end=e->vertex_head.last;
  MeshFence(e);
  MeshPolygon(e,start,end,FALSE);
  MeshPolygon(e,end->next,e->vertex_head.last,TRUE);
}

U0 MeshVertexSelAll(CMeshFrame *e,Bool val)
{
  CMeshEdVertex *tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    if (val)
      tmpv->flags|=VF_SEL;
    else
      tmpv->flags&=~VF_SEL;
    tmpv=tmpv->next;
  }
}

U0 MeshTriSelAll(CMeshFrame *e,Bool val)
{
  CMeshEdTri *tmpt=e->tri_head.next;
  while (tmpt!=&e->tri_head) {
    if (val)
      tmpt->flags|=TF_SEL;
    else
      tmpt->flags&=~TF_SEL;
    tmpt=tmpt->next;
  }
}

U0 MeshVertexIgnoreSet(CMeshFrame *e,Bool val)
{
  CMeshEdVertex *tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    tmpv->flags&=~VF_IGNORE;
    if (tmpv->flags&VF_SEL && val)
      tmpv->flags|=VF_IGNORE;
    tmpv=tmpv->next;
  }
}

U0 MeshP0Capture(CMeshFrame *e)
{
  CMeshEdVertex *tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    MemCpy(&tmpv->p0,&tmpv->p,sizeof(CD3I32));
    tmpv=tmpv->next;
  }
}

U0 MeshP0Offset(CMeshFrame *e,I64 dx,I64 dy,I64 dz)
{
  CMeshEdVertex *tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    if (tmpv->flags&VF_SEL) {
      tmpv->p.x=tmpv->p0.x+dx;
      tmpv->p.y=tmpv->p0.y+dy;
      tmpv->p.z=tmpv->p0.z+dz;
    }
    tmpv=tmpv->next;
  }
}

#define SEL_MESH_EQU	0
#define SEL_MESH_OR	1
#define SEL_MESH_AND	2

U0 MeshVertexSelRect(CMeshFrame *e,I64 sel_mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CMeshEdVertex *tmpv=e->vertex_head.next;
  if (x1>x2) SwapI64(&x1,&x2);
  if (y1>y2) SwapI64(&y1,&y2);
  while (tmpv!=&e->vertex_head) {
    if (x1<=tmpv->pt.x<=x2 &&
	  y1<=tmpv->pt.y<=y2) {
      if (sel_mode==SEL_MESH_AND)
	tmpv->flags&=~VF_SEL;
      else
	tmpv->flags|=VF_SEL;
    } else if (sel_mode==SEL_MESH_EQU)
      tmpv->flags&=~VF_SEL;
    tmpv=tmpv->next;
  }
}

U0 MeshTriSelRect(CMeshFrame *e,I64 sel_mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CMeshEdTri *tmpt=e->tri_head.next;
  if (x1>x2) SwapI64(&x1,&x2);
  if (y1>y2) SwapI64(&y1,&y2);
  while (tmpt!=&e->tri_head) {
    if (x1<=tmpt->t[0]->pt.x<=x2 &&
	  y1<=tmpt->t[0]->pt.y<=y2 &&
	  x1<=tmpt->t[1]->pt.x<=x2 &&
	  y1<=tmpt->t[1]->pt.y<=y2 &&
	  x1<=tmpt->t[2]->pt.x<=x2 &&
	  y1<=tmpt->t[2]->pt.y<=y2) {
      if (sel_mode==SEL_MESH_AND)
	tmpt->flags&=~TF_SEL;
      else
	tmpt->flags|=TF_SEL;
    } else {
      if (sel_mode==SEL_MESH_EQU)
	tmpt->flags&=~TF_SEL;
      else if (sel_mode==SEL_MESH_AND) {
	if (x1<=tmpt->t[0]->pt.x<=x2 &&
	      y1<=tmpt->t[0]->pt.y<=y2 ||
	      x1<=tmpt->t[1]->pt.x<=x2 &&
	      y1<=tmpt->t[1]->pt.y<=y2 ||
	      x1<=tmpt->t[2]->pt.x<=x2 &&
	      y1<=tmpt->t[2]->pt.y<=y2)
	  tmpt->flags&=~TF_SEL;
      }
    }
    tmpt=tmpt->next;
  }
}

I64 MeshSelCnt(CMeshFrame *e)
{
  I64 res=0;
  CMeshEdVertex *tmpv=e->vertex_head.next;
  CMeshEdTri *tmpt=e->tri_head.next;
  while (tmpv!=&e->vertex_head) {
    if (tmpv->flags&VF_SEL)
      res++;
    tmpv=tmpv->next;
  }
  while (tmpt!=&e->tri_head) {
    if (tmpt->flags&TF_SEL)
      res++;
    tmpt=tmpt->next;
  }
  return res;
}

U0 MeshSwapAxes(CMeshFrame *e,I64 o1,I64 o2)
{
  Bool unsel;
  CMeshEdVertex *tmpv=e->vertex_head.next;
  if (!MeshSelCnt(e)) {
    MeshVertexSelAll(e,TRUE);
    unsel=TRUE;
  } else
    unsel=FALSE;
  while (tmpv!=&e->vertex_head) {
    if (tmpv->flags&VF_SEL)
      SwapU32((&tmpv->p)(U8 *)+o1,(&tmpv->p)(U8 *)+o2);
    tmpv=tmpv->next;
  }
  if (unsel)
    MeshVertexSelAll(e,FALSE);
}

U0 MeshInvertAxis(CMeshFrame *e,I64 o)
{
  Bool unsel;
  CMeshEdVertex *tmpv=e->vertex_head.next;
  if (!MeshSelCnt(e)) {
    MeshVertexSelAll(e,TRUE);
    unsel=TRUE;
  } else
    unsel=FALSE;
  while (tmpv!=&e->vertex_head) {
    if (tmpv->flags&VF_SEL)
      *((&tmpv->p)(U8 *)+o)(I32 *)=-*((&tmpv->p)(U8 *)+o)(I32 *);
    tmpv=tmpv->next;
  }
  if (unsel)
    MeshVertexSelAll(e,FALSE);
}

U0 MeshTransformSel(CMeshFrame *e)
{
  Bool unsel;
  I64 r[16],x,y,z;
  CMeshEdVertex *tmpv=e->vertex_head.next;
  if (PopUpTransform(r)) {
    if (!MeshSelCnt(e)) {
      MeshVertexSelAll(e,TRUE);
      unsel=TRUE;
    } else
      unsel=FALSE;
    while (tmpv!=&e->vertex_head) {
      if (tmpv->flags&VF_SEL) {
	x=tmpv->p.x; y=tmpv->p.y; z=tmpv->p.z;
	Mat4x4MulXYZ(r,&x,&y,&z);
	tmpv->p.x=x; tmpv->p.y=y; tmpv->p.z=z;
      }
      tmpv=tmpv->next;
    }
    if (unsel)
      MeshVertexSelAll(e,FALSE);
  }
}

U0 MeshColorTris(CMeshFrame *e)
{
  Bool unsel;
  CMeshEdTri *tmpt=e->tri_head.next;
  if (!MeshSelCnt(e)) {
    MeshTriSelAll(e,TRUE);
    unsel=TRUE;
  } else
    unsel=FALSE;
  while (tmpt!=&e->tri_head) {
    if (tmpt->flags & TF_SEL)
      tmpt->mt.color=e->cur_color;
    tmpt=tmpt->next;
  }
  if (unsel)
    MeshTriSelAll(e,FALSE);
}

U0 MeshRevTris(CMeshFrame *e)
{
  Bool unsel;
  CMeshEdTri *tmpt=e->tri_head.next;
  if (!MeshSelCnt(e)) {
    MeshTriSelAll(e,TRUE);
    unsel=TRUE;
  } else
    unsel=FALSE;
  while (tmpt!=&e->tri_head) {
    if (tmpt->flags & TF_SEL)
      SwapI64(&tmpt->t[1],&tmpt->t[2]);
    tmpt=tmpt->next;
  }
  if (unsel)
    MeshTriSelAll(e,FALSE);
}

U0 MeshRecalcCxCy(CTask *task,CMeshFrame *e)
{
  e->cx=RoundI64(task->pix_width/2 -task->horz_scroll.pos,e->cur_snap);
  e->cy=RoundI64(task->pix_height/2-task->vert_scroll.pos,e->cur_snap);
}

U0 MeshCurSnap(CMeshFrame *e)
{
  I64 x1,y1,z1,x2,y2,z2;
  if (e->w2s) {
    x1=e->cur_snap<<16; y1=0; z1=0;
    Mat4x4MulXYZ(e->w2s,&x1,&y1,&z1);
    x2=0; y2=e->cur_snap<<16; z2=0;
    Mat4x4MulXYZ(e->w2s,&x2,&y2,&z2);
    ms_grid.x=Max(1,MaxI64(x1,x2)>>16);
    ms_grid.y=Max(1,MaxI64(y1,y2)>>16);
    ms_grid.z=Min(ms_grid.x,ms_grid.y);
  }
}

U0 MeshScaleZoom(CMeshFrame *e,F64 scale)
{
  CTask *task=Fs;
  I64	x=ms.pos.x-task->pix_left-task->scroll_x-task->pix_width/2,
	y=ms.pos.y-task->pix_top-task->scroll_y-task->pix_height/2;
  task->horz_scroll.pos*=scale;
  task->vert_scroll.pos*=scale;
  task->horz_scroll.pos+=scale*x-x;
  task->vert_scroll.pos+=scale*y-y;
  e->view_scale*=scale;
  MeshRecalcCxCy(task,e);
  MeshCurSnap(e);
}

U0 MPDrawIt(CMeshFrame *e)
{//Multiprocessing draw it, called by each core.

  //Makes a copy of e->dc so we can change dc->color member and stuff.
  CDC *dc=DCAlias(e->dc,e->dc->win_task);

  CMeshEdTri *tmpt=e->tri_head.next;
  I64 i,*old_r=dc->r;

  //$LK,"DCAlias",A="MN:DCAlias"$() allocs a new identity rotation matrix.
  //We want e->dc's rotation matrix.
  dc->r=e->dc->r;
  dc->depth_buf=e->depth_buf;
  MemCpy(&dc->ls,&e->dc->ls,sizeof(CD3I32));

  //... and translation (shift) vals.
  dc->x=e->dc->x;
  dc->y=e->dc->y;
  dc->z=e->dc->z;
  dc->flags|=DCF_TRANSFORMATION;

  if (e->grid_on)
//Draw grid with different cores.
    for (i=-500+25*Gs->num;i<=500;i+=25*mp_cnt) {
      if (i) {
	dc->color=DKGRAY;
	GrLine3(dc,i,-500,0,i,500,0);
	dc->color=LTGRAY;
	GrLine3(dc,-500,i,0,500,i,0);
      }
    }
  if (!Gs->num) {
    dc->color=RED;	//Y-Axis red
    GrLine3(dc,0,0,0,0,500,0);
    dc->color=ROPF_DITHER+RED;	 //Y-Axis red
    GrLine3(dc,0,-500,0,0,0,0);

    dc->color=YELLOW;	//X-Axis yellow
    GrLine3(dc,0,0,0,500,0,0);
    dc->color=ROPF_DITHER+YELLOW;	 //X-Axis yellow
    GrLine3(dc,-500,0,0,0,0,0);

    dc->color=GREEN;	//Z-Axis green
    GrLine3(dc,0,0,0,0,0,500);
    dc->color=ROPF_DITHER+GREEN;	 //Z-Axis green
    GrLine3(dc,0,0,-500,0,0,0);
  }

  while (tmpt!=&e->tri_head) {
    if (tmpt->cpu_num==Gs->num) {
      if (tmpt->flags & TF_SEL) {
	if (Blink)
	  dc->color=ROPF_DITHER+WHITE<<16+RED;
	else
	  dc->color=ROPF_DITHER+RED<<16+WHITE;
	GrFillTri0(dc,&tmpt->t[0]->pt,&tmpt->t[1]->pt,&tmpt->t[2]->pt);
      } else {
	(*dc->lighting)(dc,&tmpt->t[0]->pt,&tmpt->t[1]->pt,
	      &tmpt->t[2]->pt,tmpt->mt.color);
	GrFillTri0(dc,&tmpt->t[0]->pt,&tmpt->t[1]->pt,&tmpt->t[2]->pt);
      }
    }
    tmpt=tmpt->next;
  }
  dc->r=old_r;

  //e->dc's depth buf was copied but we don't want it freed during $LK,"DCDel",A="MN:DCDel"$().
  dc->depth_buf=NULL;

  DCDel(dc);
  LBtr(&e->mp_not_done_flags,Gs->num);
}

I64 *MeshW2S(CMeshFrame *e,CTask *task)
{//World to scrn coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=Mat4x4IdentNew(task);
  Mat4x4Scale(r,e->view_scale);
  Mat4x4RotZ(r,s->az);
  Mat4x4RotY(r,s->ay);
  if (e->flip_y)
    Mat4x4RotX(r,s->ax);
  else
    Mat4x4RotX(r,s->ax+ã);
  return r;
}

I64 *MeshS2W(CMeshFrame *e,CTask *task)
{//Scrn to world coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=Mat4x4IdentNew(task);
  if (e->flip_y)
    Mat4x4RotX(r,-s->ax);
  else
    Mat4x4RotX(r,-(s->ax+ã));
  Mat4x4RotY(r,-s->ay);
  Mat4x4RotZ(r,-s->az);
  Mat4x4Scale(r,1/e->view_scale);
  return r;
}

I64 *MeshSetW2S(CMeshFrame *e,CTask *task)
{
  Free(e->w2s);
  e->w2s=MeshW2S(e,task);
  Free(e->s2w);
  e->s2w=MeshS2W(e,task);
//returned matrix is assigned to dc->r and will be freed by $LK,"DCDel",A="MN:DCDel"$().
  return Mat4x4New(e->w2s,task);
}

U0 MeshCursorW(CMeshFrame *e,CTask *task,I64 *_x,I64 *_y,I64 *_z)
{
  I64	x_shadow,y_shadow,z_shadow,
	xc=ms.pos.x-task->pix_left-task->scroll_x-e->cx,
	yc=ms.pos.y-task->pix_top-task->scroll_y-e->cy,zc=0,
	x=0,y=0,z=e->ms_z,
	i,x2,y2,z2;
  Mat4x4MulXYZ(e->w2s,&x,&y,&z); //scrn of Z vect

  //Converges onto a solution for zc, an unknown.
  for (i=0;i<128;i++) {
    x_shadow=xc-x; //Shadow of mouse cursor on xy plane
    y_shadow=yc-y;
    z_shadow=zc-z;
    Mat4x4MulXYZ(e->s2w,&x_shadow,&y_shadow,&z_shadow);
    x2=0; y2=0; z2=-z_shadow;
    Mat4x4MulXYZ(e->w2s,&x2,&y2,&z2);
    zc+=Round(Sqrt(x2*x2+y2*y2+z2*z2))*SignI64(z2);
  }

  x=xc-x;
  y=yc-y;
  z=zc-z;
  Mat4x4MulXYZ(e->s2w,&x,&y,&z);
  x=RoundI64(x,e->cur_snap);
  y=RoundI64(y,e->cur_snap);
  z=RoundI64(e->ms_z,e->cur_snap);
  *_x=x; *_y=y; *_z=z;
}

CMeshEdVertex	sys_clip_vertex_head;
CMeshEdTri	sys_clip_tri_head;

U0 MeshClipInit()
{
  QueInit(&sys_clip_vertex_head);
  QueInit(&sys_clip_tri_head);
}

U0 MeshClipRst()
{
  QueDel(&sys_clip_vertex_head,TRUE);
  QueDel(&sys_clip_tri_head,TRUE);
  MeshClipInit;
}

U0 MeshClipCopy(CMeshFrame *e)
{
  CMeshEdVertex	*tmpv=e->vertex_head.next,*tmpv2;
  CMeshEdTri	*tmpt=e->tri_head.next,*tmpt2;

  MeshClipRst;
  while (tmpv!=&e->vertex_head) {
    if (tmpv->flags&VF_SEL) {
      tmpv->copy=tmpv2=ACAlloc(sizeof(CMeshEdVertex));
      MemCpy(&tmpv2->p,&tmpv->p,sizeof(CD3I32));
      QueIns(tmpv2,sys_clip_vertex_head.last);
      tmpv->flags|=VF_COPIED;
      tmpv->flags&=~VF_SEL;
    } else {
      tmpv->copy=NULL;
      tmpv->flags&=~(VF_COPIED|VF_SEL);
    }
    tmpv=tmpv->next;
  }
  while (tmpt!=&e->tri_head) {
    if (tmpt->flags&TF_SEL &&
	  tmpt->t[0]->copy && tmpt->t[1]->copy && tmpt->t[2]->copy) {
      tmpt2=ACAlloc(sizeof(CMeshEdTri));
      tmpt2->t[0]=tmpt->t[0]->copy;
      tmpt2->t[1]=tmpt->t[1]->copy;
      tmpt2->t[2]=tmpt->t[2]->copy;
      tmpt2->mt.color=tmpt->mt.color;
      QueIns(tmpt2,sys_clip_tri_head.last);
      tmpt->flags|=TF_COPIED;
      tmpt->flags&=~TF_SEL;
    } else
      tmpt->flags&=~(TF_COPIED|TF_SEL);
    tmpt=tmpt->next;
  }
}

U0 MeshClipCut(CMeshFrame *e)
{
  CMeshEdVertex	*tmpv=e->vertex_head.next,*tmpv1;
  CMeshEdTri	*tmpt=e->tri_head.next,*tmpt1;
  MeshClipCopy(e);
  while (tmpt!=&e->tri_head) {
    tmpt1=tmpt->next;
    if (tmpt->flags&TF_COPIED)
      MeshTriDel(e,tmpt);
    tmpt=tmpt1;
  }
  while (tmpv!=&e->vertex_head) {
    tmpv1=tmpv->next;
    if (tmpv->flags&VF_COPIED)
      MeshVertexDel(e,tmpv);
    tmpv=tmpv1;
  }
}

U0 MeshClipDel(CMeshFrame *e)
{//Technically not clip
  CMeshEdVertex	*tmpv=e->vertex_head.next,*tmpv1;
  CMeshEdTri	*tmpt=e->tri_head.next,*tmpt1;
  while (tmpt!=&e->tri_head) {
    tmpt1=tmpt->next;
    if (tmpt->flags&TF_SEL)
      MeshTriDel(e,tmpt);
    tmpt=tmpt1;
  }
  while (tmpv!=&e->vertex_head) {
    tmpv1=tmpv->next;
    if (tmpv->flags&VF_SEL)
      MeshVertexDel(e,tmpv);
    tmpv=tmpv1;
  }
}

U0 MeshClipPaste(CMeshFrame *e)
{
  CMeshEdVertex	*tmpv2=sys_clip_vertex_head.next,*tmpv;
  CMeshEdTri	*tmpt2=sys_clip_tri_head.next,*tmpt;

  MeshVertexSelAll(e,FALSE);
  MeshTriSelAll(e,FALSE);
  while (tmpv2!=&sys_clip_vertex_head) {
    tmpv2->copy=tmpv=CAlloc(sizeof(CMeshEdVertex));
    MemCpy(&tmpv->p,&tmpv2->p,sizeof(CD3I32));
    QueIns(tmpv,e->vertex_head.last);
    tmpv->flags|=VF_SEL;
    tmpv2=tmpv2->next;
  }

  while (tmpt2!=&sys_clip_tri_head) {
    tmpt=MeshTriNew(e,tmpt2->mt.color,tmpt2->t[0]->copy,
	  tmpt2->t[1]->copy,tmpt2->t[2]->copy);
    tmpt->flags|=TF_SEL;
    tmpt2=tmpt2->next;
  }
}

MeshClipInit;

U0 DrawIt(CTask *task,CDC *dc)
{
  CMeshFrame *e=FramePtr("CMeshFrame",task);
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  F64 d;
  I64 i,x,y,z;
  CMeshEdVertex *tmpv;

  task->horz_scroll.min=-(MESH_WORKSPACE_SIZE-task->pix_width)/2;
  task->horz_scroll.max= (MESH_WORKSPACE_SIZE-task->pix_width)/2;
  task->vert_scroll.min=-(MESH_WORKSPACE_SIZE-task->pix_height)/2;
  task->vert_scroll.max= (MESH_WORKSPACE_SIZE-task->pix_height)/2;
  TaskDerivedValsUpdate(task);
  MeshRecalcCxCy(task,e);

  dc->flags|=DCF_TRANSFORMATION;

  Free(dc->r); //Set rotmat doesn't free old dc->r matrix.
  DCMat4x4Set(dc,MeshSetW2S(e,task));

  dc->x=e->cx;
  dc->y=e->cy;
//z-vals less than zero are in front of scrn and not drawn.
  //we want to shift all Z-vals into a drawable range.
  //GR_Z_ALL is set to half of the Z-range which is an I32.
  dc->z=GR_Z_ALL;

  //Light source set to mouse.
  MeshCursorW(e,task,&x,&y,&z);
  dc->ls.x=x;
  dc->ls.y=y;
  dc->ls.z=z;
  d=1<<16/D3I32Norm(&dc->ls); //Light source normalized to 65536.
  dc->ls.x*=d;
  dc->ls.y*=d;
  dc->ls.z*=d;

  DCDepthBufAlloc(dc);

  tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    x=tmpv->p.x; y=tmpv->p.y; z=tmpv->p.z;
    (*dc->transform)(dc,&x,&y,&z);
    tmpv->pt.x=x; tmpv->pt.y=y; tmpv->pt.z=z;
    tmpv=tmpv->next;
  }

  e->mp_not_done_flags=1<<mp_cnt-1; //Issue jobs to all cores.
  e->dc=dc;
  e->depth_buf=dc->depth_buf;
  for (i=0;i<mp_cnt;i++)
    JobQue(&MPDrawIt,e,i);

  tmpv=e->vertex_head.next;
  while (tmpv!=&e->vertex_head) {
    x=tmpv->pt.x; y=tmpv->pt.y; z=tmpv->pt.z;
    if (e->vertex_on) {
      if (Blink(10)) //This blinks at 10 Hz.
	dc->color=ROPF_DITHER+BLACK<<16+WHITE;
      else
	dc->color=ROPF_DITHER+WHITE<<16+BLACK;
      GrLine(dc,x-3,y-3,x+3,y+3);
      GrLine(dc,x-3,y+3,x+3,y-3);
    }
    if (tmpv->flags&VF_SEL) {
      if (e->ed_mode=='t') {
	if (Blink(10)) //This blinks at 10 Hz.
	  dc->color=ROPF_DITHER+e->cur_color.c0.color<<16+
		e->cur_color.c0.color^8;
	else
	  dc->color=ROPF_DITHER+(e->cur_color.c0.color^8)<<16+
		e->cur_color.c0.color;
      } else {
	if (Blink)
	  dc->color=ROPF_DITHER+RED<<16+WHITE;
	else
	  dc->color=ROPF_DITHER+WHITE<<16+RED;
      }
      GrCircle(dc,x,y,3);
    }
    tmpv=tmpv->next;
  }

  if (CtrlInside(c,ms.presnap.x,ms.presnap.y)||winmgr.show_menu) {
    GridInit;
    task->win_inhibit=WIF_SELF_DOC;
  } else {
    MeshCurSnap(e);
    task->win_inhibit=WIG_TASK_DFT|WIF_SELF_DOC-WIF_SELF_FOCUS-WIF_SELF_BORDER
	  -WIF_SELF_CTRLS-WIF_FOCUS_TASK_MENU-WIF_SELF_GRAB_SCROLL;
  }

  MeshCursorW(e,task,&x,&y,&z);
  if (z<0)
    dc->color=ROP_XOR+RED^TRANSPARENT;
  else
    dc->color=ROP_XOR+TRANSPARENT;
  GrPrint(dc,0,0,"%6.3f%% (%d,%d,%d)",e->view_scale*100,x,y,z);
  dc->thick=1;
  dc->color&=0xF;
  if (Blink(10))
    dc->color^=0xF;
  GrLine3(dc,x,y,z,x,y,0);

  if (e->sel_rect) {
    dc->flags&=~DCF_TRANSFORMATION;
    dc->thick=1;
    if (Blink)
      dc->color=ROPF_DITHER+RED<<16+WHITE;
    else
      dc->color=ROPF_DITHER+WHITE<<16+RED;
    GrBorder(dc,e->x1,e->y1,e->x2,e->y2);
  }
//Wait for all cores to complete.
  while (e->mp_not_done_flags)
    Yield;
}

U0 MeshInit(CMeshFrame *e,Bool flip_y)
{
  MemSet(e,0,sizeof(CMeshFrame));
  QueInit(&e->vertex_head);
  QueInit(&e->tri_head);
  e->ed_mode='v';
  e->grid_on=TRUE;
  e->vertex_on=TRUE;
  e->ms_z=0;
  e->thickness=25;
  e->closed=TRUE;
  e->view_scale=1.0;
  e->w2s=NULL;
  e->s2w=NULL;
  e->cur_color=RED;
  e->cur_snap=5;
  e->flip_y=flip_y;
  e->sel_rect=FALSE;
  e->cur_tri=NULL;
  e->cur_vertex=NULL;
  e->chain_pred=NULL;
}

U0 MeshLoad(CMeshFrame *e,U8 *src)
{
  I64 i,j,x,y,z;
  CColorROPU32 color;
  CMeshEdVertex *tmpv,*va[3];

  QueInit(&e->vertex_head);
  QueInit(&e->tri_head);

  e->vertex_cnt  =*src(I32 *)++;
  e->tri_cnt=*src(I32 *)++;
  for (i=0;i<e->vertex_cnt;i++) {
    x=*src(I32 *)++;
    y=*src(I32 *)++;
    z=*src(I32 *)++;
    tmpv=MeshVertexNew(e,x,y,z);
    tmpv->num=i;
  }
  for (i=0;i<e->tri_cnt;i++) {
    color=*src(I32 *)++;
    for (j=0;j<3;j++)
      va[j]=MeshVertexFindNum(e,*src(I32 *)++);
    MeshTriNew(e,color,va[0],va[1],va[2]);
  }
}

I64 MeshSize(CMeshFrame *e)
{
  I64 i;
  CMeshEdVertex	*tmpv=e->vertex_head.next;
  CMeshEdTri	*tmpt=e->tri_head.next;

  e->vertex_cnt=0;
  while (tmpv!=&e->vertex_head) {
    tmpv->num=e->vertex_cnt++;
    tmpv=tmpv->next;
  }

  e->tri_cnt=0;
  while (tmpt!=&e->tri_head) {
    e->tri_cnt++;
    for (i=0;i<3;i++)
      tmpt->mt.nums[i]=tmpt->t[i]->num;
    tmpt=tmpt->next;
  }
  return sizeof(I32)*2+
	(offset(CMeshEdVertex.end)-offset(CMeshEdVertex.start))*e->vertex_cnt+
	(offset(CMeshEdTri.end)-offset(CMeshEdTri.start))*e->tri_cnt;
}

I32 *MeshSave(CMeshFrame *e,I64 *_size=NULL)
{
  I64 size=MeshSize(e);
  U8 *res=MAlloc(size),*dst=res;
  CMeshEdVertex	*tmpv=e->vertex_head.next;
  CMeshEdTri	*tmpt=e->tri_head.next;

  *dst(I32 *)++=e->vertex_cnt;
  *dst(I32 *)++=e->tri_cnt;

  e->vertex_cnt=0;
  while (tmpv!=&e->vertex_head) {
    MemCpy(dst,&tmpv->start,offset(CMeshEdVertex.end)
    -offset(CMeshEdVertex.start));
    dst+=offset(CMeshEdVertex.end)-offset(CMeshEdVertex.start);
    tmpv=tmpv->next;
  }

  e->tri_cnt=0;
  while (tmpt!=&e->tri_head) {
    MemCpy(dst,&tmpt->start,offset(CMeshEdTri.end)-offset(CMeshEdTri.start));
    dst+=offset(CMeshEdTri.end)-offset(CMeshEdTri.start);
    tmpt=tmpt->next;
  }
  if (_size) *_size=size;
  return res;
}

U0 MeshCleanUp(CMeshFrame *e)
{
  QueDel(&e->vertex_head,TRUE);
  QueDel(&e->tri_head,TRUE);
  Free(e->w2s);
  Free(e->s2w);
}

U0 MeshUpdateMenu(CMeshFrame *e)
{
  CMenuEntry *tmpse;
  if (tmpse=MenuEntryFind(Fs->cur_menu,"View/Grid"))
    tmpse->checked=ToBool(e->grid_on);
  if (tmpse=MenuEntryFind(Fs->cur_menu,"View/Vertex"))
    tmpse->checked=ToBool(e->vertex_on);
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/PlaceVertex"))
    tmpse->checked=ToBool(e->ed_mode=='v');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/MoveVertex"))
    tmpse->checked=ToBool(e->ed_mode=='m');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/MoveVertexZ"))
    tmpse->checked=ToBool(e->ed_mode=='M');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/Triangle"))
    tmpse->checked=ToBool(e->ed_mode=='t');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/Polygon"))
    tmpse->checked=ToBool(e->ed_mode=='n');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/Fence"))
    tmpse->checked=ToBool(e->ed_mode=='f');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"Mode/Prism"))
    tmpse->checked=ToBool(e->ed_mode=='p');
  if (tmpse=MenuEntryFind(Fs->cur_menu,"View/FlipY"))
    tmpse->checked=ToBool(e->flip_y);
}

I32 *SpriteMeshEd(I32 *head=NULL,I64 *_size=NULL,Bool flip_y=FALSE)
{/*Fmt for mesh:
{
I32 vertex_cnt;
I32 tri_cnt;
CD3I32 vertices[];
CMeshTri tris[];
}

If head points to a mesh, it will load it.

Returns a newly malloced mesh or NULL.

See $LK,"::/Demo/Graphics/SpritePlot3D.HC"$.
$WW,0$*/
  CCtrl *c=CtrlFindUnique(Fs,CTRLT_VIEWING_ANGLES);
  CViewAngles *s,*old_s;
  I64 i,msg_code,sel_mode,arg1,arg2,make_tri_vertex_num=0,x,y,z;
  CD3I32 p0a,p0b;
  CMeshEdVertex *va[3],*tmpv;
  Bool adjusting_z=FALSE,moving,save_and_exit;
  CMeshFrame e;

  if (c) {
    old_s=MAlloc(sizeof(CViewAngles));
    MemCpy(old_s,c->state,sizeof(CViewAngles));
  } else {
    c=ViewAnglesNew;
    old_s=NULL;
  }

  s=c->state;
  s->sx=0;
  s->sy=0;
  s->sz=0;
  s->cx=YELLOW;
  s->cy=RED;
  s->cz=GREEN;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  Delete(,,SC_DELETE);"
	"  DelLast(,CH_BACKSPACE);"
	"  Cut(,CH_CTRLX);"
	"  Copy(,CH_CTRLC);"
	"  Paste(,CH_CTRLV);"
	"  SelectAll(,'A');"
	"  UnSelectAll(,'U');"
	"  SelectRect(,'a');"
	"  UnSelectRect(,'u');"
	"  OrSelectRect(,'o');"
	"  JumpToZ(,'j');"
	"  ResetColor(,'C');"
	"  ReverseTri(,'r');"
	"}"
	"Mode {"
	"  PlaceVertex(,'v');"
	"  MoveVertex(,'m');"
	"  MoveVertexZ(,'M');"
	"  Triangle(,'t');"
	"  Polygon(,'n');"
	"  Fence(,'f');"
	"  Prism(,'p');"
	"}"
	"Settings {"
	"  Color(,'c');"
	"  Snap(,'s');"
	"}"
	"View {"
	"  ZoomIn(,'z');"
	"  ZoomOut(,'Z');"
	"  NullAngles(,'N');"
	"  FlipY(,'y');"
	"  Grid(,'g');"
	"  Vertex(,'V');"
	"  ToggleBorder(,CH_CTRLB);"
	"}"
	"Transforms {"
	"  Transform(,'T');"
	"  SwapXY(,'1');"
	"  SwapXZ(,'2');"
	"  SwapYZ(,'3');"
	"  InvertX(,'4');"
	"  InvertY(,'5');"
	"  InvertZ(,'6');"
	"  ReverseTri(,'R');"
	"}");

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  RegOneTimePopUp(ARf_MESH_ED,
	"$$GREEN$$Right Mouse$$FG$$: Hold and move to shift cursor z\n"
	"$$GREEN$$'j'$$FG$$: Jump cursor Z to nearest vertex's Z\n"
	"$$GREEN$$'v'$$FG$$: Place Vertex Mode\n"
	"$$GREEN$$'m'$$FG$$: Move Vertex Mode\n"
	"$$GREEN$$'M'$$FG$$: Move Vertex Z\n"
	"$$GREEN$$'t'$$FG$$: Form Triangle Mode\n"
	"$$GREEN$$'n'$$FG$$: Polygon Mode\n"
	"$$GREEN$$'f'$$FG$$: Fence Mode\n"
	"$$GREEN$$'p'$$FG$$: Prism Mode\n"
	"$$GREEN$$'c'$$FG$$: Set color\n"
	"$$GREEN$$'s'$$FG$$: Set snap\n"
	"\nSee menu at top of scrn for more.\n");

  Fs->win_inhibit=WIG_TASK_DFT|WIF_SELF_DOC-WIF_SELF_FOCUS-WIF_SELF_BORDER
	-WIF_SELF_CTRLS-WIF_FOCUS_TASK_MENU-WIF_SELF_GRAB_SCROLL;
  Fs->horz_scroll.pos=0;
  Fs->vert_scroll.pos=0;
  MeshInit(&e,flip_y);
  if (head)
    MeshLoad(&e,head);
  FramePtrAdd("CMeshFrame",&e);
  Fs->draw_it=&DrawIt;
  MeshCurSnap(&e);
  MeshRecalcCxCy(Fs,&e);

  try {//In case of <CTRL-ALT-c>
    while (TRUE) {
      MeshUpdateMenu(&e);
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_MS_MOVE|1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|
	    1<<MSG_MS_L_UP|1<<MSG_MS_R_DOWN|1<<MSG_MS_R_UP);
me_restart:
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case 0:
	      switch (arg2.u8[0]) {
		case SC_DELETE:
		  if (arg2&SCF_SHIFT)
		    goto me_clip_cut;
		  else {
		    if (MeshSelCnt(&e))
		      MeshClipDel(&e);
		    else if (e.ed_mode!='t')
		      MeshVertexDel(&e,MeshVertexFindScrPt(&e,
			    ms.presnap.x-Fs->pix_left-Fs->scroll_x,
			    ms.presnap.y-Fs->pix_top-Fs->scroll_y));
		    MeshVertexSelAll(&e,FALSE);
		    MeshTriSelAll(&e,FALSE);
		    make_tri_vertex_num=0;
		  }
		  break;
		case SC_INS:
		  if (arg2&SCF_CTRL)
		    goto me_clip_copy;
		  else if (arg2&SCF_SHIFT)
		    goto me_clip_paste;
	      }
	      break;
	    case CH_BACKSPACE:
	      switch (e.ed_mode) {
		case 'n':
		case 'f':
		case 'p':
		case 'v':
		  MeshVertexDel(&e,e.cur_vertex);
		  break;
		case 't':
		  if (make_tri_vertex_num) {
		    MeshVertexSelAll(&e,FALSE);
		    MeshTriSelAll(&e,FALSE);
		    make_tri_vertex_num=0;
		  } else
		    MeshTriDel(&e,e.cur_tri);
		  break;
	      }
	      break;
	    case 'f':
	    case 'p':
	      e.thickness=PopUpGetI64("Thickness (%d):",e.thickness);
	    case 'n':
	      if (arg1=='n' || arg1=='p')
		e.closed=TRUE;
	      else
		e.closed=PopUpNoYes("Closed?\n");
me_chain:
	      e.chain_pred=e.vertex_head.last;
	    case 't':
	      MeshVertexSelAll(&e,FALSE);
	      MeshTriSelAll(&e,FALSE);
	    case 'v':
	    case 'm':
	    case 'M':
	      adjusting_z=FALSE;
	      moving=FALSE;
	      e.ed_mode=arg1;
	      make_tri_vertex_num=0;
	      Snd;
	      break;
	    case 'T':
	      MeshTransformSel(&e);
	      break;
	    case 'A':
	      MeshTriSelAll(&e,TRUE);
	      if (e.ed_mode!='t')
		MeshVertexSelAll(&e,TRUE);
	      else
		MeshVertexSelAll(&e,FALSE);
	      make_tri_vertex_num=0;
	      break;
	    case 'U':
	      MeshTriSelAll(&e,FALSE);
	      MeshVertexSelAll(&e,FALSE);
	      make_tri_vertex_num=0;
	      break;
	    case 'a':
	    case 'u':
	    case 'o':
	      if (arg1=='a')
		sel_mode=SEL_MESH_EQU;
	      else if (arg1=='u')
		sel_mode=SEL_MESH_AND;
	      else
		sel_mode=SEL_MESH_OR;
	      if ((msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|
		    1<<MSG_MS_L_UP|1<<MSG_MS_R_DOWN|1<<MSG_MS_R_UP))
		    !=MSG_MS_L_DOWN) {
		Beep; Beep;
		goto me_restart;
	      }
	      e.x1=arg1; e.y1=arg2;
	      e.x2=arg1; e.y2=arg2;
	      e.sel_rect=TRUE;
	      while (TRUE) {
		msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_MOVE|1<<MSG_KEY_DOWN|
		      1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|1<<MSG_MS_R_DOWN|
		      1<<MSG_MS_R_UP);
		if (msg_code==MSG_MS_MOVE) {
		  e.x2=arg1; e.y2=arg2;
		} else if (msg_code==MSG_MS_L_UP) {
		  e.x2=arg1; e.y2=arg2;
		  break;
		} else {
		  e.sel_rect=FALSE;
		  Beep; Beep;
		  goto me_restart;
		}
	      }
	      e.sel_rect=FALSE;
	      MeshTriSelRect(&e,sel_mode,e.x1,e.x2,e.y1,e.y2);
	      if (e.ed_mode!='t')
		MeshVertexSelRect(&e,sel_mode,e.x1,e.x2,e.y1,e.y2);
	      else
		MeshVertexSelAll(&e,FALSE);
	      make_tri_vertex_num=0;
	      break;
	    case CH_CTRLB:
	      WinBorder(Bt(&Fs->display_flags,DISPLAYf_NO_BORDER));
	      break;
	    case CH_CTRLC:
me_clip_copy:
	      if (e.ed_mode=='t') {
		Beep;Beep;
	      } else
		MeshClipCopy(&e);
	      break;
	    case CH_CTRLV:
me_clip_paste:
	      if (e.ed_mode=='t') {
		Beep;Beep;
	      } else {
		MeshClipPaste(&e);
		e.ed_mode='m';
	      }
	      break;
	    case CH_CTRLX:
me_clip_cut:
	      if (e.ed_mode=='t') {
		Beep;Beep;
	      } else
		MeshClipCut(&e);
	      break;
	    case CH_SHIFT_ESC:
	      save_and_exit=FALSE;
	      goto me_done;
	    case CH_ESC:
	      save_and_exit=TRUE;
	      goto me_done;
	    case 'z':
	      MeshScaleZoom(&e,1.5);
	      break;
	    case 'Z':
	      MeshScaleZoom(&e,1/1.5);
	      break;
	    case 'c':
	      e.cur_color=PopUpColorLighting;
	      break;
	    case 's':
	      i=PopUpRangeI64(1,25,1,"New Snap\n");
	      if (i>=1)
		e.cur_snap=i;
	      MeshCurSnap(&e);
	      MeshRecalcCxCy(Fs,&e);
	      break;
	    case 'g':
	      e.grid_on=!e.grid_on;
	      break;
	    case 'V':
	      e.vertex_on=!e.vertex_on;
	      break;
	    case 'N':
	      s->sx=s->sy=s->sz=0;
	      break;
	    case 'y':
	      e.flip_y=!e.flip_y;
	      break;
	    case 'j':
	      if (moving)
		MeshVertexIgnoreSet(&e,TRUE);
	      if (tmpv=MeshVertexFindScrPt(&e,
		    ms.pos.x-Fs->pix_left-Fs->scroll_x,
		    ms.pos.y-Fs->pix_top-Fs->scroll_y)) {
		Noise(25,86,110);
		e.ms_z=RoundI64(tmpv->p.z,e.cur_snap);
	      } else {
		Beep; Beep;
		e.ms_z=0;
	      }
	      MeshVertexIgnoreSet(&e,FALSE);
	      if (moving) {
		MeshCursorW(&e,Fs,&x,&y,&z);
		if (adjusting_z)
		  MeshP0Offset(&e,0,0,z-p0a.z);
		else
		  MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
		p0a.x=x;
		p0a.y=y;
		p0a.z=z;
		MeshP0Capture(&e);
	      }
	      break;
	    case '1':
	      MeshSwapAxes(&e,offset(CD3I32.x),offset(CD3I32.y));
	      break;
	    case '2':
	      MeshSwapAxes(&e,offset(CD3I32.x),offset(CD3I32.z));
	      break;
	    case '3':
	      MeshSwapAxes(&e,offset(CD3I32.y),offset(CD3I32.z));
	      break;
	    case '4':
	      MeshInvertAxis(&e,offset(CD3I32.x));
	      break;
	    case '5':
	      MeshInvertAxis(&e,offset(CD3I32.y));
	      break;
	    case '6':
	      MeshInvertAxis(&e,offset(CD3I32.z));
	      break;
	    case 'r':
	      if (e.cur_tri)
		SwapI64(&e.cur_tri->t[1],&e.cur_tri->t[2]);
	      break;
	    case 'C':
	      MeshColorTris(&e);
	      break;
	    case 'R':
	      MeshRevTris(&e);
	      break;
	  }
	  break;
	case MSG_MS_L_DOWN:
	  switch (e.ed_mode) {
	    case 'm':
	      if (!moving) {
		if (!MeshSelCnt(&e) &&
		      (tmpv=MeshVertexFindScrPt(&e,arg1,arg2))) {
		  tmpv->flags|=VF_SEL;
		  e.ms_z=RoundI64(tmpv->p.z,e.cur_snap);
		}
		if (MeshSelCnt(&e)) {
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  p0a.x=x;
		  p0a.y=y;
		  p0a.z=z;
		  MeshP0Capture(&e);
		  moving=TRUE;
		}
	      }
	      break;
	    case 'M':
	      if (!adjusting_z && !moving) {
		if (!MeshSelCnt(&e) &&
		      (tmpv=MeshVertexFindScrPt(&e,arg1,arg2))) {
		  tmpv->flags|=VF_SEL;
		  e.ms_z=RoundI64(tmpv->p.z,e.cur_snap);
		}
		if (MeshSelCnt(&e)) {
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  p0a.x=x;
		  p0a.y=y;
		  p0a.z=z;
		  MeshP0Capture(&e);
		  moving=TRUE;

		  p0b.x=ms.presnap.x;
		  p0b.y=ms.presnap.y;
		  p0b.z=e.ms_z;
		  adjusting_z=TRUE;
		  Snd(ClampI64(Freq2Ona(3*e.ms_z+1500),0,I8_MAX));
		}
	      }
	      break;
	  }
	  break;
	case MSG_MS_L_UP:
	  switch (e.ed_mode) {
	    case 'n':
	    case 'f':
	    case 'p':
	    case 'v':
	      Noise(25,86,110);
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      e.cur_vertex=MeshVertexNew(&e,x,y,z);
	      break;
	    case 'm':
	    case 'M':
	      if (moving) {
		if (adjusting_z) {
		  e.ms_z=RoundI64(Sign(p0b.y-ms.presnap.y)
		  *Sqrt(Sqr(p0b.x-ms.presnap.x)+Sqr(p0b.y-ms.presnap.y))
		  +p0b.z,e.cur_snap);
		  Snd;
		  adjusting_z=FALSE;
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  MeshP0Offset(&e,0,0,z-p0a.z);
		} else {
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
		}
		MeshTriSelAll(&e,FALSE);
		MeshVertexSelAll(&e,FALSE);
		moving=FALSE;
	      }
	      break;
	    case 't':
	      if (tmpv=MeshVertexFindScrPt(&e,arg1,arg2)) {
		for (i=0;i<make_tri_vertex_num;i++)
		  if (va[i]==tmpv) {
		    Beep; Beep;
		    break;
		  }
		if (i==make_tri_vertex_num) {
		  Noise(25,86,110);
		  va[make_tri_vertex_num++]=tmpv;
		  tmpv->flags|=VF_SEL;
		  if (make_tri_vertex_num==3) {
		    e.cur_tri=MeshTriNew(&e,e.cur_color,va[0],va[1],va[2]);
		    for (i=0;i<make_tri_vertex_num;i++)
		      va[i]->flags&=~VF_SEL;
		    make_tri_vertex_num=0;
		  }
		}
	      }
	      break;
	  }
	  break;
	case MSG_MS_R_DOWN:
	  if (!adjusting_z && e.ed_mode!='M' &&
		(e.chain_pred==e.vertex_head.last ||
		e.ed_mode!='n' && e.ed_mode!='f' && e.ed_mode!='p')) {
	    if (moving) {
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      MeshP0Capture(&e);
	    }
	    p0b.x=ms.presnap.x;
	    p0b.y=ms.presnap.y;
	    p0b.z=e.ms_z;
	    adjusting_z=TRUE;
	    Snd(ClampI64(Freq2Ona(3*e.ms_z+1500),0,I8_MAX));
	  }
	  break;
	case MSG_MS_R_UP:
	  if (adjusting_z) {
	    e.ms_z=RoundI64(Sign(p0b.y-ms.presnap.y)
	    *Sqrt(Sqr(p0b.x-ms.presnap.x)+Sqr(p0b.y-ms.presnap.y))
	    +p0b.z,e.cur_snap);
	    Snd;
	    adjusting_z=FALSE;
	    if (moving) {
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      MeshP0Offset(&e,0,0,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      MeshP0Capture(&e);
	    }
	  } else  if (e.ed_mode=='n') {
	    if (e.chain_pred && e.chain_pred!=e.vertex_head.last)
	      MeshPolygon(&e,e.chain_pred->next,e.vertex_head.last,FALSE);
	    arg1=e.ed_mode;
	    goto me_chain;
	  } else if (e.ed_mode=='f') {
	    if (e.chain_pred && e.chain_pred!=e.vertex_head.last)
	      MeshFence(&e);
	    arg1=e.ed_mode;
	    goto me_chain;
	  } else if (e.ed_mode=='p') {
	    if (e.chain_pred && e.chain_pred!=e.vertex_head.last)
	      MeshPrism(&e);
	    arg1=e.ed_mode;
	    goto me_chain;
	  }
	  break;
	case MSG_MS_MOVE:
	  if (adjusting_z) {
	    e.ms_z=RoundI64(Sign(p0b.y-ms.presnap.y)
	    *Sqrt(Sqr(p0b.x-ms.presnap.x)+Sqr(p0b.y-ms.presnap.y))
	    +p0b.z,e.cur_snap);
	    Snd(ClampI64(Freq2Ona(3*e.ms_z+1500),0,I8_MAX));
	  }
	  if (moving) {
	    MeshCursorW(&e,Fs,&x,&y,&z);
	    if (adjusting_z)
	      MeshP0Offset(&e,0,0,z-p0a.z);
	    else
	      MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
	    p0a.x=x;
	    p0a.y=y;
	    p0a.z=z;
	    MeshP0Capture(&e);
	  }
	  break;
      }
    }
me_done:
  } catch
    Fs->catch_except=TRUE;
  SettingsPop;
  MenuPop;
  if (save_and_exit)
    head=MeshSave(&e,_size);
  else
    head=NULL;
  MeshCleanUp(&e);
  FramePtrDel("CMeshFrame");
  if (old_s) {
    MemCpy(c->state,old_s,sizeof(CViewAngles));
    Free(old_s);
  } else
    ViewAnglesDel;
  return head;
}
