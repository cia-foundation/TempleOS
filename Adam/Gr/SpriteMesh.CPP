#help_index "Graphics/Mesh"
#define MESH_WORKSPACE_SIZE	4000

#define VF_SELECTED	1
#define VF_COPIED	2
#define VF_IGNORE	4

class CMeshEdVertex
{
  CMeshEdVertex *next,*last,*copy;

  U0 start;
  CD3I32 p;  //World coordinates of the point.
  U0 end;
  CD3I32 p0,
       pt; //Transformed coordinates.  (Screen)
  I32 num,flags;
};

#define TF_SELECTED	1
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
  I64 my_ip_z; //Z-coordinate of the input pointer (mouse).
  I64 mode,cx,cy;
  CColorROPU32 cur_color;
  Bool grid_on,flip_y,select_rect,vertex_on,pad[4];
  I64 mp_not_done_flags; //Used for multiprocessing signaling.
  F64 view_scale;
  CDC *dc;
  I64 *w2s,*s2w; //Screen-to-world and world-to-screen transform matrices.
  I64 vertex_cnt,tri_cnt; //Set by MeshSize
  CMeshEdVertex vertex_root,*cur_vertex;
  CMeshEdTri	tri_root,*cur_tri;
  I64 x1,y1,x2,y2,cur_snap;
};

CMeshEdVertex *MeshVertexNew(CMeshFrame *e,I64 x,I64 y,I64 z)
{
  CMeshEdVertex *tempv=CAlloc(sizeof(CMeshEdVertex));
  tempv->p.x=x;
  tempv->p.y=y;
  tempv->p.z=z;
  QueIns(tempv,e->vertex_root.last);
  return tempv;
}

CMeshEdTri *MeshTriNew(CMeshFrame *e,CColorROPU32 color,CMeshEdVertex *v1,CMeshEdVertex *v2,CMeshEdVertex *v3)
{
  static I64 cpu_num=0;
  CMeshEdTri *tempt=CAlloc(sizeof(CMeshEdTri));
  tempt->cpu_num=cpu_num++%mp_cnt;
  tempt->mt.color=color;
  tempt->t[0]=v1;
  tempt->t[1]=v2;
  tempt->t[2]=v3;
  QueIns(tempt,e->tri_root.last);
  return tempt;
}

CMeshEdVertex *MeshVertexFindScrPt(CMeshFrame *e,I64 x,I64 y)
{//Screen coordinates
  CMeshEdVertex *result=NULL,*tempv=e->vertex_root.next;
  I64 dd,dz,best_dd=MAX_I64,best_dz=MAX_I64;
  while (tempv!=&e->vertex_root) {
    if (!(tempv->flags&VF_IGNORE)) {
      dd=SqrI64(x-tempv->pt.x)+SqrI64(y-tempv->pt.y);
      dz=AbsI64(e->my_ip_z-tempv->p.z);
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

CMeshEdVertex *MeshVertexFindNum(CMeshFrame *haystack_e,I64 needle_num)
{
  CMeshEdVertex *tempv=haystack_e->vertex_root.next;
  while (tempv!=&haystack_e->vertex_root) {
    if (tempv->num==needle_num)
      return tempv;
    tempv=tempv->next;
  }
  return NULL;
}

U0 MeshTriDel(CMeshFrame *e,CMeshEdTri *tempt)
{
  if (tempt) {
    if (tempt==e->cur_tri)
      e->cur_tri=NULL;
    QueRem(tempt);
    Free(tempt);
  }
}

U0 MeshVertexDel(CMeshFrame *e,CMeshEdVertex *tempv)
{
  I64 i;
  CMeshEdTri *tempt,*tempt1;
  if (tempv) {
    tempt=e->tri_root.next;
    while (tempt!=&e->tri_root) {
      tempt1=tempt->next;
      for (i=0;i<3;i++)
	if (tempt->t[i]==tempv)
	  break;
      if (i<3)
	MeshTriDel(e,tempt);
      tempt=tempt1;
    }
    if (tempv==e->cur_vertex)
      e->cur_vertex=NULL;
    QueRem(tempv);
    Free(tempv);
  }
}

U0 MeshVertexSelectAll(CMeshFrame *e,Bool val)
{
  CMeshEdVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    if (val)
      tempv->flags|=VF_SELECTED;
    else
      tempv->flags&=~VF_SELECTED;
    tempv=tempv->next;
  }
}

U0 MeshTriSelectAll(CMeshFrame *e,Bool val)
{
  CMeshEdTri *tempt=e->tri_root.next;
  while (tempt!=&e->tri_root) {
    if (val)
      tempt->flags|=TF_SELECTED;
    else
      tempt->flags&=~TF_SELECTED;
    tempt=tempt->next;
  }
}

U0 MeshVertexIgnoreSet(CMeshFrame *e,Bool val)
{
  CMeshEdVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    tempv->flags&=~VF_IGNORE;
    if (tempv->flags&VF_SELECTED && val)
      tempv->flags|=VF_IGNORE;
    tempv=tempv->next;
  }
}

U0 MeshP0Capture(CMeshFrame *e)
{
  CMeshEdVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    MemCpy(&tempv->p0,&tempv->p,sizeof(CD3I32));
    tempv=tempv->next;
  }
}

