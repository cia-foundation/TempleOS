#exe {OptOn(OPTf_KEEP_PRIVATE);};

#include "/SparrowOS/OSMain/BootDefs.HPP.Z"
#include "/SparrowOS/OSMain/Adam1a.HPP.Z"
#exe {OptOn(OPTf_EXTERNS_TO_IMPORTS);};
#include "/SparrowOS/Compiler/CmpDefs.HPP.Z"
#include "/SparrowOS/OSMain/Adam1b.HPP.Z"
#include "/SparrowOS/OSMain/Adam1c.HPP.Z"
#exe {
  OptOff(OPTf_EXTERNS_TO_IMPORTS);
  OptOff(OPTf_KEEP_PRIVATE);
};


