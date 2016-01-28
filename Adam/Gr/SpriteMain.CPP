#help_index "Graphics/Sprite;Sprites"

#define SPT_MENU	-2
#define SPT_INS_SCREEN_BITMAP			-3
#define SPT_INS_TRANSPARENT_SCREEN_BITMAP	-4
#define SPT_ED_MENU	-5
#define SPT_EXIT	-6

I64 PopUpSpriteMain(CSprite **_root,I64 *_cur_elem_num)
{
  U8 *st;
  CTask *pu_task;
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite Main Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteMain.TXT.Z\"$$\n"
	"\n$$LTBLUE$$$$MU-UL,\"Color (4-bit)\",LE=SPT_COLOR$$\n"
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

Bool PopUpExtents(I64 *_x1,I64 *_x2,I64 *_y1,I64 *_y2)
{
  I64 i;
  CDoc *doc=DocNew;
  CDocEntry *doc_e;
  doc_e=DocPrint(doc,"  $$DA,32,A=\"x1:%%d\"$$\n");
  doc_e->data=_x1;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,A=\"x2:%%d\"$$\n");
  doc_e->data=_x2;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,A=\"y1:%%d\"$$\n");
  doc_e->data=_y1;
  DocDataFmt(doc,doc_e);
  doc_e=DocPrint(doc,"  $$DA,32,A=\"y2:%%d\"$$\n\n");
  doc_e->data=_y2;
  DocDataFmt(doc,doc_e);

  DocPrint(doc," $$BT,\"Use These Extents\",LE=TRUE$$");
  DocPrint(doc,"$$CM,3,0$$$$BT,\"Drag-Out New Extents\",LE=FALSE$$\n\n");
  do i=PopUpMenu(doc);
  while (i!=FALSE && i!=TRUE);
  DocDel(doc);
  return i;
}

U0 SpriteSetSettings(CDC *dc,CSprite *root,
	I64 cur_elem_num,CColorROPU32 *_color,I64 *_width,I64 x,I64 y)
{
  CSprite *tempg;
  *_color=BLACK;
  *_width=1;
  DCRst(dc);

  tempg=root->next;
  while (cur_elem_num>0 && tempg!=root) {
    switch (tempg->type) {
      case SPT_COLOR:
	dc->color=*_color=tempg->c.color;
	break;
      case SPT_DITHER_COLOR:
	dc->color=*_color=
	      tempg->d.dither_color.u8[0]|
	      tempg->d.dither_color.u8[1]<<COLORROP_BITS|ROPF_DITHER;
	break;
      case SPT_WIDTH:
	dc->pen_width=*_width=tempg->w.width;
	break;
      case SPT_PLANAR_SYMMETRY:
	if (DCSymmetry3Set(dc,tempg->pp.x1+x,tempg->pp.y1+y,0,
	      tempg->pp.x2+x,tempg->pp.y2+y,0,tempg->pp.x2+x,tempg->pp.y2+y,1))
	  dc->flags|=DCF_SYMMETRY;
	else
	  dc->flags&=~DCF_SYMMETRY;
	break;
    }
    cur_elem_num--;
    tempg=tempg->next;
  }
}

