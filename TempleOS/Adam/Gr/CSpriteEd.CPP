#help_index "Graphics/CSprite"

#define CSPRITE_WIF	(WIF_ALL-WIF_BORDER-WIF_GRAB_SCROLL)

U8 GrSpOffset[9]={0,1,2,3,0,4,5,6,7};

Bool GrPolyPtPlot(CSprite *root,I64 x,I64 y,I64)
{
  CSprite *tempg=CAlloc(gr_sprite_qued_base_size[GRST_PT]);
  tempg->type=GRST_PT;
  tempg->x1=x;
  tempg->y1=y;
  QueIns(tempg,root->last);
  return TRUE;
}

CSprite *GrNum2Que(CSprite *root,I64 num,I64 *x=NULL,I64 *y=NULL)
{
  I64 xx=0,yy=0;
  CSprite *result=root->next;
  while (result!=root && num--) {
    if (result->type==GRST_SHIFT) {
      xx+=result->x1;
      yy+=result->y1;
    }
    result=result->next;
  }
  if (x) *x=xx;
  if (y) *y=yy;
  return result;
}

CSprite *Sprite2Que(U8 *elems)
{
  I64 s;
  CSprite *result=CAlloc(sizeof(CSprite)),
	 *tempg=elems-offset(CSprite.start),
	 *tempg1;
  result->next=result->last=result;
  while (tempg->type) {
    tempg1=MAlloc(SpriteElemSize(tempg)+offset(CSprite.start));
    s=SpriteElemSize(tempg);
    MemCpy(&tempg1->start,&tempg->start,s);
    QueIns(tempg1,result->last);
    tempg(U8 *)+=s;
  }
  return result;
}

I64 SpriteCntQue(CSprite *root)
{
  CSprite *tempg=root->next;
  I64 result=0;
  while (tempg->type) {
    result++;
    tempg=tempg->next;
  }
  return result;
}

U0 GrEdUpdate(CDoc *doc,CDocBin *tempb,CSprite *root)
{
  I64 i,size=gr_sprite_base_size[GRST_END];
  CSprite *tempg=root->next;
  U8 *dst;
  Bool unlock=DocLock(doc);

  while (tempg!=root) {
    size+=SpriteElemSize(tempg);
    tempg=tempg->next;
  }

  Free(tempb->data);
  tempb->data=dst=MAlloc(size);
  tempb->size=size;

  tempg=root->next;
  while (tempg!=root) {
    i=SpriteElemSize(tempg);
    MemCpy(dst,&tempg->start,i);
    dst+=i;
    tempg=tempg->next;
  }
  *dst=GRST_END;

  if (unlock)
    DocUnlock(doc);
}

U0 GrSetOrigin(CSprite *root,I64 dx,I64 dy,I64 dz)
{
  I64 i;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tempg=root->next;
  while (tempg!=root) {
    switch (tempg->type) {
      case GRST_PT:
      case GRST_FLOOD_FILL:
      case GRST_FLOOD_FILL_NOT:
	tempg->x1+=dx;
	tempg->y1+=dy;
	break;
      case GRST_ARROW:
      case GRST_LINE:
      case GRST_PLANAR_SYMMETRY:
	tempg->x1+=dx;
	tempg->y1+=dy;
	tempg->x2+=dx;
	tempg->y2+=dy;
	break;
      case GRST_BITMAP8:
      case GRST_ELLIPSE:
      case GRST_POLYGON:
	tempg->x2+=dx;
	tempg->y2+=dy;
	break;
      case GRST_RECT:
      case GRST_ROTATED_RECT:
	tempg->x1+=dx;
	tempg->y1+=dy;
	tempg->x2+=dx;
	tempg->y2+=dy;
	break;
      case GRST_TEXT:
      case GRST_TEXT_BOX:
      case GRST_TEXT_DIAMOND:
      case GRST_CIRCLE:
	tempg->x1+=dx;
	tempg->y1+=dy;
	break;
      case GRST_POLYLINE:
	ptr=&tempg->y1;
	for (i=0;i<tempg->num;i++) {
	  ptr[i<<1]+=dx;
	  ptr[i<<1+1]+=dy;
	}
	break;
      case GRST_POLYPT:
	ptr=&tempg->y1;
	ptr[0]+=dx;
	ptr[1]+=dy;
	break;
      case GRST_BSPLINE2:
      case GRST_BSPLINE3:
      case GRST_BSPLINE2_CLOSED:
      case GRST_BSPLINE3_CLOSED:
	p=&tempg->y1;
	for (i=0;i<tempg->num;i++,p++) {
	  p->x+=dx;
	  p->y+=dy;
	  p->z+=dz;
	}
	break;
      case GRST_MESH:
	p=&tempg->x2;
	for (i=0;i<tempg->vertex_cnt;i++,p++) {
	  p->x+=dx;
	  p->y+=dy;
	  p->z+=dz;
	}
	break;
      case GRST_SHIFTABLE_MESH:
	tempg->x1+=dx;
	tempg->y1+=dy;
	tempg->z1+=dz;
	break;
    }
    tempg=tempg->next;
  }
}


