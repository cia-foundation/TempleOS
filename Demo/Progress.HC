/*$LK,"progress1",A="MN:progress1"$-$LK,"progress4",A="MN:progress4"$ are just global vars
shown on the wallpaper. I use them in debugging a lot.
If you set $LK,"progress1_max",A="MN:progress1_max"$-$LK,"progress4_max",A="MN:progress4_max"$,
then a bar will display.
*/

U0 Main1()
{
  "\nWith description.\n";
  PressAKey;
  ProgressBarsRst;
  progress1_max=200;
  StrCpy(progress1_desc,"Progress Demo"); //PROGRESS_DESC_LEN is max
  while (progress1++!=progress1_max)
    Sleep(20);
  progress1_max=0; //hide
}

Main1;

U0 Main2()
{
  "\nWith description and time.\n";
  PressAKey;
  ProgressBarsRst;
  progress1_max=200;
  progress1_t0=tS;
  StrCpy(progress1_desc,"Progress Demo"); //PROGRESS_DESC_LEN is max
  while (progress1++!=progress1_max)
    Sleep(20);
  progress1_max=0; //hide
}

Main2;

U0 Main3()
{
  "\nNo description.\n";
  PressAKey;
  ProgressBarsRst;
//No progress1_desc
  progress1_max=200;
  while (progress1++!=progress1_max)
    Sleep(20);
  progress1_max=0; //hide
}

Main3;

"\n$$LTRED$$Now, from asm...  It's handy for debugging.$$FG$$\n"
"$$LTRED$$Watch the progress on the wallpaper.$$FG$$\n";
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
