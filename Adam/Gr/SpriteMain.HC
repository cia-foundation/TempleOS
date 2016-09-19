#help_index "Graphics/Sprite;Sprites"

#define SPT_MENU	-2
#define SPT_INS_SCREEN_BITMAP			-3
#define SPT_INS_TRANSPARENT_SCREEN_BITMAP	-4
#define SPT_ED_MENU	-5
#define SPT_EXIT	-6

I64 PopUpSpriteMain(CSprite **_head,I64 *_cur_elem_num,
	CDoc *_doc,CDocEntry *_doc_e)
{
  CTask	*pu_task;
  I64	i;
  CDoc *doc=DocNew;
  CDocEntry *doc_de;
  U8	*st;

  doc_de=DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite Main Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteMain.DD.Z\"$$\n"
	"\n$$LTBLUE$$$$DA+M,A=\"Tag Text:%%s\"$$\n\n");
  doc_de->data=StrNew(_doc_e->tag,doc->mem_task);
  DocDataFmt(doc,doc_de);

  DocPrint(doc,"$$MU-UL,\"Color (4-bit)\",LE=SPT_COLOR$$\n"
	"$$MU-UL,\"Dither Color (4-bit)\",LE=SPT_DITHER_COLOR$$\n"
	"$$MU-UL,\"Width\",LE=SPT_WIDTH$$\n"
	"$$MU-UL,\"Planar Symmetry\",LE=SPT_PLANAR_SYMMETRY$$\n"
	"\n$$MU-UL,\"Point\",LE=SPT_PT$$\n"
	"$$MU-UL,\"Line\",LE=SPT_LINE$$\n"
	"$$MU-UL,\"Arrow\",LE=SPT_ARROW$$\n"
	"$$MU-UL,\"Rect\",LE=SPT_RECT$$\n"
	"$$MU-UL,\"Circle\",LE=SPT_CIRCLE$$\n"
	"$$MU-UL,\"Ellipse\",LE=SPT_ELLIPSE$$\n"
	"$$MU-UL,\"Polygon\",LE=SPT_POLYGON$$\n"
	"$$MU-UL,\"Text\",LE=SPT_TEXT$$\n"
	"$$MU-UL,\"Text Box\",LE=SPT_TEXT_BOX$$\n"
	"$$MU-UL,\"Text Diamond\",LE=SPT_TEXT_DIAMOND$$\n"
	"$$MU-UL,\"Flood Fill\",LE=SPT_FLOOD_FILL$$\n"
	"$$MU-UL,\"Flood Fill Not Color\",LE=SPT_FLOOD_FILL_NOT$$\n"
	"$$MU-UL,\"PolyLine\",LE=SPT_POLYLINE$$\n"
	"$$MU-UL,\"PolyPoint\",LE=SPT_POLYPT$$\n"
	"$$MU-UL,\"BSpline2\",LE=SPT_BSPLINE2$$\n"
	"$$MU-UL,\"BSpline3\",LE=SPT_BSPLINE3$$\n"
	"$$MU-UL,\"BSpline2 Closed\",LE=SPT_BSPLINE2_CLOSED$$\n"
	"$$MU-UL,\"BSpline3 Closed\",LE=SPT_BSPLINE3_CLOSED$$\n"
	"$$MU-UL,\"Insert Screen-Captured BitMap\",LE=SPT_INS_SCREEN_BITMAP$$\n"
	"$$MU-UL,\"Insert Transparent Screen-Captured BitMap\","
	"LE=SPT_INS_TRANSPARENT_SCREEN_BITMAP$$\n"
	"$$PURPLE$$$$MU-UL,\"+] Create or Edit 3D Mesh\",LE=SPT_MESH$$\n"
	"$$MU-UL,\"+] Create or Edit Shiftable 3D Mesh\","
	"LE=SPT_SHIFTABLE_MESH$$\n"
	"$$MU-UL,\"+] Convert to BitMap or Edit BitMap\","
	"LE=SPT_BITMAP$$$$LTBLUE$$\n"
	"\n$$MU-UL,\"Transform On  (for use with 3D icons)\","
	"LE=SPT_TRANSFORM_ON$$\n"
	"$$MU-UL,\"Transform Off (for use with 3D icons)\","
	"LE=SPT_TRANSFORM_OFF$$\n"
	"\n"
	"$$PURPLE$$$$MU-UL,\"+] Sprite Edit Menu\",LE=SPT_ED_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=SPT_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$\n"
	"\nRight-Click to get back to this menu.");
  st=MStrPrint("SpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_head,_cur_elem_num);
  PopUp(st,NULL,&pu_task);
  Free(st);
  i=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_head=SpriteSideBar2SpriteQue(DocPut(pu_task),*_head,_cur_elem_num);
    Kill(pu_task);
  }
  Free(_doc_e->tag);
  _doc_e->tag=StrNew(doc_de->data,_doc->mem_task);
  _doc->cur_col=0;
  DocDel(doc);
  return i;
}

Bool PopUpExtents(I64 *_x1,I64 *_x2,I64 *_y1,I64 *_y2)
{
  I64 i;
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  doc_e=DocPrint(doc,"  $$DA,A=\"x1:%%d\"$$\n");
  doc_e->data=_x1;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,A=\"x2:%%d\"$$\n");
  doc_e->data=_x2;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,A=\"y1:%%d\"$$\n");
  doc_e->data=_y1;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,A=\"y2:%%d\"$$\n\n");
  doc_e->data=_y2;
  DocDataFmt(doc,doc_e);

  DocPrint(doc," $$BT,\"Use These Extents\",LE=TRUE$$");
  DocPrint(doc,"$$CM,3,0$$$$BT,\"Drag-Out New Extents\",LE=FALSE$$\n\n");
  do i=PopUpMenu(doc);
  while (i!=FALSE && i!=TRUE);
  DocDel(doc);
  return i;
}

