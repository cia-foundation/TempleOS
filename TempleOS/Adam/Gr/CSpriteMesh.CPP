#help_index "Graphics/Mesh"
#define GE_WORKSPACE_SIZE	4000

#define VF_SELECTED	1
#define VF_COPIED	2
#define VF_IGNORE	4

class CGrMVertex
{
  CGrMVertex *next,*last,*copy;

  U0 start;
  CD3I32 p;  //This is the world coordinates of the point.
  U0 end;
  CD3I32 p0,
       pt; //This is the transformed coordinates.  (Screen)
  I32 num,flags;
};

#define TF_SELECTED	1
#define TF_COPIED	2

class CGrMTri
{
  CGrMTri *next,*last;

  U0 start;
  CMeshTri mt;
  U0 end;

  I32 cpu_num,flags; //Draw different tris with different cores.
  CGrMVertex *t[3];
};


class CGrMFrame
{
  I64 my_ipz; //This is the Z-coordinate of the input pointer (mouse).
  CTask *task;
  I64 mode,cx,cy;
  CColorROPU32 cur_color;
  Bool grid_on,flip_y,select_rect,vertex_on,pad[4];
  I64 mp_not_done_flags; //Used for multiprocessing signaling.
  F64 view_scale;
  CDC *dc;
  I64 *w2s,*s2w; //Screen-to-world and world-to-screen transform matrices.
  I64 vertex_cnt,tri_cnt; //Set by GrMSize
  CGrMVertex vertex_root,*cur_vertex;
  CGrMTri    tri_root,*cur_tri;
  I64 x1,y1,x2,y2,cur_snap;
};

CGrMVertex *GrMVertexNew(CGrMFrame *e,I64 x,I64 y,I64 z)
{
  CGrMVertex *tempv=CAlloc(sizeof(CGrMVertex));
  tempv->p.x=x;
  tempv->p.y=y;
  tempv->p.z=z;
  QueIns(tempv,e->vertex_root.last);
  return tempv;
}

CGrMTri *GrMTriNew(CGrMFrame *e,CColorROPU32 color,CGrMVertex *v1,CGrMVertex *v2,CGrMVertex *v3)
{
  static I64 cpu_num=0;
  CGrMTri *tempt=CAlloc(sizeof(CGrMTri));
  tempt->cpu_num=cpu_num++%mp_cnt;
  tempt->mt.color=color;
  tempt->t[0]=v1;
  tempt->t[1]=v2;
  tempt->t[2]=v3;
  QueIns(tempt,e->tri_root.last);
  return tempt;
}

CGrMVertex *GrMVertexFindS(CGrMFrame *e,I64 x,I64 y)
{//Screen coordinates
  CGrMVertex *result=NULL,*tempv=e->vertex_root.next;
  I64 dd,dz,best_dd=MAX_I64,best_dz=MAX_I64;
  while (tempv!=&e->vertex_root) {
    if (!(tempv->flags&VF_IGNORE)) {
      dd=SqrI64(x-tempv->pt.x)+SqrI64(y-tempv->pt.y);
      dz=AbsI64(e->my_ipz-tempv->p.z);
      if (dd<best_dd || dd==best_dd && dz<best_dz) {
	result=tempv;
	best_dd=dd;
	best_dz=dz;
      }
    }
    tempv=tempv->next;
  }
  return result;
}

CGrMVertex *GrMVertexFindNum(CGrMFrame *e,I64 num)
{
  CGrMVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    if (tempv->num==num)
      return tempv;
    tempv=tempv->next;
  }
  return NULL;
}

U0 GrMTriDel(CGrMFrame *e,CGrMTri *tempt)
{
  if (tempt) {
    if (tempt==e->cur_tri)
      e->cur_tri=NULL;
    QueRem(tempt);
    Free(tempt);
  }
}

U0 GrMVertexDel(CGrMFrame *e,CGrMVertex *tempv)
{
  I64 i;
  CGrMTri *tempt,*tempt1;
  if (tempv) {
    tempt=e->tri_root.next;
    while (tempt!=&e->tri_root) {
      tempt1=tempt->next;
      for (i=0;i<3;i++)
	if (tempt->t[i]==tempv)
	  break;
      if (i<3)
	GrMTriDel(e,tempt);
      tempt=tempt1;
    }
    if (tempv==e->cur_vertex)
      e->cur_vertex=NULL;
    QueRem(tempv);
    Free(tempv);
  }
}

U0 GrMVertexSelectAll(CGrMFrame *e,Bool val)
{
  CGrMVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    if (val)
      tempv->flags|=VF_SELECTED;
    else
      tempv->flags&=~VF_SELECTED;
    tempv=tempv->next;
  }
}

U0 GrMTriSelectAll(CGrMFrame *e,Bool val)
{
  CGrMTri *tempt=e->tri_root.next;
  while (tempt!=&e->tri_root) {
    if (val)
      tempt->flags|=TF_SELECTED;
    else
      tempt->flags&=~TF_SELECTED;
    tempt=tempt->next;
  }
}

U0 GrMVertexIgnoreSet(CGrMFrame *e,Bool val)
{
  CGrMVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    tempv->flags&=~VF_IGNORE;
    if (tempv->flags&VF_SELECTED && val)
      tempv->flags|=VF_IGNORE;
    tempv=tempv->next;
  }
}

U0 GrMP0Capture(CGrMFrame *e)
{
  CGrMVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    MemCpy(&tempv->p0,&tempv->p,sizeof(CD3I32));
    tempv=tempv->next;
  }
}

U0 GrMP0Offset(CGrMFrame *e,I64 dx,I64 dy,I64 dz)
{
  CGrMVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED) {
      tempv->p.x=tempv->p0.x+dx;
      tempv->p.y=tempv->p0.y+dy;
      tempv->p.z=tempv->p0.z+dz;
    }
    tempv=tempv->next;
  }
}

