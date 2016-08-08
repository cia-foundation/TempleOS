//See $LK,"PCI",A="HI:PCI"$.

#if mp_cnt>1
#define DEST_CPU	1
#else
#define DEST_CPU	0
#endif

#assert I_USER==0x40

interrupt U0 IntMy40()
{
  lock {progress1++;}
*(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
}

interrupt U0 IntMy41()
{
  lock {progress2++;}
*(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
}

interrupt U0 IntMy42()
{
  lock {progress3++;}
*(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
}

interrupt U0 IntMy43()
{
  lock {progress4++;}
*(dev.uncached_alias+LAPIC_EOI)(U32 *)=0;
}

I64 vect[4]={&IntMy40,&IntMy41,&IntMy42,&IntMy43};

U0 Main()
{
  I64 i;
  U8 *da=dev.uncached_alias+IOAPIC_REG;
  U32 *_d=dev.uncached_alias+IOAPIC_DATA;

  ProgressBarsRst;
  for (i=I_USER;i<I_USER+4;i++)
    IntEntrySet(i,vect[i&3],IDTET_IRQ);

    //There are like 4*6 vects.  Bus zero
    //has the first four vects.
  for (i=IOREDTAB;i<IOREDTAB+4*2;i+=2) {
    *da=i+1;
    *_d=dev.mp_apic_ids[DEST_CPU]<<24;
    *da=i;
    *_d=0x4000+0x40+(i-IOREDTAB)/2;
  }

  Sleep(1000);
  "The keyboard and timer seem to be active.\n"
	"IntA:%d IntB:%d IntC:%d IntD:%d\n\n"
	"IntB is my keybaord and regs a single KEY-UP.\n"
	"Rerun and press a few keys.\n",
	progress1,progress2,progress3,progress4;

  //Disable vects
  for (i=IOREDTAB;i<IOREDTAB+4*2;i+=2) {
    *da=i+1;
    *_d=dev.mp_apic_ids[DEST_CPU]<<24;
    *da=i;
    *_d=0x14000+0x40+(i-IOREDTAB)/2;
  }
  ProgressBarsRst;
}

Main;
