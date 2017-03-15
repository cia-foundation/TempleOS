#help_index "Graphics/Sprite;Sprites"

#define SPBM_EXIT		0
#define SPBM_MAIN_MENU		1
#define SPBM_COLOR		2
#define SPBM_DITHER_COLOR	3
#define SPBM_WIDTH		4
#define SPBM_PT			5
#define SPBM_LINE		6
#define SPBM_ARROW		7
#define SPBM_RECT		8
#define SPBM_CIRCLE		9
#define SPBM_TEXT		10
#define SPBM_TEXT_BOX		11
#define SPBM_TEXT_DIAMOND	12
#define SPBM_FLOOD_FILL		13
#define SPBM_FLOOD_FILL_NOT	14
#define SPBM_POLYLINE		15
#define SPBM_POLYPT		16
#define SPBM_COPY		17
#define SPBM_DELETE		18
#define SPBM_PASTE		19
#define SPBM_PASTE_TRANSPARENT	20
#define SPBM_FIND_AND_REPLACE	21
#define SPBM_TRIM_TO_EXTENTS	22
#define SPBM_ADD_OUTLINE	23
#define SPBM_ETCH		24
#define SPBM_UNDO		25
#define SPBM_SAVE_BMP		26
#define SPBM_SAVE_GRA		27

U0 GrInit4()
{
  DefineLstLoad("ST_SPRITE_BITMAP_MENU",
	"Exit\0Main Menu\0Color\0Dither Color\0Width\0Point\0Line\0Arrow\0"
	"Rect\0Circle\0Text\0Text Box\0Text Diamond\0Flood Fill\0"
	"Flood Fill Not Color\0PolyLine\0PolyPoint\0Copy\0Delete\0Paste\0"
	"Paste Transparent\0Find and Replace\0Trim to Extents\0Add Outline\0"
	"Etch\0Undo\0Save BMP\0Save GR\0");
}
GrInit4;