#define GRM_EQU	0
#define GRM_OR	1
#define GRM_AND	2
U0 GrMVertexSelectRect(CGrMFrame *e,I64 mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CGrMVertex *tempv=e->vertex_root.next;
  if (x1>x2) SwapU64(&x1,&x2);
  if (y1>y2) SwapU64(&y1,&y2);
  while (tempv!=&e->vertex_root) {
    if (x1<=tempv->pt.x<=x2 &&
	y1<=tempv->pt.y<=y2) {
      if (mode==GRM_AND)
	tempv->flags&=~VF_SELECTED;
      else
	tempv->flags|=VF_SELECTED;
    } else if (mode==GRM_EQU)
      tempv->flags&=~VF_SELECTED;
    tempv=tempv->next;
  }
}

U0 GrMTriSelectRect(CGrMFrame *e,I64 mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CGrMTri *tempt=e->tri_root.next;
  if (x1>x2) SwapU64(&x1,&x2);
  if (y1>y2) SwapU64(&y1,&y2);
  while (tempt!=&e->tri_root) {
    if (x1<=tempt->t[0]->pt.x<=x2 &&
	y1<=tempt->t[0]->pt.y<=y2 &&
	x1<=tempt->t[1]->pt.x<=x2 &&
	y1<=tempt->t[1]->pt.y<=y2 &&
	x1<=tempt->t[2]->pt.x<=x2 &&
	y1<=tempt->t[2]->pt.y<=y2) {
      if (mode==GRM_AND)
	tempt->flags&=~TF_SELECTED;
      else
	tempt->flags|=TF_SELECTED;
    } else {
      if (mode==GRM_EQU)
	tempt->flags&=~TF_SELECTED;
      else if (mode==GRM_AND) {
	if (x1<=tempt->t[0]->pt.x<=x2 &&
	    y1<=tempt->t[0]->pt.y<=y2 ||
	    x1<=tempt->t[1]->pt.x<=x2 &&
	    y1<=tempt->t[1]->pt.y<=y2 ||
	    x1<=tempt->t[2]->pt.x<=x2 &&
	    y1<=tempt->t[2]->pt.y<=y2)
	tempt->flags&=~TF_SELECTED;
      }
    }
    tempt=tempt->next;
  }
}

I64 GrMSelectCnt(CGrMFrame *e)
{
  I64 result=0;
  CGrMVertex *tempv=e->vertex_root.next;
  CGrMTri *tempt=e->tri_root.next;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED)
      result++;
    tempv=tempv->next;
  }
  while (tempt!=&e->tri_root) {
    if (tempt->flags&TF_SELECTED)
      result++;
    tempt=tempt->next;
  }
  return result;
}

U0 GrMSwapAxes(CGrMFrame *e,I64 o1,I64 o2)
{
  Bool unselect;
  CGrMVertex *tempv=e->vertex_root.next;
  if (!GrMSelectCnt(e)) {
    GrMVertexSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED)
      SwapU32((&tempv->p)(U8 *)+o1,(&tempv->p)(U8 *)+o2);
    tempv=tempv->next;
  }
  if (unselect)
    GrMVertexSelectAll(e,FALSE);
}

U0 GrMInvertAxis(CGrMFrame *e,I64 o)
{
  Bool unselect;
  CGrMVertex *tempv=e->vertex_root.next;
  if (!GrMSelectCnt(e)) {
    GrMVertexSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED)
      *((&tempv->p)(U8 *)+o)(I32 *)=-*((&tempv->p)(U8 *)+o)(I32 *);
    tempv=tempv->next;
  }
  if (unselect)
    GrMVertexSelectAll(e,FALSE);
}

U0 GrMTransformSelect(CGrMFrame *e)
{
  Bool unselect;
  I64 r[16],x,y,z;
  CGrMVertex *tempv=e->vertex_root.next;
  if (PopUpTransform(r)) {
    if (!GrMSelectCnt(e)) {
      GrMVertexSelectAll(e,TRUE);
      unselect=TRUE;
    } else
      unselect=FALSE;
    while (tempv!=&e->vertex_root) {
      if (tempv->flags&VF_SELECTED) {
	x=tempv->p.x; y=tempv->p.y; z=tempv->p.z;
	GrRot(r,&x,&y,&z);
	tempv->p.x=x; tempv->p.y=y; tempv->p.z=z;
      }
      tempv=tempv->next;
    }
    if (unselect)
      GrMVertexSelectAll(e,FALSE);
  }
}

U0 GrMFlipTris(CGrMFrame *e)
{
  Bool unselect;
  CGrMTri *tempt=e->tri_root.next;
  if (!GrMSelectCnt(e)) {
    GrMTriSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempt!=&e->tri_root) {
    if (tempt->flags & TF_SELECTED)
      SwapU64(&tempt->t[1],&tempt->t[2]);
    tempt=tempt->next;
  }
  if (unselect)
    GrMTriSelectAll(e,FALSE);
}

U0 GrMRecalcCxCy(CTask *task,CGrMFrame *e)
{
  e->cx=RoundI64(task->win_pixel_width/2 -task->horz_scroll.pos,e->cur_snap);
  e->cy=RoundI64(task->win_pixel_height/2-task->vert_scroll.pos,e->cur_snap);
}

U0 GrMCurSnap(CGrMFrame *e)
{
  CTask *task=e->task;
  I64 x1,y1,z1,x2,y2,z2;
  if (e->w2s) {
    x1=e->cur_snap<<16; y1=0; z1=0;
    GrRot(e->w2s,&x1,&y1,&z1);
    x2=0; y2=e->cur_snap<<16; z2=0;
    GrRot(e->w2s,&x2,&y2,&z2);
    task->snap.x=MaxI64(1,MaxI64(x1,x2)>>16);
    task->snap.y=MaxI64(1,MaxI64(y1,y2)>>16);
    task->snap.z=MinI64(task->snap.x,task->snap.y);
  }
}

