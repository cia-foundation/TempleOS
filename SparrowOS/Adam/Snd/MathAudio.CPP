#help_index "Snd/Math;Math"
public F64 Saw(F64 t,F64 period)
{ //0.0 - 1.0 think "(Sin+1)/2"
  if (period) {
    if (t>=0.0)
      return t%period/period;
    else
      return 1.0+t%period/period;
  } else
    return 0.0;
}

public F64 Caw(F64 t,F64 period)
{ //1.0 - 0.0 think "(Cos+1)/2"
  if (period) {
    if (t>=0.0)
      return 1.0-t%period/period;
    else
      return -(t%period)/period;
  } else
    return 1.0;
}

public F64 FullCaw(F64 t,F64 period)
{ //1.0 - -1.0 think "Cos"
  if (period) {
    if (t>=0.0)
      return 1.0-2.0*(t%period/period);
    else
      return -1.0-2.0*(t%period/period);
  } else
    return 1.0;
}

public F64 Tri(F64 t,F64 period)
{ //0.0 - 1.0 - 0.0
  if (period) {
    t=2.0*(Abs(t)%period)/period;
    if (t<=1.0)
      return t;
    else
      return 2.0-t;
  } else
    return 0.0;
}


public F64 FullTri(F64 t,F64 period)
{ //0.0 - 1.0 - 0.0 - -1.0 -0.0
  if (period) {
    t=4.0*(t%period)/period;
    if (t<=-1.0) {
      if (t<=-3.0)
	return t+4.0;
      else
	return -2.0-t;
    } else {
      if (t<=1.0)
	return t;
      else if (t<=3.0)
	return 2.0-t;
      else
	return t-4.0;
    }
  } else
    return 0.0;
}

