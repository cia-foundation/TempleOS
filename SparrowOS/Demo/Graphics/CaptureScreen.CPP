//See $LK,"::/SparrowOS/Demo/GameStarters/Stadium/StadiumGen.CPP.Z","FI:::/SparrowOS/Demo/GameStarters/Stadium/StadiumGen.CPP.Z"$.

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
  GRACaptureScreen("HOME/ScreenShot.GRA.Z");

  "View captured screen...\n";
  PressAKey;
  ViewScreenCapture("HOME/ScreenShot.GRA.Z");

  PressAKey;
  DCFill;
}

Main;
