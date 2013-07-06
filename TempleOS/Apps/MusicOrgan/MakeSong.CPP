class MakeSongSettings
{
  Bool simple_rhythms		fmtstr "$$CB,\"Simple Rhythms\"$$\n";
  Bool normal_rhythms		fmtstr "$$CB,\"Normal Rhythms\"$$\n";
  Bool complex_rhythms		fmtstr "$$CB,\"Complex Rhythms\"$$\n";
  Bool rests			fmtstr "$$CB,\"Rests\"$$\n";
  Bool six_eight		fmtstr "$$CB,\"Six Eight\"$$\n";
  I64  octave			fmtstr "$$DA-T,32,\"Octave:%d\"$$\n";
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

#define DUR_4		0
#define DUR_8_8		1
#define DUR_3_3_3	2
#define DUR_16_16_16_16	3
#define DUR_8DOT_16	4
#define DUR_8_16_16	5
#define DUR_16_16_8	6

U8 simple_songs [5]={DUR_4,DUR_4,DUR_4,DUR_4,DUR_8_8};
U8 normal_songs [5]={DUR_4,DUR_4,DUR_8_8,DUR_3_3_3,DUR_16_16_16_16};
U8 complex_songs[9]={DUR_4,DUR_4,DUR_8_8,DUR_8_8,DUR_8DOT_16,DUR_3_3_3,DUR_8_16_16,DUR_16_16_8,DUR_16_16_16_16};

U8 *MakeSong()
{
  MakeSongSettings mss;
  U8 *buf;
  I64 i,j=0,k,n,k2,duration,last_duration=-1,len;

  mss.simple_rhythms=FALSE;
  mss.normal_rhythms=TRUE;
  mss.complex_rhythms=FALSE;
  mss.rests=FALSE;
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

  buf[j++]='0'+music_octave;
  if (mss.six_eight) {
    len=6;
    buf[j++]='M';
    buf[j++]='6';
    buf[j++]='/';
    buf[j++]='8';
  } else
    len=8;
  for (i=0;i<len;i++) {
    n=TimerPickedNibble<<4+TimerPickedNibble;
    if (mss.complex_rhythms)
      duration=complex_songs[n%9];
    else if (mss.normal_rhythms)
      duration=normal_songs[n%5];
    else
      duration=simple_songs[n%5];

    k=TimerPickedNibble;
    switch (duration) {
      case DUR_8_8:
	if (last_duration!=DUR_8_8)
	  buf[j++]='e';
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	break;
      case DUR_8DOT_16:
	buf[j++]='e';
	buf[j++]='.';
	InsNote(&mss,buf,k,&j);
	buf[j++]='s';
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	duration=DUR_16_16_16_16;
	break;
      case DUR_3_3_3:
	if (last_duration!=DUR_3_3_3) {
	  buf[j++]='e';
	  buf[j++]='t';
	}
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	break;
      case DUR_8_16_16:
	if (last_duration!=DUR_8_8)
	  buf[j++]='e';
	InsNote(&mss,buf,k,&j);
	buf[j++]='s';
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	duration=DUR_16_16_16_16;
	break;
      case DUR_16_16_8:
	if (last_duration!=DUR_16_16_16_16)
	  buf[j++]='s';
	InsNote(&mss,buf,k,&j);
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	buf[j++]='e';
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	duration=DUR_8_8;
	break;
      case DUR_16_16_16_16:
	if (last_duration!=DUR_16_16_16_16)
	  buf[j++]='s';
	InsNote(&mss,buf,k,&j);
	k2=k;
	k=TimerPickedNibble;
	InsNote(&mss,buf,k,&j);
	InsNote(&mss,buf,k2,&j);
	InsNote(&mss,buf,k,&j);
	break;
      default:
	if (last_duration!=DUR_4)
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
