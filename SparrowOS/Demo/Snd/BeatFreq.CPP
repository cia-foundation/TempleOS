Bool beat_done;
I64 start_buf_num;
U0 (*fp_task_end_cb)();
U0 (*fp_old_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;

F64 freq1,freq2;

U0 BeatFillBuf(SND_OUT_CONTAINER *buf,I64 buf_num)
{ //Gets called by HD Audio task -- $LK,"HDAudioTask","MN:HDAudioTask"$().
  I64 j,m;
  F64 t;
  if (beat_done) return;
  t=ToF64((buf_num-start_buf_num)*(SND_BUF_LEN/SND_OCHANNELS))/SND_SAMPLE_RATE;
  j=0;
  while (j<SND_BUF_LEN) {
    m=MAX_I32*(Sin(freq1*(2*ã)*t)+Sin(freq2*(2*ã)*t));

    //Samples are 24-bit, placed in upper 24 bits of an I32.
    m*=snd_vol;
    buf[j++]=ToI64(m)&0xFFFFFF00;	//Left
    buf[j++]=ToI64(m)&0xFFFFFF00;	//Right

    t+=1.0/SND_SAMPLE_RATE;
  }
}

U0 BeatTaskEndCB()
{
  beat_done=TRUE;
  fp_snd_fill_buf=fp_old_fill_buf;
  if (fp_task_end_cb)
    (*fp_task_end_cb)();
  else
    Exit;
}

U0 BeatFreq()
{
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }
  freq1=600.0;
  freq2=601.0;
  beat_done=FALSE;
  fp_task_end_cb=Fs->task_end_cb;
  Fs->task_end_cb=&BeatTaskEndCB; //Catch <CTRL-ALT-X> or Kill() task
  start_buf_num=snd_obuf_num;
  fp_old_fill_buf=fp_snd_fill_buf;
  fp_snd_fill_buf=&BeatFillBuf;
  try {
    "Pick two frequencies within a few Hz of each other to hear beats.\n"
    "Pick two frequencies a multiple of two for an octave harmonic.\n"
    "$$FG,GREEN$$<CTRL-ALT-C>$$FG$$ to exit.\n";
    while (TRUE) {
      freq1=PmtF64("Freq #1 (%0.2fHz):",freq1,20,20000);
      freq2=PmtF64("Freq #2 (%0.2fHz):",freq2,20,20000);
    }
  } catch { //Catch <CTRL-ALT-C>, but pass it on to next higher handler.
    fp_snd_fill_buf=fp_old_fill_buf;
    Fs->task_end_cb=fp_task_end_cb;
  }
  beat_done=TRUE;
  fp_snd_fill_buf=fp_old_fill_buf;
  Fs->task_end_cb=fp_task_end_cb;
}

BeatFreq;
