U0 TanksPlot()
{
  Unit *tempu=NULL;
  I64 msg_code,p1,p2;
  F64 target_x,target_y;
  SetViewPlayer(cur_player);
  while (TRUE) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw (EXCEPT_LOCAL,T_GAME_OVER);
    msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|
	1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|
	1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	HandleChar(p1);
	break;
      case MSG_IP_L_DOWN:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  if (tempu=FindUnit(cursor_row,cursor_col)) {
	    if (tempu->player==enemy_player || tempu->fired || !tempu->indirect_fire)
	      tempu=NULL;
	    else {
	      RowCol2XY(&fire_radius_x,&fire_radius_y,tempu->row,tempu->col);
	      fire_radius=tempu->range*2*HEX_RADIUS;
	    }
	  }
	}
	break;
      case MSG_IP_L_UP:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  RowCol2XY(&target_x,&target_y,cursor_row,cursor_col);
	  if (!tempu)
	    Beep;
	  else {
	    if (Sqrt(Sqr(fire_radius_x-target_x)+Sqr(fire_radius_y-target_y))>fire_radius)
	      Beep;
	    else
	      PlotShot(tempu,cursor_row,cursor_col);
	  }
	}
	tempu=NULL;
	fire_radius=0;
	break;
      case MSG_IP_R_UP:
	if (CursorInWindow(Fs,p1,p2))
	  throw (EXCEPT_LOCAL,T_TURN_OVER);
	break;
    }
  }
}

U0 TanksMove()
{
  Unit *tempu=NULL,*target;
  I64 msg_code,p1,p2;
  SetViewPlayer(cur_player);
  while (TRUE) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw (EXCEPT_LOCAL,T_GAME_OVER);
    msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|
	1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|
	1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	HandleChar(p1);
	break;
      case MSG_IP_L_DOWN:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  if (tempu=FindUnit(cursor_row,cursor_col)) {
	    if (tempu->player==enemy_player || !tempu->remaining_movement)
	      tempu=NULL;
	  }
	}
	break;
      case MSG_IP_L_UP:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  target=FindUnit(cursor_row,cursor_col);
	  if (!tempu)
	    Beep;
	  else {
	    if (target)
	      Beep;
	    else
	      MoveUnit(tempu,p1+Fs->horz_scroll.pos,
		  p2+Fs->vert_scroll.pos);
	    break;
	  }
	}
	tempu=NULL;
	break;
      case MSG_IP_R_UP:
	if (CursorInWindow(Fs,p1,p2))
	  throw (EXCEPT_LOCAL,T_TURN_OVER);
	break;
    }
  }
}

U0 TanksFire()
{
  Unit *tempu=NULL,*target;
  I64 msg_code,p1,p2;
  SetViewPlayer(cur_player);
  while (TRUE) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw (EXCEPT_LOCAL,T_GAME_OVER);
    msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|
	1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|
	1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	HandleChar(p1);
	break;
      case MSG_IP_L_DOWN:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  if (tempu=FindUnit(cursor_row,cursor_col)) {
	    if (tempu->player==enemy_player || tempu->fired || tempu->indirect_fire)
	      tempu=NULL;
	    else {
	      RVSetUp(cur_player);
	      RowCol2XY(&fire_radius_x,&fire_radius_y,tempu->row,tempu->col);
	      fire_radius=tempu->range*2*HEX_RADIUS;
	      RecalcVisible(RV_ONE_FRIENDLY_UNIT,tempu);
	    }
	  }
	}
	break;
      case MSG_IP_L_UP:
	if (CursorInWindow(Fs,p1,p2)) {
	  UpdateCursor(Fs,p1,p2);
	  target=FindUnit(cursor_row,cursor_col);
	  if (!tempu)
	    Beep;
	  else {
	    if (!target || target->player!=enemy_player || !Bt(&target->visible,0))
	      Beep;
	    else
	      FireShot(tempu,target);
	    RecalcVisible(RV_UPDATE_FRIENDLY_UNIT,tempu);
	  }
	}
	tempu=NULL;
	fire_radius=0;
	break;
      case MSG_IP_R_UP:
	if (CursorInWindow(Fs,p1,p2))
	  throw (EXCEPT_LOCAL,T_TURN_OVER);
	break;
    }
  }
}