U0 SpriteScreenInit(CDC *dc,I64,I64)
{
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 xx,yy,old_pen_width=dc->pen_width;
  CColorROPU32 old_color=dc->color;
  WinMgrSync;
  DCFill(dc);
  if (dc->flags&DCF_SYMMETRY) {
    dc->flags&=~DCF_SYMMETRY;
    dc->pen_width=1;
    xx=dc->sym.sny*8192;
    yy=-dc->sym.snx*8192;
    dc->color=RED;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
	  dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,0);
    dc->color=WHITE;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
	  dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,1);
    dc->color=BLACK;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
	  dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,2);
    dc->flags|=DCF_SYMMETRY;
  }
  dc->color=old_color;
  dc->pen_width=old_pen_width;
}

CSprite *SMLine(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrLine3(dc,x1,y1,0,x2,y2,0);
  res=CAlloc(SpriteElemQuedBaseSize(SPT_LINE));
  res->type=SPT_LINE;
  res->pp.x1=x1-x;
  res->pp.y1=y1-y;
  res->pp.x2=x2-x;
  res->pp.y2=y2-y;
  return res;
}

CSprite *SMArrow(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrArrow3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrArrow3(dc,x1,y1,0,x2,y2,0);
  res=CAlloc(SpriteElemQuedBaseSize(SPT_ARROW));
  res->type=SPT_ARROW;
  res->pp.x1=x1-x;
  res->pp.y1=y1-y;
  res->pp.x2=x2-x;
  res->pp.y2=y2-y;
  return res;
}

CSprite *SMPlanarSymmetry(CDC *dc,I64 x,I64 y,I64 a1,I64 a2)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2,old_width,old_flags;
  CSprite *res;
  old_width=dc->pen_width;
  old_flags=dc->flags;
  dc->flags&=~DCF_SYMMETRY;
  dc->pen_width=1;
  do {
    dc->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  dc->flags=old_flags&DCF_SYMMETRY|dc->flags&~DCF_SYMMETRY;
  dc->pen_width=old_width;
  res=CAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
  res->type=SPT_PLANAR_SYMMETRY;
  res->pp.x1=x1-x;
  res->pp.y1=y1-y;
  res->pp.x2=x2-x;
  res->pp.y2=y2-y;
  return res;
}

CSprite *SMRect(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2,xx1=a1,yy1=a2,xx2=a1,yy2=a2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
    if (x2<x1) {
      xx1=x2; xx2=x1;
    } else {
      xx1=x1; xx2=x2;
    }
    if (y2<y1) {
      yy1=y2; yy2=y1;
    } else {
      yy1=y1; yy2=y2;
    }
  } while (msg_code!=MSG_IP_L_UP);
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
  res=CAlloc(SpriteElemQuedBaseSize(SPT_RECT));
  res->type=SPT_RECT;
  res->pp.x1=xx1-x;
  res->pp.y1=yy1-y;
  res->pp.x2=xx2-x;
  res->pp.y2=yy2-y;
  return res;
}

CSprite *SMScreenBitMap(I64 eletype,CDC *dc,CDC *dc2,I64 x,I64 y,I64 a1,I64 a2,
	CColorROPU32 bm_bkcolor)
{
  I64 i,msg_code,x1=a1,y1=a2,x2=a1,y2=a2,xx1=a1,yy1=a2,xx2=a1,yy2=a2,old_width;
  CDC *dc3,*img;
  CSprite *res;
  old_width=dc2->pen_width;
  dc2->pen_width=1;
  do {
    dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrBorder(dc2,xx1+Fs->pix_left+Fs->scroll_x,yy1+Fs->pix_top+Fs->scroll_y,
	  xx2+Fs->pix_left+Fs->scroll_x,yy2+Fs->pix_top+Fs->scroll_y);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
    if (x2<x1) {
      xx1=x2; xx2=x1;
    } else {
      xx1=x1; xx2=x2;
    }
    if (y2<y1) {
      yy1=y2; yy2=y1;
    } else {
      yy1=y1; yy2=y2;
    }
  } while (msg_code!=MSG_IP_L_UP);
  xx2++; yy2++;
  res=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+((xx2-xx1+7)&~7)*(yy2-yy1));
  res->type=SPT_BITMAP;
  res->pwhu.width=xx2-xx1;
  res->pwhu.height=yy2-yy1;
  res->pwhu.x1=0;
  res->pwhu.y1=0;
  SpriteScreenInit(dc,x,y);
  i=gr.screen_zoom;
  GrScaleZoom(1.0/i);
  WinMgrSync(2,TRUE);

  dc3=DCScreenCapture;
  img=DCExt(dc3,Fs->pix_left+Fs->scroll_x+xx1,Fs->pix_top+Fs->scroll_y+yy1,
	Fs->pix_left+Fs->scroll_x+xx2-1,Fs->pix_top+Fs->scroll_y+yy2-1);
  if (eletype==SPT_INS_TRANSPARENT_SCREEN_BITMAP)
    DCColorChg(img,bm_bkcolor);
  GrScaleZoom(i);
  MemCpy(&res->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
  DCDel(img);
  DCDel(dc3);
  dc2->pen_width=old_width;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL;
  return res;
}

CSprite *SMCircle(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
  res=CAlloc(SpriteElemQuedBaseSize(SPT_CIRCLE));
  res->type=SPT_CIRCLE;
  res->pr.x1=x1-x;
  res->pr.y1=y1-y;
  res->pr.radius=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
  return res;
}