#help_index "Snd/Math"
public F64 Note2Freq(I64 note,I64 octave=3)
{
  return 440.0*2.0`(octave-3+note/12.0);
}


F64 SinPhaseCont(F64 last_y,F64 last_dydt,
		    F64 current_amp,F64 phase_offset)
{
  F64 phase;
  phase=last_y/current_amp;
  if (phase>1.0) phase=1.0;
  if (phase<-1.0) phase=-1.0;
  if (last_dydt<0)
    phase=ã-ASin(phase);
  else
    phase=ASin(phase);
  return phase-phase_offset;
}

public CSndWaveCtrl *SndWaveCtrlNew(I64 sample_rate=SND_SAMPLE_RATE,I64 sample_bits=SND_SAMPLE_BITS,
  I64 channels=SND_OCHANNELS,CTask *mem_task=NULL)
{
  CSndWaveCtrl *sc=CAlloc(sizeof(CSndWaveCtrl),mem_task);
  sc->sample_rate=sample_rate;
  sc->sample_bits=sample_bits;
  sc->channels=channels;
  sc->last_dydt=1.0;
  return sc;
}

public U0 SndWaveCtrlDel(CSndWaveCtrl *sc)
{
  Free(sc);
}

public U0 SndWaveFillBuf(CSndWaveCtrl *sc,U8 *buf,I64 num_samples,
  F64 _freq,I64 _waveform=WF_SQUARE,F64 _amp=1.0,F64 _left=1.0, F64 _right=1.0)
{
//num_samples is multiplied by channels to get buf_len.
//left,right range from 0.0-1.0
//Supports 16,24 and 32 bits
  I64 reg i,reg j,reg k;
  F64 a,f,amp,reg phase;
  if (!sc) return;
  if (!_freq) {
    MemSet(buf,0,num_samples*sc->channels*(sc->sample_bits>16 ? sizeof(I32):sizeof(I16)));
    sc->last_y=sc->phase=0;
    sc->last_dydt=1.0;
  } else {
    phase=sc->phase;
    i=0;
    amp=Min(MAX_I32,MAX_I32*_amp);
    f=2*ã/sc->sample_rate*_freq;
    switch (_waveform) {
      case WF_NOISE:
	a=2.0/ã*amp;
	break;
      case WF_SAWTOOTH:
	a=amp/ã;
	break;
      case WF_SINE:
	phase=SinPhaseCont(sc->last_y,sc->last_dydt,amp,0.0);
	break;
    }
    while (phase<0)
      phase+=2*ã;
    while (phase>=2*ã)
      phase-=2*ã;
    num_samples*=sc->channels;
    while (i<num_samples) {
      switch (_waveform) {
	case WF_SQUARE:
	  if (phase>=ã)
	    j=-amp;
	  else
	    j=amp;
	  break;
	case WF_SINE:
	  j=amp*Sin(phase);
	  break;
	case WF_TRIANGLE:
	  if (phase>=ã) {
	    sc->last_y=sc->next_y;
	    sc->next_y=-amp*Sign(sc->last_y)+.00001;
	    phase-=ã;
	  }
	  j=(sc->last_y*(ã-phase)+sc->next_y*phase)/ã;
	  break;
	case WF_SAWTOOTH:
	  j=a*(phase-ã);
	  break;
	case WF_NOISE:
	  if (phase<ã) {
	    if (phase<f) {
	      sc->last_y=sc->next_y;
	      sc->next_y=a*RandI16/MAX_U16;
	    }
	    j=sc->last_y*(ã-phase)+sc->next_y*phase;
	  } else {
	    if (phase-ã<f) {
	      sc->last_y=sc->next_y;
	      sc->next_y=a*RandI16/MAX_U16;
	    }
	    j=sc->last_y*(2.0*ã-phase)+sc->next_y*(phase-ã);
	  }
	  break;
      }
      //left channel
      k=j*_left;
      if (sc->sample_bits==16) {
	k>>=16;
	buf(I16 *)[i++]=k;
      } else {
	if (sc->sample_bits==24)
	  k&=0xFFFFFF00;
	buf(I32 *)[i++]=k;
      }
      //right channel
      if (sc->channels==2) {
	k=j*_right;
	if (sc->sample_bits==16) {
	  k>>=16;
	  buf(I16 *)[i++]=k;
	} else {
	  if (sc->sample_bits==24)
	    k&=0xFFFFFF00;
	  buf(I32 *)[i++]=k;
	}
      }
      phase+=f;
      while (phase>=2*ã)
	phase-=2*ã;
    }
    if (_waveform==WF_SINE) {
      sc->last_y=amp*Sin(phase);
      sc->last_dydt=Cos(phase);
    }
    sc->phase=phase;
  }
}

#help_index "Snd/HDAudio"
I64 snd_reverb_buf_cnt;
SND_OUT_CONTAINER *snd_reverb_buf=NULL;

U0 SndReverb(SND_OUT_CONTAINER *buf,I64 buf_num)
{
  I64 i,j,mask,span=RoundI64(SND_SAMPLE_RATE*SND_OCHANNELS*snd_reverb_delay,SND_OCHANNELS);
  if (!snd_reverb_buf || span>snd_reverb_buf_cnt) {
    Free(snd_reverb_buf);
    snd_reverb_buf_cnt=1<<(Bsr(span)+1);
    snd_reverb_buf=ACAlloc(snd_reverb_buf_cnt*sizeof(SND_OUT_CONTAINER));
  }
  j=buf_num*SND_BUF_LEN;
  mask=snd_reverb_buf_cnt-1;
  for (i=0;i<SND_BUF_LEN;i++,j++) {
    snd_reverb_buf[j&mask]=
    buf[i]=
      LimitI64(buf[i]+snd_reverb_intensity*snd_reverb_buf[(j-span)&mask],
	MIN_I32,MAX_I32)&0xFFFFFF00;
  }
}

I64 AudioSize()
{
  return MSize2(snd_reverb_buf);
}

#help_index "Snd"
