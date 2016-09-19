/*TempleOS runs exclusively in ring 0.
Ring 0 is part of the $LK,"Charter",A="FI:::/Doc/Charter.DD"$.
This demo is for you to play around
with ring 3.  TempleOS is for
recreational programming, after all.

This redirects the general protection
fault, switches to ring 3, and generates
a fault to switch back.
*/

U8 *old_stk,*new_rip;

asm {
INT_TO_RING0::	//Set to handle general protection 0xD fault temporarily.
	INC	U64 [SYS_PROGRESS1]
	PUSH	U32 CGDT.ds	//STKSEG
	MOV	RAX,U64 [&old_stk]
	PUSH	RAX
	PUSH	U32 0		//FLAGS--interrupts off
	PUSH	U32 CGDT.cs64
	MOV	RAX,U64 [&new_rip]
	PUSH	RAX
	IRET
}

U0 Ring3Demo()
{
  U8 *old_vect;
  "Progress1 Before:%X\n",progress1;
  CLI
  old_vect=IntEntrySet(0x0D,INT_TO_RING0,IDTET_TRAP,0);

  TSSBusy(Gs->tss->tr_ring3,OFF);
  SetRAX(Gs->tss->tr_ring3+3);
  LTR	AX

  asm {
    MOV	U64 [&old_stk],RSP

    LEA	RAX,[R3_CALLBACK]
    MOV	U64 [&new_rip],RAX

    MOV	AX,CGDT.ds_ring3+3
    MOV	DS,AX
    MOV	ES,AX

    PUSH	U32 CGDT.ds_ring3+3	//STKSEG
    PUSH	U64 [&old_stk]
    PUSH	U32 0		//FLAGS--interrupts off
    PUSH	U32 CGDT.cs64_ring3+3
    LEA	RAX,[R3_START]
    PUSH	RAX
    IRET

    R3_START:
    INC	U64 [SYS_PROGRESS1]
    CLI	//This causes general protection fault #13

    R3_CALLBACK:
    MOV	AX,CGDT.ds
    MOV	DS,AX
    MOV	ES,AX
  }

  TSSBusy(Gs->tss->tr,OFF);
  SetRAX(Gs->tss->tr);
  LTR	AX

  IntEntrySet(0x0D,old_vect,IDTET_IRQ,0);
  STI
  "Progress1 After :%X\n",progress1;
}

Ring3Demo;
