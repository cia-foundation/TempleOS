#define SE_NOISE	0
#define SE_SWEEP	1

class CSoundEffectFrame
{
  I32 type,waveform;
  F64 duration,freq1,freq2;
};

U0 SoundEffectEndTaskCB()
{
  Free(NamedFramePtr("CSoundEffectFrame"));
  music_mute--;
  SndTaskEndCB;
}

U0 SoundEffectTask(CSoundEffectFrame *ns)
{
  I64 i;
  F64 f,t0=tS,t,timeout=t0+ns->duration;
  NamedFramePtrAdd("CSoundEffectFrame",ns);
  Fs->task_end_cb=&SoundEffectEndTaskCB;
  switch (ns->type) {
    case SE_NOISE:
      i=MaxI64(ns->freq2-ns->freq1,1);
      while (tS<timeout)
	if (f=RandU16%i+ns->freq1) {
	  Snd(f,ns->waveform);
	  Sleep(MaxI64(3000/f,1));
	} else
	  break;
      break;
    case SE_SWEEP:
      while (tS<timeout) {
	t=(tS-t0)/ns->duration;
	if (f=(1.0-t)*ns->freq1+t*ns->freq2) {
	  Snd(f,ns->waveform);
	  Sleep(MaxI64(3000/f,1));
	} else
	  break;
      }
      break;
  }
}

public CTask *Noise(I64 mS,F64 min_freq,F64 max_freq,I64 waveform=WF_NOISE)
{
  CSoundEffectFrame *ns=MAlloc(sizeof(CSoundEffectFrame));
  ns->type=SE_NOISE;
  ns->waveform=waveform;
  ns->duration=mS/1000.0;
  ns->freq1=min_freq;
  ns->freq2=max_freq;
  music_mute++;
  return Spawn(&SoundEffectTask,ns,"Noise",Fs);
}

public CTask *Sweep(I64 mS,F64 freq1,F64 freq2,I64 waveform=WF_SINE)
{
  CSoundEffectFrame *ns=MAlloc(sizeof(CSoundEffectFrame));
  ns->type=SE_SWEEP;
  ns->waveform=waveform;
  ns->duration=mS/1000.0;
  ns->freq1=freq1;
  ns->freq2=freq2;
  music_mute++;
  return Spawn(&SoundEffectTask,ns,"Noise",Fs);
}
