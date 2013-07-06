#define NUM	1000000

I64 my_mp_cnt=1<<Bsr(mp_cnt);//Power of 2

I32 *a1,*a2;
I32 *b[my_mp_cnt],bn[my_mp_cnt];
I64 mp_not_done_flags;

I64 Compare(I32 *e1,I32 *e2)
{
  return *e1-*e2;
}


U0 QSortU32(I32 *base,I64 num)
{
// By customizing, we dramatically improve it!
// Cut and paste from $LK,"QSortI64","MN:QSortI64"$().
  I64 i;
  I32 *less,*greater,pivot;
  if (num>1) {
    do {
      less=base;
      greater=base+num;
      pivot=base[num/2];
      while (less<greater) {
	if (*less<=pivot)
	  less++;
	else {
	  greater--;
	  SwapU32(less,greater);
	}
      }
      i=less-base;
      if (i==num) { //All less or equal to pivot

      //Point greater to first less
	do greater--;
	while (--i && *greater==pivot);

	if (i) {
	  less=base+num/2; //Pivot was not moved, point to it
	  if (less<greater)
	    SwapU32(less,greater);
	  num=i;
	} else //All equal
	  break;
      } else if (i<num/2) {
	QSortU32(base,i);
	num-=i;
	base=greater;
      } else {
	QSortU32(greater,num-i);
	num=i;
      }
    } while (num>1);
  }
}

