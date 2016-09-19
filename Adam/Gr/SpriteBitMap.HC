#help_index "Graphics/Sprite;Sprites"

#define GBM_EXIT		0
#define GBM_MAIN_MENU		1
#define GBM_COLOR		2
#define GBM_DITHER_COLOR	3
#define GBM_WIDTH		4
#define GBM_PT			5
#define GBM_LINE		6
#define GBM_ARROW		7
#define GBM_RECT		8
#define GBM_CIRCLE		9
#define GBM_TEXT		10
#define GBM_TEXT_BOX		11
#define GBM_TEXT_DIAMOND	12
#define GBM_FLOOD_FILL		13
#define GBM_FLOOD_FILL_NOT	14
#define GBM_POLYLINE		15
#define GBM_POLYPT		16
#define GBM_COPY		17
#define GBM_DELETE		18
#define GBM_PASTE		19
#define GBM_PASTE_TRANSPARENT	20
#define GBM_FIND_AND_REPLACE	21
#define GBM_TRIM_TO_EXTENTS	22
#define GBM_ADD_OUTLINE	23
#define GBM_ETCH		24
#define GBM_UNDO		25
#define GBM_SAVE_BMP		26
#define GBM_SAVE_GRA		27

U0 GrInit4()
{
  DefineLstLoad("ST_SPRITE_BITMAP_MENU",
	"Exit\0Main Menu\0Color\0Dither Color\0Width\0Point\0Line\0Arrow\0"
	"Rect\0Circle\0Text\0Text Box\0Text Diamond\0Flood Fill\0"
	"Flood Fill Not Color\0PolyLine\0PolyPoint\0Copy\0Delete\0Paste\0"
	"Paste Transparent\0Find and Replace\0Trim to Extents\0Add Outline\0"
	"Etch\0Undo\0Save BMP\0Save GRA\0");
}
GrInit4;

I64 PopUpSpriteBitMap(CColorROPU32 color,I64 width)
{
  I64 i;
  U8 *st1,*st2,buf[STR_LEN];
  CDoc *doc=DocNew;

  Color2Str(buf,color);
  if (color&ROPF_DITHER) {
    st1="";
    st2=buf;
  } else {
    st1=buf;
    st2="";
  }
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Sprite BitMap Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/SpriteBitMap.DD.Z\"$$\n"
	"\n$$LTBLUE$$$$MU-UL,\"Color        %s\",LE=GBM_COLOR$$\n"
	"$$MU-UL,\"Dither Color %s\",LE=GBM_DITHER_COLOR$$\n"
	"$$MU-UL,\"Width        %d\",LE=GBM_WIDTH$$\n"
	"$$MU-UL,\"Find & Replace Color\",LE=GBM_FIND_AND_REPLACE$$\n"
	"$$MU-UL,\"Trim to Extents\",LE=GBM_TRIM_TO_EXTENTS$$\n"
	"$$MU-UL,\"Add Outline\",LE=GBM_ADD_OUTLINE$$\n"
	"$$MU-UL,\"Etch\",LE=GBM_ETCH$$\n"
	"\n$$MU-UL,\"Point\",LE=GBM_PT$$\n"
	"$$MU-UL,\"Line\",LE=GBM_LINE$$\n"
	"$$MU-UL,\"Arrow\",LE=GBM_ARROW$$\n"
	"$$MU-UL,\"Rect\",LE=GBM_RECT$$\n"
	"$$MU-UL,\"Circle\",LE=GBM_CIRCLE$$\n"
	"$$MU-UL,\"Text\",LE=GBM_TEXT$$\n"
	"$$MU-UL,\"Text Box\",LE=GBM_TEXT_BOX$$\n"
	"$$MU-UL,\"Text Diamond\",LE=GBM_TEXT_DIAMOND$$\n"
	"$$MU-UL,\"Flood Fill\",LE=GBM_FLOOD_FILL$$\n"
	"$$MU-UL,\"Flood Fill Not Color\",LE=GBM_FLOOD_FILL_NOT$$\n"
	"$$MU-UL,\"PolyLine\",LE=GBM_POLYLINE$$\n"
	"$$MU-UL,\"PolyPoint\",LE=GBM_POLYPT$$\n"
	"\n$$MU-UL,\"Copy to Clipboard\",LE=GBM_COPY$$\n"
	"$$MU-UL,\"Delete to Clipboard\",LE=GBM_DELETE$$\n"
	"$$MU-UL,\"Paste Clipboard\",LE=GBM_PASTE$$\n"
	"$$MU-UL,\"Paste Transparent Clipboard\",LE=GBM_PASTE_TRANSPARENT$$\n"
	"\n$$MU-UL,\"Save BMP File\",LE=GBM_SAVE_BMP$$\n"
	"$$MU-UL,\"Save GRA File\",LE=GBM_SAVE_GRA$$\n"
	"\n$$MU-UL,\"Undo\",LE=GBM_UNDO$$\n"
	"\n$$PURPLE$$$$MU-UL,\"+] Sprite Main Menu\",LE=GBM_MAIN_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=GBM_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$\n"
	"\nRight-Click to get back to this menu.",st1,st2,width);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 GrBitMapEdPrepPersistentDC(CDC *dc,I64 xx1,I64 yy1,CDC *img)
{
  DCFill(dc);
  GrBlot(dc,xx1,yy1,img);
}

