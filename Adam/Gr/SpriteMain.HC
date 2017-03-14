#help_index "Graphics/Sprite;Sprites"

#define SPT_MENU	-2
#define SPT_INS_SCRN_BITMAP		-3
#define SPT_INS_TRANSPARENT_SCRN_BITMAP	-4
#define SPT_ED_MENU	-5
#define SPT_EXIT	-6

I64 PopUpSpriteMain(CSprite **_head,I64 *_cur_elem_num,
	CDoc *_doc,CDocEntry *_doc_e)
{
  CTask	*pu_task;
  I64	res;
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
	"$$MU-UL,\"Thick\",LE=SPT_THICK$$\n"
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
	"$$MU-UL,\"Insert Scrn-Captured BitMap\",LE=SPT_INS_SCRN_BITMAP$$\n"
	"$$MU-UL,\"Insert Transparent Scrn-Captured BitMap\","
	"LE=SPT_INS_TRANSPARENT_SCRN_BITMAP$$\n"
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
  res=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_head=SpriteSideBar2SpriteQue(DocPut(pu_task),*_head,_cur_elem_num);
    Kill(pu_task);
  }
  Free(_doc_e->tag);
  _doc_e->tag=StrNew(doc_de->data,_doc->mem_task);
  _doc->cur_col=0;
  DocDel(doc);
  return res;
}

Bool PopUpExtents(I64 *_x1,I64 *_x2,I64 *_y1,I64 *_y2)
{
  I64 res;
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
  do res=PopUpMenu(doc);
  while (res!=FALSE && res!=TRUE);
  DocDel(doc);
  return res;
}

