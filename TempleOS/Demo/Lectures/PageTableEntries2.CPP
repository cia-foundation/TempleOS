I64 *addr=VGA_GRAPHIC_MEM_BASE,
   *pte_addr=MemPtAtPageTableEntry(addr),
   err_code=0,
   *reported_fault_addr;


/*
  0[RBP]=old base ptr
  8[RBP]=err code
  16[RBP]=RIP
  24[RBP]=RIP_SELECTOR
  32[RBP]=RFLAGS
  40[RBP]=RSP
  48[RBP]=STK_SELECTOR
*/

interrupt U0 PageFault()
{
  I64 reg *rbp=GetBasePtr,reg *rsp=GetStkPtr,reg *src=rbp,reg *dst=rbp+1;
  *pte_addr|=1;
  InvlPg(addr);
  err_code=rbp[1];
asm {  MOV_RAX_CR2}
  reported_fault_addr=GetRAX;
//Remove err_code from stk
//so IRET works right.
  while (src>=rsp)
    *dst--=*src--;
  SetBasePtr(rbp+1);
  SetStkPtr(rsp+1);
}


U0 PageTableDemo2()
{
  U8 *old_idte=IntEntrySet(I_PAGE_FAULT,&PageFault,IDTET_IRQ);
  err_code=0;
  *pte_addr &=~1; //Not present
  InvlPg(addr);
  WinMgrSync; //wait for VGA update
  WinMgrSync;
  IntEntrySet(I_PAGE_FAULT,old_idte,IDTET_IRQ);
  "Err Code:%X Addr:%X\n",err_code,reported_fault_addr;
}

PageTableDemo2;
