#help_index "Graphics/CSprite"
#define CSPRITE_WIF	(WIF_ALL-WIF_BORDER-WIF_GRAB_SCROLL)

#define GRST_MENU	-2
#define GRST_SNAP	-3
#define GRST_INS_SCREEN_BITMAP		-5
#define GRST_INS_TRANSPARENT_SCREEN_BITMAP	-6
#define GRST_ED		-7

I64 PopUpCSpriteVect(Bool new_sprite,CSprite **_root,I64 snap,F64 ip_scaler,I64 *cur_num)
{
  U8 *st;
  CTask *pu_task;
  I64 i;
  CDoc *doc=DocNew;
  DocPrintF(doc,"$$FG,PURPLE$$$$TX+CX,\"Vect Object Menu\"$$$$FG$$\n"
	"$$FG,RED$$Settings$$FG$$\n"
	"$$MU-UL,\"Color (4-bit)\",GRST_COLOR8$$\n"
	"$$MU-UL,\"Dither Color (4-bit)\",GRST_DITHER_COLOR8$$\n"
	"$$MU-UL,\"Width\",GRST_WIDTH$$\n"
	"$$MU-UL,\"Snap %d %5.3f\",GRST_SNAP$$\n"
	"$$MU-UL,\"Planar Symmetry\",GRST_PLANAR_SYMMETRY$$\n"
	"\n$$FG,RED$$Objects$$FG$$\n"
	"$$MU-UL,\"Point\",GRST_PT$$\n"
	"$$MU-UL,\"Line\",GRST_LINE$$\n"
	"$$MU-UL,\"Arrow\",GRST_ARROW$$\n"
	"$$MU-UL,\"Rect\",GRST_RECT$$\n"
	"$$MU-UL,\"Circle\",GRST_CIRCLE$$\n"
	"$$MU-UL,\"Ellipse\",GRST_ELLIPSE$$\n"
	"$$MU-UL,\"Reg Polygon\",GRST_REG_POLY$$\n"
	"$$MU-UL,\"Text\",GRST_TEXT$$\n"
	"$$MU-UL,\"Text Box\",GRST_TEXT_BOX$$\n"
	"$$MU-UL,\"Text Diamond\",GRST_TEXT_DIAMOND$$\n"
	"$$MU-UL,\"Flood Fill\",GRST_FLOOD_FILL$$\n"
	"$$MU-UL,\"Flood Fill Not Color\",GRST_FLOOD_FILL_NOT$$\n"
	"$$MU-UL,\"PolyLine\",GRST_POLYLINE$$\n"
	"$$MU-UL,\"PolyPoint\",GRST_POLYPT$$\n"
	"$$MU-UL,\"BSpline2\",GRST_BSPLINE2$$\n"
	"$$MU-UL,\"BSpline3\",GRST_BSPLINE3$$\n"
	"$$MU-UL,\"BSpline2 Closed\",GRST_BSPLINE2_CLOSED$$\n"
	"$$MU-UL,\"BSpline3 Closed\",GRST_BSPLINE3_CLOSED$$\n"
	"$$MU-UL,\"Insert Screen BitMap\",GRST_INS_SCREEN_BITMAP$$\n"
	"$$MU-UL,\"Insert Transparent Screen BitMap\",GRST_INS_TRANSPARENT_SCREEN_BITMAP$$\n"
	"$$FM,PURPLE$$$$MU-UL,\"+] Create/Edit 3D Mesh\",GRST_MESH$$\n"
	"$$MU-UL,\"+] Create/Edit Shiftable 3D Mesh\",GRST_SHIFTABLE_MESH$$\n"
	"$$MU-UL,\"+] Create/Edit BitMap\",GRST_BITMAP8$$\n"
	"\n$$FG,RED$$Advanced$$FG$$\n"
	"$$MU-UL,\"Transform On (For use with icons)\",GRST_TRANSFORM_ON$$\n"
	"$$MU-UL,\"Transform Off(For use with icons)\",GRST_TRANSFORM_OFF$$\n"
	"\n"
	"$$FM,PURPLE$$$$MU-UL,\"+] Edit Menu\",GRST_ED$$$$FM,LTBLUE$$\n"
	"$$MU-UL,\"End\",GRST_END$$\n",snap,ip_scaler);
  if (new_sprite)
    DocPutS(doc,"$$MU-UL,\"Cancel\",DOCM_CANCEL$$\n");
  DocPutS(doc,"\nRight-Click to get back to this menu.");
  st=MSPrintF("CSpriteSideBarTask(0x%X,0x%X,0x%X);",Fs,_root,cur_num);
  PopUp(st,NULL,&pu_task);
  Free(st);
  i=PopUpMenu(doc);
  if (TaskValidate(pu_task)) {
    *_root=CSpriteDoc2Que(DocCur(pu_task),*_root,cur_num);
    Kill(pu_task);
  }
  DocDel(doc);
  return i;
}

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

