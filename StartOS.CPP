//This is executed by the Adam task at boot.
//See $LK,"Adam Start-up",A="FF:::/Kernel/KEnd.CPP,\"StartOS"$.

#help_index "Compiler/Directive"
public extern I8i Option(I64i num,I8i val);
Option(0,0); //(0,0)=EchoOff   (0,1)=EchoOn

#include "/Kernel/KernelA.HPP"
#include "/Compiler/CompilerA.HPP"
#include "/Kernel/KernelB.HPP"
#include "/Kernel/KernelC.HPP"
#include "/Compiler/CompilerB.HPP"

Option(OPTf_WARN_PAREN,ON);
Option(OPTf_WARN_DUP_TYPES,ON);
HashTablePurge(adam_task->hash_table);

#help_index "Misc"
//Use Dbg("Type 'G;'"); to debug before window mgr is running.
public F64 os_version=4.050;//Operating system version.

#include "/Adam/MakeAdam"

//Dbg("Type 'G;'");
DocTermNew;
WinVert(2,10);

sys_winmgr_task=Spawn(&WinMgrTask,NULL,"Window Mgr");
Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_BORDER
	-WIF_SELF_GRAB_SCROLL-WIF_SELF_CTRLS;
LBts(&Fs->display_flags,DISPLAYf_CHILDREN_NOT_ON_TOP);
LBts(&Fs->display_flags,DISPLAYf_SHOW);
RegInit;
LBts(&sys_run_level,RLf_REGISTRY);
WallPaperInit;

if (DrvIsWritable)
  MkDir("/Temp"); //Good to have a Temp

Option(OPTf_WARN_PAREN,OFF);
Option(OPTf_WARN_DUP_TYPES,OFF);
LBts(&sys_run_level,RLf_HOME);

#help_index ""
#include "/Home/MakeHome"

//After this file, the Adam task enters $LK,"server mode",A="HI:Task/Srv"$.
