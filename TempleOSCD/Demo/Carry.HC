//Shows the $LK,"Carry",A="MN:Carry"$ function.  It holds the CPU carry flag.
//$LK,"U",A="MN:U"$() Unassemble to make sure it is right.

U0 BigCnt()
{
  U64 lo=0,hi=0;
  "BigCnt\n";
  while (hi<0x10) {
    lo+=1<<58;
    hi+=Carry;
    "%016X %016X\n",hi,lo;
  }
}

BigCnt;

U0 BigShift()
{
  U64 lo=1,hi=0;
  "Big Shift\n";
  while (lo||hi) {
    hi<<=1;
    lo<<=1;
    hi+=Carry;
    "%016X %016X\n",hi,lo;
  }
}

BigShift;

U0 Branch()
{
  U64 i=0xFFCC3311,j;
  'Branch\n';
  for (j=0;j<64;j++) {
    i<<=1;
    if (Carry)
      '1';
    else
      '0';
  }
  '\n';
}

Branch;