U0 GrMScaleZoom(CGrMFrame *e,F64 scale)
{
  CTask *task=Fs;
  I64 x=ipx-task->win_pixel_left-task->win_scroll_x-task->win_pixel_width/2,
     y=ipy-task->win_pixel_top-task->win_scroll_y-task->win_pixel_height/2;
  task->horz_scroll.pos*=scale;
  task->vert_scroll.pos*=scale;
  task->horz_scroll.pos+=scale*x-x;
  task->vert_scroll.pos+=scale*y-y;
  e->view_scale*=scale;
  GrMRecalcCxCy(task,e);
  GrMCurSnap(e);
}

U0 MPDrawIt(CGrMFrame *e) //Multiprocessing draw it, called by each core.
{

//Makes a copy of e->dc so we can change dc->color member and stuff.
  CDC *dc=DCAlias(e->dc,e->dc->win_task);

  CGrMTri *tempt=e->tri_root.next;
  I64 i,*old_r=dc->r;

//$LK,"DCAlias","MN:DCAlias"$() allocates a new identity rotation matrix.
//We want e->dc's rotation matrix.
  dc->r=e->dc->r;

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

  while (tempt!=&e->tri_root) {
    if (tempt->cpu_num==Gs->num) {
      if (tempt->flags & TF_SELECTED) {
	if (Blink)
	  dc->color=ROPF_DITHER+WHITE<<16+RED;
	else
	  dc->color=ROPF_DITHER+RED<<16+WHITE;
	GrFillTri0(dc,&tempt->t[0]->pt,&tempt->t[1]->pt,&tempt->t[2]->pt);
      } else {
	(*dc->lighting)(dc,&tempt->t[0]->pt,&tempt->t[1]->pt,&tempt->t[2]->pt,tempt->mt.color);
	GrFillTri0(dc,&tempt->t[0]->pt,&tempt->t[1]->pt,&tempt->t[2]->pt);
      }
    }
    tempt=tempt->next;
  }
  dc->r=old_r;

//e->dc's depth buf was copied but we don't want it freed during $LK,"DCDel","MN:DCDel"$().
  dc->depth_buf=NULL;

  DCDel(dc);
  LBtr(&e->mp_not_done_flags,Gs->num);
}

I64 *GrMW2S(CGrMFrame *e,CTask *task)
{//World to screen coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=GrIdent(task);
  GrScaleMatEqu(r,e->view_scale);
  GrRotZEqu(r,s->az);
  GrRotYEqu(r,s->ay);
  if (e->flip_y)
    GrRotXEqu(r,s->ax);
  else
    GrRotXEqu(r,s->ax+ã);
  return r;
}

I64 *GrMS2W(CGrMFrame *e,CTask *task)
{//Screen to world coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=GrIdent(task);
  if (e->flip_y)
    GrRotXEqu(r,-s->ax);
  else
    GrRotXEqu(r,-(s->ax+ã));
  GrRotYEqu(r,-s->ay);
  GrRotZEqu(r,-s->az);
  GrScaleMatEqu(r,1/e->view_scale);
  return r;
}

I64 *GrMSetW2S(CGrMFrame *e,CTask *task)
{
  Free(e->w2s);
  e->w2s=GrMW2S(e,task);
  Free(e->s2w);
  e->s2w=GrMS2W(e,task);
  return MAllocIdentical(e->w2s,task); //returned matrix is assigned to dc->r and will be freed by $LK,"DCDel","MN:DCDel"$().
}

U0 GrMCursorW(CGrMFrame *e,CTask *task,I64 *_x,I64 *_y,I64 *_z)
{
  I64 x_shadow,y_shadow,z_shadow,
     xc=ipx-task->win_pixel_left-task->win_scroll_x-e->cx,yc=ipy-task->win_pixel_top-task->win_scroll_y-e->cy,zc=0,
     x=0,y=0,z=e->my_ipz,
     i,x2,y2,z2;
  GrRot(e->w2s,&x,&y,&z); //screen of Z vect

//This converges onto a solution for zc, an unknown.
  for (i=0;i<128;i++) {
    x_shadow=xc-x; //Shadow of mouse cursor on xy plane
    y_shadow=yc-y;
    z_shadow=zc-z;
    GrRot(e->s2w,&x_shadow,&y_shadow,&z_shadow);
    x2=0; y2=0; z2=-z_shadow;
    GrRot(e->w2s,&x2,&y2,&z2);
    zc+=Round(Sqrt(x2*x2+y2*y2+z2*z2))*SignI64(z2);
  }

  x=xc-x;
  y=yc-y;
  z=zc-z;
  GrRot(e->s2w,&x,&y,&z);
  x=RoundI64(x,e->cur_snap);
  y=RoundI64(y,e->cur_snap);
  z=RoundI64(e->my_ipz,e->cur_snap);
  *_x=x; *_y=y; *_z=z;
}


CGrMVertex sys_vertex_root;
CGrMTri	  sys_tri_root;

U0 GrMClipBoardInit()
{
  QueInit(&sys_vertex_root);
  QueInit(&sys_tri_root);
}

U0 GrMClipBoardRst()
{
  QueDel(&sys_vertex_root,TRUE);
  QueDel(&sys_tri_root,TRUE);
  GrMClipBoardInit;
}

