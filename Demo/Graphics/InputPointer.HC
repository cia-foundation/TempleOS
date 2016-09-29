U0 InputPointer()
{
  "Press left mouse button to exit.\n";
  while (!ip.lb) { //Left button to exit.
    GrPlot(,ip.pos.x,ip.pos.y);
    Refresh;
  }
  DCFill;
}

InputPointer;