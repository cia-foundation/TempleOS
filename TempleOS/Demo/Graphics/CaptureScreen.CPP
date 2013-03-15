//See $LK-A,"::/TempleOS/Demo/GameStarters/Stadium/StadiumGen.CPP"$.

U0 ViewScreenCapture(U8 *filename)
{
  CDC	*dc=DCAlias(gr_dc2,Fs),
	*dc2=GRARead(filename);
  GrBlot(dc,0,0,dc2);
  DCDel(dc2);
  DCDel(dc);
}


U0 Main()
{
  "Capture screen...\n";
  PressAKey;
  GRACaptureScreen("HOME/ScreenShot");

  "View captured screen...\n";
  PressAKey;
  ViewScreenCapture("HOME/ScreenShot");

  PressAKey;
  DCFill;
}

Main;
