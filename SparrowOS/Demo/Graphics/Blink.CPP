U0 DrawIt(CTask *task,CDC *dc)
{ //This routine is called by the window mgr task.
  GrPrintF(dc,0,0*FONT_HEIGHT,"tNP:%4.3f tP:%4.3f",tNP,tP(task));

  //default 2.5Hz
  if (Blink)
    GrPutS(dc,0,1*FONT_HEIGHT,"Blink");

   //10 Hz
  if (Blink(10))
    GrPutS(dc,0,2*FONT_HEIGHT,"Blink(10)");

  //$LK,"tNP","MN:tNP"$ is a float var with time in seconds, function actually.
  //You can skip arguments if they have defaults.
  if (Blink(,tNP))
    GrPutS(dc,0,3*FONT_HEIGHT,"Blink(,tNP)");

  //$LK,"tP","MN:tP"$ is a pausable function.
  if (Blink(,tP(task)))
    GrPutS(dc,0,4*FONT_HEIGHT,"Blink(,tP(task))");
}

U0 BlinkDemo()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  DocClear;
  NewLine(10);
  "Pause and unpause task with $$FG,GREEN$$<CTRL-ALT-P>$$FG$$\n";
  PressAKey;
  SettingsPop;
}

BlinkDemo;
