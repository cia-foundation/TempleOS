Bool poly_done;
I64 start_buf_num;
U0 (*fp_task_end_cb)();
U0 (*fp_old_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;

U0 PolyFillBuf(SND_OUT_CONTAINER *buf,I64 buf_num)
{ //Gets called by HD Audio task -- $LK,"HDAudioTask","MN:HDAudioTask"$().

//Note: the buffer is in uncached memory
//so you might want to avoid multiple
//accesses due to slow speed.  (If you were
//to add a bunch of waveforms together, you
//might want to do it in a cached buffer
//and copy it once.)

  I64 j,m,r;
  F64 t,d;
  if (poly_done) return;
  t=ToF64((buf_num-start_buf_num)*(SND_BUF_LEN/SND_OCHANNELS))/SND_SAMPLE_RATE;
  j=0;
  while (j<SND_BUF_LEN) {
    d=0.5*Sin(1.0*(2*ã)*t)+0.5;  //Stereo pos from 0.0-1.0
    r=RandI32;
    m=0;
 
    m+=0.2*MAX_I32*Caw(t,0.25)*Sin(220.0*(2*ã)*t);
    m+=0.2*MAX_I32*Caw(t,0.25)*Sin(222.0*(2*ã)*t);
    m+=0.2*MAX_I32*Caw(t,0.25)*Sin(880.0*(2*ã)*t);
    m+=0.2*MAX_I32*Caw(t,0.25)*Sin(884.0*(2*ã)*t);
    m+=0.4*r*Caw(t,.125);
    if (Caw(t-.25,0.5)<=0.05)
      m+=0.3*r*Caw(t,0.05);
    if (t>4.0) {
      if (0.90<Caw(t,1.0))
	m+=0.6*MAX_I32*Caw(t,0.1)*FullCaw(t,0.001);
      if (t>8.0) {
	if (0.70<Caw(t,1.0)<0.90)
	  m+=0.4*MAX_I32*Caw(t-.7,0.5)*FullCaw(t,0.00075);
	if (t>12.0) {
	  if (0.30<Caw(t,1.0)<0.40)
	    m+=0.4*MAX_I32*Sin(100.0/(1.1-Saw(t,0.01)));
	  if (t>16.0)
	    m+=0.3*MAX_I32*Caw(t,1.0/6)*Sin(440.0*(2*ã)*t);
	}
      }
    }

    //Samples are 24-bit, placed in upper 24 bits of an I32.
    m*=snd_vol;
    buf[j++]=ToI64(m*d)&0xFFFFFF00;		//Left
    buf[j++]=ToI64(m*(1.0-d))&0xFFFFFF00;	//Right

    t+=1.0/SND_SAMPLE_RATE;
  }
}

U0 PolyTaskEndCB()
{
  poly_done=TRUE;
  fp_snd_fill_buf=fp_old_fill_buf;
  if (fp_task_end_cb)
    (*fp_task_end_cb)();
  else
    Exit;
}

U0 PolyPhonic()
{
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }
  poly_done=FALSE;
  fp_task_end_cb=Fs->task_end_cb;
  Fs->task_end_cb=&PolyTaskEndCB; //Catch <CTRL-ALT-X> or Kill() task
  start_buf_num=snd_obuf_num;
  fp_old_fill_buf=fp_snd_fill_buf;
  fp_snd_fill_buf=&PolyFillBuf;
  try
#if IncludeDepth<2
    PressAKey;
#else
    View;
#endif
  catch { //Catch <CTRL-ALT-C>, but pass it on to next higher handler.
    fp_snd_fill_buf=fp_old_fill_buf;
    Fs->task_end_cb=fp_task_end_cb;
  }
  poly_done=TRUE;
  fp_snd_fill_buf=fp_old_fill_buf;
  Fs->task_end_cb=fp_task_end_cb;
}

//This is so I can use this file stand alone
//or as a component of a program.
#if IncludeDepth<2
PolyPhonic;
#endif