CSprite *CSpriteFromQue(CSprite *root,U32 *_size=NULL)
{
  I64 s,size=gr_sprite_base_size[GRST_END];
  CSprite *result,*tempg;
  U8 *dst;

  tempg=root->next;
  while (tempg!=root) {
    size+=CSpriteElemSize(tempg);
    tempg=tempg->next;
  }
  result=MAlloc(size);
  dst=result;
  tempg=root->next;
  while (tempg!=root) {
    s=CSpriteElemSize(tempg);
    MemCpy(dst,&tempg->start,s);
    dst+=s;
    tempg=tempg->next;
  }
  *dst=GRST_END;
  if (_size) *_size=size;
  return result;
}

CSprite *CSprite2Que(U8 *elems)
{
  I64 s;
  CSprite *result=CAlloc(sizeof(CSprite)),
	 *tempg=elems-offset(CSprite.start),
	 *tempg1;
  result->next=result->last=result;
  while (tempg->type) {
    tempg1=MAlloc(CSpriteElemSize(tempg)+offset(CSprite.start));
    s=CSpriteElemSize(tempg);
    MemCpy(&tempg1->start,&tempg->start,s);
    QueIns(tempg1,result->last);
    tempg(U8 *)+=s;
  }
  return result;
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

U0 GrEdUpdate(CDoc *doc,CDocBin *tempb,CSprite *root)
{
  Bool unlock=DocLock(doc);
  Free(tempb->data);
  tempb->data=CSpriteFromQue(root,&tempb->size);
  if (unlock)
    DocUnlock(doc);
}

U0 GrSetSettings(CDC *dc,CSprite *root,I64 cur_num,CColorROPU32 *_color,I64 *_width,I64 x,I64 y)
{
  CSprite *tempg;
  *_color=ROP_EQU|BLACK;
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
//This uses $LK,"fixed-point","FI:::/SparrowOS/Demo/Lectures/FixedPoint.CPP.Z"$.
  I64 xx,yy,old_pen_width=dc->pen_width;
  CColorROPU32 old_color=dc->color;
  WinMgrSync;
  DCFill(dc);
  if (dc->flags&DCF_SYMMETRY) {
    dc->flags&=~DCF_SYMMETRY;
    dc->pen_width=1;
    xx=dc->sym.sny*8192;
    yy=-dc->sym.snx*8192;
    dc->color=ROP_EQU+RED;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
      dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,0);
    dc->color=ROP_EQU+WHITE;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
      dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,1);
    dc->color=ROP_EQU+BLACK;
    GrLine3(dc,dc->sym.sx-xx.i32[1],dc->sym.sy-yy.i32[1],0,
      dc->sym.sx+xx.i32[1],dc->sym.sy+yy.i32[1],0,3,2);
    dc->flags|=DCF_SYMMETRY;
  }
  dc->color=old_color;
  dc->pen_width=old_pen_width;
}

