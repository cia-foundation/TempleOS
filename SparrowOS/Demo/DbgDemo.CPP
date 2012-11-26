/*
Press <CTRL-ALT-C> to break.

Rerun Program

Press <CTRL-ALT-D> to enter Dbg.
> $LK,"Dr","MN:Dr"$;	 //Dump Registers
> $LK,"BptsR","MN:BptsR"$;  //Clear break-points.
> $LK,"Go2","MN:Go2"$;	 //Cont program


Press <CTRL-ALT-D> to enter Dbg.
> $LK,"Uf","MN:Uf"$("DbgDemo");
> $LK,"U","MN:U"$(RIP);

See $LK,"Debugging","HI:Debugging"$

*/

U0 DbgDemo()
{
  I64 i=0;
  while (TRUE)
    if (!(i++%2000000))
      ".";
}

DbgDemo;