U0 SpriteScreenInit(CDC *dc,I64,I64)
{
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
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

I64 SpriteMainEd(CDoc *doc,CDocEntry *doc_e,I64 x,I64 y)
{
  I64 result,i,msg_code,z,x1,y1,z1,x2,y2,x3,y3,a1,a2,xx1,yy1,xx2,yy2,
	num,old_width,size,eletype=SPT_MENU,width,cur_elem_num,
	old_border_src=Fs->border_src,
	old_title_src =Fs->title_src,
	old_de_flags=doc_e->de_flags,old_flags;
  CColorROPU32 bm_bkcolor,color;
  CSprite *root,root2,*tempg,*tempg1,*tempg2,*insert_pt;
  CDC *dc=DCAlias(,Fs),
	*dc2=DCAlias(,sys_winmgr_task),
	*dc3,
	*img;
  CD3I32 *p;
  CDocBin *tempb=doc_e->bin_data;
  F64 angle1,angle2;
  I32 *ptr,*mesh,*old_mesh;
  U8 *st,*old_task_title=StrNew(Fs->task_title);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinMgrSync(2,TRUE);
  dc2->flags|=DCF_ON_TOP;
  root=Sprite2SpriteQue(tempb->data);
  cur_elem_num=QueCnt(root);
  insert_pt=SpriteElemNum2QueEntry(root,cur_elem_num,&x1,&y1);
  x+=x1; y+=y1;

  while (TRUE) {
    SpriteSetSettings(dc,root,cur_elem_num,&color,&width,x,y);
    DCFill;
    if (eletype==SPT_MENU) {
      Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	    -WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL;
      if (winmgr.fps<10)
	doc_e->de_flags|=DOCEF_DONT_DRAW;
      StrCpy(Fs->task_title,old_task_title);
      Fs->border_src=old_border_src;
      Fs->title_src =old_title_src;

      eletype=PopUpSpriteMain(&root,&cur_elem_num);
      SpriteEdUpdate(doc,tempb,root);
      insert_pt=SpriteElemNum2QueEntry(root,cur_elem_num);
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
      doc_e->de_flags=old_de_flags;
    }

    Fs->title_src=TTS_CONST; //Violates TTS_LOCKED_CONST
    if (eletype<0)
      switch (eletype) {
	case SPT_INS_SCREEN_BITMAP:
	  StrCpy(Fs->task_title,"Insert Screen BitMap");
	  break;
	case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
	  StrCpy(Fs->task_title,"Insert Transparent Screen BitMap");
	  break;
      }
    else
      StrCpy(Fs->task_title,DefineSub(eletype,"ST_SPRITE_ELEM_TYPES"));
    Fs->border_src=BDS_CONST;

    switch (eletype) {
      case SPT_COLOR:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor(,,FALSE);
	if (i>=0) {
	  color=i;
	  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	  tempg->type=SPT_COLOR;
	  tempg->c.color=color;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	doc_e->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_DITHER_COLOR:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColorDither;
	if (i>=0) {
	  color=i;
	  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_DITHER_COLOR));
	  tempg->type=SPT_DITHER_COLOR;
	  tempg->d.dither_color=color.c0.color|color.c1.color<<8;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	doc_e->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_ED_MENU:
	switch (SpriteEd(doc,doc_e,x,y,&root,&cur_elem_num)) {
	  case GE_ABORT:
	    eletype=DOCM_CANCEL;
	    break;
	  case GE_EXIT:
	    eletype=SPT_EXIT;
	    break;
	  case GE_CONT:
	    eletype=SPT_MENU;
	    break;
	}
	break;
      case SPT_MESH:
	doc_e->de_flags|=DOCEF_DONT_DRAW;

	tempg1=insert_pt;
	if (tempg1!=root && tempg1->type==SPT_MESH)
	  old_mesh=&tempg1->mu.vertex_cnt;
	else if (tempg1!=root && tempg1->type==SPT_SHIFTABLE_MESH) {
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
	  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_MESH)-sizeof(I32)*2+size);
	  tempg->type=SPT_MESH;
	  MemCpy(&tempg->mu.vertex_cnt,mesh,size);
	  Free(mesh);
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  if (old_mesh) {
	    insert_pt=tempg;
	    QueRem(tempg1);
	    Free(tempg1);
	    SpriteEdUpdate(doc,tempb,root);
	  } else
	    cur_elem_num++;
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
	doc_e->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_SHIFTABLE_MESH:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	if (PopUpNoYes("Use same origin?\n")) {
	  doc_e->de_flags=old_de_flags;
	  a1=x; a2=y;
	  goto ei_shiftable_mesh;
	}
	PopUpOk("Select Origin.\n");
	doc_e->de_flags=old_de_flags;
	break;
      case SPT_INS_SCREEN_BITMAP:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	PopUpOk("Drag-out a rect for the extents of the\n"
	      "bitmap.\n");
	doc_e->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_IP_L|WIF_FOCUS_TASK_IP_R|
	      WIF_FOCUS_TASK_BORDER-WIF_SELF_FOCUS-WIF_SELF_GRAB_SCROLL;
	break;
      case SPT_BITMAP:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor("Background Color\n\n",,FALSE);
	if (i<0)
	  eletype=SPT_MENU;
	else {
	  bm_bkcolor=i;
	  SpriteEdUpdate(doc,tempb,root);
	  SpriteExtents(tempb->data,&xx1,&xx2,&yy1,&yy2);
	  if (!(xx1<=xx2 && yy1<=yy2))
	    xx1=xx2=yy1=yy2=0;
	  if (PopUpExtents(&xx1,&xx2,&yy1,&yy2)) {
	    doc_e->de_flags=old_de_flags;
	    old_width=dc->pen_width;
	    xx1+=x; yy1+=y;
	    xx2+=x; yy2+=y;
	    goto ei_bitmap;
	  }
	}
	doc_e->de_flags=old_de_flags;
	break;
      case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpColor("Color to Become Transparent\n\n",,FALSE);
	if (i<0)
	  eletype=SPT_MENU;
	else {
	  bm_bkcolor=i;
	  PopUpOk("Drag-out a rect for the extents of the\n"
		"bitmap.\n");
	}
	doc_e->de_flags=old_de_flags;
	Fs->win_inhibit=WIG_TASK_DFT|WIF_FOCUS_TASK_IP_L|WIF_FOCUS_TASK_IP_R|
	      WIF_FOCUS_TASK_BORDER-WIF_SELF_FOCUS-WIF_SELF_GRAB_SCROLL;
	break;
      case SPT_WIDTH:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpRangeI64(1,16,1,"Pen Width\n");
	if (i>=1) {
	  width=i;
	  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_WIDTH));
	  tempg->type=SPT_WIDTH;
	  tempg->w.width=width;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	doc_e->de_flags=old_de_flags;
	eletype=SPT_MENU;
	break;
      case SPT_TRANSFORM_ON:
      case SPT_TRANSFORM_OFF:
	tempg=MAlloc(SpriteElemQuedBaseSize(SPT_TRANSFORM_ON));
	if (eletype==SPT_TRANSFORM_ON)
	  tempg->type=SPT_TRANSFORM_ON;
	else
	  tempg->type=SPT_TRANSFORM_OFF;
	QueIns(tempg,insert_pt->last);
	SpriteEdUpdate(doc,tempb,root);
	cur_elem_num++;
	eletype=SPT_MENU;
	break;
      case SPT_POLYGON:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	i=PopUpRangeI64(3,16,1,"Num of Sides\n");
	doc_e->de_flags=old_de_flags;
	if (i<3)
	  eletype=SPT_MENU;
	break;
      case SPT_TEXT:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	st=PopUpGetStr("Enter text and press <ESC>.\n");
	doc_e->de_flags=old_de_flags;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrPrint3(dc,x1,y1,0,"%s",st);
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    SpriteScreenInit(dc,x,y);
	    x1=a1; y1=a2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_TEXT)+StrLen(st)+1);
	  tempg->type=SPT_TEXT;
	  tempg->ps.x1=x1-x;
	  tempg->ps.y1=y1-y;
	  StrCpy(tempg->ps.st,st);
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	Free(st);
	eletype=SPT_MENU;
	break;
      case SPT_TEXT_BOX:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	st=PopUpGetStr("Enter text and press <ESC>.\n");
	doc_e->de_flags=old_de_flags;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrTextBox3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    SpriteScreenInit(dc,x,y);
	    x1=a1; y1=a2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_TEXT_BOX)+StrLen(st)+1);
	  tempg->type=SPT_TEXT_BOX;
	  tempg->ps.x1=x1-x;
	  tempg->ps.y1=y1-y;
	  StrCpy(tempg->ps.st,st);
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	Free(st);
	eletype=SPT_MENU;
	break;
      case SPT_TEXT_DIAMOND:
	doc_e->de_flags|=DOCEF_DONT_DRAW;
	st=PopUpGetStr("Enter text and press <ESC>.\n");
	doc_e->de_flags=old_de_flags;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK;
	    GrTextDiamond3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&a1,&a2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    SpriteScreenInit(dc,x,y);
	    x1=a1; y1=a2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_TEXT_DIAMOND)+StrLen(st)+1);
	  tempg->type=SPT_TEXT_DIAMOND;
	  tempg->ps.x1=x1-x;
	  tempg->ps.y1=y1-y;
	  StrCpy(tempg->ps.st,st);
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	}
	Free(st);
	eletype=SPT_MENU;
	break;
    }

    if (eletype!=SPT_MENU) {
ei_restart:
      SpriteScreenInit(dc,x,y);
      if (eletype==SPT_EXIT) {
	result=GE_EXIT;
	goto ei_done;
      } else if (eletype==DOCM_CANCEL) {
	result=GE_ABORT;
	goto ei_done;
      }
      msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_R_UP);
      if (msg_code==MSG_IP_R_UP) {
	if (eletype==SPT_PLANAR_SYMMETRY) {
	  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
	  tempg->type=SPT_PLANAR_SYMMETRY;
	  QueIns(tempg,insert_pt->last);
	  SpriteEdUpdate(doc,tempb,root);
	  cur_elem_num++;
	  eletype=SPT_MENU;
	} else
	  eletype=SPT_MENU;
      }
      if (msg_code==MSG_KEY_DOWN) {
	switch (a1) {
	  case CH_ESC:
	    result=GE_EXIT;
	    goto ei_done;
	  case CH_SHIFT_ESC:
	    result=GE_ABORT;
	    goto ei_done;
	  case 'c': //eye-dropper
	    dc3=DCScreenCapture(FALSE);
	    color=GrPeek(dc3,ip.pos.x,ip.pos.y)^15;//Mouse cursor is XORed.
	    DCDel(dc3);
	    tempg=MAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	    tempg->type=SPT_COLOR;
	    tempg->c.color=color;
	    QueIns(tempg,insert_pt->last);
	    SpriteEdUpdate(doc,tempb,root);
	    cur_elem_num++;
	    break;
	  case 't': //Set to transparent color
	    tempg=MAlloc(SpriteElemQuedBaseSize(SPT_COLOR));
	    tempg->type=SPT_COLOR;
	    tempg->c.color=TRANSPARENT;
	    QueIns(tempg,insert_pt->last);
	    SpriteEdUpdate(doc,tempb,root);
	    cur_elem_num++;
	    break;
	}
	goto ei_restart;
      } else {
	x1=a1; y1=a2;
	x2=a1; y2=a2;
	switch (eletype) {
	  sub_switch_start:
	    sub_switch_start:
	      case SPT_LINE:
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&a1,&a2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  SpriteScreenInit(dc,x,y);
		  x2=a1; y2=a2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(SpriteElemQuedBaseSize(SPT_LINE));
		tempg->type=SPT_LINE;
		break;
	      case SPT_ARROW:
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  GrArrow3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&a1,&a2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  SpriteScreenInit(dc,x,y);
		  x2=a1; y2=a2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrArrow3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(SpriteElemQuedBaseSize(SPT_ARROW));
		tempg->type=SPT_ARROW;
		break;
	      case SPT_PLANAR_SYMMETRY:
		old_width=dc->pen_width;
		old_flags=dc->flags;
		dc->flags&=~DCF_SYMMETRY;
		dc->pen_width=1;
		do {
		  dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		  GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&a1,&a2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		  SpriteScreenInit(dc,x,y);
		  x2=a1; y2=a2;
		} while (msg_code!=MSG_IP_L_UP);
		dc->flags=old_flags&DCF_SYMMETRY|dc->flags&~DCF_SYMMETRY;
		dc->pen_width=old_width;
		eletype=SPT_MENU;
		tempg=MAlloc(SpriteElemQuedBaseSize(SPT_PLANAR_SYMMETRY));
		tempg->type=SPT_PLANAR_SYMMETRY;
		break;
	    sub_switch_end:
	      tempg->pp.x1=x1-x;
	      tempg->pp.y1=y1-y;
	      tempg->pp.x2=x2-x;
	      tempg->pp.y2=y2-y;
	      break;
	    case SPT_RECT:
	      xx1=a1; yy1=a2;
	      xx2=a1; yy2=a2;
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
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
	      tempg=MAlloc(SpriteElemQuedBaseSize(SPT_RECT));
	      tempg->type=SPT_RECT;
	      tempg->pp.x1=xx1-x;
	      tempg->pp.y1=yy1-y;
	      tempg->pp.x2=xx2-x;
	      tempg->pp.y2=yy2-y;
	      break;
	    case SPT_INS_SCREEN_BITMAP:
	    case SPT_INS_TRANSPARENT_SCREEN_BITMAP:
	      xx1=a1; yy1=a2;
	      xx2=a1; yy2=a2;
	      old_width=dc2->pen_width;
	      dc2->pen_width=1;
	      do {
		dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
		GrBorder(dc2,xx1+Fs->pix_left+Fs->scroll_x,
		      yy1+Fs->pix_top+Fs->scroll_y,
		      xx2+Fs->pix_left+Fs->scroll_x,
		      yy2+Fs->pix_top+Fs->scroll_y);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
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
	      tempg=CAlloc(SpriteElemQuedBaseSize(SPT_BITMAP)+
		    ((xx2-xx1+7)&~7)*(yy2-yy1));
	      tempg->type=SPT_BITMAP;
	      tempg->pwhu.width=xx2-xx1;
	      tempg->pwhu.height=yy2-yy1;
	      tempg->pwhu.x1=0;
	      tempg->pwhu.y1=0;
	      SpriteScreenInit(dc,x,y);
	      i=gr.screen_zoom;
	      GrScaleZoom(1.0/i);
	      WinMgrSync(2,TRUE);

	      dc3=DCScreenCapture;
	      img=DCExtract(dc3,
		    Fs->pix_left+Fs->scroll_x+xx1,
		    Fs->pix_top+Fs->scroll_y+yy1,
		    Fs->pix_left+Fs->scroll_x+xx2-1,
		    Fs->pix_top+Fs->scroll_y+yy2-1);
	      if (eletype==SPT_INS_TRANSPARENT_SCREEN_BITMAP)
		DCColorChg(img,bm_bkcolor);
	      GrScaleZoom(i);
	      MemCpy(&tempg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
	      DCDel(img);
	      DCDel(dc3);
	      dc2->pen_width=old_width;
	      eletype=SPT_MENU;
	      Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
		    -WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL;
	      break;
	    case SPT_CIRCLE:
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		SpriteScreenInit(dc,x,y);
		x2=a1; y2=a2;
	      } while (msg_code!=MSG_IP_L_UP);
	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
	      tempg=MAlloc(SpriteElemQuedBaseSize(SPT_CIRCLE));
	      tempg->type=SPT_CIRCLE;
	      tempg->pr.x1=x1-x;
	      tempg->pr.y1=y1-y;
	      tempg->pr.radius=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
	      break;
	    case SPT_ELLIPSE:
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrEllipse3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		      AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		SpriteScreenInit(dc,x,y);
		x2=a1; y2=a2;
	      } while (msg_code!=MSG_IP_L_UP);
	      tempg=MAlloc(SpriteElemQuedBaseSize(SPT_ELLIPSE));
	      tempg->type=SPT_ELLIPSE;
	      tempg->pwha.x1=(x1+x2)>>1-x;
	      tempg->pwha.y1=(y1+y2)>>1-y;
	      tempg->pwha.width =AbsI64(x1-x2)>>1;
	      tempg->pwha.height=AbsI64(y1-y2)>>1;
	      angle2=Arg(x2-(tempg->pwha.x1+x),y2-(tempg->pwha.y1+y));
	      if (tempg->pwha.width<tempg->pwha.height)
		angle2-=ã/2.0;
	      do {
		angle1=Arg(x2-(tempg->pwha.x1+x),y2-(tempg->pwha.y1+y));
		if (tempg->pwha.width>=tempg->pwha.height)
		  tempg->pwha.angle=-(angle1-angle2);
		else
		  tempg->pwha.angle=-(angle1-angle2)+ã/2.0;
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrEllipse3(dc,tempg->pwha.x1+x,tempg->pwha.y1+y,0,
		      tempg->pwha.width,tempg->pwha.height,tempg->pwha.angle);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		SpriteScreenInit(dc,x,y);
		x2=a1; y2=a2;
	      } while (msg_code!=MSG_IP_L_UP);

	      angle1=Arg(x2-(tempg->pwha.x1+x),y2-(tempg->pwha.y1+y));
	      if (tempg->pwha.width>=tempg->pwha.height)
		tempg->pwha.angle=-(angle1-angle2);
	      else
		tempg->pwha.angle=-(angle1-angle2)+ã/2.0;

	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrEllipse3(dc,tempg->pwha.x1+x,tempg->pwha.y1+y,0,
		    tempg->pwha.width,tempg->pwha.height,tempg->pwha.angle);
	      break;
	    case SPT_POLYGON:
	      eletype=SPT_MENU;
	      if (i<3) {
		tempg=NULL;
		break;
	      }
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRegPoly3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		      AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1,i);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		SpriteScreenInit(dc,x,y);
		x2=a1; y2=a2;
	      } while (msg_code!=MSG_IP_L_UP);
	      tempg=MAlloc(SpriteElemQuedBaseSize(SPT_POLYGON));
	      tempg->type=SPT_POLYGON;
	      tempg->pwhas.x1=(x1+x2)>>1-x;
	      tempg->pwhas.y1=(y1+y2)>>1-y;
	      tempg->pwhas.width =AbsI64(x1-x2)>>1;
	      tempg->pwhas.height=AbsI64(y1-y2)>>1;
	      tempg->pwhas.sides=i;
	      angle2=Arg(x2-(tempg->pwhas.x1+x),y2-(tempg->pwhas.y1+y));
	      if (tempg->pwhas.width<tempg->pwhas.height)
		angle2-=ã/2.0;
	      do {
		angle1=Arg(x2-(tempg->pwhas.x1+x),y2-(tempg->pwhas.y1+y));
		if (tempg->pwhas.width>=tempg->pwhas.height)
		  tempg->pwhas.angle=-(angle1-angle2);
		else
		  tempg->pwhas.angle=-(angle1-angle2)+ã/2.0;
		dc->color=color&COLORROP_NO_ROP0_MASK;
		GrRegPoly3(dc,tempg->pwhas.x1+x,tempg->pwhas.y1+y,0,
		      tempg->pwhas.width,tempg->pwhas.height,
		      tempg->pwhas.sides,tempg->pwhas.angle);
		msg_code=GetMsg(&a1,&a2,
		      1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		SpriteScreenInit(dc,x,y);
		x2=a1; y2=a2;
	      } while (msg_code!=MSG_IP_L_UP);

	      angle1=Arg(x2-(tempg->pwhas.x1+x),y2-(tempg->pwhas.y1+y));
	      if (tempg->pwhas.width>=tempg->pwhas.height)
		tempg->pwhas.angle=-(angle1-angle2);
	      else
		tempg->pwhas.angle=-(angle1-angle2)+ã/2.0;

	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      GrRegPoly3(dc,tempg->pwhas.x1+x,tempg->pwhas.y1+y,0,
		    tempg->pwhas.width,tempg->pwhas.height,tempg->pwhas.sides,
		    tempg->pwhas.angle);
	      break;
	    case SPT_PT:
	    case SPT_FLOOD_FILL:
	    case SPT_FLOOD_FILL_NOT:
	      tempg=MAlloc(SpriteElemQuedBaseSize(eletype));
	      tempg->type=eletype;
	      tempg->p.x1=x1-x;
	      tempg->p.y1=y1-y;
	      break;
	    sub_switch_start:
	      QueInit(&root2);
	      num=0;
	      do {
		do {
		  dc->color=color&COLORROP_NO_ROP0_MASK;
		  if (num)
		    GrLine3(dc,x1,y1,0,x2,y2,0);
		  msg_code=GetMsg(&a1,&a2,
			1<<MSG_IP_L_UP+1<<MSG_IP_MOVE+
			1<<MSG_IP_R_UP);
		  dc->color=TRANSPARENT;
		  if (num)
		    GrLine3(dc,x1,y1,0,x2,y2,0);
		  x2=a1; y2=a2;
		} while (msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
		dc->color=color&COLORROP_NO_ROP0_MASK;
		if (msg_code==MSG_IP_L_UP) {
		  if (num)
		    GrLine3(dc,x1,y1,0,x2,y2,0);
		  tempg=MAlloc(SpriteElemQuedBaseSize(SPT_PT));
		  tempg->type=SPT_PT;
		  tempg->p.x1=x2-x;
		  tempg->p.y1=y2-y;
		  QueIns(tempg,root2.last);
		  x1=x2;y1=y2;
		  num++;
		}
	      } while (msg_code!=MSG_IP_R_UP);
	      case SPT_POLYLINE:
		if (num>1) {
		  tempg=CAlloc(SpriteElemQuedBaseSize(SPT_POLYLINE)+
			num*sizeof(CD2I32));
		  ptr=&tempg->nu.u;
		  tempg2=root2.next;
		  for (i=0;i<num;i++) {
		    tempg1=tempg2->next;
		    ptr[i<<1]=tempg2->p.x1;
		    ptr[i<<1+1]=tempg2->p.y1;
		    Free(tempg2);
		    tempg2=tempg1;
		  }
		  tempg->type=SPT_POLYLINE;
		  tempg->nu.num=num;
		} else {
		  tempg2=root2.next;
		  for (i=0;i<num;i++) {
		    tempg1=tempg2->next;
		    Free(tempg2);
		    tempg2=tempg1;
		  }
		  tempg=NULL;
		}
		break;
	      case SPT_BSPLINE2:
	      case SPT_BSPLINE3:
	      case SPT_BSPLINE2_CLOSED:
	      case SPT_BSPLINE3_CLOSED:
		if (num>2) {
		  tempg=CAlloc(SpriteElemQuedBaseSize(eletype)+
			num*sizeof(CD3I32));
		  p=&tempg->nu.u;
		  tempg2=root2.next;
		  for (i=0;i<num;i++) {
		    tempg1=tempg2->next;
		    p[i].x=tempg2->p.x1;
		    p[i].y=tempg2->p.y1;
		    Free(tempg2);
		    tempg2=tempg1;
		  }
		  tempg->type=eletype;
		  tempg->nu.num=num;
		} else {
		  tempg2=root2.next;
		  for (i=0;i<num;i++) {
		    tempg1=tempg2->next;
		    Free(tempg2);
		    tempg2=tempg1;
		  }
		  tempg=NULL;
		}
		break;
	    sub_switch_end:
	      break;
	    case SPT_POLYPT:
	      QueInit(&root2);
	      x3=a1-x; y3=a2-y;
	      dc->color=color&COLORROP_NO_ROP0_MASK;
	      do {
		msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		x2=a1; y2=a2;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		Line(&root2,x1-x,y1-y,0,x2-x,y2-y,0,&SpritePolyPtPlot);
		x1=x2;y1=y2;
	      } while (msg_code!=MSG_IP_L_UP);

	      num=0;
	      tempg=root2.next;
	      x1=x3;y1=y3;
	      while (tempg!=&root2) {
		tempg1=tempg->next;
		if (tempg->p.x1==x1 && tempg->p.y1==y1) {
		  QueRem(tempg);
		  Free(tempg);
		} else {
		  num++;
		  x1=tempg->p.x1;
		  y1=tempg->p.y1;
		}
		tempg=tempg1;
	      }

	      tempg=CAlloc(SpriteElemQuedBaseSize(SPT_POLYPT)+(num*3+7)>>3);
	      tempg->npu.x=x3;
	      tempg->npu.y=y3;
	      ptr=&tempg->npu.u;
	      x1=x3;y1=y3;
	      i=0;
	      tempg2=root2.next;
	      while (tempg2!=&root2) {
		tempg1=tempg2->next;
		BFieldOrU32(ptr,i,
		      polypt_map[SignI64(tempg2->p.x1-x1)+1+
		      3*(SignI64(tempg2->p.y1-y1)+1)]);
		i+=3;
		x1=tempg2->p.x1;y1=tempg2->p.y1;
		QueRem(tempg2);
		Free(tempg2);
		tempg2=tempg1;
	      }
	      tempg->type=SPT_POLYPT;
	      tempg->npu.num=num;
	      break;
	  sub_switch_end:
	    if (tempg) {
	      QueIns(tempg,insert_pt->last);
	      SpriteEdUpdate(doc,tempb,root);
	      cur_elem_num++;
	    }
	    break;
	  case SPT_BITMAP:
	    xx1=a1; yy1=a2;
	    xx2=a1; yy2=a2;
	    old_width=dc->pen_width;
	    dc->pen_width=1;
	    do {
	      dc->color=ROPF_DITHER+WHITE<<16+BLACK;
	      GrBorder(dc,xx1,yy1,xx2,yy2);
	      msg_code=GetMsg(&a1,&a2,
		    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
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
ei_bitmap:
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
	    if (tempg1==root || tempg1->type!=SPT_BITMAP) {
	      SpriteSetSettings(img,root,0,&color,&width,-(xx1-x),-(yy1-y));
	      Sprite3(img,-(xx1-x),-(yy1-y),0,tempb->data);
	      QueDel(root);
	      insert_pt=root->next=root->last=root;
	      cur_elem_num=1;
	    } else {
	      SpriteSetSettings(img,root,cur_elem_num,&color,&width,
		    -(xx1-x),-(yy1-y));
	      Sprite3(img,-(xx1-x),-(yy1-y),0,&tempg1->start,TRUE);
	      insert_pt=tempg1->next;
	      QueRem(tempg1);
	      Free(tempg1);
	    }
	    MemCpy(&tempg->pwhu.u,img->body,((xx2-xx1+7)&~7)*(yy2-yy1));

	    switch (i=SpriteBitMapEd(doc,doc_e,dc,&xx1,&yy1,
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
	    doc_e->de_flags=old_de_flags;

	    QueIns(tempg,insert_pt->last);
	    DCDel(img);
	    SpriteEdUpdate(doc,tempb,root);
	    dc->pen_width=old_width;
	    eletype=SPT_MENU;
	    if (i==GE_EXIT) {
	      result=GE_EXIT;
	      goto ei_done;
	    }
	    break;
	  case SPT_SHIFTABLE_MESH:
	    GetMsg(NULL,NULL,1<<MSG_IP_L_UP);
ei_shiftable_mesh:
	    doc_e->de_flags|=DOCEF_DONT_DRAW;

	    tempg1=insert_pt;
	    if (tempg1!=root && tempg1->type==SPT_MESH) {
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
	    } else if (tempg1!=root && tempg1->type==SPT_SHIFTABLE_MESH) {
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
	      tempg=MAlloc(SpriteElemQuedBaseSize(SPT_SHIFTABLE_MESH)
	      -sizeof(I32)*2+size);
	      tempg->type=SPT_SHIFTABLE_MESH;
	      MemCpy(&tempg->pmu.vertex_cnt,mesh,size);
	      Free(mesh);
	      tempg->pmu.x=a1-x;
	      tempg->pmu.y=a2-y;
	      tempg->pmu.z=-z;
	      QueIns(tempg,insert_pt->last);
	      SpriteEdUpdate(doc,tempb,root);
	      if (old_mesh) {
		insert_pt=tempg;
		QueRem(tempg1);
		Free(tempg1);
		SpriteEdUpdate(doc,tempb,root);
	      } else
		cur_elem_num++;
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
	    doc_e->de_flags=old_de_flags;
	    eletype=SPT_MENU;
	    break;
	}
      }
    }
  }
ei_done:
  DCFill;
  SettingsPop;
  doc_e->de_flags=old_de_flags;
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

U0 EdSpriteIns(CDoc *doc,I64 x,I64 y)
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
    QueIns(tempb,doc->bin_root.last);
    doc_e=DocPrint(doc,"%s",st);
    doc_e->bin_data=tempb;
    Free(st);
    if (doc_e && doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
      tempb->tag=StrNew(doc_e->tag,doc->mem_task);
    DocUnlock(doc);
    x+=StrLen(doc_e->tag)*FONT_WIDTH;

    if (SpriteMainEd(doc,doc_e,x,y)==GE_ABORT) {
      DocLock(doc);
      QueRem(doc_e);
      Free(doc_e);
      Free(doc_e->tag);
      DocBinDel(doc,tempb);
      if (unlock)
	DocUnlock(doc);
    }
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS)))
    DocBinsValidate(doc);
}

U0 EdSpriteEd(CDoc *doc,I64 x,I64 y)
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

  if (SpriteMainEd(doc,doc_e,x,y)==GE_ABORT) {
    DocLock(doc);
    Free(tempb->data);
    tempb->data=old_csprite;
    tempb->size=old_size;
    if (unlock)
      DocUnlock(doc);
  } else
    Free(old_csprite);
}
