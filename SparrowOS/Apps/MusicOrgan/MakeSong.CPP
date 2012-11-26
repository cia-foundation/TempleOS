class MakeSongSettings
{
  Bool simple_rhythms		fmtstr "$$CB,\"Simple Rhythms\"$$\n";
  Bool rests			fmtstr "$$CB,\"Rests\"$$\n";
  Bool change_waveforms	fmtstr "$$CB,\"Change Waveforms\"$$\n";
  Bool six_eight		fmtstr "$$CB,\"Six Eight\"$$\n";
  I64	 octave			fmtstr "$$DA-T,32,\"Octave:%d\"$$\n";
};

U0 InsNote(MakeSongSettings *mss,U8 *buf,I64 k,I64 *j)
{  //k is a random note nibble
  I64 a=TimerPickedNibble; //accent
  if (!a) {
    buf[*j]='!';
    *j+=1;
  }
  if (a<4) {
    buf[*j]='!';
    *j+=1;
  }
  if (!k && mss->rests) {
    buf[*j]='R';
    *j+=1;
  } else {
    k/=2;
    if (!k) {
      if (music_octave) {
	buf[*j]=music_octave-1+'0';
	*j+=1;
	buf[*j]='G';
	*j+=1;
	buf[*j]=music_octave+'0';
	*j+=1;
      } else {
	buf[*j]='G';
	*j+=1;
      }
    } else {
      buf[*j]=k-1+'A';
      *j+=1;
    }
  }
}

#define DUR_QUARTER	0
#define DUR_EIGHTH	1
#define DUR_TRIPLET	2
#define DUR_SIXTEENTH	3

U8 simple_songs [5]={DUR_EIGHTH,DUR_QUARTER,DUR_QUARTER,DUR_QUARTER,DUR_QUARTER};
U8 complex_songs[6]={DUR_EIGHTH,DUR_TRIPLET,DUR_SIXTEENTH,DUR_QUARTER,DUR_QUARTER,DUR_QUARTER};

U8 *MakeSong()
{
  MakeSongSettings mss;
  U8 *buf;
  I64 i,j=0,k,n,k2,duration,last_duration=-1,
     wf[2],w=0,len;

  mss.simple_rhythms=FALSE;
  mss.rests=FALSE;
  mss.change_waveforms=FALSE;
  mss.octave=music_octave;

  if (!PopUpForm(&mss,"MakeSongSettings"))
    return NULL;

  buf=CAlloc(256);
  music_octave=LimitI64(mss.octave,0,6);
  progress4=0;
  if (mss.six_eight)
    progress4_max=6;
  else
    progress4_max=8;

  n=TimerPickedNi$WW,0$bble/2;
  if (WF_NULL<n<WF_NOISE)
    wf[0]=n;
  else
    wf[0]=WF_SQUARE;

  n=TimerPickedNibble/2;
  if (WF_NULL<n<WF_NOISE)
    wf[1]=n;
  else
    wf[1]=wf[0];

  buf[j++]='0'+music_octave;
  if (mss.six_eight) {
    len=6;
    buf[j++]='M';
    buf[j++]='6';
    buf[j++]='/';
    buf[j++]='8';
  } else
    len=8;
  if (mss.change_waveforms) {
    buf[j++]='W';
    buf[j++]='0'+wf[w++&1];
  }
  for (i=0;i<len;i++) {
    if (mss.change_waveforms) {
      if (TimerPickedNibble<6) {
	buf[j++]='W';
	buf[j++]='0'+wf[w++&1];
      }
    }

    n=TimerPickedNibble;
    k=TimerPickedNibble;
    if (mss.simple_rhythms)
      duration=simple_songs[n%5];
    else
      duration=complex_songs[n%6];
    switch (duration) {
      case DUR_EIGHTH:
	if (last_duration!=DUR_EIGHTH)
	  buf[j++]='e';
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	break;
      case DUR_TRIPLET:
	if (last_duration!=DUR_TRIPLET) {
	  buf[j++]='e';
	  buf[j++]='t';
	}
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	break;
      case DUR_SIXTEENTH:
	if (last_duration!=DUR_SIXTEENTH)
	  buf[j++]='s';
	InsNote(&mss,buf,k,&j);
	k2=k;
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	InsNote(&mss,buf,k2,&j);
	InsNote(&mss,buf,k,&j);
	break;
      default:
	if (last_duration!=DUR_QUARTER)
	  buf[j++]='q';
	InsNote(&mss,buf,k,&j);
    }
    last_duration=duration;
    progress4++;
  }
  buf[j++]=0;
  progress4=progress4_max=0;
  return buf;
}
