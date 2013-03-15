#exe {OptOn(OPTf_KEEP_PRIVATE);};

#include "/TempleOS/Kernel/BootDefs.HPP"
#include "/TempleOS/Kernel/Adam1a.HPP"
#exe {OptOn(OPTf_EXTERNS_TO_IMPORTS);};
#include "/TempleOS/Compiler/CmpDefs.HPP"
#include "/TempleOS/Kernel/Adam1b.HPP"
#include "/TempleOS/Kernel/Adam1c.HPP"
#exe {
  OptOff(OPTf_EXTERNS_TO_IMPORTS);
  OptOff(OPTf_KEEP_PRIVATE);
};