U0 GrBitMapEdTrimToExtents(CDC **_img,I64 *_xx1,I64 *_yy1,
	I64 *_xx2,I64 *_yy2,CColorROPU32 bkcolor)
{
  CDC *img=*_img;
  I64 i,c,
	x1=0,y1=0,x2=img->width-1,y2=img->height-1; //inclusive
  while (y1<y2) {
    for (i=x1;i<=x2;i++) {
      c=GrPeek(img,i,y1);
      if (c!=bkcolor&&c!=TRANSPARENT) goto tr_y2;
    }
    y1++;
  }

  tr_y2:
  while (y1<y2) {
    for (i=x1;i<=x2;i++) {
      c=GrPeek(img,i,y2);
      if (c!=bkcolor&&c!=TRANSPARENT) goto tr_x1;
    }
    y2--;
  }

  tr_x1:
  while (x1<x2) {
    for (i=y1;i<=y2;i++) {
      c=GrPeek(img,x1,i);
      if (c!=bkcolor&&c!=TRANSPARENT) goto tr_x2;
    }
    x1++;
  }

  tr_x2:
  while (x1<x2) {
    for (i=y1;i<=y2;i++) {
      c=GrPeek(img,x2,i);
      if (c!=bkcolor&&c!=TRANSPARENT) goto tr_done;
    }
    x2--;
  }

  tr_done:
  *_img=DCExt(img,x1,y1,x2,y2);
  *_xx1+=x1;		    *_yy1+=y1;
  *_xx2+=x2-(img->width-1); *_yy2+=y2-(img->height-1); //not inclusive
  DCDel(img);
}

U0 GrBitMapEdAddOutline(CDC *img,I64 width,
	CColorROPU32 color,CColorROPU32 bkcolor)
{
  I64 i,j,k,c;
  CColorROPU32 old_color;
  CDC *src;
  if (img->width && img->height) {
    old_color=img->color;
    img->color=color;
    while (width-->0) {
      src=DCExt(img,0,0,img->width-1,img->height-1);
      for (i=0;i<img->height;i++)
	for (j=0;j<img->width;j++)
	  if (GrPeek(src,j,i)==bkcolor)
	    for (k=0;k<8;k++) {
	      c=GrPeek(src,j+gr_x_offsets[k],i+gr_y_offsets[k]);
	      if (c>=0 && c!=bkcolor) {
		GrPlot(img,j,i);
		break;
	      }
	    }
      DCDel(src);
    }
    img->color=old_color;
  }
}

U0 GrBitMapEdEtch(CDC *img,I64 width,CColorROPU32 bkcolor)
{
  I64 i,j,k,c;
  CColorROPU32 old_color;
  CDC *src;
  if (img->width && img->height) {
    old_color=img->color;
    img->color=bkcolor;
    while (width-->0) {
      src=DCExt(img,0,0,img->width-1,img->height-1);
      for (i=0;i<img->height;i++)
	for (j=0;j<img->width;j++)
	  if (GrPeek(src,j,i)!=bkcolor)
	    for (k=0;k<8;k++) {
	      c=GrPeek(src,j+gr_x_offsets[k],i+gr_y_offsets[k]);
	      if (c<0 || c==bkcolor) {
		GrPlot(img,j,i);
		break;
	      }
	    }
      DCDel(src);
    }
    img->color=old_color;
  }
}

