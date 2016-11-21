#define RADIX	256
#define N	16

U8 a[N];

class Lst
{
  Lst *next;
  U8 *a;
} l[N],*r[RADIX];

U0 DumpIn()
{
  I64 i;
  "$$RED$$\n\nInput$$FG$$\n";
  for (i=0;i<N;i++)
    "%d:%d\n",i,a[i];
}

U0 DumpOut()
{
  I64 i,j=0;
  Lst *tmpl;
  "$$RED$$\n\nOutput$$FG$$\n";
  for (i=0;i<RADIX;i++) {
    tmpl=r[i];
    while (tmpl) {
      "%d:%d\n",j++,*tmpl->a;
      tmpl=tmpl->next;
    }
  }
}

U0 Init()
{
  I64 i;
  MemSet(r,0,sizeof(r));
  for (i=0;i<N;i++) {
    a[i]=RandU16&255;
    l[i].next=NULL;
    l[i].a=&a[i];
  }
}

U0 Sort()
{
  I64 i;
  for (i=0;i<N;i++) {
    l[i].next=r[*l[i].a];
    r[*l[i].a]=&l[i];
  }
}

U0 RadixSort()
{
  Init;
  DumpIn;
  Sort;
  DumpOut;
}

RadixSort;
