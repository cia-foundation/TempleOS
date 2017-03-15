#help_index "Snd"
public U0 SndTaskEndCB()
{//Will turn-off snd when a task gets killed.
  Snd;
  Exit;
}

#help_index "Snd/Math;Math"
public F64 Saw(F64 t,F64 period)
{//Sawtooth. 0.0 - 1.0 think "(Sin+1)/2"
  if (period) {
    if (t>=0.0)
      return t%period/period;
    else
      return 1.0+t%period/period;
  } else
    return 0.0;
}

public F64 FullSaw(F64 t,F64 period)
{//Plus&Minus Sawtooth. 1.0 - -1.0 think "Sin"
  if (period) {
    if (t>=0.0)
      return 2.0*(t%period/period)-1.0;
    else
      return 2.0*(t%period/period)+1.0;
  } else
    return 0.0;
}

public F64 Caw(F64 t,F64 period)
{//Cawtooth. 1.0 - 0.0 think "(Cos+1)/2"
  if (period) {
    if (t>=0.0)
      return 1.0-t%period/period;
    else
      return -(t%period)/period;
  } else
    return 1.0;
}

public F64 FullCaw(F64 t,F64 period)
{//Plus&Minus Cawtooth. 1.0 - -1.0 think "Cos"
  if (period) {
    if (t>=0.0)
      return -2.0*(t%period/period)+1.0;
    else
      return -2.0*(t%period/period)-1.0;
  } else
    return 1.0;
}

