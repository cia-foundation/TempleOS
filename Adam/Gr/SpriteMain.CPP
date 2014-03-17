#help_index "Graphics/Sprite"

#define GRST_MENU	-2
#define GRST_INS_SCREEN_BITMAP			-3
#define GRST_INS_TRANSPARENT_SCREEN_BITMAP	-4
#define GRST_ED_MENU	-5
#define GRST_EXIT	-6

I64 PopUpSpriteMain(CSprite **_root,I64 *_cur_num)
{
  U8 *st;
  CTask *pu_task;
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite Main Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteMain.TXT.Z\"$$\n"
	"\n$$LTBLUE$$$$MU-UL,\"Color (4-bit)\",LE=GRST_COLOR8$$\n"
	"$$MU-UL,\"Dither Color (4-bit)\",LE=GRST_DITHER_COLOR8$$\n"
	"$$MU-UL,\"Width\",LE=GRST_WIDTH$$\n"
	"$$MU-UL,\"Planar Symmetry\",LE=GRST_PLANAR_SYMMETRY$$\n"
	"\n$$MU-UL,\"Point\",LE=GRST_PT$$\n"
	"$$MU-UL,\"Line\",LE=GRST_LINE$$\n"
	"$$MU-UL,\"Arrow\",LE=GRST_ARROW$$\n"
	"$$MU-UL,\"Rect\",LE=GRST_RECT$$\n"
	"$$MU-UL,\"Circle\",LE=GRST_CIRCLE$$\n"
	"$$MU-UL,\"Ellipse\",LE=GRST_ELLIPSE$$\n"
	"$$MU-UL,\"Polygon\",LE=GRST_POLYGON$$\n"
	"$$MU-UL,\"Text\",LE=GRST_TEXT$$\n"
	"$$MU-UL,\"Text Box\",LE=GRST_TEXT_BOX$$\n"
	"$$MU-UL,\"Text Diamond\",LE=GRST_TEXT_DIAMOND$$\n"
	"$$MU-UL,\"Flood Fill\",LE=GRST_FLOOD_FILL$$\n"
	"$$MU-UL,\"Flood Fill Not Color\",LE=GRST_FLOOD_FILL_NOT$$\n"
	"$$MU-UL,\"PolyLine\",LE=GRST_POLYLINE$$\n"
	"$$MU-UL,\"PolyPoint\",LE=GRST_POLYPT$$\n"
	"$$MU-UL,\"BSpline2\",LE=GRST_BSPLINE2$$\n"
	"$$MU-UL,\"BSpline3\",LE=GRST_BSPLINE3$$\n"
	"$$MU-UL,\"BSpline2 Closed\",LE=GRST_BSPLINE2_CLOSED$$\n"
	"$$MU-UL,\"BSpline3 Closed\",LE=GRST_BSPLINE3_CLOSED$$\n"
	"$$MU-UL,\"Insert Screen-Captured BitMap\",LE=GRST_INS_SCREEN_BITMAP$$\n"
	"$$MU-UL,\"Insert Transparent Screen-Captured BitMap\",LE=GRST_INS_TRANSPARENT_SCREEN_BITMAP$$\n"
	"$$PURPLE$$$$MU-UL,\"+] Create or Edit 3D Mesh\",LE=GRST_MESH$$\n"
	"$$MU-UL,\"+] Create or Edit Shiftable 3D Mesh\",LE=GRST_SHIFTABLE_MESH$$\n"
	"$$MU-UL,\"+] Convert to BitMap or Edit BitMap\",LE=GRST_BITMAP8$$$$LTBLUE$$\n"
	"\n$$MU-UL,\"Transform On  (for use with 3D icons)\",LE=GRST_TRANSFORM_ON$$\n"
	"$$MU-UL,\"Transform Off (for use with 3D icons)\",LE=GRST_TRANSFORM_OFF$$\n"
	"\n"
	"$$PURPLE$$$$MU-UL,\"+] Sprite Edit Menu\",LE=GRST_ED_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=GRST_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$\n");
  DocPrint(doc,"\nRight-Click to get back to this menu.");
  st=MPrint("SpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_root,_cur_num);
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

Bool PopUpExtents(I64 *_x1,I64 *_x2,I64 *_y1,I64 *_y2)
{
  I64 i;
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  doc_e=DocPrint(doc,"  $$DA,32,\"x1:%%d\"$$\n");
  doc_e->data=_x1;
  DocDataFmt(doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,\"x2:%%d\"$$\n");
  doc_e->data=_x2;
  DocDataFmt(doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,\"y1:%%d\"$$\n");
  doc_e->data=_y1;
  DocDataFmt(doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,\"y2:%%d\"$$\n\n");
  doc_e->data=_y2;
  DocDataFmt(doc_e);

  DocPrint(doc," $$BT,\"Use These Extents\",LE=TRUE$$");
  DocPrint(doc,"$$CM,3,0$$$$BT,\"Drag-Out New Extents\",LE=FALSE$$\n\n");
  do i=PopUpMenu(doc);
  while (i!=FALSE && i!=TRUE);
  DocDel(doc);
  return i;
}

U0 GrSetSettings(CDC *dc,CSprite *root,I64 cur_num,CColorROPU32 *_color,I64 *_width,I64 x,I64 y)
{
  CSprite *tempg;
  *_color=BLACK;
  *_width=1;
  DCRst(dc);

  tempg=root->next;
  while (cur_num>0 && tempg!=root) {
    switch (tempg->type) {
      case GRST_COLOR8:
	dc->color=*_color=tempg->color8;
	break;
      case GRST_DITHER_COLOR8:
	dc->color=*_color=
	       tempg->dither_color8.u8[0]|
	       tempg->dither_color8.u8[1]<<COLORROP_BITS|ROPF_DITHER;
	break;
      case GRST_WIDTH:
	dc->pen_width=*_width=tempg->width;
	break;
      case GRST_PLANAR_SYMMETRY:
	if (GrSetSymmetry3(dc,tempg->x1+x,tempg->y1+y,0,tempg->x2+x,tempg->y2+y,0,tempg->x2+x,tempg->y2+y,1))
	  dc->flags|=DCF_SYMMETRY;
	else
	  dc->flags&=~DCF_SYMMETRY;
	break;
    }
    cur_num--;
    tempg=tempg->next;
  }
}

U0 GrScreenInit(CDC *dc,I64,I64)
{
//This uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
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

I64 SpriteMain(CDoc *doc,CDocEntry *doc_e,I64 x,I64 y)
{
  I64 result,i,msg_code,z,x1,y1,z1,x2,y2,x3,y3,p1,p2,xx1,yy1,xx2,yy2,
	num,old_width,size,eletype=GRST_MENU,width,cur_num,
	old_border_src=Fs->border_src,
	old_title_src =Fs->title_src,
	old_e_flags1=doc_e->flags1,old_flags;
  CColorROPU32 bm_bkcolor,color;
  CSprite *root,root2,*tempg,*tempg1,*tempg2,*insert_pt;
  CDC *dc=DCAlias(,Fs),
	*dc2=DCAlias(,sys_winmgr_task),
	*dc3,
	*img;
  CD3I32 *p;
  CDocBin *tempb=doc_e->bin_data;
  F64 a1,a2;
  I32 *ptr,*mesh,*old_mesh;
  U8 *st,*old_task_title=StrNew(Fs->task_title);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WordStat;
  WinMgrSync(2,TRUE);
  dc2->flags|=DCF_ON_TOP;
  root=Sprite2Que(tempb->data);
  cur_num=SpriteCntQue(root);
  insert_pt=GrNum2Que(root,cur_num,&x1,&y1);
  x+=x1; y+=y1;

  Fs->win_inhibit=CSPRITE_WIF;
  while (TRUE) {
    GrSetSettings(dc,root,cur_num,&color,&width,x,y);
    DCFill;
    if (eletype==GRST_MENU) {
      if (win_actual_refresh<10)
	doc_e->flags1|=DOCEF1_DONT_DRAW;
      StrCpy(Fs->task_title,old_task_title);
      Fs->border_src=old_border_src;
      Fs->title_src =old_title_src;

      eletype=PopUpSpriteMain(&root,&cur_num);
      GrEdUpdate(doc,tempb,root);
      insert_pt=GrNum2Que(root,cur_num);
      switch (eletype) {
	case GRST_FLOOD_FILL:
	case GRST_FLOOD_FILL_NOT:
	  AcctOneTimePopUp(ARf_FLOODFILL,
		ST_WARN_ST "This is affected by what's underneath\n"
		"when it is drawn.  You will probably want to\n"
		"convert it to a bitmap.\n\n"
		"A tip on artistry you might consider\n"
		"is using lines to fill regions because\n"
		"brush strokes look cool.\n");
	  break;
	case GRST_PLANAR_SYMMETRY:
	  AcctOneTimePopUp(ARf_PLANAR_SYMMETRY,
		"Right-click to turn-off symmetry.\n");
	  break;
      }
      doc_e->flags1=old_e_flags1;
    }

    Fs->title_src=TTS_CONST; //Violates TTS_LOCKED_CONST
    if (eletype<0)
      switch (eletype) {
	case GRST_INS_SCREEN_BITMAP:
	  StrCpy(Fs->task_title,"Insert Screen BitMap");
	  break;
	case GRST_INS_TRANSPARENT_SCREEN_BITMAP:
	  StrCpy(Fs->task_title,"Insert Transparent Screen BitMap");
	  break;
      }
    else
      StrCpy(Fs->task_title,DefineSub(eletype,"ST_SPRITE_TYPES"));
    Fs->border_src=BDS_CONST;

    switch (eletype) {
      case GRST_COLOR8:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColor(,,FALSE);
	if (i>=0) {
	  color=i;
	  tempg=MAlloc(SpriteQuedBaseSize(GRST_COLOR8));
	  tempg->type=GRST_COLOR8;
	  tempg->color8=color;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	doc_e->flags1=old_e_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_DITHER_COLOR8:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColorDither;
	if (i>=0) {
	  color=i;
	  tempg=MAlloc(SpriteQuedBaseSize(GRST_DITHER_COLOR8));
	  tempg->type=GRST_DITHER_COLOR8;
	  tempg->dither_color8=color.c0.color|color.c1.color<<8;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	doc_e->flags1=old_e_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_ED_MENU:
	switch (SpriteEd(doc,doc_e,x,y,&root,&cur_num)) {
	  case GE_ABORT:
	    eletype=DOCM_CANCEL;
	    break;
	  case GE_EXIT:
	    eletype=GRST_EXIT;
	    break;
	  case GE_CONT:
	    eletype=GRST_MENU;
	    break;
	}
	break;
      case GRST_MESH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;

	tempg1=insert_pt;
	if (tempg1!=root && tempg1->type==GRST_MESH)
	  old_mesh=&tempg1->vertex_cnt;
	else if (tempg1!=root && tempg1->type==GRST_SHIFTABLE_MESH) {
	  x1=tempg1->x1;
	  y1=tempg1->y1;
	  z1=tempg1->z1;
	  p=(&tempg1->shiftable_tri_cnt)(U8 *)+sizeof(I32);
	  for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
	    p->x+=x1;
	    p->y+=y1;
	    p->z+=z1;
	  }
	  old_mesh=&tempg1->shiftable_vertex_cnt;
	} else
	  old_mesh=NULL;
	if (mesh=SpriteMesh(old_mesh,&size,TRUE)) {
	  tempg=MAlloc(SpriteQuedBaseSize(GRST_MESH)-sizeof(I32)*2+size);
	  tempg->type=GRST_MESH;
	  MemCpy(&tempg->vertex_cnt,mesh,size);
	  Free(mesh);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  if (old_mesh) {
	    insert_pt=tempg;
	    QueRem(tempg1);
	    Free(tempg1);
	    GrEdUpdate(doc,tempb,root);
	  } else
	    cur_num++;
	} else if (old_mesh && tempg1->type==GRST_SHIFTABLE_MESH) {
	  x1=tempg1->x1;
	  y1=tempg1->y1;
	  z1=tempg1->z1;
	  p=(&tempg1->shiftable_tri_cnt)(U8 *)+sizeof(I32);
	  for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
	    p->x-=x1;
	    p->y-=y1;
	    p->z-=z1;
	  }
	}
	doc_e->flags1=old_e_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_SHIFTABLE_MESH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	if (PopUpNoYes("Use same origin?\n")) {
	  doc_e->flags1=old_e_flags1;
	  p1=x; p2=y;
	  goto ei_shiftable_mesh;
	}
	PopUpOk("Select Origin.\n");
	doc_e->flags1=old_e_flags1;
	break;
      case GRST_INS_SCREEN_BITMAP:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	PopUpOk("Drag-out a rect for the extents of the\n"
	      "bitmap.\n");
	doc_e->flags1=old_e_flags1;
	break;
      case GRST_BITMAP8:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColor("Background Color\n\n",,FALSE);
	if (i<0)
	  eletype=GRST_MENU;
	else {
	  bm_bkcolor=i;
	  GrEdUpdate(doc,tempb,root);
	  SpriteExtents(tempb->data,&xx1,&xx2,&yy1,&yy2);
	  if (!(xx1<=xx2 && yy1<=yy2))
	    xx1=xx2=yy1=yy2=0;
	  if (PopUpExtents(&xx1,&xx2,&yy1,&yy2)) {
	    doc_e->flags1=old_e_flags1;
	    old_width=dc->pen_width;
	    xx1+=x; yy1+=y;
	    xx2+=x; yy2+=y;
	    goto ei_bitmap;
	  }
	}
	doc_e->flags1=old_e_flags1;
	break;
      case GRST_INS_TRANSPARENT_SCREEN_BITMAP:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColor("Color to Become Transparent\n\n",,FALSE);
	if (i<0)
	  eletype=GRST_MENU;
	else {
	  bm_bkcolor=i;
	  PopUpOk("Drag-out a rect for the extents of the\n"
		"bitmap.\n");
	}
	doc_e->flags1=old_e_flags1;
	break;
      case GRST_WIDTH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpRangeI64(1,16,1,"Pen Width\n");
	if (i>=1) {
	  width=i;
	  tempg=MAlloc(SpriteQuedBaseSize(GRST_WIDTH));
	  tempg->type=GRST_WIDTH;
	  tempg->width=width;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	doc_e->flags1=old_e_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_TRANSFORM_ON:
      case GRST_TRANSFORM_OFF:
	tempg=MAlloc(SpriteQuedBaseSize(GRST_TRANSFORM_ON));
	if (eletype==GRST_TRANSFORM_ON)
	  tempg->type=GRST_TRANSFORM_ON;
	else
	  tempg->type=GRST_TRANSFORM_OFF;
	QueIns(tempg,insert_pt->last);
	GrEdUpdate(doc,tempb,root);
	cur_num++;
	eletype=GRST_MENU;
	break;
      case GRST_POLYGON:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpRangeI64(3,16,1,"Num of Sides\n");
	doc_e->flags1=old_e_flags1;
	if (i<3)
	  eletype=GRST_MENU;
	break;
      case GRST_TEXT:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_e_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrPrint3(dc,x1,y1,0,"%s",st);
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteQuedBaseSize(GRST_TEXT)+StrLen(st)+1);
	  tempg->type=GRST_TEXT;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
      case GRST_TEXT_BOX:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_e_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrTextBox3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteQuedBaseSize(GRST_TEXT_BOX)+StrLen(st)+1);
	  tempg->type=GRST_TEXT_BOX;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
      case GRST_TEXT_DIAMOND:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_e_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrTextDiamond3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteQuedBaseSize(GRST_TEXT_DIAMOND)+StrLen(st)+1);
	  tempg->type=GRST_TEXT_DIAMOND;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
    }

    if (eletype!=GRST_MENU) {
ei_restart:
      GrScreenInit(dc,x,y);
      if (eletype==GRST_EXIT) {
	result=GE_EXIT;
	goto ei_done;
      } else if (eletype==DOCM_CANCEL) {
	result=GE_ABORT;
	goto ei_done;
      }
      msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN | 1<<MSG_IP_L_DOWN | 1<<MSG_IP_R_UP);
      if (msg_code==MSG_IP_R_UP) {
	if (eletype==GRST_PLANAR_SYMMETRY) {
	  tempg=CAlloc(SpriteQuedBaseSize(GRST_PLANAR_SYMMETRY));
	  tempg->type=GRST_PLANAR_SYMMETRY;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,root);
	  cur_num++;
	  eletype=GRST_MENU;
	} else
	  eletype=GRST_MENU;
      }
      if (msg_code==MSG_KEY_DOWN) {
	if (p1==CH_ESC) {
	  result=GE_EXIT;
	  goto ei_done;
	} else if (p1==CH_SHIFT_ESC) {
	  result=GE_ABORT;
	  goto ei_done;
	} else
	  goto ei_restart;
      } else {
	x1=p1; y1=p2;
	x2=p1; y2=p2;
	switch (eletype) {
	  sub_switch_start:
	    sub_switch_start:
	      case GRST_LINE:
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&p1,&p2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  GrScreenInit(dc,x,y);
		  x2=p1; y2=p2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(SpriteQuedBaseSize(GRST_LINE));
		tempg->type=GRST_LINE;
		break;
	      case GRST_ARROW:
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  GrArrow3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&p1,&p2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  GrScreenInit(dc,x,y);
		  x2=p1; y2=p2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrArrow3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(SpriteQuedBaseSize(GRST_ARROW));
		tempg->type=GRST_ARROW;
		break;
	      case GRST_PLANAR_SYMMETRY:
		old_width=dc->pen_width;
		old_flags=dc->flags;
		dc->flags&=~DCF_SYMMETRY;
		dc->pen_width=1;
		do {
		  dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&p1,&p2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  GrScreenInit(dc,x,y);
		  x2=p1; y2=p2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->flags=old_flags&DCF_SYMMETRY|dc->flags&~DCF_SYMMETRY;
		dc->pen_width=old_width;
		eletype=GRST_MENU;
		tempg=MAlloc(SpriteQuedBaseSize(GRST_PLANAR_SYMMETRY));
		tempg->type=GRST_PLANAR_SYMMETRY;
		break;
	    sub_switch_end:
	      tempg->x1=x1-x;
	      tempg->y1=y1-y;
	      tempg->x2=x2-x;
	      tempg->y2=y2-y;
	      break;
	    case GRST_RECT:
	      xx1=p1; yy1=p2;
	      xx2=p1; yy2=p2;
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
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
	      tempg=MAlloc(SpriteQuedBaseSize(GRST_RECT));
	      tempg->type=GRST_RECT;
	      tempg->x1=xx1-x;
	      tempg->y1=yy1-y;
	      tempg->x2=xx2-x;
	      tempg->y2=yy2-y;
	      break;
	    case GRST_INS_SCREEN_BITMAP:
	    case GRST_INS_TRANSPARENT_SCREEN_BITMAP:
	      xx1=p1; yy1=p2;
	      xx2=p1; yy2=p2;
	      old_width=dc2->pen_width;
	      dc2->pen_width=1;
	      do {
		dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
		GrLineRect4(dc2,xx1+Fs->pix_left+Fs->scroll_x,yy1+Fs->pix_top+Fs->scroll_y,
		      xx2+Fs->pix_left+Fs->scroll_x,yy2+Fs->pix_top+Fs->scroll_y);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
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
	      tempg=CAlloc(SpriteQuedBaseSize(GRST_BITMAP8)+
		    ((xx2-xx1+7)&~7)*(yy2-yy1));
	      tempg->type=GRST_BITMAP8;
	      tempg->width=xx2-xx1;
	      tempg->height=yy2-yy1;
	      tempg->x2=0;
	      tempg->y2=0;
	      GrScreenInit(dc,x,y);
	      i=gr_screen_zoom;
	      GrScaleZoom(1.0/i);
	      WinMgrSync(2,TRUE);

	      dc3=DCCaptureScreen;
	      img=DCExtract(dc3,
		    Fs->pix_left+Fs->scroll_x+xx1,Fs->pix_top+Fs->scroll_y+yy1,
		    Fs->pix_left+Fs->scroll_x+xx2-1,Fs->pix_top+Fs->scroll_y+yy2-1);
	      if (eletype==GRST_INS_TRANSPARENT_SCREEN_BITMAP)
		DCColorChg(img,bm_bkcolor);
	      GrScaleZoom(i);
	      MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
	      DCDel(img);
	      DCDel(dc3);
	      dc2->pen_width=old_width;
	      eletype=GRST_MENU;
	      break;
	    case GRST_CIRCLE:
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);
	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
	      tempg=MAlloc(SpriteQuedBaseSize(GRST_CIRCLE));
	      tempg->type=GRST_CIRCLE;
	      tempg->x1=x1-x;
	      tempg->y1=y1-y;
	      tempg->x2=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
	      break;
	    case GRST_ELLIPSE:
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrEllipse3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		      AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);
	      tempg=MAlloc(SpriteQuedBaseSize(GRST_ELLIPSE));
	      tempg->type=GRST_ELLIPSE;
	      tempg->x2=(x1+x2)>>1-x;
	      tempg->y2=(y1+y2)>>1-y;
	      tempg->width =AbsI64(x1-x2)>>1;
	      tempg->height=AbsI64(y1-y2)>>1;
	      a2=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
	      if (tempg->width<tempg->height)
		a2-=ã/2.0;
	      do {
		a1=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
		if (tempg->width>=tempg->height)
		  tempg->rot_angle=-(a1-a2);
		else
		  tempg->rot_angle=-(a1-a2)+ã/2.0;
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrEllipse3(dc,tempg->x2+x,tempg->y2+y,0,
		      tempg->width,tempg->height,tempg->rot_angle);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);

	      a1=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
	      if (tempg->width>=tempg->height)
		tempg->rot_angle=-(a1-a2);
	      else
		tempg->rot_angle=-(a1-a2)+ã/2.0;

	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrEllipse3(dc,tempg->x2+x,tempg->y2+y,0,
		    tempg->width,tempg->height,tempg->rot_angle);
	      break;
	    case GRST_POLYGON:
	      eletype=GRST_MENU;
	      if (i<3) {
		tempg=NULL;
		break;
	      }
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRegPoly3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		      AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1,i);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);
	      tempg=MAlloc(SpriteQuedBaseSize(GRST_POLYGON));
	      tempg->type=GRST_POLYGON;
	      tempg->x2=(x1+x2)>>1-x;
	      tempg->y2=(y1+y2)>>1-y;
	      tempg->width =AbsI64(x1-x2)>>1;
	      tempg->height=AbsI64(y1-y2)>>1;
	      tempg->sides=i;
	      a2=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
	      if (tempg->width<tempg->height)
		a2-=ã/2.0;
	      do {
		a1=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
		if (tempg->width>=tempg->height)
		  tempg->rot_angle=-(a1-a2);
		else
		  tempg->rot_angle=-(a1-a2)+ã/2.0;
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRegPoly3(dc,tempg->x2+x,tempg->y2+y,0,
		      tempg->width,tempg->height,tempg->sides,tempg->rot_angle);
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);

	      a1=Arg(x2-(tempg->x2+x),y2-(tempg->y2+y));
	      if (tempg->width>=tempg->height)
		tempg->rot_angle=-(a1-a2);
	      else
		tempg->rot_angle=-(a1-a2)+ã/2.0;

	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrRegPoly3(dc,tempg->x2+x,tempg->y2+y,0,
		    tempg->width,tempg->height,tempg->sides,tempg->rot_angle);
	      break;
	    case GRST_PT:
	    case GRST_FLOOD_FILL:
	    case GRST_FLOOD_FILL_NOT:
	      tempg=MAlloc(SpriteQuedBaseSize(eletype));
	      tempg->type=eletype;
	      tempg->x1=x1-x;
	      tempg->y1=y1-y;
	      break;
	    sub_switch_start:
	      QueInit(&root2);
	      num=0;
	      x3=p1-x; y3=p2-y;
	      do {
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&p1,&p2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE+
			1<<MSG_IP_R_UP);
		  dc->color=COLOR_TRANSPARENT;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  x2=p1; y2=p2;
		} while (msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		if (msg_code==MSG_IP_L_UP) {
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  tempg=MAlloc(SpriteQuedBaseSize(GRST_PT));
		  tempg->type=GRST_PT;
		  tempg->x1=x2-x;
		  tempg->y1=y2-y;
		  QueIns(tempg,root2.last);
		  x1=x2;y1=y2;
		  num++;
		}
	      } while (msg_code!=MSG_IP_R_UP);
	      if (!num) {
		tempg=NULL;
		break;
	      }
	      case GRST_POLYLINE:
		tempg=CAlloc(SpriteQuedBaseSize(GRST_POLYLINE)+
		      (num+1)<<1*sizeof(I32));
		ptr=&tempg->y1;
		tempg2=root2.next;
		ptr[0]=x3;
		ptr[1]=y3;
		ptr+=2;
		for (i=0;i<num;i++) {
		  tempg1=tempg2->next;
		  ptr[i<<1]=tempg2->x1;
		  ptr[i<<1+1]=tempg2->y1;
		  Free(tempg2);
		  tempg2=tempg1;
		}
		tempg->type=GRST_POLYLINE;
		tempg->num=num+1;
		break;
	      case GRST_BSPLINE2:
	      case GRST_BSPLINE3:
	      case GRST_BSPLINE2_CLOSED:
	      case GRST_BSPLINE3_CLOSED:
		tempg=CAlloc(SpriteQuedBaseSize(GRST_POLYLINE)+
		      (num+1)*sizeof(CD3I32));
		p=&tempg->y1;
		tempg2=root2.next;
		p[0].x=x3;
		p[0].y=y3;
		for (i=1;i<=num;i++) {
		  tempg1=tempg2->next;
		  p[i].x=tempg2->x1;
		  p[i].y=tempg2->y1;
		  Free(tempg2);
		  tempg2=tempg1;
		}
		tempg->type=eletype;
		tempg->num=num+1;
		break;
	    sub_switch_end:
	      break;
	    case GRST_POLYPT:
	      QueInit(&root2);
	      x3=p1-x; y3=p2-y;
	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      do {
		msg_code=GetMsg(&p1,&p2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		x2=p1; y2=p2;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		Line(&root2,x1-x,y1-y,0,x2-x,y2-y,0,&GrPolyPtPlot);
		x1=x2;y1=y2;
	      } while (msg_code!=MSG_IP_L_UP);

	      num=0;
	      tempg=root2.next;
	      x1=x3;y1=y3;
	      while (tempg!=&root2) {
		tempg1=tempg->next;
		if (tempg->x1==x1 && tempg->y1==y1) {
		  QueRem(tempg);
		  Free(tempg);
		} else {
		  num++;
		  x1=tempg->x1;
		  y1=tempg->y1;
		}
		tempg=tempg1;
	      }

	      tempg=CAlloc(SpriteQuedBaseSize(GRST_POLYPT)+
		    sizeof(I32)*2+(num*3+7)>>3);
	      ptr=&tempg->y1;
	      ptr[0]=x3;
	      ptr[1]=y3;
	      ptr+=2;
	      x1=x3;y1=y3;
	      i=0;
	      tempg2=root2.next;
	      while (tempg2!=&root2) {
		tempg1=tempg2->next;
		BFieldOrU32(ptr,i,
		      GrSpOffset[SignI64(tempg2->x1-x1)+1+
		      3*(SignI64(tempg2->y1-y1)+1)]);
		i+=3;
		x1=tempg2->x1;y1=tempg2->y1;
		QueRem(tempg2);
		Free(tempg2);
		tempg2=tempg1;
	      }
	      tempg->type=GRST_POLYPT;
	      tempg->num=num;
	      break;
	  sub_switch_end:
	    if (tempg) {
	      QueIns(tempg,insert_pt->last);
	      GrEdUpdate(doc,tempb,root);
	      cur_num++;
	    }
	    break;
	  case GRST_BITMAP8:
	    xx1=p1; yy1=p2;
	    xx2=p1; yy2=p2;
	    old_width=dc->pen_width;
	    dc->pen_width=1;
	    do {
	      dc->color=ROPF_DITHER+WHITE<<16+BLACK;
	      GrLineRect4(dc,xx1,yy1,xx2,yy2);
	      msg_code=GetMsg(&p1,&p2,
		    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
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
ei_bitmap:
	    xx2++; yy2++;
	    tempg=CAlloc(SpriteQuedBaseSize(GRST_BITMAP8)+
		  ((xx2-xx1+7)&~7)*(yy2-yy1));
	    tempg->type=eletype;
	    tempg->width=xx2-xx1;
	    tempg->height=yy2-yy1;
	    tempg->x2=xx1-x;
	    tempg->y2=yy1-y;
	    img=DCNew(tempg->width,tempg->height,Fs);
	    img->color=bm_bkcolor;
	    GrRect(img,0,0,tempg->width,tempg->height);
	    tempg1=insert_pt;
	    if (tempg1==root || tempg1->type!=GRST_BITMAP8) {
	      GrSetSettings(img,root,0,&color,&width,-(xx1-x),-(yy1-y));
	      Sprite3(img,-(xx1-x),-(yy1-y),0,tempb->data);
	      QueDel(root);
	      insert_pt=root->next=root->last=root;
	      cur_num=1;
	    } else {
	      GrSetSettings(img,root,cur_num,&color,&width,-(xx1-x),-(yy1-y));
	      Sprite3(img,-(xx1-x),-(yy1-y),0,tempg1(U8 *)+offset(CSprite.start),TRUE);
	      insert_pt=tempg1->next;
	      QueRem(tempg1);
	      Free(tempg1);
	    }
	    MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));

	    switch (i=SpriteBitMap(doc,doc_e,dc,&xx1,&yy1,&xx2,&yy2,&img,bm_bkcolor)) {
	      case GE_EXIT:
	      case GE_CONT:
		Free(tempg);
		tempg=CAlloc(SpriteQuedBaseSize(GRST_BITMAP8)+
		      ((xx2-xx1+7)&~7)*(yy2-yy1));
		tempg->type=eletype;
		tempg->width=xx2-xx1;
		tempg->height=yy2-yy1;
		tempg->x2=xx1-x;
		tempg->y2=yy1-y;
		MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
		break;
	    }
	    doc_e->flags1=old_e_flags1;

	    QueIns(tempg,insert_pt->last);
	    DCDel(img);
	    GrEdUpdate(doc,tempb,root);
	    dc->pen_width=old_width;
	    eletype=GRST_MENU;
	    if (i==GE_EXIT) {
	      result=GE_EXIT;
	      goto ei_done;
	    }
	    break;
	  case GRST_SHIFTABLE_MESH:
	    GetMsg(NULL,NULL,1<<MSG_IP_L_UP);
ei_shiftable_mesh:
	    doc_e->flags1|=DOCEF1_DONT_DRAW;

	    tempg1=insert_pt;
	    if (tempg1!=root && tempg1->type==GRST_MESH) {
	      z=0;
	      x1=-(p1-x);
	      y1=-(p2-y);
	      z1=z;
	      p=&tempg1->x2;
	      for (i=0;i<tempg1->vertex_cnt;i++,p++) {
		p->x+=x1;
		p->y+=y1;
		p->z+=z1;
	      }
	      old_mesh=&tempg1->vertex_cnt;
	    } else if (tempg1!=root && tempg1->type==GRST_SHIFTABLE_MESH) {
	      z=-tempg1->z1;
	      x1=tempg1->x1-(p1-x);
	      y1=tempg1->y1-(p2-y);
	      z1=tempg1->z1+z;
	      p=(&tempg1->shiftable_tri_cnt)(U8 *)+sizeof(I32);
	      for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
		p->x+=x1;
		p->y+=y1;
		p->z+=z1;
	      }
	      old_mesh=&tempg1->shiftable_vertex_cnt;
	    } else {
	      z=0;
	      old_mesh=NULL;
	    }
	    if (mesh=SpriteMesh(old_mesh,&size,TRUE)) {
	      tempg=MAlloc(SpriteQuedBaseSize(GRST_SHIFTABLE_MESH)-sizeof(I32)*2+size);
	      tempg->type=GRST_SHIFTABLE_MESH;
	      MemCpy(&tempg->shiftable_vertex_cnt,mesh,size);
	      Free(mesh);
	      tempg->x1=p1-x;
	      tempg->y1=p2-y;
	      tempg->z1=-z;
	      QueIns(tempg,insert_pt->last);
	      GrEdUpdate(doc,tempb,root);
	      if (old_mesh) {
		insert_pt=tempg;
		QueRem(tempg1);
		Free(tempg1);
		GrEdUpdate(doc,tempb,root);
	      } else
		cur_num++;
	    } else if (old_mesh && tempg1->type==GRST_SHIFTABLE_MESH) {
	      x1=tempg1->x1-(p1-x);
	      y1=tempg1->y1-(p2-y);
	      z1=tempg1->z1+z;
	      p=(&tempg1->shiftable_tri_cnt)(U8 *)+sizeof(I32);
	      for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
		p->x-=x1;
		p->y-=y1;
		p->z-=z1;
	      }
	    } else if (old_mesh && tempg1->type==GRST_MESH) {
	      x1=-(p1-x);
	      y1=-(p2-y);
	      z1= z;
	      p=&tempg1->x2;
	      for (i=0;i<tempg1->vertex_cnt;i++,p++) {
		p->x-=x1;
		p->y-=y1;
		p->z-=z1;
	      }
	    }
	    doc_e->flags1=old_e_flags1;
	    eletype=GRST_MENU;
	    break;
	}
      }
    }
  }
