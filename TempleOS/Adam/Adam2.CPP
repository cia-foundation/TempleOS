#exe {OptOn(OPTf_WARN_PAREN);};
#exe {OptOn(OPTf_WARN_DUP_TYPES);};
#help_index "Task/Login"

//Use Dbg("Type 'G;'"); to debug before window mgr is running.

public F64 os_version=1.00;

#include "AdamExt.HPP"

#include "Math2a"
#include "Training"
#include "Utils2a"
#include "MathODE"

Cd("::/TempleOS/Adam/Gr");;
#include "Gr"

Cd("::/TempleOS/Adam/Snd");;
#include "Snd"

Cd("::/TempleOS/Adam/Dsk");;
#include "Dsk2"

Cd("::/TempleOS/Adam");;
#include "Menu"
#include "Window"
#include "Dbg2a"

Cd("::/TempleOS/Adam/Doc");;
#include "Doc"
LBts(&sys_run_level,RLf_DOC);

Cd("::/TempleOS/Adam/Ctrls");;
#include "Ctrls"

Cd("::/TempleOS/Adam");;
#include "AutoFile2a"
#include "LogIn"

Cd("::/TempleOS/Adam/WordStat");;
#include "WordStat"

Cd("::/TempleOS/Adam");;
#include "Profiler"
#include "Hash2a"
#include "TaskSettings"
#include "DevInfo"
#include "DocDefines"

//Dbg("Type 'G;'");

UseTermDoc;
SetWinVert(2,10);

sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);
LBts(&Fs->display_flags,DISPLAYf_SHOW);
SysLogin;
LBts(&sys_run_level,RLf_LOGIN);
RegistryInit;
LBts(&sys_run_level,RLf_REGISTRY);

Cd("::/TempleOS/Adam");;
#include "WallPaper"
#exe {OptOff(OPTf_WARN_PAREN);};
#exe {OptOff(OPTf_WARN_DUP_TYPES);};
LBts(&sys_run_level,RLf_ADAM2);

Cd("HOME");;
#include "Adam3"