I64 PopUpSpriteBitMap(CColorROPU32 color,I64 width)
{
  I64 res;
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
	"\n$$LTBLUE$$$$MU-UL,\"Color        %s\",LE=SPBM_COLOR$$\n"
	"$$MU-UL,\"Dither Color %s\",LE=SPBM_DITHER_COLOR$$\n"
	"$$MU-UL,\"Width        %d\",LE=SPBM_WIDTH$$\n"
	"$$MU-UL,\"Find & Replace Color\",LE=SPBM_FIND_AND_REPLACE$$\n"
	"$$MU-UL,\"Trim to Extents\",LE=SPBM_TRIM_TO_EXTENTS$$\n"
	"$$MU-UL,\"Add Outline\",LE=SPBM_ADD_OUTLINE$$\n"
	"$$MU-UL,\"Etch\",LE=SPBM_ETCH$$\n"
	"\n$$MU-UL,\"Point\",LE=SPBM_PT$$\n"
	"$$MU-UL,\"Line\",LE=SPBM_LINE$$\n"
	"$$MU-UL,\"Arrow\",LE=SPBM_ARROW$$\n"
	"$$MU-UL,\"Rect\",LE=SPBM_RECT$$\n"
	"$$MU-UL,\"Circle\",LE=SPBM_CIRCLE$$\n"
	"$$MU-UL,\"Text\",LE=SPBM_TEXT$$\n"
	"$$MU-UL,\"Text Box\",LE=SPBM_TEXT_BOX$$\n"
	"$$MU-UL,\"Text Diamond\",LE=SPBM_TEXT_DIAMOND$$\n"
	"$$MU-UL,\"Flood Fill\",LE=SPBM_FLOOD_FILL$$\n"
	"$$MU-UL,\"Flood Fill Not Color\",LE=SPBM_FLOOD_FILL_NOT$$\n"
	"$$MU-UL,\"PolyLine\",LE=SPBM_POLYLINE$$\n"
	"$$MU-UL,\"PolyPoint\",LE=SPBM_POLYPT$$\n"
	"\n$$MU-UL,\"Copy to Clip\",LE=SPBM_COPY$$\n"
	"$$MU-UL,\"Delete to Clip\",LE=SPBM_DELETE$$\n"
	"$$MU-UL,\"Paste Clip\",LE=SPBM_PASTE$$\n"
	"$$MU-UL,\"Paste Transparent Clip\",LE=SPBM_PASTE_TRANSPARENT$$\n"
	"\n$$MU-UL,\"Save BMP File\",LE=SPBM_SAVE_BMP$$\n"
	"$$MU-UL,\"Save GR File\",LE=SPBM_SAVE_GRA$$\n"
	"\n$$MU-UL,\"Undo\",LE=SPBM_UNDO$$\n"
	"\n$$PURPLE$$$$MU-UL,\"+] Sprite Main Menu\",LE=SPBM_MAIN_MENU$$$$LTBLUE$$\n"
	"$$MU-UL,\"Exit  Sprite\",LE=SPBM_EXIT$$\n"
	"$$MU-UL,\"Abort Sprite\",LE=DOCM_CANCEL$$\n"
	"\nRight-Click to get back to this menu.",st1,st2,width);
  res=PopUpMenu(doc);
  DocDel(doc);
  return res;
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
  I64 i,j,mode=SPBM_LINE,color=BLACK,width=1,msg_code,arg1,arg2,x1,y1,x11,y11,
	x22,y22,res,xx1=*_xx1,yy1=*_yy1,xx2=*_xx2,yy2=*_yy2,
	old_de_flags=doc_e->de_flags;
  Bool down=FALSE;
  U8 *st=NULL;
  CEdFileName filename;
  CDC *img=*_img,
	*clip=NULL,*undo=NULL,*dc2;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$

  doc_e->de_flags|=DOCEF_DONT_DRAW;
  goto bm_menu;

  while (TRUE) {
    if (kbd.scan_code&SCF_CTRL)//grab scroll update?
      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);

    dc->color=ROPF_DITHER+WHITE<<16+BLACK;
    dc->thick=1;
    GrBorder(dc,xx1-1,yy1-1,xx2,yy2);//This is done little bit too often.

    while (msg_code=ScanMsg(&arg1,&arg2,1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|
	  1<<MSG_MS_R_DOWN|1<<MSG_MS_MOVE|1<<MSG_KEY_DOWN)) {
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case CH_SHIFT_ESC:
	      res=SPE_ABORT;
	      goto bm_key_up_done;
	    case CH_ESC:
	      res=SPE_CONT;
	      goto bm_key_up_done;
	    case 'c': //eye-dropper
	      dc2=DCScrnCapture(FALSE);
	      color=GrPeek(dc2,ms.pos.x,ms.pos.y)^15;//Mouse cursor is XORed.
	      DCDel(dc2);
	      break;
	    case 't': //Set to transparent color
	      color=TRANSPARENT;
	      break;
	  }
	  break;

	case MSG_MS_R_DOWN:
