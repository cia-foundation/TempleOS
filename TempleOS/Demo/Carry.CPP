//This shows the $LK,"Carry","MN:Carry"$ function.
//It's tricky to use.

U0 BigCnt()
{
  U64 lo=0,hi=0;
  while (hi<0x10) {
    lo+=1<<58;
    hi+=Carry;
    "%016X %016X\n",hi,lo;
  }
}

BigCnt;
PressAKey;

U0 BigShift()
{
  U64 lo=1,hi=0;
  while (lo||hi) {
    hi<<=1;
    lo<<=1;
    hi+=Carry;
    "%016X %016X\n",hi,lo;
  }
}

BigShift;
