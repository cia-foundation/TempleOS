#help_index "Graphics/Sprite;Sprites"

Bool SpritePolyPtPlot(CSprite *root,I64 x,I64 y,I64)
{
  CSprite *tempg=CAlloc(SpriteElemQuedBaseSize(SPT_PT));
  tempg->type=SPT_PT;
  tempg->p.x1=x;
  tempg->p.y1=y;
  QueIns(tempg,root->last);
  return TRUE;
}

CSprite *SpriteElemNum2QueEntry(CSprite *root,I64 elem_num,
	I64 *_x=NULL,I64 *_y=NULL)
{
  I64 xx=0,yy=0;
  CSprite *result=root->next;
  while (result!=root && elem_num--) {
    if (result->type==SPT_SHIFT) {
      xx+=result->p.x1;
      yy+=result->p.y1;
    }
    result=result->next;
  }
  if (_x) *_x=xx;
  if (_y) *_y=yy;
  return result;
}

CSprite *Sprite2SpriteQue(U8 *elems)
{
  I64 s;
  CSprite *result=CAlloc(sizeof(CSprite)),
	*tempg=elems-offset(CSprite.start),*tempg1;
  QueInit(result);
  while (tempg->type) {
    tempg1=MAlloc(SpriteElemSize(tempg)+offset(CSprite.start));
    s=SpriteElemSize(tempg);
    MemCpy(&tempg1->start,&tempg->start,s);
    QueIns(tempg1,result->last);
    tempg(U8 *)+=s;
  }
  return result;
}

U8 *SpriteQue2Sprite(CSprite *root,I64 *_size=NULL)
{
  I64 i,size=sprite_elem_base_sizes[SPT_END];
  CSprite *tempg=root->next;
  U8 *result,*dst;
  while (tempg!=root) {
    size+=SpriteElemSize(tempg);
    tempg=tempg->next;
  }
  if (_size) *_size=size;
  result=dst=MAlloc(size);
  tempg=root->next;
  while (tempg!=root) {
    i=SpriteElemSize(tempg);
    MemCpy(dst,&tempg->start,i);
    dst+=i;
    tempg=tempg->next;
  }
  *dst=SPT_END;
  return result;
}

U0 SpriteEdUpdate(CDoc *doc,CDocBin *tempb,CSprite *root)
{
  I64 size;
  Bool unlock=DocLock(doc);
  Free(tempb->data);
  tempb->data=SpriteQue2Sprite(root,&size);
  tempb->size=size;
  if (unlock)
    DocUnlock(doc);
}

U0 SpriteSetOrigin(CSprite *root,I64 dx,I64 dy,I64 dz)
{
  I64 i;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tempg=root->next;
  while (tempg!=root) {
    switch (tempg->type) {
      case SPT_ARROW:
      case SPT_LINE:
      case SPT_PLANAR_SYMMETRY:
      case SPT_RECT:
      case SPT_ROTATED_RECT:
	tempg->pp.x2+=dx;
	tempg->pp.y2+=dy;
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
	tempg->p.x1+=dx;
	tempg->p.y1+=dy;
	break;
      case SPT_POLYLINE:
	ptr=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++) {
	  ptr[i<<1]+=dx;
	  ptr[i<<1+1]+=dy;
	}
	break;
      case SPT_POLYPT:
	tempg->npu.x+=dx;
	tempg->npu.y+=dy;
	break;
      case SPT_BSPLINE2:
      case SPT_BSPLINE3:
      case SPT_BSPLINE2_CLOSED:
      case SPT_BSPLINE3_CLOSED:
	p=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++,p++) {
	  p->x+=dx;
	  p->y+=dy;
	  p->z+=dz;
	}
	break;
      case SPT_MESH:
	p=&tempg->mu.u;
	for (i=0;i<tempg->mu.vertex_cnt;i++,p++) {
	  p->x+=dx;
	  p->y+=dy;
	  p->z+=dz;
	}
	break;
      case SPT_SHIFTABLE_MESH:
	tempg->pmu.x+=dx;
	tempg->pmu.y+=dy;
	tempg->pmu.z+=dz;
	break;
    }
    tempg=tempg->next;
  }
}

CSprite *SpriteTransformCircle(I64 *r,CSprite *tempg)
{
  I64 x,y,z;
  F64 m1,a1,m2,radius=tempg->pr.radius<<16;
  CSprite *tempg1=CAlloc(SpriteElemQuedBaseSize(SPT_ELLIPSE));
  tempg1->type=SPT_ELLIPSE;

  x=tempg->pr.x1; y=tempg->pr.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tempg1->pwha.x1=x;
  tempg1->pwha.y1=y;

  x=radius; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=0; y=radius; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  m2=Sqrt(x*x+y*y);

  tempg1->pwha.width =ToI64(m1)/0x10000;
  tempg1->pwha.height=ToI64(m2)/0x10000;
  tempg1->pwha.angle=-a1;

  return tempg1;
}

