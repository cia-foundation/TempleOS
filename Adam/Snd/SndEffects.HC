#help_index "Snd"

#define SE_NOISE	0
#define SE_SWEEP	1

class CSoundEffectFrame
{
  I32	type;
  I8	ona1,ona2;
  F64	duration;
};

U0 SoundEffectEndTaskCB()
{
  Free(FramePtr("CSoundEffectFrame"));
  music.mute--;
  SndTaskEndCB;
}

U0 SoundEffectTask(CSoundEffectFrame *ns)
{
  I64 i,ona;
  F64 t0=tS,t,timeout=t0+ns->duration;
  FramePtrAdd("CSoundEffectFrame",ns);
  Fs->task_end_cb=&SoundEffectEndTaskCB;
  switch (ns->type) {
    case SE_NOISE:
      i=MaxI64(ns->ona2-ns->ona1,1);
      while (tS<timeout) {
	ona=RandU16%i+ns->ona1;
	Snd(ona);
	t=Clamp(3000.0/Ona2Freq(ona),1.0,50.0);
	if (t+tS>timeout)
	  t=timeout-tS;
	Sleep(t);
      }
      break;
    case SE_SWEEP:
      while (tS<timeout) {
	t=(tS-t0)/ns->duration;
	ona=(1.0-t)*ns->ona1+t*ns->ona2;
	Snd(ona);
	t=Clamp(3000.0/Ona2Freq(ona),1.0,50.0);
	if (t+tS>timeout)
	  t=timeout-tS;
	Sleep(t);
      }
      break;
  }
}

public CTask *Noise(I64 mS,F64 min_ona,F64 max_ona)
{//Make white noise for given number of mS.
  CSoundEffectFrame *ns;
  if (mS>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_NOISE;
    ns->duration=mS/1000.0;
    ns->ona1=min_ona;
    ns->ona2=max_ona;
    music.mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}

public CTask *Sweep(I64 mS,F64 ona1,F64 ona2)
{//Sweep through freq range in given number of mS.
  CSoundEffectFrame *ns;
  if (mS>0) {
    ns=MAlloc(sizeof(CSoundEffectFrame));
    ns->type=SE_SWEEP;
    ns->duration=mS/1000.0;
    ns->ona1=ona1;
    ns->ona2=ona2;
    music.mute++;
    return Spawn(&SoundEffectTask,ns,"Noise",,Fs);
  } else
    return NULL;
}