U0 MeshP0Offset(CMeshFrame *e,I64 dx,I64 dy,I64 dz)
{
  CMeshEdVertex *tempv=e->vertex_root.next;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED) {
      tempv->p.x=tempv->p0.x+dx;
      tempv->p.y=tempv->p0.y+dy;
      tempv->p.z=tempv->p0.z+dz;
    }
    tempv=tempv->next;
  }
}

#define MESH_EQU	0
#define MESH_OR		1
#define MESH_AND	2

U0 MeshVertexSelectRect(CMeshFrame *e,I64 mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CMeshEdVertex *tempv=e->vertex_root.next;
  if (x1>x2) SwapI64(&x1,&x2);
  if (y1>y2) SwapI64(&y1,&y2);
  while (tempv!=&e->vertex_root) {
    if (x1<=tempv->pt.x<=x2 &&
	y1<=tempv->pt.y<=y2) {
      if (mode==MESH_AND)
	tempv->flags&=~VF_SELECTED;
      else
	tempv->flags|=VF_SELECTED;
    } else if (mode==MESH_EQU)
      tempv->flags&=~VF_SELECTED;
    tempv=tempv->next;
  }
}

U0 MeshTriSelectRect(CMeshFrame *e,I64 mode,I64 x1,I64 x2,I64 y1,I64 y2)
{
  CMeshEdTri *tempt=e->tri_root.next;
  if (x1>x2) SwapI64(&x1,&x2);
  if (y1>y2) SwapI64(&y1,&y2);
  while (tempt!=&e->tri_root) {
    if (x1<=tempt->t[0]->pt.x<=x2 &&
	y1<=tempt->t[0]->pt.y<=y2 &&
	x1<=tempt->t[1]->pt.x<=x2 &&
	y1<=tempt->t[1]->pt.y<=y2 &&
	x1<=tempt->t[2]->pt.x<=x2 &&
	y1<=tempt->t[2]->pt.y<=y2) {
      if (mode==MESH_AND)
	tempt->flags&=~TF_SELECTED;
      else
	tempt->flags|=TF_SELECTED;
    } else {
      if (mode==MESH_EQU)
	tempt->flags&=~TF_SELECTED;
      else if (mode==MESH_AND) {
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

I64 MeshSelectCnt(CMeshFrame *e)
{
  I64 result=0;
  CMeshEdVertex *tempv=e->vertex_root.next;
  CMeshEdTri *tempt=e->tri_root.next;
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

U0 MeshSwapAxes(CMeshFrame *e,I64 o1,I64 o2)
{
  Bool unselect;
  CMeshEdVertex *tempv=e->vertex_root.next;
  if (!MeshSelectCnt(e)) {
    MeshVertexSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED)
      SwapU32((&tempv->p)(U8 *)+o1,(&tempv->p)(U8 *)+o2);
    tempv=tempv->next;
  }
  if (unselect)
    MeshVertexSelectAll(e,FALSE);
}

U0 MeshInvertAxis(CMeshFrame *e,I64 o)
{
  Bool unselect;
  CMeshEdVertex *tempv=e->vertex_root.next;
  if (!MeshSelectCnt(e)) {
    MeshVertexSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED)
      *((&tempv->p)(U8 *)+o)(I32 *)=-*((&tempv->p)(U8 *)+o)(I32 *);
    tempv=tempv->next;
  }
  if (unselect)
    MeshVertexSelectAll(e,FALSE);
}

U0 MeshTransformSelect(CMeshFrame *e)
{
  Bool unselect;
  I64 r[16],x,y,z;
  CMeshEdVertex *tempv=e->vertex_root.next;
  if (PopUpTransform(r)) {
    if (!MeshSelectCnt(e)) {
      MeshVertexSelectAll(e,TRUE);
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
      MeshVertexSelectAll(e,FALSE);
  }
}

U0 MeshFlipTris(CMeshFrame *e)
{
  Bool unselect;
  CMeshEdTri *tempt=e->tri_root.next;
  if (!MeshSelectCnt(e)) {
    MeshTriSelectAll(e,TRUE);
    unselect=TRUE;
  } else
    unselect=FALSE;
  while (tempt!=&e->tri_root) {
    if (tempt->flags & TF_SELECTED)
      SwapI64(&tempt->t[1],&tempt->t[2]);
    tempt=tempt->next;
  }
  if (unselect)
    MeshTriSelectAll(e,FALSE);
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
    GrRot(e->w2s,&x1,&y1,&z1);
    x2=0; y2=e->cur_snap<<16; z2=0;
    GrRot(e->w2s,&x2,&y2,&z2);
    ip_grid.x=Max(1,MaxI64(x1,x2)>>16);
    ip_grid.y=Max(1,MaxI64(y1,y2)>>16);
    ip_grid.z=Min(ip_grid.x,ip_grid.y);
  }
}

U0 MeshScaleZoom(CMeshFrame *e,F64 scale)
{
  CTask *task=Fs;
  I64	x=ip.pos.x-task->pix_left-task->scroll_x-task->pix_width/2,
	y=ip.pos.y-task->pix_top-task->scroll_y-task->pix_height/2;
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

  CMeshEdTri *tempt=e->tri_root.next;
  I64 i,*old_r=dc->r;

//$LK,"DCAlias",A="MN:DCAlias"$() alllocs a new identity rotation matrix.
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

//e->dc's depth buf was copied but we don't want it freed during $LK,"DCDel",A="MN:DCDel"$().
  dc->depth_buf=NULL;

  DCDel(dc);
  LBtr(&e->mp_not_done_flags,Gs->num);
}

I64 *MeshW2S(CMeshFrame *e,CTask *task)
{//World to screen coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=GrIdentNew(task);
  GrScaleEqu(r,e->view_scale);
  GrRotZEqu(r,s->az);
  GrRotYEqu(r,s->ay);
  if (e->flip_y)
    GrRotXEqu(r,s->ax);
  else
    GrRotXEqu(r,s->ax+ã);
  return r;
}

I64 *MeshS2W(CMeshFrame *e,CTask *task)
{//Screen to world coordinate transform matrix.
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  CViewAngles *s=c->state;
  I64 *r=GrIdentNew(task);
  if (e->flip_y)
    GrRotXEqu(r,-s->ax);
  else
    GrRotXEqu(r,-(s->ax+ã));
  GrRotYEqu(r,-s->ay);
  GrRotZEqu(r,-s->az);
  GrScaleEqu(r,1/e->view_scale);
  return r;
}

I64 *MeshSetW2S(CMeshFrame *e,CTask *task)
{
  Free(e->w2s);
  e->w2s=MeshW2S(e,task);
  Free(e->s2w);
  e->s2w=MeshS2W(e,task);
  return MAllocIdentical(e->w2s,task); //returned matrix is assigned to dc->r and will be freed by $LK,"DCDel",A="MN:DCDel"$().
}

U0 MeshCursorW(CMeshFrame *e,CTask *task,I64 *_x,I64 *_y,I64 *_z)
{
  I64	x_shadow,y_shadow,z_shadow,
	xc=ip.pos.x-task->pix_left-task->scroll_x-e->cx,yc=ip.pos.y-task->pix_top-task->scroll_y-e->cy,zc=0,
	x=0,y=0,z=e->my_ip_z,
	i,x2,y2,z2;
  GrRot(e->w2s,&x,&y,&z); //screen of Z vect

//Converges onto a solution for zc, an unknown.
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
  z=RoundI64(e->my_ip_z,e->cur_snap);
  *_x=x; *_y=y; *_z=z;
}

CMeshEdVertex	sys_clipboard_vertex_root;
CMeshEdTri	sys_clipboard_tri_root;

U0 MeshClipboardInit()
{
  QueInit(&sys_clipboard_vertex_root);
  QueInit(&sys_clipboard_tri_root);
}

U0 MeshClipboardRst()
{
  QueDel(&sys_clipboard_vertex_root,TRUE);
  QueDel(&sys_clipboard_tri_root,TRUE);
  MeshClipboardInit;
}

U0 MeshClipboardCopy(CMeshFrame *e)
{
  CMeshEdVertex	*tempv=e->vertex_root.next,*tempv2;
  CMeshEdTri	*tempt=e->tri_root.next,*tempt2;

  MeshClipboardRst;
  while (tempv!=&e->vertex_root) {
    if (tempv->flags&VF_SELECTED) {
      tempv->copy=tempv2=ACAlloc(sizeof(CMeshEdVertex));
      MemCpy(&tempv2->p,&tempv->p,sizeof(CD3I32));
      QueIns(tempv2,sys_clipboard_vertex_root.last);
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
      tempt2=ACAlloc(sizeof(CMeshEdTri));
      tempt2->t[0]=tempt->t[0]->copy;
      tempt2->t[1]=tempt->t[1]->copy;
      tempt2->t[2]=tempt->t[2]->copy;
      tempt2->mt.color=tempt->mt.color;
      QueIns(tempt2,sys_clipboard_tri_root.last);
      tempt->flags|=TF_COPIED;
      tempt->flags&=~TF_SELECTED;
    } else
      tempt->flags&=~(TF_COPIED|TF_SELECTED);
    tempt=tempt->next;
  }
}

U0 MeshClipboardCut(CMeshFrame *e)
{
  CMeshEdVertex	*tempv=e->vertex_root.next,*tempv1;
  CMeshEdTri	*tempt=e->tri_root.next,*tempt1;
  MeshClipboardCopy(e);
  while (tempt!=&e->tri_root) {
    tempt1=tempt->next;
    if (tempt->flags&TF_COPIED)
      MeshTriDel(e,tempt);
    tempt=tempt1;
  }
  while (tempv!=&e->vertex_root) {
    tempv1=tempv->next;
    if (tempv->flags&VF_COPIED)
      MeshVertexDel(e,tempv);
    tempv=tempv1;
  }
}

U0 MeshClipboardDel(CMeshFrame *e)
{//Technically not clipboard
  CMeshEdVertex	*tempv=e->vertex_root.next,*tempv1;
  CMeshEdTri	*tempt=e->tri_root.next,*tempt1;
  while (tempt!=&e->tri_root) {
    tempt1=tempt->next;
    if (tempt->flags&TF_SELECTED)
      MeshTriDel(e,tempt);
    tempt=tempt1;
  }
  while (tempv!=&e->vertex_root) {
    tempv1=tempv->next;
    if (tempv->flags&VF_SELECTED)
      MeshVertexDel(e,tempv);
    tempv=tempv1;
  }
}

U0 MeshClipboardPaste(CMeshFrame *e)
{
  CMeshEdVertex	*tempv2=sys_clipboard_vertex_root.next,*tempv;
  CMeshEdTri	*tempt2=sys_clipboard_tri_root.next,*tempt;
 
  MeshVertexSelectAll(e,FALSE);
  MeshTriSelectAll(e,FALSE);
  while (tempv2!=&sys_clipboard_vertex_root) {
    tempv2->copy=tempv=CAlloc(sizeof(CMeshEdVertex));
    MemCpy(&tempv->p,&tempv2->p,sizeof(CD3I32));
    QueIns(tempv,e->vertex_root.last);
    tempv->flags|=VF_SELECTED;
    tempv2=tempv2->next;
  }

  while (tempt2!=&sys_clipboard_tri_root) {
    tempt=MeshTriNew(e,tempt2->mt.color,tempt2->t[0]->copy,tempt2->t[1]->copy,tempt2->t[2]->copy);
    tempt->flags|=TF_SELECTED;
    tempt2=tempt2->next;
  }
}

MeshClipboardInit;

U0 DrawIt(CTask *task,CDC *dc)
{
  CMeshFrame *e=FramePtr("CMeshFrame",task);
  CCtrl *c=CtrlFindUnique(task,CTRLT_VIEWING_ANGLES);
  F64 d;
  I64 i,x,y,z;
  CMeshEdVertex *tempv;
 
  task->horz_scroll.min=-(MESH_WORKSPACE_SIZE-task->pix_width)/2;
  task->horz_scroll.max= (MESH_WORKSPACE_SIZE-task->pix_width)/2;
  task->vert_scroll.min=-(MESH_WORKSPACE_SIZE-task->pix_height)/2;
  task->vert_scroll.max= (MESH_WORKSPACE_SIZE-task->pix_height)/2;
  TaskDerivedValsUpdate(task);
  MeshRecalcCxCy(task,e);

  dc->flags|=DCF_TRANSFORMATION;

  Free(dc->r); //Set rotmat doesn't free old dc->r matrix.
  GrSetRotMat(dc,MeshSetW2S(e,task));

  dc->x=e->cx;
  dc->y=e->cy;
//z-vals less than zero are in front of screen and not drawn.
//we want to shift all Z-vals into a drawable range.
//GR_Z_ALL is set to half of the Z-range which is an I32.
  dc->z=GR_Z_ALL;

//Light source set to input pointer (mouse).
  MeshCursorW(e,task,&x,&y,&z);
  dc->ls.x=x;
  dc->ls.y=y;
  dc->ls.z=z;
  d=1<<16/D3I32Norm(&dc->ls); //Light source normalized to 65536.
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

  if (CtrlInside(c,ip.presnap.x,ip.presnap.y)||winmgr.show_menu) {
    GridInit;
    task->win_inhibit=WIF_SELF_DOC;
  } else {
    MeshCurSnap(e);
    task->win_inhibit=WIG_TASK_DFT|WIF_SELF_DOC-WIF_SELF_FOCUS-WIF_SELF_BORDER-WIF_SELF_CTRLS-WIF_FOCUS_TASK_MENU-WIF_SELF_GRAB_SCROLL;
  }

  MeshCursorW(e,task,&x,&y,&z);
  if (z<0)
    dc->color=ROP_XOR+RED^TRANSPARENT;
  else
    dc->color=ROP_XOR+TRANSPARENT;
  GrPrint(dc,0,0,"%6.3f%% (%d,%d,%d)",e->view_scale*100,x,y,z);
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
    GrBorder(dc,e->x1,e->y1,e->x2,e->y2);
  }
//Wait for all cores to complete.
  while (e->mp_not_done_flags)
    Yield;
}

U0 MeshInit(CMeshFrame *e,Bool flip_y)
{
  MemSet(e,0,sizeof(CMeshFrame));
  e->vertex_root.next=e->vertex_root.last=&e->vertex_root;
  e->tri_root.next=e->tri_root.last=&e->tri_root;
  e->mode='v';
  e->grid_on=TRUE;
  e->vertex_on=TRUE;
  e->my_ip_z=0;
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

U0 MeshLoad(CMeshFrame *e,U8 *src)
{
  I64 i,j,x,y,z;
  CColorROPU32 color;
  CMeshEdVertex *tempv,*va[3];

  e->vertex_root.next=e->vertex_root.last=&e->vertex_root;
  e->tri_root.next=e->tri_root.last=&e->tri_root;

  e->vertex_cnt  =*src(I32 *)++;
  e->tri_cnt=*src(I32 *)++;
  for (i=0;i<e->vertex_cnt;i++) {
    x=*src(I32 *)++;
    y=*src(I32 *)++;
    z=*src(I32 *)++;
    tempv=MeshVertexNew(e,x,y,z);
    tempv->num=i;
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
  CMeshEdVertex	*tempv=e->vertex_root.next;
  CMeshEdTri	*tempt=e->tri_root.next;

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
	 (offset(CMeshEdVertex.end)-offset(CMeshEdVertex.start))*e->vertex_cnt+
	 (offset(CMeshEdTri.end)-offset(CMeshEdTri.start))*e->tri_cnt;
}

I32 *MeshSave(CMeshFrame *e,I64 *_size=NULL)
{
  I64 size=MeshSize(e);
  U8 *result=MAlloc(size),*dst=result;
  CMeshEdVertex	*tempv=e->vertex_root.next;
  CMeshEdTri	*tempt=e->tri_root.next;

  *dst(I32 *)++=e->vertex_cnt;
  *dst(I32 *)++=e->tri_cnt;

  e->vertex_cnt=0;
  while (tempv!=&e->vertex_root) {
    MemCpy(dst,&tempv->start,offset(CMeshEdVertex.end)-offset(CMeshEdVertex.start));
    dst+=offset(CMeshEdVertex.end)-offset(CMeshEdVertex.start);
    tempv=tempv->next;
  }

  e->tri_cnt=0;
  while (tempt!=&e->tri_root) {
    MemCpy(dst,&tempt->start,offset(CMeshEdTri.end)-offset(CMeshEdTri.start));
    dst+=offset(CMeshEdTri.end)-offset(CMeshEdTri.start);
    tempt=tempt->next;
  }
  if (_size) *_size=size;
  return result;
}

U0 MeshCleanUp(CMeshFrame *e)
{
  QueDel(&e->vertex_root,TRUE);
  QueDel(&e->tri_root,TRUE);
  Free(e->w2s);
  Free(e->s2w);
}

U0 MeshUpdateMenu(CMeshFrame *e)
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

I32 *SpriteMeshEd(I32 *root=NULL,I64 *_size=NULL,Bool flip_y=FALSE)
{/*Fmt for mesh:
{
I32 vertex_cnt;
I32 tri_cnt;
CD3I32 vertices[];
CMeshTri tris[];
}

If root points to a mesh, it will load it.

Returns a newly malloced mesh or NULL.

See $LK,"::/Demo/Graphics/SpritePlot3D.CPP"$.
$WW,0$*/
  CCtrl *c=CtrlFindUnique(Fs,CTRLT_VIEWING_ANGLES);
  CViewAngles *s,*old_s;
  I64 i,msg_code,a1,a2,vn=0,x,y,z;
  CD3I32 p0a,p0b;
  CMeshEdVertex *va[3],*tempv;
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

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  Preempt(OFF); //We don't really want window update while modifying links
  AcctOneTimePopUp(ARf_MESH_ED,
	"$$GREEN$$Right Mouse$$FG$$: Hold and move to shift cursor z\n"
	"$$GREEN$$'j'$$FG$$: Jump cursor Z to nearest vertex's Z\n"
	"$$GREEN$$'v'$$FG$$: Place Vertex Mode\n"
	"$$GREEN$$'t'$$FG$$: Make Tri Mode\n"
	"$$GREEN$$'m'$$FG$$: Move Vertex Mode\n"
	"$$GREEN$$'M'$$FG$$: Move Vertex Z\n"
	"$$GREEN$$'c'$$FG$$: Set color\n"
	"$$GREEN$$'s'$$FG$$: Set snap\n"
	"\nSee menu at top of screen for more.\n");

  Fs->win_inhibit=WIG_TASK_DFT|WIF_SELF_DOC-WIF_SELF_FOCUS-WIF_SELF_BORDER-WIF_SELF_CTRLS-WIF_FOCUS_TASK_MENU-WIF_SELF_GRAB_SCROLL;
  Fs->horz_scroll.pos=0;
  Fs->vert_scroll.pos=0;
  MeshInit(&e,flip_y);
  if (root)
    MeshLoad(&e,root);
  FramePtrAdd("CMeshFrame",&e);
  Fs->draw_it=&DrawIt;
  MeshCurSnap(&e);
  MeshRecalcCxCy(Fs,&e);

  try {//In case of <CTRL-ALT-c>
    while (TRUE) {
      MeshUpdateMenu(&e);
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP);
gm_restart:
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case 0:
	      switch (a2.u8[0]) {
		case SC_DELETE:
		  if (a2&SCF_SHIFT) {
		    if (e.mode=='t') {
		      Beep;Beep;
		    } else
		      MeshClipboardCut(&e);
		  } else {
		    if (MeshSelectCnt(&e))
		      MeshClipboardDel(&e);
		    else if (e.mode!='t')
		      MeshVertexDel(&e,MeshVertexFindScrPt(&e,ip.presnap.x-Fs->pix_left-Fs->scroll_x,ip.presnap.y-Fs->pix_top-Fs->scroll_y));
		    MeshVertexSelectAll(&e,FALSE);
		    MeshTriSelectAll(&e,FALSE);
		    vn=0;
		  }
		  break;
		case SC_INS:
		  if (e.mode=='t') {
		    Beep; Beep;
		  } else {
		    if (a2&SCF_CTRL)
		      MeshClipboardCopy(&e);
		    else if (a2&SCF_SHIFT) {
		      MeshClipboardPaste(&e);
		      e.mode='m';
		    }
		  }
		  break;
	      }
	      break;
	    case CH_BACKSPACE:
	      switch (e.mode) {
		case 'v':
		  MeshVertexDel(&e,e.cur_vertex);
		  break;
		case 't':
		  if (vn) {
		    MeshVertexSelectAll(&e,FALSE);
		    MeshTriSelectAll(&e,FALSE);
		    vn=0;
		  } else
		    MeshTriDel(&e,e.cur_tri);
		  break;
	      }
	      break;
	    case 't':
	      MeshVertexSelectAll(&e,FALSE);
	      MeshTriSelectAll(&e,FALSE);
	    case 'v':
	    case 'm':
	    case 'M':
	      adjusting_z=FALSE;
	      moving=FALSE;
	      e.mode=a1;
	      vn=0;
	      Snd(0);
	      break;
	    case 'T':
	      MeshTransformSelect(&e);
	      break;
	    case 'A':
	      MeshTriSelectAll(&e,TRUE);
	      if (e.mode!='t')
		MeshVertexSelectAll(&e,TRUE);
	      else
		MeshVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case 'U':
	      MeshTriSelectAll(&e,FALSE);
	      MeshVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case 'a':
	    case 'u':
	    case 'o':
	      if (a1=='a')
		i=MESH_EQU;
	      else if (a1=='u')
		i=MESH_AND;
	      else
		i=MESH_OR;
	      if ((msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP))!=MSG_IP_L_DOWN) {
		Beep; Beep;
		goto gm_restart;
	      }
	      e.x1=a1; e.y1=a2;
	      e.x2=a1; e.y2=a2;
	      e.select_rect=TRUE;
	      while (TRUE) {
		msg_code=GetMsg(&a1,&a2,1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|1<<MSG_IP_R_UP);
		if (msg_code==MSG_IP_MOVE) {
		  e.x2=a1; e.y2=a2;
		} else if (msg_code==MSG_IP_L_UP) {
		  e.x2=a1; e.y2=a2;
		  break;
		} else {
		  e.select_rect=FALSE;
		  Beep; Beep;
		  goto gm_restart;
		}
	      }
	      e.select_rect=FALSE;
	      MeshTriSelectRect(&e,i,e.x1,e.x2,e.y1,e.y2);
	      if (e.mode!='t')
		MeshVertexSelectRect(&e,i,e.x1,e.x2,e.y1,e.y2);
	      else
		MeshVertexSelectAll(&e,FALSE);
	      vn=0;
	      break;
	    case CH_CTRLB:
	      WinBorder(Bt(&Fs->display_flags,DISPLAYf_NO_BORDER));
	      break;
	    case CH_CTRLC:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else
		MeshClipboardCopy(&e);
	      break;
	    case CH_CTRLV:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else {
		MeshClipboardPaste(&e);
		e.mode='m';
	      }
	      break;
	    case CH_CTRLX:
	      if (e.mode=='t') {
		Beep;Beep;
	      } else
		MeshClipboardCut(&e);
	      break;
	    case CH_SHIFT_ESC:
	      save_and_exit=FALSE;
	      goto mesh_done;
	    case CH_ESC:
	      save_and_exit=TRUE;
	      goto mesh_done;
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
	    case 'n':
	      s->sx=s->sy=s->sz=0;
	      break;
	    case 'y':
	      e.flip_y=!e.flip_y;
	      break;
	    case 'j':
	      if (moving)
		MeshVertexIgnoreSet(&e,TRUE);
	      if (tempv=MeshVertexFindScrPt(&e,ip.pos.x-Fs->pix_left-Fs->scroll_x,ip.pos.y-Fs->pix_top-Fs->scroll_y)) {
		Noise(25,2000,8000);
		e.my_ip_z=RoundI64(tempv->p.z,e.cur_snap);
	      } else {
		Beep; Beep;
		e.my_ip_z=0;
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
	    case 'f':
	      if (e.cur_tri)
		SwapI64(&e.cur_tri->t[1],&e.cur_tri->t[2]);
	      break;
	    case 'F':
	      MeshFlipTris(&e);
	      break;

	  }
	  break;
	case MSG_IP_L_DOWN:
	  switch (e.mode) {
	    case 'm':
	      if (!moving) {
		if (!MeshSelectCnt(&e) &&
		    (tempv=MeshVertexFindScrPt(&e,a1,a2))) {
		  tempv->flags|=VF_SELECTED;
		  e.my_ip_z=RoundI64(tempv->p.z,e.cur_snap);
		}
		if (MeshSelectCnt(&e)) {
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
		if (!MeshSelectCnt(&e) &&
		    (tempv=MeshVertexFindScrPt(&e,a1,a2))) {
		  tempv->flags|=VF_SELECTED;
		  e.my_ip_z=RoundI64(tempv->p.z,e.cur_snap);
		}
		if (MeshSelectCnt(&e)) {
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  p0a.x=x;
		  p0a.y=y;
		  p0a.z=z;
		  MeshP0Capture(&e);
		  moving=TRUE;

		  p0b.x=ip.presnap.x;
		  p0b.y=ip.presnap.y;
		  p0b.z=e.my_ip_z;
		  adjusting_z=TRUE;
		  Snd(ClampI64(3*e.my_ip_z+1500,0,15000));
		}
	      }
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (e.mode) {
	    case 'v':
	      Noise(25,2000,8000);
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      e.cur_vertex=MeshVertexNew(&e,x,y,z);
	      break;
	    case 'm':
	    case 'M':
	      if (moving) {
		if (adjusting_z) {
		  e.my_ip_z=RoundI64(Sign(p0b.y-ip.presnap.y)*Sqrt(Sqr(p0b.x-ip.presnap.x)+Sqr(p0b.y-ip.presnap.y))+p0b.z,e.cur_snap);
		  Snd(0);
		  adjusting_z=FALSE;
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  MeshP0Offset(&e,0,0,z-p0a.z);
		} else {
		  MeshCursorW(&e,Fs,&x,&y,&z);
		  MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
		}
		MeshTriSelectAll(&e,FALSE);
		MeshVertexSelectAll(&e,FALSE);
		moving=FALSE;
	      }
	      break;
	    case 't':
	      if (tempv=MeshVertexFindScrPt(&e,a1,a2)) {
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
		    e.cur_tri=MeshTriNew(&e,e.cur_color,va[0],va[1],va[2]);
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
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      MeshP0Offset(&e,x-p0a.x,y-p0a.y,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      MeshP0Capture(&e);
	    }
	    p0b.x=ip.presnap.x;
	    p0b.y=ip.presnap.y;
	    p0b.z=e.my_ip_z;
	    adjusting_z=TRUE;
	    Snd(ClampI64(3*e.my_ip_z+1500,0,15000));
	  }
	  break;
	case MSG_IP_R_UP:
	  if (e.mode!='M' && adjusting_z) {
	    e.my_ip_z=RoundI64(Sign(p0b.y-ip.presnap.y)*Sqrt(Sqr(p0b.x-ip.presnap.x)+Sqr(p0b.y-ip.presnap.y))+p0b.z,e.cur_snap);
	    Snd(0);
	    adjusting_z=FALSE;
	    if (moving) {
	      MeshCursorW(&e,Fs,&x,&y,&z);
	      MeshP0Offset(&e,0,0,z-p0a.z);
	      p0a.x=x;
	      p0a.y=y;
	      p0a.z=z;
	      MeshP0Capture(&e);
	    }
	  }
	  break;
	case MSG_IP_MOVE:
	  if (adjusting_z) {
	    e.my_ip_z=RoundI64(Sign(p0b.y-ip.presnap.y)*Sqrt(Sqr(p0b.x-ip.presnap.x)+Sqr(p0b.y-ip.presnap.y))+p0b.z,e.cur_snap);
	    Snd(ClampI64(3*e.my_ip_z+1500,0,15000));
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
mesh_done:
  } catch
    Fs->catch_except=TRUE;
  SettingsPop;
  MenuPop;
  if (save_and_exit)
    root=MeshSave(&e,_size);
  else
    root=NULL;
  MeshCleanUp(&e);
  FramePtrDel("CMeshFrame");
  if (old_s) {
    MemCpy(c->state,old_s,sizeof(CViewAngles));
    Free(old_s);
  } else
    ViewAnglesDel;
  return root;
}
