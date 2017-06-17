U0 InitDefines()
{
  I64 w=GR_WIDTH,h=GR_HEIGHT-FONT_HEIGHT,cols,rows,
	size=PopUpRangeI64(1,9,,"Size\n"),rad=16-size;
  if (size<0) throw;
  DefinePrint("UNITS_NUM","%d",4096/SqrI64(rad)&~1);
  DefinePrint("CIRCLE_RAD","%d",rad);
  DefinePrint("MOVE_CIRCLES","%d",40/rad);

  cols=(w-8-rad)/(2*rad)&~1-1;
  DefinePrint("BORDER_X","%d",(w+rad-cols*2*rad)/2);

  rows=(h-8)/(2*rad)&~1;
  DefinePrint("BORDER_Y","%d",(h-rows*2*rad)/2);

} InitDefines;

class Unit
{
  I64 num,x,y,player,link,color;
  Bool alive,king;
} u[UNITS_NUM];

CDC *map_dc;
I64 cur_player,num_alive[2],start_x,start_y,end_x,end_y;
Bool show_start;

U0 S2Circle(I64 *_x,I64 *_y)
{
  I64 i,j;
  j=(*_y-BORDER_Y)/(CIRCLE_RAD*2);
  if (j&1)
    i=(*_x-CIRCLE_RAD-BORDER_X)/(CIRCLE_RAD*2);
  else
    i=(*_x-BORDER_X)/(CIRCLE_RAD*2);
  *_y=j;
  *_x=i;
}

U0 Circle2S(I64 *_x,I64 *_y)
{
  I64 j=*_y,i=*_x;
  *_y=j*CIRCLE_RAD*2+CIRCLE_RAD+BORDER_Y;
  *_x=i*CIRCLE_RAD*2+CIRCLE_RAD+BORDER_X;
  if (j&1) *_x+=CIRCLE_RAD;
}

U0 S2W(I64 *_x,I64 *_y)
{
  S2Circle(_x,_y);
  Circle2S(_x,_y);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i;
  map_dc->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,0,0,map_dc);

  if (cur_player==0) {
    dc->color=LTCYAN;
    GrFloodFill(dc,0,0);
    dc->color=CYAN;
  } else {
    dc->color=LTPURPLE;
    GrFloodFill(dc,0,0);
    dc->color=PURPLE;
  }
  GrPrint(dc,2,2,"Player %d",cur_player+1);

  for (i=0;i<UNITS_NUM;i++)
    if (u[i].alive) {
      dc->color=u[i].color;
      GrFloodFill(dc,u[i].x,u[i].y);
    }
  for (i=0;i<UNITS_NUM;i++)
    if (u[i].alive && !u[i].king) {
      dc->color=BLACK;
      GrLine(dc,u[i].x,u[i].y,u[u[i].link].x,u[u[i].link].y);
      if (Blink) {
	dc->color=BLUE;
	dc->thick=6;
	GrPlot3(dc,(u[i].x+u[u[i].link].x)>>1,(u[i].y+u[u[i].link].y)>>1,0);
      }
    }
  if (show_start) {
    dc->color=LTRED;
    GrLine(dc,start_x-4,start_y-4,start_x+4,start_y+4);
    GrLine(dc,start_x-4,start_y+4,start_x+4,start_y-4);
    GrLine(dc,start_x,start_y,end_x,end_y);
  }
  if ((num_alive[0]==1 || num_alive[1]==1) && Blink(4)) {
    dc->color=BLACK;
    GrPrint(dc,(task->pix_width-FONT_WIDTH*9)>>1,
	  (task->pix_height-FONT_HEIGHT)>>1,"Game Over");
  }
}

Unit *UnitFind(I64 x,I64 y,I64 player)
{
  I64 i,dd,best_dd=I64_MAX;
  Unit *res=NULL;
  for (i=0;i<UNITS_NUM;i++) {
    if (u[i].player==player && u[i].alive) {
      dd=SqrI64(u[i].x-x)+SqrI64(u[i].y-y);
      if (dd<best_dd) {
	best_dd=dd;
	res=&u[i];
      }
    }
  }
  return res;
}

