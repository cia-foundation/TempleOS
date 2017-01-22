/*$LK,"progress1",A="MN:progress1"$-$LK,"progress4",A="MN:progress4"$ are just global vars
shown on the wallpaper. I use them in debugging a lot.

If you set $LK,"progress1_max",A="MN:progress1_max"$-$LK,"progress4_max",A="MN:progress4_max"$,
then a progress bar will display.

If you set $LK,"progress1_t0",A="MN:progress1_t0"$-$LK,"progress4_t0",A="MN:progress4_t0"$
then the elapsed time will display.

If you set $LK,"progress1_tf",A="MN:progress1_tf"$-$LK,"progress4_tf",A="MN:progress4_tf"$
then the estimated time will display.
*/

U0 Main1()
{
  "\nWith description.\n";
  PressAKey;
  progress1_max=200;
  StrCpy(progress1_desc,"Progress Demo"); //PROGRESS_DESC_LEN is max
  while (progress1++!=progress1_max)
    Sleep(20);
  ProgressBarsRst;
}
Main1;

U0 Main2()
{
  "\nWith description and elapsed time.\n";
  PressAKey;
  progress1_max=200;
  progress1_t0=tS;
  StrCpy(progress1_desc,"Progress Demo"); //PROGRESS_DESC_LEN is max
  while (progress1++!=progress1_max)
    Sleep(20);
  ProgressBarsRst;
}
Main2;

U0 Main3()
{
  "\nWith description, elapsed time and registry total time.\n";
  PressAKey;
  progress1_max=200;
  progress1_t0=tS;
  RegExe("DemoCompany/ProgressBars");
  StrCpy(progress1_desc,"Progress Demo"); //PROGRESS_DESC_LEN is max
  while (progress1++!=progress1_max)
    Sleep(20);
  ProgressBarsRst("DemoCompany/ProgressBars");
}
Main3;

U0 Main4()
{
  "\nNo description.\n";
  PressAKey;
  progress1_max=200;
  while (progress1++!=progress1_max)
    Sleep(20);
  ProgressBarsRst;
}
Main4;


"\nNow, from asm...  It's handy for debugging.\n"
"Watch the progress on the wallpaper.\n";
WinBorder(ON);
PressAKey;

asm {
_PROGRESS_DEMO::
	MOV	RCX,12000000
@@05:	MOV	U64 [&progress1],RCX
	CALL	&Yield
	LOOP	@@05
	MOV	U64 [&progress1],RCX
	RET
}
_extern _PROGRESS_DEMO U0 AsmProgressDemo();
AsmProgressDemo;
