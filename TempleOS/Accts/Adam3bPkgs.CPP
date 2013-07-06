//Change anything you want in this
//acct file or others.

Cd("/TempleOS/Adam/Boot");;
#include "InsBoot"
Cd("/TempleOS/Adam/Utils");;
#include "Profiler"
#include "Diff"
#include "Grep"
#include "LineCnt"
#include "Merge"
#include "S2T"
#include "LinkChk"
#include "HeapLog"
#include "MemRep"
#include "ToHtml"
#include "ToTXT"
#include "BinRep"
#include "FileBMP"
Cd("/TempleOS/Apps");;

MapFileLoad("::/TempleOS/Kernel/Kernel");
MapFileLoad("::/TempleOS/Compiler/Compiler");

Cd("HOME");;