ei_done:
  DCFill;
  SettingsPop;
  doc_e->flags1=old_e_flags1;
  DCDel(dc);
  DCDel(dc2);
  StrCpy(Fs->task_title,old_task_title);
  Free(old_task_title);
  Fs->border_src=old_border_src;
  Fs->title_src =old_title_src;
  QueDel(root);
  Free(root);
  return result;
}

U0 EdInsGraphic(CDoc *doc,I64 x,I64 y)
{
  Bool unlock;
  U8 *st;
  CDocEntry *doc_e;
  CDocBin *tempb;
  if (Fs!=doc->mem_task)
    Dbg;
  if (st=EdSprite(doc->cur_bin_num)) {
    unlock=DocLock(doc);
    tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
    tempb->size=sprite_base_size[GRST_END];
    tempb->data=CAlloc(tempb->size,doc->mem_task);
    tempb->num=doc->cur_bin_num++;
    tempb->use_cnt=1;
    QueIns(tempb,doc->bin_root.last);
    doc_e=DocPrint(doc,"%s",st);
    doc_e->bin_data=tempb;
    Free(st);
    if (doc_e && doc_e->flags1&DOCEF1_TAG && doc_e->tag && *doc_e->tag)
      tempb->tag=StrNew(doc_e->tag,doc->mem_task);
    DocUnlock(doc);
    x+=StrLen(doc_e->tag)*FONT_WIDTH;

    if (SpriteMain(doc,doc_e,x,y)==GE_ABORT) {
      DocLock(doc);
      QueRem(doc_e);
      Free(doc_e);
      Free(doc_e->tag);
      DocBinDel(doc,tempb);
      if (unlock)
	DocUnlock(doc);
    }
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)))
    DocBinsValidate(doc);
}

U0 EdExistingGraphic(CDoc *doc,I64 x,I64 y)
{
  CDocEntry *doc_e;
  CDocBin *tempb;
  I64 old_size;
  CSprite *old_csprite;
  Bool unlock;

  unlock=DocLock(doc);
  doc_e=doc->cur_entry;
  tempb=doc_e->bin_data;
  old_size=tempb->size;
  old_csprite=tempb->data;
  tempb->data=MAllocIdentical(old_csprite);
  DocUnlock(doc);

  if (SpriteMain(doc,doc_e,x,y)==GE_ABORT) {
    DocLock(doc);
    Free(tempb->data);
    tempb->data=old_csprite;
    tempb->size=old_size;
    if (unlock)
      DocUnlock(doc);
  } else
    Free(old_csprite);
}
