Bool echo_done;

U0 (*fp_task_end_cb)();
U0 (*fp_old_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;
U0 (*fp_old_copy_buf)(SND_IN_CONTAINER *buf,I64 buf_num)=NULL;

#define NUM_BUF	8
SND_IN_CONTAINER my_buf[SND_BUF_LEN*NUM_BUF];

I64 my_ibuf_ptr,my_obuf_ptr;

U0 EchoFillBuf(SND_OUT_CONTAINER *buf,I64)
{
  I64 j;
  if (echo_done) return;
  j=0;
  while (j<SND_BUF_LEN) {
    buf[j++]=ToI64(snd_vol*my_buf[my_obuf_ptr++ &(SND_BUF_LEN*NUM_BUF-1)])<<16&0xFFFFFF00;
    buf[j++]=ToI64(snd_vol*my_buf[my_obuf_ptr++ &(SND_BUF_LEN*NUM_BUF-1)])<<16&0xFFFFFF00;
  }
}

U0 EchoCopyBuf(SND_IN_CONTAINER *buf,I64)
{
  I64 j;
  if (echo_done) return;
  j=0;
  while (j<SND_BUF_LEN) {
    my_buf[my_ibuf_ptr++ &(SND_BUF_LEN*NUM_BUF-1)]=buf[j++];
    my_buf[my_ibuf_ptr++ &(SND_BUF_LEN*NUM_BUF-1)]=buf[j++];
  }
}

U0 EchoTaskEndCB()
{
  echo_done=TRUE;
  HDStop(TRUE,FALSE);
  fp_snd_fill_buf=fp_old_fill_buf;
  fp_snd_copy_buf=fp_old_copy_buf;
  if (fp_task_end_cb)
    (*fp_task_end_cb)();
  else
    Exit;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  SND_IN_CONTAINER *buf=_hda->istr0_buf[0];
  I64 i,x1,y1,x2=0,y2=0,
    cy=task->win_pixel_height>>1;
  dc->color=BLUE;
  for (i=0;i<SND_BUF_LEN;i++) {
    x1=i*task->win_pixel_width/SND_BUF_LEN;
    y1=cy-buf[i]*cy/MAX_I16;
    GrLine(dc,x2,y2,x1,y1);
    x2=x1;
    y2=y1;
  }
}

U0 EchoDemo()
{
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  HDRun(TRUE,TRUE);
  echo_done=FALSE;
  fp_task_end_cb=Fs->task_end_cb;
  Fs->task_end_cb=&EchoTaskEndCB; //Catch <CTRL-ALT-X> or Kill() task
  my_ibuf_ptr=(NUM_BUF/2)*SND_BUF_LEN;
  my_obuf_ptr=0;
  fp_old_fill_buf=fp_snd_fill_buf;
  fp_old_copy_buf=fp_snd_copy_buf;
  fp_snd_fill_buf=&EchoFillBuf;
  fp_snd_copy_buf=&EchoCopyBuf;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try
    GetChar;
  catch { //Catch <CTRL-ALT-C>, but pass it on to next higher handler.
    fp_snd_fill_buf=fp_old_fill_buf;
    fp_snd_copy_buf=fp_old_copy_buf;
    Fs->task_end_cb=fp_task_end_cb;
  }
  echo_done=TRUE;
  HDStop(TRUE,FALSE);
  fp_snd_fill_buf=fp_old_fill_buf;
  fp_snd_copy_buf=fp_old_copy_buf;
  Fs->task_end_cb=fp_task_end_cb;
  SettingsPop;
}

EchoDemo;
