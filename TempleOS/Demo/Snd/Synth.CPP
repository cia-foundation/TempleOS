#define M_WAVEFORM	0
#define M_FREQUENCY	1
#define M_ENVELOPE	2

#define M_NUM_MODES	3

I64 mode;
Bool my_ip_down;
I64 last_x,last_y;

Bool synth_done;

U0 (*fp_task_end_cb)();
U0 (*fp_old_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;

#define NUM_BUF	8
SND_OUT_CONTAINER my_buf[SND_BUF_LEN*NUM_BUF];

#define WF_BUF_LEN	2048
#define WF_Y_SCALE	2048.0
I64 waveform_buf[WF_BUF_LEN];

#define FQ_BUF_LEN	2048
#define FQ_Y_SCALE	2048.0
I64 frequency_buf[FQ_BUF_LEN];
F64 fq_scale;

#define EL_BUF_LEN	2048
#define EL_Y_SCALE	2048.0
I64 envelope_buf[EL_BUF_LEN];
F64 el_scale;

U0 SynthFillBuf(SND_OUT_CONTAINER *buf,I64 buf_num)
{
  I64 i,j,l,k0,k1,k2,k3;
  F64 d;
  static F64 k=0;
  if (synth_done) return;
  j=0;
  k0=buf_num*(SND_BUF_LEN/SND_OCHANNELS);
  while (j<SND_BUF_LEN) {
    k1=WF_BUF_LEN*k/(SND_BUF_LEN/SND_OCHANNELS);
    k2=fq_scale*FQ_BUF_LEN*k0/(SND_BUF_LEN/SND_OCHANNELS);
    k3=el_scale*EL_BUF_LEN*k0/(SND_BUF_LEN/SND_OCHANNELS);

    d=envelope_buf[k3%EL_BUF_LEN]/EL_Y_SCALE;
    d*=snd_vol;
    d*=MAX_I32*waveform_buf[k1%WF_BUF_LEN]/WF_Y_SCALE;
    i=ToI64(d)&0xFFFFFF00;

    for (l=0;l<SND_OCHANNELS;l++)
      buf[j++]=i;

    d=frequency_buf[k2%FQ_BUF_LEN]/FQ_Y_SCALE; // [0.0,1.0]
    d=(SND_BUF_LEN/SND_OCHANNELS)*Exp(5.0*d+4)/SND_SAMPLE_RATE;
    k+=d;
    k0++;
  }
}

U0 SynthTaskEndCB()
{
  synth_done=TRUE;
  HDStop(TRUE,FALSE);
  fp_snd_fill_buf=fp_old_fill_buf;
  if (fp_task_end_cb)
    (*fp_task_end_cb)();
  else
    Exit;
}

Bool WFPlot(U8 *,I64 x,I64 y,I64)
{
  if (0<=x<=WF_BUF_LEN)
    waveform_buf[x]=y;
  return TRUE;
}

Bool FQPlot(U8 *,I64 x,I64 y,I64)
{
  if (0<=x<=FQ_BUF_LEN)
    frequency_buf[x]=AbsI64(y);
  return TRUE;
}

Bool ELPlot(U8 *,I64 x,I64 y,I64)
{
  if (0<=x<=EL_BUF_LEN)
    envelope_buf[x]=AbsI64(y);
  return TRUE;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,x1,y1,x2,y2,
      cy=task->win_pixel_height>>1,
      h_max=task->win_pixel_width-1;

  if (!win_show_menu) {
    if (ip_lb) {
      if (my_ip_down)
	switch (mode) {
	  case M_WAVEFORM:
	    Line(NULL,
	    (last_x-task->win_pixel_left-task->win_scroll_x)*WF_BUF_LEN/h_max,
		WF_Y_SCALE*(cy-last_y+task->win_pixel_top+task->win_scroll_y)/cy,0,
	    (  ipx-task->win_pixel_left-task->win_scroll_x)*WF_BUF_LEN/h_max,
		WF_Y_SCALE*(cy-ipy+task->win_pixel_top+task->win_scroll_y)/cy,0,
		&WFPlot);
	    break;
	  case M_FREQUENCY:
	    Line(NULL,
	    (last_x-task->win_pixel_left-task->win_scroll_x)*FQ_BUF_LEN/h_max,
		FQ_Y_SCALE*(cy-last_y+task->win_pixel_top+task->win_scroll_y)/cy,0,
	    (  ipx-task->win_pixel_left-task->win_scroll_x)*FQ_BUF_LEN/h_max,
		FQ_Y_SCALE*(cy-ipy+task->win_pixel_top+task->win_scroll_y)/cy,0,
		&FQPlot);
	    break;
	  case M_ENVELOPE:
	    Line(NULL,
	    (last_x-task->win_pixel_left-task->win_scroll_x)*EL_BUF_LEN/h_max,
		EL_Y_SCALE*(cy-last_y+task->win_pixel_top+task->win_scroll_y)/cy,0,
	    (  ipx-task->win_pixel_left-task->win_scroll_x)*EL_BUF_LEN/h_max,
		EL_Y_SCALE*(cy-ipy+task->win_pixel_top+task->win_scroll_y)/cy,0,
		&ELPlot);
	    break;
	}
      my_ip_down=TRUE;
      last_x=ipx;
      last_y=ipy;
    } else
      my_ip_down=FALSE;
  }

  x2=-1;
  if (mode==M_WAVEFORM && Blink) {
    dc->color=ROPF_DITHER+WHITE<<16+RED;
    GrPutS(dc,0,cy-FONT_HEIGHT,"Set Waveform.");
  } else
    dc->color=RED;
  for (i=0;i<WF_BUF_LEN;i++) {
    x1=i*task->win_pixel_width/WF_BUF_LEN;
    y1=cy-cy*waveform_buf[i]/WF_Y_SCALE;
    if (x2>=0)
      GrLine(dc,x2,y2,x1,y1);
    x2=x1;
    y2=y1;
  }

  x2=-1;
  if (mode==M_FREQUENCY && Blink) {
    dc->color=ROPF_DITHER+WHITE<<16+BLUE;
    GrPutS(dc,0,cy-FONT_HEIGHT,"Set Frequency.");
  } else
    dc->color=BLUE;
  for (i=0;i<FQ_BUF_LEN;i++) {
    x1=i*task->win_pixel_width/FQ_BUF_LEN;
    y1=cy-cy*frequency_buf[i]/FQ_Y_SCALE;
    if (x2>=0)
      GrLine(dc,x2,y2,x1,y1);
    GrLine(dc,x2,cy-(y2-cy),x1,cy-(y1-cy));
    x2=x1;
    y2=y1;
  }

  x2=-1;
  if (mode==M_ENVELOPE && Blink) {
    dc->color=ROPF_DITHER+WHITE<<16+GREEN;
    GrPutS(dc,0,cy-FONT_HEIGHT,"Set Envelope.");
  } else
    dc->color=GREEN;
  for (i=0;i<EL_BUF_LEN;i++) {
    x1=i*task->win_pixel_width/EL_BUF_LEN;
    y1=cy-cy*envelope_buf[i]/EL_Y_SCALE;
    if (x2>=0)
      GrLine(dc,x2,y2,x1,y1);
    GrLine(dc,x2,cy-(y2-cy),x1,cy-(y1-cy));
    x2=x1;
    y2=y1;
  }

  dc->color=BLACK;
  GrLine(dc,0,cy,task->win_pixel_width,cy);
}

U0 SetMenu()
{
  MenuEntryFind(Fs->cur_menu,"Mode/Waveform")->checked=mode==M_WAVEFORM;
  MenuEntryFind(Fs->cur_menu,"Mode/Frequency")->checked=mode==M_FREQUENCY;
  MenuEntryFind(Fs->cur_menu,"Mode/Envelope")->checked=mode==M_ENVELOPE;
}

U0 Init()
{
  my_ip_down=FALSE;
  mode=M_WAVEFORM;
  MemSetU64(&waveform_buf[0]	       ,-0.5 *WF_Y_SCALE,WF_BUF_LEN/2);
  MemSetU64(&waveform_buf[WF_BUF_LEN/2] , 0.5 *WF_Y_SCALE,WF_BUF_LEN/2);
  MemSetU64(frequency_buf	       , 0.25*FQ_Y_SCALE,FQ_BUF_LEN);
  MemSetU64(envelope_buf	       , 0.75*EL_Y_SCALE,EL_BUF_LEN);
  fq_scale=0.02;
  el_scale=0.02;
  SetMenu;
}

U0 Synth()
{
  I64 p1,p2;
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Preempt(OFF);
  MenuPush(
  "File {"
  "  New(,'\n');"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Mode {"
  "  Waveform(,'1');"
  "  Frequency(,'2');"
  "  Envelope(,'3');"
  "}"
  );
  WordStat;
  WinMax;
  WinBorder;
  Fs->win_inhibit=WIF_ALL-WIF_MENU;

  Init;

  synth_done=FALSE;
  fp_task_end_cb=Fs->task_end_cb;
  Fs->task_end_cb=&SynthTaskEndCB; //Catch <CTRL-ALT-X> or Kill() task
  fp_old_fill_buf=fp_snd_fill_buf;
  fp_snd_fill_buf=&SynthFillBuf;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      switch (GetMsg(&p1,&p2,1<<MSG_KEY_DOWN+1<<MSG_IP_R_DOWN)) {
	case MSG_KEY_DOWN:
	  switch (p1) {
	    case '1':
	    case '2':
	    case '3':
	      mode=p1-'1';
	      SetMenu;
	      break;
	    case '\n':
	      Init;
	      break;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      goto sy_done;
	  }
	  break;
	case MSG_IP_R_DOWN:
	  if (++mode==M_NUM_MODES)
	    mode=0;
	  break;
      }
    }
sy_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch { //Catch <CTRL-ALT-C>, but pass it on to next higher handler.
    fp_snd_fill_buf=fp_old_fill_buf;
    Fs->task_end_cb=fp_task_end_cb;
  }
  synth_done=TRUE;
  fp_snd_fill_buf=fp_old_fill_buf;
  Fs->task_end_cb=fp_task_end_cb;
  SettingsPop;
}

Synth;