public F64 Tri(F64 t,F64 period)
{//Triangle waveform. 0.0 - 1.0 - 0.0
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
{//Plus&Minus Triangle waveform. 0.0 - 1.0 - 0.0 - -1.0 -0.0
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

#help_index "Snd/Music"

public class CMusicGlbls
{
  U8	*cur_song;
  CTask	*cur_song_task;
  I64	octave;
  F64	note_len;
  U8	note_map[7];
  Bool	mute;
  I64	meter_top,meter_bottom;
  F64	tempo,stacatto_factor;

  //If you wish to sync with a
  //note in a Play() string.  0 is the start
  I64	play_note_num;

  F64	tM_correction,last_Beat,last_tM;
} music={NULL,NULL,4,1.0,{0,2,3,5,7,8,10},FALSE,4,4,2.5,0.9,0,0,0,0};

#help_index "Snd/Music;Time/Seconds"
public F64 tM()
{//Time in seconds synced to music subsystem.
  return (cnts.jiffies+music.tM_correction)/JIFFY_FREQ;
}

public F64 Beat()
{//Time in music beats.
  F64 res,cur_tM;
  PUSHFD
  CLI
  if (mp_cnt>1)
    while (LBts(&sys_semas[SEMA_TMBEAT],0))
      PAUSE
  cur_tM=tM;
  res=music.last_Beat;
  if (music.tempo)
    res+=(cur_tM-music.last_tM)*music.tempo;
  music.last_tM=cur_tM;
  music.last_Beat=res;
  LBtr(&sys_semas[SEMA_TMBEAT],0);
  POPFD
  return res;
}

#help_index "Snd/Music"
U8 *MusicSetOctave(U8 *st)
{
  I64 ch;
  ch=*st++;
  while ('0'<=ch<='9') {
    music.octave=ch-'0';
    ch=*st++;
  }
  return --st;
}

U8 *MusicSetMeter(U8 *st)
{
  I64 ch;
  ch=*st++;
  while (ch=='M') {
    ch=*st++;
    if ('0'<=ch<='9') {
      music.meter_top=ch-'0';
      ch=*st++;
    }
    if (ch=='/')
      ch=*st++;
    if ('0'<=ch<='9') {
      music.meter_bottom=ch-'0';
      ch=*st++;
    }
  }
  return --st;
}

U8 *MusicSetNoteLen(U8 *st)
{
  Bool cont=TRUE;
  do {
    switch (*st++) {
      case 'w': music.note_len=4.0;  break;
      case 'h': music.note_len=2.0;  break;
      case 'q': music.note_len=1.0;  break;
      case 'e': music.note_len=0.5;   break;
      case 's': music.note_len=0.25;   break;
      case 't': music.note_len=2.0*music.note_len/3.0; break;
      case '.': music.note_len=1.5*music.note_len; break;
      default:
	st--;
	cont=FALSE;
    }
  } while (cont);
  return st;
}

public I8 Note2Ona(I64 note,I64 octave=4)
{//Note to ona. Mid C is ona=51, note=3 and octave=4.
  if (note<3)
    return (octave+1)*12+note;
  else
    return octave*12+note;
}

public I8 Ona2Note(I8 ona)
{//Ona to note in octave. Mid C is ona=51, note=3 and octave=4.
  return ona%12;
}

public I8 Ona2Octave(I8 ona)
{//Ona to octave. Mid C is ona=51, note=3 and octave=4.
  I64 note=ona%12,octave=ona/12;
  if (note<3)
    return octave-1;
  else
    return octave;
}

public U0 Play(U8 *st,U8 *words=NULL)
{/* Notes are entered with a capital letter.

Octaves are entered with a digit and
stay set until changed.  Mid C is octave 4.

Durations are entered with
'w' whole note
'h' half note
'q' quarter note
'e' eighth note
't' sets to 2/3rds the current duration
'.' sets to 1.5 times the current duration
durations stay set until changed.

'(' tie, placed before the note to be extended

$LK,"music.meter_top",A="MN:CMusicGlbls"$,$LK,"music.meter_bottom",A="MN:CMusicGlbls"$ is set with
"M3/4"
"M4/4"
etc.

Sharp and flat are done with '#' or 'b'.

The var music.stacatto_factor can
be set to a range from 0.0 to 1.0.

The var music.tempo is quarter-notes
per second.  It defaults to
2.5 and gets faster when bigger.
*/
  U8 *word,*last_st;
  I64 note,octave,i=0,ona,timeout_val,timeout_val2;
  Bool tie;
  F64 d,on_jiffies,off_jiffies;
  music.play_note_num=0;
  while (*st) {
    timeout_val=cnts.jiffies;
    tie=FALSE;

    do {
      last_st=st;
      if (*st=='(') {
	tie=TRUE;
	st++;
      } else {
	st=MusicSetMeter(st);
	st=MusicSetOctave(st);
	st=MusicSetNoteLen(st);
      }
    } while (st!=last_st);

    if (!*st) break;
    note=*st++-'A';
    if (note<7) {
      note=music.note_map[note];
      octave=music.octave;
      if (*st=='b') {
	note--;
	if (note==2)
	  octave--;
	st++;
      } else if (*st=='#') {
	note++;
	if (note==3)
	  octave++;
	st++;
      }
      ona=Note2Ona(note,octave);
    } else
      ona=0;
    if (words && (word=LstSub(i++,words)) && StrCmp(word," "))
      "%s",word;

    d=JIFFY_FREQ*music.note_len/music.tempo;
    on_jiffies	=d*music.stacatto_factor;
    off_jiffies =d*(1.0-music.stacatto_factor);

    timeout_val+=on_jiffies;
    timeout_val2=timeout_val+off_jiffies;

    if (!music.mute)
      Snd(ona);
    SleepUntil(timeout_val);
    music.tM_correction+=on_jiffies-ToI64(on_jiffies);

    if (!music.mute && !tie)
      Snd;
    SleepUntil(timeout_val2);
    music.tM_correction+=off_jiffies-ToI64(off_jiffies);

    music.play_note_num++;
  }
}

U0 MusicSettingsRst()
{
  music.play_note_num=0;
  music.stacatto_factor=0.9;
  music.tempo=2.5;
  music.octave=4;
  music.note_len=1.0;
  music.meter_top=4;
  music.meter_bottom=4;
  SndRst;
  PUSHFD
  CLI
  if (mp_cnt>1)
    while (LBts(&sys_semas[SEMA_TMBEAT],0))
      PAUSE
  music.last_tM=tM;
  music.last_Beat=0.0;
  LBtr(&sys_semas[SEMA_TMBEAT],0);
  POPFD
}

MusicSettingsRst;

U0 CurSongTask()
{
  Fs->task_end_cb=&SndTaskEndCB;
  while (TRUE)
    Play(music.cur_song);
}

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
