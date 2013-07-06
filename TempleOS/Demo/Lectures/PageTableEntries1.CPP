//What's funner than page tables for programmers to play with!!

//This changes a page table entry for VGA graphics memory.
//TempleOS keeps a cached copy of VGA memory in
//$LK,"gr_screen_image","MN:gr_screen_image"$.  See $LK,"GrUpdateVGAGraphics","MN:GrUpdateVGAGraphics"$.

//TempleOS used 4096 size pages for memory below addr 2 Meg
//and 2 Meg pages above two Meg.  See $LK,"MemPageSize","MN:MemPageSize"$() and $LK,"INIT_MEMORY","FF:::/TempleOS/Kernel/Memory.CPP,SYS_INIT_MEM"$.


U0 PageTableDemo1()
{
  I64 *pte=MemPageTable(VGA_GRAPHIC_MEM_BASE),p0,p1;
  "Page table entries for VGA screen memory\n";
  DocD(pte);
  PressAKey;

  p0=pte[0];
  p1=pte[1];
  pte[0]=p1;
  pte[1]=p0;
  InvlPg(VGA_GRAPHIC_MEM_BASE); //Invalidate translation look aside buffer (TLB).
  InvlPg(VGA_GRAPHIC_MEM_BASE+0x1000);

//Use this if you wish to change
//the page table register.

//  SetRAX(0x100000+0x1000*(NUM_PML1+NUM_PML2+NUM_PML3));
//  asm {MOV_CR3_EAX}

//<CTRL-ALT-F> will flush screen VGA cache.
  VGAFlush;
  PressAKey;

  pte[0]=p0;
  pte[1]=p1;
  InvlPg(VGA_GRAPHIC_MEM_BASE);
  InvlPg(VGA_GRAPHIC_MEM_BASE+0x1000);

//<CTRL-ALT-F> will flush screen VGA cache.
  VGAFlush;
}

PageTableDemo1;

//See $LK,"MemBlks2MegAlloc","MN:MemBlks2MegAlloc"$(), $LK,"MemBlks2MegFree","MN:MemBlks2MegFree"$() for page aligned allocations.
//See $LK,"CBlkPool","MN:CBlkPool"$, $LK,"CHeapCtrl","MN:CHeapCtrl"$, $LK,"sys_code_bp","MN:sys_code_bp"$, $LK,"sys_data_bp","MN:sys_data_bp"$.
//See $LK,"Memory","HI:Memory"$
