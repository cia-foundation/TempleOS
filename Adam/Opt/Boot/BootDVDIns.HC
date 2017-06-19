//See $LK,"Install Documentation",A="FI:::/Doc/Install.DD"$.
//Study my account examples: $LK,"Cfg Strs",A="FL:::/Demo/AcctExample/TOS/TOSCfg.HC,1"$, $LK,"Update Funs",A="FL:::/Demo/AcctExample/TOS/TOSDistro.HC,1"$

#include "BootDVD"
#include "DskISORedSea"

#help_index "Install"

#define KERNEL_BIN_C	"Kernel.BIN.C"
#define BOOT_DIR	"/0000Boot"
#define BOOT_DIR_KERNEL_BIN_C	BOOT_DIR "/0000" KERNEL_BIN_C

U0 MakeAll()
{
  if (Cmp("/Compiler/Compiler","Compiler",,':'))
    throw;
  if (Cmp("/Kernel/Kernel","Kernel",,':'))
    throw;
}

public U0 BootDVDIns(U8 drv_let=0)
{//See $LK,"::/Misc/DoDistro.HC"$.
  try {
    if (!Drv(drv_let))
      throw;
    MakeAll;
    Move("/Kernel/Kernel.BIN.Z",BOOT_DIR_KERNEL_BIN_C);
  } catch {
    PutExcept;
    Beep;
  }
}
