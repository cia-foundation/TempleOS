/*I got tricky by not defining a color
right away in these $LK,"CSprite",A="MN:CSprite"$s so they can
work for both players by setting dc->color
before drawing them.  I actually made these
graphics by defining a color in the <CTRL-r>
menu, drawing the unit and deleting the color.

I had to leave a gap between the tank tread
and body because of how it is rendered when rotated.
*/

$SP,"<1>",BI=1$

$SP,"<2>",BI=2$

//This is an infantry.

$SP,"<3>",BI=3$

$SP,"<4>",BI=4$


U0 DrawHexes()
{
  F64 dx=2*HEX_SIDE+2*DCOS,dy=2*DSIN,
	x,y,x1,y1,x2,y2;
  I64 i,j;
  map_dc->color=WHITE;
  GrRect(map_dc,0,0,map_dc->width,map_dc->height);
  map_dc->color=BLACK;
  y=0;
  for (j=0;j<map_rows;j+=2) {
    x=DCOS;
    GrLine(map_dc,x,y,x-DCOS,y+DSIN);
    GrLine(map_dc,x-DCOS,y+DSIN,x,y+2*DSIN);
    for (i=0;i<map_cols;i++) {
      x1=x; y1=y;
      x2=x1+HEX_SIDE; y2=y1;
      GrLine(map_dc,x1,y1,x2,y2);
      x1=x2; y1=y2;
      x2+=DCOS; y2+=DSIN;
      GrLine(map_dc,x1,y1,x2,y2);
      GrLine(map_dc,x2,y2,x2-DCOS,y2+DSIN);
      x1=x2; y1=y2;
      x2+=HEX_SIDE;
      GrLine(map_dc,x1,y1,x2,y2);
      GrLine(map_dc,x2,y2,x2+DCOS,y2+DSIN);
      x1=x2; y1=y2;
      x2+=DCOS; y2-=DSIN;
      if (j || i<map_cols-1)
	GrLine(map_dc,x1,y1,x2,y2);
      x+=dx;
    }
    y+=dy;
  }
  x=DCOS;
  for (i=0;i<map_cols;i++) {
    x1=x; y1=y;
    x2=x1+HEX_SIDE; y2=y1;
    GrLine(map_dc,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=DCOS; y2+=DSIN;
    GrLine(map_dc,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=HEX_SIDE;
    GrLine(map_dc,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=DCOS; y2-=DSIN;
    GrLine(map_dc,x1,y1,x2,y2);
    x+=dx;
  }
}

U0 MakeTerrain(U8 color,I64 cnt,I64 clus_lo,I64 clus_hi)
{
  I64 i,j,l,row,col;
  for (i=0;i<cnt;i++) {
    col=RandU32%map_cols;
    row=RandU32%map_rows;
    l=clus_lo+RandU16%(clus_hi-clus_lo+1);
    for (j=0;j<l;j++) {
      terrain[row][col]=color;
      Toward(&row,&col,RandU16%6);
      col=ClampI64(col,0,map_cols-1);
      row=ClampI64(row,0,map_rows-1);
    }
  }
}

U0 MakeRivers()
{
  I64 i,row,col,direction;
  for (i=0;i<4;i++) {
    row=RandU32%map_rows;
    col=RandU32%map_cols;
    direction=RandU16%6;
    while (TRUE) {
      rivers[row][col]=TRUE;
      Toward(&row,&col,direction);
      if (!(0<=row<map_rows && 0<=col<map_cols))
	break;
      if (!(RandU16&3))
	direction=(direction+(7-RandU16%3))%6;
    }
  }
}

U0 MakeRoads()
{
  I64 i,row,col,direction;
  for (i=0;i<5;i++) {
    row=RandU32%map_rows;
    col=RandU32%map_cols;
    direction=RandU16%6;
    while (TRUE) {
      roads[row][col]=TRUE;
      Toward(&row,&col,direction);
      if (!(0<=row<map_rows && 0<=col<map_cols))
	break;
      if (!(RandU16%3))
	direction=(direction+(7-RandU16%3))%6;
    }
  }
}

U0 DrawTerrain()
{
  I64 i,j;
  F64 x,y;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      map_dc->color=terrain[j][i];
      RowCol2XY(&x,&y,j,i);
      GrFloodFill(map_dc,x,y);
    }
}

U0 DrawRivers()
{
  I64 i,j,k,r,c;
  F64 x1,y1,x2,y2;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      if (rivers[j][i]) {
	RowCol2XY(&x1,&y1,j,i);
	for (k=0;k<6;k++) {
	  r=j;c=i;
	  Toward(&r,&c,k);
	  if (0<=r<map_rows && 0<=c<map_cols &&
		rivers[r][c]) {
	    RowCol2XY(&x2,&y2,r,c);
	    map_dc->color=LTBLUE;
	    map_dc->thick=4;
	    GrLine3(map_dc,x1,y1,0,x2,y2,0);
	    map_dc->color=BLUE;
	    map_dc->thick=2;
	    GrLine3(map_dc,x1,y1,0,x2,y2,0);
	  }
	}
      }
    }
}

U0 DrawRoads()
{
  I64 i,j,k,r,c;
  F64 x1,y1,x2,y2;
  map_dc->color=RED;
  map_dc->thick=3;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      if (roads[j][i]) {
	RowCol2XY(&x1,&y1,j,i);
	for (k=0;k<6;k++) {
	  r=j;c=i;
	  Toward(&r,&c,k);
	  if (0<=r<map_rows && 0<=c<map_cols &&
		roads[r][c]) {
	    RowCol2XY(&x2,&y2,r,c);
	    GrLine3(map_dc,x1,y1,0,x2,y2,0);
	  }
	}
      }
    }
}

