#help_index "Snd/Snd Files"
#define SNDFILE_SAMPLE_RATE	8000
//Header for a ".SND" file
class CFileSND
{ //big endian
  U32 signature;    //0x646e732e
  U32 offset;	   //24
  U32 data_size;
  U32 coding;	   //3=16bit uncompressed
  U32 sample_rate;  //Hz
  U32 channels;	   //1=mono
  I16 body[1];
};

//This is a windows media constraint.
#define MAX_SND_FILE_DATA	0x7FF00


public I64 SndFileCreate(U8 *base_filename,F64 normalized_vol=1.0,F64 averaging=0.0,I64 waveform=WF_NULL,
  F64 reverb_delay=0,F64 reverb_intensity=0)
//Use "snd_record" flag to start or stop
//recording, then call this routine.

//Averaging should be a num from 0.0 to 0.999.
//Vol should be from 0.0 to 1.0.
//If waveform is $LK,"WF_NULL","MN:WF_NULL"$, the original passed to $LK,"Snd","MN:Snd"$() will be used.

//Set reverb_delay to like 0.3 sec and reverb_intensity to like 0.4.

{
  I64 i,k,cnt,cnt2,level;
  F64 avg,dt;
  CFileSND *s;
  CSndWaveCtrl *sc=SndWaveCtrlNew(SNDFILE_SAMPLE_RATE,16,1);
  CSndData *d,*d1;
  U8 *name,*name2;
  I64 file_num;

  snd_record=FALSE;

  dt=snd_record_root.last->time_stamp-snd_record_root.next->time_stamp;
  if (!dt) return 0;
  cnt=dt*SNDFILE_SAMPLE_RATE;
  cnt++; //terminator

  name=StrNew(base_filename);
  FileExtRem(name);

  s=CAlloc(offset(CFileSND.body)+cnt*sizeof(I16));
  s->signature=0x646e732e;
  s->offset=EndianU32(offset(CFileSND.body));
  s->coding=EndianU32(3);
  s->sample_rate=EndianU32(SNDFILE_SAMPLE_RATE);
  s->channels=EndianU32(1);

  d=snd_record_root.next;
  k=0;
  while (d->next!=&snd_record_root) {
    d1=d->next;
    dt=d1->time_stamp-d->time_stamp;
    cnt2=dt*SNDFILE_SAMPLE_RATE;
    SndWaveFillBuf(sc,&s->body[k],cnt2,d->freq,waveform ?waveform:d->waveform,d->amp*normalized_vol);
    k+=cnt2;
    QueRem(d);
    Free(d);
    d=d1;
  }

  //Average
  if (averaging) {
    avg=0;
    for (i=0;i<cnt-1;i++)
      s->body[i]=avg=LowPass1(averaging,avg,s->body[i],1.0);
    }

    //reverb
  if (reverb_intensity) {
    if (dt=reverb_delay*SNDFILE_SAMPLE_RATE)
      for (i=dt;i<cnt;i++)
	s->body[i]+=reverb_intensity*s->body[i-dt];
      }

      //Get rid of D.C. component
  for (k=0;k<3;k++) {
    level=0;
    for (i=0;i<cnt-1;i++)
      level+=s->body[i];
    level/=cnt-1;
    for (i=0;i<cnt-1;i++)
      s->body[i]=LimitI64(s->body[i]-level,MIN_I16,MAX_I16);
    }

  for (i=0;i<cnt-1;i++)
    s->body[i]=EndianU16(s->body[i]);

  s->body[cnt-1]=0;

  d=snd_record_root.next;
  while (d!=&snd_record_root) {
    d1=d->next;
    QueRem(d);
    Free(d);
    d=d1;
  }

  name2=MAlloc(StrLen(name)+3+1+3+1);
  cnt2=cnt;
  file_num=0;
  while (cnt2>0) {
    i=cnt2;
    if (i>MAX_SND_FILE_DATA)
      i=MAX_SND_FILE_DATA;
    s->data_size=EndianU32(i*sizeof(I16));
    MemCpy(s->body,&s->body[file_num*MAX_SND_FILE_DATA],i*sizeof(I16));
    SPrintF(name2,"%s%03d.SND",name,file_num++);
    FileWrite(name2,s,offset(CFileSND.body)+i*sizeof(I16));
    cnt2-=i;
  }
  Free(s);
  Free(name);
  Free(name2);

  SndWaveCtrlDel(sc);
  return cnt;
}

