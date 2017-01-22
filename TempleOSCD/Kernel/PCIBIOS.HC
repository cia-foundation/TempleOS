asm {
USE32

SYS_PCIBIOS_SERVICE_DIR:: DU32	0;
SYS_PCI_SERVICES::	 DU32	0;

SYS_FIND_PCIBIOS_SERVICE_DIR::
	MOV	ESI,0xE0000
	MOV	ECX,(0x100000-0xE0000)/4
@@05:	CMP	U32 [ESI],'_32_'
	JNE	@@20
	PUSH	ECX
	XOR	ECX,ECX
	MOV	CL,U8 9[ESI]
	SHL	ECX,4
@@10:	MOV	EDI,ESI
	XOR	EAX,EAX
	XOR	EDX,EDX
@@15:	MOV	DL,U8 [EDI]
	ADD	EAX,EDX
	INC	EDI
	DEC	ECX
	JNZ	@@15
	POP	ECX
	TEST	AL,AL
	JNZ	@@20
	MOV	U32 [SYS_PCIBIOS_SERVICE_DIR],ESI
	MOV	ESI,U32 4[ESI]
	MOV	U32 [SYS_PCIBIOS_SERVICE_CALL],ESI
	RET

@@20:	ADD	ESI,4
	LOOP	@@05
	MOV	U32 [SYS_PCIBIOS_SERVICE_DIR],0
	RET

SYS_FIND_PCI_SERVICES::
	MOV	ESI,U32 [SYS_PCIBIOS_SERVICE_DIR]
	TEST	ESI,ESI
	JNZ	@@05
	MOV	U32 [SYS_PCI_SERVICES],0
	RET
@@05:	MOV	EAX,'$$PCI'
	XOR	EBX,EBX
	DU8	0x9A;		 //CALL CGDT.cs32:PCIBIOS_SERVICE
SYS_PCIBIOS_SERVICE_CALL:: DU32	0;
	DU16	CGDT.cs32;
	TEST	AL,AL
	JNZ	@@05
	LEA	ESI,U32 [EBX+EDX]
	MOV	U32 [SYS_PCI_SERVICES],ESI
	RET

@@05:	MOV	U32 [SYS_PCI_SERVICES],0
	RET

USE64
C32_EAX::	DU32	0;
C32_EBX::	DU32	0;
C32_ECX::	DU32	0;
C32_EDX::	DU32	0;
C32_ESI::	DU32	0;
C32_EDI::	DU32	0;
C32_EFLAGS::	DU32	0;

C32_RSP::	DU64	0;

_FAR_CALL32::
//This calls a 32-bit mode routine.
//(We must switch from 64-bit mode to do it.)
//
//NON REENTRANT
//
	PUSH	RBP
	MOV	RBP,RSP
	MOV	RAX,U64 SF_ARG1[RBP]
	TEST	RAX,RAX
	JNZ	@@05
	POP	RBP
	RET1	8	//return FALSE
@@05:	MOV	U32 [C32_ADD],EAX
	PUSH_REGS
	PUSHFD
	XOR	RAX,RAX
	PUSH	U64 FS:CTask.addr[RAX]
	PUSH	U64 GS:CCPU.addr[RAX]
	MOV	U64 [C32_RSP],RSP
	PUSH	U32 CGDT.ds		//STKSEG
	PUSH	U32 BOOT_RAM_LIMIT	//STK
	PUSH	U32 0			//FLAGS--interrupts off
	PUSH	U32 CGDT.cs32
	LEA	RAX,[@@15]
	PUSH	RAX
	IRET
USE32
@@15:
	WBINVD
//disable paging
	MOV_EAX_CR0
	BTR	EAX,31
	MOV_CR0_EAX

	MOV	ECX,IA32_EFER
	XOR	EDX,EDX
	XOR	EAX,EAX
	WRMSR

	MOV	AX,CGDT.ds
	MOV	FS,AX
	MOV	GS,AX
//SS already set

	MOV	EAX,U32 [C32_EAX]
	MOV	EBX,U32 [C32_EBX]
	MOV	ECX,U32 [C32_ECX]
	MOV	EDX,U32 [C32_EDX]
	MOV	ESI,U32 [C32_ESI]
	MOV	EDI,U32 [C32_EDI]
	MOV	U32 [C32_EFLAGS],0

	DU8	0x9A;		 //CALL CGDT.cs32:[C32_ADD]
C32_ADD:: DU32	0;
	DU16	CGDT.cs32;

	PUSHFD
	POP	U32 [C32_EFLAGS]

	MOV	U32 [C32_EAX],EAX
	MOV	U32 [C32_EBX],EBX
	MOV	U32 [C32_ECX],ECX
	MOV	U32 [C32_EDX],EDX
	MOV	U32 [C32_ESI],ESI
	MOV	U32 [C32_EDI],EDI

	PUSH	U32 0	//Return from next call will be 64-bit
	CALL	SYS_ENTER_LONG_MODE

USE64	MOV	RSP,U64 [C32_RSP]
	POP	RAX
	CALL	SET_GS_BASE
	POP	RAX
	CALL	SET_FS_BASE

	POPFD
	POP_REGS
	XOR	RAX,RAX
	MOV	AL,TRUE
	POP	RBP
	RET1	8
}

