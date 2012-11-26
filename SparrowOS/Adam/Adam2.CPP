#exe {OptOn(OPTf_WARN_PAREN);};
#exe {OptOn(OPTf_WARN_DUP_TYPES);};
#help_index "Task/Login"

//Use Dbg("Type 'Go1;'"); to debug before window mgr is running.

public F64 os_version=1.02;

#include "AdamExt.HPP.Z"

#include "Math2a"
#include "Training"
#include "Utils2a"
#include "MathODE"

Cd("::/SparrowOS/Adam/Gr");;
#include "Gr"

Cd("::/SparrowOS/Adam/Snd");;
#include "Snd"

Cd("::/SparrowOS/Adam/Dsk");;
#include "Dsk2"

Cd("::/SparrowOS/Adam");;
#include "Menu"
#include "Window"
#include "Dbg2a"

Cd("::/SparrowOS/Adam/Doc");;
#include "Doc"
LBts(&sys_run_level,RLf_DOC);

Cd("::/SparrowOS/Adam/Ctrls");;
#include "Ctrls"

Cd("::/SparrowOS/Adam");;
#include "AutoFile2a"
#include "LogIn"

Cd("::/SparrowOS/Adam/WordStat");;
#include "WordStat"

Cd("::/SparrowOS/Adam");;
#include "Profiler"
#include "Hash2a"
#include "TaskSettings"
#include "DevInfo"
#include "DocDefines"

//Dbg("Type 'Go1;'");

UseTermDoc;

adam_task->win_top=2;
adam_task->win_bottom=10;
sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);
LBts(&Fs->display_flags,DISPLAYf_SHOW);
SysLogin;
LBts(&sys_run_level,RLf_LOGIN);
RegistryInit;
LBts(&sys_run_level,RLf_REGISTRY);

Cd("::/SparrowOS/Adam");;
#include "WallPaper"
#exe {OptOff(OPTf_WARN_PAREN);};
#exe {OptOff(OPTf_WARN_DUP_TYPES);};
LBts(&sys_run_level,RLf_ADAM2);

Cd("HOME");;
#include "Adam3"
