/*What's funner than page tables for programmers to play with!!

This changes a page table entry for VGA graphics memory.
TempleOS keeps a cached copy of VGA memory in
$LK,"gr.screen_image",A="MN:CGrGlbls"$.  See $LK,"GrUpdateVGAGraphics",A="MN:GrUpdateVGAGraphics"$.

TempleOS used 4096 size pages for memory below addr 2 Meg
and 2 Meg pages above two Meg.  See $LK,"MemPageSize",A="MN:MemPageSize"$() and $LK,"INIT_MEMORY",A="FF:::/Kernel/Mem1a.CPP,SYS_INIT_MEM"$.
*/

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
//the page table reg.

//  SetRAX(0x100000+0x1000*(MEM_NUM_PML1+MEM_NUM_PML2+MEM_NUM_PML3));
//  MOV_CR3_EAX

//<CTRL-ALT-f> will flush screen VGA cache.
  VGAFlush;
  PressAKey;

  pte[0]=p0;
  pte[1]=p1;
  InvlPg(VGA_GRAPHIC_MEM_BASE);
  InvlPg(VGA_GRAPHIC_MEM_BASE+0x1000);

//<CTRL-ALT-f> will flush screen VGA cache.
  VGAFlush;
}

PageTableDemo1;

//See $LK,"MemBlks2MegAlloc",A="MN:MemBlks2MegAlloc"$(), $LK,"MemBlks2MegFree",A="MN:MemBlks2MegFree"$() for page aligned allocations.
//See $LK,"CBlkPool",A="MN:CBlkPool"$, $LK,"CHeapCtrl",A="MN:CHeapCtrl"$, $LK,"sys_code_bp",A="MN:sys_code_bp"$ and $LK,"sys_data_bp",A="MN:sys_data_bp"$.
//See $LK,"Memory Overview",A="FI:::/Doc/MemOverview.TXT"$ and $LK,"Memory Routines",A="HI:Memory"$.