bm_menu:
	  DCFill(dc);
	  StrCpy(Fs->task_title,"Sprite BitMap Menu");
	  i=PopUpSpriteBitMap(color,width);
	  if (i>=0)
	    StrCpy(Fs->task_title,DefineSub(i,"ST_SPRITE_BITMAP_MENU"));
	  switch (i) {
	    case DOCM_CANCEL:
	      res=SPE_ABORT;
	      goto bm_done;
	    case SPBM_EXIT:
	      res=SPE_EXIT;
	      goto bm_done;
	    case SPBM_MAIN_MENU:
	      res=SPE_CONT;
	      goto bm_done;
	    case SPBM_COLOR:
	      i=PopUpColor(,,FALSE);
	      if (i>=0) color=i;
	      goto bm_menu;
	    case SPBM_FIND_AND_REPLACE:
	      i=PopUpColor("Find Color\n",,FALSE);
	      if (i>=0) {
		j=PopUpColor("Replace Color\n",,FALSE);
		if (j>=0) {
		  DCColorChg(img,i,j);
		  GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		}
	      }
	      goto bm_menu;
	    case SPBM_TRIM_TO_EXTENTS:
	      GrBitMapEdTrimToExtents(&img,&xx1,&yy1,&xx2,&yy2,bkcolor);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      goto bm_menu;
	    case SPBM_ADD_OUTLINE:
	      i=PopUpRangeI64(1,16,1,"Outline Width\n");
	      if (i>=0) {
		GrBitMapEdAddOutline(img,i,color,bkcolor);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      goto bm_menu;
	    case SPBM_ETCH:
	      i=PopUpRangeI64(1,16,1,"Etch Width\n");
	      if (i>=0) {
		GrBitMapEdEtch(img,i,bkcolor);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      goto bm_menu;
	    case SPBM_SAVE_BMP:
	      *filename.name=0;
	      if (DocForm(&filename) && *filename.name)
		BMPWrite(filename.name,img);
	      goto bm_menu;
	    case SPBM_SAVE_GRA:
	      *filename.name=0;
	      if (DocForm(&filename) && *filename.name)
		GRWrite(filename.name,img,DCSF_COMPRESSED|DCSF_PALETTE_GET);
	      goto bm_menu;
	    case SPBM_DITHER_COLOR:
	      i=PopUpColorDither;
	      if (i>=0) color=i;
	      goto bm_menu;
	    case SPBM_WIDTH:
	      i=PopUpRangeI64(1,16,1,"Thick\n");
	      if (i>=0) width=i;
	      goto bm_menu;
	    case SPBM_UNDO:
	      if (undo) {
		DCFill(img,bkcolor);
		img->color=ROP_EQU;
		GrBlot(img,0,0,undo);
		DCDel(undo);
		undo=NULL;
	      }
	      goto bm_menu;
	    case SPBM_PT:
	    case SPBM_LINE:
	    case SPBM_ARROW:
	    case SPBM_RECT:
	    case SPBM_CIRCLE:
	    case SPBM_FLOOD_FILL:
	    case SPBM_FLOOD_FILL_NOT:
	    case SPBM_POLYPT:
	    case SPBM_POLYLINE:
	    case SPBM_COPY:
	    case SPBM_DELETE:
	    case SPBM_PASTE:
	    case SPBM_PASTE_TRANSPARENT:
	      mode=i;
	      break;
	    case SPBM_TEXT:
	    case SPBM_TEXT_BOX:
	    case SPBM_TEXT_DIAMOND:
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
	  Refresh(2,TRUE);	//Let popup close
	  GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	  down=FALSE;
	  break;
	case MSG_MS_L_DOWN:
	  switch (mode) {
	    case SPBM_PT:
	      img->color=color;
	      img->thick=width;
	      GrPlot3(img,arg1-xx1,arg2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    start:
	      if (down)
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      x1=arg1; y1=arg2;
	      down=TRUE;
	      dc->color=color;
	      dc->thick=width;
	      case SPBM_LINE:
		GrLine3(dc,x1,y1,0,arg1,arg2,0);
		break;
	      case SPBM_ARROW:
		GrArrow3(dc,x1,y1,0,arg1,arg2,0);
		break;
	      case SPBM_RECT:
		GrRect(dc,x1,y1,1,1);
		break;
	      case SPBM_CIRCLE:
		GrCircle3(dc,x1,y1,0,1);
		break;
	      case SPBM_COPY:
	      case SPBM_DELETE:
		dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		dc->thick=1;
		GrBorder(dc,x1,y1,x1,y1);
		break;
	    end:
	      break;
	    case SPBM_PASTE:
	    case SPBM_PASTE_TRANSPARENT:
	      if (clip) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==SPBM_PASTE) {
		  clip->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(dc,arg1,arg2,clip);
		  clip->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clip);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(dc,arg1,arg2,dc2);
		  DCDel(dc2);
		}
	      }
	      break;
	    case SPBM_TEXT:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrPrint(dc,arg1,arg2,"%s",st);
	      break;
	    case SPBM_TEXT_BOX:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextBox3(dc,arg1,arg2,0,st);
	      break;
	    case SPBM_TEXT_DIAMOND:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextDiamond3(dc,arg1,arg2,0,st);
	      break;
	    case SPBM_FLOOD_FILL:
	      img->color=color;
	      GrFloodFill(img,arg1-xx1,arg2-yy1);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case SPBM_FLOOD_FILL_NOT:
	      img->color=color;
	      GrFloodFill(img,arg1-xx1,arg2-yy1,TRUE);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case SPBM_POLYLINE:
	      if (!down) {
		x1=arg1; y1=arg2;
		down=TRUE;
		dc->color=color;
		dc->thick=width;
		GrLine3(dc,x1,y1,0,arg1,arg2,0);
	      }
	      break;
	    case SPBM_POLYPT:
	      x1=arg1; y1=arg2;
	      down=TRUE;
	      img->color=color;
	      img->thick=width;
	      GrLine3(img,x1-xx1,y1-yy1,0,arg1-xx1,arg2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	  }
	  break;
	case MSG_MS_MOVE:
	  switch (mode) {
	    case SPBM_LINE:
	    case SPBM_ARROW:
	    case SPBM_POLYLINE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		dc->color=color;
		dc->thick=width;
		if (mode==SPBM_ARROW)
		  GrArrow3(dc,x1,y1,0,arg1,arg2,0);
		else
		  GrLine3(dc,x1,y1,0,arg1,arg2,0);
	      }
	      break;
	    case SPBM_RECT:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (x1<arg1) {
		  x11=x1;
		  x22=arg1;
		} else {
		  x11=arg1;
		  x22=x1;
		}
		if (y1<arg2) {
		  y11=y1;
		  y22=arg2;
		} else {
		  y11=arg2;
		  y22=y1;
		}
		dc->color=color;
		GrRect(dc,x11,y11,x22-x11+1,y22-y11+1);
	      }
	      break;
	    case SPBM_COPY:
	    case SPBM_DELETE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (x1<arg1) {
		  x11=x1;
		  x22=arg1;
		} else {
		  x11=arg1;
		  x22=x1;
		}
		if (y1<arg2) {
		  y11=y1;
		  y22=arg2;
		} else {
		  y11=arg2;
		  y22=y1;
		}
		dc->color=ROPF_DITHER+WHITE<<16+BLACK;
		dc->thick=1;
		GrBorder(dc,x11,y11,x22,y22);
	      }
	      break;
	    case SPBM_CIRCLE:
	      if (down) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		dc->color=color;
		dc->thick=width;
		GrCircle3(dc,x1,y1,0,Sqrt(SqrI64(arg1-x1)+SqrI64(arg2-y1)));
	      }
	      break;
	    case SPBM_PASTE:
	    case SPBM_PASTE_TRANSPARENT:
	      if (clip) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==SPBM_PASTE) {
		  clip->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(dc,arg1,arg2,clip);
		  clip->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clip);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(dc,arg1,arg2,dc2);
		  DCDel(dc2);
		}
	      }
	      break;
	    case SPBM_TEXT:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrPrint(dc,arg1,arg2,"%s",st);
	      break;
	    case SPBM_TEXT_BOX:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextBox3(dc,arg1,arg2,0,st);
	      break;
	    case SPBM_TEXT_DIAMOND:
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      dc->color=color;
	      GrTextDiamond3(dc,arg1,arg2,0,st);
	      break;
	    case SPBM_POLYPT:
	      if (down) {
		img->color=color;
		img->thick=width;
		GrLine3(img,x1-xx1,y1-yy1,0,arg1-xx1,arg2-yy1,0);
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		x1=arg1; y1=arg2;
	      }
	      break;
	  }
	  break;
	case MSG_MS_L_UP:
	  switch (mode) {
	    case SPBM_LINE:
	    case SPBM_ARROW:
	    case SPBM_POLYPT:
	    case SPBM_POLYLINE:
	      img->color=color;
	      img->thick=width;
	      if (mode==SPBM_ARROW)
		GrArrow3(img,x1-xx1,y1-yy1,0,arg1-xx1,arg2-yy1,0);
	      else
		GrLine3(img,x1-xx1,y1-yy1,0,arg1-xx1,arg2-yy1,0);
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      if (mode==SPBM_POLYLINE) {
		x1=arg1; y1=arg2;
	      } else
		down=FALSE;
	      break;
	    case SPBM_RECT:
	      img->color=color;
	      if (x1<arg1) {
		x11=x1;
		x22=arg1;
	      } else {
		x11=arg1;
		x22=x1;
	      }
	      if (y1<arg2) {
		y11=y1;
		y22=arg2;
	      } else {
		y11=arg2;
		y22=y1;
	      }
	      GrRect(img,x11-xx1,y11-yy1,x22-x11+1,y22-y11+1);
	      down=FALSE;
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case SPBM_COPY:
	    case SPBM_DELETE:
	      if (x1<arg1) {
		x11=x1;
		x22=arg1;
	      } else {
		x11=arg1;
		x22=x1;
	      }
	      if (y1<arg2) {
		y11=y1;
		y22=arg2;
	      } else {
		y11=arg2;
		y22=y1;
	      }
	      DCDel(clip);
	      clip=DCExt(img,x11-xx1,y11-yy1,x22-xx1,y22-yy1);
	      clip->bkcolor=bkcolor;
	      if (mode==SPBM_DELETE) {
		img->color=bkcolor;
		GrRect(img,x11-xx1,y11-yy1,x22-x11+1,y22-y11+1);
	      }
	      goto bm_menu;
	    case SPBM_CIRCLE:
	      img->color=color;
	      img->thick=width;
	      GrCircle3(img,x1-xx1,y1-yy1,0,
		    Sqrt(SqrI64(arg1-x1)+SqrI64(arg2-y1)));
	      down=FALSE;
	      GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      break;
	    case SPBM_PASTE:
	    case SPBM_PASTE_TRANSPARENT:
	      if (clip) {
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
		if (mode==SPBM_PASTE) {
		  clip->flags|=DCF_NO_TRANSPARENTS;
		  GrBlot(img,arg1-xx1,arg2-yy1,clip);
		  clip->flags&=~DCF_NO_TRANSPARENTS;
		} else {
		  dc2=DCCopy(clip);
		  DCColorChg(dc2,bkcolor);
		  GrBlot(img,arg1-xx1,arg2-yy1,dc2);
		  DCDel(dc2);
		}
		GrBitMapEdPrepPersistentDC(dc,xx1,yy1,img);
	      }
	      break;
	    case SPBM_TEXT:
	      img->color=color;
	      GrPrint(img,arg1-xx1,arg2-yy1,"%s",st);
	      goto bm_menu;
	    case SPBM_TEXT_BOX:
	      img->color=color;
	      GrTextBox3(img,arg1-xx1,arg2-yy1,0,st);
	      goto bm_menu;
	    case SPBM_TEXT_DIAMOND:
	      img->color=color;
	      GrTextDiamond3(img,arg1-xx1,arg2-yy1,0,st);
	      goto bm_menu;
	  }
	  break;
      }
    }
    Refresh;
  }
bm_key_up_done:
  GetMsg(,,1<<MSG_KEY_UP);
bm_done:
  DCDel(clip);
  DCDel(undo);
  Free(st);
  DCFill(dc);
  SettingsPop;
  doc_e->de_flags=old_de_flags;
  *_img=img;
  *_xx1=xx1,*_yy1=yy1,*_xx2=xx2,*_yy2=yy2;
  return res;
}
