//Place this file in /Home and change
//anything you want.

Cd("/Adam/Boot");;
#include "Boot"
Cd("/Adam/Utils");;
#include "Profiler"
#include "Diff"
#include "Find"
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

Cd("::/Home");;

#help_index ""