U0 GrMClipBoardCopy(CGrMFrame *e)
{
  CGrMVertex *tempv=e->vertex_root.next,*tempv2;
  CGrMTri *tempt=e->tri_root.next,*tempt2;

  GrMClipBoardRst;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED) {
      tempv->copy=tempv2=ACAlloc(sizeof(CGrMVertex));
      MemCpy(&tempv2->p,&tempv->p,sizeof(CD3I32));
      QueIns(tempv2,sys_vertex_root.last);
      tempv->flags|=VF_COPIED;
      tempv->flags&=~VF_SELECTED;
    } else {
      tempv->copy=NULL;
      tempv->flags&=~(VF_COPIED|VF_SELECTED);
    }
    tempv=tempv->next;
  }
  while (tempt!=&e->tri_root) {
    if (tempt->flags&TF_SELECTED &&
	tempt->t[0]->copy && tempt->t[1]->copy && tempt->t[2]->copy) {
      tempt2=ACAlloc(sizeof(CGrMTri));
      tempt2->t[0]=tempt->t[0]->copy;
      tempt2->t[1]=tempt->t[1]->copy;
      tempt2->t[2]=tempt->t[2]->copy;
      tempt2->mt.color=tempt->mt.color;
      QueIns(tempt2,sys_tri_root.last);
      tempt->flags|=TF_COPIED;
      tempt->flags&=~TF_SELECTED;
    } else
      tempt->flags&=~(TF_COPIED|TF_SELECTED);
    tempt=tempt->next;
  }
}

U0 GrMClipBoardCut(CGrMFrame *e)
{
  CGrMVertex *tempv=e->vertex_root.next,*tempv1;
  CGrMTri  *tempt=e->tri_root.next,*tempt1;
  GrMClipBoardCopy(e);
  while (tempt!=&e->tri_root) {
    tempt1=tempt->next;
    if (tempt->flags&TF_COPIED)
      GrMTriDel(e,tempt);
    tempt=tempt1;
  }
  while (tempv!=&e->vertex_root) {
    tempv1=tempv->next;
    if (tempv->flags&VF_COPIED)
      GrMVertexDel(e,tempv);
    tempv=tempv1;
  }
}

U0 GrMClipBoardDel(CGrMFrame *e)
{//Technically not clipboard
  CGrMVertex *tempv=e->vertex_root.next,*tempv1;
  CGrMTri *tempt=e->tri_root.next,*tempt1;
  while (tempt!=&e->tri_root) {
    tempt1=tempt->next;
    if (tempt->flags&TF_SELECTED)
      GrMTriDel(e,tempt);
    tempt=tempt1;
  }
  while (tempv!=&e->vertex_root) {
    tempv1=tempv->next;
    if (tempv->flags&VF_SELECTED)
      GrMVertexDel(e,tempv);
    tempv=tempv1;
  }
}

U0 GrMClipBoardPaste(CGrMFrame *e)
{
  CGrMVertex *tempv2=sys_vertex_root.next,*tempv;
  CGrMTri *tempt2=sys_tri_root.next,*tempt;
 
  GrMVertexSelectAll(e,FALSE);
  GrMTriSelectAll(e,FALSE);
  while (tempv2!=&sys_vertex_root) {
    tempv2->copy=tempv=CAlloc(sizeof(CGrMVertex));
    MemCpy(&tempv->p,&tempv2->p,sizeof(CD3I32));
    QueIns(tempv,e->vertex_root.last);
    tempv->flags|=VF_SELECTED;
    tempv2=tempv2->next;
  }

  while (tempt2!=&sys_tri_root) {
    tempt=GrMTriNew(e,tempt2->mt.color,tempt2->t[0]->copy,tempt2->t[1]->copy,tempt2->t[2]->copy);
    tempt->flags|=TF_SELECTED;
    tempt2=tempt2->next;
  }
}

GrMClipBoardInit;


