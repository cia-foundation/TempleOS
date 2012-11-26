//This is executed by Adam at boot.
//See $LK,"Adam1 Start-up","FF:::/SparrowOS/OSMain/OSEnd.CPP.Z,Adam1:2"$
extern I8i OptEqu(I64i num,I8i val);
OptEqu(0,0); //(0,0)=EchoOff   (0,1)=EchoOn
#include "/SparrowOS/OSMain/BootDefs.HPP.Z"
#include "/SparrowOS/OSMain/Adam1a.HPP.Z"
#include "/SparrowOS/Compiler/CmpDefs.HPP.Z"
#include "/SparrowOS/OSMain/Adam1b.HPP.Z"
#include "/SparrowOS/OSMain/Adam1c.HPP.Z"
#include "/SparrowOS/Compiler/Compiler.HPP.Z"
Cd("/SparrowOS/Adam");;
#include "Adam2.CPP.Z"