I64 SpriteBitMapEd(CDoc *,CDocEntry *doc_e,CDC *dc,I64 *_xx1,I64 *_yy1,
	I64 *_xx2,I64 *_yy2,CDC **_img,CColorROPU32 bkcolor)
{
  I64 i,j,mode=GBM_LINE,color=BLACK,width=1,msg_code,a1,a2,x1,y1,x11,y11,
	x22,y22,res,xx1=*_xx1,yy1=*_yy1,xx2=*_xx2,yy2=*_yy2,
	old_de_flags=doc_e->de_flags;
  Bool down=FALSE;
  U8 *st=NULL;
  CEdFileName filename;
  CDC *img=*_img,
	*clipboard=NULL,*undo=NULL,*dc2;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$

  doc_e->de_flags|=DOCEF_DONT_DRAW;
  goto bm_menu;

  while (TRUE) {
    if (kbd.scan_code&SCF_CTRL)//grab scroll update?
      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);

    dc->color=ROPF_DITHER+WHITE<<16+BLACK;
    dc->pen_width=1;
    GrBorder(dc,xx1-1,yy1-1,xx2,yy2);//This is done little bit too often.

    while (msg_code=ScanMsg(&a1,&a2,1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|
	  1<<MSG_IP_R_DOWN|1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN)) {
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case CH_SHIFT_ESC:
	      res=GE_ABORT;
	      goto bm_key_up_done;
	    case CH_ESC:
	      res=GE_CONT;
	      goto bm_key_up_done;
	    case 'c': //eye-dropper
	      dc2=DCScreenCapture(FALSE);
	      color=GrPeek(dc2,ip.pos.x,ip.pos.y)^15;//Mouse cursor is XORed.
	      DCDel(dc2);
	      break;
	    case 't': //Set to transparent color
	      color=TRANSPARENT;
	      break;
	  }
	  break;

	case MSG_IP_R_DOWN:
bm_menu:
	  DCFill(dc);
	  StrCpy(Fs->task_title,"Sprite BitMap Menu");
	  i=PopUpSpriteBitMap(color,width);
	  if (i>=0)
	    StrCpy(Fs->task_title,DefineSub(i,"ST_SPRITE_BITMAP_MENU"));
	  switch (i) {
	    case DOCM_CANCEL:
	      res=GE_ABORT;
	      goto bm_done;
	    case GBM_EXIT:
	      res=GE_EXIT;
	      goto bm_done;
	    case GBM_MAIN_MENU:
	      res=GE_CONT;
	      goto bm_done;
	    case GBM_COLOR:
	      i=PopUpColor(,,FALSE);
	      if (i>=0) color=i;
	      goto bm_menu;
	    case GBM_FIND_AND_REPLACE:
	      i=PopUpColor("Find Color\n",,FALSE);
	      if (i>=0) {
		j=PopUpColor("Replace Color\n",,FALSE);
		if (j>=0) {
		  DCColorChg(img,i,j);
		  GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		}
	      }
	      goto bm_menu;
	    case GBM_TRIM_TO_EXTENTS:
	      GrBitMapEdTrimToExtents(&img,&xx1,&yy1,&xx2,&yy2,bkcolor);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      goto bm_menu;
	    case GBM_ADD_OUTLINE:
	      i=PopUpRangeI64(1,16,1,"Outline Width\n");
	      if (i>=0) {
		GrBitMapEdAddOutline(img,i,color,bkcolor);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      goto bm_menu;
	    case GBM_ETCH:
	      i=PopUpRangeI64(1,16,1,"Etch Width\n");
	      if (i>=0) {
		GrBitMapEdEtch(img,i,bkcolor);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      goto bm_menu;
	    case GBM_SAVE_BMP:
	      *filename.name=0;
	      if (DocForm(&filename) && *filename.name)
		BMPWrite(filename.name,img);
	      goto bm_menu;
	    case GBM_SAVE_GRA:
	      *filename.name=0;
	      if (DocForm(&filename) && *filename.name)
		GRAWrite(filename.name,img,DCSF_COMPRESSED|DCSF_PALETTE_GET);
	      goto bm_menu;
	    case GBM_DITHER_COLOR:
	      i=PopUpColorDither;
	      if (i>=0) color=i;
	      goto bm_menu;
	    case GBM_WIDTH:
	      i=PopUpRangeI64(1,16,1,"Pen Width\n");
	      if (i>=0) width=i;
	      goto bm_menu;
	    case GBM_UNDO:
	      if (undo) {
		DCFill(img,bkcolor);
		img->color=ROP_EQU;
		GrBlot(img,0,0,undo);
		DCDel(undo);
		undo=NULL;
	      }
	      goto bm_menu;
	    case GBM_PT:
	    case GBM_LINE:
	    case GBM_ARROW:
	    case GBM_RECT:
	    case GBM_CIRCLE:
	    case GBM_FLOOD_FILL:
	    case GBM_FLOOD_FILL_NOT:
	    case GBM_POLYPT:
	    case GBM_POLYLINE:
	    case GBM_COPY:
	    case GBM_DELETE:
	    case GBM_PASTE:
	    case GBM_PASTE_TRANSPARENT:
	      mode=i;
	      break;
	    case GBM_TEXT:
	    case GBM_TEXT_BOX:
	    case GBM_TEXT_DIAMOND:
	      Free(st);
	      st=PopUpGetStr("Enter text and press <ESC>.\n");
	      if (st && *st)
		mode=i;
	      else
		goto bm_menu;
	      break;
	  }
	  DCDel(undo);
	  undo=DCExt(img,0,0,img->width-1,img->height-1);
	  undo->bkcolor=bkcolor;
	  WinMgrSync(2,TRUE);	//Let popup close
	  GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	  down=FALSE;
	  break;
	case MSG_IP_L_DOWN:
	  switch (mode) {
	    case GBM_PT:
	      img->color=color;
	      img->pen_width=width;
	      GrPlot3(img,a1-xx1,a2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    start:
	      if (down)
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      x1=a1; y1=a2;
	      down=TRUE;
	      dc->color=color;
	      dc->pen_width=width;
	      case GBM_LINE:
		GrLine3(dc,x1,y1,0,a1,a2,0);
		break;
	      case GBM_ARROW:
		GrArrow3(dc,x1,y1,0,a1,a2,0);
		break;
	      case GBM_RECT:
		GrRect(dc,x1,y1,1,1);
		break;
	      case GBM_CIRCLE:
		GrCircle3(dc,x1,y1,0,1);
		break;
	      case GBM_COPY:
	      case GBM_DELETE:
		dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		dc->pen_width=1;
		GrBorder(dc,x1,y1,x1,y1);
		break;
	    end:
	      break;
	    case GBM_PASTE:
	    case GBM_PASTE_TRANSPARENT:
	      if (clipboard) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==GBM_PASTE) {
		  clipboard->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(dc,a1,a2,clipboard);
		  clipboard->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clipboard);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(dc,a1,a2,dc2);
		  DCDel(dc2);
		}
	      }
	      break;
	    case GBM_TEXT:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrPrint(dc,a1,a2,"%s",st);
	      break;
	    case GBM_TEXT_BOX:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextBox3(dc,a1,a2,0,st);
	      break;
	    case GBM_TEXT_DIAMOND:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextDiamond3(dc,a1,a2,0,st);
	      break;
	    case GBM_FLOOD_FILL:
	      img->color=color;
	      GrFloodFill(img,a1-xx1,a2-yy1);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case GBM_FLOOD_FILL_NOT:
	      img->color=color;
	      GrFloodFill(img,a1-xx1,a2-yy1,TRUE);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case GBM_POLYLINE:
	      if (!down) {
		x1=a1; y1=a2;
		down=TRUE;
		dc->color=color;
		dc->pen_width=width;
		GrLine3(dc,x1,y1,0,a1,a2,0);
	      }
	      break;
	    case GBM_POLYPT:
	      x1=a1; y1=a2;
	      down=TRUE;
	      img->color=color;
	      img->pen_width=width;
	      GrLine3(img,x1-xx1,y1-yy1,0,a1-xx1,a2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	  }
	  break;
	case MSG_IP_MOVE:
	  switch (mode) {
	    case GBM_LINE:
	    case GBM_ARROW:
	    case GBM_POLYLINE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		dc->color=color;
		dc->pen_width=width;
		if (mode==GBM_ARROW)
		  GrArrow3(dc,x1,y1,0,a1,a2,0);
		else
		  GrLine3(dc,x1,y1,0,a1,a2,0);
	      }
	      break;
	    case GBM_RECT:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (x1<a1) {
		  x11=x1;
		  x22=a1;
		} else {
		  x11=a1;
		  x22=x1;
		}
		if (y1<a2) {
		  y11=y1;
		  y22=a2;
		} else {
		  y11=a2;
		  y22=y1;
		}
		dc->color=color;
		GrRect(dc,x11,y11,x22-x11+1,y22-y11+1);
	      }
	      break;
	    case GBM_COPY:
	    case GBM_DELETE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (x1<a1) {
		  x11=x1;
		  x22=a1;
		} else {
		  x11=a1;
		  x22=x1;
		}
		if (y1<a2) {
		  y11=y1;
		  y22=a2;
		} else {
		  y11=a2;
		  y22=y1;
		}
		dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		dc->pen_width=1;
		GrBorder(dc,x11,y11,x22,y22);
	      }
	      break;
	    case GBM_CIRCLE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		dc->color=color;
		dc->pen_width=width;
		GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(a1-x1)+SqrI64(a2-y1)));
	      }
	      break;
	    case GBM_PASTE:
	    case GBM_PASTE_TRANSPARENT:
	      if (clipboard) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==GBM_PASTE) {
		  clipboard->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(dc,a1,a2,clipboard);
		  clipboard->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clipboard);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(dc,a1,a2,dc2);
		  DCDel(dc2);
		}
	      }
	      break;
	    case GBM_TEXT:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrPrint(dc,a1,a2,"%s",st);
	      break;
	    case GBM_TEXT_BOX:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextBox3(dc,a1,a2,0,st);
	      break;
	    case GBM_TEXT_DIAMOND:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextDiamond3(dc,a1,a2,0,st);
	      break;
	    case GBM_POLYPT:
	      if (down) {
		img->color=color;
		img->pen_width=width;
		GrLine3(img,x1-xx1,y1-yy1,0,a1-xx1,a2-yy1,0);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		x1=a1; y1=a2;
	      }
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (mode) {
	    case GBM_LINE:
	    case GBM_ARROW:
	    case GBM_POLYPT:
	    case GBM_POLYLINE:
	      img->color=color;
	      img->pen_width=width;
	      if (mode==GBM_ARROW)
		GrArrow3(img,x1-xx1,y1-yy1,0,a1-xx1,a2-yy1,0);
	      else
		GrLine3(img,x1-xx1,y1-yy1,0,a1-xx1,a2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      if (mode==GBM_POLYLINE) {
		x1=a1; y1=a2;
	      } else
		down=FALSE;
	      break;
	    case GBM_RECT:
	      img->color=color;
	      if (x1<a1) {
		x11=x1;
		x22=a1;
	      } else {
		x11=a1;
		x22=x1;
	      }
	      if (y1<a2) {
		y11=y1;
		y22=a2;
	      } else {
		y11=a2;
		y22=y1;
	      }
	      GrRect(img,x11-xx1,y11-yy1,x22-x11+1,y22-y11+1);
	      down=FALSE;
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case GBM_COPY:
	    case GBM_DELETE:
	      if (x1<a1) {
		x11=x1;
		x22=a1;
	      } else {
		x11=a1;
		x22=x1;
	      }
	      if (y1<a2) {
		y11=y1;
		y22=a2;
	      } else {
		y11=a2;
		y22=y1;
	      }
	      DCDel(clipboard);
	      clipboard=DCExt(img,x11-xx1,y11-yy1,x22-xx1,y22-yy1);
	      clipboard->bkcolor=bkcolor;
	      if (mode==GBM_DELETE) {
		img->color=bkcolor;
		GrRect(img,x11-xx1,y11-yy1,x22-x11+1,y22-y11+1);
	      }
	      goto bm_menu;
	    case GBM_CIRCLE:
	      img->color=color;
	      img->pen_width=width;
	      GrCircle3(img,x1-xx1,y1-yy1,0,Sqrt(SqrI64(a1-x1)+SqrI64(a2-y1)));
	      down=FALSE;
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case GBM_PASTE:
	    case GBM_PASTE_TRANSPARENT:
	      if (clipboard) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==GBM_PASTE) {
		  clipboard->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(img,a1-xx1,a2-yy1,clipboard);
		  clipboard->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clipboard);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(img,a1-xx1,a2-yy1,dc2);
		  DCDel(dc2);
		}
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      break;
	    case GBM_TEXT:
	      img->color=color;
	      GrPrint(img,a1-xx1,a2-yy1,"%s",st);
	      goto bm_menu;
	    case GBM_TEXT_BOX:
	      img->color=color;
	      GrTextBox3(img,a1-xx1,a2-yy1,0,st);
	      goto bm_menu;
	    case GBM_TEXT_DIAMOND:
	      img->color=color;
	      GrTextDiamond3(img,a1-xx1,a2-yy1,0,st);
	      goto bm_menu;
	  }
	  break;
      }
    }
    WinMgrSync;
  }
bm_key_up_done:
  GetMsg(,,1<<MSG_KEY_UP);
bm_done:
  DCDel(clipboard);
  DCDel(undo);
  Free(st);
  DCFill(dc);
  SettingsPop;
  doc_e->de_flags=old_de_flags;
  *_img=img;
  *_xx1=xx1,*_yy1=yy1,*_xx2=xx2,*_yy2=yy2;
  return res;
}