U0 DrawIt(CTask *task,CDC *dc)
{
  CGrMFrame *e=NamedFramePtr("CGrMFrame",task);
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  F64 d;
  I64 i,x,y,z;
  CGrMVertex *tempv;
 
  task->horz_scroll.min=-(GE_WORKSPACE_SIZE-task->win_pixel_width)/2;
  task->horz_scroll.max= (GE_WORKSPACE_SIZE-task->win_pixel_width)/2;
  task->vert_scroll.min=-(GE_WORKSPACE_SIZE-task->win_pixel_height)/2;
  task->vert_scroll.max= (GE_WORKSPACE_SIZE-task->win_pixel_height)/2;
  TaskDerivedValsUpdate(task);
  GrMRecalcCxCy(task,e);

  dc->flags|=DCF_TRANSFORMATION;

  Free(dc->r); //Set rotmat doesn't free old dc->r matrix.
  GrSetRotMat(dc,GrMSetW2S(e,task));

  dc->x=e->cx;
  dc->y=e->cy;
//z-vals less than zero are in front of screen and not drawn.
//we want to shift all Z-vals into a drawable range.
//GR_Z_ALL is set to half of the Z-range which is an I32.
  dc->z=GR_Z_ALL;

//Light source set to input pointer (mouse).
  GrMCursorW(e,task,&x,&y,&z);
  dc->ls.x=x;
  dc->ls.y=y;
  dc->ls.z=z;
  d=1<<16/P3I32Norm(&dc->ls); //Light source normalized to 65536.
  dc->ls.x*=d;
  dc->ls.y*=d;
  dc->ls.z*=d;

  DCAllocDepthBuf(dc);

  tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    x=tempv->p.x; y=tempv->p.y; z=tempv->p.z;
    (*dc->transform)(dc,&x,&y,&z);
    tempv->pt.x=x; tempv->pt.y=y; tempv->pt.z=z;
    tempv=tempv->next;
  }

  e->mp_not_done_flags=1<<mp_cnt-1; //Issue jobs to all cores.
  e->dc=dc;
  for (i=0;i<mp_cnt;i++)
    JobQue(&MPDrawIt,e,i);

  tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    x=tempv->pt.x; y=tempv->pt.y; z=tempv->pt.z;
    if (e->vertex_on) {
      if (Blink(10)) //This blinks at 10 Hz.
	dc->color=ROPF_DITHER+BLACK<<16+WHITE;
      else
	dc->color=ROPF_DITHER+WHITE<<16+BLACK;
      GrLine(dc,x-3,y-3,x+3,y+3);
      GrLine(dc,x-3,y+3,x+3,y-3);
    }
    if (tempv->flags&VF_SELECTED) {
      if (e->mode=='t') {
	if (Blink(10)) //This blinks at 10 Hz.
	  dc->color=ROPF_DITHER+e->cur_color.c0.color<<16+e->cur_color.c0.color^8;
	else
	  dc->color=ROPF_DITHER+(e->cur_color.c0.color^8)<<16+e->cur_color.c0.color;
      } else {
	if (Blink)
	  dc->color=ROPF_DITHER+RED<<16+WHITE;
	else
	  dc->color=ROPF_DITHER+WHITE<<16+RED;
      }
      GrCircle(dc,x,y,3);
    }
    tempv=tempv->next;
  }

  if (CtrlInside(c,ipx_presnap,ipy_presnap)||win_show_menu) {
    SetSnap(&task->snap,1,1.0);
    task->win_inhibit=WIF_DOC;
  } else {
    GrMCurSnap(e);
    task->win_inhibit=WIF_ALL|WIF_DOC-WIF_BORDER-WIF_CTRLS-WIF_MENU-WIF_GRAB_SCROLL;
  }

  GrMCursorW(e,task,&x,&y,&z);
  if (z<0)
    dc->color=ROP_XOR+RED^COLOR_TRANSPARENT;
  else
    dc->color=ROP_XOR+COLOR_TRANSPARENT;
  GrPrintF(dc,0,0,"%6.3f%% (%d,%d,%d)",e->view_scale*100,x,y,z);
  dc->pen_width=1;
  dc->color&=0xF;
  if (Blink(10))
    dc->color^=0xF;
  GrLine3(dc,x,y,z,x,y,0);

  if (e->select_rect) {
    dc->flags&=~DCF_TRANSFORMATION;
    dc->pen_width=1;
    if (Blink)
      dc->color=ROPF_DITHER+RED<<16+WHITE;
    else
      dc->color=ROPF_DITHER+WHITE<<16+RED;
    GrLineRect4(dc,e->x1,e->y1,e->x2,e->y2);
  }
//Wait for all cores to complete.
  while (e->mp_not_done_flags)
    Yield;
}

U0 GrMInit(CGrMFrame *e,Bool flip_y)
{
  MemSet(e,0,sizeof(CGrMFrame));
  e->task=Fs;
  e->vertex_root.next=e->vertex_root.last=&e->vertex_root;
  e->tri_root.next=e->tri_root.last=&e->tri_root;
  e->mode='v';
  e->grid_on=TRUE;
  e->vertex_on=TRUE;
  e->my_ipz=0;
  e->view_scale=1.0;
  e->w2s=NULL;
  e->s2w=NULL;
  e->cur_color=RED;
  e->cur_snap=5;
  e->flip_y=flip_y;
  e->select_rect=FALSE;
  e->cur_tri=NULL;
  e->cur_vertex=NULL;
}

U0 GrMLoad(CGrMFrame *e,U8 *src)
{
  I64 i,j,x,y,z;
  CColorROPU32 color;
  CGrMVertex *tempv,*va[3];

  e->vertex_root.next=e->vertex_root.last=&e->vertex_root;
  e->tri_root.next=e->tri_root.last=&e->tri_root;

  e->vertex_cnt  =*src(I32 *)++;
  e->tri_cnt=*src(I32 *)++;
  for (i=0;i<e->vertex_cnt;i++) {
    x=*src(I32 *)++;
    y=*src(I32 *)++;
    z=*src(I32 *)++;
    tempv=GrMVertexNew(e,x,y,z);
    tempv->num=i;
  }
  for (i=0;i<e->tri_cnt;i++) {
    color=*src(I32 *)++;
    for (j=0;j<3;j++)
      va[j]=GrMVertexFindNum(e,*src(I32 *)++);
    GrMTriNew(e,color,va[0],va[1],va[2]);
  }
}

I64 GrMSize(CGrMFrame *e)
{
  I64 i;
  CGrMVertex   *tempv=e->vertex_root.next;
  CGrMTri *tempt=e->tri_root.next;

  e->vertex_cnt=0;
  while (tempv!=&e->vertex_root) {
    tempv->num=e->vertex_cnt++;
    tempv=tempv->next;
  }

  e->tri_cnt=0;
  while (tempt!=&e->tri_root) {
    e->tri_cnt++;
    for (i=0;i<3;i++)
      tempt->mt.nums[i]=tempt->t[i]->num;
    tempt=tempt->next;
  }
  return sizeof(I32)*2+
	 (offset(CGrMVertex.end)-offset(CGrMVertex.start))*e->vertex_cnt+
	 (offset(CGrMTri.end)-offset(CGrMTri.start))*e->tri_cnt;
}

I32 *GrMSave(CGrMFrame *e,I64 *_size=NULL)
{
  I64 size=GrMSize(e);
  U8 *result=MAlloc(size),*dst=result;
  CGrMVertex   *tempv=e->vertex_root.next;
  CGrMTri *tempt=e->tri_root.next;

  *dst(I32 *)++=e->vertex_cnt;
  *dst(I32 *)++=e->tri_cnt;

  e->vertex_cnt=0;
  while (tempv!=&e->vertex_root) {
    MemCpy(dst,&tempv->start,offset(CGrMVertex.end)-offset(CGrMVertex.start));
    dst+=offset(CGrMVertex.end)-offset(CGrMVertex.start);
    tempv=tempv->next;
  }

  e->tri_cnt=0;
  while (tempt!=&e->tri_root) {
    MemCpy(dst,&tempt->start,offset(CGrMTri.end)-offset(CGrMTri.start));
    dst+=offset(CGrMTri.end)-offset(CGrMTri.start);
    tempt=tempt->next;
  }
  if (_size) *_size=size;
  return result;
}

