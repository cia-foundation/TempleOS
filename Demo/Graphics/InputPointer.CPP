U0 InputPointer()
{
  while (!ip.lb) //Left Button
    GrPlot(,ip.pos.x,ip.pos.y);
  DCFill;
}

InputPointer;