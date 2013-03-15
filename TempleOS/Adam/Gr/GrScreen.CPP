#help_index "Graphics/DCs;Graphics/Screen"
public CDC *gr_screen_image; //This is read only

#help_index "Graphics/DCs"
public CDC *gr_dc=NULL, //This is updated every refresh
	    *gr_dc_cache,
	    *gr_dc1=NULL,
	    *gr_dc2=NULL;
CDC *gr_zoomed_dc;
U16 *gr_win_uncovered_bitmap=NULL,gr_highest_uncovered;

U16 *gr_vga_text_cache;

public U0 DCFill(CDC *dc=NULL,CColorROPU32 val=COLOR_TRANSPARENT)
{
  if (!dc) dc=gr_dc2;
  nobound_switch (dc->type) {
    case DCT_COLOR4:
      MemSet(dc->body,val,(dc->width_internal*dc->height)>>1);
      break;
    case DCT_COLOR8:
      MemSet(dc->body,val,dc->width_internal*dc->height);
      break;
  }
}

public U0 DCClear(CDC *dc=NULL)
{
  if (!dc) dc=gr_dc2;
  DCFill(dc,0);
}

#help_index "Graphics/Screen"
public U0 (*fp_final_screen_update)(CDC *dc)=NULL;
public U0 (*fp_wall_paper)(CTask *task)=NULL;

#help_index "Graphics/Screen;Windows"



