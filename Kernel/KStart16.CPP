asm {/* See $LK,"::/Doc/Boot.TXT"$.
TempleOS starts in real, calls some BIOS
routines, switches to 32 bit, and 64 bit mode
and continues in $LK,"HolyC",A="FI:::/Doc/HolyC.TXT"$ at $LK,"KernelMain",A="MN:KernelMain"$().

The boot loader jumps here in real-mode (16-bit).
It actually jumps to the $LK,"CBinFile",A="MN:CBinFile"$ header which is
placed just before this by $LK,"the compiler",A="FF:::/Compiler/CMain.CPP,16 ALIGN"$.
The header begins with a short jmp to
the start of this file's code which begins
with the following small jump past some data.

This file is first in the Kernel image
because it is #included first.  $LK,"Kernel.PRJ",A="FF:::/Kernel/Kernel.PRJ,KStart16:1"$
*/
USE16
SYS_KERNEL:: //This must match $LK,"CKernel",A="MN:CKernel"$.
	JMP	U16 CORE0_16BIT_INIT

//************************************
//  ASM Global vars required for 16-bit start-up
	ALIGN	4,OC_NOP
SYS_BOOT_SRC::			DU32	BOOT_SRC_NULL;
SYS_BOOT_BLK::			DU32	0;
SYS_BOOT_PATCH_TABLE_BASE::	DU32	0;
SYS_RUN_LEVEL::			DU32	0;
#exe {StreamPrint("SYS_COMPILE_TIME:: DU64 0x%X;",Now);} //See $LK,"BootDVDProbe",A="MN:BootDVDProbe"$
#assert SYS_COMPILE_TIME+sizeof(CDate)+sizeof(CBinFile)<DVD_BLK_SIZE

SYS_BOOT_BASE::	DU32	0;	//Offset from start used by reboot
SYS_MEM_E801::	DU16	0,0;
SYS_MEM_E820::	DU8	MEM_NUM_E820_ENTRIES*MEM_E820_ENTRY_SIZE DUP (0);

SYS_GDT_PTR::	DU16	sizeof(CGDT)-1;
		DU64	0;
SYS_PCI_BUSES::	DU16	0;

	ALIGN	16,OC_NOP
SYS_GDT:: //See $LK,"CGDT",A="MN:CGDT"$
GDT_NULL:	DU64	0,0;
GDT_BOOT_DS:	DU64	0x00CF92000000FFFF,0; //Gets patched.
GDT_BOOT_CS:	DU64	0x00CF9A000000FFFF,0; //Gets patched.
GDT_CS32:	DU64	0x00CF9A000000FFFF,0;
GDT_CS64:	DU64	0x00209A0000000000,0; //The $LK,"Charter",A="FI:::/Doc/Charter.TXT"$ says just ring0.
GDT_CS64_RING3:	DU64	0x0020FA0000000000,0; //$LK,"Ring3",A="FI:::/Demo/Lectures/Ring3.CPP"$, so you can play with.
GDT_DS:		DU64	0x00CF92000000FFFF,0;
GDT_DS_RING3:	DU64	0x00CFF2000000FFFF,0;
GDT_TR:		DU8	MP_MAX_PROCESSORS*16 DUP(0);
GDT_TR_RING3:	DU8	MP_MAX_PROCESSORS*16 DUP(0);
#assert $$-SYS_GDT==sizeof(CGDT)

#assert $$-SYS_KERNEL==sizeof(CKernel)-sizeof(CBinFile)

//************************************
CORE0_16BIT_INIT::
//EAX is $LK,"SYS_BOOT_SRC",A="FF:::/Kernel/KStart16.CPP,[SYS_BOOT_SRC]"$. (Val passed from boot blk, $LK,"BootHD",A="FF:::/Adam/Opt/Boot/BootHD.CPP,BOOT_SRC_HARDDRV"$, $LK,"BootDVD",A="FF:::/Adam/Opt/Boot/BootDVD.CPP,BOOT_SRC_DVD"$, & $LK,"BootRAM",A="FF:::/Adam/Opt/Boot/BootRAM.CPP,BOOT_SRC_RAM"$.)
	MOV	ECX,EAX
	MOV	AX,(BOOT_RAM_LIMIT-BOOT_STK_SIZE)/16
	MOV	SS,AX
	MOV	SP,BOOT_STK_SIZE
	PUSH	ECX	//Will be $LK,"SYS_BOOT_SRC",A="FF:::/Kernel/KStart16.CPP,[SYS_BOOT_SRC]"$. See $LK,"BootHD",A="FF:::/Adam/Opt/Boot/BootHD.CPP,BOOT_SRC_HARDDRV"$, $LK,"BootDVD",A="FF:::/Adam/Opt/Boot/BootDVD.CPP,BOOT_SRC_DVD"$ & $LK,"BootRAM",A="FF:::/Adam/Opt/Boot/BootRAM.CPP,BOOT_SRC_RAM"$.
	PUSH	EBX

	CALL	U16 GET_IP
GET_IP:	POP	BX
	SUB	BX,GET_IP
	SHR	BX,4
	MOV	AX,CS
	ADD	AX,BX
	PUSH	AX
	PUSH	U16 @@05
	RETF

@@05:	STI
	MOV	AX,CS
	MOV	DS,AX

	MOV	U32 [SYS_RUN_LEVEL],RLF_16BIT

	MOV	AX,0x4F02
	MOV	BX,0x12		//640x480 16 color
#exe {
  if (!kernel_cfg->opts[CFG_TEXT_MODE])
    StreamPrint("INT 0x10");	//Enable VGA
};
	CMP	AX,0x004F
	JNE	@@10		//Jmp if fail
	BTS	U32 [SYS_RUN_LEVEL],RLf_VGA
