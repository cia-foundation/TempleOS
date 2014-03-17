#help_index "Graphics/Screen"
U0 GrSetUpTables()
{
  CDC *dc;
  I64 i,j,k,l,m,x,y,rr;
  U8 *dst;

  k=0;
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      if (Bt(&i,j))
	gr_1_to_8_bits(U8 *)[k++]=0xFF;
      else
	gr_1_to_8_bits(U8 *)[k++]=0x00;
  k=0;
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      gr_1_to_8_colors(U8 *)[k++]=i;

  for (i=0;i<GR_NUM_PEN_BRUSHES;i++) {
    k=i+1;
    dc=DCNew(i,i);
    gr_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr_collision_pen_brushes[i]=dc;
    dc->color=COLOR_INVALID;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr_even_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (!(((x-1)^(y-1))&1) && SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr_odd_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (((x-1)^(y-1))&1 && SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);
  }
  if (text_mode) {
    MemSet(VGA_TEXT_MEM_BASE,0,TEXT_ROWS*TEXT_COLS*sizeof(U16));
    MemSet(gr_vga_text_cache,0,TEXT_ROWS*TEXT_COLS*sizeof(U16));
  } else {
    OutU8(VGA_SC_INDEX,VGA_MAP_MASK);
    OutU8(VGA_SC_DATA,0x0F);
//Virtual Box crashes on the following for some reason.
//    MemSet(VGA_GRAPHIC_MEM_BASE,0,GR_HEIGHT*GR_WIDTH>>3);
    MemSet(gr_screen_image->body,0,GR_WIDTH*GR_HEIGHT>>1);
  }
  for (i=1;i<=GR_MAX_SCREEN_ZOOM;i++) {
    dst=gr_screen_zoom_tables[i]=MAlloc(256*i);
    for (j=0;j<256;j++) {
      m=0;
      for (k=0;k<8;k++) {
	if (Bt(&j,k)) {
	  for (l=0;l<i;l++)
	    Bts(&m,l+k*i);
	}
      }
      for (l=0;l<i;l++)
	dst[j+l*256]=m.u8[l];
    }
  }
  for (i=0;i<GR_REFRESH_LOG_CNT;i++)
    gr_refresh_time_stamps[i]=tS;
}

#help_index "Graphics/Screen;Windows"

U0 WinZBufFill(CTask *task)
{ //Not public
  I64 y,t,b,l,r,w;
  if (!Bt(&task->display_flags,DISPLAYf_NO_BORDER)) {
    if (task->win_top-1>0)
      t=task->win_top-1;
    else
      t=0;
    if (task->win_bottom+1<TEXT_ROWS)
      b=task->win_bottom+1;
    else
      b=TEXT_ROWS-1;
    if (task->win_left-1>0)
      l=task->win_left-1;
    else
      l=0;
    if (task->win_right+1<TEXT_COLS)
      r=task->win_right+1;
    else
      r=TEXT_COLS-1;
  } else {
    if (task->win_top>0)
      t=task->win_top;
    else
      t=0;
    if (task->win_bottom<TEXT_ROWS)
      b=task->win_bottom;
    else
      b=TEXT_ROWS-1;
    if (task->win_left>0)
      l=task->win_left;
    else
      l=0;
    if (task->win_right<TEXT_COLS)
      r=task->win_right;
    else
      r=TEXT_COLS-1;
  }
  t=ClampI64(t,0,TEXT_ROWS-1);
  b=ClampI64(b,t,TEXT_ROWS-1);
  l=ClampI64(l,0,TEXT_COLS-1);
  r=ClampI64(r,l,TEXT_COLS-1);
  if (w=r-l+1)
    for (y=t;y<=b;y++)
      MemSetU16(gr_win_z_buf(U8 *)+(y*TEXT_COLS+l)*sizeof(U16),
	task->win_z_num,w);
}

public U0 WinZBufUpdate()
{ //You might have to call this if you are doing graphics outside
  //the window mgr callback routines.  Call it if, for example,
  //a pop-up window closes and you need to refresh before graphics.
  I64 i,z=1;
  U16 *ptr;
  CTask *task,*task1;
  Bool old_preempt=Preempt(OFF);
  if (gr_win_z_buf) {
    LBtr(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0);
    task1=task=sys_winmgr_task;
    do {
      if (!TaskValidate(task)) break;
      if (Bt(&task->display_flags,DISPLAYf_SHOW)) {
	task->win_z_num=z++;
	WinZBufFill(task);
      }
      task=task->next_task;
    } while (task!=task1 && z<0x10000);

    MemSet(gr_win_uncovered_bitmap,0,(gr_highest_uncovered+7)>>3+1);
    gr_highest_uncovered=z-1;
    for (ptr=gr_win_z_buf,i=TEXT_ROWS*TEXT_COLS;i;i--)
      Bts(gr_win_uncovered_bitmap,*ptr++);
  }
  Preempt(old_preempt);
}

#help_index "Graphics/Char;Char"
public U0 WinTextFill(I64 d,CTask *task=NULL)
{//You probably don't want this.
  I64 y,w,t,b,l,r;
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  if (task->win_top>0)
    t=task->win_top;
  else
    t=0;
  if (task->win_bottom<TEXT_ROWS)
    b=task->win_bottom;
  else
    b=TEXT_ROWS-1;
  if (task->win_left>0)
    l=task->win_left;
  else
    l=0;
  if (task->win_right<TEXT_COLS)
    r=task->win_right;
  else
    r=TEXT_COLS-1;
  if (w=r-l+1)
    for (y=t;y<=b;y++)
      MemSetU32(gr_text_base(U8 *)+(y*TEXT_COLS+l)*sizeof(U32),
	    d,w);
}

U0 GrInit2()
{
  SetStdPalette;
  fp_set_std_palette=&SetStdPalette;
  SetVGAPalette4Indices;

  gr_1_to_8_bits  =MAlloc(256*sizeof(I64));
  gr_1_to_8_colors=MAlloc(256*sizeof(I64));

  gr_text_base=CAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U32));
  gr_vga_text_cache=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));
  gr_win_uncovered_bitmap=CAlloc(65536/8);
  gr_highest_uncovered=0;
  gr_win_z_buf=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));

  gr_dc2=DCNew(GR_WIDTH,GR_HEIGHT);
  gr_dc2->flags|=DCF_SCREEN_BITMAP;
  gr_dc_cache=DCNew(GR_WIDTH,GR_HEIGHT);

  gr_dc=DCNew(GR_WIDTH,GR_HEIGHT);
  gr_dc->flags|=DCF_SCREEN_BITMAP|DCF_ON_TOP;
  DCFill;

  gr_dc1=DCNew(GR_WIDTH,GR_HEIGHT);
  gr_dc1->flags|=DCF_SCREEN_BITMAP;

  gr_screen_image=DCNew(GR_WIDTH,GR_HEIGHT); //4-bit
  gr_zoomed_dc	 =DCNew(GR_WIDTH,GR_HEIGHT); //4-bit
  gr_zoomed_dc->flags|=DCF_SCREEN_BITMAP;
}

GrInit2;
