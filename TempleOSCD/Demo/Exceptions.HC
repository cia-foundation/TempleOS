//$BK,1$WARNING:$BK,0$ Don't return out of a catch{}.  This
//might get fixed.

//You can use $LK,"PutExcept",A="MN:PutExcept"$() in a catch stmt
//and it will report args.
Bool Prompt(I64 i)
{
  "%d ",i;
  return YorN;
}

U0 D1()
{
  "D1\n";
  if (Prompt(1)) throw('Point1');
  if (Prompt(2)) throw('Point2');
}

U0 D2()
{
  "D2\n";
  try {
    D1;
    if (Prompt(4)) throw('Point4');
  } catch {
    "D2 hndlr\n"
	  "Ch:%c:%P\n",Fs->except_ch,Fs->except_callers[0];
    if (Fs->except_ch=='Point1') {
      "Caught in D2\n";
      Fs->catch_except=TRUE;
    }
  }
}

U0 Demo()
{
  try {
    D2;
    if (Prompt(3)) throw('Point3');
  } catch {
    "Demo hndlr\n"
	  "Ch:%c:%P\n",Fs->except_ch,Fs->except_callers[0];
    Fs->catch_except=TRUE;
  }
}

Demo;
