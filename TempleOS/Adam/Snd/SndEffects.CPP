#define SE_NOISE	0
#define SE_SWEEP	1

class CSoundEffectFrame
{
  I32 type;
  F64 duration,freq1,freq2;
};

U0 SoundEffectEndTaskCB()
{
  Free(FramePtr("CSoundEffectFrame"));
  music_mute--;
  SndTaskEndCB;
}

U0 SoundEffectTask(CSoundEffectFrame *ns)
{
  I64 i;
  F64 f,t0=tS,t,timeout=t0+ns->duration;
  FramePtrAdd("CSoundEffectFrame",ns);
  Fs->task_end_cb=&SoundEffectEndTaskCB;
  switch (ns->type) {
    case SE_NOISE:
      i=MaxI64(ns->freq2-ns->freq1,1);
      while (tS<timeout)
	if (f=RandU16%i+ns->freq1) {
	  Snd(f);
	  Sleep(MaxI64(3000/f,1));
	} else
	  break;
      break;
    case SE_SWEEP:
      while (tS<timeout) {
	t=(tS-t0)/ns->duration;
	if (f=(1.0-t)*ns->freq1+t*ns->freq2) {
	  Snd(f);
	  Sleep(MaxI64(3000/f,1));
	} else
	  break;
      }
      break;
  }
}

public CTask *Noise(I64 mS,F64 min_freq,F64 max_freq)
{
  CSoundEffectFrame *ns;
  if (mS>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_NOISE;
    ns->duration=mS/1000.0;
    ns->freq1=min_freq;
    ns->freq2=max_freq;
    music_mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}

public CTask *Sweep(I64 mS,F64 freq1,F64 freq2)
{
  CSoundEffectFrame *ns;
  if (mS>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_SWEEP;
    ns->duration=mS/1000.0;
    ns->freq1=freq1;
    ns->freq2=freq2;
    music_mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}
