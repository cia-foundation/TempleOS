//See $LK,"Install Documentation",A="FI:::/Doc/Install.DD"$.
//Study my account examples: $LK,"Cfg Strs",A="FL:::/Demo/AcctExample/TOS/TOSCfg.HC,1"$, $LK,"Update Funs",A="FL:::/Demo/AcctExample/TOS/TOSDistro.HC,1"$

#include "BootHD"

#help_index "Install"

#define KERNEL_BIN_C	"Kernel.BIN.C"

U0 MakeAll()
{
  if (Cmp("/Compiler/Compiler","Compiler",,':'))
    throw;
  if (Cmp("/Kernel/Kernel","Kernel",,':'))
    throw;
}

public U0 BootHDIns(U8 drv_let=0)
{//$LK,"MakeAll",A="MN:MakeAll"$ and install new boot-loader.
  CDrv *dv;
  CFAT32Boot br;
  CDirEntry de;
  I64 i;
  try {
    if (!Drv(drv_let))
      throw;
    dv=Fs->cur_dv;
    MakeAll;
    switch (Let2BlkDevType(dv->drv_let)) {
      case BDT_RAM:
      case BDT_ATA:
	Move("/Kernel/Kernel.BIN.Z","/" KERNEL_BIN_C);
	if (!FileFind("/" KERNEL_BIN_C,&de,FUF_JUST_FILES))
	  "No Kernel.BIN.C\n";
	else {
	  Free(de.full_name);
	  "Modifying partition boot record.\n";
	  BlkRead(dv,&br,dv->drv_offset,1);
	  br.jump_and_nop[0]=OC_JMP_REL8;
	  br.jump_and_nop[1]=offset(CFAT32Boot.code)-2;
#assert offset(CFAT32Boot.code)>=offset(CRedSeaBoot.code)
	  br.jump_and_nop[2]=OC_NOP;
	  *BHD_BLK_CNT(U16 *)=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	  *BHD_DAP_BLK(I64 *)=Clus2Blk(dv,de.clus);
	  for (i=0;i<BHD_END-BHD_CODE;i++)
	    br.code[i]=BHD_CODE(U8 *)[i];
#assert sizeof(CFAT32Boot.code)>=BHD_END-BHD_CODE
	  for (;i<sizeof(CFAT32Boot.code);i++)
	    br.code[i]=0;
	  BlkWrite(dv,&br,dv->drv_offset,1);
	}
	break;
      default:
	throw;
    }
  } catch {
    PutExcept;
    Beep;
  }
}
