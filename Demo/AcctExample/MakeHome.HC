Cd(__DIR__);;

//If these are not present in /Home, it uses the version in the root dir.  You
//can make your own, modified, version of these files in your /Home directory.
#include "~/HomeLocalize"
#include "/Adam/Opt/Boot/MakeBoot"
#include "/Adam/Opt/Utils/MakeUtils"
#include "~/HomeWrappers"
MapFileLoad("::/Kernel/Kernel");
MapFileLoad("::/Compiler/Compiler");

#include "::/Apps/Psalmody/Load"
#include "~/TOS/MakeTOS"
#include "~/HomeKeyPlugIns"
#include "~/HomeSys"
Cd("..");;
