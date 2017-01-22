#help_index "Call"
asm {
_HI_CALL::
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RAX,U64 16[RBP]
	TEST	RAX,RAX
	JZ	@@05
	CALL	RAX
@@05:	POP	RBP
	RET1	8
//************************************
_HI_MEMCPY::
	PUSH	RBP
	MOV	RBP,RSP
	PUSH	RSI
	PUSH	RDI
	CLD
	MOV	RDI,U64 SF_ARG1[RBP]
	MOV	RSI,U64 SF_ARG2[RBP]
	MOV	RCX,U64 SF_ARG3[RBP]
	REP_MOVSB
	MOV	RAX,RDI
	POP	RDI
	POP	RSI
	POP	RBP
	RET1	24
}
_extern _HI_CALL I64 HiCall(U8 *machine_code);
_extern _HI_MEMCPY U8 *HiMemCpy(U8 *dst,U8 *src,I64 cnt);

#help_index "Boot"
public U0 BootRAM(U8 *filename=NULL)
{//Softboot Kernel.BIN file. No hardware reset.
  I64 size;
  CKernel *hi_image,*lo_image=mem_boot_base-sizeof(CBinFile),
	reg *sys_ram_reboot;
  if (!filename)
    filename="::/" KERNEL_BIN_C;
  do if (!(hi_image=FileRead(filename,&size))) return;
  while (hi_image<0x100000); //If alloc from low 640K, just get another copy.

  do sys_ram_reboot=MAlloc(SYS_RAM_REBOOT_END-SYS_RAM_REBOOT,Fs->code_heap);
  while (sys_ram_reboot<0x100000);

  hi_image->boot_src=BOOT_SRC_RAM;
  hi_image->boot_blk=0;
  hi_image->boot_patch_table_base=lo_image(U8 *)+hi_image->h.patch_table_offset;
  hi_image->sys_run_level =lo_image->sys_run_level&(RLF_VGA|RLF_16BIT);
  MemCpy(&hi_image->start,&lo_image->start,
	sizeof(CKernel)-offset(CKernel.start));

  CLI
  if (mp_cnt>1)
    MPHalt;

  HiMemCpy(sys_ram_reboot,SYS_RAM_REBOOT,SYS_RAM_REBOOT_END-SYS_RAM_REBOOT);
  HiMemCpy(lo_image,hi_image,size);
  HiCall(sys_ram_reboot);
}