U0 WinZBufFill(CTask *task)
{ //Not public
  I64 y,t,dc,l,r,w;
  if (!Bt(&task->display_flags,DISPLAYf_NO_BORDER)) {
    t= task->win_top-1>0
       ? task->win_top-1 : 0,
    dc= task->win_bottom+1<TEXT_ROWS
       ? task->win_bottom+1 : TEXT_ROWS-1,
    l= task->win_left-1>0
       ? task->win_left-1 : 0,
    r= task->win_right+1<TEXT_COLS
       ? task->win_right+1 : TEXT_COLS-1;
  } else {
    t= task->win_top>0
       ? task->win_top : 0,
    dc= task->win_bottom<TEXT_ROWS
       ? task->win_bottom : TEXT_ROWS-1,
    l= task->win_left>0
       ? task->win_left : 0,
    r= task->win_right<TEXT_COLS
       ? task->win_right : TEXT_COLS-1;
  }
  t=LimitI64(t,0,TEXT_ROWS-1);
  dc=LimitI64(dc,t,TEXT_ROWS-1);
  l=LimitI64(l,0,TEXT_COLS-1);
  r=LimitI64(r,l,TEXT_COLS-1);
  if (w=r-l+1)
    for (y=t;y<=dc;y++)
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
{
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  I64 y,w,
      t= task->win_top>0
       ? task->win_top : 0,
      dc= task->win_bottom<TEXT_ROWS
       ? task->win_bottom : TEXT_ROWS-1,
      l= task->win_left>0
       ? task->win_left : 0,
      r= task->win_right<TEXT_COLS
       ? task->win_right : TEXT_COLS-1;
  if (w=r-l+1)
    for (y=t;y<=dc;y++)
      MemSetU32(gr_text_base(U8 *)+(y*TEXT_COLS+l)*sizeof(U32),
	d,w);
}

public U0 WinTextClear(CTask *task=NULL)
{
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  WinTextFill(task->text_attr<<8,task);
}

public U0 SetWinBkColor(I64 c,CTask *task=NULL)
{
  if (!task) task=Fs;
  if (!TaskValidate(task)) return;
  task->text_attr=task->text_attr&0x0F | c<<4;
  WinTextFill(c<<12,task);
}

#help_index "Graphics/Color"
public I32 gr_rainbow_10[10]=
{
BLACK,BROWN,RED,LTRED,YELLOW,
GREEN,BLUE,PURPLE,LTGRAY,WHITE
};

DefineLstLoad("ST_RAINBOW_10",
"BLACK\0BROWN\0RED\0LTRED\0YELLOW\0"
"GREEN\0BLUE\0PURPLE\0LTGRAY\0WHITE\0");

U0 SetVGAPalette4Indices()
{//4-bit (16 color)
/*There is a level of indirection we wish to identity map.
This sets the 16-colors to use
the first 16 DAC rgb entries, so
we never need to worry about indexes,
just DAC palette settings.
*/
  I64 i;
  if (text_mode) return;
  PushFD;
  Cli;
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  InU8(VGA_INPUT_STAT); //Rsts attr index/data
  for (i=0;i<16;i++) {
    OutU8(VGA_ATTR_INDEX,i);
    OutU8(VGA_ATTR_DATA_WRITE,i);
  }
  OutU8(VGA_ATTR_INDEX,0x20);
  OutU8(VGA_ATTR_DATA_WRITE,0); //Dummy write
  InU8(VGA_INPUT_STAT); //Rsts attr index/data
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  PopFD;
}

public U0 SetVGAPaletteColor(I64 color_num,CBGR bgr)
{ //6-bits per r,dc and g
  if (text_mode) return;
  PushFD;
  Cli;
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  OutU8(VGA_PALETTE_MASK,0xFF);
  OutU8(VGA_REG_WRITE,color_num);
  OutU8(VGA_PALETTE_DATA,bgr.r>>2);
  OutU8(VGA_PALETTE_DATA,bgr.g>>2);
  OutU8(VGA_PALETTE_DATA,bgr.b>>2);
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  PopFD;
}

public CBGR GetVGAPaletteColor(I64 color_num)
{ //6-bits per r,dc and g
  CBGR result=0;
  if (text_mode) return 0;
  PushFD;
  Cli;
  while (LBts(&sys_semas[SYS_SEMA_VGA],0))
    Yield;
  OutU8(VGA_PALETTE_MASK,0xFF);
  OutU8(VGA_REG_READ,color_num);
  result.r=InU8(VGA_PALETTE_DATA)<<2;
  result.g=InU8(VGA_PALETTE_DATA)<<2;
  result.b=InU8(VGA_PALETTE_DATA)<<2;
  LBtr(&sys_semas[SYS_SEMA_VGA],0);
  PopFD;
  return result;
}

public CBGR vga_palette4_std[16]={
0x000000,0x0000AA,0x00AA00,0x00AAAA,
0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
0x555555,0x5555FF,0x55FF55,0x55FFFF,
0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF};

public CBGR vga_palette4_gray[16]={
0x000000,0x111111,0x222222,0x333333,
0x444444,0x555555,0x666666,0x777777,
0x888888,0x999999,0xAAAAAA,0xBBBBBB,
0xCCCCCC,0xDDDDDD,0xEEEEEE,0xFFFFFF};

public U0 GetVGAPalette4(CBGR *bgr)
{//4-bit (16 color)
  I64 i;
  for (i=0;i<16;i++)
    bgr[i]=GetVGAPaletteColor(i);
}

public U0 SetVGAPalette4(CBGR *bgr)
{//4-bit (16 color)
  I64 i;
  for (i=0;i<16;i++)
    SetVGAPaletteColor(i,bgr[i]);
}

public U0 SetStdPalette()
{
  SetVGAPalette4(vga_palette4_std);
}

#help_index "Graphics/Screen"

#define GR_NUM_PEN_BRUSHES 64
CDC	*gr_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_collision_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_even_pen_brushes[GR_NUM_PEN_BRUSHES],
	*gr_odd_pen_brushes[GR_NUM_PEN_BRUSHES];


#define GR_MAX_SCREEN_ZOOM	8
U8 *gr_screen_zoom_tables[GR_MAX_SCREEN_ZOOM+1];
I64 gr_screen_zoom=1,grsx=0,grsy=0;

//When zoomed, this keeps the mouse centered.
public Bool gr_continuous_scroll=FALSE;


#define GR_REFRESH_LOG_CNT	16
F64 gr_refresh_time_stamps[GR_REFRESH_LOG_CNT];
I64 gr_refresh_ts_ptr=0;

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
    dc=DCNew(DCT_COLOR8,i,i);
    gr_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(DCT_COLOR8,i,i);
    gr_collision_pen_brushes[i]=dc;
    dc->color=COLOR_INVALID;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(DCT_COLOR8,i,i);
    gr_even_pen_brushes[i]=dc;
    dc->color=COLOR_MONO;
    rr=k*k;
    for (y=1;y<k;y++)
      for (x=1;x<k;x++)
	if (!(((x-1)^(y-1))&1) && SqrI64(y*2-k)+SqrI64(x*2-k)<rr)
	  GrPlot0(dc,x-1,y-1);

    dc=DCNew(DCT_COLOR8,i,i);
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
    gr_refresh_time_stamps[i]=tT;
}