public U0 SndFileWrite(U8 *filename,SND_IN_CONTAINER *buf,I64 cnt)
{
  CFileSND *s;
  I64 i,j,l=SND_SAMPLE_RATE/SNDFILE_SAMPLE_RATE;
  F64 level,a1,a2;

  cnt/=l;
  s=CAlloc(offset(CFileSND.body)+cnt*sizeof(I16));
  s->signature=0x646e732e;
  s->offset=EndianU32(offset(CFileSND.body));
  s->coding=EndianU32(3);
  s->sample_rate=EndianU32(SNDFILE_SAMPLE_RATE);
  s->channels=EndianU32(1);
  s->data_size=EndianU32(cnt*sizeof(I16));

  //Get rid of D.C. component
  j=cnt*l;
  level=0;
  for (i=0;i<j;i++)
    level+=buf[i];
  level/=j;
  for (i=0;i<j;i++)
    buf[i]=LimitI64(buf[i]-level,MIN_I16,MAX_I16);

  //2nd order low pass filter.	Time constant set by nyquest freq.
  a1=1.0/SND_SAMPLE_RATE/(2.0/SNDFILE_SAMPLE_RATE+1.0/SND_SAMPLE_RATE);
  a2=1.0-a1;
  for (i=1;i<j;i++)
    buf[i]=a1*buf[i]+a2*buf[i-1];
  for (i=1;i<j;i++)
    buf[i]=a1*buf[i]+a2*buf[i-1];

  j=0;
  for (i=0;i<cnt;i++) {
    s->body[i]=EndianU16(buf[j]);
    j+=l;
  }
  FileWrite(filename,s,offset(CFileSND.body)+cnt*sizeof(I16));
  Free(s);
}

public U0 SndFileView(U8 *name,Bool zoom=FALSE)
{
  I64 i,start,end,w=Fs->win_pixel_width,h=Fs->win_pixel_height;
  F64 x_scale,y_scale;
  CFileSND *s=FileRead(name);
  if (s) {
    s->data_size=EndianU32(s->data_size)/sizeof(I16);
    for (i=0;i<s->data_size;i++)
      s->body[i]=EndianU16(s->body[i]);
    start=1;
    end=s->data_size;
    if (zoom) {
      for (i=1;i<end;i++)
	if (s->body[i]!=s->body[i-1]) {
	  start=i;
	  break;
	}
      if (s->data_size-start>w)
	end=start+w;
    }
    "$$CL$$";
    DCFill;
    CDC *dc=DCAlias;
    dc->color=GREEN;
    x_scale=ToF64(w)/(end-start+1);
    y_scale=ToF64(h)/MAX_U16;
    for (i=start;i<end;i++)
      GrLine(dc,x_scale*(i-start),y_scale*(s->body[i]+MAX_I16),
		x_scale*(i-start-1),y_scale*(s->body[i-1]+MAX_I16));
    Free(s);
    PressAKey;
    DCFill;
    DCDel(dc);
  }
}


CFileSND *snd_play_snd_file=NULL;
I64 snd_play_start_buf_num,snd_play_samples_per_sample;
Bool snd_play_done=TRUE,snd_play_single_done;
U0 (*fp_snd_play_task_end_cb)()=NULL;
U0 (*fp_snd_play_old_fill_buf)(SND_OUT_CONTAINER *buf,I64 buf_num)=NULL;

