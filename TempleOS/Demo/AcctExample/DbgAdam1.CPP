//This is executed by Adam at boot.
//See $LK,"Adam1 Start-up","FF:::/TempleOS/Kernel/KEnd.CPP,Adam1:2"$
extern I8i OptEqu(I64i num,I8i val);
OptEqu(0,0); //(0,0)=EchoOff   (0,1)=EchoOn
#include "/TempleOS/Kernel/BootDefs.HPP"
#include "/TempleOS/Kernel/Adam1a.HPP"
#include "/TempleOS/Compiler/CmpDefs.HPP"
#include "/TempleOS/Kernel/Adam1b.HPP"
#include "/TempleOS/Kernel/Adam1c.HPP"
#include "/TempleOS/Compiler/Compiler.HPP"
Dbg("Type 'G;'");