U0 SpriteScrnInit(CDC *dc,I64,I64)
{
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 xx,yy,old_pen_width=dc->thick;
  CColorROPU32 old_color=dc->color;
  Refresh;
  DCFill(dc);
  if (dc->flags&DCF_SYMMETRY) {
    dc->flags&=~DCF_SYMMETRY;
    dc->thick=1;
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
  dc->thick=old_pen_width;
}

CSprite *SMLine(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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

CSprite *SMArrow(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrArrow3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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

CSprite *SMPlanarSymmetry(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2,old_width,old_flags;
  CSprite *res;
  old_width=dc->thick;
  old_flags=dc->flags;
  dc->flags&=~DCF_SYMMETRY;
  dc->thick=1;
  do {
    dc->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
  dc->flags=old_flags&DCF_SYMMETRY|dc->flags&~DCF_SYMMETRY;
  dc->thick=old_width;
  res=CAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
  res->type=SPT_PLANAR_SYMMETRY;
  res->pp.x1=x1-x;
  res->pp.y1=y1-y;
  res->pp.x2=x2-x;
  res->pp.y2=y2-y;
  return res;
}

CSprite *SMRect(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2,
	xx1=arg1,yy1=arg2,xx2=arg1,yy2=arg2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
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
  } while (msg_code!=MSG_MS_L_UP);
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

CSprite *SMScrnBitMap(I64 eletype,CDC *dc,CDC *dc2,I64 x,I64 y,
	I64 arg1,I64 arg2,CColorROPU32 bm_bkcolor)
{
  I64 i,msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2,
	xx1=arg1,yy1=arg2,xx2=arg1,yy2=arg2,old_width;
  CDC *dc3,*img;
  CSprite *res;
  old_width=dc2->thick;
  dc2->thick=1;
  do {
    dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrBorder(dc2,xx1+Fs->pix_left+Fs->scroll_x,yy1+Fs->pix_top+Fs->scroll_y,
	  xx2+Fs->pix_left+Fs->scroll_x,yy2+Fs->pix_top+Fs->scroll_y);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
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
  } while (msg_code!=MSG_MS_L_UP);
  xx2++; yy2++;
  res=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+((xx2-xx1+7)&~7)*(yy2-yy1));
  res->type=SPT_BITMAP;
  res->pwhu.width=xx2-xx1;
  res->pwhu.height=yy2-yy1;
  res->pwhu.x1=0;
  res->pwhu.y1=0;
  SpriteScrnInit(dc,x,y);
  i=gr.scrn_zoom;
  GrScaleZoom(1.0/i);
  Refresh(2,TRUE);

  dc3=DCScrnCapture;
  img=DCExt(dc3,Fs->pix_left+Fs->scroll_x+xx1,Fs->pix_top+Fs->scroll_y+yy1,
	Fs->pix_left+Fs->scroll_x+xx2-1,Fs->pix_top+Fs->scroll_y+yy2-1);
  if (eletype==SPT_INS_TRANSPARENT_SCRN_BITMAP)
    DCColorChg(img,bm_bkcolor);
  GrScaleZoom(i);
  MemCpy(&res->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
  DCDel(img);
  DCDel(dc3);
  dc2->thick=old_width;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL;
  return res;
}

CSprite *SMCircle(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
  dc->color=color&COLORROP_NO_ROP0_MASK;
  GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
  res=CAlloc(SpriteElemQuedBaseSize(SPT_CIRCLE));
  res->type=SPT_CIRCLE;
  res->pr.x1=x1-x;
  res->pr.y1=y1-y;
  res->pr.radius=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
  return res;
}

CSprite *SMEllipse(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  F64 angle1,angle2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrEllipse3(dc,(x1+x2)>>1,(y1+y2)>>1,0,AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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

CSprite *SMPolygon(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,I64 sides,
	CColorROPU32 color)
{
  I64 msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  F64 angle1,angle2;
  CSprite *res;
  do {
    dc->color=color&COLORROP_NO_ROP0_MASK;
    GrRegPoly3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
	  AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1,sides);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    SpriteScrnInit(dc,x,y);
    x2=arg1; y2=arg2;
  } while (msg_code!=MSG_MS_L_UP);
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

CSprite *SMPolyLineFamily(I64 eletype,CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,
	CColorROPU32 color)
{
  I64 i,num=0,msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2;
  I32 *ptr;
  CD3I32 *p;
  CSprite *res,*tmpg,*tmpg1,head2;

  QueInit(&head2);
  do {
    do {
      dc->color=color&COLORROP_NO_ROP0_MASK;
      if (num)
	GrLine3(dc,x1,y1,0,x2,y2,0);
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_MS_L_UP+1<<MSG_MS_MOVE+1<<MSG_MS_R_UP);
      dc->color=TRANSPARENT;
      if (num)
	GrLine3(dc,x1,y1,0,x2,y2,0);
      x2=arg1; y2=arg2;
    } while (msg_code!=MSG_MS_L_UP && msg_code!=MSG_MS_R_UP);
    dc->color=color&COLORROP_NO_ROP0_MASK;
    if (msg_code==MSG_MS_L_UP) {
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
  } while (msg_code!=MSG_MS_R_UP);

  switch (eletype) {
    case SPT_POLYLINE:
      if (num>1) {
	res=CAlloc(SpriteElemQuedBaseSize(SPT_POLYLINE)+num*sizeof(CD2I32));
	ptr=&res->nu.u;
	tmpg1=head2.next;
	for (i=0;i<num;i++) {
	  tmpg=tmpg1->next;
	  ptr[i<<1]=tmpg1->p.x1;
	  ptr[i<<1+1]=tmpg1->p.y1;
	  Free(tmpg1);
	  tmpg1=tmpg;
	}
	res->type=SPT_POLYLINE;
	res->nu.num=num;
      } else {
	tmpg1=head2.next;
	for (i=0;i<num;i++) {
	  tmpg=tmpg1->next;
	  Free(tmpg1);
	  tmpg1=tmpg;
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
	tmpg1=head2.next;
	for (i=0;i<num;i++) {
	  tmpg=tmpg1->next;
	  p[i].x=tmpg1->p.x1;
	  p[i].y=tmpg1->p.y1;
	  Free(tmpg1);
	  tmpg1=tmpg;
	}
	res->type=eletype;
	res->nu.num=num;
      } else {
	tmpg1=head2.next;
	for (i=0;i<num;i++) {
	  tmpg=tmpg1->next;
	  Free(tmpg1);
	  tmpg1=tmpg;
	}
	res=NULL;
      }
      break;
  }
  return res;
}

CSprite *SMPolyPoint(CDC *dc,I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color)
{
  I64 i,num=0,msg_code,x1=arg1,y1=arg2,x2=arg1,y2=arg2,x3,y3;
  I32 *ptr;
  CSprite *res,*tmpg,*tmpg1,head2;

  QueInit(&head2);
  x3=arg1-x; y3=arg2-y;
  dc->color=color&COLORROP_NO_ROP0_MASK;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
    x2=arg1; y2=arg2;
    GrLine3(dc,x1,y1,0,x2,y2,0);
    Line(&head2,x1-x,y1-y,0,x2-x,y2-y,0,&SpritePolyPtPlot);
    x1=x2;y1=y2;
  } while (msg_code!=MSG_MS_L_UP);

  num=0;
  res=head2.next;
  x1=x3; y1=y3;
  while (res!=&head2) {
    tmpg=res->next;
    if (res->p.x1==x1 && res->p.y1==y1) {
      QueRem(res);
      Free(res);
    } else {
      num++;
      x1=res->p.x1;
      y1=res->p.y1;
    }
    res=tmpg;
  }

  res=CAlloc(SpriteElemQuedBaseSize(SPT_POLYPT)+(num*3+7)>>3);
  res->npu.x=x3;
  res->npu.y=y3;
  ptr=&res->npu.u;
  x1=x3; y1=y3;
  i=0;
  tmpg1=head2.next;
  while (tmpg1!=&head2) {
    tmpg=tmpg1->next;
    BFieldOrU32(ptr,i,polypt_map[SignI64(tmpg1->p.x1-x1)+1+
	  3*(SignI64(tmpg1->p.y1-y1)+1)]);
    i+=3;
    x1=tmpg1->p.x1;y1=tmpg1->p.y1;
    QueRem(tmpg1);
    Free(tmpg1);
    tmpg1=tmpg;
  }
  res->type=SPT_POLYPT;
  res->npu.num=num;
  return res;
}

U0 SMTextFamily(I64 eletype,CDoc *doc,CDocEntry *doc_ce,CSprite *head,CDC *dc,
	I64 x,I64 y,I64 arg1,I64 arg2,CColorROPU32 color,I64 *_cur_elem_num,
	I64 old_de_flags)
{
  CSprite *tmpg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num);
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
      msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
      SpriteScrnInit(dc,x,y);
      x1=arg1; y1=arg2;
    } while (msg_code!=MSG_MS_L_UP);
    tmpg=CAlloc(SpriteElemQuedBaseSize(eletype)+StrLen(st)+1);
    tmpg->type=eletype;
    tmpg->ps.x1=x1-x;
    tmpg->ps.y1=y1-y;
    StrCpy(tmpg->ps.st,st);
    QueIns(tmpg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    *_cur_elem_num+=1;
  }
  Free(st);
}

I64 SMBitMap(I64 eletype,CDoc *doc,CDocEntry *doc_ce,CSprite *head,
	CDC *dc,I64 xx,I64 yy,I64 arg1,I64 arg2,CColorROPU32 bm_bkcolor,
	Bool sel,I64 xx1=0,I64 yy1=0,I64 xx2=0,I64 yy2=0,I64 *_cur_elem_num)
{
  I64 i,msg_code,x=xx,y=yy,x1=arg1,y1=arg2,x2=arg1,y2=arg2,old_width;
  CSprite *tmpg,*tmpg1,*insert_pt;
  CDC *img;

  insert_pt=SpriteSetSettings(,head,*_cur_elem_num,xx,yy,,,&x,&y);
  x+=xx; y+=yy;

  if (sel) {
    xx1=arg1; yy1=arg2;
    xx2=arg1; yy2=arg2;
    old_width=dc->thick;
    dc->thick=1;
    do {
      dc->color=ROPF_DITHER+WHITE<<16+BLACK;
      GrBorder(dc,xx1,yy1,xx2,yy2);
      msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
      SpriteScrnInit(dc,x,y);
      x2=arg1; y2=arg2;
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
    } while (msg_code!=MSG_MS_L_UP);
    dc->thick=old_width;
  }

  xx2++; yy2++;
  tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+
	((xx2-xx1+7)&~7)*(yy2-yy1));
  tmpg->type=SPT_BITMAP;
  tmpg->pwhu.width=xx2-xx1;
  tmpg->pwhu.height=yy2-yy1;
  tmpg->pwhu.x1=xx1-x;
  tmpg->pwhu.y1=yy1-y;
  img=DCNew(tmpg->pwhu.width,tmpg->pwhu.height,Fs);
  img->color=bm_bkcolor;
  GrRect(img,0,0,tmpg->pwhu.width,tmpg->pwhu.height);
  tmpg1=insert_pt;
  if (tmpg1==head || tmpg1->type&SPG_TYPE_MASK!=SPT_BITMAP) {
    SpriteSetSettings(img,head,0,-(xx1-x),-(yy1-y));
    x=xx; y=yy;
    Sprite3(img,-(xx1-x),-(yy1-y),0,doc_ce->bin_data->data);
    QueDel(head);
    insert_pt=head->next=head->last=head;
    *_cur_elem_num=1;
  } else {
    SpriteSetSettings(img,head,*_cur_elem_num,-(xx1-x),-(yy1-y));
    Sprite3(img,-(xx1-x),-(yy1-y),0,&tmpg1->start,TRUE);
    insert_pt=tmpg1->next;
    QueRem(tmpg1);
    Free(tmpg1);
  }
  MemCpy(&tmpg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));

  switch (i=SpriteBitMapEd(doc,doc_ce,dc,&xx1,&yy1,
	&xx2,&yy2,&img,bm_bkcolor)) {
    case SPE_EXIT:
    case SPE_CONT:
      Free(tmpg);
      tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+
	    ((xx2-xx1+7)&~7)*(yy2-yy1));
      tmpg->type=eletype;
      tmpg->pwhu.width=xx2-xx1;
      tmpg->pwhu.height=yy2-yy1;
      tmpg->pwhu.x1=xx1-x;
      tmpg->pwhu.y1=yy1-y;
      MemCpy(&tmpg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
      break;
  }
  QueIns(tmpg,insert_pt->last);
  DCDel(img);
  SpriteEdUpdate(doc,doc_ce,head);
  return i;
}

U0 SMMesh(CDoc *doc,CDocEntry *doc_ce,CSprite *head,I64 *_cur_elem_num)
{
  CSprite *tmpg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num),
	*tmpg1=insert_pt;
  CD3I32 *p;
  I64 i,size,x1,y1,z1;
  I32 *mesh,*old_mesh;
  if (tmpg1!=head && tmpg1->type&SPG_TYPE_MASK==SPT_MESH)
    old_mesh=&tmpg1->mu.vertex_cnt;
  else if (tmpg1!=head && tmpg1->type&SPG_TYPE_MASK==SPT_SHIFTABLE_MESH) {
    x1=tmpg1->pmu.x;
    y1=tmpg1->pmu.y;
    z1=tmpg1->pmu.z;
    p=&tmpg1->pmu.u;
    for (i=0;i<tmpg1->pmu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tmpg1->pmu.vertex_cnt;
  } else
    old_mesh=NULL;
  if (mesh=SpriteMeshEd(old_mesh,&size,TRUE)) {
    tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_MESH)-sizeof(I32)*2+size);
    tmpg->type=SPT_MESH;
    MemCpy(&tmpg->mu.vertex_cnt,mesh,size);
    Free(mesh);
    QueIns(tmpg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    if (old_mesh) {
      insert_pt=tmpg;
      QueRem(tmpg1);
      Free(tmpg1);
      SpriteEdUpdate(doc,doc_ce,head);
    } else
      *_cur_elem_num+=1;
  } else if (old_mesh && tmpg1->type&SPG_TYPE_MASK==SPT_SHIFTABLE_MESH) {
    x1=tmpg1->pmu.x;
    y1=tmpg1->pmu.y;
    z1=tmpg1->pmu.z;
    p=&tmpg1->pmu.u;
    for (i=0;i<tmpg1->pmu.vertex_cnt;i++,p++) {
      p->x-=x1;
      p->y-=y1;
      p->z-=z1;
    }
  }
}

U0 SMShiftableMesh(CDoc *doc,CDocEntry *doc_ce,CSprite *head,
	I64 x,I64 y,I64 arg1,I64 arg2,I64 *_cur_elem_num)
{
  CSprite *tmpg,*insert_pt=SpriteSetSettings(,head,*_cur_elem_num),
	*tmpg1=insert_pt;
  CD3I32 *p;
  I64 i,size,z,x1,y1,z1;
  I32 *mesh,*old_mesh;
  if (tmpg1!=head && tmpg1->type&SPG_TYPE_MASK==SPT_MESH) {
    z=0;
    x1=-(arg1-x);
    y1=-(arg2-y);
    z1=z;
    p=&tmpg1->mu.u;
    for (i=0;i<tmpg1->mu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tmpg1->mu.vertex_cnt;
  } else if (tmpg1!=head && tmpg1->type&SPG_TYPE_MASK==SPT_SHIFTABLE_MESH) {
    z=-tmpg1->pmu.z;
    x1=tmpg1->pmu.x-(arg1-x);
    y1=tmpg1->pmu.y-(arg2-y);
    z1=tmpg1->pmu.z+z;
    p=&tmpg1->pmu.u;
    for (i=0;i<tmpg1->pmu.vertex_cnt;i++,p++) {
      p->x+=x1;
      p->y+=y1;
      p->z+=z1;
    }
    old_mesh=&tmpg1->pmu.vertex_cnt;
  } else {
    z=0;
    old_mesh=NULL;
  }
  if (mesh=SpriteMeshEd(old_mesh,&size,TRUE)) {
    tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_SHIFTABLE_MESH)-sizeof(I32)*2+size);
    tmpg->type=SPT_SHIFTABLE_MESH;
    MemCpy(&tmpg->pmu.vertex_cnt,mesh,size);
    Free(mesh);
    tmpg->pmu.x=arg1-x;
    tmpg->pmu.y=arg2-y;
    tmpg->pmu.z=-z;
    QueIns(tmpg,insert_pt->last);
    SpriteEdUpdate(doc,doc_ce,head);
    if (old_mesh) {
      insert_pt=tmpg;
      QueRem(tmpg1);
      Free(tmpg1);
      SpriteEdUpdate(doc,doc_ce,head);
    } else
      *_cur_elem_num+=1;
  } else if (old_mesh && tmpg1->type&SPG_TYPE_MASK==SPT_SHIFTABLE_MESH) {
    x1=tmpg1->pmu.x-(arg1-x);
    y1=tmpg1->pmu.y-(arg2-y);
    z1=tmpg1->pmu.z+z;
    p=&tmpg1->pmu.u;
    for (i=0;i<tmpg1->pmu.vertex_cnt;i++,p++) {
      p->x-=x1;
      p->y-=y1;
      p->z-=z1;
    }
  } else if (old_mesh && tmpg1->type&SPG_TYPE_MASK==SPT_MESH) {
    x1=-(arg1-x);
    y1=-(arg2-y);
    z1= z;
    p=&tmpg1->mu.u;
    for (i=0;i<tmpg1->mu.vertex_cnt;i++,p++) {
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
    case SPT_INS_SCRN_BITMAP:
      StrCpy(Fs->task_title,"Insert Scrn BitMap");
      break;
    case SPT_INS_TRANSPARENT_SCRN_BITMAP:
      StrCpy(Fs->task_title,"Insert Transparent Scrn BitMap");
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
  I64 res,i,x,y,arg1,arg2,xx,yy,xx1,yy1,xx2,yy2,thick,eletype=SPT_MENU,
	cur_elem_num,old_border_src=Fs->border_src,old_title_src=Fs->title_src,
	old_de_flags=doc_ce->de_flags;
  CColorROPU32 bm_bkcolor,color;
  CSprite *head,*tmpg,*insert_pt;
  CDC *dc=DCAlias(,Fs),*dc2=DCAlias(,sys_winmgr_task),*dc3;
  U8 *old_task_title=StrNew(Fs->task_title);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  Refresh(2,TRUE);
  dc2->flags|=DCF_ON_TOP;
  head=Sprite2SpriteQue(doc_ce->bin_data->data);
  cur_elem_num=QueCnt(head);
  xx=(doc_ce->x+doc_ce->max_col-doc->line_start_col)*FONT_WIDTH;
  yy=(doc_ce->y-doc->top_line_num)*FONT_HEIGHT;

  while (TRUE) {
    insert_pt=SpriteSetSettings(dc,head,cur_elem_num,xx,yy,
	  &color,&thick,&x,&y);
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
	    &color,&thick,&x,&y);
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
	  tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	  tmpg->type=SPT_COLOR;
	  tmpg->c.color=color;
	  QueIns(tmpg,insert_pt->last);
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
	  tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_DITHER_COLOR));
	  tmpg->type=SPT_DITHER_COLOR;
	  tmpg->d.dither_color=color.c0.color|color.c1.color<<8;
	  QueIns(tmpg,insert_pt->last);
	  SpriteEdUpdate(doc,doc_ce,head);
	  cur_elem_num++;
	}
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_ED_MENU:
	switch (SpriteEd(doc,doc_ce,x,y,&head,&cur_elem_num)) {
	  case SPE_ABORT:	eletype=DOCM_CANCEL;	break;
	  case SPE_EXIT:	eletype=SPT_EXIT;	break;
	  case SPE_CONT:	eletype=SPT_MENU;	break;
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
	  arg1=x; arg2=y;
	  SMShiftableMesh(doc,doc_ce,head,x,y,arg1,arg2,&cur_elem_num);
	  eletype=SPT_MENU;
	} else
	  PopUpOk("Select Origin.\n");
	doc_ce->de_flags=old_de_flags;
	break;
      case SPT_INS_SCRN_BITMAP:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	PopUpOk("Drag-out a rect for the extents of the\nbitmap.\n");
	doc_ce->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_MS_L|WIF_FOCUS_TASK_MS_R|
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
	    if (SMBitMap(eletype,doc,doc_ce,head,dc,xx,yy,arg1,arg2,bm_bkcolor,
		  FALSE,xx1,yy1,xx2,yy2,&cur_elem_num)==SPE_EXIT) {
	      res=SPE_EXIT;
	      goto ei_done;
	    }
	    eletype=SPT_MENU;
	  }
	}
	doc_ce->de_flags=old_de_flags;
	break;
      case SPT_INS_TRANSPARENT_SCRN_BITMAP:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor("Color to Become Transparent\n\n",,FALSE);
	if (i<0)
	  eletype=SPT_MENU;
	else {
	  bm_bkcolor=i;
	  PopUpOk("Drag-out a rect for the extents of the\nbitmap.\n");
	}
	doc_ce->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_MS_L|WIF_FOCUS_TASK_MS_R|
	      WIF_FOCUS_TASK_BORDER-WIF_SELF_FOCUS-WIF_SELF_GRAB_SCROLL;
	break;
      case SPT_THICK:
	doc_ce->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpRangeI64(1,16,1,"Thick\n");
	if (i>=1) {
	  thick=i;
	  tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_THICK));
	  tmpg->type=SPT_THICK;
	  tmpg->t.thick=thick;
	  QueIns(tmpg,insert_pt->last);
	  SpriteEdUpdate(doc,doc_ce,head);
	  cur_elem_num++;
	}
	doc_ce->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_TRANSFORM_ON:
      case SPT_TRANSFORM_OFF:
	tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_TRANSFORM_ON));
	if (eletype==SPT_TRANSFORM_ON)
	  tmpg->type=SPT_TRANSFORM_ON;
	else
	  tmpg->type=SPT_TRANSFORM_OFF;
	QueIns(tmpg,insert_pt->last);
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
	SMTextFamily(eletype,doc,doc_ce,head,dc,xx,yy,arg1,arg2,
	      color,&cur_elem_num,old_de_flags);
	eletype=SPT_MENU;
	break;
    }

    if (eletype!=SPT_MENU) {
      insert_pt=SpriteSetSettings(dc,head,cur_elem_num,xx,yy,
	    &color,&thick,&x,&y);
      x+=xx; y+=yy;
      SpriteScrnInit(dc,x,y);
      if (eletype==SPT_EXIT) {
	res=SPE_EXIT;
	goto ei_done;
      } else if (eletype==DOCM_CANCEL) {
	res=SPE_ABORT;
	goto ei_done;
      }
      switch (GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN|1<<MSG_MS_R_UP|1<<MSG_MS_L_DOWN)) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case CH_ESC:
	      res=SPE_EXIT;
	      goto ei_done;
	    case CH_SHIFT_ESC:
	      res=SPE_ABORT;
	      goto ei_done;
	    case 'c': //eye-dropper
	      dc3=DCScrnCapture(FALSE);
	      color=GrPeek(dc3,ms.pos.x,ms.pos.y)^15;//Mouse cursor is XORed.
	      DCDel(dc3);
	      tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	      tmpg->type=SPT_COLOR;
	      tmpg->c.color=color;
	      QueIns(tmpg,insert_pt->last);
	      SpriteEdUpdate(doc,doc_ce,head);
	      cur_elem_num++;
	      break;
	    case 't': //Set to transparent color
	      tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	      tmpg->type=SPT_COLOR;
	      tmpg->c.color=TRANSPARENT;
	      QueIns(tmpg,insert_pt->last);
	      SpriteEdUpdate(doc,doc_ce,head);
	      cur_elem_num++;
	      break;
	  }
	  break;
	case MSG_MS_R_UP:
	  if (eletype==SPT_PLANAR_SYMMETRY) {
	    tmpg=CAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
	    tmpg->type=SPT_PLANAR_SYMMETRY;
	    QueIns(tmpg,insert_pt->last);
	    SpriteEdUpdate(doc,doc_ce,head);
	    cur_elem_num++;
	    eletype=SPT_MENU;
	  } else
	    eletype=SPT_MENU;
	  break;
	case MSG_MS_L_DOWN:
	  switch (eletype) {
	    start:
	      case SPT_LINE:
		tmpg=SMLine(dc,x,y,arg1,arg2,color);
		break;
	      case SPT_ARROW:
		tmpg=SMArrow(dc,x,y,arg1,arg2,color);
		break;
	      case SPT_PLANAR_SYMMETRY:
		tmpg=SMPlanarSymmetry(dc,x,y,arg1,arg2);
		eletype=SPT_MENU;
		break;
	      case SPT_RECT:
		tmpg=SMRect(dc,x,y,arg1,arg2,color);
		break;
	      case SPT_INS_SCRN_BITMAP:
	      case SPT_INS_TRANSPARENT_SCRN_BITMAP:
		tmpg=SMScrnBitMap(eletype,dc,dc2,x,y,arg1,arg2,bm_bkcolor);
		eletype=SPT_MENU;
		break;
	      case SPT_CIRCLE:
		tmpg=SMCircle(dc,x,y,arg1,arg2,color);
		break;
	      case SPT_ELLIPSE:
		tmpg=SMEllipse(dc,x,y,arg1,arg2,color);
		break;
	      case SPT_POLYGON:
		tmpg=SMPolygon(dc,x,y,arg1,arg2,i,color);
		eletype=SPT_MENU;
		break;
	      case SPT_PT:
	      case SPT_FLOOD_FILL:
	      case SPT_FLOOD_FILL_NOT:
		tmpg=CAlloc(SpriteElemQuedBaseSize(eletype));
		tmpg->type=eletype;
		tmpg->p.x1=arg1-x;
		tmpg->p.y1=arg2-y;
		break;
	      case SPT_POLYLINE:
	      case SPT_BSPLINE2:
	      case SPT_BSPLINE3:
	      case SPT_BSPLINE2_CLOSED:
	      case SPT_BSPLINE3_CLOSED:
		tmpg=SMPolyLineFamily(eletype,dc,x,y,arg1,arg2,color);
		break;
	      case SPT_POLYPT:
		tmpg=SMPolyPoint(dc,x,y,arg1,arg2,color);
		break;
	    end:
	      if (tmpg) {
		QueIns(tmpg,insert_pt->last);
		SpriteEdUpdate(doc,doc_ce,head);
		cur_elem_num++;
	      }
	      break;
	    case SPT_BITMAP:
	      if (SMBitMap(eletype,doc,doc_ce,head,dc,xx,yy,
		    arg1,arg2,bm_bkcolor,TRUE,,,,,&cur_elem_num)==SPE_EXIT) {
		res=SPE_EXIT;
		goto ei_done;
	      }
	      doc_ce->de_flags=old_de_flags;
	      eletype=SPT_MENU;
	      break;
	    case SPT_SHIFTABLE_MESH:
	      GetMsg(NULL,NULL,1<<MSG_MS_L_UP);
	      doc_ce->de_flags|=DOCEF_DONT_DRAW;
	      SMShiftableMesh(doc,doc_ce,head,x,y,arg1,arg2,&cur_elem_num);
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
  CDocBin *tmpb;
  if (Fs!=doc->mem_task)
    throw('Graphics');
  if (st=EdSprite(doc->cur_bin_num)) {
    unlock=DocLock(doc);
    tmpb=CAlloc(sizeof(CDocBin),doc->mem_task);
    tmpb->size=sprite_elem_base_sizes[SPT_END];
    tmpb->data=CAlloc(tmpb->size,doc->mem_task);
    tmpb->num=doc->cur_bin_num++;
    tmpb->use_cnt=1;
    QueIns(tmpb,doc->bin_head.last);
    doc_e=DocPrint(doc,"%s",st);
    doc_e->bin_data=tmpb;
    Free(st);
    if (doc_e) {
      if (doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
	tmpb->tag=StrNew(doc_e->tag,doc->mem_task);
      doc->cur_entry=doc_e;
      doc->cur_col=0;
      DocUnlock(doc);
      DocRecalc(doc);
      if (SpriteMainEd(doc)==SPE_ABORT) {
	DocLock(doc);
	DocEntryDel(doc,doc_e);
      } else
	SpriteSelAll(tmpb->data,FALSE);
    } else
      DocBinDel(doc,tmpb);
    if (unlock)
      DocUnlock(doc);
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)))
    DocBinsValidate(doc);
}

U0 EdSpriteEd(CDoc *doc)
{
  CDocEntry *doc_ce;
  CDocBin *tmpb;
  CSprite *old_csprite;
  I64	old_size;
  Bool unlock=DocLock(doc);
  doc_ce=doc->cur_entry;
  tmpb=doc_ce->bin_data;
  old_size=tmpb->size;
  old_csprite=tmpb->data;
  tmpb->data=MAllocIdent(old_csprite,doc->mem_task);
  DocUnlock(doc);
  if (SpriteMainEd(doc)==SPE_ABORT) {
    DocLock(doc);
    Free(tmpb->data);
    tmpb->data=old_csprite;
    tmpb->size=old_size;
  } else {
    SpriteSelAll(tmpb->data,FALSE);
    Free(old_csprite);
  }
  if (unlock)
    DocUnlock(doc);
}