CFileSND *SndFileLoad(U8 *name)
{
  I64 i;
  CFileSND *s=FileRead(name);
  if (s) {
    s->data_size=EndianU32(s->data_size)/sizeof(I16);
    for (i=0;i<s->data_size;i++)
      s->body[i]=EndianU16(s->body[i]);
  }
  return s;
}

U0 SndFileFillBuf(SND_OUT_CONTAINER *buf,I64 buf_num)
{
  I64 i,j,k,m;
  if (snd_play_done) return;
  buf_num-=snd_play_start_buf_num;
  j=buf_num*(SND_BUF_LEN/SND_OCHANNELS*SNDFILE_SAMPLE_RATE/SND_SAMPLE_RATE);
  i=j%snd_play_snd_file->data_size;
  k=j%snd_play_samples_per_sample;
  j=0;
  while (j<SND_BUF_LEN) {
    m=snd_vol*snd_play_snd_file->body[i++]<<16;
    while (j<SND_BUF_LEN) {
      buf[j++]=m;
      buf[j++]=m;
      if (++k==snd_play_samples_per_sample)
	break;
    }
    if (i>=snd_play_snd_file->data_size) {
      while (j<SND_BUF_LEN) {
	buf[j++]=0;
	buf[j++]=0;
      }
      snd_play_done=TRUE;
      break;
    }
    k=0;
  }
}

U0 SndFilePlayTaskEndCB()
{
  snd_play_done=TRUE;
  fp_snd_fill_buf=fp_snd_play_old_fill_buf;
  if (fp_snd_play_task_end_cb)
    (*fp_snd_play_task_end_cb)();
  else
    Exit;
}

//See $LK,"::/SparrowOS/Apps/MusicOrgan/Help.TXT.Z","FI:::/SparrowOS/Apps/MusicOrgan/Help.TXT.Z"$
//See $LK,"SndFileCreate","MN:SndFileCreate"$().
public U0 SndFilePlay(U8 *snz_filename)
{
  if (snd_play_done) {
    TaskInitExt;
    snd_play_snd_file=SndFileLoad(snz_filename);
    snd_play_samples_per_sample=SND_SAMPLE_RATE/SNDFILE_SAMPLE_RATE;
    fp_snd_play_task_end_cb=Fs->task_end_cb;
    Fs->task_end_cb=&SndFilePlayTaskEndCB;
    snd_play_start_buf_num=snd_obuf_num;
    snd_play_done=FALSE;
    fp_snd_play_old_fill_buf=fp_snd_fill_buf;
    fp_snd_fill_buf=&SndFileFillBuf;
    try {
      while (!snd_play_done)
	Sleep(1);
    } catch {
      snd_play_done=TRUE;
      fp_snd_fill_buf=fp_snd_play_old_fill_buf;
      Fs->task_end_cb=fp_snd_play_task_end_cb;
      Free(snd_play_snd_file);
      snd_play_snd_file=NULL;
    }
    fp_snd_fill_buf=fp_snd_play_old_fill_buf;
    Fs->task_end_cb=fp_snd_play_task_end_cb;
    Free(snd_play_snd_file);
    snd_play_snd_file=NULL;
  }
  snd_play_single_done=TRUE;
}

public U0 SndFilePlaySeq(U8 *snz_filename,I64 num)
{ //This has gaps between files.  I might fix it someday.  You fix it;-)
  I64 i;
  Bool old_preempt=Preempt(OFF),old_idle;
  U8 *fname=StrNew(snz_filename),*fname2;
  FileExtRem(fname);
  old_idle=LBts(&Fs->task_flags,TASKf_IDLE);
  for (i=0;i<num;i++) {
    fname2=MSPrintF("%s%04d.SNZ",fname,i);
    while (!snd_play_done)
      Yield;
    snd_play_single_done=FALSE;
    Spawn(&SndFilePlay,fname2,"Play .SNZ",Fs);
    while (!snd_play_single_done)
      Yield;
    Free(fname2);
  }
  old_idle=LBEqu(&Fs->task_flags,TASKf_IDLE,old_idle);
  Free(fname);
  Preempt(old_preempt);
}

#help_index ""
