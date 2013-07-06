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

public F64 FullSaw(F64 t,F64 period)
{ //1.0 - -1.0 think "Sin"
  if (period) {
    if (t>=0.0)
      return 2.0*(t%period/period)-1.0;
    else
      return 2.0*(t%period/period)+1.0;
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
      return -2.0*(t%period/period)+1.0;
    else
      return -2.0*(t%period/period)-1.0;
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

public CSndWaveCtrl *SndWaveCtrlNew(I64 sample_rate=8000,I64 sample_bits=24,
  I64 channels=2,CTask *mem_task=NULL)
{
  CSndWaveCtrl *swc=CAlloc(sizeof(CSndWaveCtrl),mem_task);
  swc->freq_multiplier=1.0;
  swc->amp_multiplier=1.0;
  swc->sample_rate=sample_rate;
  swc->sample_bits=sample_bits;
  swc->channels=channels;
  swc->last_dydt=1.0;
  return swc;
}

public U0 SndWaveCtrlDel(CSndWaveCtrl *swc)
{
  Free(swc);
}

#define WF_NULL			0
#define WF_SQUARE		1
#define WF_SINE			2
#define WF_TRI		3
#define WF_SAWTOOTH		4
#define WF_NOISE		5
#define WF_NUM_WAVEFORMS	6

public U0 SndWaveAddBuf(CSndWaveCtrl *swc,U8 *buf,I64 num_samples,
  F64 _freq,I64 _waveform=WF_SQUARE,F64 _amp=1.0,F64 _left=1.0, F64 _right=1.0)
{
//num_samples is multiplied by channels to get buf_len.
//left,right range from 0.0-1.0
//Supports 16,24 and 32 bits
  I64 reg i,reg j,reg k;
  F64 a,f,amp,reg phase;
  if (!swc) return;
  _freq*=swc->freq_multiplier;
  _amp*=swc->amp_multiplier;
  if (!_freq||!_amp) {
    swc->last_y=swc->phase=0;
    swc->last_dydt=1.0;
  } else {
    phase=swc->phase;
    i=0;
    amp=Min(MAX_I32,MAX_I32*_amp);
    f=2*ã/swc->sample_rate*_freq;
    switch (_waveform) {
      case WF_NOISE:
	a=2.0/ã*amp;
	break;
      case WF_SAWTOOTH:
	a=amp/ã;
	break;
      case WF_SINE:
	phase=SinPhaseCont(swc->last_y,swc->last_dydt,amp,0.0);
	break;
    }
    while (phase<0)
      phase+=2*ã;
    while (phase>=2*ã)
      phase-=2*ã;
    num_samples*=swc->channels;
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
	case WF_TRI:
	  if (phase>=ã) {
	    swc->last_y=swc->next_y;
	    swc->next_y=-amp*Sign(swc->last_y)+.00001;
	    phase-=ã;
	  }
	  j=(swc->last_y*(ã-phase)+swc->next_y*phase)/ã;
	  break;
	case WF_SAWTOOTH:
	  j=a*(phase-ã);
	  break;
	case WF_NOISE:
	  if (phase<ã) {
	    if (phase<f) {
	      swc->last_y=swc->next_y;
	      swc->next_y=a*RandI16/MAX_U16;
	    }
	    j=swc->last_y*(ã-phase)+swc->next_y*phase;
	  } else {
	    if (phase-ã<f) {
	      swc->last_y=swc->next_y;
	      swc->next_y=a*RandI16/MAX_U16;
	    }
	    j=swc->last_y*(2.0*ã-phase)+swc->next_y*(phase-ã);
	  }
	  break;
      }
      //left channel
      k=j*_left;
      if (swc->sample_bits==16) {
	k>>=16;
	buf(I16 *)[i++]+=k;
      } else {
	if (swc->sample_bits==24)
	  k&=0xFFFFFF00;
	buf(I32 *)[i++]+=k;
      }
      //right channel
      if (swc->channels==2) {
	k=j*_right;
	if (swc->sample_bits==16) {
	  k>>=16;
	  buf(I16 *)[i++]+=k;
	} else {
	  if (swc->sample_bits==24)
	    k&=0xFFFFFF00;
	  buf(I32 *)[i++]+=k;
	}
      }
      phase+=f;
      while (phase>=2*ã)
	phase-=2*ã;
    }
    if (_waveform==WF_SINE) {
      swc->last_y=amp*Sin(phase);
      swc->last_dydt=Cos(phase);
    }
    swc->phase=phase;
  }
}

#help_index "Snd"