I64 CSpriteVect(CDoc *doc,CDocEntry *doc_e,CSprite **_root,I64 *cur_num,I64 x,I64 y,Bool new_sprite,I64 *_import_snap=NULL,F64 *_import_ip_scaler=NULL)
{
  I64 result,i,msg_code,z,x1,y1,z1,x2,y2,x3,y3,p1,p2,xx1,yy1,xx2,yy2,
      num,old_width,size,eletype=GRST_MENU,width,new_snap=1,
      old_doc_flags=doc->flags&(DOCF_ATTR_BY_PRT|DOCF_ATTR_BY_FILENAME),
      old_ce_flags1=doc_e->flags1,old_flags;
  CColorROPU32 bm_bkcolor,color;
  CSprite root2,*tempg,*tempg1,*tempg2,*insert_pt;
  CDC *dc=DCAlias(gr_dc2,Fs),
      *dc2=DCAlias(gr_dc2,sys_winmgr_task),
      *dc3,
      *img;
  CD3I32 *p;
  CDocBin *tempb=doc_e->bin_data;
  F64 a1,a2,new_ip_scaler=1.0;
  Fs->task_title[TASK_TITLE_LEN]=0;
  I32 *ptr,*mesh,*old_mesh;
  U8 *st,*old_desc=StrNew(Fs->task_title),
      *old_filename=StrNew(doc->filename.name);

  WinMgrSync(2,TRUE);
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  dc2->flags|=DCF_ON_TOP;
  insert_pt=GrNum2Que(*_root,*cur_num,&x1,&y1);
  x+=x1; y+=y1;

  if (_import_snap) new_snap=*_import_snap;
  if (_import_ip_scaler) new_ip_scaler=*_import_ip_scaler;
  SetSnap(&Fs->snap,new_snap,new_ip_scaler);
  Fs->win_inhibit=CSPRITE_WIF;
  while (TRUE) {
    GrSetSettings(dc,*_root,*cur_num,&color,&width,x,y);
    DCFill;
    if (eletype==GRST_MENU) {
      doc_e->flags1|=DOCEF1_DONT_DRAW;
      StrCpy(Fs->task_title,old_desc);
      StrCpy(doc->filename.name,old_filename);
      doc->flags|=old_doc_flags;

      eletype=PopUpCSpriteVect(new_sprite,_root,new_snap,new_ip_scaler,cur_num);
      GrEdUpdate(doc,tempb,*_root);
      insert_pt=GrNum2Que(*_root,*cur_num);
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
      doc_e->flags1=old_ce_flags1;
    }

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
      StrCpy(Fs->task_title,DefineSub(eletype,"ST_CSPRITE_TYPES"));
    StrCpy(doc->filename.name,Fs->task_title);
    doc->flags&=~(DOCF_ATTR_BY_PRT|DOCF_ATTR_BY_FILENAME);

    switch (eletype) {
      case GRST_COLOR8:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColor(,,FALSE);
	if (i>=0) {
	  color=ROP_EQU|i;
	  tempg=MAlloc(gr_sprite_qued_base_size[GRST_COLOR8]);
	  tempg->type=GRST_COLOR8;
	  tempg->color8=color;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	doc_e->flags1=old_ce_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_DITHER_COLOR8:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpDitherColor;
	if (i>=0) {
	  color=ROP_EQU|i;
	  tempg=MAlloc(gr_sprite_qued_base_size[GRST_DITHER_COLOR8]);
	  tempg->type=GRST_DITHER_COLOR8;
	  tempg->dither_color8=color.c0.color|color.c1.color<<8;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	doc_e->flags1=old_ce_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_MESH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;

	tempg1=insert_pt;
	if (tempg1!=*_root && tempg1->type==GRST_MESH)
	  old_mesh=&tempg1->vertex_cnt;
	else if (tempg1!=*_root && tempg1->type==GRST_SHIFTABLE_MESH) {
	  x1=tempg1->x1;
	  y1=tempg1->y1;
	  z1=tempg1->z1;
	  p=(&tempg1->shiftable_triangle_cnt)(U8 *)+sizeof(I32);
	  for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
	    p->x+=x1;
	    p->y+=y1;
	    p->z+=z1;
	  }
	  old_mesh=&tempg1->shiftable_vertex_cnt;
	} else
	  old_mesh=NULL;
	if (mesh=CSpriteMesh(old_mesh,&size,TRUE)) {
	  tempg=MAlloc(gr_sprite_qued_base_size[GRST_MESH]-sizeof(I32)*2+size);
	  tempg->type=GRST_MESH;
	  MemCpy(&tempg->vertex_cnt,mesh,size);
	  Free(mesh);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  if (old_mesh) {
	    insert_pt=tempg;
	    QueRem(tempg1);
	    Free(tempg1);
	    GrEdUpdate(doc,tempb,*_root);
	  } else
	    *cur_num+=1;
	} else if (old_mesh && tempg1->type==GRST_SHIFTABLE_MESH) {
	  x1=tempg1->x1;
	  y1=tempg1->y1;
	  z1=tempg1->z1;
	  p=(&tempg1->shiftable_triangle_cnt)(U8 *)+sizeof(I32);
	  for (i=0;i<tempg1->shiftable_vertex_cnt;i++,p++) {
	    p->x-=x1;
	    p->y-=y1;
	    p->z-=z1;
	  }
	  }
	doc_e->flags1=old_ce_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_SHIFTABLE_MESH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	if (PopUpNoYes("Use same origin?\n")) {
	  doc_e->flags1=old_ce_flags1;
	  p1=x; p2=y;
	  goto ei_shiftable_mesh;
	}
	PopUpOk("Select Origin.\n");
	doc_e->flags1=old_ce_flags1;
	break;
      case GRST_INS_SCREEN_BITMAP:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	PopUpOk("Drag-out a rect for the extents of the\n"
		"bitmap.\n");
	doc_e->flags1=old_ce_flags1;
	break;
      case GRST_BITMAP8:
      case GRST_INS_TRANSPARENT_SCREEN_BITMAP:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpColor("Background Color\n\n",,FALSE);
	if (i<0)
	  eletype=GRST_MENU;
	else {
	  bm_bkcolor=i;
	  if (eletype!=GRST_INS_TRANSPARENT_SCREEN_BITMAP) {
	    GrEdUpdate(doc,tempb,*_root);
	    CSpriteExtents(tempb->data,&xx1,&xx2,&yy1,&yy2);
	    if (xx1<=xx2 && yy1<=yy2 && PopUpNoYes("Use extents of the CSprite?\n")) {
	      doc_e->flags1=old_ce_flags1;
	      old_width=dc->pen_width;
	      xx1+=x; yy1+=y;
	      xx2+=x; yy2+=y;
	      goto ei_bitmap;
	    }
	  }
	  PopUpOk("Drag-out a rect for the extents of the\n"
		"bitmap.\n");
	}
	doc_e->flags1=old_ce_flags1;
	break;
      case GRST_WIDTH:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpRangeI64(1,16,1,"Pen Width\n");
	if (i>=1) {
	  width=i;
	  tempg=MAlloc(gr_sprite_qued_base_size[GRST_WIDTH]);
	  tempg->type=GRST_WIDTH;
	  tempg->width=width;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	doc_e->flags1=old_ce_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_TRANSFORM_ON:
      case GRST_TRANSFORM_OFF:
	tempg=MAlloc(gr_sprite_qued_base_size[GRST_TRANSFORM_ON]);
	if (eletype==GRST_TRANSFORM_ON)
	  tempg->type=GRST_TRANSFORM_ON;
	else
	  tempg->type=GRST_TRANSFORM_OFF;
	QueIns(tempg,insert_pt->last);
	GrEdUpdate(doc,tempb,*_root);
	*cur_num+=1;
	eletype=GRST_MENU;
	break;
      case GRST_REG_POLY:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	i=PopUpRangeI64(3,16,1,"Num of Sides\n");
	doc_e->flags1=old_ce_flags1;
	if (i<3)
	  eletype=GRST_MENU;
	break;
      case GRST_SNAP:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	PopUpSnap(&Fs->snap);
	new_snap=Fs->snap.x;
	new_ip_scaler=Fs->snap.dx;
	doc_e->flags1=old_ce_flags1;
	eletype=GRST_MENU;
	break;
      case GRST_TEXT:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_ce_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrPutS3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&p1,&p2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(gr_sprite_qued_base_size[GRST_TEXT]+StrLen(st)+1);
	  tempg->type=GRST_TEXT;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
      case GRST_TEXT_BOX:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_ce_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrTextBox3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&p1,&p2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(gr_sprite_qued_base_size[GRST_TEXT_BOX]+StrLen(st)+1);
	  tempg->type=GRST_TEXT_BOX;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
      case GRST_TEXT_DIAMOND:
	doc_e->flags1|=DOCEF1_DONT_DRAW;
	st=PopUpMGetS("Enter text and press <ESC>.\n");
	doc_e->flags1=old_ce_flags1;
	if (st && *st) {
	  x1=0; y1=0;
	  do {
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrTextDiamond3(dc,x1,y1,0,st);
	    msg_code=GetMsg(&p1,&p2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	    GrScreenInit(dc,x,y);
	    x1=p1; y1=p2;
	  } while (msg_code!=MSG_IP_L_UP);
	  tempg=CAlloc(gr_sprite_qued_base_size[GRST_TEXT_DIAMOND]+StrLen(st)+1);
	  tempg->type=GRST_TEXT_DIAMOND;
	  tempg->x1=x1-x;
	  tempg->y1=y1-y;
	  StrCpy(tempg->st,st);
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	}
	Free(st);
	eletype=GRST_MENU;
	break;
    }



    if (eletype!=GRST_MENU) {
ei_restart:
      GrScreenInit(dc,x,y);
      if (eletype==GRST_END) {
	result=GE_OKAY_DONE;
	goto ei_done;
      } else if (eletype==GRST_ED) {
	result=GE_OKAY_CONT;
	goto ei_done;
      } else if (eletype==DOCM_CANCEL) {
	result=GE_CANCEL;
	goto ei_done;
      }
      msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN | 1<<MSG_IP_L_DOWN | 1<<MSG_IP_R_UP);
      if (msg_code==MSG_IP_R_UP) {
	if (eletype==GRST_PLANAR_SYMMETRY) {
	  tempg=CAlloc(gr_sprite_qued_base_size[GRST_PLANAR_SYMMETRY]);
	  tempg->type=GRST_PLANAR_SYMMETRY;
	  QueIns(tempg,insert_pt->last);
	  GrEdUpdate(doc,tempb,*_root);
	  *cur_num+=1;
	  eletype=GRST_MENU;
	} else
	  eletype=GRST_MENU;
      }
      if (msg_code==MSG_KEY_DOWN) {
	if (p1==CH_ESC) {
	  result=GE_OKAY_DONE;
	  goto ei_done;
	} else if (p1==CH_SHIFT_ESC) {
	  result=GE_CANCEL;
	  goto ei_done;
	} else
	  goto ei_restart;
      } else {
	switch (eletype) {
	  case GRST_LINE:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      GrLine3(dc,x1,y1,0,x2,y2,0);
	      msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
	    } while (msg_code!=MSG_IP_L_UP);
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrLine3(dc,x1,y1,0,x2,y2,0);
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_LINE]);
	    tempg->type=GRST_LINE;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    tempg->x2=x2-x;
	    tempg->y2=y2-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_ARROW:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      GrArrow3(dc,x1,y1,0,x2,y2,0);
	      msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
	    } while (msg_code!=MSG_IP_L_UP);
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrArrow3(dc,x1,y1,0,x2,y2,0);
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_ARROW]);
	    tempg->type=GRST_ARROW;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    tempg->x2=x2-x;
	    tempg->y2=y2-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_PLANAR_SYMMETRY:
	    old_width=dc->pen_width;
	    old_flags=dc->flags;
	    dc->flags&=~DCF_SYMMETRY;
	    dc->pen_width=1;
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      dc->color=ROPF_DITHER+WHITE<<16+ROP_EQU+BLACK;
	      GrLine3(dc,x1,y1,0,x2,y2,0);
	      msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
	    } while (msg_code!=MSG_IP_L_UP);
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_PLANAR_SYMMETRY]);
	    tempg->type=GRST_PLANAR_SYMMETRY;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    tempg->x2=x2-x;
	    tempg->y2=y2-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    dc->flags=old_flags&DCF_SYMMETRY|dc->flags&~DCF_SYMMETRY;
	    dc->pen_width=old_width;
	    eletype=GRST_MENU;
	    break;
	  case GRST_RECT:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    xx1=p1; yy1=p2;
	    xx2=p1; yy2=p2;
	    do {
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
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
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrRect3(dc,xx1,yy1,0,xx2-xx1,yy2-yy1);
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_RECT]);
	    tempg->type=GRST_RECT;
	    tempg->x1=xx1-x;
	    tempg->y1=yy1-y;
	    tempg->x2=xx2-x;
	    tempg->y2=yy2-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_BITMAP8:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    xx1=p1; yy1=p2;
	    xx2=p1; yy2=p2;
	    old_width=dc->pen_width;
	    dc->pen_width=1;
	    do {
	      dc->color=ROPF_DITHER+WHITE<<16+ROP_EQU+BLACK;
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
	    tempg=CAlloc(gr_sprite_qued_base_size[GRST_BITMAP8]+
		((xx2-xx1+7)&~7)*(yy2-yy1));
	    tempg->type=eletype;
	    tempg->width=xx2-xx1;
	    tempg->height=yy2-yy1;
	    tempg->x2=xx1-x;
	    tempg->y2=yy1-y;
	    img=DCNew(DCT_COLOR8,tempg->width,tempg->height,Fs);
	    img->color=bm_bkcolor;
	    GrRect(img,0,0,tempg->width,tempg->height);
	    tempg1=insert_pt;
	    if (tempg1==*_root || tempg1->type!=GRST_BITMAP8) {
	      GrSetSettings(img,*_root,0,&color,&width,-(xx1-x),-(yy1-y));
	      CSpritePlot3(img,-(xx1-x),-(yy1-y),0,tempb->data);
	      QueDel(*_root);
	      insert_pt=(*_root)->next=(*_root)->last=*_root;
	      *cur_num=1;
	    } else {
	      GrSetSettings(img,*_root,*cur_num,&color,&width,-(xx1-x),-(yy1-y));
	      CSpritePlot3(img,-(xx1-x),-(yy1-y),0,tempg1(U8 *)+offset(CSprite.start),TRUE);
	      insert_pt=tempg1->next;
	      QueRem(tempg1);
	      Free(tempg1);
	    }
	    MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));

	    doc_e->flags1|=DOCEF1_DONT_DRAW;
	    switch (i=CSpriteBitMap(dc,&xx1,&yy1,&xx2,&yy2,&img,bm_bkcolor,&new_snap,&new_ip_scaler)) {
	      case GE_OKAY_DONE:
	      case GE_OKAY_CONT:
		Free(tempg);
		tempg=CAlloc(gr_sprite_qued_base_size[GRST_BITMAP8]+
		    ((xx2-xx1+7)&~7)*(yy2-yy1));
		tempg->type=eletype;
		tempg->width=xx2-xx1;
		tempg->height=yy2-yy1;
		tempg->x2=xx1-x;
		tempg->y2=yy1-y;
		MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
		break;
	    }
	    SetSnap(&Fs->snap,new_snap,new_ip_scaler);
	    doc_e->flags1=old_ce_flags1;

	    QueIns(tempg,insert_pt->last);
	    DCDel(img);
	    GrEdUpdate(doc,tempb,*_root);
	    dc->pen_width=old_width;
	    eletype=GRST_MENU;
	    if (i==GE_OKAY_DONE) {
	      result=GE_OKAY_DONE;
	      goto ei_done;
	    }
	    break;
	  case GRST_INS_SCREEN_BITMAP:
	  case GRST_INS_TRANSPARENT_SCREEN_BITMAP:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    xx1=p1; yy1=p2;
	    xx2=p1; yy2=p2;
	    old_width=dc2->pen_width;
	    dc2->pen_width=1;
	    do {
	      dc2->color=ROPF_DITHER+WHITE<<16+ROP_EQU+BLACK;
	      GrLineRect4(dc2,xx1+Fs->win_pixel_left+Fs->win_scroll_x,yy1+Fs->win_pixel_top+Fs->win_scroll_y,
		  xx2+Fs->win_pixel_left+Fs->win_scroll_x,yy2+Fs->win_pixel_top+Fs->win_scroll_y);
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
	    tempg=CAlloc(gr_sprite_qued_base_size[GRST_BITMAP8]+
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
		Fs->win_pixel_left+Fs->win_scroll_x+xx1,Fs->win_pixel_top+Fs->win_scroll_y+yy1,
		Fs->win_pixel_left+Fs->win_scroll_x+xx2-1,Fs->win_pixel_top+Fs->win_scroll_y+yy2-1);
	    if (eletype==GRST_INS_TRANSPARENT_SCREEN_BITMAP)
	      DCColorChg(img,bm_bkcolor);
	    GrScaleZoom(i);
	    MemCpy(tempg(U8 *)+offset(CSprite.linespeed),img->body,((xx2-xx1+7)&~7)*(yy2-yy1));
	    DCDel(img);
	    DCDel(dc3);
	    dc2->pen_width=old_width;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    eletype=GRST_MENU;
	    break;
	  case GRST_CIRCLE:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
	      msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
	    } while (msg_code!=MSG_IP_L_UP);
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2)));
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_CIRCLE]);
	    tempg->type=GRST_CIRCLE;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    tempg->x2=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_ELLIPSE:
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      GrEllipse3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		  AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1);
	      msg_code=GetMsg(&p1,&p2,
		  1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	      GrScreenInit(dc,x,y);
	      x2=p1; y2=p2;
	    } while (msg_code!=MSG_IP_L_UP);
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_ELLIPSE]);
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
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
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

	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	    GrEllipse3(dc,tempg->x2+x,tempg->y2+y,0,
		tempg->width,tempg->height,tempg->rot_angle);
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_REG_POLY:
	    if (i>=3) {
	      x1=p1; y1=p2;
	      x2=p1; y2=p2;
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
		GrRegPoly3(dc,(x1+x2)>>1,(y1+y2)>>1,0,
		    AbsI64(x1-x2)>>1,AbsI64(y1-y2)>>1,i);
		msg_code=GetMsg(&p1,&p2,
		    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
		GrScreenInit(dc,x,y);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP);
	      tempg=MAlloc(gr_sprite_qued_base_size[GRST_REG_POLY]);
	      tempg->type=GRST_REG_POLY;
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
		dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
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

	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      GrRegPoly3(dc,tempg->x2+x,tempg->y2+y,0,
		  tempg->width,tempg->height,tempg->sides,tempg->rot_angle);
	      QueIns(tempg,insert_pt->last);
	      GrEdUpdate(doc,tempb,*_root);
	      *cur_num+=1;
	    }
	    eletype=GRST_MENU;
	    break;
	  case GRST_PT:
	    x1=p1; y1=p2;
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_PT]);
	    tempg->type=GRST_PT;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_FLOOD_FILL:
	  case GRST_FLOOD_FILL_NOT:
	    x1=p1; y1=p2;
	    tempg=MAlloc(gr_sprite_qued_base_size[GRST_FLOOD_FILL]);
	    tempg->type=eletype;
	    tempg->x1=x1-x;
	    tempg->y1=y1-y;
	    QueIns(tempg,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_POLYLINE:
	    QueInit(&root2);
	    num=0;
	    x3=p1-x; y3=p2-y;
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		msg_code=GetMsg(&p1,&p2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE+
		    1<<MSG_IP_R_UP);
		dc->color=ROP_EQU|COLOR_TRANSPARENT;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      if (msg_code==MSG_IP_L_UP) {
		GrLine3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(gr_sprite_qued_base_size[GRST_PT]);
		tempg->type=GRST_PT;
		tempg->x1=x2-x;
		tempg->y1=y2-y;
		QueIns(tempg,root2.last);
		x1=x2;y1=y2;
		num++;
	      }
	    } while (msg_code!=MSG_IP_R_UP);
	    if (num) {
	      tempg2=CAlloc(gr_sprite_qued_base_size[GRST_POLYLINE]+
		  (num+1)<<1*sizeof(I32));
	      ptr=&tempg2->y1;
	      tempg=root2.next;
	      ptr[0]=x3;
	      ptr[1]=y3;
	      ptr+=2;
	      for (i=0;i<num;i++) {
		tempg1=tempg->next;
		ptr[i<<1]=tempg->x1;
		ptr[i<<1+1]=tempg->y1;
		Free(tempg);
		tempg=tempg1;
	      }
	      tempg2->type=GRST_POLYLINE;
	      tempg2->num=num+1;
	      QueIns(tempg2,insert_pt->last);
	      GrEdUpdate(doc,tempb,*_root);
	      *cur_num+=1;
	    }
	    break;
	  case GRST_POLYPT:
	    QueInit(&root2);
	    x3=p1-x; y3=p2-y;
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
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

	    tempg2=CAlloc(gr_sprite_qued_base_size[GRST_POLYPT]+
		sizeof(I32)*2+(num*3+7)>>3);
	    ptr=&tempg2->y1;
	    ptr[0]=x3;
	    ptr[1]=y3;
	    ptr+=2;
	    x1=x3;y1=y3;
	    i=0;
	    tempg=root2.next;
	    while (tempg!=&root2) {
	      tempg1=tempg->next;
	      BFieldOrU32(ptr,i,
		  GrSpOffset[SignI64(tempg->x1-x1)+1+
		  3*(SignI64(tempg->y1-y1)+1)]);
	      i+=3;
	      x1=tempg->x1;y1=tempg->y1;
	      QueRem(tempg);
	      Free(tempg);
	      tempg=tempg1;
	    }
	    tempg2->type=GRST_POLYPT;
	    tempg2->num=num;
	    QueIns(tempg2,insert_pt->last);
	    GrEdUpdate(doc,tempb,*_root);
	    *cur_num+=1;
	    break;
	  case GRST_BSPLINE2:
	  case GRST_BSPLINE3:
	  case GRST_BSPLINE2_CLOSED:
	  case GRST_BSPLINE3_CLOSED:
	    QueInit(&root2);
	    num=0;
	    x3=p1-x; y3=p2-y;
	    x1=p1; y1=p2;
	    x2=p1; y2=p2;
	    do {
	      do {
		dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		msg_code=GetMsg(&p1,&p2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE+
		    1<<MSG_IP_R_UP);
		dc->color=ROP_EQU|COLOR_TRANSPARENT;
		GrLine3(dc,x1,y1,0,x2,y2,0);
		x2=p1; y2=p2;
	      } while (msg_code!=MSG_IP_L_UP && msg_code!=MSG_IP_R_UP);
	      dc->color=color&COLORROP_NO_ROP0_MASK|ROP_EQU;
	      if (msg_code==MSG_IP_L_UP) {
		GrLine3(dc,x1,y1,0,x2,y2,0);
		tempg=MAlloc(gr_sprite_qued_base_size[GRST_PT]);
		tempg->type=GRST_PT;
		tempg->x1=x2-x;
		tempg->y1=y2-y;
		QueIns(tempg,root2.last);
		x1=x2;y1=y2;
		num++;
	      }
	    } while (msg_code!=MSG_IP_R_UP);
	    if (num) {
	      tempg2=CAlloc(gr_sprite_qued_base_size[GRST_POLYLINE]+
		  (num+1)*sizeof(CD3I32));
	      p=&tempg2->y1;
	      tempg=root2.next;
	      p[0].x=x3;
	      p[0].y=y3;
	      for (i=1;i<=num;i++) {
		tempg1=tempg->next;
		p[i].x=tempg->x1;
		p[i].y=tempg->y1;
		Free(tempg);
		tempg=tempg1;
	      }
	      tempg2->type=eletype;
	      tempg2->num=num+1;
	      QueIns(tempg2,insert_pt->last);
	      GrEdUpdate(doc,tempb,*_root);
	      *cur_num+=1;
	    }
	    break;
	  case GRST_SHIFTABLE_MESH:
	    GetMsg(NULL,NULL,1<<MSG_IP_L_UP);