U0 DrawDots()
{
  I64 i,j;
  F64 x,y;
  map_dc->color=BLACK;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      RowCol2XY(&x,&y,j,i);
      GrPlot(map_dc,x,y);
    }
}

U0 HexCentersCalc()
{
  I64 i,j;
  F64 x,y;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      x=(2*HEX_SIDE+2*DCOS)*i+HEX_SIDE/2+DCOS;
      if (j&1)
	x+=HEX_SIDE+DCOS;
      y=DSIN*(j+1);
      hex_centers[j][i].x=x;
      hex_centers[j][i].y=y;
    }
}

U0 InitMap()
{
  HexCentersCalc;
  DrawHexes;
  MemSet(terrain,PLAINS,sizeof(terrain));
  MemSet(roads,FALSE,sizeof(roads));
  MemSet(rivers,FALSE,sizeof(rivers));
  MemSet(vis_map,FALSE,sizeof(vis_map));
  MakeTerrain(MOUNTAINS,0.03*map_cols*map_cols,5,35);
  MakeTerrain(TREES,0.03*map_cols*map_cols,5,35);
  DrawTerrain;
  MakeRivers;
  DrawRivers;
  MakeRoads;
  DrawRoads;
  DrawDots;
}

U0 InitUnits()
{
  I64 i,j,row,col,type;
  Unit *tmpu;
  MemSet(units,0,sizeof(units));
  alive_cnt[0]=alive_cnt[1]=UNITS_NUM;
  for (j=0;j<2;j++)
    for (i=0;i<alive_cnt[j];i++) {
      tmpu=&units[j][i];
      tmpu->player=j;
      tmpu->num=i;
      tmpu->life=100;
      tmpu->facing=RandU16%6;
      if (!j) {
	if (i>=UNITS_NUM/2) {
	  if (i>=15*UNITS_NUM/16)
	    type=UT_ARTILLERY;
	  else
	    type=UT_INFANTRY;
	} else {
	  if (i>=UNITS_NUM/4)
	    type=UT_MD_TANK;
	  else
	    type=UT_LT_TANK;
	}
      } else {
	if (i>=UNITS_NUM/2) {
	  if (i>=15*UNITS_NUM/16)
	    type=UT_ARTILLERY;
	  else
	    type=UT_INFANTRY;
	} else {
	  if (i>=UNITS_NUM/4)
	    type=UT_MD_TANK;
	  else
	    type=UT_LT_TANK;
	}
      }
      tmpu->type=type;
      switch (type) {
	case UT_INFANTRY:
	  tmpu->infantry=TRUE;
	  tmpu->indirect_fire=FALSE;
	  tmpu->armor	 =0;
	  tmpu->armored_attack	=15;
	  tmpu->unarmored_attack=180;
	  tmpu->accuracy=45;
	  tmpu->range	 =5;
	  tmpu->movement=4;
	  tmpu->img	 =$IB,"<1>",BI=1$;
	  break;
	case UT_ARTILLERY:
	  tmpu->infantry=TRUE;
	  tmpu->indirect_fire=TRUE;
	  tmpu->armor	 =0;
	  tmpu->armored_attack	=60;
	  tmpu->unarmored_attack=180;
	  tmpu->accuracy=25;
	  tmpu->range	 =20;
	  tmpu->movement=2;
	  tmpu->img	 =$IB,"<2>",BI=2$;
	  break;
	case UT_LT_TANK:
	  tmpu->infantry=FALSE;
	  tmpu->indirect_fire=FALSE;
	  tmpu->armor	 =30;
	  tmpu->armored_attack	=40;
	  tmpu->unarmored_attack=60;
	  tmpu->accuracy=25;
	  tmpu->range	 =8;
	  tmpu->movement=24;
	  tmpu->img	 =$IB,"<3>",BI=3$;
	  break;
	case UT_MD_TANK:
	  tmpu->infantry=FALSE;
	  tmpu->indirect_fire=FALSE;
	  tmpu->armor	 =60;
	  tmpu->armored_attack	=60;
	  tmpu->unarmored_attack=80;
	  tmpu->accuracy=25;
	  tmpu->range	 =12;
	  tmpu->movement=16;
	  tmpu->img	 =$IB,"<4>",BI=4$;
	  break;
      }
      do {
	row=RandU32%map_rows;
	col=RandU32%(map_cols/3);
	if (j)
	  col+=2*map_cols/3;
      } while (UnitFind(row,col));
      tmpu->row=row;
      tmpu->col=col;
      LBts(&tmpu->vis[cur_player],0);
    }
}