U0 MPSort(I64 dummy=0)
{
  nounusedwarn dummy;
  QSortU32(b[Gs->num],bn[Gs->num]);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 MPRadixSortDemo(I64 dummy=0)
{
  nounusedwarn dummy;
  I64 i,j,k1,k2;
  F64 t0;
  a1=MAlloc(NUM*sizeof(I32));
  for (i=0;i<NUM;i++)
    a1[i]=RandI32;


  a2=MAlloc(NUM*sizeof(I32));

  "$$FG,GREEN$$QSort$$FG$$\n";
  t0=tT;
  MemCpy(a2,a1,sizeof(I32)*NUM);
  QSort(a2,NUM,sizeof(I32),&Compare);
  "Time:%9.6f\n",tT-t0;
  D(a2+NUM/4);


  "$$FG,GREEN$$QSortU32$$FG$$\n";
  t0=tT;
  MemCpy(a2,a1,sizeof(I32)*NUM);
  QSortU32(a2,NUM);
  "Time:%9.6f\n",tT-t0;
  D(a2+NUM/4);

  for (i=0;i<my_mp_cnt;i++) {
    //$WW,0$We must do full size, just in case.
    //There will be uneven split between cores
    //depending on the distribution of rand numbers.
    b[i]=MAlloc(NUM*sizeof(I32));
    bn[i]=0;
  }

  if (my_mp_cnt<2) throw(EXCEPT_MULTICORE);

  "$$FG,GREEN$$MP Radix QSortU32$$FG$$\n";
  t0=tT;
  k1=32-Bsr(my_mp_cnt);
  k2=my_mp_cnt/2;
  for (i=0;i<NUM;i++) {
    j=a1[i]>>k1+k2; //This is a preliminary radix sort.
    b[j][bn[j]++]=a1[i];
  }
  mp_not_done_flags=1<<my_mp_cnt-1;
  for (i=0;i<my_mp_cnt;i++)
    Spawn(&MPSort,NULL,NULL,i);
  while (mp_not_done_flags)
    Yield;
  j=0;
  for (i=0;i<my_mp_cnt;i++) {
    MemCpy(&a2[j],b[i],bn[i]*sizeof(I32));
    j+=bn[i];
  }
  "Time:%9.6f\n",tT-t0;
  D(a2+NUM/4);

  Free(a1);
  Free(a2);
  for (i=0;i<my_mp_cnt;i++)
    Free(b[i]);
}

MPRadixSortDemo;

/*$HL,0$ Results on 8 Cores 2.660GHz Core i7:
$FG,2$QSort$FG$
Time: 1.535474
00000000 EA 1D F6 BF 16 2E F6 BF 0B 3C F6 BF CD 44 F6 BF ê.ö¿..ö¿.<ö¿ÍDö¿
00000010 B1 45 F6 BF 52 4A F6 BF 96 71 F6 BF 9F 78 F6 BF ±Eö¿RJö¿–qö¿Ÿxö¿
00000020 52 87 F6 BF 23 9F F6 BF EF AD F6 BF 2D B4 F6 BF R‡ö¿#Ÿö¿ï­ö¿-´ö¿
00000030 4B EE F6 BF 60 FA F6 BF 31 08 F7 BF 1A 0E F7 BF Kîö¿`úö¿1.÷¿..÷¿
00000040 5C 39 F7 BF 26 3B F7 BF C7 3D F7 BF CF 3D F7 BF \9÷¿&;÷¿Ç=÷¿Ï=÷¿
00000050 7D 51 F7 BF 2F 79 F7 BF 7B 7A F7 BF CF 8C F7 BF }Q÷¿/y÷¿{z÷¿ÏŒ÷¿
00000060 09 95 F7 BF 04 A3 F7 BF D2 A8 F7 BF 61 EA F7 BF .•÷¿.£÷¿Ò¨÷¿aê÷¿
00000070 2D ED F7 BF F2 03 F8 BF 4C 09 F8 BF BB 09 F8 BF -í÷¿ò.ø¿L.ø¿».ø¿
$FG,2$QSortU32$FG$
Time: 0.132599
00000000 EA 1D F6 BF 16 2E F6 BF 0B 3C F6 BF CD 44 F6 BF ê.ö¿..ö¿.<ö¿ÍDö¿
00000010 B1 45 F6 BF 52 4A F6 BF 96 71 F6 BF 9F 78 F6 BF ±Eö¿RJö¿–qö¿Ÿxö¿
00000020 52 87 F6 BF 23 9F F6 BF EF AD F6 BF 2D B4 F6 BF R‡ö¿#Ÿö¿ï­ö¿-´ö¿
00000030 4B EE F6 BF 60 FA F6 BF 31 08 F7 BF 1A 0E F7 BF Kîö¿`úö¿1.÷¿..÷¿
00000040 5C 39 F7 BF 26 3B F7 BF C7 3D F7 BF CF 3D F7 BF \9÷¿&;÷¿Ç=÷¿Ï=÷¿
00000050 7D 51 F7 BF 2F 79 F7 BF 7B 7A F7 BF CF 8C F7 BF }Q÷¿/y÷¿{z÷¿ÏŒ÷¿
00000060 09 95 F7 BF 04 A3 F7 BF D2 A8 F7 BF 61 EA F7 BF .•÷¿.£÷¿Ò¨÷¿aê÷¿
00000070 2D ED F7 BF F2 03 F8 BF 4C 09 F8 BF BB 09 F8 BF -í÷¿ò.ø¿L.ø¿».ø¿
$FG,2$MP Radix QSortU32$FG$
Time: 0.024471
00000000 EA 1D F6 BF 16 2E F6 BF 0B 3C F6 BF CD 44 F6 BF ê.ö¿..ö¿.<ö¿ÍDö¿
00000010 B1 45 F6 BF 52 4A F6 BF 96 71 F6 BF 9F 78 F6 BF ±Eö¿RJö¿–qö¿Ÿxö¿
00000020 52 87 F6 BF 23 9F F6 BF EF AD F6 BF 2D B4 F6 BF R‡ö¿#Ÿö¿ï­ö¿-´ö¿
00000030 4B EE F6 BF 60 FA F6 BF 31 08 F7 BF 1A 0E F7 BF Kîö¿`úö¿1.÷¿..÷¿
00000040 5C 39 F7 BF 26 3B F7 BF C7 3D F7 BF CF 3D F7 BF \9÷¿&;÷¿Ç=÷¿Ï=÷¿
00000050 7D 51 F7 BF 2F 79 F7 BF 7B 7A F7 BF CF 8C F7 BF }Q÷¿/y÷¿{z÷¿ÏŒ÷¿
00000060 09 95 F7 BF 04 A3 F7 BF D2 A8 F7 BF 61 EA F7 BF .•÷¿.£÷¿Ò¨÷¿aê÷¿
00000070 2D ED F7 BF F2 03 F8 BF 4C 09 F8 BF BB 09 F8 BF -í÷¿ò.ø¿L.ø¿».ø¿
$HL,1$*/