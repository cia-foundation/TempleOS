/*
Press <CTRL-ALT-c> to break.

Rerun Program

Press <CTRL-ALT-d> to enter Dbg.
> $LK,"Dr",A="MN:Dr"$;	//Dump Regs
> $LK,"G2",A="MN:G2"$;	//Cont program

Press <CTRL-ALT-d> to enter Dbg.
> $LK,"Uf",A="MN:Uf"$("DbgDemo");
> $LK,"U",A="MN:U"$(_RIP);

See $LK,"Debugging",A="HI:Debugging"$
*/

U0 DbgDemo()
{
  I64 i=0;
  while (TRUE)
    if (!(i++%2000000))
      ".";
}

DbgDemo;

