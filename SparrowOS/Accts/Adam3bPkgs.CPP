//Change anything you want in this
//acct file or others.

Cd("/SparrowOS/Adam/Boot");;
#include "InsBoot"
Cd("/SparrowOS/Adam");;
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
Cd("/SparrowOS/Apps");;

MapFileLoad("::/SparrowOS/OSMain/OSMain");
MapFileLoad("::/SparrowOS/Compiler/Compiler");

Cd("HOME");;