U0 GrInit()
{
  SetStdPalette;
  fp_set_std_palette=&SetStdPalette;
  SetVGAPalette4Indices;

  gr_1_to_8_bits  =MAlloc(256*sizeof(I64));
  gr_1_to_8_colors=MAlloc(256*sizeof(I64));

  gr_text_base=CAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U32));
  gr_vga_text_cache=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));
  gr_win_z_buf=MAlloc(TEXT_ROWS*TEXT_COLS*sizeof(U16));
  gr_win_uncovered_bitmap=CAlloc(65536/8);
  gr_highest_uncovered=0;

  gr_dc=DCNew(DCT_COLOR8,GR_WIDTH,GR_HEIGHT);
  gr_dc->flags|=DCF_SCREEN_BITMAP;
  gr_dc_cache=DCNew(DCT_COLOR8,GR_WIDTH,GR_HEIGHT);

  gr_dc2=DCNew(DCT_COLOR8,GR_WIDTH,GR_HEIGHT);
  gr_dc2->flags|=DCF_SCREEN_BITMAP;
  DCFill;

  gr_dc1=DCNew(DCT_COLOR4,GR_WIDTH,GR_HEIGHT);
  gr_dc1->flags|=DCF_SCREEN_BITMAP;

  gr_screen_image=DCNew(DCT_COLOR4,GR_WIDTH,GR_HEIGHT);
  gr_zoomed_dc	 =DCNew(DCT_COLOR4,GR_WIDTH,GR_HEIGHT);
  gr_zoomed_dc->flags|=DCF_SCREEN_BITMAP;

  GrSetUpTables;

  WinZBufUpdate;
}

