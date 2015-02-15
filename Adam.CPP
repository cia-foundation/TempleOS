//This is executed by the Adam task at boot.
//See $LK,"Adam Start-up",A="FF:::/Kernel/KEnd.CPP,\"Adam:2"$.

//You do not place this file in /Home. (It would be ignored.)
//but look at the bottom of this file to see
//files you do customize.

extern I8i OptEqu(I64i num,I8i val);
OptEqu(0,0); //(0,0)=EchoOff   (0,1)=EchoOn
#include "/Kernel/KernelA.HPP"
#include "/Compiler/CompilerA.HPP"
#include "/Kernel/KernelB.HPP"
#include "/Kernel/KernelC.HPP"
#include "/Compiler/CompilerB.HPP"

Cd("/Adam");;
#exe {OptOn(OPTf_WARN_PAREN);};
#exe {OptOn(OPTf_WARN_DUP_TYPES);};

HashTablePurge(adam_task->hash_table);

#help_index "Misc"

//Use Dbg("Type 'G;'"); to debug before window mgr is running.

public F64 os_version=3.03;//Operating system version.

#include "AExts.HPP"
#include "AMath"
#include "Training"
#include "AUtils"
#include "AMathODE"
Cd("::/Adam/Gr");;
#include "Gr"

Cd("::/Adam/Snd");;
#include "Snd"

Cd("::/Adam/Dsk");;
#include "ADsk"

Cd("::/Adam");;
#include "Menu"
#include "WinA"
#include "WinB"
#include "ADbg"

#include "::/Adam/God/GodExt"

Cd("::/Adam/DolDoc");;
#include "Doc"
LBts(&sys_run_level,RLf_DOC);

Cd("::/Adam/Ctrls");;
#include "Ctrls"

Cd("::/Adam");;
#include "AutoFile"
#include "Acct"

Cd("::/Adam/AutoComplete");;
#include "AutoComplete"

Cd("::/Adam/God");;
#include "God"

Cd("::/Adam");;
#include "AHash"
#include "TaskSettings"
#include "DevInfo"
#include "ADefine"

//Dbg("Type 'G;'");
DocTermNew;
WinVert(2,10);

sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_BORDER-WIF_SELF_GRAB_SCROLL-WIF_SELF_CTRLS;
LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);
LBts(&Fs->display_flags,DISPLAYf_SHOW);
RegistryInit;
LBts(&sys_run_level,RLf_REGISTRY);

Cd("::/Adam");;
#include "WallPaper"

if (IsDrvWritable)
  MkDir("/Temp"); //Good to have a Temp

#exe {OptOff(OPTf_WARN_PAREN);};
#exe {OptOff(OPTf_WARN_DUP_TYPES);};
LBts(&sys_run_level,RLf_HOME);

#help_index ""
Cd("::/Home");;

//If these are not present in /Home, it uses the version in the root dir.
//You should make your own version of these files in you /Home directory.
#include "::/Home/AdamLocalize"
#include "::/Home/AdamPkgs"
#include "::/Home/AdamKeyPlugIns"
#include "::/Home/AdamWrappers"
#include "::/Home/AdamSys"

//After this file, the Adam task enters $LK,"server mode",A="HI:Task/Srv"$.