ei_shiftable_mesh:
	  doc_e->flags1|=DOCEF1_DONT_DRAW;

	    tempg1=insert_pt;
	    if (tempg1!=*_root && tempg1->type==GRST_MESH) {
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
	    } else if (tempg1!=*_root && tempg1->type==GRST_SHIFTABLE_MESH) {
	      z=-tempg1->z1;
	      x1=tempg1->x1-(p1-x);
	      y1=tempg1->y1-(p2-y);
	      z1=tempg1->z1+z;
	      p=(&tempg1->shiftable_triangle_cnt)(U8 *)+sizeof(I32);
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
	    if (mesh=CSpriteMesh(old_mesh,&size,TRUE)) {
	      tempg=MAlloc(gr_sprite_qued_base_size[GRST_SHIFTABLE_MESH]-sizeof(I32)*2+size);
	      tempg->type=GRST_SHIFTABLE_MESH;
	      MemCpy(&tempg->shiftable_vertex_cnt,mesh,size);
	      Free(mesh);
	      tempg->x1=p1-x;
	      tempg->y1=p2-y;
	      tempg->z1=-z;
	      QueIns(tempg,insert_pt->last);
	      GrEdUpdate(doc,tempb,*_root);
	      if (old_mesh) {
		insert_pt=tempg;
		QueRem(tempg1);
		Free(tempg1);
		GrEdUpdate(doc,tempb,*_root);
	      } else
		*cur_num+=1;
	    } else if (old_mesh && tempg1->type==GRST_SHIFTABLE_MESH) {
	      x1=tempg1->x1-(p1-x);
	      y1=tempg1->y1-(p2-y);
	      z1=tempg1->z1+z;
	      p=(&tempg1->shiftable_triangle_cnt)(U8 *)+sizeof(I32);
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
	    doc_e->flags1=old_ce_flags1;
	    eletype=GRST_MENU;
	    break;
	}
      }
    }
  }