U0 ViewPlayerSet(I8 p)
{
  CMenuEntry *tmpse;
  view_player=p;
  if (tmpse=MenuEntryFind(Fs->cur_menu,"View/Player1"))
    tmpse->checked= view_player==0;
  if (tmpse=MenuEntryFind(Fs->cur_menu,"View/Player2"))
    tmpse->checked= view_player==1;
}

U0 Init()
{
  DocClear;
  "GameSeed(0x%X)\n",Seed(PopUpGetI64("GameSeed(0x%X):",Seed));
  moving_unit=NULL;
  InitMap;
  ViewPlayerSet(cur_player=0);
  enemy_player=1;
  if (map_width<GR_WIDTH) {
    x0=(MAP_WIDTH-map_width)>>1;
    y0=(MAP_HEIGHT-map_height)>>1+FONT_HEIGHT;
  } else {
    x0=0;
    y0=FONT_HEIGHT;
  }
  InitUnits;
  QueInit(&indirect_head);
  turn=0;
  fire_radius=0;
  show_vis_row=-1;
  show_vis_col=-1;
  *msg_buf=0;
  msg_off_timeout=0;
  phase=PHASE_END;
}

U0 CleanUp()
{
  QueDel(&indirect_head,TRUE);
}

U0 PlayerPick(U8 *dirname,I64 player)
{
  I64 i=0;
  U8 *st;
  CDirEntry *tmpde,*tmpde1,*tmpde2;
  CDoc *doc=DocNew;
  Bool *old_silent=Silent;
  st=MStrPrint("%s/*.HC*",dirname);
  tmpde=FilesFind(st);
  Free(st);
  tmpde2=FilesFind("~/ToTheFront/*.HC*");
  tmpde1=tmpde;
  Silent(old_silent);

  DocPrint(doc,"Player %d Type\n\n$$LTBLUE$$",player+1);
  while (tmpde1) {
    if (!(i++&3))
      DocPrint(doc,"\n");
    st=StrNew(tmpde1->name);
    FileExtRem(st);
    tmpde1->user_data=DocPrint(doc,"$$MU-UL,\"%-10ts\",LE=%d$$ ",st,tmpde1);
    Free(st);
    tmpde1=tmpde1->next;
  }
  tmpde1=tmpde2;
  while (tmpde1) {
    if (!(i++&3))
      DocPrint(doc,"\n");
    st=StrNew(tmpde1->name);
    FileExtRem(st);
    tmpde1->user_data=DocPrint(doc,"$$MU-UL,\"%-10ts\",LE=%d$$ ",st,tmpde1);
    Free(st);
    tmpde1=tmpde1->next;
  }
  DocPrint(doc,"\n\n\n$$FG$$Create your own AI in ~/ToTheFront.");
  while ((tmpde1=PopUpMenu(doc))<=0);
  ExeFile(tmpde1->full_name);
  DocDel(doc);
  DirTreeDel(tmpde);
  DirTreeDel(tmpde2);
  ExePrint("player_indirect[%d]=&PlayerIndirect;"
	"player_move[%d]=&PlayerMove;"
	"player_direct[%d]=&PlayerDirect;",
	player,player,player);
}
        L         ÿÿÿÿüÿÿÿ          úÿÿÿ      üÿÿÿúÿÿÿþÿÿÿ   
           øÿÿÿ              üÿÿÿüÿÿÿ          
ûÿÿÿûÿÿÿ   ûÿÿÿ
   ûÿÿÿ      
      ûÿÿÿ   
ûÿÿÿ   ûÿÿÿûÿÿÿ
üÿÿÿüÿÿÿ      
   üÿÿÿüÿÿÿ           G      ÿÿÿÿýÿÿÿ       
   üÿÿÿ      
þÿÿÿüÿÿÿþÿÿÿ   
           úÿÿÿ        Ï      þÿÿÿþÿÿÿ      
ÿÿÿÿýÿÿÿ   ýÿÿÿ
ÿÿÿÿüÿÿÿ   üÿÿÿ 
    üÿÿÿ       
ýÿÿÿ         
þÿÿÿûÿÿÿ   ûÿÿÿ
   ýÿÿÿ   üÿÿÿ
þÿÿÿýÿÿÿþÿÿÿüÿÿÿ
ýÿÿÿýÿÿÿýÿÿÿ   
   ýÿÿÿ      
            
ÿÿÿÿ   ÿÿÿÿ    