U0 PlayerIndirect()
{
  Unit *tmpu=NULL;
  I64 i,remaining=0,msg_code,arg1,arg2;
  F64 target_x,target_y;
  ViewPlayerSet(cur_player);
  for (i=0;i<UNITS_NUM;i++) {
    tmpu=&units[cur_player][i];
    if (tmpu->life>0 && tmpu->indirect_fire)
      remaining++;
  }
  while (remaining) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw('GameOver',TRUE);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|
	  1<<MSG_MS_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	CharDo(arg1);
	break;
      case MSG_MS_L_DOWN:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  if (tmpu=UnitFind(cursor_row,cursor_col)) {
	    if (tmpu->player==enemy_player || tmpu->fired ||
		  !tmpu->indirect_fire)
	      tmpu=NULL;
	    else {
	      RowCol2XY(&fire_radius_x,&fire_radius_y,tmpu->row,tmpu->col);
	      fire_radius=tmpu->range*2*HEX_RADIUS;
	    }
	  }
	}
	break;
      case MSG_MS_L_UP:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  RowCol2XY(&target_x,&target_y,cursor_row,cursor_col);
	  if (!tmpu)
	    Beep;
	  else {
	    if (Sqrt(Sqr(fire_radius_x-target_x)+Sqr(fire_radius_y-target_y))>
		  fire_radius)
	      Beep;
	    else {
	      IndirectAdd(tmpu,cursor_row,cursor_col);
	      remaining--;
	    }
	  }
	}
	tmpu=NULL;
	fire_radius=0;
	break;
      case MSG_MS_R_UP:
	if (CursorInWin(Fs,arg1,arg2))
	  throw('PhaseOvr',TRUE);
	break;
    }
  }
  throw('PhaseOvr',TRUE);
}

U0 PlayerMove()
{
  Unit *tmpu=NULL;
  I64 msg_code,arg1,arg2;
  ViewPlayerSet(cur_player);
  while (TRUE) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw('GameOver',TRUE);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|
	  1<<MSG_MS_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	CharDo(arg1);
	break;
      case MSG_MS_L_DOWN:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  if (tmpu=UnitFind(cursor_row,cursor_col)) {
	    if (tmpu->player==enemy_player || !tmpu->remaining_movement)
	      tmpu=NULL;
	  }
	}
	break;
      case MSG_MS_L_UP:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  if (!tmpu)
	    Beep;
	  else {
	    UnitMove(tmpu,arg1,arg2);
	    break;
	  }
	}
	tmpu=NULL;
	break;
      case MSG_MS_R_UP:
	if (CursorInWin(Fs,arg1,arg2))
	  throw('PhaseOvr',TRUE);
	break;
    }
  }
}

U0 PlayerDirect()
{
  Unit *tmpu=NULL,*target;
  I64 msg_code,arg1,arg2;
  ViewPlayerSet(cur_player);
  while (TRUE) {
    if (!alive_cnt[0] || !alive_cnt[1])
      throw('GameOver',TRUE);
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|
	  1<<MSG_MS_R_UP);
    switch (msg_code) {
      case MSG_KEY_DOWN:
	CharDo(arg1);
	break;
      case MSG_MS_L_DOWN:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  if (tmpu=UnitFind(cursor_row,cursor_col)) {
	    if (tmpu->player==enemy_player || tmpu->fired ||
		  tmpu->indirect_fire)
	      tmpu=NULL;
	    else {
	      VRSetUp(cur_player);
	      RowCol2XY(&fire_radius_x,&fire_radius_y,tmpu->row,tmpu->col);
	      fire_radius=tmpu->range*2*HEX_RADIUS;
	      VisRecalc(VR_ONE_FRIENDLY_UNIT,tmpu);
	    }
	  }
	}
	break;
      case MSG_MS_L_UP:
	if (CursorInWin(Fs,arg1,arg2)) {
	  arg1-=x0; arg2-=y0;
	  CursorUpdate(Fs,arg1,arg2);
	  target=UnitFind(cursor_row,cursor_col);
	  if (!tmpu)
	    Beep;
	  else {
	    if (!target || target->player!=enemy_player ||
		  !Bt(&target->vis,0))
	      Beep;
	    else
	      UnitDirectFire(tmpu,target);
	    VisRecalc(VR_UPDATE_FRIENDLY_UNIT,tmpu);
	  }
	}
	tmpu=NULL;
	fire_radius=0;
	break;
      case MSG_MS_R_UP:
	if (CursorInWin(Fs,arg1,arg2))
	  throw('PhaseOvr',TRUE);
	break;
    }
  }
}
