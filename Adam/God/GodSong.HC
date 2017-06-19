#help_index "God"

DefineLstLoad("ST_RHYTHM_COMPLEXITY","Simple\0Normal\0Complex\0");

class CMakeSongSettings
{
  I64  complexity		format "$$LS,D=\"ST_RHYTHM_COMPLEXITY\"$$\n";
  Bool rests			format "$$CB,\"Rests\"$$\n";
  Bool six_eight		format "$$CB,\"Six Eight\"$$\n";
  I64  octave			format "$$DA-TRM,A=\"Octave:%d\"$$\n";
  I64  octave_state;
};

U0 InsNote(CMakeSongSettings *mss,U8 *buf,I64 k,I64 *j)
{//k is a random note nibble
  if (!k && mss->rests) {
    buf[*j]='R';
    *j+=1;
  } else {
    k/=2;
    if (k<3) {
      if (mss->octave_state!=mss->octave) {
	mss->octave_state=mss->octave;
	buf[*j]=mss->octave_state+'0';
	*j+=1;
      }
      if (!k)
	buf[*j]='G';
      else
	buf[*j]=k-1+'A';
      *j+=1;
    } else {
      if (mss->octave_state!=mss->octave+1) {
	mss->octave_state=mss->octave+1;
	buf[*j]=mss->octave_state+'0';
	*j+=1;
      }
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

U8 god_simple_songs [5]={DUR_4,DUR_4,DUR_4,DUR_4,DUR_8_8};
U8 god_normal_songs [5]={DUR_4,DUR_4,DUR_8_8,DUR_3_3_3,DUR_16_16_16_16};
U8 god_complex_songs[9]={DUR_4,DUR_4,DUR_8_8,DUR_8_8,DUR_8DOT_16,DUR_3_3_3,
DUR_8_16_16,DUR_16_16_8,DUR_16_16_16_16};

public U8 *GodSongStr()
{//Make God generate 2 measures of a song. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  CMakeSongSettings mss;
  U8 *buf;
  I64 i,j=0,k,n,k2,duration,last_duration=-1,len;

  MemSet(&mss,0,sizeof(mss));
  mss.complexity=1;
  mss.rests=FALSE;
  mss.octave=music.octave;

  if (!PopUpForm(&mss))
    return NULL;

  buf=CAlloc(256);
  music.octave=mss.octave=ClampI64(mss.octave,1,7);
  progress4=0;
  if (mss.six_eight)
    progress4_max=6;
  else
    progress4_max=8;

  mss.octave_state=mss.octave+1;
  buf[j++]='0'+mss.octave_state;
  if (mss.six_eight) {
    len=6;
    buf[j++]='M';
    buf[j++]='6';
    buf[j++]='/';
    buf[j++]='8';
  } else
    len=8;
  FifoU8Flush(god.fifo);
  for (i=0;i<len;i++) {
    n=GodBits(8);
    if (mss.complexity==2)
      duration=god_complex_songs[n%9];
    else if (mss.complexity==1)
      duration=god_normal_songs[n%5];
    else
      duration=god_simple_songs[n%5];

    switch (duration) {
      case DUR_8_8:
	if (last_duration!=DUR_8_8)
	  buf[j++]='e';
	InsNote(&mss,buf,GodBits(4),&j);
	InsNote(&mss,buf,GodBits(4),&j);
	break;
      case DUR_8DOT_16:
	buf[j++]='e';
	buf[j++]='.';
	InsNote(&mss,buf,GodBits(4),&j);
	buf[j++]='s';
	InsNote(&mss,buf,GodBits(4),&j);
	duration=DUR_16_16_16_16;
	break;
      case DUR_3_3_3:
	if (last_duration!=DUR_3_3_3) {
	  buf[j++]='e';
	  buf[j++]='t';
	}
	InsNote(&mss,buf,GodBits(4),&j);
	InsNote(&mss,buf,GodBits(4),&j);
	InsNote(&mss,buf,GodBits(4),&j);
	break;
      case DUR_8_16_16:
	if (last_duration!=DUR_8_8)
	  buf[j++]='e';
	InsNote(&mss,buf,GodBits(4),&j);
	buf[j++]='s';
	InsNote(&mss,buf,GodBits(4),&j);
	InsNote(&mss,buf,GodBits(4),&j);
	duration=DUR_16_16_16_16;
	break;
      case DUR_16_16_8:
	if (last_duration!=DUR_16_16_16_16)
	  buf[j++]='s';
	InsNote(&mss,buf,GodBits(4),&j);
	InsNote(&mss,buf,GodBits(4),&j);
	buf[j++]='e';
	InsNote(&mss,buf,GodBits(4),&j);
	duration=DUR_8_8;
	break;
      case DUR_16_16_16_16:
	if (last_duration!=DUR_16_16_16_16)
	  buf[j++]='s';
	k =GodBits(4);
	k2=GodBits(4);
	InsNote(&mss,buf,k,&j);
	InsNote(&mss,buf,k2,&j);
	InsNote(&mss,buf,k,&j);
	InsNote(&mss,buf,k2,&j);
	break;
      default:
	if (last_duration!=DUR_4)
	  buf[j++]='q';
	InsNote(&mss,buf,GodBits(4),&j);
    }
    last_duration=duration;
    progress4++;
  }
  buf[j++]=0;
  progress4=progress4_max=0;
  return buf;
}

public U0 GodSong()
{//Make God generate 2measuresx2+2measuresx2. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  U8 *st1=GodSongStr,*st2=GodSongStr;
  if (st1 && st2)
    DocPrint(DocPut,"$$SO,\"<Song>\",A=\"%s%s%s%s\"$$",st1,st1,st2,st2);
  Free(st1);
  Free(st2);
}
