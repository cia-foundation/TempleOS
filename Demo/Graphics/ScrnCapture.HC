//See $LK,"::/Demo/Games/Stadium/StadiumGen.HC"$.

U0 ViewScrnCapture(U8 *filename)
{
  CDC	*dc=DCAlias(,Fs),
	*dc2=GRRead(filename);
  GrBlot(dc,0,0,dc2);
  DCDel(dc2);
  DCDel(dc);
}

U0 Main()
{
  "Capture scrn...\n";
  PressAKey;
  GRScrnCapture("~/DemoScrnShot");

  "View captured scrn...\n";
  PressAKey;
  ViewScrnCapture("~/DemoScrnShot");

  PressAKey;
  DCFill;
}

Main;
