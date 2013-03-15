import U0 PrintF(U8 *fmt,...);

U0 Routine3(U8 *st)
{
  PrintF("Routine3:%s\n",st);
}


//Any routine with var parameters
//automatically has vars argc and argv[]
//whether it is main or not.
I64 ModuleDemo(...)
{
  I64 i;
  if (argc>0) {
    for (i=0;i<argc;i++)
      PrintF("Arg %d:%08X\n",i,argv[i]);
    Routine1(argv[0]);
    if (argc>1) {
      Routine2(argv[1]);
      if (argc>2)
	Routine3(argv[2]);
    }
  } else
    PrintF("Pass string parameters for args 1 and 2.\n");
  return argc;
}