U0 GrMCleanUp(CGrMFrame *e)
{
  QueDel(&e->vertex_root,TRUE);
  QueDel(&e->tri_root,TRUE);
  Free(e->w2s);
  Free(e->s2w);
}

U0 GrMUpdateMenu(CGrMFrame *e)
{
  CMenuEntry *tempse;
  if (tempse=MenuEntryFind(Fs->cur_menu,"View/Grid"))
    tempse->checked=ToBool(e->grid_on);
  if (tempse=MenuEntryFind(Fs->cur_menu,"View/Vertex"))
    tempse->checked=ToBool(e->vertex_on);
  if (tempse=MenuEntryFind(Fs->cur_menu,"Mode/PlaceVertex"))
    tempse->checked=ToBool(e->mode=='v');
  if (tempse=MenuEntryFind(Fs->cur_menu,"Mode/MoveVertex"))
    tempse->checked=ToBool(e->mode=='m');
  if (tempse=MenuEntryFind(Fs->cur_menu,"Mode/MoveVertexZ"))
    tempse->checked=ToBool(e->mode=='M');
  if (tempse=MenuEntryFind(Fs->cur_menu,"Mode/MakeTri"))
    tempse->checked=ToBool(e->mode=='t');
  if (tempse=MenuEntryFind(Fs->cur_menu,"View/FlipY"))
    tempse->checked=ToBool(e->flip_y);
}

