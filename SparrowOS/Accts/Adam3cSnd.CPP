//Change anything you want in this
//acct file or others.

Cd("/SparrowOS/Adam/Snd");;
#include "SndFile"
#include "HDAudio"
//Uncomment this after running $LK,"::/SparrowOS/Adam/Snd/HDCfg.CPP.Z","FI:::/SparrowOS/Adam/Snd/HDCfg.CPP.Z"$.
//#include "HOME/HDAudioCfg.CPP.Z"
LBtr(&sys_semas[SYS_SEMA_SND],0);

Cd("HOME");;
