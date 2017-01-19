#help_index "Graphics/Scrn"

U0 GrSetUpTables()
{
  CDC *dc;
  I64 i,j,k,l,m,x,y,rr;
  U8 *dst;

  k=0;
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      if (Bt(&i,j))
	gr.to_8_bits(U8 *)[k++]=0xFF;
      else
	gr.to_8_bits(U8 *)[k++]=0x00;
  k=0;
  for (i=0;i<256;i++)
    for (j=0;j<8;j++)
      gr.to_8_colors(U8 *)[k++]=i;

  for (i=0;i<GR_PEN_BRUSHES_NUM;i++) {
    k=i+1;

    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr) {
	  if (x-1-i>>1<gr.circle_lo[i][i-y])
	    gr.circle_lo[i][i-y]=x-1-i>>1;
	  if (x-1-i>>1>gr.circle_hi[i][i-y])
	    gr.circle_hi[i][i-y]=x-1-i>>1;
	}

    dc=DCNew(i,i);
    gr.pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr.collision_pen_brushes[i]=dc;
    dc->color=COLOR_INVALID;//Want color that never occurs.
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr.even_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (!(((x-1)^(y-1))&1) && SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(i,i);
    gr.odd_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (((x-1)^(y-1))&1 && SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);
  }
  if (!Bt(&sys_run_level,RLf_VGA)) { //if text mode
    MemSet(text.vga_text_alias,0,TEXT_ROWS*TEXT_COLS*sizeof(U16));
    MemSet(gr.vga_text_cache,0,TEXT_ROWS*TEXT_COLS*sizeof(U16));
  } else {
    OutU8(VGAP_IDX,VGAR_MAP_MASK);
    OutU8(VGAP_DATA,0x0F);
//Virtual Box crashes on the following for some reason.
    //	  MemSet(text.vga_alias,0,GR_HEIGHT*GR_WIDTH>>3);
    MemSet(gr.scrn_image->body,0,GR_WIDTH*GR_HEIGHT>>1);
  }
  for (i=1;i<=GR_SCRN_ZOOM_MAX;i++) {
    dst=gr.scrn_zoom_tables[i]=MAlloc(256*i);
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
}

#help_index "Graphics/Scrn;Windows"

U0 WinZBufFill(CTask *task)
{//Not public
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
      MemSetU16(gr.win_z_buf(U8 *)+(y*TEXT_COLS+l)*sizeof(U16),
	    task->win_z_num,w);
}

public U0 WinZBufUpdate()
{//Might have to call if doing graphics outside winmgr callback routines.
//Call it if, for example, when a pop-up window closes and you need to refresh
  //before graphics.
  I64 i,z=1;
  U16 *ptr;
  CTask *task,*task1;
  if (gr.win_z_buf) {
    LBtr(&sys_semas[SEMA_UPDATE_WIN_Z_BUF],0);
    task1=task=sys_winmgr_task;
    do {
      if (!TaskValidate(task)) break;
      if (Bt(&task->display_flags,DISPLAYf_SHOW)) {
	task->win_z_num=z++;
	WinZBufFill(task);
      }
      task=task->next_task;
    } while (task!=task1 && z<0x10000);

    MemSet(gr.win_uncovered_bitmap,0,(gr.highest_uncovered+7)>>3+1);
    gr.highest_uncovered=z-1;
    for (ptr=gr.win_z_buf,i=TEXT_ROWS*TEXT_COLS;i;i--)
      Bts(gr.win_uncovered_bitmap,*ptr++);
  }
}

#help_index "Graphics"
U0 GrInit2()
{
  MemSet(&gr,0,sizeof(CGrGlbls));
  gr.sprite_hash=HashTableNew(512);
  HashDefineLstAdd("ST_SPRITE_ELEM_CODES",SPHT_ELEM_CODE,gr.sprite_hash);
  gr.scrn_zoom=1;

  PaletteSetStd;
  fp_set_std_palette=&PaletteSetStd;
  GrPaletteIndicesSet;

  gr.to_8_bits	=MAlloc(256*sizeof(I64));
  gr.to_8_colors=MAlloc(256*sizeof(I64));

  gr.text_base=CAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U32));
  gr.vga_text_cache=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));
  gr.win_uncovered_bitmap=CAlloc(65536/8);
  gr.highest_uncovered=0;
  gr.win_z_buf=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));

  gr.dc2=DCNew(GR_WIDTH,GR_HEIGHT);
  gr.dc2->flags|=DCF_SCRN_BITMAP;
  gr.dc_cache=DCNew(GR_WIDTH,GR_HEIGHT);

  gr.dc=DCNew(GR_WIDTH,GR_HEIGHT);
  gr.dc->flags|=DCF_SCRN_BITMAP|DCF_ON_TOP;
  DCFill;

  gr.dc1=DCNew(GR_WIDTH,GR_HEIGHT);
  gr.dc1->flags|=DCF_SCRN_BITMAP;

  gr.scrn_image=DCNew(GR_WIDTH,GR_HEIGHT); //4-bit
  gr.zoomed_dc	 =DCNew(GR_WIDTH,GR_HEIGHT); //4-bit
  gr.zoomed_dc->flags|=DCF_SCRN_BITMAP;
}

GrInit2;
