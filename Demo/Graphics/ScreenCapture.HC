//See $LK,"::/Demo/Games/Stadium/StadiumGen.HC"$.

U0 ViewScreenCapture(U8 *filename)
{
  CDC	*dc=DCAlias(,Fs),
	*dc2=GRARead(filename);
  GrBlot(dc,0,0,dc2);
  DCDel(dc2);
  DCDel(dc);
}

U0 Main()
{
  "Capture screen...\n";
  PressAKey;
  GRAScreenCapture("~/DemoScreenShot");

  "View captured screen...\n";
  PressAKey;
  ViewScreenCapture("~/DemoScreenShot");

  PressAKey;
  DCFill;
}

Main;