CSprite *SpriteTransformEllipse(I64 *r,CSprite *tempg)
{
  I64 x,y,z;
  F64 m1,a1,m2,a2,s,c,x_radius=tempg->pwha.width<<16,
	y_radius=tempg->pwha.height<<16;
  CSprite *tempg1=CAlloc(SpriteElemQuedBaseSize(tempg->type));
  tempg1->type=tempg->type;
  if (tempg->type==SPT_POLYGON)
    tempg1->pwhas.sides=tempg->pwhas.sides;

  x=tempg->pwha.x1; y=tempg->pwha.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tempg1->pwha.x1=x;
  tempg1->pwha.y1=y;

  c=Cos(-tempg->pwha.angle);
  s=Sin(-tempg->pwha.angle);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m2,&a2,x,y);
  m2*=Abs(Sin(a2-a1));

  tempg1->pwha.width=ToI64(m1)/0x10000;
  if (tempg1->pwha.width<1) tempg1->pwha.width=1;
  tempg1->pwha.height=ToI64(m2)/0x10000;
  if (tempg1->pwha.height<1) tempg1->pwha.height=1;
  tempg1->pwha.angle=-a1;

  return tempg1;
}

CSprite *SpriteTransformRect(I64 *r,CSprite *tempg,F64 é)
{
  I64 x,y,z,w,h;
  F64 m1,a1,m2,a2,s,c,
	x_radius=(tempg->pp.x2-tempg->pp.x1)<<16,
	y_radius=(tempg->pp.y2-tempg->pp.y1)<<16;
  CSprite *tempg1=CAlloc(SpriteElemQuedBaseSize(SPT_ROTATED_RECT));
  tempg1->type=SPT_ROTATED_RECT;

  x=tempg->pp.x1; y=tempg->pp.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tempg1->ppa.x1=x;
  tempg1->ppa.y1=y;

  c=Cos(-é);
  s=Sin(-é);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  R2P(&m2,&a2,x,y);
  m2*=Abs(Sin(a2-a1));

  w=ToI64(m1)/0x10000;
  if (w<1) w=1;
  h=ToI64(m2)/0x10000;
  if (h<1) h=1;
  tempg1->ppa.x2=tempg1->ppa.x1+w;
  tempg1->ppa.y2=tempg1->ppa.y1+h;
  tempg1->ppa.angle=-a1;

  return tempg1;
}

CSprite *SpriteTransformBitmap(I64 *r,CSprite *tempg)
{
  CDC *img,*tempb;
  U8 *elems;
  I64 x,y,z,minx,maxx,miny,maxy,minz,maxz;
  CSprite *tempg1;

  x=0; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  minx=maxx=x;
  miny=maxy=y;
  minz=maxz=z;

  x=0; y=tempg->pwhu.height; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  x=tempg->pwhu.width; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  x=tempg->pwhu.width; y=tempg->pwhu.height; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  tempb=DCNew(maxx-minx+1,maxy-miny+1);

  img=CAlloc(sizeof(CDC));
  img->width=tempg->pwhu.width;
  img->width_internal=(tempg->pwhu.width+7)&~7;
  img->height=tempg->pwhu.height;
  img->body=&tempg->pwhu.u;
  img->dc_signature=DCS_SIGNATURE_VAL;

  tempb->color=TRANSPARENT;
  GrRect(tempb,0,0,maxx-minx+1,maxy-miny+1);

  Free(tempb->r);
  DCMat4x4Set(tempb,r);
  tempb->flags|=DCF_TRANSFORMATION;

  tempb->x=-minx;
  tempb->y=-miny;
  tempb->z=-minz;
  GrBlot3(tempb,0,0,0,img);
  Free(img);

  elems=DC2Sprite(tempb);
  tempb->r=NULL;
  DCDel(tempb);
  tempg1=CAlloc(offset(CSprite.start)+MSize(elems));
  MemCpy(tempg1(U8 *)+offset(CSprite.start),elems,MSize(elems));
  tempg1->type=tempg->type;

  x=tempg->pwhu.x1; y=tempg->pwhu.y1; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tempg1->pwhu.x1=x;
  tempg1->pwhu.y1=y;

  x=0; y=0; z=0;
  Mat4x4MulXYZ(r,&x,&y,&z);
  tempg1->pwhu.x1+=minx-x;
  tempg1->pwhu.y1+=miny-y;

  return tempg1;
}