I32 *CSpriteMesh(I32 *root=NULL,I64 *_size=NULL,Bool flip_y=FALSE)
/*$WW,1$

Fmt for mesh:
{
I32 vertex_cnt;
I32 tri_cnt;
CD3I32 vertices[];
CMeshTri tris[];
}

If root points to a mesh, it will load it.

Returns a newly malloced mesh or NULL.


See $LK-A,"::/TempleOS/Demo/Graphics/SpritePlot3D.CPP"$.


$WW,0$*/
{
  CCtrl *c=CtrlFindUnique(Fs,CTRLT_VIEWING_ANGLES);
  CViewAngles *s,*old_s;
  I64 i,msg_code,p1,p2,vn=0,x,y,z;
  CD3I32 p0a,p0b;
  CGrMVertex *va[3],*tempv;
  Bool adjusting_z=FALSE,moving,save_and_exit;
  CGrMFrame e;

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
  "  FlipTri(,'f');"
  "}"
  "Mode {"
  "  PlaceVertex(,'v');"
  "  MoveVertex(,'m');"
  "  MoveVertexZ(,'M');"
  "  MakeTri(,'t');"
  "}"
  "Settings {"
  "  Color(,'c');"
  "  Snap(,'s');"
  "}"
  "View {"
  "  ZoomIn(,'z');"
  "  ZoomOut(,'Z');"
  "  NullAngles(,'n');"
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
  "  FlipTri(,'F');"
  "}");

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WordStat;
  Preempt(OFF); //We don't really want window update while modifying links
  AcctOneTimePopUp(ARf_MESH_ED,
	"$$FG,GREEN$$Right Mouse$$FG$$: Hold and move to shift cursor z\n"
	"$$FG,GREEN$$'j'$$FG$$: Jump cursor Z to nearest vertex's Z\n"
	"$$FG,GREEN$$'v'$$FG$$: Place Vertex Mode\n"
	"$$FG,GREEN$$'t'$$FG$$: Make Tri Mode\n"
	"$$FG,GREEN$$'m'$$FG$$: Move Vertex Mode\n"
	"$$FG,GREEN$$'M'$$FG$$: Move Vertex Z\n"
	"$$FG,GREEN$$'c'$$FG$$: Set color\n"
	"$$FG,GREEN$$'s'$$FG$$: Set snap\n"
	"\nSee menu at top of screen for more.\n");

  Fs->win_inhibit=WIF_ALL|WIF_DOC-WIF_BORDER-WIF_CTRLS-WIF_MENU-WIF_GRAB_SCROLL;
  Fs->horz_scroll.pos=0;
  Fs->vert_scroll.pos=0;
  GrMInit(&e,flip_y);
  if (root)
    GrMLoad(&e,root);
  NamedFramePtrAdd("CGrMFrame",&e);
  Fs->draw_it=&DrawIt;
  GrMCurSnap(&e);
  GrMRecalcCxCy(Fs,&e);

  try { //In case of <CTRL-ALT-C>
    while (TRUE) {
      GrMUpdateMenu(&e);
      msg_code=GetMsg(&p1,&p2,1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP);
gm_restart:
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (p1) {
	    case 0:
	      switch (p2.u8[0]) {
		case SC_DELETE:
		  if (p2&SCF_SHIFT) {
		    if (e.mode=='t') {
		      Beep;Beep;
		    } else
		      GrMClipBoardCut(&e);
		  } else {
		    if (GrMSelectCnt(&e))
		      GrMClipBoardDel(&e);
		    else if (e.mode!='t')
		      GrMVertexDel(&e,GrMVertexFindS(&e,ipx_presnap-Fs->win_pixel_left-Fs->win_scroll_x,ipy_presnap-Fs->win_pixel_top-Fs->win_scroll_y));
		    GrMVertexSelectAll(&e,FALSE);
		    GrMTriSelectAll(&e,FALSE);
		    vn=0;
		  }
		  break;
		case SC_INS:
		  if (e.mode=='t') {
		    Beep; Beep;
		  } else {
		    if (p2&SCF_CTRL)
		      GrMClipBoardCopy(&e);
		    else if (p2&SCF_SHIFT) {
		      GrMClipBoardPaste(&e);
		      e.mode='m';
		    }
		  }
		  break;
	      }
	      break;
	    case CH_BACKSPACE:
	      switch (e.mode) {
		case 'v':
		  GrMVertexDel(&e,e.cur_vertex);
		  break;
		case 't':
		  if (vn) {
		    GrMVertexSelectAll(&e,FALSE);
		    GrMTriSelectAll(&e,FALSE);
		    vn=0;
		  } else
		    GrMTriDel(&e,e.cur_tri);
		  break;
	      }
	      break;
	    case 't':
	      GrMVertexSelectAll(&e,FALSE);
	      GrMTriSelectAll(&e,FALSE);
	    case 'v':
	    case 'm':
	    case 'M':
	      adjusting_z=FALSE;
	      moving=FALSE;
	      e.mode=p1;
	      vn=0;
	      Snd(0);
	      break;
	    case 'T':
	      GrMTransformSelect(&e);
	      break;
	    case 'A':
	      GrMTriSelectAll(&e,TRUE);
	      if (e.mode!='t')
		GrMVertexSelectAll(&e,TRUE);
	      else
		GrMVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case 'U':
	      GrMTriSelectAll(&e,FALSE);
	      GrMVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case 'a':
	    case 'u':
	    case 'o':
	      if (p1=='a')
		i=GRM_EQU;
	      else if (p1=='u')
		i=GRM_AND;
	      else
		i=GRM_OR;
	      if ((msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP))!=MSG_IP_L_DOWN) {
		Beep; Beep;
		goto gm_restart;
	      }
	      e.x1=p1; e.y1=p2;
	      e.x2=p1; e.y2=p2;
	      e.select_rect=TRUE;
	      while (TRUE) {
		msg_code=GetMsg(&p1,&p2,1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP);
		if (msg_code==MSG_IP_MOVE) {
		  e.x2=p1; e.y2=p2;
		} else if (msg_code==MSG_IP_L_UP) {
		  e.x2=p1; e.y2=p2;
		  break;
		} else {
		  e.select_rect=FALSE;
		  Beep; Beep;
		  goto gm_restart;
		}
	      }
	      e.select_rect=FALSE;
	      GrMTriSelectRect(&e,i,e.x1,e.x2,e.y1,e.y2);
	      if (e.mode!='t')
		GrMVertexSelectRect(&e,i,e.x1,e.x2,e.y1,e.y2);
	      else
		GrMVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case CH_CTRLB:
	      WinBorder(Bt(&Fs->display_flags,DISPLAYf_NO_BORDER));
	      break;
	    case CH_CTRLC:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else
		GrMClipBoardCopy(&e);
	      break;
	    case CH_CTRLV:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else {
		GrMClipBoardPaste(&e);
		e.mode='m';
	      }
	      break;
	    case CH_CTRLX:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else
		GrMClipBoardCut(&e);
	      break;
	    case CH_SHIFT_ESC:
	      save_and_exit=FALSE;
	      goto mesh_done;
	    case CH_ESC:
	      save_and_exit=TRUE;
	      goto mesh_done;
	    case 'z':
	      GrMScaleZoom(&e,1.5);
	      break;
	    case 'Z':
	      GrMScaleZoom(&e,1/1.5);
	      break;
	    case 'c':
	      e.cur_color=PopUpColorLighting;
	      break;
	    case 's':
	      i=PopUpRangeI64(1,25,1,"New Snap\n");
	      if (i>=1)
		e.cur_snap=i;
	      GrMCurSnap(&e);
	      GrMRecalcCxCy(Fs,&e);
	      break;
	    case 'g':
	      e.grid_on=!e.grid_on;
	      break;
	    case 'V':
	      e.vertex_on=!e.vertex_on;
	      break;
	    case 'n':
	      s->sx=s->sy=s->sz=0;
	      break;
	    case 'y':
	      e.flip_y=!e.flip_y;
	      break;
	    case 'j':
	      if (moving)
		GrMVertexIgnoreSet(&e,TRUE);
	      if (tempv=GrMVertexFindS(&e,ipx-Fs->win_pixel_left-Fs->win_scroll_x,ipy-Fs->win_pixel_top-Fs->win_scroll_y)) {
		Noise(25,2000,8000);
		e.my_ipz=RoundI64(tempv->p.z,e.cur_snap);
	      } else {
		Beep; Beep;
		e.my_ipz=0;
	      }
	      GrMVertexIgnoreSet(&e,FALSE);
	      if (moving) {
		GrMCursorW(&e,Fs,&x,&y,&z);
		if (adjusting_z)
		  GrMP0Offset(&e,0,0,z-p0a.z);
		else
		  GrMP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
		p0a.x=x;
		p0a.y=y;
		p0a.z=z;
		GrMP0Capture(&e);
	      }
	      break;
	    case '1':
	      GrMSwapAxes(&e,offset(CD3I32.x),offset(CD3I32.y));
	      break;
	    case '2':
	      GrMSwapAxes(&e,offset(CD3I32.x),offset(CD3I32.z));
	      break;
	    case '3':
	      GrMSwapAxes(&e,offset(CD3I32.y),offset(CD3I32.z));
	      break;
	    case '4':
	      GrMInvertAxis(&e,offset(CD3I32.x));
	      break;
	    case '5':
	      GrMInvertAxis(&e,offset(CD3I32.y));
	      break;
	    case '6':
	      GrMInvertAxis(&e,offset(CD3I32.z));
	      break;
	    case 'f':
	      if (e.cur_tri)
		SwapU64(&e.cur_tri->t[1],&e.cur_tri->t[2]);
	      break;
	    case 'F':
	      GrMFlipTris(&e);
	      break;

	  }
	  break;
	case MSG_IP_L_DOWN:
	  switch (e.mode) {
	    case 'm':
	      if (!moving) {
		if (!GrMSelectCnt(&e) &&
		    (tempv=GrMVertexFindS(&e,p1,p2))) {
		  tempv->flags|=VF_SELECTED;
		  e.my_ipz=RoundI64(tempv->p.z,e.cur_snap);
		}
		if (GrMSelectCnt(&e)) {
		  GrMCursorW(&e,Fs,&x,&y,&z);
		  p0a.x=x;
		  p0a.y=y;
		  p0a.z=z;
		  GrMP0Capture(&e);
		  moving=TRUE;
		}
	      }
	      break;
	    case 'M':
	      if (!adjusting_z && !moving) {
		if (!GrMSelectCnt(&e) &&
		    (tempv=GrMVertexFindS(&e,p1,p2))) {
		  tempv->flags|=VF_SELECTED;
		  e.my_ipz=RoundI64(tempv->p.z,e.cur_snap);
		}
		if (GrMSelectCnt(&e)) {
		  GrMCursorW(&e,Fs,&x,&y,&z);
		  p0a.x=x;
		  p0a.y=y;
		  p0a.z=z;
		  GrMP0Capture(&e);
		  moving=TRUE;

		  p0b.x=ipx_presnap;
		  p0b.y=ipy_presnap;
		  p0b.z=e.my_ipz;
		  adjusting_z=TRUE;
		  Snd(LimitI64(3*e.my_ipz+1500,0,15000));
		}
	      }
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (e.mode) {
	    case 'v':
	      Noise(25,2000,8000);
	      GrMCursorW(&e,Fs,&x,&y,&z);
	      e.cur_vertex=GrMVertexNew(&e,x,y,z);
	      break;
	    case 'm':
	    case 'M':
	      if (moving) {
		if (adjusting_z) {
		  e.my_ipz=RoundI64(Sign(p0b.y-ipy_presnap)*Sqrt(Sqr(p0b.x-ipx_presnap)+Sqr(p0b.y-ipy_presnap))+p0b.z,e.cur_snap);
		  Snd(0);
		  adjusting_z=FALSE;
		  GrMCursorW(&e,Fs,&x,&y,&z);
		  GrMP0Offset(&e,0,0,z-p0a.z);
		} else {
		  GrMCursorW(&e,Fs,&x,&y,&z);
		  GrMP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
		}
		GrMTriSelectAll(&e,FALSE);
		GrMVertexSelectAll(&e,FALSE);
		moving=FALSE;
	      }
	      break;
	    case 't':
	      if (tempv=GrMVertexFindS(&e,p1,p2)) {
		for (i=0;i<vn;i++)
		  if (va[i]==tempv) {
		    Beep; Beep;
		    break;
		  }
		if (i==vn) {
		  Noise(25,2000,8000);
		  va[vn++]=tempv;
		  tempv->flags|=VF_SELECTED;
		  if (vn==3) {
		    e.cur_tri=GrMTriNew(&e,e.cur_color,va[0],va[1],va[2]);
		    for (i=0;i<vn;i++)
		      va[i]->flags&=~VF_SELECTED;
		    vn=0;
		  }
		}
	      }
	      break;
	  }
	  break;
	case MSG_IP_R_DOWN:
	  if (e.mode!='M' && !adjusting_z) {
	    if (moving) {
	      GrMCursorW(&e,Fs,&x,&y,&z);
	      GrMP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      GrMP0Capture(&e);
	    }
	    p0b.x=ipx_presnap;
	    p0b.y=ipy_presnap;
	    p0b.z=e.my_ipz;
	    adjusting_z=TRUE;
	    Snd(LimitI64(3*e.my_ipz+1500,0,15000));
	  }
	  break;
	case MSG_IP_R_UP:
	  if (e.mode!='M' && adjusting_z) {
	    e.my_ipz=RoundI64(Sign(p0b.y-ipy_presnap)*Sqrt(Sqr(p0b.x-ipx_presnap)+Sqr(p0b.y-ipy_presnap))+p0b.z,e.cur_snap);
	    Snd(0);
	    adjusting_z=FALSE;
	    if (moving) {
	      GrMCursorW(&e,Fs,&x,&y,&z);
	      GrMP0Offset(&e,0,0,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      GrMP0Capture(&e);
	    }
	  }
	  break;
	case MSG_IP_MOVE:
	  if (adjusting_z) {
	    e.my_ipz=RoundI64(Sign(p0b.y-ipy_presnap)*Sqrt(Sqr(p0b.x-ipx_presnap)+Sqr(p0b.y-ipy_presnap))+p0b.z,e.cur_snap);
	    Snd(LimitI64(3*e.my_ipz+1500,0,15000));
	  }
	  if (moving) {
	    GrMCursorW(&e,Fs,&x,&y,&z);
	    if (adjusting_z)
	      GrMP0Offset(&e,0,0,z-p0a.z);
	    else
	      GrMP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
	    p0a.x=x;
	    p0a.y=y;
	    p0a.z=z;
	    GrMP0Capture(&e);
	  }
	  break;
      }
    }
mesh_done:
  } catch
    Fs->catch_except=TRUE;
  SettingsPop;
  MenuPop;
  if (save_and_exit)
    root=GrMSave(&e,_size);
  else
    root=NULL;
  GrMCleanUp(&e);
  NamedFramePtrDel("CGrMFrame");
  if (old_s) {
    MemCpy(c->state,old_s,sizeof(CViewAngles));
    Free(old_s);
  } else
    ViewAnglesDel;
  return root;
}

#help_index "Graphics"
