U0 RandPts()
{
  F64 t0=tS;
  do {
    gr.dc->color=RandI16&15;
    GrPlot(gr.dc,RandI32%GR_WIDTH,RandI32%GR_HEIGHT);
  } while (tS-t0<1);
  DCFill;
}

U0 RandDemo()
{
  I64 i;

  "Seed(0) will do timer randomized random numbers.  A task starts with "
	"$LK,"Seed(0)",A="FF:::/Kernel/KTask.HC,Seed("$ by dft.\n";
  PressAKey;
  Seed;
  RandPts;
  DCFill;

  "\nSeed(num) will do non-timer starting at specific seed.  "
	"Non-timer rand is awful.  You can run a specific video game by "
	"typing Seed(num) at the cmd line before the game.\n";
  PressAKey;
  Seed(1);
  RandPts;
  DCFill;

  "\n\n\nWatch when we specify specific seeds:";

  "\n\nSeed(1);\n";
  Seed(1);
  for (i=0;i<5;i++)
    "%d: %d\n",i,RandI16;

  "\nSeed(2); Different, right?\n";
  Seed(2);
  for (i=0;i<5;i++)
    "%d: %d\n",i,RandI16;

  "\nSeed(1); Repeats earlier values.\n";
  Seed(1);
  for (i=0;i<5;i++)
    "%d: %d\n",i,RandI16;

  "\nSeed(0); Zero is special.\n";
  Seed(0);
  for (i=0;i<5;i++)
    "%d: %d\n",i,RandI16;

  "\nSeed(0); Non-repeating.\n";
  Seed(0);
  for (i=0;i<5;i++)
    "%d: %d\n",i,RandI16;

  Seed; //Dft should be kept to zero.
}

RandDemo;