CSprite *SMEllipse(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  F64 angle1,angle2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrEllipse3(dc,(x1+x2)>>1,(y1+y2)>>1,0,AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  res=CAlloc(SpriteElemQuedBaseSize(SPT_ELLIPSE));
  res->type=SPT_ELLIPSE;
  res->pwha.x1=(x1+x2)>>1-x;
  res->pwha.y1=(y1+y2)>>1-y;
  res->pwha.width =AbsI64(x1-x2)>>1;
  res->pwha.height=AbsI64(y1-y2)>>1;
  angle2=Arg(x2-(res->pwha.x1+x),y2-(res->pwha.y1+y));
  if (res->pwha.width<res->pwha.height)
    angle2-=ã/2.0;
  do {
    angle1=Arg(x2-(res->pwha.x1+x),y2-(res->pwha.y1+y));
    if (res->pwha.width>=res->pwha.height)
      res->pwha.angle=-(angle1-angle2);
    else
      res->pwha.angle=-(angle1-angle2)+ã/2.0;
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrEllipse3(dc,res->pwha.x1+x,res->pwha.y1+y,0,
	  res->pwha.width,res->pwha.height,res->pwha.angle);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  angle1=Arg(x2-(res->pwha.x1+x),y2-(res->pwha.y1+y));
  if (res->pwha.width>=res->pwha.height)
    res->pwha.angle=-(angle1-angle2);
  else
    res->pwha.angle=-(angle1-angle2)+ã/2.0;
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrEllipse3(dc,res->pwha.x1+x,res->pwha.y1+y,0,
	res->pwha.width,res->pwha.height,res->pwha.angle);
  return res;
}

CSprite *SMPolygon(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,I64 sides,
	CColorROPU32 color)
{
  I64 msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  F64 angle1,angle2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrRegPoly3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
	  AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1,sides);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  res=CAlloc(SpriteElemQuedBaseSize(SPT_POLYGON));
  res->type=SPT_POLYGON;
  res->pwhas.x1=(x1+x2)>>1-x;
  res->pwhas.y1=(y1+y2)>>1-y;
  res->pwhas.width =AbsI64(x1-x2)>>1;
  res->pwhas.height=AbsI64(y1-y2)>>1;
  res->pwhas.sides=sides;
  angle2=Arg(x2-(res->pwhas.x1+x),y2-(res->pwhas.y1+y));
  if (res->pwhas.width<res->pwhas.height)
    angle2-=ã/2.0;
  do {
    angle1=Arg(x2-(res->pwhas.x1+x),y2-(res->pwhas.y1+y));
    if (res->pwhas.width>=res->pwhas.height)
      res->pwhas.angle=-(angle1-angle2);
    else
      res->pwhas.angle=-(angle1-angle2)+ã/2.0;
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrRegPoly3(dc,res->pwhas.x1+x,res->pwhas.y1+y,0,
	  res->pwhas.width,res->pwhas.height,
	  res->pwhas.sides,res->pwhas.angle);
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    SpriteScreenInit(dc,x,y);
    x2=a1; y2=a2;
  } while (msg_code!=MSG_IP_L_UP);
  angle1=Arg(x2-(res->pwhas.x1+x),y2-(res->pwhas.y1+y));
  if (res->pwhas.width>=res->pwhas.height)
    res->pwhas.angle=-(angle1-angle2);
  else
    res->pwhas.angle=-(angle1-angle2)+ã/2.0;
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrRegPoly3(dc,res->pwhas.x1+x,res->pwhas.y1+y,0,
	res->pwhas.width,res->pwhas.height,res->pwhas.sides,
	res->pwhas.angle);
  return res;
}

CSprite *SMPolyLineFamily(I64 eletype,CDC *dc,I64 x,I64 y,I64 a1,I64 a2,
	CColorROPU32 color)
{
  I64 i,num=0,msg_code,x1=a1,y1=a2,x2=a1,y2=a2;
  I32 *ptr;
  CD3I32 *p;
  CSprite *res,*tempg,*tempg1,head2;

  QueInit(&head2);
  do {
    do {
      dc->color=color&COLORROP_NO_ROP0_MASK;
      if (num)
	GrLine3(dc,x1,y1,0,x2,y2,0);
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE+1<<MSG_IP_R_UP);
      dc->color=TRANSPARENT;
      if (num)
	GrLine3(dc,x1,y1,0,x2,y2,0);
      x2=a1; y2=a2;
    } while (msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
    dc->color=color&COLORROP_NO_ROP0_MASK;
    if (msg_code==MSG_IP_L_UP) {
      if (num)
	GrLine3(dc,x1,y1,0,x2,y2,0);
      res=CAlloc(SpriteElemQuedBaseSize(SPT_PT));
      res->type=SPT_PT;
      res->p.x1=x2-x;
      res->p.y1=y2-y;
      QueIns(res,head2.last);
      x1=x2;y1=y2;
      num++;
    }
  } while (msg_code!=MSG_IP_R_UP);

  switch (eletype) {
    case SPT_POLYLINE:
      if (num>1) {
	res=CAlloc(SpriteElemQuedBaseSize(SPT_POLYLINE)+num*sizeof(CD2I32));
	ptr=&res->nu.u;
	tempg1=head2.next;
	for (i=0;i<num;i++) {
	  tempg=tempg1->next;
	  ptr[i<<1]=tempg1->p.x1;
	  ptr[i<<1+1]=tempg1->p.y1;
	  Free(tempg1);
	  tempg1=tempg;
	}
	res->type=SPT_POLYLINE;
	res->nu.num=num;
      } else {
	tempg1=head2.next;
	for (i=0;i<num;i++) {
	  tempg=tempg1->next;
	  Free(tempg1);
	  tempg1=tempg;
	}
	res=NULL;
      }
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      if (num>2) {
	res=CAlloc(SpriteElemQuedBaseSize(eletype)+num*sizeof(CD3I32));
	p=&res->nu.u;
	tempg1=head2.next;
	for (i=0;i<num;i++) {
	  tempg=tempg1->next;
	  p[i].x=tempg1->p.x1;
	  p[i].y=tempg1->p.y1;
	  Free(tempg1);
	  tempg1=tempg;
	}
	res->type=eletype;
	res->nu.num=num;
      } else {
	tempg1=head2.next;
	for (i=0;i<num;i++) {
	  tempg=tempg1->next;
	  Free(tempg1);
	  tempg1=tempg;
	}
	res=NULL;
      }
      break;
  }
  return res;
}

CSprite *SMPolyPoint(CDC *dc,I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color)
{
  I64 i,num=0,msg_code,x1=a1,y1=a2,x2=a1,y2=a2,x3,y3;
  I32 *ptr;
  CSprite *res,*tempg,*tempg1,head2;

  QueInit(&head2);
  x3=a1-x; y3=a2-y;
  dc->color=color&COLORROP_NO_ROP0_MASK;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
    x2=a1; y2=a2;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    Line(&head2,x1-x,y1-y,0,x2-x,y2-y,0,&SpritePolyPtPlot);
    x1=x2;y1=y2;
  } while (msg_code!=MSG_IP_L_UP);

  num=0;
  res=head2.next;
  x1=x3; y1=y3;
  while (res!=&head2) {
    tempg=res->next;
    if (res->p.x1==x1 && res->p.y1==y1) {
      QueRem(res);
      Free(res);
    } else {
      num++;
      x1=res->p.x1;
      y1=res->p.y1;
    }
    res=tempg;
  }

  res=CAlloc(SpriteElemQuedBaseSize(SPT_POLYPT)+(num*3+7)>>3);
  res->npu.x=x3;
  res->npu.y=y3;
  ptr=&res->npu.u;
  x1=x3; y1=y3;
  i=0;
  tempg1=head2.next;
  while (tempg1!=&head2) {
    tempg=tempg1->next;
    BFieldOrU32(ptr,i,polypt_map[SignI64(tempg1->p.x1-x1)+1+
	  3*(SignI64(tempg1->p.y1-y1)+1)]);
    i+=3;
    x1=tempg1->p.x1;y1=tempg1->p.y1;
    QueRem(tempg1);
    Free(tempg1);
    tempg1=tempg;
  }
  res->type=SPT_POLYPT;
  res->npu.num=num;
  return res;
}

