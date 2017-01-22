U0 Mouse()
{
  "Press left mouse bttn to exit.\n";
  while (!ms.lb) { //Left bttn to exit.
    GrPlot(,ms.pos.x,ms.pos.y);
    Refresh;
  }
  DCFill;
}

Mouse;
