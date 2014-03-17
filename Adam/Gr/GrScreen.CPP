#help_index "Graphics/Screen"

U0 GrUpdateWins()
{ //Only called by WinMgr
  I64 i;
  CTask *task,*task1;
  CD3I64 saved_scroll;
  CDC *dc;
  Preempt(OFF);
  try {
    refresh_ode_time=0;
    if (Bt(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0))
      WinZBufUpdate;
    task1=task=sys_winmgr_task;
    do {
      if (!TaskValidate(task))
	break;
      sys_task_being_screen_updated=task;
      Preempt(ON);
      ODEsUpdate(task);
      Preempt(OFF);
      if (!TaskValidate(task))
	break;
      if (Bt(&task->display_flags,DISPLAYf_SHOW) &&
	  Bt(gr_win_uncovered_bitmap,task->win_z_num))
	try {
	  if (!Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	    DrawTaskBorder(task);
	  WinTextFill(task->text_attr<<8,task);
	  Preempt(ON);
	  if (task==sys_winmgr_task) {
	    if (fp_wall_paper)
	      (*fp_wall_paper)(task);
	  } else {
	    if (!(task->win_inhibit&WIF_DOC))
	      DocUpdateTaskDocs(task);
	  }
	  if (TaskValidate(task)) {
	    if (task->draw_it) {
	      dc=DCAlias(gr_dc2,task);
	      (*task->draw_it)(task,dc);
	      DCDel(dc);
	    }
	    if (TaskValidate(task)) {
	      Preempt(OFF);
	      WinScrollNull(task,&saved_scroll);
	      DrawCtrls(task);
	      WinScrollRestore(task,&saved_scroll);
	    }
	  }
	  Preempt(OFF);
	} catch {
	  if (task!=Fs && TaskValidate(task)) {
	    LBtr(&task->display_flags,DISPLAYf_SHOW);
	    "Exception in WinMgr: DrawIt\n";
	    PutExcept;
	    Sleep(3000);
	    VGAFlush;
	  }
	}
      if (!TaskValidate(task))
	break;
      task=task->next_task;
    } while (task!=task1);

    for (i=1;i<mp_cnt;i++) {
      task1=task=cpu_structs[i].seth_task;
      do {
	if (!TaskValidate(task))
	  break;
	try {
	  sys_task_being_screen_updated=task;
	  Preempt(ON);
	  ODEsUpdate(task);
	  Preempt(OFF);
	} catch {
	  LBts(&task->win_inhibit,WIf_ODE);
	  "Exception in WinMgr: ODE\n";
	  PutExcept;
	  Sleep(3000);
	  VGAFlush;
	}
	if (!TaskValidate(task))
	  break;
	task=task->next_task;
      } while (task!=task1);
    }
  } catch {
    PutExcept(FALSE);
    Dbg("Exception in WinMgr");
  }

  last_refresh_ode_time=refresh_ode_time;
  ode_allocated_factor=LowPass1(0.1,ode_allocated_factor,
      Clamp(Gs->idle_factor-0.1,0.2,0.8),1/win_actual_refresh);

  sys_task_being_screen_updated=NULL;
  gr_refresh_time_stamps[gr_refresh_ts_ptr++&(GR_REFRESH_LOG_CNT-1)]=tS;
  win_actual_refresh=(GR_REFRESH_LOG_CNT-1)/
      (gr_refresh_time_stamps[(gr_refresh_ts_ptr-1)&(GR_REFRESH_LOG_CNT-1)]-
      gr_refresh_time_stamps[gr_refresh_ts_ptr&(GR_REFRESH_LOG_CNT-1)]);
}

U0 GrFixZoomScale()
{
  gr_screen_zoom=ClampI64(gr_screen_zoom,1,GR_MAX_SCREEN_ZOOM);
  if (gr_screen_zoom==1) {
    grsx=0;
    grsy=0;
  } else {
    grsx=ClampI64(grsx,0,GR_WIDTH-GR_WIDTH/gr_screen_zoom)&~7;
    grsy=ClampI64(grsy,0,GR_HEIGHT-GR_HEIGHT/gr_screen_zoom);
  }
}

public U0 GrScaleZoom(F64 scale)
{
  Bool old_preempt=Preempt(OFF);
  F64 s=gr_screen_zoom;
  gr_screen_zoom=gr_screen_zoom*scale;
  GrFixZoomScale;
  s/=gr_screen_zoom;
  ipx_scale*=s;
  ipy_scale*=s;
  ipz_scale*=s;
  ipx_offset=ipx-(ipx-ipx_offset)*s;
  ipy_offset=ipy-(ipy-ipy_offset)*s;
  ipz_offset=ipz-(ipz-ipz_offset)*s;
  grsx=ipx-gr_zoomed_dc->width >>1/gr_screen_zoom;
  grsy=ipy-gr_zoomed_dc->height>>1/gr_screen_zoom;
  GrFixZoomScale;
  Preempt(old_preempt);
}

U0 GrZoomInScreen()
{
  GrFixZoomScale;
  I64 plane,row,col,k,l,
    d2=gr_zoomed_dc->width>>3/gr_screen_zoom,
    d4=gr_zoomed_dc->width_internal>>3,
    d5=d4-d2*gr_screen_zoom,
    d3=gr_zoomed_dc->height/gr_screen_zoom,
    d6=(gr_zoomed_dc->height-d3)*gr_dc1->width_internal>>3,
    d7=gr_zoomed_dc->height%gr_screen_zoom*d4;
  U8 *src,*src2,*dst,*src3,*map=gr_screen_zoom_tables[gr_screen_zoom];

  src=gr_dc1->body+grsx>>3+grsy*gr_dc1->width_internal>>3;
  dst=gr_zoomed_dc->body;
  for (plane=1;plane<0x10;plane<<=1) {
    row=d3;
    while (row--) {
      k=gr_screen_zoom;
      while (k--) {
	src2=src;
	col=d2;
	while (col--) {
	  src3=&map[*src2++];
	  l=gr_screen_zoom;
	  while (l--) {
	    *dst++=*src3;
	    src3+=256;
	  }
	}
	l=d5;
	while (l--)
	  *dst++=0;
      }
      src+=d4;
    }
    l=d7;
    while (l--)
      *dst++=0;
    src+=d6;
  }
}

U0 GrUpdateBackgroundOfText()
{
  I64 reg RSI *dst=gr_dc2->body,reg R13 c,row,col,
      num_rows=TEXT_ROWS,num_cols=TEXT_COLS,i,j,cur_ch,
      reg R12 w1=gr_dc2->width_internal,w2=-7*w1+8,w3=7*w1,w4=0;
  U32 *src=gr_text_base;
  Bool blink_flag=Blink;
  U8 *dst2=dst;

  if (gr_pan_text_x) {
    gr_pan_text_x=ClampI64(gr_pan_text_x,-7,7);
    j=AbsI64(gr_pan_text_x)/FONT_WIDTH+1;
    num_cols-=j;
    if (gr_pan_text_x<0) {
      src+=j;
      i=FONT_WIDTH*j+gr_pan_text_x;
    } else
      i=gr_pan_text_x;
    dst2=dst(U8 *)+i;
    w4=j;
    w3+=j*FONT_WIDTH;

    j*=FONT_WIDTH;
    dst(U8 *)=gr_dc2->body;
    for (row=num_rows*FONT_HEIGHT;row--;) {
      for (col=i;col--;)
	*dst(U8 *)++=0;
      dst(U8 *)+=w1-i-j;
      for (col=j;col--;)
	*dst(U8 *)++=0;
    }
  }
  dst=dst2;

  if (gr_pan_text_y) {
    gr_pan_text_y=ClampI64(gr_pan_text_y,-7,7);
    j=AbsI64(gr_pan_text_y)/FONT_HEIGHT+1;
    num_rows-=j;
    if (gr_pan_text_y<0) {
      src+=w1/FONT_WIDTH*j;
      i=w1*(FONT_HEIGHT*j+gr_pan_text_y);
    } else
      i=w1*gr_pan_text_y;
    dst2=dst(U8 *)+i;

    j*=w1*FONT_HEIGHT;
    dst(U8 *)=gr_dc2->body;
    for (row=i;row--;)
      *dst(U8 *)++=0;
    dst(U8 *)=gr_dc2->body+TEXT_ROWS*TEXT_COLS*FONT_HEIGHT*FONT_WIDTH-j;
    for (row=j;row--;)
      *dst(U8 *)++=0;
  }
  dst=dst2;

  for (row=num_rows;row--;) {
    for (col=num_cols;col--;) {
      cur_ch=*src++;
      if (cur_ch & (DOCET_SELECTED | DOCET_INVERT | DOCET_BLINK)) {
	if (cur_ch & DOCET_SELECTED)
	  cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
	if (cur_ch & DOCET_INVERT)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
	if (cur_ch & DOCET_BLINK && blink_flag)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
      }
      c=gr_1_to_8_colors[cur_ch.u8[1]>>4];
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
	ADD	RSI,R12
	MOV	U64 [RSI],R13
      dst(U8 *)+=w2;
    }
    src+=w4;
    dst(U8 *)+=w3;
  }
}

//$AN,"ExtScreenCodes",A="ExtScreenCodes"$
//   Bits 0-7	ASCII (Screen Code)
//   Bits 8-11	Foreground color
//   Bits 12-15 Background color
//   Bits 16-20 Signed X pos shift val
//   Bits 21-25 Signed Y pos shift val
//   Bit  28	Blink
//   Bit  29	Inverted (Swap foreground and background)
//   Bit  30	Selected (XOR colors with FF)
//   Bit  31	Underline
U0 GrUpdateForegroundOfText()
{
  U32 *src=gr_text_base;
  I64 i,j,cur_ch,*dst=gr_dc2->body,
      w1=gr_dc2->width_internal,w2=7*w1,w4=0,
      num_rows=TEXT_ROWS,num_cols=TEXT_COLS,row,col;
  U8 *dst_start=gr_dc2->body,*dst_end=dst_start+w1*gr_dc2->height-7*w1-8;
  Bool blink_flag=Blink;

  if (gr_pan_text_x) {
    gr_pan_text_x=ClampI64(gr_pan_text_x,-7,7);
    j=AbsI64(gr_pan_text_x)/FONT_WIDTH+1;
    num_cols-=j;
    if (gr_pan_text_x<0) {
      src+=j;
      dst(U8 *)+=FONT_WIDTH*j;
    }
    w4=j;
    w2+=j*FONT_WIDTH;
  }

  if (gr_pan_text_y) {
    gr_pan_text_y=ClampI64(gr_pan_text_y,-7,7);
    j=AbsI64(gr_pan_text_y)/FONT_HEIGHT+1;
    num_rows-=j;
    if (gr_pan_text_y<0) {
      src+=w1/FONT_WIDTH*j;
      dst(U8 *)+=w1*FONT_HEIGHT*j;
    }
  }

  for (row=num_rows;row--;) {
    for (col=num_cols;col--;) {
      cur_ch=*src++;
      if (cur_ch & (DOCET_UNDERLINE | DOCET_SELECTED | DOCET_INVERT | DOCET_BLINK)) {
	if (cur_ch & DOCET_SELECTED)
	  cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
	if (cur_ch & DOCET_INVERT)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
	if (cur_ch & DOCET_BLINK && blink_flag)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
      }
      if (i=cur_ch.u16[1]&0x3FF+gr_pan_text_x+gr_pan_text_y<<5) {
	j=i&0x1F;
	if (j&0x10) j|=~0x1F;
	i>>=5;
	if (i&0x10) i|=~0x1F;
	i=w1*i+j;
	if (dst_start<=dst(U8 *)+i<dst_end)
	  GrRopEquU8NoClipping(cur_ch&(DOCET_UNDERLINE+0xFFF),dst(U8 *)+i,w1);
      } else
	GrRopEquU8NoClipping(cur_ch&(DOCET_UNDERLINE+0xFFF),dst,w1);
      dst(U8 *)+=8;
    }
    src+=w4;
    dst(U8 *)+=w2;
  }
}

U0 DCBlotColor8(CDC *dc,CDC *img)
{
  U8  reg *src=img->body,*b0=dc->body;
  I64 reg j,reg k,reg d0=img->width_internal*img->height;
  for (k=0;k<d0;k++) {
    j=*src++;
    if (j!=COLOR_TRANSPARENT)
      *b0++=j;
    else
      b0++;
  }
}

U0 GrUpdateTextModeText()
{
  U32 *src=gr_text_base;
  I64 cur_ch,i,d0=TEXT_COLS*TEXT_ROWS;
  U16 *dst=VGA_TEXT_MEM_BASE,*dst2=gr_vga_text_cache;
  Bool blink_flag=Blink;
  if (LBtr(&sys_semas[SYS_SEMA_FLUSH_VGA_IMAGE],0)) {
    i=d0;
    while (i--) {
      cur_ch=*src++;
      if (cur_ch & DOCET_SELECTED)
	cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
      if (cur_ch & DOCET_INVERT)
	cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
      if (cur_ch & DOCET_BLINK)
	if (blink_flag)
	  cur_ch.u8[1]= cur_ch.u8[1]<<4+ cur_ch.u8[1]>>4;
      cur_ch&=0x7FFF;
      *dst++=*dst2++=cur_ch;
    }
  } else {
    i=d0;
    while (i--) {
      cur_ch=*src++;
      if (cur_ch & DOCET_SELECTED)
	cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
      if (cur_ch & DOCET_INVERT)
	cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
      if (cur_ch & DOCET_BLINK)
	if (blink_flag)
	  cur_ch.u8[1]= cur_ch.u8[1]<<4+ cur_ch.u8[1]>>4;
      cur_ch&=0x7FFF;
      if (*dst2!=cur_ch)
	*dst++=*dst2++=cur_ch;
      else {
	dst++;
	dst2++;
      }
    }
  }
}

U0 GrUpdateVGAGraphics()
{
  I64 row,plane,d2=gr_zoomed_dc->width_internal>>5;
  U32 *src,*dst,*dst2;
  //Update Graphic Card
  if (gr_screen_zoom==1)
    src=gr_dc1->body;
  else {
    GrZoomInScreen;
    src=gr_zoomed_dc->body;
  }
  dst2=gr_screen_image->body;
  if (LBtr(&sys_semas[SYS_SEMA_FLUSH_VGA_IMAGE],0)) {
    for (plane=1;plane<0x10;plane<<=1) {
      OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
      OutU8(VGA_SC_DATA,plane);
      dst=VGA_GRAPHIC_MEM_BASE;
      row=gr_zoomed_dc->height;
      while (row--)
	GrUpdateLine32FlushCache(&dst,&src,d2,&dst2);
    }
  } else {
    for (plane=1;plane<0x10;plane<<=1) {
      OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
      OutU8(VGA_SC_DATA,plane);
      dst=VGA_GRAPHIC_MEM_BASE;
      row=gr_zoomed_dc->height;
      while (row--)
	GrUpdateLine32(&dst,&src,d2,&dst2);
    }
  }
}

U0 GrUpdateScreen()
{
  CDC *dc;
  Preempt(ON);
  if (text_mode) //If 640x480 VGA is not available, falls back to text mode.
    GrUpdateWins;
  else {
    GrUpdateBackgroundOfText;
    GrUpdateForegroundOfText;
    GrUpdateWins;
    DCBlotColor8(gr_dc2,gr_dc);
  }
  Preempt(OFF);

  dc=DCAlias(gr_dc2,Fs);
  dc->flags|=DCF_ON_TOP;
  if (fp_final_screen_update)
    (*fp_final_screen_update)(dc);
  DCDel(dc);

  if (text_mode)
    GrUpdateTextModeText;
  else {
    DCBlotColor4(gr_dc1->body,gr_dc2->body,gr_dc_cache->body,
      gr_dc2->height*gr_dc2->width_internal>>3);
    GrUpdateVGAGraphics;
  }
}