U0 SMTextFamily(I64 eletype,CDoc *doc,CDocEntry *doc_ce,CSprite *head,CDC *dc,
	I64 x,I64 y,I64 a1,I64 a2,CColorROPU32 color,I64 *_cur_elem_num,
	I64 old_de_flags)
{
  CSprite *tempg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num);
  I64 msg_code,x1,y1;
  U8 *st;
  doc_ce->de_flags|=DOCEF_DONT_DRAW;
  st=PopUpGetStr("Enter text and press <ESC>.\n");
  doc_ce->de_flags=old_de_flags;
  if (st && *st) {
    x1=0; y1=0;
    do {
      dc->color=color&COLORROP_NO_ROP0_MASK;
      switch (eletype) {
	case SPT_TEXT:		GrPrint3(dc,x1,y1,0,"%s",st);	break;
	case SPT_TEXT_BOX:	GrTextBox3(dc,x1,y1,0,st);	break;
	case SPT_TEXT_DIAMOND:	GrTextDiamond3(dc,x1,y1,0,st);	break;
      }
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
      SpriteScreenInit(dc,x,y);
      x1=a1; y1=a2;
    } while (msg_code!=MSG_IP_L_UP);
    tempg=CAlloc(SpriteElemQuedBaseSize(eletype)+StrLen(st)+1);
    tempg->type=eletype;
    tempg->ps.x1=x1-x;
    tempg->ps.y1=y1-y;
    StrCpy(tempg->ps.st,st);
    QueIns(tempg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    *_cur_elem_num+=1;
  }
  Free(st);
}