_extern C32_EAX U32 c32_eax;
_extern C32_EBX U32 c32_ebx;
_extern C32_ECX U32 c32_ecx;
_extern C32_EDX U32 c32_edx;
_extern C32_ESI U32 c32_esi;
_extern C32_EDI U32 c32_edi;
_extern C32_EFLAGS U32 c32_eflags;
_extern SYS_PCI_SERVICES U32 sys_pci_services;

_extern _FAR_CALL32 Bool FarCall32(U0 (*fp_addr)());//Not reentrant.For PCIBIOS.

U8 PCIReadU8(I64 bus,I64 dev,I64 fun,I64 rg)
{//Read U8 in PCI configspace at bus,dev,fun,reg.
  I64 res;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB108;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  if (FarCall32(sys_pci_services))
    res=c32_ecx.u8[0];
  else
    res=0xFF;
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
  return res;
}

U16 PCIReadU16(I64 bus,I64 dev,I64 fun,I64 rg)
{//Read U16 in PCI configspace at bus,dev,fun,reg.
  I64 res;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB109;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  if (FarCall32(sys_pci_services))
    res=c32_ecx.u16[0];
  else
    res=0xFFFF;
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
  return res;
}

U32 PCIReadU32(I64 bus,I64 dev,I64 fun,I64 rg)
{//Read U32 in PCI configspace at bus,dev,fun,reg.
  I64 res;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB10A;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  if (FarCall32(sys_pci_services))
    res=c32_ecx;
  else
    res=0xFFFFFFFF;
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
  return res;
}

U0 PCIWriteU8(I64 bus,I64 dev,I64 fun,I64 rg,I64 val)
{//Write U8 in PCI configspace at bus,dev,fun,reg.
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB10B;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  c32_ecx=val;
  FarCall32(sys_pci_services);
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
}

U0 PCIWriteU16(I64 bus,I64 dev,I64 fun,I64 rg,I64 val)
{//Write U16 in PCI configspace at bus,dev,fun,reg.
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB10C;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  c32_ecx=val;
  FarCall32(sys_pci_services);
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
}

U0 PCIWriteU32(I64 bus,I64 dev,I64 fun,I64 rg,I64 val)
{//Write U32 in PCI configspace at bus,dev,fun,reg.
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB10D;
  c32_ebx=bus<<8+dev<<3+fun;
  c32_edi=rg;
  c32_ecx=val;
  FarCall32(sys_pci_services);
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
}

I64 PCIClassFind(I64 class_code,I64 n)
{/*Find bus,dev,fun of Nth class_code dev.

class_code is low three bytes
n is index starting at zero
Return: -1 not found
else bus,dev,fun.
*/
  I64 res;
  PUSHFD
  CLI
  while (LBts(&sys_semas[SEMA_FAR_CALL32],0))
    Yield;
  c32_eax=0xB103;
  c32_esi=n;
  c32_ecx=class_code;
  if (FarCall32(sys_pci_services) && !c32_eax.u8[1])
    res=c32_ebx.u8[1]<<16+(c32_ebx&0xF8)<<5+c32_ebx&7;
  else
    res=-1;
  LBtr(&sys_semas[SEMA_FAR_CALL32],0);
  POPFD
  return res;
}