U0 GrUpdateWins()
{ //Only called by WinMgr
  I64 i;
  CTask *task,*task1;
  CD3I64 saved_scroll;
  CDC *dc;
  Bool old_preempt=Preempt(OFF);
  try {
    refresh_ode_time=0;
    if (Bt(&sys_semas[SYS_SEMA_UPDATE_WIN_Z_BUF],0))
      WinZBufUpdate;
    task1=task=sys_winmgr_task;
    do {
      if (!TaskValidate(task))
	break;
      sys_task_being_screen_updated=task;
      Preempt(old_preempt);
      ODEsUpdate(task);
      Preempt(OFF);
      if (!TaskValidate(task))
	break;
      if (Bt(&task->display_flags,DISPLAYf_SHOW) &&
	  Bt(gr_win_uncovered_bitmap,task->win_z_num)) {
	try {
	  if (!Bt(&task->display_flags,DISPLAYf_NO_BORDER))
	    DrawTaskBorder(task);
	  WinTextClear(task);
	  Preempt(old_preempt);
	  if (task==sys_winmgr_task) {
	    if (fp_wall_paper)
	      (*fp_wall_paper)(task);
	  } else {
	    if (!(task->win_inhibit&WIF_DOC))
	      DocUpdateTaskDocs(task);
	  }
	  if (TaskValidate(task)) {
	    if (task->draw_it) {
	      dc=DCAlias(gr_dc,task);
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
	} catch {
	  if (task!=Fs && TaskValidate(task)) {
	    LBtr(&task->display_flags,DISPLAYf_SHOW);
	    "Exception in WinMgr: DrawIt\n";
	    PutExcept;
	    Sleep(3000);
	    VGAFlush;
	    Fs->catch_except=TRUE;
	  }
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
	  Preempt(old_preempt);
	  ODEsUpdate(task);
	  Preempt(OFF);
	} catch {
	  LBts(&task->win_inhibit,WIf_ODE);
	  "Exception in WinMgr: ODE\n";
	  PutExcept;
	  Sleep(3000);
	  VGAFlush;
	  Fs->catch_except=TRUE;
	}
	if (!TaskValidate(task))
	  break;
	task=task->next_task;
      } while (task!=task1);
    }
  } catch {
    PutExcept;
    Dbg("Exception in WinMgr");
  }

  last_refresh_ode_time=refresh_ode_time;
  ode_allocated_factor=LowPass1(0.1,ode_allocated_factor,
      Limit(Gs->idle_factor-0.1,0.2,0.8),1/win_actual_refresh);

  sys_task_being_screen_updated=NULL;
  gr_refresh_time_stamps[gr_refresh_ts_ptr++&(GR_REFRESH_LOG_CNT-1)]=tT;
  win_actual_refresh=(GR_REFRESH_LOG_CNT-1)/
      (gr_refresh_time_stamps[(gr_refresh_ts_ptr-1)&(GR_REFRESH_LOG_CNT-1)]-
      gr_refresh_time_stamps[gr_refresh_ts_ptr&(GR_REFRESH_LOG_CNT-1)]);
  Preempt(old_preempt);
}

U0 GrFixZoomScale()
{
  gr_screen_zoom=LimitI64(gr_screen_zoom,1,GR_MAX_SCREEN_ZOOM);
  if (gr_screen_zoom==1) {
    grsx=0;
    grsy=0;
  } else {
    grsx=LimitI64(grsx,0,GR_WIDTH-GR_WIDTH/gr_screen_zoom)&~7;
    grsy=LimitI64(grsy,0,GR_HEIGHT-GR_HEIGHT/gr_screen_zoom);
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

public U0 GrZoomIn()
{
  GrScaleZoom(2.0);
}

public U0 GrZoomOut()
{
  GrScaleZoom(0.5);
}

U0 CtrlAltZ(I64 sc)
{
  if (sc&SCF_SHIFT)
    GrZoomOut;
  else
    GrZoomIn;
}
SetCtrlAltLetCB('Z',&CtrlAltZ,"Sys/Zoom In or Out");

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
  I64 reg RSI *dst=gr_dc->body,reg R13 c,row,col,,cur_ch,
      reg R12 w1=gr_dc->width_internal,reg R10 w2=-7*w1+8,w3=7*w1,
      reg *my_1_to_8_colors=gr_1_to_8_colors;
  U32 reg *src=gr_text_base;
$HL,1$	Bool blink_flag=Blink;
  nounusedwarn w2;
  for (row=TEXT_ROWS;row--;) {
    for (col=TEXT_COLS;col--;) {
      cur_ch=*src++;
      if (cur_ch & (DOCET_SELECTED | DOCET_INVERT | DOCET_BLINK)) {
	if (cur_ch & DOCET_SELECTED)
	  cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
	if (cur_ch & DOCET_INVERT)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
	if (cur_ch & DOCET_BLINK)
	  if (blink_flag)
	    cur_ch.u8[1]= cur_ch.u8[1]<<4+ cur_ch.u8[1]>>4;
      }
      c=my_1_to_8_colors[cur_ch.u8[1]>>4];
asm {
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
	ADD	RSI,R10
}
    }
    dst(U8 *)+=w3;
  }
}

//$AN,"ExtScreenCodes","ExtScreenCodes"$
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
  I64 i,j,cur_ch,reg *dst=gr_dc->body,
      reg w1=gr_dc->width_internal,w2=7*w1,
      row,col;
  U8 *dst_start=gr_dc->body,*dst_end=dst_start+w1*gr_dc->height-7*w1-8;
  Bool blink_flag=Blink;
  for (row=TEXT_ROWS;row--;) {
    for (col=TEXT_COLS;col--;) {
      cur_ch=*src++;
      if (cur_ch & (DOCET_UNDERLINE | DOCET_SELECTED | DOCET_INVERT | DOCET_BLINK)) {
	if (cur_ch & DOCET_SELECTED)
	  cur_ch.u8[1]=cur_ch.u8[1]^0xFF;
	if (cur_ch & DOCET_INVERT)
	  cur_ch.u8[1]=cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
	if (cur_ch & DOCET_BLINK)
	  if (blink_flag)
	    cur_ch.u8[1]= cur_ch.u8[1]<<4+cur_ch.u8[1]>>4;
      }
      if (i=cur_ch.u16[1]&0x3FF) {
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
  Bool old_preempt=IsPreempt;
  if (!IsSysDeterministic)
    Preempt(ON);

  if (text_mode)
    GrUpdateWins;
  else {
    GrUpdateBackgroundOfText;
    GrUpdateForegroundOfText;
    GrUpdateWins;
    DCBlotColor8(gr_dc,gr_dc2);
  }

  dc=DCAlias(gr_dc,Fs);
  dc->flags|=DCF_ON_TOP;
  if (fp_final_screen_update)
    (*fp_final_screen_update)(dc);
  DCDel(dc);

  if (text_mode)
    GrUpdateTextModeText;
  else {
    DCBlotColor4(gr_dc1->body,gr_dc->body,gr_dc_cache->body,
      gr_dc->height*gr_dc->width_internal>>3);
    GrUpdateVGAGraphics;
  }
  Preempt(old_preempt);
}

#help_index "Graphics"