I64 SMBitMap(I64 eletype,CDoc *doc,CDocEntry *doc_ce,CSprite *head,
	CDC *dc,I64 xx,I64 yy,I64 a1,I64 a2,CColorROPU32 bm_bkcolor,
	Bool sel,I64 xx1=0,I64 yy1=0,I64 xx2=0,I64 yy2=0,I64 *_cur_elem_num)
{
  I64 i,msg_code,x=xx,y=yy,x1=a1,y1=a2,x2=a1,y2=a2,old_width;
  CSprite *tempg,*tempg1,*insert_pt;
  CDC *img;

  insert_pt=SpriteSetSettings(,head,*_cur_elem_num,xx,yy,,,&x,&y);
  x+=xx; y+=yy;

  if (sel) {
    xx1=a1; yy1=a2;
    xx2=a1; yy2=a2;
    old_width=dc->pen_width;
    dc->pen_width=1;
    do {
      dc->color=ROPF_DITHER+WHITE<<16+BLACK;
      GrBorder(dc,xx1,yy1,xx2,yy2);
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
      SpriteScreenInit(dc,x,y);
      x2=a1; y2=a2;
      if (x2<x1) {
	xx1=x2; xx2=x1;
      } else {
	xx1=x1; xx2=x2;
      }
      if (y2<y1) {
	yy1=y2; yy2=y1;
      } else {
	yy1=y1; yy2=y2;
      }
    } while (msg_code!=MSG_IP_L_UP);
    dc->pen_width=old_width;
  }

  xx2++; yy2++;
  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+
	((xx2-xx1+7)&~7)*(yy2-yy1));
  tempg->type=SPT_BITMAP;
  tempg->pwhu.width=xx2-xx1;
  tempg->pwhu.height=yy2-yy1;
  tempg->pwhu.x1=xx1-x;
  tempg->pwhu.y1=yy1-y;
  img=DCNew(tempg->pwhu.width,tempg->pwhu.height,Fs);
  img->color=bm_bkcolor;
  GrRect(img,0,0,tempg->pwhu.width,tempg->pwhu.height);
  tempg1=insert_pt;
  if (tempg1==head || tempg1->type!=SPT_BITMAP) {
    SpriteSetSettings(img,head,0,-(xx1-x),-(yy1-y));
    x=xx; y=yy;
    Sprite3(img,-(xx1-x),-(yy1-y),0,doc_ce->bin_data->data);
    QueDel(head);
    insert_pt=head->next=head->last=head;
    *_cur_elem_num=1;
  } else {
    SpriteSetSettings(img,head,*_cur_elem_num,-(xx1-x),-(yy1-y));
    Sprite3(img,-(xx1-x),-(yy1-y),0,&tempg1->start,TRUE);
    insert_pt=tempg1->next;
    QueRem(tempg1);
    Free(tempg1);
  }
  MemCpy(&tempg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));

  switch (i=SpriteBitMapEd(doc,doc_ce,dc,&xx1,&yy1,
	&xx2,&yy2,&img,bm_bkcolor)) {
    case GE_EXIT:
    case GE_CONT:
      Free(tempg);
      tempg=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+
	    ((xx2-xx1+7)&~7)*(yy2-yy1));
      tempg->type=eletype;
      tempg->pwhu.width=xx2-xx1;
      tempg->pwhu.height=yy2-yy1;
      tempg->pwhu.x1=xx1-x;
      tempg->pwhu.y1=yy1-y;
      MemCpy(&tempg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
      break;
  }
  QueIns(tempg,insert_pt->last);
  DCDel(img);
  SpriteEdUpdate(doc,doc_ce,head);
  return i;
}

