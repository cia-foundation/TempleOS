#help_index "Registry/Install Registration"

public CInsReg ins_reg;
MemSet(&ins_reg,0,sizeof(CInsReg));

DefineLstLoad("ST_HOST_OSES","Other\0Native\0Linux\0Windows\0");
DefineLstLoad("ST_HOST_VMS","Other\0Native\0VMware\0QEMU\0VirtualBox\0");

#define RR_UNREGISTER	-1
#define RR_REGISTER	0
#define RR_REREGISTER	1

public Bool InsReg(U8 drv_let=0,I64 action=RR_REGISTER)
{//Register this install in local $LK,"Registry.HC",A="FI:~/Registry.HC"$ file.
  CInsReg r;
  I64 old_drv_let;
  Bool res=FALSE;

  if (action==RR_UNREGISTER) {
    MemSet(&r,0,sizeof(CInsReg));
    res=TRUE;
  } else if (action==RR_REREGISTER || !ins_reg.registered) {
    MemCpy(&r,&ins_reg,sizeof(CInsReg));
    r.registered=TRUE;
    if (PopUpForm(&r))
      res=TRUE;
  }

  if (res) {
    old_drv_let=*sys_registry_doc->filename.name;
    if (drv_let)
      *sys_registry_doc->filename.name=drv_let;
    RegWrite("Adam/InsReg",
	"#help_index \"Registry/Install Registration\"\n"
	"MemSet(&ins_reg,0,sizeof(CInsReg));\n"
	"ins_reg.registered=%d;\n"
	"ins_reg.host_os=%d;\n"
	"ins_reg.host_vm=%d;\n"
	"\n#define INS_REG_PERSONAL_INITIALS \"%s\"\n"
	"StrCpy(ins_reg.initials,\"%s\");\n"
	"HashPublic(\"INS_REG_PERSONAL_INITIALS\",HTT_DEFINE_STR);\n"
	"\n#define INS_REG_MACHINE_NUM %d\n"
	"ins_reg.machine_num=%d;\n"
	"HashPublic(\"INS_REG_MACHINE_NUM\",HTT_DEFINE_STR);\n"
	"\n\"InsReg:%%s:%%d\\n\","
	"INS_REG_PERSONAL_INITIALS,INS_REG_MACHINE_NUM;\n"
	"\n#help_index \"\"\n"
	,r.registered,r.host_os,r.host_vm,
	r.initials,r.initials,r.machine_num,r.machine_num);
    *sys_registry_doc->filename.name=old_drv_let;
    RegExe("Adam/InsReg");
  }
  return res;
}

public Bool InsRereg(U8 drv_let=0)
{//Reregister this install in local $LK,"Registry.HC",A="FI:~/Registry.HC"$ file.
  return InsReg(drv_let,RR_REREGISTER);
}

public Bool InsUnreg(U8 drv_let=0)
{//Unregister this install in local $LK,"Registry.HC",A="FI:~/Registry.HC"$ file.
  return InsReg(drv_let,RR_UNREGISTER);
}