U0 SpriteTransform(I64 *r,CSprite *root)
{
  I64 i,j,k,num,
	x,y,z,x1,y1,z1,x2,y2,z2,x3,y3,z3;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tempg=root->next,root2,*tempg1,*tempg2,*tempg3;
  while (tempg!=root) {
    switch (tempg->type) {
      case SPT_WIDTH:
	tempg->w.width*=Sqrt(Mat4x4NormSqr65536(r))/65536;
	if (tempg->w.width<0) tempg->w.width=0;
	break;
      case SPT_PLANAR_SYMMETRY:
      case SPT_ARROW:
      case SPT_LINE:
	x=tempg->pp.x2; y=tempg->pp.y2; z=0;
	Mat4x4MulXYZ(r,&x,&y,&z);
	tempg->pp.x2=x;
	tempg->pp.y2=y;
      case SPT_PT:
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
      case SPT_TEXT:
      case SPT_TEXT_BOX:
      case SPT_TEXT_DIAMOND:
	x=tempg->p.x1; y=tempg->p.y1; z=0;
	Mat4x4MulXYZ(r,&x,&y,&z);
	tempg->p.x1=x;
	tempg->p.y1=y;
	break;
      case SPT_BITMAP:
	tempg1=SpriteTransformBitmap(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case SPT_ROTATED_RECT:
	tempg1=SpriteTransformRect(r,tempg,tempg->ppa.angle);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case SPT_RECT:
	tempg1=SpriteTransformRect(r,tempg,0);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case SPT_CIRCLE:
	tempg1=SpriteTransformCircle(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case SPT_ELLIPSE:
      case SPT_POLYGON:
	tempg1=SpriteTransformEllipse(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case SPT_POLYLINE:
	ptr=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++) {
	  x=ptr[i<<1]; y=ptr[i<<1+1]; z=0;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  ptr[i<<1]=x;
	  ptr[i<<1+1]=y;
	}
	break;
      case SPT_POLYPT:
	QueInit(&root2);
	x=tempg->npu.x; y=tempg->npu.y; z=0;
	x1=x; y1=y; z1=z;  //unrotated cur coordinates
	Mat4x4MulXYZ(r,&x,&y,&z);
	ptr=&tempg->npu.u;
	k=tempg->npu.num*3;
	x2=x; y2=y; z2=z;  //rotated start coordinates
	x3=x; y3=y; z3=z;  //lag 1 rotated coordinates
	for (i=0;i<k;i+=3) {
	  j=BFieldExtU32(ptr,i,3);
	  x1+=gr_x_offsets[j];
	  y1+=gr_y_offsets[j];
	  x=x1; y=y1; z=z1;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  Line(&root2,x3-x2,y3-y2,0,x-x2,y-y2,0,&SpritePolyPtPlot);
	  x3=x; y3=y; z3=z;
	}

	num=0;
	tempg1=root2.next;
	x3=0; y3=0; z3=0;
	while (tempg1!=&root2) {
	  tempg2=tempg1->next;
	  if (tempg1->p.x1==x3 && tempg1->p.y1==y3) {
	    QueRem(tempg1);
	    Free(tempg1);
	  } else {
	    num++;
	    x3=tempg1->p.x1;
	    y3=tempg1->p.y1;
	  }
	  tempg1=tempg2;
	}

	tempg3=CAlloc(SpriteElemQuedBaseSize(SPT_POLYPT)+(num*3+7)>>3);
	tempg3->npu.x=x2;
	tempg3->npu.y=y2;
	ptr=&tempg3->npu.u;
	x3=0;y3=0; z3=0;
	i=0;
	tempg1=root2.next;
	while (tempg1!=&root2) {
	  tempg2=tempg1->next;
	  BFieldOrU32(ptr,i,
		polypt_map[SignI64(tempg1->p.x1-x3)+1+
		3*(SignI64(tempg1->p.y1-y3)+1)]);
	  i+=3;
	  x3=tempg1->p.x1;y3=tempg1->p.y1;
	  QueRem(tempg1);
	  Free(tempg1);
	  tempg1=tempg2;
	}
	tempg3->type=SPT_POLYPT;
	tempg3->npu.num=num;
	QueIns(tempg3,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg3;
	break;
      case SPT_BSPLINE2:
      case SPT_BSPLINE3:
      case SPT_BSPLINE2_CLOSED:
      case SPT_BSPLINE3_CLOSED:
	p=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
      case SPT_SHIFTABLE_MESH:
	x=tempg->pmu.x; y=tempg->pmu.y; z=tempg->pmu.z;
	Mat4x4MulXYZ(r,&x,&y,&z);
	tempg->pmu.x=x;
	tempg->pmu.y=y;
	tempg->pmu.z=z;
	p=&tempg->pmu.u;
	for (i=0;i<tempg->pmu.vertex_cnt;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
      case SPT_MESH:
	p=&tempg->mu.u;
	for (i=0;i<tempg->mu.vertex_cnt;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  Mat4x4MulXYZ(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
    }
    tempg=tempg->next;
  }
}

CSprite *SpriteSideBar2SpriteQue(CDoc *doc,CSprite *root,I64 *_cur_elem_num)
{//For the side-bar
  CSprite *result=CAlloc(sizeof(CSprite)),*tempg;
  CDocEntry *doc_e=doc->root.next;
  Bool found=FALSE;
  I64 num=0;
  QueInit(result);
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL && doc_e->left_exp>=0) {
      tempg=SpriteElemNum2QueEntry(root,doc_e->left_exp);
      if (tempg!=root) {
	tempg=MAllocIdentical(tempg);
	if (*_cur_elem_num==doc_e->left_exp) {
	  *_cur_elem_num=num;
	  found=TRUE;
	}
	QueIns(tempg,result->last);
	num++;
      }
    }
    doc_e=doc_e->next;
  }
  if (!found)
    *_cur_elem_num=num;
  QueDel(root);
  Free(root);
  return result;
}

U0 SpriteSideBarPickNew(CDoc *doc,CSprite *root,I64 old_num)
{
  CSprite *tempg;
  CDocEntry *doc_e=NULL;
  I64 cur_elem_num=0;
  U8 *st;

  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite SideBar\"$$\n"
	"$$LK+PU+CX,\"Click for Help\","
	"A=\"FI:::/Doc/SpriteSideBar.TXT.Z\"$$\n\n");

  tempg=root->next;
  while (tempg!=root) {
    st=SpriteElem2Summary(tempg);
    if (cur_elem_num==old_num) {
      DocPrint(doc,"$$LTRED$$");
      doc_e=DocPrint(doc,"$$MU-UL,\"%Q\",LE=%d$$\n",st,cur_elem_num++);
    } else {
      DocPrint(doc,"$$LTBLUE$$");
      DocPrint(doc,"$$MU-UL,\"%Q\",LE=%d$$\n",st,cur_elem_num++);
    }
    Free(st);
    tempg=tempg->next;
  }
  if (cur_elem_num==old_num) {
    DocPrint(doc,"$$LTRED$$");
    doc_e=DocPrint(doc,"$$MU-UL,\"END\",LE=%d$$\n",cur_elem_num);
  } else {
    DocPrint(doc,"$$LTBLUE$$");
    DocPrint(doc,"$$MU-UL,\"END\",LE=%d$$\n",cur_elem_num);
  }
  if (doc_e)
    doc->cur_entry=doc_e;
}

U0 SpriteSideBarTask2(CTask *grand_parent)
{
  I64 w,h;
  CTask *parent=Fs->parent_task,*pu_task;
  while (TRUE) {
    pu_task=grand_parent->popup_task;
    if (TaskValidate(pu_task)) {
      w=parent->win_right-parent->win_left;
      WinHorz(pu_task->win_right+2,pu_task->win_right+2+w,parent);
      h=parent->win_bottom-parent->win_top;
      WinVert(pu_task->win_top,pu_task->win_top+h,parent);
    }
    WinMgrSync;
  }
}

U0 SpriteSideBarTask(CTask *parent,CSprite **_root,I64 *_cur_elem_num)
{
  CDocEntry *doc_e;
  CDoc *doc=DocPut;
  Spawn(&SpriteSideBarTask2,parent,"CSpriteTask",,Fs);
  doc->flags|=DOCF_MIN_SIZE|DOCF_FORM;
  while (TRUE) {
    SpriteSideBarPickNew(doc,*_root,*_cur_elem_num);
    View;
    doc_e=doc->cur_entry;
    if (doc_e->type_u8==DOCT_MENU_VAL)
      *_cur_elem_num=doc_e->left_exp;
    *_root=SpriteSideBar2SpriteQue(doc,*_root,_cur_elem_num);
    DocClear(doc);
  }
}

Bool SpriteEdText(CSprite **_root,I64 *_cur_elem_num)
{
  Bool result;
  CSprite *root=*_root;
  U8 *elems=SpriteQue2Sprite(root);
  CDoc *doc=DocNew,*doc2,*old_put=DocPut;
  StrPrint(doc->filename.name,"AI:0x%X",doc);
  DocPrint(doc,"//$$PURPLE$$$$TX+CX,\"Sprite Edit as Text\"$$$$FG$$\n"
	"//$$LK+PU+CX,\"Click for Help\","
	"A=\"FI:::/Doc/SpriteEdText.TXT.Z\"$$\n\n");
  Sprite2Code(doc,elems);
  Free(elems);
  while (TRUE) {
    if (result=PopUpPrint("DocEd(0x%X,0x%X);",doc,0)) {
      Fs->put_doc=doc2=DocNew;
      "$$WW,1$$";
      if (elems=Code2Sprite(doc)) {
	DocDel(doc2);
	Fs->put_doc=old_put;
	QueDel(root);
	Free(root);
	root=Sprite2SpriteQue(elems);
	Free(elems);
	*_cur_elem_num=QueCnt(root); //TODO: Might want to improve this.
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
  if (_root) *_root=root;
  return result;
}

#define GED_SHIFT_PTS		0
#define GED_SHIFT_RECTS		1
#define GED_TEXT_ED		2
#define GED_TRANSFORM		3
#define GED_SET_ORIGIN		4
#define GED_SHIFT_ORIGIN	5
#define GED_SHIFT_SUB_ORIGIN	6
#define GED_PICK_GRAPHICALLY	7
#define GED_DELETE		8
#define GED_INS_CLIPBOARD	9
#define GED_MAIN_MENU		10
#define GED_EXIT		11

U0 GrInit3()
{
  DefineLstLoad("ST_SPRITE_ED_MENU","Shift Points\0Shift Rects\0"
	"Edit as Text\0Transform\0Set Origin\0Shift Origin\0Shift SubOrigin\0"
	"Pick Graphically\0Delete Object\0Insert Clipboard\0Main Menu\0");
}
GrInit3;

I64 PopUpSpriteEd(CSprite **_root,I64 *_cur_elem_num)
{
  U8 *st;
  CTask *pu_task;
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite Edit Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteEd.TXT.Z\"$$\n\n"
	"$$LTBLUE$$$$MU-UL,\"Shift Points\",LE=GED_SHIFT_PTS$$\n"
	"$$MU-UL,\"Shift Points with Rects\",LE=GED_SHIFT_RECTS$$\n"
	"$$MU-UL,\"Edit as Text\",LE=GED_TEXT_ED$$\n"
	"$$MU-UL,\"Transform\",LE=GED_TRANSFORM$$\n"
	"$$MU-UL,\"Set Origin\",LE=GED_SET_ORIGIN$$\n"
	"$$MU-UL,\"Shift Origin\",LE=GED_SHIFT_ORIGIN$$\n"
	"$$MU-UL,\"Insert Shift SubOrigin\",LE=GED_SHIFT_SUB_ORIGIN$$\n"
	"$$MU-UL,\"Pick or Delete Objects Graphically\","
	"LE=GED_PICK_GRAPHICALLY$$\n"
	"$$MU-UL,\"Delete\",LE=GED_DELETE$$\n"
	"$$MU-UL,\"Insert Clipboard Sprite's\",LE=GED_INS_CLIPBOARD$$\n"
	"\n$$PURPLE$$$$MU-UL,\"+] Sprite Main Menu\",LE=GED_MAIN_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=GED_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$");
  st=MStrPrint("SpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_root,_cur_elem_num);
  PopUp(st,NULL,&pu_task);
  Free(st);
  i=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_root=SpriteSideBar2SpriteQue(DocPut(pu_task),*_root,_cur_elem_num);
    Kill(pu_task);
  }
  DocDel(doc);
  return i;
}

#define GSPT_SIMPLE_PT		0
#define GSPT_WIDTH_HEIGHT	1

#define GSPF_SELECTED		1

class CEdSprite
{
  CEdSprite *next,*last;
  I32 type,num,flags,xx,yy,zz;
  I32 *x,*y,*z,*w,*h;
};

U0 SpritePtQueNew(U8 *elems,I64 x,I64 y,CEdSprite *root)
{
  I64 i,num=0;
  I32 *ptr;
  CD3I32 *p;
  CEdSprite *temppe;
  CSprite *tempg=elems-offset(CSprite.start);
  QueInit(root);
  while (tempg->type) {
    switch (tempg->type) {
      case SPT_ELLIPSE:
      case SPT_POLYGON:
	temppe=CAlloc(sizeof(CEdSprite));
	temppe->type=GSPT_WIDTH_HEIGHT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->pwha.x1;
	temppe->y=&tempg->pwha.y1;
	temppe->w=&tempg->pwha.width;
	temppe->h=&tempg->pwha.height;
	temppe->num=num;
	QueIns(temppe,root->last);
	goto pq_x1_y1;
      case SPT_RECT:
      case SPT_ROTATED_RECT:
      case SPT_LINE:
      case SPT_ARROW:
      case SPT_PLANAR_SYMMETRY:
	temppe=CAlloc(sizeof(CEdSprite));
	temppe->type=GSPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->pp.x2;
	temppe->y=&tempg->pp.y2;
	temppe->num=num;
	QueIns(temppe,root->last);
      case SPT_TEXT:
      case SPT_TEXT_BOX:
      case SPT_TEXT_DIAMOND:
      case SPT_PT:
      case SPT_BITMAP:
      case SPT_FLOOD_FILL:
      case SPT_FLOOD_FILL_NOT:
      case SPT_CIRCLE:
pq_x1_y1:
	temppe=CAlloc(sizeof(CEdSprite));
	temppe->type=GSPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->p.x1;
	temppe->y=&tempg->p.y1;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
      case SPT_SHIFT:
	x+=tempg->p.x1;
	y+=tempg->p.y1;
	break;
      case SPT_POLYLINE:
	ptr=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++) {
	  temppe=CAlloc(sizeof(CEdSprite));
	  temppe->type=GSPT_SIMPLE_PT;
	  temppe->xx=x;
	  temppe->yy=y;
	  temppe->x=&ptr[i<<1];
	  temppe->y=&ptr[i<<1+1];
	  temppe->num=num;
	  QueIns(temppe,root->last);
	}
	break;
      case SPT_POLYPT:
	temppe=CAlloc(sizeof(CEdSprite));
	temppe->type=GSPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->npu.x;
	temppe->y=&tempg->npu.y;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
      case SPT_BSPLINE2:
      case SPT_BSPLINE3:
      case SPT_BSPLINE2_CLOSED:
      case SPT_BSPLINE3_CLOSED:
	p=&tempg->nu.u;
	for (i=0;i<tempg->nu.num;i++) {
	  temppe=CAlloc(sizeof(CEdSprite));
	  temppe->type=GSPT_SIMPLE_PT;
	  temppe->xx=x;
	  temppe->yy=y;
	  temppe->x=&p[i].x;
	  temppe->y=&p[i].y;
	  temppe->z=&p[i].z;
	  temppe->num=num;
	  QueIns(temppe,root->last);
	}
	break;
      case SPT_MESH:
	break;
      case SPT_SHIFTABLE_MESH:
	temppe=CAlloc(sizeof(CEdSprite));
	temppe->type=GSPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->pmu.x;
	temppe->y=&tempg->pmu.y;
	temppe->z=&tempg->pmu.z;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
    }
    tempg(U8 *)+=SpriteElemSize(tempg);
    num++;
  }
}

U0 SpriteCtrlPtsDraw(CDC *dc,CEdSprite *root)
{
  I64 x,y;
  CEdSprite *temppe;
  WinMgrSync;
  DCFill(dc);
  if (Blink(20)) {
    temppe=root->next;
    while (temppe!=root) {
      switch (temppe->type) {
	case GSPT_SIMPLE_PT:
	  x=*temppe->x+temppe->xx;
	  y=*temppe->y+temppe->yy;
	  break;
	case GSPT_WIDTH_HEIGHT:
	  x=*temppe->w+*temppe->x+temppe->xx;
	  y=*temppe->h+*temppe->y+temppe->yy;
	  break;
      }
      if (temppe->flags&GSPF_SELECTED)
	dc->color=RED;
      else
	dc->color=BLACK;
      GrRect(dc,x-2,y-2,4,4);
      dc->color=WHITE;
      GrRect(dc,x-1,y-1,2,2);
      temppe=temppe->next;
    }
  }
}

U0 SpriteCtrlPtsMove(CEdSprite *root,I64 dx,I64 dy)
{
  CEdSprite *temppe;
  temppe=root->next;
  while (temppe!=root) {
    if (temppe->flags&GSPF_SELECTED)
      switch (temppe->type) {
	case GSPT_SIMPLE_PT:
	  if (temppe->x) *temppe->x+=dx;
	  if (temppe->y) *temppe->y+=dy;
	  break;
	case GSPT_WIDTH_HEIGHT:
	  if (temppe->w) *temppe->w+=dx;
	  if (temppe->h) *temppe->h+=dy;
	  break;
      }
    temppe=temppe->next;
  }
}

Bool SpriteShiftPts(U8 *elems,I64 x,I64 y,I64 *_cur_elem_num,I64 mode)
{
  I64 msg_code,a1,a2,xx,yy,xx2,yy2,dd,best_dd,cur_elem_num;
  Bool result=TRUE;
  CDC *dc=DCAlias;
  CEdSprite root,*temppe,*best_pe;

  SpritePtQueNew(elems,x,y,&root);
  cur_elem_num=0;
  if (root.next!=&root) {
    while (TRUE) {
      SpriteCtrlPtsDraw(dc,&root); //has WinMgrSync
      switch (msg_code=ScanMsg(&a1,&a2,
	    1<<MSG_IP_R_UP|1<<MSG_IP_L_DOWN|1<<MSG_KEY_DOWN)) {
	case MSG_IP_L_DOWN:
	  switch (mode) {
	    case GED_SHIFT_PTS:
	      xx=a1; yy=a2;
	      best_dd=MAX_I64;
	      temppe=root.next;
	      while (temppe!=&root) {
		switch (temppe->type) {
		  case GSPT_SIMPLE_PT:
		    dd=SqrI64(*temppe->x+temppe->xx-xx)+
			  SqrI64(*temppe->y+temppe->yy-yy);
		    break;
		  case GSPT_WIDTH_HEIGHT:
		    dd=SqrI64(*temppe->x+*temppe->w+temppe->xx-xx)+
			  SqrI64(*temppe->y+*temppe->h+temppe->yy-yy);
		    break;
		}
		if (dd<best_dd) {
		  best_dd=dd;
		  best_pe=temppe;
		}
		temppe=temppe->next;
	      }
	      cur_elem_num=best_pe->num;
	      best_pe->flags|=GSPF_SELECTED;
	      break;
	    case GED_SHIFT_RECTS:
	      xx2=xx=a1; yy2=yy=a2;
	      while (TRUE) {
		SpriteCtrlPtsDraw(dc,&root);
		dc->color=ROPF_DITHER+WHITE<<16+RED;
		GrBorder(dc,xx,yy,xx2,yy2);
		if (msg_code=ScanMsg(&a1,&a2,1<<MSG_IP_MOVE|1<<MSG_IP_L_UP)) {
		  if (msg_code==MSG_IP_MOVE) {
		    xx2=a1; yy2=a2;
		  } else
		    break;
		}
	      }
	      if (xx2<xx) SwapI64(&xx,&xx2);
	      if (yy2<yy) SwapI64(&yy,&yy2);
	      temppe=root.next;
	      while (temppe!=&root) {
		switch (temppe->type) {
		  case GSPT_SIMPLE_PT:
		    if (xx<=*temppe->x+temppe->xx<=xx2 &&
			  yy<=*temppe->y+temppe->yy<=yy2)
		      temppe->flags|=GSPF_SELECTED;
		    break;
		  case GSPT_WIDTH_HEIGHT:
		    if (xx<=*temppe->x+*temppe->w+temppe->xx<=xx2 &&
			  yy<=*temppe->y+*temppe->h+temppe->yy<=yy2)
		      temppe->flags|=GSPF_SELECTED;
		    break;
		}
		temppe=temppe->next;
	      }
	      do {
		SpriteCtrlPtsDraw(dc,&root);
		msg_code=ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN);
		if (msg_code==MSG_KEY_DOWN) goto gs_key;
	      } while (msg_code!=MSG_IP_L_DOWN);
	      xx=a1;yy=a2;
	      break;
	  }
	  do {
	    SpriteCtrlPtsDraw(dc,&root);
	    if (msg_code=ScanMsg(&a1,&a2,1<<MSG_IP_MOVE|1<<MSG_IP_L_UP)) {
	      SpriteCtrlPtsMove(&root,a1-xx,a2-yy);
	      xx=a1;yy=a2;
	    }
	  } while (msg_code!=MSG_IP_L_UP);
	  temppe=root.next;
	  while (temppe!=&root) {
	    temppe->flags&=~GSPF_SELECTED;
	    temppe=temppe->next;
	  }
	  break;
	case MSG_KEY_DOWN:
gs_key:
	  switch (a1.u8[0]) {
	    case CH_SHIFT_ESC:
	      result=FALSE;
	    case CH_ESC:
	      GetMsg(&a1,&a2,1<<MSG_KEY_UP);
	      goto gs_done;
	    case 'p':
	    case 'P':
	      mode=GED_SHIFT_PTS;
	      break;
	    case 'r':
	    case 'R':
	      mode=GED_SHIFT_RECTS;
	      break;
	  }
	  break;
	case MSG_IP_R_UP:
	  goto gs_done;
      }
    }
gs_done:
    QueDel(&root,TRUE);
  }
  DCFill(dc);
  DCDel(dc);
  if (_cur_elem_num && result)
    *_cur_elem_num=cur_elem_num;
  return result;
}

I64 SpriteEd(CDoc *doc,CDocEntry *doc_e,I64 x,I64 y,
	CSprite **_root,I64 *_cur_elem_num)
{
  CDocEntry *doc_e2;
  CDocBin *tempb;
  Bool unlock,del_down;
  I64 i,r[16],msg_code,a1,a2,xx,yy,
	old_de_flags;
  CSprite *cur_elem,*root2,*next,*last,*tempg,*insert_pt;

  old_de_flags=doc_e->de_flags;
  tempb=doc_e->bin_data;
  DocUnlock(doc);
  do {
    if (winmgr.fps<10)
      doc_e->de_flags|=DOCEF_DONT_DRAW;
    StrCpy(Fs->task_title,"Sprite Edit Menu");
    i=PopUpSpriteEd(_root,_cur_elem_num);
    SpriteEdUpdate(doc,tempb,*_root);
    if (0<=i<GED_EXIT) {
      StrCpy(Fs->task_title,DefineSub(i,"ST_SPRITE_ED_MENU"));
      switch (i) {
	case GED_PICK_GRAPHICALLY:
	  RegOneTimePopUp(ARf_CSPRITE_PICK_GRAPHICALLY,
		"You can delete elements by\n"
		"holding $$GREEN$$Delete$$FG$$ and clicking.\n");
	  doc_e->de_flags=old_de_flags;
	  doc->cur_sprite=tempb->data;
	  doc->nearest_sprite_elem_num=0;
	  del_down=FALSE;
	  do {
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|1<<MSG_KEY_DOWN|1<<MSG_KEY_UP);
	    if (a2.u8[0]==SC_DELETE&& !(a2&SCF_IP_L_DOWN)) {
	      if (msg_code==MSG_KEY_DOWN)
		del_down=TRUE;
	      else if (msg_code==MSG_KEY_UP)
		del_down=FALSE;
	    }
	    if (msg_code==MSG_IP_L_UP && kbd.scan_code&SCF_DELETE ||
		  msg_code==MSG_KEY_DOWN && a2&SCF_IP_L_DOWN &&
		  a2.u8[0]==SC_DELETE) {
	      *_cur_elem_num=doc->nearest_sprite_elem_num;
	      cur_elem=SpriteElemNum2QueEntry(*_root,*_cur_elem_num);
	      if (cur_elem!=*_root) {
		QueRem(cur_elem);
		Free(cur_elem);
		SpriteEdUpdate(doc,tempb,*_root);
	      }
	      doc->cur_sprite=tempb->data;
	      doc->nearest_sprite_elem_num=0;
	      if (msg_code==MSG_KEY_DOWN)
		GetMsg(&a1,&a2,1<<MSG_IP_L_UP);
	    }
	  } while (del_down || msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
	  *_cur_elem_num=doc->nearest_sprite_elem_num;
	  doc->cur_sprite=NULL;
	  break;
	case GED_SET_ORIGIN:
	  doc_e->de_flags=old_de_flags;
	  GetMsg(&a1,&a2,1<<MSG_IP_L_UP);
	  SpriteSetOrigin(*_root,x-a1,y-a2,0);
	  SpriteEdUpdate(doc,tempb,*_root);
	  break;
	case GED_SHIFT_ORIGIN:
	  doc_e->de_flags=old_de_flags;
	  GetMsg(&a1,&a2,1<<MSG_IP_L_DOWN);
	  xx=a1; yy=a2;
	  do {
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    SpriteSetOrigin(*_root,a1-xx,a2-yy,0);
	    xx=a1; yy=a2;
	    SpriteEdUpdate(doc,tempb,*_root);
	  } while (msg_code!=MSG_IP_L_UP);
	  break;
	case GED_SHIFT_PTS:
	case GED_SHIFT_RECTS:
	  RegOneTimePopUp(ARf_CSPRITE_PTS_RECTANGLES,
		"You can switch between points\n"
		"and rectangles with '$$GREEN$$p$$FG$$' and '$$GREEN$$r$$FG$$'.\n"
		"Press '$$GREEN$$r$$FG$$' after one rectangle\n"
		"to OR another rectangle.\n");
	  doc_e->de_flags=old_de_flags;
	  if (SpriteShiftPts(tempb->data,x,y,_cur_elem_num,i)) {
	    QueDel(*_root);
	    Free(*_root);
	    *_root=Sprite2SpriteQue(tempb->data);
	  } else
	    SpriteEdUpdate(doc,tempb,*_root);
	  break;
	case GED_TRANSFORM:
	  if (PopUpTransform(r)) {
	    SpriteTransform(r,*_root);
	    SpriteEdUpdate(doc,tempb,*_root);
	  }
	  break;
	case GED_SHIFT_SUB_ORIGIN:
	  doc_e->de_flags=old_de_flags;
	  insert_pt=SpriteElemNum2QueEntry(*_root,*_cur_elem_num);
	  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_SHIFT));
	  tempg->type=SPT_SHIFT;
	  tempg->p.x1=0;
	  tempg->p.y1=0;
	  QueIns(tempg,insert_pt->last);
	  GetMsg(&a1,&a2,1<<MSG_IP_L_DOWN);
	  xx=a1; yy=a2;
	  do {
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    tempg->p.x1=a1-xx;
	    tempg->p.y1=a2-yy;
	    SpriteEdUpdate(doc,tempb,*_root);
	  } while (msg_code!=MSG_IP_L_UP);
	  *_cur_elem_num+=1;
	  break;
	case GED_INS_CLIPBOARD:
	  RegOneTimePopUp(ARf_CSPRITE_INS_CLIPBOARD,
		"You will probably want to shift around\n"
		"the location of element groups.  Use\n"
		"'Insert shift sub-origin' after picking the\n"
		"element to insert before.  Or,\n"
		"use 'shift points'.\n");
	  insert_pt=SpriteElemNum2QueEntry(*_root,*_cur_elem_num);
	  unlock=DocLock(sys_clipboard_doc);
	  doc_e2=sys_clipboard_doc->root.next;
	  while (doc_e2!=sys_clipboard_doc) {
	    if (doc_e2->type_u8==DOCT_SPRITE) {
	      root2=Sprite2SpriteQue(doc_e2->bin_data->data);
	      if (root2->next!=root2) {
		tempg=root2->next;
		while (tempg!=root2) {
		  *_cur_elem_num+=1;
		  tempg=tempg->next;
		}
		next=root2->next;
		last=root2->last;
		insert_pt->last->next=next;
		next->last=insert_pt->last;
		insert_pt->last=last;
		last->next=insert_pt;
	      }
	      Free(root2);
	    }
	    doc_e2=doc_e2->next;
	  }
	  if (unlock)
	    DocUnlock(sys_clipboard_doc);
	  SpriteEdUpdate(doc,tempb,*_root);
	  break;
	case GED_DELETE:
	  cur_elem=SpriteElemNum2QueEntry(*_root,*_cur_elem_num);
	  if (cur_elem!=*_root) {
	    QueRem(cur_elem);
	    Free(cur_elem);
	    SpriteEdUpdate(doc,tempb,*_root);
	  }
	  break;
	case GED_TEXT_ED:
	  if (SpriteEdText(_root,_cur_elem_num))
	    SpriteEdUpdate(doc,tempb,*_root);
	  break;
      }
    }
  } while (i!=DOCM_CANCEL && i!=GED_EXIT && i!=GED_MAIN_MENU);
  doc_e->de_flags=old_de_flags;

  switch (i) {
    case DOCM_CANCEL:	return GE_ABORT;
    case GED_EXIT:	return GE_EXIT;
    case GED_MAIN_MENU:	return GE_CONT;
  }
}
