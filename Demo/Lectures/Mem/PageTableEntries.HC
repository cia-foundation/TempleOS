I64 *addr=text.vga_alias,
   *pte_addr=MemPageTable(addr),
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

interrupt haserrcode U0 IntPageFault()
{
  I64 reg *rbp=GetRBP;
  *pte_addr|=1;
  InvlPg(addr);
  err_code=rbp[1];
  MOV_RAX_CR2
  reported_fault_addr=GetRAX;
}

U0 PageTableDemo()
{
  U8 *old_idte=IntEntrySet(I_PAGE_FAULT,&IntPageFault,IDTET_IRQ);
  err_code=0;
  *pte_addr &=~1; //Not present
  InvlPg(addr);
  WinMgrSync; //wait for VGA update
  WinMgrSync;
  IntEntrySet(I_PAGE_FAULT,old_idte,IDTET_IRQ);
  "Err Code:%X Addr:%X\n",err_code,reported_fault_addr;
}

PageTableDemo;
