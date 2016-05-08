I64 mp_n;

I64 MPSum(I64 my_mp_cnt)
{//We could use the formula n*(n+1)/2
  I64 lo=mp_n*Gs->num/my_mp_cnt,
	hi=mp_n*(Gs->num+1)/my_mp_cnt,
	res=0,i;
  for (i=lo;i<hi;i++)
    res+=i;
  return res;
}

I64 Sum(I64 n,I64 my_mp_cnt)
{
  CSrvCmd *tempm[MP_MAX_PROCESSORS];
  I64 res=0,i;
  mp_n=n+1;
  for (i=0;i<my_mp_cnt;i++)
    tempm[i]=JobQue(&MPSum,my_mp_cnt,i,0);
  for (i=0;i<my_mp_cnt;i++)
    res+=JobResGet(tempm[i]);
  return res;
}

#define SAMPLE_SIZE 100

F64 Test(I64 n,I64 my_mp_cnt)
{
  I64 i,val=0;
  F64 start,end;
  start=tS;
  for (i=0;i<SAMPLE_SIZE;i++)
    val+=Sum(n,my_mp_cnt);
  end=tS;
  "Val:%,d\n",val/SAMPLE_SIZE;
  "$$RED$$N:%12,d Time:%10.8f$$FG$$\n",n,(end-start)/SAMPLE_SIZE;
  return end-start;
}

#define MIN_VAL		1000
#define MIN_TEST	3
#define MAX_TEST	7
#define NUM_TESTS	(MAX_TEST-MIN_TEST+1)
#define MAX_PERCENT	200

U0 MPAdd()
{
  I64 i,n,
	h=Fs->pix_width,
	v=Fs->pix_height;
  F64 t1,t2,ress[NUM_TESTS];
  CDC *dc=DCAlias;

  for (i=0,n=MIN_VAL;i<NUM_TESTS;i++,n*=10) {
    t1=Test(n,1);
    t2=Test(n,mp_cnt);
    ress[i]=t2*100.0/t1;
    "$$GREEN$$%8.4f%%$$FG$$\n\n",ress[i];
  }
  PressAKey;

  DocClear;

  dc->color=BLUE;
  for (i=MAX_PERCENT/10;i<MAX_PERCENT;i+=MAX_PERCENT/10) {
    GrPrint(dc,0,v-ToF64(i)/MAX_PERCENT*v-FONT_HEIGHT,"%3d%%",i);
    GrLine(dc,0,v-ToF64(i)/MAX_PERCENT*v,h,v-ToF64(i)/MAX_PERCENT*v);
  }

  for (i=0;i<NUM_TESTS-1;i++) {
    dc->color=RED;
    dc->pen_width=2;
    GrLine3(dc,i*h/(NUM_TESTS-1),      v-ress[i  ]/MAX_PERCENT*v,0,
	  (i+1)*h/(NUM_TESTS-1),v-ress[i+1]/MAX_PERCENT*v,0);
    dc->color=GREEN;
    GrPrint(dc,i*h/(NUM_TESTS-1),v-FONT_HEIGHT,"10e%d",i+MIN_TEST);
    GrLine(dc,i*h/(NUM_TESTS-1),0,i*h/(NUM_TESTS-1),v);
  }
  PressAKey;

  DCFill;
  DCDel(dc);
}

MPAdd;
