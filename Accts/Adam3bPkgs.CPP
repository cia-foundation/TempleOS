//Change anything you want in this
//acct file or others.

Cd("/Adam/Boot");;
#include "InsBoot"
Cd("/Adam/Utils");;
#include "Profiler"
#include "Diff"
#include "Grep"
#include "LineRep"
#include "Merge"
#include "S2T"
#include "LinkChk"
#include "HeapLog"
#include "MemRep"
#include "ToHtml"
#include "ToTXT"
#include "FileBMP"

MapFileLoad("::/Kernel/Kernel");
MapFileLoad("::/Compiler/Compiler");

Cd("HOME");;

#help_index ""
