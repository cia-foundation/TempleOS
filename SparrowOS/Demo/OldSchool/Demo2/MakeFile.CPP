Cd("::/SparrowOS/Demo/OldSchool/Demo2");;
Cmp("OD1");
Cmp("OD2");
Cmp("ODMain");
Load("OD1");
Load("OD2");
Load("ODMain");
#include "ObjDemo.HPP.Z"

ObjDemo("Success");

#ifndef BinRep
#include "::/SparrowOS/Adam/BinRep.CPP.Z"
#endif

PressAKey;
BinRep("OD2");

PressAKey;
BinRep("ODMain");

PressAKey;
HashRep(Fs->hash_table,HTG_ALL,NULL,"-p-r");

