//This is executed by Adam at boot.
//See $LK,"Adam1 Start-up",A="FF:::/Kernel/KEnd.CPP,Adam1:2"$
extern I8i OptEqu(I64i num,I8i val);
OptEqu(0,0); //(0,0)=EchoOff   (0,1)=EchoOn
#include "/Kernel/Adam1a.HPP"
#include "/Compiler/CmpDefs.HPP"
#include "/Kernel/Adam1b.HPP"
#include "/Kernel/Adam1c.HPP"
#include "/Compiler/Cmp.HPP"
Cd("/Adam");;
#include "Adam2"
