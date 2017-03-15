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
  CJob *tmpm[MP_PROCESSORS_NUM];
  I64 res=0,i;
  mp_n=n+1;
  for (i=0;i<my_mp_cnt;i++)
    tmpm[i]=JobQue(&MPSum,my_mp_cnt,i,0);
  for (i=0;i<my_mp_cnt;i++)
    res+=JobResGet(tmpm[i]);
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

#define VAL_MIN		1000
#define TEST_MIN	3
#define TEST_MAX	7
#define TESTS_NUM	(TEST_MAX-TEST_MIN+1)
#define PERCENT_MAX	200

U0 MPAdd()
{
  I64 i,n,
	h=Fs->pix_width,
	v=Fs->pix_height;
  F64 t1,t2,ress[TESTS_NUM];
  CDC *dc=DCAlias;

  for (i=0,n=VAL_MIN;i<TESTS_NUM;i++,n*=10) {
    t1=Test(n,1);
    t2=Test(n,mp_cnt);
    ress[i]=t2*100.0/t1;
    "$$GREEN$$%8.4f%%$$FG$$\n\n",ress[i];
  }
  PressAKey;

  DocClear;

  dc->color=BLUE;
  for (i=PERCENT_MAX/10;i<PERCENT_MAX;i+=PERCENT_MAX/10) {
    GrPrint(dc,0,v-ToF64(i)/PERCENT_MAX*v-FONT_HEIGHT,"%3d%%",i);
    GrLine(dc,0,v-ToF64(i)/PERCENT_MAX*v,h,v-ToF64(i)/PERCENT_MAX*v);
  }

  for (i=0;i<TESTS_NUM-1;i++) {
    dc->color=RED;
    dc->thick=2;
    GrLine3(dc,i*h/(TESTS_NUM-1),      v-ress[i  ]/PERCENT_MAX*v,0,
	  (i+1)*h/(TESTS_NUM-1),v-ress[i+1]/PERCENT_MAX*v,0);
    dc->color=GREEN;
    GrPrint(dc,i*h/(TESTS_NUM-1),v-FONT_HEIGHT,"10e%d",i+TEST_MIN);
    GrLine(dc,i*h/(TESTS_NUM-1),0,i*h/(TESTS_NUM-1),v);
  }
  PressAKey;

  DCFill;
  DCDel(dc);
}

MPAdd;
