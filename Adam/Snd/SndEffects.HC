#help_index "Snd"

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
  music.mute--;
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
	  Sleep(ClampI64(3000/f,1,50));
	} else
	  break;
      break;
    case SE_SWEEP:
      while (tS<timeout) {
	t=(tS-t0)/ns->duration;
	if (f=(1.0-t)*ns->freq1+t*ns->freq2) {
	  Snd(f);
	  Sleep(ClampI64(3000/f,1,50));
	} else
	  break;
      }
      break;
  }
}

public CTask *Noise(I64 ms,F64 min_freq,F64 max_freq)
{//Make white noise for given number of ms.
  CSoundEffectFrame *ns;
  if (ms>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_NOISE;
    ns->duration=ms/1000.0;
    ns->freq1=min_freq;
    ns->freq2=max_freq;
    music.mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}

public CTask *Sweep(I64 ms,F64 freq1,F64 freq2)
{//Sweep through freq range in given number of ms.
  CSoundEffectFrame *ns;
  if (ms>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_SWEEP;
    ns->duration=ms/1000.0;
    ns->freq1=freq1;
    ns->freq2=freq2;
    music.mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}
