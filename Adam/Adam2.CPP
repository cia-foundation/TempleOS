#exe {OptOn(OPTf_WARN_PAREN);};
#exe {OptOn(OPTf_WARN_DUP_TYPES);};

HashTablePurge(adam_task->hash_table);

#help_index "Misc"

//Use Dbg("Type 'G;'"); to debug before window mgr is running.

public F64 os_version=2.03; //Fix here, too: $LK,"::/Demo/AcctExample/DbgAdam2.CPP.Z"$

#include "AdamExt.HPP"
#include "Math2a"
#include "Training"
#include "Utils2a"
#include "MathODE"
Cd("::/Adam/Gr");;
#include "Gr"

Cd("::/Adam/Snd");;
#include "Snd"

Cd("::/Adam/Dsk");;
#include "Dsk2"

Cd("::/Adam");;
#include "Menu"
#include "Win2a"
#include "Win2b"
#include "Dbg2a"

#include "::/Adam/God/GodExt"

Cd("::/Adam/Doc");;
#include "Doc"
LBts(&sys_run_level,RLf_DOC);

Cd("::/Adam/Ctrls");;
#include "Ctrls"

Cd("::/Adam");;
#include "AutoFile2a"
#include "LogIn"

Cd("::/Adam/WordStat");;
#include "WordStat"

Cd("::/Adam/God");;
#include "God"

Cd("::/Adam");;
#include "Hash2a"
#include "TaskSettings"
#include "DevInfo"
#include "DocDefines"

//Dbg("Type 'G;'");

DocTermNew;
SetWinVert(2,10);

sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
Fs->win_inhibit=WIG_TASK_DFT-WIF_BORDER-WIF_GRAB_SCROLL-WIF_CTRLS;
LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);
LBts(&Fs->display_flags,DISPLAYf_SHOW);
SysLogin;
LBts(&sys_run_level,RLf_LOGIN);
RegistryInit;
LBts(&sys_run_level,RLf_REGISTRY);

Cd("::/Adam");;
#include "WallPaper"
#exe {OptOff(OPTf_WARN_PAREN);};
#exe {OptOff(OPTf_WARN_DUP_TYPES);};
LBts(&sys_run_level,RLf_ADAM2);

Cd("HOME");;
#include "Adam3"
