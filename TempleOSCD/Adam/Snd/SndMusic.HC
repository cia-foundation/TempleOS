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
  I64 note,i=0,ona,timeout_val,timeout_val2;
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
      if (*st=='b') {
	note--;
	st++;
      } else if (*st=='#') {
	note++;
	st++;
      }
      ona=Note2Ona(note,music.octave);
    } else
      ona=ONA_REST;
    if (words && (word=LstSub(i++,words)) && StrCmp(word," "))
      "%s",word;
    d=JIFFY_FREQ*music.note_len/music.tempo;
    if (tie) {
      on_jiffies  =d;
      off_jiffies =0;
    } else {
      on_jiffies  =d*music.stacatto_factor;
      off_jiffies =d*(1.0-music.stacatto_factor);
    }
    timeout_val+=on_jiffies;
    timeout_val2=timeout_val+off_jiffies;

    if (!music.mute)
      Snd(ona);
    SleepUntil(timeout_val);
    music.tM_correction+=on_jiffies-ToI64(on_jiffies);

    if (!music.mute)
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
