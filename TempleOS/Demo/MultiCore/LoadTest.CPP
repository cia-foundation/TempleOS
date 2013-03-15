I64 load_test_done;

U0 Job1(I64)
{ //Compile Compiler
  TaskInitExt; //Enable file access
  Preempt(ON);	 //Off by default
  Silent(ON);
  while (!load_test_done) {
    Cd("::/TempleOS/Compiler");
    Cmp("Compiler");
  }
  lock {load_test_done--;}
}

U0 Job2(I64)
{ //Log a file
  TaskInitExt; //Enable file access
  Preempt(ON);	 //Off by default
  while (!load_test_done) {
    AdamLog("%F","::/TempleOS/Demo/MultiCore/LoadTest.CPP");
    Yield;
  }
  lock {load_test_done--;}
}

U0 Job3(I64)
{ //Log a file
  TaskInitExt; //Enable file access
  Preempt(ON);	 //Off by default
  while (!load_test_done) {
    Cd("::/TempleOS/Compiler");
    AdamLog("%F","Compiler.PRJ.Z");
    Yield;
  }
  lock {load_test_done--;}
}

U0 Main()
{
  load_test_done=0;
  "Running MultiCore Test...\n";
  MPSpawn(&Job1);
  MPSpawn(&Job2);
  MPSpawn(&Job1);
  MPSpawn(&Job3);
  Spawn(&Job1);
  Spawn(&Job2);
  Spawn(&Job1);
  Spawn(&Job3);
  PressAKey;
  load_test_done=8;
//We don't want to yank code from under MP cores.
  while (load_test_done)
    Yield;
}

Main;
