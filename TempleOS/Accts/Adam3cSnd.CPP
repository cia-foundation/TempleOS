//Change anything you want in this
//acct file or others.

Cd("/TempleOS/Adam/Snd");;
#include "HDAudio"
//Uncomment this after running $LK-A,"::/TempleOS/Adam/Snd/HDCfg.CPP"$.
//#include "HOME/HDAudioCfg"
LBtr(&sys_semas[SYS_SEMA_SND],0);

Cd("HOME");;