@@10:

//Get mem maps
	MOV	AX,0xE801
	INT	0x15
	MOV	U16 [SYS_MEM_E801],CX
	MOV	U16 [SYS_MEM_E801+2],DX

	MOV	CX,MEM_NUM_E820_ENTRIES-1 //Leave one to terminate
	XOR	EBX,EBX
	MOV	AX,DS
	MOV	ES,AX
	MOV	DI,SYS_MEM_E820
@@15:	PUSH	CX
	MOV	EAX,0xE820
	MOV	ECX,MEM_E820_ENTRY_SIZE
	MOV	EDX,'PAMS'
	INT	0x15
	JC	@@20
	CMP	EAX,'PAMS'
	JNE	@@20
	TEST	EBX,EBX
	JZ	@@20
	ADD	DI,MEM_E820_ENTRY_SIZE
	POP	CX
	LOOP	@@15
	JMP	@@25
@@20:	ADD	SP,2

//Get PCI Bus Info
@@25:	MOV	U16 [SYS_PCI_BUSES],256
	XOR	DX,DX
	MOV	AX,0xB101
	INT	0x1A
	CMP	DX,'PC'
	JNE	@@30
	MOV	CH,0
	INC	CX
	MOV	U16 [SYS_PCI_BUSES],CX
@@30:
	CLI
//Enable A20
	IN	AL,0x92
	OR	AL,2
	OUT	0x92,AL

	POP	U32 [SYS_BOOT_BLK]
	POP	U32 [SYS_BOOT_SRC]	//See $LK,"BootHD",A="FF:::/Adam/Opt/Boot/BootHD.CPP,BOOT_SRC_HARDDRV"$, $LK,"BootDVD",A="FF:::/Adam/Opt/Boot/BootDVD.CPP,BOOT_SRC_DVD"$, & $LK,"BootRAM",A="FF:::/Adam/Opt/Boot/BootRAM.CPP,BOOT_SRC_RAM"$.

	CLD
	XOR	EAX,EAX
	MOV	AX,CS
	MOV	DS,AX
	MOV	ES,AX
	SHL	EAX,4

	MOV	U32 [SYS_BOOT_BASE],EAX

	MOV	DX,CS
	SUB	DX,sizeof(CBinFile)/16
#assert !(sizeof(CBinFile)&15)
	MOV	GS,DX

	MOV	EDX,EAX
	ADD	EDX,U32 GS:[CBinFile.patch_table_offset]
	SUB	EDX,sizeof(CBinFile)
	MOV	U32 [SYS_BOOT_PATCH_TABLE_BASE],EDX

	ADD	U32 [GDT_BOOT_DS+2],EAX
	ADD	U32 [GDT_BOOT_CS+2],EAX
	ADD	EAX,I32 SYS_GDT
	MOV	U32 [SYS_GDT_PTR+CSysLimitBase.base],EAX
	LGDT	U32 [SYS_GDT_PTR]

	MOV	EAX,SYS_START_CR0
	MOV_CR0_EAX

/* The assembler doesn't support far jumps so
we hand code it.  16-bit code is not important
enough to fully implement in the assembler.

To complete the switch to 32-bit mode, we have to load
the code segment with a far jump.
*/
	DU8	0x66,0xEA; //JMP CGDT.boot_cs:CORE0_32BIT_INIT
	DU32	CORE0_32BIT_INIT;
	DU16	CGDT.boot_cs;
#assert $$+16<=0xFFFF
}