U0 KillsChk(I64 x1,I64 y1,I64 player)
{
  I64 i,j,x2,y2,dd;
  Bool found;
  for (i=0;i<UNITS_NUM;i++) {
    if (u[i].player!=player && u[i].alive && !u[i].king) {
      x2=(u[i].x+u[u[i].link].x)>>1;
      y2=(u[i].y+u[u[i].link].y)>>1;
      dd=SqrI64(x2-x1)+SqrI64(y2-y1);
      if (dd<=(CIRCLE_RAD+2)*(CIRCLE_RAD+2)) {
	u[i].alive=FALSE;
	Snd(62);Sleep(100);Snd;Sleep(25);
	num_alive[u[i].player]--;
	do {
	  found=FALSE;
	  for (j=0;j<UNITS_NUM;j++)
	    if (u[j].alive && u[j].player!=player && !u[j].king &&
		  !u[u[j].link].alive) {
	      found=TRUE;
	      u[j].alive=FALSE;
	      Snd(62);Sleep(100);Snd;Sleep(25);
	      num_alive[u[j].player]--;
	    }
	} while (found);
      }
    }
  }
}

Bool UnitMove(Unit *tmpu,I64 x2,I64 y2)
{
  I64 i,r,c,r2,c2,x=start_x,y=start_y;
  S2W(&x2,&y2);
  c2=x2; r2=y2;
  S2Circle(&c2,&r2);
  for (i=0;i<MOVE_CIRCLES+1;i++) {
    c=x; r=y;
    S2Circle(&c,&r);
    if (c==c2 && r==r2) {
      end_x=tmpu->x=x2;
      end_y=tmpu->y=y2;
      return TRUE;
    }
    if (r2!=r) {
      if (r&1) {
	if (c<c2) c++;
      } else {
	if (c>c2) c--;
      }
    }
    if (r2>r) {
      x=c; y=++r;
      Circle2S(&x,&y);
    } else if (r2<r) {
      x=c; y=--r;
      Circle2S(&x,&y);
    } else if (c2>c) {
      x=++c; y=r;
      Circle2S(&x,&y);
    } else if (c2<c) {
      x=--c; y=r;
      Circle2S(&x,&y);
    }
  }
  return FALSE;
}

CDC *DrawHexMap(I64 *_w,I64 *_h)
{
  CDC *dc=DCNew(*_w,*_h);
  I64 i,j,x,y;
  *_w=(dc->width-BORDER_X*2-CIRCLE_RAD)/(CIRCLE_RAD*2);
  *_h=(dc->height-BORDER_Y*2)/(CIRCLE_RAD*2);
  DCFill(dc,WHITE);
  dc->color=LTGRAY;
  for (j=0;j<*_h;j++) {
    for (i=0;i<*_w;i++) {
      x=i*CIRCLE_RAD*2+BORDER_X+CIRCLE_RAD;
      y=j*CIRCLE_RAD*2+BORDER_Y+CIRCLE_RAD;
      if (j&1) x+=CIRCLE_RAD;
      S2W(&x,&y);
      GrCircle(dc,x,y,CIRCLE_RAD);
    }
  }
  *_w*=CIRCLE_RAD*2;
  *_h*=CIRCLE_RAD*2;
  return dc;
}

