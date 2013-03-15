#define BORDER_CHARS	1

U8 *buf;
I64 size,mp_done_flags;
CDoc *doc;

U0 PDNormalize()
{
  U8 *r=MAlloc(size+2*BORDER_CHARS),
     *src=buf,*dst=r;
  I64 ch;
  *dst++='['; //Border
  while (size--) {
    ch=*src++;
    if ('a'<=ch<='z') //lower is most common so do first
      *dst++=ch;
    else if ('A'<=ch<='Z')
      *dst++=ch+'a'-'A'; //Convert to lower
  }
  *dst++=']'; //Border
  Free(buf);
  buf=r;
  size=dst-r-2*BORDER_CHARS;
}


U0 PDAnswer(U8 *a,I64 len)
{
  DocLock(doc);
  a-=(len-1)/2;
  DocPrintF(doc,"CPU:%d Len:%2d ",Gs->num,len);
  while (len--)
    DocPrintF(doc,"%C",*a++); //%C is toupper char
  DocPutS(doc,"\n");
  DocUnlock(doc);
}

U0 MPPalindrome(I64 dummy=0)
{
  nounusedwarn dummy;
  U8 *src=buf+BORDER_CHARS+size*Gs->num/mp_cnt,
     *f,*b;
  I64 len,best=0,
     my_size=(size+mp_cnt-1)/mp_cnt;
  while (my_size--) {

//Odd
    f=src+1;
    b=src-1;
    while (*f==*b) {
      f++;
      b--;
    }
    len=f-b+1-2;
    if (len>best) {
      best=len;
      PDAnswer(src,len);
    }

//Even
    f=src+1;
    b=src;
    while (*f==*b) {
      f++;
      b--;
    }
    len=f-b+1-2;
    if (len>best) {
      best=len;
      PDAnswer(src,len);
    }

    src++;
  }
  LBts(&mp_done_flags,Gs->num);
}

U0 Palindrome(U8 *filename)
{
  I64 i;
  F64 t0=tT;

  buf=FileRead(filename,&size);
  PDNormalize;

  doc=DocCur;
  mp_done_flags=0;
  for (i=1;i<mp_cnt;i++)
    MPJob(&MPPalindrome,NULL,i);
  MPPalindrome;
  while (mp_done_flags!=1<<mp_cnt-1)
    Yield;

  Free(buf);
  "Time:%9.5f\n",tT-t0;
}

Palindrome(Define("ST_BIBLE_FILE"));