ei_done:
  DCFill;
  SettingsPop;
  doc_e->flags1=old_ce_flags1;
  DCDel(dc);
  DCDel(dc2);
  StrCpy(Fs->task_title,old_desc);
  Free(old_desc);
  StrCpy(doc->filename.name,old_filename);
  Free(old_filename);
  if (_import_snap) *_import_snap=new_snap;
  if (_import_ip_scaler) *_import_ip_scaler=new_ip_scaler;
  doc->flags|=old_doc_flags;
  return result;
}

I64 CSpriteCntQue(CSprite *root)
{
  CSprite *tempg=root->next;
  I64 result=0;
  while (tempg->type) {
    result++;
    tempg=tempg->next;
  }
  return result;
}

U0 EdInsGraphic(CDoc *doc,I64 x,I64 y)
{
  Bool unlock;
  U8 *st;
  CDocEntry *doc_e;
  CDocBin *tempb;
  I64 cur_num=0,new_snap=1;
  CSprite *root;
  F64 new_ip_scaler=1.0;
  if (Fs!=doc->mem_task)
    Dbg;
  if (st=EdSprite(doc->cur_bin_num)) {
    SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
    WordStat;
    unlock=DocLock(doc);
    tempb=CAlloc(sizeof(CDocBin),doc->mem_task);
    tempb->type=DOCBT_CSPRITE;
    tempb->size=gr_sprite_base_size[GRST_END];
    tempb->data=CAlloc(tempb->size,doc->mem_task);
    tempb->num=doc->cur_bin_num++;
    tempb->use_cnt=1;
    QueIns(tempb,doc->bin_root.last);
    doc_e=DocPutS(doc,st);
    doc_e->bin_data=tempb;
    Free(st);
    if (doc_e && Bt(doc_tag_types,doc_e->type_u8) && doc_e->tag && *doc_e->tag)
      tempb->tag=StrNew(doc_e->tag,doc->mem_task);
    DocUnlock(doc);
    root=CSprite2Que(tempb->data);
    x+=StrLen(doc_e->tag)*FONT_WIDTH;

    WinMgrSync(2,TRUE);
    switch (CSpriteVect(doc,doc_e,&root,&cur_num,x,y,TRUE,&new_snap,&new_ip_scaler)) {
      case GE_CANCEL:
	QueDel(root);
	Free(root);
	DocLock(doc);
	QueRem(doc_e);
	Free(doc_e);
	Free(doc_e->tag);
	QueRem(tempb);
	Free(tempb->data);
	Free(tempb);
	if (unlock)
	  DocUnlock(doc);
	break;
      case GE_OKAY_DONE:
	QueDel(root);
	Free(root);
	break;
      case GE_OKAY_CONT:
	QueDel(root);
	Free(root);
	doc->cur_entry=doc_e;
	doc->cur_data_col=0;
	DocRecalc(doc);
	EdExistingGraphic(doc,x,y,&new_snap,&new_ip_scaler);
	break;
    }
    SettingsPop;
  }
  if (!(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS)))
    DocBinsValidate(doc);
}