U0 SMMesh(CDoc *doc,CDocEntry *doc_ce,CSprite *head,I64 *_cur_elem_num)
{
  CSprite *tempg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num),
	*tempg1=insert_pt;
  CD3I32 *p;
  I64 i,size,x1,y1,z1;
  I32 *mesh,*old_mesh;
  if (tempg1!=head && tempg1->type==SPT_MESH)
    old_mesh=&tempg1->mu.vertex_cnt;
  else if (tempg1!=head && tempg1->type==SPT_SHIFTABLE_MESH) {
    x1=tempg1->pmu.x;
    y1=tempg1->pmu.y;
    z1=tempg1->pmu.z;
    p=&tempg1->pmu.u;
    for (i=0;i<tempg1->pmu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tempg1->pmu.vertex_cnt;
  } else
    old_mesh=NULL;
  if (mesh=SpriteMeshEd(old_mesh,&size,TRUE)) {
    tempg=CAlloc(SpriteElemQuedBaseSize(SPT_MESH)-sizeof(I32)*2+size);
    tempg->type=SPT_MESH;
    MemCpy(&tempg->mu.vertex_cnt,mesh,size);
    Free(mesh);
    QueIns(tempg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    if (old_mesh) {
      insert_pt=tempg;
      QueRem(tempg1);
      Free(tempg1);
      SpriteEdUpdate(doc,doc_ce,head);
    } else
      *_cur_elem_num+=1;
  } else if (old_mesh && tempg1->type==SPT_SHIFTABLE_MESH) {
    x1=tempg1->pmu.x;
    y1=tempg1->pmu.y;
    z1=tempg1->pmu.z;
    p=&tempg1->pmu.u;
    for (i=0;i<tempg1->pmu.vertex_cnt;i++,p++) {
      p->x-=x1;
      p->y-=y1;
      p->z-=z1;
    }
  }
}

U0 SMShiftableMesh(CDoc *doc,CDocEntry *doc_ce,CSprite *head,
	I64 x,I64 y,I64 a1,I64 a2,I64 *_cur_elem_num)
{
  CSprite *tempg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num),
	*tempg1=insert_pt;
  CD3I32 *p;
  I64 i,size,z,x1,y1,z1;
  I32 *mesh,*old_mesh;
  if (tempg1!=head && tempg1->type==SPT_MESH) {
    z=0;
    x1=-(a1-x);
    y1=-(a2-y);
    z1=z;
    p=&tempg1->mu.u;
    for (i=0;i<tempg1->mu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tempg1->mu.vertex_cnt;
  } else if (tempg1!=head && tempg1->type==SPT_SHIFTABLE_MESH) {
    z=-tempg1->pmu.z;
    x1=tempg1->pmu.x-(a1-x);
    y1=tempg1->pmu.y-(a2-y);
    z1=tempg1->pmu.z+z;
    p=&tempg1->pmu.u;
    for (i=0;i<tempg1->pmu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tempg1->pmu.vertex_cnt;
  } else {
    z=0;
    old_mesh=NULL;
  }
  if (mesh=SpriteMeshEd(old_mesh,&size,TRUE)) {
    tempg=CAlloc(SpriteElemQuedBaseSize(SPT_SHIFTABLE_MESH)-sizeof(I32)*2+size);
    tempg->type=SPT_SHIFTABLE_MESH;
    MemCpy(&tempg->pmu.vertex_cnt,mesh,size);
    Free(mesh);
    tempg->pmu.x=a1-x;
    tempg->pmu.y=a2-y;
    tempg->pmu.z=-z;
    QueIns(tempg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    if (old_mesh) {
      insert_pt=tempg;
      QueRem(tempg1);
      Free(tempg1);
      SpriteEdUpdate(doc,doc_ce,head);
    } else
      *_cur_elem_num+=1;
  } else if (old_mesh && tempg1->type==SPT_SHIFTABLE_MESH) {
    x1=tempg1->pmu.x-(a1-x);
    y1=tempg1->pmu.y-(a2-y);
    z1=tempg1->pmu.z+z;
    p=&tempg1->pmu.u;
    for (i=0;i<tempg1->pmu.vertex_cnt;i++,p++) {
      p->x-=x1;
      p->y-=y1;
      p->z-=z1;
    }
  } else if (old_mesh && tempg1->type==SPT_MESH) {
    x1=-(a1-x);
    y1=-(a2-y);
    z1= z;
    p=&tempg1->mu.u;
    for (i=0;i<tempg1->mu.vertex_cnt;i++,p++) {
      p->x-=x1;
      p->y-=y1;
      p->z-=z1;
    }
  }
}

U0 SMTaskTitleSet(I64 eletype)
{
  Fs->title_src=TTS_CONST; //Violates TTS_LOCKED_CONST
  switch (eletype) {
    case SPT_INS_SCREEN_BITMAP:
      StrCpy(Fs->task_title,"Insert Screen BitMap");
      break;
    case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
      StrCpy(Fs->task_title,"Insert Transparent Screen BitMap");
      break;
    default:
      if (eletype>=0)
	StrCpy(Fs->task_title,DefineSub(eletype,"ST_SPRITE_ELEM_TYPES"));
  }
  Fs->border_src=BDS_CONST;
}

I64 SpriteMainEd(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry;
  I64 res,i,x,y,a1,a2,xx,yy,xx1,yy1,xx2,yy2,width,eletype=SPT_MENU,
	cur_elem_num,old_border_src=Fs->border_src,old_title_src=Fs->title_src,
	old_de_flags=doc_ce->de_flags;
  CColorROPU32 bm_bkcolor,color;
  CSprite *head,*tempg,*insert_pt;
  CDC *dc=DCAlias(,Fs),*dc2=DCAlias(,sys_winmgr_task),*dc3;
  U8 *old_task_title=StrNew(Fs->task_title);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinMgrSync(2,TRUE);
  dc2->flags|=DCF_ON_TOP;
  head=Sprite2SpriteQue(doc_ce->bin_data->data);
  cur_elem_num=QueCnt(head);
  xx=(doc_ce->x+doc_ce->max_col-doc->line_start_col)*FONT_WIDTH;
  yy=(doc_ce->y-doc->top_line_num)*FONT_HEIGHT;

  while (TRUE) {
    insert_pt=SpriteSetSettings(dc,head,cur_elem_num,xx,yy,
	  &color,&width,&x,&y);
    x+=xx; y+=yy;
    DCFill;
    if (eletype==SPT_MENU) {
      Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	    -WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL;
      if (winmgr.fps<10)
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
      StrCpy(Fs->task_title,old_task_title);
      Fs->border_src=old_border_src;
      Fs->title_src =old_title_src;

      xx-=StrLen(doc_ce->tag)*FONT_WIDTH;
      eletype=PopUpSpriteMain(&head,&cur_elem_num,doc,doc_ce);
      xx+=StrLen(doc_ce->tag)*FONT_WIDTH;
      insert_pt=SpriteSetSettings(dc,head,cur_elem_num,x,y,
	    &color,&width,&x,&y);
      x+=xx; y+=yy;

      SpriteEdUpdate(doc,doc_ce,head);
      switch (eletype) {
	case SPT_FLOOD_FILL:
	case SPT_FLOOD_FILL_NOT:
	  RegOneTimePopUp(ARf_FLOODFILL,
		ST_WARN_ST "This is affected by what's underneath\n"
		"when it is drawn.  You will probably want to\n"
		"convert it to a bitmap.\n\n"
		"A tip on artistry you might consider\n"
		"is using lines to fill regions because\n"
		"brush strokes look cool.\n");
	  break;
	case SPT_PLANAR_SYMMETRY:
	  RegOneTimePopUp(ARf_PLANAR_SYMMETRY,
		"Right-click to turn-off symmetry.\n");
	  break;
      }
      doc_ce->de_flags=old_de_flags;
    }
    SMTaskTitleSet(eletype);
    switch (eletype) {
      case SPT_COLOR:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor(,,FALSE);
	if (i>=0) {
	  color=i;
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	  tempg->type=SPT_COLOR;
	  tempg->c.color=color;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,doc_ce,head);
	  cur_elem_num++;
	}
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_DITHER_COLOR:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColorDither;
	if (i>=0) {
	  color=i;
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_DITHER_COLOR));
	  tempg->type=SPT_DITHER_COLOR;
	  tempg->d.dither_color=color.c0.color|color.c1.color<<8;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,doc_ce,head);
	  cur_elem_num++;
	}
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_ED_MENU:
	switch (SpriteEd(doc,doc_ce,x,y,&head,&cur_elem_num)) {
	  case GE_ABORT:	eletype=DOCM_CANCEL;	break;
	  case GE_EXIT:		eletype=SPT_EXIT;	break;
	  case GE_CONT:		eletype=SPT_MENU;	break;
	}
	break;
      case SPT_MESH:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	SMMesh(doc,doc_ce,head,&cur_elem_num);
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_SHIFTABLE_MESH:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	if (PopUpNoYes("Study the $$LK,\"X-Caliber\","
		"A=\"FF:::/PersonalMenu.DD.Z,X-Caliber\"$$ icon.\n\n"
		"It has black rectangle background with stars.  The\n"
		"mesh is in front and rotates.  To keep the background\n"
		"rectangle from rotating, "
		"$$GREEN$$TRANSFORM OFF$$FG$$ has been used.\n\n"
		"The X-Caliber mesh has a different origin for rotation.\n"
		"To avoid clipping, it also has also been moved in the\n"
		"negative Z direction by editing the sprite as text\n"
		"and changing the first vector.\n\n"
		"For the mesh you are creating, use same origin as\n"
		"the rest of the sprite?  If $$GREEN$$YES$$FG$$, you can always\n"
		"shift the mesh origin point in the sprite edit menu.\n")) {
	  doc_ce->de_flags=old_de_flags;
	  a1=x; a2=y;
	  SMShiftableMesh(doc,doc_ce,head,x,y,a1,a2,&cur_elem_num);
	  eletype=SPT_MENU;
	} else
	  PopUpOk("Select Origin.\n");
	doc_ce->de_flags=old_de_flags;
	break;
      case SPT_INS_SCREEN_BITMAP:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	PopUpOk("Drag-out a rect for the extents of the\nbitmap.\n");
	doc_ce->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_IP_L|WIF_FOCUS_TASK_IP_R|
	      WIF_FOCUS_TASK_BORDER-WIF_SELF_FOCUS-WIF_SELF_GRAB_SCROLL;
	break;
      case SPT_BITMAP:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor("Background Color\n\n",,FALSE);
	if (i<0)
	  eletype=SPT_MENU;
	else {
	  bm_bkcolor=i;
	  SpriteEdUpdate(doc,doc_ce,head);
	  SpriteExtents(doc_ce->bin_data->data,&xx1,&xx2,&yy1,&yy2);
	  if (!(xx1<=xx2 && yy1<=yy2))
	    xx1=xx2=yy1=yy2=0;
	  if (PopUpExtents(&xx1,&xx2,&yy1,&yy2)) {
	    doc_ce->de_flags=old_de_flags;
	    xx1+=xx; yy1+=yy;
	    xx2+=xx; yy2+=yy;
	    if (SMBitMap(eletype,doc,doc_ce,head,dc,xx,yy,a1,a2,bm_bkcolor,
		  FALSE,xx1,yy1,xx2,yy2,&cur_elem_num)==GE_EXIT) {
	      res=GE_EXIT;
	      goto ei_done;
	    }
	    eletype=SPT_MENU;
	  }
	}
	doc_ce->de_flags=old_de_flags;
	break;
      case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor("Color to Become Transparent\n\n",,FALSE);
	if (i<0)
	  eletype=SPT_MENU;
	else {
	  bm_bkcolor=i;
	  PopUpOk("Drag-out a rect for the extents of the\nbitmap.\n");
	}
	doc_ce->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_IP_L|WIF_FOCUS_TASK_IP_R|
	      WIF_FOCUS_TASK_BORDER-WIF_SELF_FOCUS-WIF_SELF_GRAB_SCROLL;
	break;
      case SPT_WIDTH:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpRangeI64(1,16,1,"Pen Width\n");
	if (i>=1) {
	  width=i;
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_WIDTH));
	  tempg->type=SPT_WIDTH;
	  tempg->w.width=width;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,doc_ce,head);
	  cur_elem_num++;
	}
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_TRANSFORM_ON:
      case SPT_TRANSFORM_OFF:
	tempg=CAlloc(SpriteElemQuedBaseSize(SPT_TRANSFORM_ON));
	if (eletype==SPT_TRANSFORM_ON)
	  tempg->type=SPT_TRANSFORM_ON;
	else
	  tempg->type=SPT_TRANSFORM_OFF;
	QueIns(tempg,insert_pt->last);
	SpriteEdUpdate(doc,doc_ce,head);
	cur_elem_num++;
	eletype=SPT_MENU;
	break;
      case SPT_POLYGON:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpRangeI64(3,16,1,"Num of Sides\n");
	doc_ce->de_flags=old_de_flags;
	if (i<3)
	  eletype=SPT_MENU;
	break;
      case SPT_TEXT:
      case SPT_TEXT_BOX:
      case SPT_TEXT_DIAMOND:
	SMTextFamily(eletype,doc,doc_ce,head,dc,xx,yy,a1,a2,
	      color,&cur_elem_num,old_de_flags);
	eletype=SPT_MENU;
	break;
    }

    if (eletype!=SPT_MENU) {
      insert_pt=SpriteSetSettings(dc,head,cur_elem_num,xx,yy,
	    &color,&width,&x,&y);
      x+=xx; y+=yy;
      SpriteScreenInit(dc,x,y);
      if (eletype==SPT_EXIT) {
	res=GE_EXIT;
	goto ei_done;
      } else if (eletype==DOCM_CANCEL) {
	res=GE_ABORT;
	goto ei_done;
      }
      switch (GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_R_UP|1<<MSG_IP_L_DOWN)) {
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case CH_ESC:
	      res=GE_EXIT;
	      goto ei_done;
	    case CH_SHIFT_ESC:
	      res=GE_ABORT;
	      goto ei_done;
	    case 'c': //eye-dropper
	      dc3=DCScreenCapture(FALSE);
	      color=GrPeek(dc3,ip.pos.x,ip.pos.y)^15;//Mouse cursor is XORed.
	      DCDel(dc3);
	      tempg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	      tempg->type=SPT_COLOR;
	      tempg->c.color=color;
	      QueIns(tempg,insert_pt->last);
	      SpriteEdUpdate(doc,doc_ce,head);
	      cur_elem_num++;
	      break;
	    case 't': //Set to transparent color
	      tempg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	      tempg->type=SPT_COLOR;
	      tempg->c.color=TRANSPARENT;
	      QueIns(tempg,insert_pt->last);
	      SpriteEdUpdate(doc,doc_ce,head);
	      cur_elem_num++;
	      break;
	  }
	  break;
	case MSG_IP_R_UP:
	  if (eletype==SPT_PLANAR_SYMMETRY) {
	    tempg=CAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
	    tempg->type=SPT_PLANAR_SYMMETRY;
	    QueIns(tempg,insert_pt->last);
	    SpriteEdUpdate(doc,doc_ce,head);
	    cur_elem_num++;
	    eletype=SPT_MENU;
	  } else
	    eletype=SPT_MENU;
	  break;
	case MSG_IP_L_DOWN:
	  switch (eletype) {
	    start:
	      case SPT_LINE:
		tempg=SMLine(dc,x,y,a1,a2,color);
		break;
	      case SPT_ARROW:
		tempg=SMArrow(dc,x,y,a1,a2,color);
		break;
	      case SPT_PLANAR_SYMMETRY:
		tempg=SMPlanarSymmetry(dc,x,y,a1,a2);
		eletype=SPT_MENU;
		break;
	      case SPT_RECT:
		tempg=SMRect(dc,x,y,a1,a2,color);
		break;
	      case SPT_INS_SCREEN_BITMAP:
	      case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
		tempg=SMScreenBitMap(eletype,dc,dc2,x,y,a1,a2,bm_bkcolor);
		eletype=SPT_MENU;
		break;
	      case SPT_CIRCLE:
		tempg=SMCircle(dc,x,y,a1,a2,color);
		break;
	      case SPT_ELLIPSE:
		tempg=SMEllipse(dc,x,y,a1,a2,color);
		break;
	      case SPT_POLYGON:
		tempg=SMPolygon(dc,x,y,a1,a2,i,color);
		eletype=SPT_MENU;
		break;
	      case SPT_PT:
	      case SPT_FLOOD_FILL:
	      case SPT_FLOOD_FILL_NOT:
		tempg=CAlloc(SpriteElemQuedBaseSize(eletype));
		tempg->type=eletype;
		tempg->p.x1=a1-x;
		tempg->p.y1=a2-y;
		break;
	      case SPT_POLYLINE:
	      case SPT_BSPLINE2:
	      case SPT_BSPLINE3:
	      case SPT_BSPLINE2_CLOSED:
	      case SPT_BSPLINE3_CLOSED:
		tempg=SMPolyLineFamily(eletype,dc,x,y,a1,a2,color);
		break;
	      case SPT_POLYPT:
		tempg=SMPolyPoint(dc,x,y,a1,a2,color);
		break;
	    end:
	      if (tempg) {
		QueIns(tempg,insert_pt->last);
		SpriteEdUpdate(doc,doc_ce,head);
		cur_elem_num++;
	      }
	      break;
	    case SPT_BITMAP:
	      if (SMBitMap(eletype,doc,doc_ce,head,dc,xx,yy,a1,a2,bm_bkcolor,
		    TRUE,,,,,&cur_elem_num)==GE_EXIT) {
		res=GE_EXIT;
		goto ei_done;
	      }
	      doc_ce->de_flags=old_de_flags;
	      eletype=SPT_MENU;
	      break;
	    case SPT_SHIFTABLE_MESH:
	      GetMsg(NULL,NULL,1<<MSG_IP_L_UP);
	      doc_ce->de_flags|=DOCEF_DONT_DRAW;
	      SMShiftableMesh(doc,doc_ce,head,x,y,a1,a2,&cur_elem_num);
	      doc_ce->de_flags=old_de_flags;
	      eletype=SPT_MENU;
	      break;
	  }
	  break;
      }
    }
  }
