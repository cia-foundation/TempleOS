//Simple and fancy way of searching

//See ::/Apps/Vocabulary/VocabQuiz.HC.Z
//for another example of dictionary usage.

#define ROWS_NUM	20
#define COLS_NUM	20
#define DIM_MAX	MaxI64(ROWS_NUM,COLS_NUM)

U8 m[ROWS_NUM][COLS_NUM];

U0 Init()
{
  I64 x,y;
  for (y=0;y<ROWS_NUM;y++)
    for (x=0;x<COLS_NUM;x++)
      m[y][x]='A'+RandU16%26;
}

U0 Display()
{
  I64 x,y;
  for (y=0;y<ROWS_NUM;y++) {
    for (x=0;x<COLS_NUM;x++)
      '' m[y][x];
    '\n';
  }
}

U0 GetWord(U8 *dst,I64 x,I64 y,I64 dx,I64 dy)
{
  while (0<=x<COLS_NUM && 0<=y<ROWS_NUM) {
    *dst++=m[y][x];
    y+=dy;
    x+=dx;
  }
  *dst=0;
}

//************************************
U0 Search(I64 dx,I64 dy)
{
  I64 x,y,i,n;
  U8 buf[DIM_MAX+1],*ptr;
  for (y=0;y<ROWS_NUM;y++)
    for (x=0;x<COLS_NUM;x++) {
      GetWord(buf,x,y,dx,dy);
      n=StrLen(buf);
      for (i=n;i>2;i--) {
	buf[i]=0;
	if ((ptr=ACDWordPtAt(buf)) && *ptr++==ACD_WORD_CHAR &&
	      !StrICmp(ptr,buf)) {
	  "%s ",buf;
	}
      }
    }
}

U0 SimpleWordSearch()
{
  Search(1,0);
  Search(0,1);
  Search(-1,0);
  Search(0,-1);
  Search(1,1);
  Search(-1,-1);
  Search(1,-1);
  Search(-1,1);
  '\n';
}

//************************************
U8 words[ROWS_NUM*COLS_NUM*8][DIM_MAX+1];
U8 *word_ptrs[ROWS_NUM*COLS_NUM*8];

U0 CollectWords(I64 dx,I64 dy,I64 *_n)
{
  I64 x,y,n=*_n;
  for (y=0;y<ROWS_NUM;y++)
    for (x=0;x<COLS_NUM;x++) {
      GetWord(&words[n],x,y,dx,dy);
      word_ptrs[n]=&words[n];
      n++;
    }
  *_n=n;
}

I64 WordsCompare(U8 *e1,U8 *e2)
{
  return StrCmp(e1,e2);
}

/*Fmt of word lst entry:
  U8 ACD_WORD_CHAR
  U8 word[] with terminating zero
  I16 block; //definition offset in ::/Adam/AutoComplete/ACDefs.DATA
*/

U0 DisplayWords(I64 n)
{
  I64 i=0,k;
  U8 *w1,*w2,*dict=acd.word_lst;
  for (i=0;i<n;i++)
    while (*dict) {
      w1=word_ptrs[i];
      w2=dict+1;
      while (*w2 && *w1==ToUpper(*w2)) {
	w1++;
	w2++;
      }
      if (ToUpper(*w2)>*w1)
	break;
      if (!*w2) {
	if (StrLen(dict+1)>2) {
	  k=i;
	  while (k<n && !StrNICmp(word_ptrs[k++],dict+1,StrLen(dict+1)))
	    "%s ",dict+1;
	}
	w2=dict+1;
	while (!StrICmp(w2,dict+1))
	  dict+=StrLen(dict+1)+4;
      } else
	dict+=StrLen(dict+1)+4;
    }
}

U0 FancyWordSearch()
{
  I64 n=0;
  CollectWords(1,0,&n);
  CollectWords(0,1,&n);
  CollectWords(-1,0,&n);
  CollectWords(0,-1,&n);
  CollectWords(1,1,&n);
  CollectWords(-1,-1,&n);
  CollectWords(1,-1,&n);
  CollectWords(-1,1,&n);
  QSortI64(word_ptrs,n,&WordsCompare);
  DisplayWords(n);
  '\n';
}

//************************************
U0 WordSearch()
{
  Init;
  Display;
  '\n';
  SimpleWordSearch;
  FancyWordSearch;
}

WordSearch;
