#exe {OptOn(OPTf_WARN_PAREN);};
#exe {OptOn(OPTf_WARN_DUP_TYPES);};

HashTablePurge(adam_task->hash_table);

#help_index "Misc"

F64 os_version=2.11;//Operating system version.

#include "AdamExts.HPP"
#include "Math2a"
#include "Training"
#include "Utils2a"
#include "MathODE"

Dbg("Type 'G;'");

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

Cd("::/Adam/DolDoc");;
#include "Doc"
LBts(&sys_run_level,RLf_DOC);

Cd("::/Adam/Ctrls");;
#include "Ctrls"

Cd("::/Adam");;
#include "AutoFile2a"
#include "LogIn"

Cd("::/Adam/WordStat");;
#include "WordStat"

Cd("::/Adam");;
#include "Hash2a"
#include "TaskSettings"
#include "DevInfo"
#include "DocDefines"
