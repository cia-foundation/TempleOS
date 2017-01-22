#define M_DASH_THRESHOLD  0.140
#define M_SPACE_THRESHOLD 0.300

U8 *morse_lst=

"A.-\0"
"B-...\0"
"C-.-.\0"
"D-..\0"
"E.\0"
"F..-.\0"
"G--.\0"
"H....\0"
"I..\0"
"J.---\0"
"K-.-\0"
"L.-..\0"
"M--\0"
"N-.\0"
"O---\0"
"P.--.\0"
"Q--.-\0"
"R.-.\0"
"S...\0"
"T-\0"
"U..-\0"
"V...-\0"
"W.--\0"
"X-..-\0"
"Y-.--\0"
"Z--..\0"

"1.----\0"
"2..---\0"
"3...--\0"
"4....-\0"
"5.....\0"
"6-....\0"
"7--...\0"
"8---..\0"
"9----.\0"
"0-----\0"

"/-..-.\0"
"+.-.-.\0"
"..-.-.-\0"
",--..--\0"
"?..--..\0"
"(-.--.\0"
")-.--.-\0"
"--....-\0"
"\".-..-.\0"
"_..--.-\0"
"'.----.\0"
":---...\0"
";-.-.-.\0"
"=-...-\0";

U0 MorseTable()
{
  U8 *st;
  I64 i=0;
  for (i=0;i<13;i++ ) {
    if (st=LstSub(i,morse_lst))
      "$$RED$$%C$$FG$$ %-7s ",*st,st+1;
    if (st=LstSub(i+13,morse_lst))
      "$$RED$$%C$$FG$$ %-7s ",*st,st+1;
    if (st=LstSub(i+26,morse_lst))
      "$$RED$$%C$$FG$$ %-7s ",*st,st+1;
    if (st=LstSub(i+39,morse_lst))
      "$$RED$$%C$$FG$$ %-7s ",*st,st+1;
    '\n';
  }
  '\n';
}

F64 m_start,m_end,dt_down,dt_up;
Bool space_sent;

U8 *MorseTimes(CDoc *,CDocEntry *,CTask *mem_task)
{
  U8 *st=MAlloc(64,mem_task);
  StrPrint(st,"Down:%10.6f Up:%10.6f",dt_down,dt_up);
  return st;
}

I64 MorseWaitKey()
{
  I64 ch;
  F64 dt;
  while (TRUE) {
    if (m_start) {
      GetMsg(NULL,NULL,1<<MSG_KEY_UP);
      m_end=tS;
      Snd;
      dt_down=m_end-m_start;
      m_start=0;
      space_sent=FALSE;
      if (dt_down<M_DASH_THRESHOLD)
	return '.';
      else
	return '-';
    } else {
      if (!space_sent) {
	while (!ScanMsg(&ch,NULL,1<<MSG_KEY_DOWN)) {
	  dt=tS-m_end;
	  if (dt>=M_SPACE_THRESHOLD) {
	    space_sent=TRUE;
	    return CH_SPACE;
	  }
	  Yield;
	}
      } else
	ch=GetChar(,FALSE);
      m_start=tS;
      Snd(74);
      if (ch==CH_SHIFT_ESC || ch==CH_ESC) {
	'\n';
	Snd;
	throw; //exit program
      }
      dt_up=m_start-m_end;
      if (!space_sent && dt_up>=M_SPACE_THRESHOLD) {
	space_sent=TRUE;
	return CH_SPACE;
      }
    }
  }
}

U0 Morse()
{
  CDocEntry *doc_e;
  I64 ch;
  U8 buf[8],*dst,*src;

  MorseTable;
  m_start=0;
  m_end=tS;
  dt_down=0;
  dt_up=0;
  space_sent=TRUE;

  "$$GREEN$$";

  doc_e=DocPrint(DocPut,"$$TX+TC,\" \"$$");
  doc_e->tag_cb=&MorseTimes;

  "$$FG$$\n";

  dst=buf;
  while (TRUE) {
    ch=MorseWaitKey;
    if (ch==CH_SPACE) {
      *dst=0;
      src=morse_lst;
      while (*src) {
	if (!StrCmp(src+1,buf)) {
	  "$$GREEN$$%C$$FG$$",*src;
	  break;
	} else
	  src+=StrLen(src)+1;
      }
      '' CH_SPACE;
      dst=buf;
    } else {
      if (dst-buf<sizeof(buf)-1) {
	'' ch;
	*dst++=ch;
      }
    }
  }
  Snd;
}

Morse;
