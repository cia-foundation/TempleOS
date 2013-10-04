Cd("::/TempleOS/Demo/OldSchool/Demo2");;
Cmp("OD1");
Cmp("OD2");
Cmp("ODMain");
Load("OD1");
Load("OD2");
Load("ODMain");
#include "ObjDemo.HPP"

ObjDemo("Success");

#ifndef BinRep
#include "::/TempleOS/Adam/Utils/BinRep"
#endif

PressAKey;
BinRep("OD2");

PressAKey;
BinRep("ODMain");

PressAKey;
Who("-r");

