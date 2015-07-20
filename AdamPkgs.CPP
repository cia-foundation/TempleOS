//Place this file in /Home and change
//anything you want.

Cd("/Adam/Boot");;
#include "Boot"
Cd("/Adam/Utils");;
#include "Diff"
#include "FileBMP"
#include "Find"
#include "HeapLog"
#include "LineRep"
#include "LinkChk"
#include "MemRep"
#include "Merge"
#include "Profiler"
#include "StrUtils"
#include "ToHtml"
#include "ToTXT"

MapFileLoad("::/Kernel/Kernel");
MapFileLoad("::/Compiler/Compiler");

Cd("::/Home");;

#help_index ""
