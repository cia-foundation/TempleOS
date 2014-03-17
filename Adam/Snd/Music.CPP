#help_index "Snd/Music"
U8 *cur_song=NULL;
CTask *cur_song_task=NULL;
I64 music_octave;
F64 music_note_len;
U8 music_note_map[7]={0,2,3,5,7,8,10};
I64 music_mute=0;
I64 music_meter_top,music_meter_bottom;
public F64 music_tempo,music_stacatto_factor;

//This is if you wish to sync with a
//note in a Play() string.  0 is the start
public I64 music_play_note_num=0;

#help_index "Snd/Music;Time/Seconds"
F64 music_tM_correction=0,music_last_Beat=0,music_last_tM=0;
public F64 tM()
{
  return (sys_jiffies+music_tM_correction)/JIFFY_FREQ;
}

public F64 Beat()
{ //Time in music beats
  F64 result,cur_tM;
  PUSHFD
  CLI
  if (mp_cnt>1)
    while (LBts(&sys_semas[SYS_SEMA_TMBEAT],0))
      PAUSE
  cur_tM=tM;
  result=music_last_Beat;
  if (music_tempo)
    result+=(cur_tM-music_last_tM)*music_tempo;
  music_last_tM=cur_tM;
  music_last_Beat=result;
  LBtr(&sys_semas[SYS_SEMA_TMBEAT],0);
  POPFD
  return result;
}

#help_index "Snd/Music"
U8 *MusicSetOctave(U8 *st)
{
  I64 ch;
  ch=*st++;
  while ('0'<=ch<='9') {
    music_octave=ch-'0';
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
      music_meter_top=ch-'0';
      ch=*st++;
    }
    if (ch=='/')
      ch=*st++;
    if ('0'<=ch<='9') {
      music_meter_bottom=ch-'0';
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
      case 'w': music_note_len=4.0;  break;
      case 'h': music_note_len=2.0;  break;
      case 'q': music_note_len=1.0;  break;
      case 'e': music_note_len=0.5;   break;
      case 's': music_note_len=0.25;   break;
      case 't': music_note_len=2.0*music_note_len/3.0; break;
      case '.': music_note_len=1.5*music_note_len; break;
      default:
	st--;
	cont=FALSE;
    }
  } while (cont);
  return st;
}

public U0 Play(U8 *st,U8 *words=NULL)
/*
  Notes are entered with a capital letter.

  Octaves are entered with a digit and
  stay set until changed.

  Durations are entered with
    'w' whole note
    'h' half note
    'q' quarter note
    'e' eighth note
    't' sets to 2/3rds the current duration
    '.' sets to 1.5 times the current duration
  durations stay set until changed.

    '(' tie, placed before the note to be extended

  $LK,"music_meter_top",A="MN:music_meter_top"$,$LK,"music_meter_bottom",A="MN:music_meter_bottom"$ is set with
    "M3/4"
    "M4/4"
    etc.

  Sharp and flat are done with '#' or 'b'.

  The var music_stacatto_factor can
  be set to a range from 0.0 to 1.0.

  The var music_tempo is quarter-notes
  per second.  It defaults to
  2.5 and gets faster when bigger.
 
*/

{
  U8 *word,*last_st;
  I64 note,i=0,timeout_val,timeout_val2;
  Bool tie,old_preempt=Preempt(OFF);
  F64 f,d,on_jiffies,off_jiffies;
  music_play_note_num=0;
  while (*st) {
    timeout_val=sys_jiffies;
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
      note=music_note_map[note];
      if (*st=='b') {
	note--;
	st++;
      } else if (*st=='#') {
	note++;
	st++;
      }
      f=Note2Freq(note,music_octave);
    } else
      f=0;
    if (words && (word=LstSub(i++,words)) && StrCmp(word," "))
      "%s",word;
    d=JIFFY_FREQ*music_note_len/music_tempo;
    if (tie) {
      on_jiffies  =d;
      off_jiffies =0;
    } else {
      on_jiffies  =d*music_stacatto_factor;
      off_jiffies =d*(1.0-music_stacatto_factor);
    }
    timeout_val+=on_jiffies;
    timeout_val2=timeout_val+off_jiffies;

    if (!music_mute)
      Snd(f);
    SleepUntil(timeout_val);
    music_tM_correction+=on_jiffies-ToI64(on_jiffies);

    if (!music_mute)
      Snd(0);
    SleepUntil(timeout_val2);
    music_tM_correction+=off_jiffies-ToI64(off_jiffies);

    music_play_note_num++;
  }
  Preempt(old_preempt);
}

U0 MusicSettingsRst()
{
  music_play_note_num=0;
  music_stacatto_factor=0.9;
  music_tempo=2.5;
  music_octave=3;
  music_note_len=1.0;
  music_meter_top=4;
  music_meter_bottom=4;

  PUSHFD
  CLI
  if (mp_cnt>1)
    while (LBts(&sys_semas[SYS_SEMA_TMBEAT],0))
      PAUSE
  music_last_tM=tM;
  music_last_Beat=0.0;
  LBtr(&sys_semas[SYS_SEMA_TMBEAT],0);
  POPFD
}

MusicSettingsRst;

U0 CurSongTask()
{
  Preempt(ON);
  Fs->task_end_cb=&SndTaskEndCB;
  while (TRUE)
    Play(cur_song);
}
#help_index "Snd"