ei_done:
  DCFill;
  SettingsPop;
  doc_ce->de_flags=old_de_flags;
  DCDel(dc);
  DCDel(dc2);
  StrCpy(Fs->task_title,old_task_title);
  Free(old_task_title);
  Fs->border_src=old_border_src;
  Fs->title_src =old_title_src;
  QueDel(head);
  Free(head);
  return res;
}

U0 EdSpriteIns(CDoc *doc)
{
  Bool unlock;
  U8 *st;
  CDocEntry *doc_e;
  CDocBin *tempb;
  if (Fs!=doc->mem_task)
    throw('Graphics');
  if (st=EdSprite(doc->cur_bin_num)) {
    unlock=DocLock(doc);
    tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
    tempb->size=sprite_elem_base_sizes[SPT_END];
    tempb->data=CAlloc(tempb->size,doc->mem_task);
    tempb->num=doc->cur_bin_num++;
    tempb->use_cnt=1;
    QueIns(tempb,doc->bin_head.last);
    doc_e=DocPrint(doc,"%s",st);
    doc_e->bin_data=tempb;
    Free(st);
    if (doc_e) {
      if (doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
	tempb->tag=StrNew(doc_e->tag,doc->mem_task);
      doc->cur_entry=doc_e;
      doc->cur_col=0;
      DocUnlock(doc);
      DocRecalc(doc);
      if (SpriteMainEd(doc)==GE_ABORT) {
	DocLock(doc);
	DocEntryDel(doc,doc_e);
      }
    } else
      DocBinDel(doc,tempb);
    if (unlock)
      DocUnlock(doc);
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)))
    DocBinsValidate(doc);
}

U0 EdSpriteEd(CDoc *doc)
{
  CDocEntry *doc_ce;
  CDocBin *tempb;
  CSprite *old_csprite;
  I64	old_size;
  Bool unlock=DocLock(doc);
  doc_ce=doc->cur_entry;
  tempb=doc_ce->bin_data;
  old_size=tempb->size;
  old_csprite=tempb->data;
  tempb->data=MAllocIdent(old_csprite,doc->mem_task);
  DocUnlock(doc);
  if (SpriteMainEd(doc)==GE_ABORT) {
    DocLock(doc);
    Free(tempb->data);
    tempb->data=old_csprite;
    tempb->size=old_size;
  } else
    Free(old_csprite);
  if (unlock)
    DocUnlock(doc);
}