U0 Init()
{
  I64 i,j,j1,j2,dd,best_dd,best,w=Fs->pix_width,h=Fs->pix_height;

  map_dc=DrawHexMap(&w,&h);
  cur_player=0;

  for (i=0;i<UNITS_NUM/2;i++) {
    u[i].num=i;
    u[i+UNITS_NUM/2].num=i+UNITS_NUM/2;

ti_restart:
    u[i].y=FloorI64(RandU32%h,CIRCLE_RAD*2)+CIRCLE_RAD;
    u[i+UNITS_NUM/2].y=h-1-u[i].y;
    j1=u[i].y/(CIRCLE_RAD*2);
    j2=u[i+UNITS_NUM/2].y/(CIRCLE_RAD*2);
    u[i].y+=BORDER_Y;
    u[i+UNITS_NUM/2].y+=BORDER_Y;

    if (!i) {
      u[i].x=CIRCLE_RAD;
      if (j1&1)
	goto ti_restart;
    } else
      u[i].x=FloorI64(RandU32%((w-CIRCLE_RAD*2*2)/2),CIRCLE_RAD*2)+CIRCLE_RAD;
    u[i+UNITS_NUM/2].x=w-1-u[i].x;

    if (j1&1)
      u[i].x+=CIRCLE_RAD;
    if (j2&1)
      u[i+UNITS_NUM/2].x+=CIRCLE_RAD;
    u[i].x+=BORDER_X;
    u[i+UNITS_NUM/2].x+=BORDER_X;

    S2W(&u[i].x,&u[i].y);
    S2W(&u[i+UNITS_NUM/2].x,&u[i+UNITS_NUM/2].y);

    u[i].player=0;
    u[i+UNITS_NUM/2].player=1;

    u[i].alive=TRUE;
    u[i+UNITS_NUM/2].alive=TRUE;

    if (!i) {
      u[i].color=LTCYAN;
      u[i+UNITS_NUM/2].color=LTPURPLE;
      u[i].king=TRUE;
      u[i+UNITS_NUM/2].king=TRUE;
    } else {
      u[i].color=CYAN;
      u[i+UNITS_NUM/2].color=PURPLE;
      u[i].king=FALSE;
      u[i+UNITS_NUM/2].king=FALSE;
    }
  }
  for (i=0;i<UNITS_NUM/2;i++) {
    if (!u[i].king) {
      best_dd=I64_MAX;
      for (j=0;j<UNITS_NUM/2;j++) {
	if (i!=j) {
	  dd=SqrI64(u[i].x-u[j].x)+SqrI64(u[i].y-u[j].y);
	  if ((u[j].x<u[i].x || u[j].king) && dd<best_dd ) {
	    best_dd=dd;
	    best=j;
	  }
	}
      }
      u[i].link=best;
      u[i+UNITS_NUM/2].link=best+UNITS_NUM/2;
    }
  }
  num_alive[0]=UNITS_NUM/2;
  num_alive[1]=UNITS_NUM/2;
}

U0 CleanUp()
{
  DCDel(map_dc);
}

U0 TreeCheckers()
{
  I64 msg_code,arg1,arg2,ch,sc;
  Unit *tmpu;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"}"
	);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  PopUpOk("Step on the link mid points.\n");
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU;
  try {
cn_start:
    Init;
    tmpu=NULL;
    show_start=FALSE;
    Fs->draw_it=&DrawIt;
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
      switch (msg_code) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '\n':
	      CleanUp;
	      goto cn_start;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto cn_done;
	  }
	  break;
	case MSG_MS_L_DOWN:
	  if (num_alive[0]>1 && num_alive[1]>1) {
	    tmpu=UnitFind(arg1,arg2,cur_player);
	    end_x=start_x=tmpu->x;
	    end_y=start_y=tmpu->y;
	    show_start=TRUE;
	  }
	  break;
	case MSG_MS_MOVE:
	  if (tmpu)
	    UnitMove(tmpu,arg1,arg2);
	  break;
	case MSG_MS_L_UP:
	  UnitMove(tmpu,arg1,arg2);
	  KillsChk(tmpu->x,tmpu->y,cur_player);
	  show_start=FALSE;
	  tmpu=NULL;
	  cur_player=1-cur_player;
	  break;
      }
    }
cn_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
}

TreeCheckers;