CSprite *GrTransformCircle(I64 r[4][4],CSprite *tempg)
{
  I64 x,y,z;
  F64 m1,a1,m2,radius=tempg->x2<<16;
  CSprite *tempg1=CAlloc(gr_sprite_qued_base_size[GRST_ELLIPSE]);
  tempg1->type=GRST_ELLIPSE;

  x=tempg->x1; y=tempg->y1; z=0;
  GrRot(r,&x,&y,&z);
  tempg1->x2=x;
  tempg1->y2=y;

  x=radius; y=0; z=0;
  GrRot(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=0; y=radius; z=0;
  GrRot(r,&x,&y,&z);
  m2=Sqrt(x*x+y*y);

  tempg1->width =ToI64(m1)/0x10000;
  tempg1->height=ToI64(m2)/0x10000;
  tempg1->rot_angle=-a1;

  return tempg1;
}

CSprite *GrTransformEllipse(I64 r[4][4],CSprite *tempg)
{
  I64 x,y,z;
  F64 m1,a1,m2,a2,s,c,x_radius=tempg->width<<16,y_radius=tempg->height<<16;
  CSprite *tempg1=CAlloc(gr_sprite_qued_base_size[tempg->type]);
  tempg1->type=tempg->type;
  if (tempg->type==GRST_POLYGON)
    tempg1->sides=tempg->sides;

  x=tempg->x2; y=tempg->y2; z=0;
  GrRot(r,&x,&y,&z);
  tempg1->x2=x;
  tempg1->y2=y;

  c=Cos(-tempg->rot_angle);
  s=Sin(-tempg->rot_angle);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  GrRot(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  GrRot(r,&x,&y,&z);
  R2P(&m2,&a2,x,y);
  m2*=Abs(Sin(a2-a1));

  tempg1->width=ToI64(m1)/0x10000;
  if (tempg1->width<1) tempg1->width=1;
  tempg1->height=ToI64(m2)/0x10000;
  if (tempg1->height<1) tempg1->height=1;
  tempg1->rot_angle=-a1;

  return tempg1;
}

CSprite *GrTransformRect(I64 r[4][4],CSprite *tempg,F64 é)
{
  I64 x,y,z,w,h;
  F64 m1,a1,m2,a2,s,c,
    x_radius=(tempg->x2-tempg->x1)<<16,
    y_radius=(tempg->y2-tempg->y1)<<16;
  CSprite *tempg1=CAlloc(gr_sprite_qued_base_size[GRST_ROTATED_RECT]);
  tempg1->type=GRST_ROTATED_RECT;

  x=tempg->x1; y=tempg->y1; z=0;
  GrRot(r,&x,&y,&z);
  tempg1->x1=x;
  tempg1->y1=y;

  c=Cos(-é);
  s=Sin(-é);

  x=x_radius*c;
  y=x_radius*s;
  z=0;
  GrRot(r,&x,&y,&z);
  R2P(&m1,&a1,x,y);

  x=-y_radius*s;
  y=y_radius*c;
  z=0;
  GrRot(r,&x,&y,&z);
  R2P(&m2,&a2,x,y);
  m2*=Abs(Sin(a2-a1));

  w=ToI64(m1)/0x10000;
  if (w<1) w=1;
  h=ToI64(m2)/0x10000;
  if (h<1) h=1;
  tempg1->x2=tempg1->x1+w;
  tempg1->y2=tempg1->y1+h;
  tempg1->rot_angle=-a1;

  return tempg1;
}

CSprite *GrTransformBitMap(I64 r[4][4],CSprite *tempg)
{
  CDC *img,*tempb;
  U8 *elems;
  I64 x,y,z,minx,maxx,miny,maxy,minz,maxz;
  CSprite *tempg1;


  x=0; y=0; z=0;
  GrRot(r,&x,&y,&z);
  minx=maxx=x;
  miny=maxy=y;
  minz=maxz=z;

  x=0; y=tempg->height; z=0;
  GrRot(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  x=tempg->width; y=0; z=0;
  GrRot(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  x=tempg->width; y=tempg->height; z=0;
  GrRot(r,&x,&y,&z);
  if (x<minx) minx=x;
  if (x>maxx) maxx=x;
  if (y<minx) miny=y;
  if (y>maxx) maxy=y;
  if (z<minx) minz=z;
  if (z>maxx) maxz=z;

  tempb=DCNew(DCT_COLOR8,maxx-minx+1,maxy-miny+1);

  img=CAlloc(sizeof(CDC));
  img->type=DCT_COLOR8;
  img->width=tempg->width;
  img->width_internal=(tempg->width+7)&~7;
  img->height=tempg->height;
  img->plane_size=(img->width_internal*img->height)>>3;
  img->body=tempg(U8 *)+offset(CSprite.linespeed);
  img->dc_signature=DCS_SIGNATURE_VAL;

  tempb->color=COLOR_TRANSPARENT;
  GrRect(tempb,0,0,maxx-minx+1,maxy-miny+1);

  Free(tempb->r);
  GrSetRotMat(tempb,r);
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

  x=tempg->x2; y=tempg->y2; z=0;
  GrRot(r,&x,&y,&z);
  tempg1->x2=x;
  tempg1->y2=y;

  x=0; y=0; z=0;
  GrRot(r,&x,&y,&z);
  tempg1->x2+=minx-x;
  tempg1->y2+=miny-y;

  return tempg1;
}


U0 SpriteTransform(I64 r[4][4],CSprite *root)
{
  I64 i,j,k,num,
      x,y,z,x1,y1,z1,x2,y2,z2,x3,y3,z3;
  I32 *ptr;
  CD3I32 *p;
  CSprite *tempg=root->next,root2,*tempg1,*tempg2,*tempg3;
  while (tempg!=root) {
    switch (tempg->type) {
      case GRST_WIDTH:
	// (1.0/Sqrt(3))*65536=37837.22
	x=37838;y=37838;z=37838;
	GrRot(r,&x,&y,&z);
	//scaled 32 bits
	tempg->width*=Sqrt(x*x+y*y+z*z)/65536;
	if (tempg->width<0) tempg->width=0;
	break;
      case GRST_PT:
      case GRST_FLOOD_FILL:
      case GRST_FLOOD_FILL_NOT:
	x=tempg->x1; y=tempg->y1; z=0;
	GrRot(r,&x,&y,&z);
	tempg->x1=x;
	tempg->y1=y;
	break;
      case GRST_PLANAR_SYMMETRY:
      case GRST_ARROW:
      case GRST_LINE:
	x=tempg->x1; y=tempg->y1; z=0;
	GrRot(r,&x,&y,&z);
	tempg->x1=x;
	tempg->y1=y;
	x=tempg->x2; y=tempg->y2; z=0;
	GrRot(r,&x,&y,&z);
	tempg->x2=x;
	tempg->y2=y;
	break;
      case GRST_BITMAP8:
	tempg1=GrTransformBitMap(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case GRST_ROTATED_RECT:
	tempg1=GrTransformRect(r,tempg,tempg->rot_angle);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case GRST_RECT:
	tempg1=GrTransformRect(r,tempg,0);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case GRST_TEXT:
      case GRST_TEXT_BOX:
      case GRST_TEXT_DIAMOND:
	x=tempg->x1; y=tempg->y1; z=0;
	GrRot(r,&x,&y,&z);
	tempg->x1=x;
	tempg->y1=y;
	break;
      case GRST_CIRCLE:
	tempg1=GrTransformCircle(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case GRST_ELLIPSE:
      case GRST_POLYGON:
	tempg1=GrTransformEllipse(r,tempg);
	QueIns(tempg1,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg1;
	break;
      case GRST_POLYLINE:
	ptr=&tempg->y1;
	for (i=0;i<tempg->num;i++) {
	  x=ptr[i<<1]; y=ptr[i<<1+1]; z=0;
	  GrRot(r,&x,&y,&z);
	  ptr[i<<1]=x;
	  ptr[i<<1+1]=y;
	}
	break;
      case GRST_POLYPT:
	QueInit(&root2);
	ptr=&tempg->y1;
	x=ptr[0]; y=ptr[1]; z=0;
	x1=x; y1=y; z1=z;  //unrotated cur coordinates
	GrRot(r,&x,&y,&z);
	ptr+=2;
	k=tempg->num*3;
	x2=x; y2=y; z2=z;  //rotated start coordinates
	x3=x; y3=y; z3=z;  //lag 1 rotated coordinates
	for (i=0;i<k;i+=3) {
	  j=BFieldExtU32(ptr,i,3);
	  x1+=gr_x_offsets[j];
	  y1+=gr_y_offsets[j];
	  x=x1; y=y1; z=z1;
	  GrRot(r,&x,&y,&z);
	  Line(&root2,x3-x2,y3-y2,0,x-x2,y-y2,0,&GrPolyPtPlot);
	  x3=x; y3=y; z3=z;
	}

	num=0;
	tempg1=root2.next;
	x3=0; y3=0; z3=0;
	while (tempg1!=&root2) {
	  tempg2=tempg1->next;
	  if (tempg1->x1==x3 && tempg1->y1==y3) {
	    QueRem(tempg1);
	    Free(tempg1);
	  } else {
	    num++;
	    x3=tempg1->x1;
	    y3=tempg1->y1;
	  }
	  tempg1=tempg2;
	}

	tempg3=CAlloc(gr_sprite_qued_base_size[GRST_POLYPT]+
	    sizeof(I32)*2+(num*3+7)>>3);
	ptr=&tempg3->y1;
	ptr[0]=x2;
	ptr[1]=y2;
	ptr+=2;
	x3=0;y3=0; z3=0;
	i=0;
	tempg1=root2.next;
	while (tempg1!=&root2) {
	  tempg2=tempg1->next;
	  BFieldOrU32(ptr,i,
	      GrSpOffset[SignI64(tempg1->x1-x3)+1+
	      3*(SignI64(tempg1->y1-y3)+1)]);
	  i+=3;
	  x3=tempg1->x1;y3=tempg1->y1;
	  QueRem(tempg1);
	  Free(tempg1);
	  tempg1=tempg2;
	}
	tempg3->type=GRST_POLYPT;
	tempg3->num=num;
	QueIns(tempg3,tempg);
	QueRem(tempg);
	Free(tempg);
	tempg=tempg3;
	break;
      case GRST_BSPLINE2:
      case GRST_BSPLINE3:
      case GRST_BSPLINE2_CLOSED:
      case GRST_BSPLINE3_CLOSED:
	p=&tempg->y1;
	for (i=0;i<tempg->num;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  GrRot(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
      case GRST_SHIFTABLE_MESH:
	x=tempg->x1; y=tempg->y1; z=tempg->z1;
	GrRot(r,&x,&y,&z);
	tempg->x1=x;
	tempg->y1=y;
	tempg->z1=z;
	p=(&tempg->shiftable_tri_cnt)(U8 *)+sizeof(I32);
	for (i=0;i<tempg->shiftable_vertex_cnt;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  GrRot(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
      case GRST_MESH:
	p=&tempg->x2;
	for (i=0;i<tempg->vertex_cnt;i++,p++) {
	  x=p->x; y=p->y; z=p->z;
	  GrRot(r,&x,&y,&z);
	  p->x=x;
	  p->y=y;
	  p->z=z;
	}
	break;
    }
    tempg=tempg->next;
  }
}

CSprite *SpriteDoc2Que(CDoc *doc,CSprite *root,I64 *_cur_num)
{
  CSprite *result=CAlloc(sizeof(CSprite)),*tempg;
  CDocEntry *doc_e=doc->root.next;
  Bool found=FALSE;
  I64 num=0;
  result->next=result->last=result;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL && doc_e->left_exp>=0) {
      tempg=GrNum2Que(root,doc_e->left_exp);
      if (tempg!=root) {
	tempg=MAllocIdentical(tempg);
	if (*_cur_num==doc_e->left_exp) {
	  *_cur_num=num;
	  found=TRUE;
	}
	QueIns(tempg,result->last);
	num++;
      }
    }
    doc_e=doc_e->next;
  }
  if (!found)
    *_cur_num=num;
  QueDel(root);
  Free(root);
  return result;
}

U0 PickSpriteDocNew(CDoc *doc,CSprite *root,I64 old_num)
{
  CSprite *tempg;
  CDocEntry *doc_e=NULL;
  I64 cur_num=0;
  U8 *st;

  DocPutS(doc,"$$FG,PURPLE$$$$TX+CX,\"Sprite SideBar\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",\"FI:::/TempleOS/Doc/SpriteSideBar.TXT.Z\"$$\n\n");

  tempg=root->next;
  while (tempg!=root) {
    st=Sprite2Text(tempg);
    if (cur_num==old_num) {
      DocPutS(doc,"$$FG,LTRED$$");
      doc_e=DocPrintF(doc,"$$MU-UL,\"%Q\",%d$$\n",st,cur_num++);
    } else {
      DocPutS(doc,"$$FG,LTBLUE$$");
      DocPrintF(doc,"$$MU-UL,\"%Q\",%d$$\n",st,cur_num++);
    }
    Free(st);
    tempg=tempg->next;
  }
  if (cur_num==old_num) {
    DocPutS(doc,"$$FG,LTRED$$");
    doc_e=DocPrintF(doc,"$$MU-UL,\"END\",%d$$\n",cur_num);
  } else {
    DocPutS(doc,"$$FG,LTBLUE$$");
    DocPrintF(doc,"$$MU-UL,\"END\",%d$$\n",cur_num);
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
      SetWinHorz(pu_task->win_right+2,pu_task->win_right+2+w,parent);
      h=parent->win_bottom-parent->win_top;
      SetWinVert(pu_task->win_top,pu_task->win_top+h,parent);
    }
    WinMgrSync;
  }
}

U0 SpriteSideBarTask(CTask *parent,CSprite **_root,I64 *_cur_num)
{
  CDocEntry *doc_e;
  CDoc *doc=DocPut;
  Spawn(&SpriteSideBarTask2,parent,"CSpriteTask",,Fs);
  doc->flags|=DOCF_MIN_SIZE|DOCF_FORM;
  while (TRUE) {
    PickSpriteDocNew(doc,*_root,*_cur_num);
    View;
    doc_e=doc->cur_entry;
    if (doc_e->type_u8==DOCT_MENU_VAL)
      *_cur_num=doc_e->left_exp;
    *_root=SpriteDoc2Que(doc,*_root,_cur_num);
    DocClear(doc);
  }
}

U8 *SpriteHexDoc2Sprite(CDoc *doc,I64 *_cur_num,CTask *mem_task=NULL)
{
  I64 num=0,size=gr_sprite_base_size[GRST_END];
  U8 *result,*dst;
  Bool unlock_doc=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_HEX_ED)
      size+=doc_e->len;
    doc_e=doc_e->next;
  }
  *_cur_num=0;
  dst=result=MAlloc(size,mem_task);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_HEX_ED) {
      if (doc_e==doc->cur_entry)
	*_cur_num=num;
      MemCpy(dst,doc_e->data,doc_e->len);
      dst+=doc_e->len;
      num++;
    }
    doc_e=doc_e->next;
  }
  *dst=GRST_END;
  if (unlock_doc)
    DocUnlock(doc);
  return result;
}

Bool SpriteTextEd(CSprite **_root,I64 *_cur_num)
{
  Bool result;
  U8 *st;
  CDoc *doc=DocNew;
  CSprite *root=*_root,*tempg;
  CDocEntry *doc_e;
  doc->flags|=DOCF_FORM;
  DocPutS(doc,"$$FG,PURPLE$$$$TX+CX,\"Sprite Edit as Text\"$$$$FG$$\n"
	"$$LK+PU+CX,\"Click for Help\",\"FI:::/TempleOS/Doc/SpriteEdText.TXT.Z\"$$\n\n");
  tempg=root->next;
  while (tempg!=root) {
    st=Sprite2Text(tempg);
    DocPrintF(doc,"$$FG,BLUE$$%s$$FG$$\n",st);
    Free(st);
    doc_e=DocPrintF(doc,"$$HX,%d,16$$",SpriteElemSize(tempg));
    doc_e->data=&tempg->start;
    tempg=tempg->next;
  }
  if (result=SrvUser("DocDo(0x%X,0x%X);",doc,0)) {
    tempg=SpriteHexDoc2Sprite(doc,_cur_num);
    QueDel(root);
    Free(root);
    root=Sprite2Que(tempg);
  }
  DocDel(doc);
  if (_root) *_root=root;
  return result;
}

#define GED_PICK_GRAPHICALLY	0
#define GED_SET_ORIGIN		1
#define GED_SHIFT_ORIGIN	2
#define GED_SHIFT_PTS		3
#define GED_SHIFT_RECTS		4
#define GED_TRANSFORM		5
#define GED_SHIFT_SUB_ORIGIN	6
#define GED_MAIN_MENU		7
#define GED_INS_CLIPBOARD	8
#define GED_DELETE		9
#define GED_TEXT_ED		10
#define GED_SNAP		11
#define GED_EXIT		12

U0 GrInit2()
{
  DefineLstLoad("ST_SPRITE_ED_MENU","Pick Graphically\0Set Origin\0"
	"Shift Origin\0Shift Points\0Shift Rects\0Transform\0Shift SubOrigin\0Main Menu\0"
	"Insert Clipboard\0Delete Object\0Edit as Text\0Snap\0");
}
GrInit2;

I64 PopUpSpriteEd(CSprite **_root,I64 snap,F64 ip_scaler,I64 *_cur_num)
{
  U8 *st;
  CTask *pu_task;
  I64 i;
  CDoc *doc=DocNew;
  DocPrintF(doc,"$$FG,PURPLE$$$$TX+CX,\"Sprite Edit Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",\"FI:::/TempleOS/Doc/SpriteEd.TXT.Z\"$$\n\n"
	"$$FG,LTBLUE$$$$MU-UL,\"Shift Points\",GED_SHIFT_PTS$$\n"
	"$$MU-UL,\"Shift Points with Rects\",GED_SHIFT_RECTS$$\n"
	"$$MU-UL,\"Shift Origin\",GED_SHIFT_ORIGIN$$\n"
	"$$MU-UL,\"Set Origin\",GED_SET_ORIGIN$$\n"
	"$$MU-UL,\"Insert Shift SubOrigin\",GED_SHIFT_SUB_ORIGIN$$\n"
	"$$MU-UL,\"Snap %d %5.3f\",GED_SNAP$$\n"
	"$$MU-UL,\"Pick or Delete Objects Graphically\",GED_PICK_GRAPHICALLY$$\n"
	"$$MU-UL,\"Transform\",GED_TRANSFORM$$\n"
	"$$MU-UL,\"Delete\",GED_DELETE$$\n"
	"$$MU-UL,\"Edit as Text\",GED_TEXT_ED$$\n"
	"$$MU-UL,\"Insert Clipboard Sprite's\",GED_INS_CLIPBOARD$$\n"
	"\n$$FG,PURPLE$$$$MU-UL,\"+] Sprite Main Menu\",GED_MAIN_MENU$$$$FG,LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",GED_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",DOCM_CANCEL$$",snap,ip_scaler);
  st=MSPrintF("SpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_root,_cur_num);
  PopUp(st,NULL,&pu_task);
  Free(st);
  i=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_root=SpriteDoc2Que(DocPut(pu_task),*_root,_cur_num);
    Kill(pu_task);
  }
  DocDel(doc);
  return i;
}

#define GESPT_SIMPLE_PT	0
#define GESPT_WIDTH_HEIGHT	1

#define GESPF_SELECTED		1

class CSpritePt
{
  CSpritePt *next,*last;
  I32 type,num,flags,xx,yy,zz;
  I32 *x,*y,*z,*w,*h;
};

U0 SpritePtQueNew(U8 *elems,I64 x,I64 y,CSpritePt *root)
{
  I64 i,num=0;
  I32 *ptr;
  CD3I32 *p;
  CSpritePt *temppe;
  CSprite *tempg=elems-offset(CSprite.start);
  QueInit(root);
  while (tempg->type) {
    switch (tempg->type) {
      case GRST_RECT:
      case GRST_ROTATED_RECT:
      case GRST_LINE:
      case GRST_ARROW:
      case GRST_PLANAR_SYMMETRY:
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->x2;
	temppe->y=&tempg->y2;
	temppe->num=num;
	QueIns(temppe,root->last);
      case GRST_TEXT:
      case GRST_TEXT_BOX:
      case GRST_TEXT_DIAMOND:
      case GRST_PT:
      case GRST_FLOOD_FILL:
      case GRST_FLOOD_FILL_NOT:
      case GRST_CIRCLE:
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->x1;
	temppe->y=&tempg->y1;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
      case GRST_SHIFT:
	x+=tempg->x1;
	y+=tempg->y1;
	break;
      case GRST_ELLIPSE:
      case GRST_POLYGON:
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_WIDTH_HEIGHT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->x2;
	temppe->y=&tempg->y2;
	temppe->w=&tempg->width;
	temppe->h=&tempg->height;
	temppe->num=num;
	QueIns(temppe,root->last);
      case GRST_BITMAP8:
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->x2;
	temppe->y=&tempg->y2;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
      case GRST_POLYLINE:
	ptr=&tempg->y1;
	for (i=0;i<tempg->num;i++) {
	  temppe=CAlloc(sizeof(CSpritePt));
	  temppe->type=GESPT_SIMPLE_PT;
	  temppe->xx=x;
	  temppe->yy=y;
	  temppe->x=&ptr[i<<1];
	  temppe->y=&ptr[i<<1+1];
	  temppe->num=num;
	  QueIns(temppe,root->last);
	}
	break;
      case GRST_POLYPT:
	ptr=&tempg->y1;
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&ptr[0];
	temppe->y=&ptr[1];
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
      case GRST_BSPLINE2:
      case GRST_BSPLINE3:
      case GRST_BSPLINE2_CLOSED:
      case GRST_BSPLINE3_CLOSED:
	p=&tempg->y1;
	for (i=0;i<tempg->num;i++) {
	  temppe=CAlloc(sizeof(CSpritePt));
	  temppe->type=GESPT_SIMPLE_PT;
	  temppe->xx=x;
	  temppe->yy=y;
	  temppe->x=&p[i].x;
	  temppe->y=&p[i].y;
	  temppe->z=&p[i].z;
	  temppe->num=num;
	  QueIns(temppe,root->last);
	}
	break;
      case GRST_MESH:
	break;
      case GRST_SHIFTABLE_MESH:
	temppe=CAlloc(sizeof(CSpritePt));
	temppe->type=GESPT_SIMPLE_PT;
	temppe->xx=x;
	temppe->yy=y;
	temppe->x=&tempg->x1;
	temppe->y=&tempg->y1;
	temppe->z=&tempg->z1;
	temppe->num=num;
	QueIns(temppe,root->last);
	break;
    }
    tempg(U8 *)+=SpriteElemSize(tempg);
    num++;
  }
}

U0 GESPCtrlPtsDraw(CDC *dc,CSpritePt *root)
{
  I64 x,y;
  CSpritePt *temppe;
  WinMgrSync;
  DCFill(dc);
  if (Blink(20)) {
    temppe=root->next;
    while (temppe!=root) {
      switch (temppe->type) {
	case GESPT_SIMPLE_PT:
	  x=*temppe->x+temppe->xx;
	  y=*temppe->y+temppe->yy;
	  break;
	case GESPT_WIDTH_HEIGHT:
	  x=*temppe->w+*temppe->x+temppe->xx;
	  y=*temppe->h+*temppe->y+temppe->yy;
	  break;
      }
      if (temppe->flags&GESPF_SELECTED)
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

U0 GESPCtrlPtsMove(CSpritePt *root,I64 dx,I64 dy)
{
  CSpritePt *temppe;
  temppe=root->next;
  while (temppe!=root) {
    if (temppe->flags&GESPF_SELECTED)
      switch (temppe->type) {
	case GESPT_SIMPLE_PT:
	  if (temppe->x) *temppe->x+=dx;
	  if (temppe->y) *temppe->y+=dy;
	  break;
	case GESPT_WIDTH_HEIGHT:
	  if (temppe->w) *temppe->w+=dx;
	  if (temppe->h) *temppe->h+=dy;
	  break;
      }
    temppe=temppe->next;
  }
}

Bool SpriteShiftPts(U8 *elems,I64 x,I64 y,I64 *_cur_num,I64 mode)
{
  I64 msg_code,p1,p2,xx,yy,xx2,yy2,dd,best_dd,cur_num;
  Bool old_preempt,result=TRUE;
  CDC *dc=DCAlias;
  CSpritePt root,*temppe,*best_pe;

  SpritePtQueNew(elems,x,y,&root);
  cur_num=0;
  if (root.next!=&root) {
    old_preempt=Preempt(OFF);
    while (TRUE) {
      GESPCtrlPtsDraw(dc,&root); //has WinMgrSync
      switch (msg_code=ScanMsg(&p1,&p2,1<<MSG_IP_R_UP|1<<MSG_IP_L_DOWN|1<<MSG_KEY_DOWN)) {
	case MSG_IP_L_DOWN:
	  switch (mode) {
	    case GED_SHIFT_PTS:
	      xx=p1; yy=p2;
	      best_dd=MAX_I64;
	      temppe=root.next;
	      while (temppe!=&root) {
		switch (temppe->type) {
		  case GESPT_SIMPLE_PT:
		    dd=SqrI64(*temppe->x+temppe->xx-xx)+SqrI64(*temppe->y+temppe->yy-yy);
		    break;
		  case GESPT_WIDTH_HEIGHT:
		    dd=SqrI64(*temppe->x+*temppe->w+temppe->xx-xx)+SqrI64(*temppe->y+*temppe->h+temppe->yy-yy);
		    break;
		}
		if (dd<best_dd) {
		  best_dd=dd;
		  best_pe=temppe;
		}
		temppe=temppe->next;
	      }
	      cur_num=best_pe->num;
	      best_pe->flags|=GESPF_SELECTED;
	      break;
	    case GED_SHIFT_RECTS:
	      xx2=xx=p1; yy2=yy=p2;
	      while (TRUE) {
		GESPCtrlPtsDraw(dc,&root);
		dc->color=ROPF_DITHER+WHITE<<16+RED;
		GrLineRect4(dc,xx,yy,xx2,yy2);
		if (msg_code=ScanMsg(&p1,&p2,1<<MSG_IP_MOVE|1<<MSG_IP_L_UP)) {
		  if (msg_code==MSG_IP_MOVE) {
		    xx2=p1; yy2=p2;
		  } else
		    break;
		}
	      }
	      if (xx2<xx) SwapU64(&xx,&xx2);
	      if (yy2<yy) SwapU64(&yy,&yy2);
	      temppe=root.next;
	      while (temppe!=&root) {
		switch (temppe->type) {
		  case GESPT_SIMPLE_PT:
		    if (xx<=*temppe->x+temppe->xx<=xx2 &&
			yy<=*temppe->y+temppe->yy<=yy2)
		      temppe->flags|=GESPF_SELECTED;
		    break;
		  case GESPT_WIDTH_HEIGHT:
		    if (xx<=*temppe->x+*temppe->w+temppe->xx<=xx2 &&
			yy<=*temppe->y+*temppe->h+temppe->yy<=yy2)
		      temppe->flags|=GESPF_SELECTED;
		    break;
		}
		temppe=temppe->next;
	      }
	      do {
		GESPCtrlPtsDraw(dc,&root);
		msg_code=ScanMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN);
		if (msg_code==MSG_KEY_DOWN) goto gs_key;
	      } while (msg_code!=MSG_IP_L_DOWN);
	      xx=p1;yy=p2;
	      break;
	  }
	  do {
	    GESPCtrlPtsDraw(dc,&root);
	    if (msg_code=ScanMsg(&p1,&p2,1<<MSG_IP_MOVE|1<<MSG_IP_L_UP)) {
	      GESPCtrlPtsMove(&root,p1-xx,p2-yy);
	      xx=p1;yy=p2;
	    }
	  } while (msg_code!=MSG_IP_L_UP);
	  temppe=root.next;
	  while (temppe!=&root) {
	    temppe->flags&=~GESPF_SELECTED;
	    temppe=temppe->next;
	  }
	  break;
	case MSG_KEY_DOWN:
	  gs_key:
	  switch (p1.u8[0]) {
	    case CH_SHIFT_ESC:
	      result=FALSE;
	    case CH_ESC:
	      GetMsg(&p1,&p2,1<<MSG_KEY_UP);
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
    Preempt(old_preempt);
    QueDel(&root,TRUE);
  }
  DCFill(dc);
  DCDel(dc);
  if (_cur_num && result)
    *_cur_num=cur_num;
  return result;
}

I64 SpriteEd(CDoc *doc,CDocEntry *doc_e,I64 x,I64 y,CSprite **_root,I64 *_cur_num,I64 *_ip_snap,F64 *_ip_scaler)
{
  CDocEntry *doc_e2;
  CDocBin *tempb;
  Bool unlock,del_down;
  I64 i,r[16],msg_code,p1,p2,xx,yy,ip_snap=1,
	old_e_flags1;
  F64 ip_scaler=1.0;
  CSprite *cur_elem,*root2,*next,*last,*tempg,*insert_pt;

  if (_ip_snap) ip_snap=*_ip_snap;
  if (_ip_scaler) ip_scaler=*_ip_scaler;
  SetSnap(&Fs->snap,ip_snap,ip_scaler);
  old_e_flags1=doc_e->flags1;
  tempb=doc_e->bin_data;
  DocUnlock(doc);
  do {
    if (win_actual_refresh<10)
      doc_e->flags1|=DOCEF1_DONT_DRAW;
    StrCpy(Fs->task_title,"Sprite Edit Menu");
    i=PopUpSpriteEd(_root,ip_snap,ip_scaler,_cur_num);
    GrEdUpdate(doc,tempb,*_root);
    if (0<=i<GED_EXIT) {
      StrCpy(Fs->task_title,DefineSub(i,"ST_SPRITE_ED_MENU"));
      switch (i) {
	case GED_PICK_GRAPHICALLY:
	  AcctOneTimePopUp(ARf_CSPRITE_PICK_GRAPHICALLY,
		"You can delete elements by\n"
		"holding $$FG,GREEN$$Delete$$FG$$ and clicking.\n");
	  doc_e->flags1=old_e_flags1;
	  doc->cur_sprite=tempb->data;
	  doc->cur_sprite_elem_num=0;
	  del_down=FALSE;
	  do {
	    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|1<<MSG_KEY_DOWN|1<<MSG_KEY_UP);
	    if (p2.u8[0]==SC_DELETE&& !(p2&SCF_IP_L_DOWN)) {
	      if (msg_code==MSG_KEY_DOWN)
		del_down=TRUE;
	      else if (msg_code==MSG_KEY_UP)
		del_down=FALSE;
	    }
	    if (msg_code==MSG_IP_L_UP && sys_cur_scan_code&SCF_DELETE ||
		  msg_code==MSG_KEY_DOWN && p2&SCF_IP_L_DOWN && p2.u8[0]==SC_DELETE) {
	      *_cur_num=doc->cur_sprite_elem_num;
	      cur_elem=GrNum2Que(*_root,*_cur_num);
	      if (cur_elem!=*_root) {
		QueRem(cur_elem);
		Free(cur_elem);
		GrEdUpdate(doc,tempb,*_root);
	      }
	      doc->cur_sprite=tempb->data;
	      doc->cur_sprite_elem_num=0;
	      if (msg_code==MSG_KEY_DOWN)
		GetMsg(&p1,&p2,1<<MSG_IP_L_UP);
	    }
	  } while (del_down || msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
	  *_cur_num=doc->cur_sprite_elem_num;
	  doc->cur_sprite=NULL;
	  break;
	case GED_SNAP:
	  PopUpSnap(&Fs->snap);
	  ip_snap=Fs->snap.x;
	  ip_scaler=Fs->snap.dx;
	  break;
	case GED_SET_ORIGIN:
	  doc_e->flags1=old_e_flags1;
	  GetMsg(&p1,&p2,1<<MSG_IP_L_UP);
	  GrSetOrigin(*_root,x-p1,y-p2,0);
	  GrEdUpdate(doc,tempb,*_root);
	  break;
	case GED_SHIFT_ORIGIN:
	  doc_e->flags1=old_e_flags1;
	  GetMsg(&p1,&p2,1<<MSG_IP_L_DOWN);
	  xx=p1; yy=p2;
	  do {
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrSetOrigin(*_root,p1-xx,p2-yy,0);
	    xx=p1; yy=p2;
	    GrEdUpdate(doc,tempb,*_root);
	  } while (msg_code!=MSG_IP_L_UP);
	  break;
	case GED_SHIFT_PTS:
	case GED_SHIFT_RECTS:
	  AcctOneTimePopUp(ARf_CSPRITE_PTS_RECTANGLES,
		"You can switch between points\n"
		"and rectangles with '$$FG,GREEN$$p$$FG$$' and '$$FG,GREEN$$r$$FG$$'.\n"
		"Press '$$FG,GREEN$$r$$FG$$' after one rectangle\n"
		"to OR another rectangle.\n");
	  doc_e->flags1=old_e_flags1;
	  if (SpriteShiftPts(tempb->data,x,y,_cur_num,i)) {
	    QueDel(*_root);
	    Free(*_root);
	    *_root=Sprite2Que(tempb->data);
	  } else
	    GrEdUpdate(doc,tempb,*_root);
	  break;
	case GED_TRANSFORM:
	  if (PopUpTransform(r)) {
	    SpriteTransform(r,*_root);
	    GrEdUpdate(doc,tempb,*_root);
	  }
	  break;
	case GED_SHIFT_SUB_ORIGIN:
	  doc_e->flags1=old_e_flags1;
	  insert_pt=GrNum2Que(*_root,*_cur_num);
	  tempg=MAlloc(gr_sprite_qued_base_size[GRST_SHIFT]);
	  tempg->type=GRST_SHIFT;
	  tempg->x1=0;
	  tempg->y1=0;
	  QueIns(tempg,insert_pt->last);
	  GetMsg(&p1,&p2,1<<MSG_IP_L_DOWN);
	  xx=p1; yy=p2;
	  do {
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    tempg->x1=p1-xx;
	    tempg->y1=p2-yy;
	    GrEdUpdate(doc,tempb,*_root);
	  } while (msg_code!=MSG_IP_L_UP);
	  *_cur_num+=1;
	  break;
	case GED_INS_CLIPBOARD:
	  AcctOneTimePopUp(ARf_CSPRITE_INS_CLIPBOARD,
		"You will probably want to shift around\n"
		"the location of element groups.  Use\n"
		"'Insert shift sub-origin' after picking the\n"
		"element to insert before.  Or,\n"
		"use 'shift points'.\n");
	  insert_pt=GrNum2Que(*_root,*_cur_num);
	  unlock=DocLock(sys_clipboard_doc);
	  doc_e2=sys_clipboard_doc->root.next;
	  while (doc_e2!=sys_clipboard_doc) {
	    if (doc_e2->type_u8==DOCT_SPRITE) {
	      root2=Sprite2Que(doc_e2->bin_data->data);
	      if (root2->next!=root2) {
		tempg=root2->next;
		while (tempg!=root2) {
		  *_cur_num+=1;
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
	  GrEdUpdate(doc,tempb,*_root);
	  break;
	case GED_DELETE:
	  cur_elem=GrNum2Que(*_root,*_cur_num);
	  if (cur_elem!=*_root) {
	    QueRem(cur_elem);
	    Free(cur_elem);
	    GrEdUpdate(doc,tempb,*_root);
	  }
	  break;
	case GED_TEXT_ED:
	  if (SpriteTextEd(_root,_cur_num))
	    GrEdUpdate(doc,tempb,*_root);
	  break;
      }
    }
  } while (i!=DOCM_CANCEL && i!=GED_EXIT && i!=GED_MAIN_MENU);
  doc_e->flags1=old_e_flags1;

  if (_ip_snap)   *_ip_snap=ip_snap;
  if (_ip_scaler) *_ip_scaler=ip_scaler;
  switch (i) {
    case DOCM_CANCEL:	return GE_ABORT;
    case GED_EXIT:	return GE_EXIT;
    case GED_MAIN_MENU:	return GE_CONT;
  }
}

#help_index "Graphics"
