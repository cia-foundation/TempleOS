//Be sure to Adam Include this.

#help_index "Comm"

#define COM1_BASE	0x3F8
#define COM2_BASE	0x2F8
#define COM3_BASE	0x3E8
#define COM4_BASE	0x2E8

#define UART_THR		0
#define UART_RDR		0
#define UART_BRDL		0
#define UART_IER		1
#define UART_BRDH		1
#define UART_IIR		2
#define UART_LCR		3
#define UART_MCR		4
#define UART_LSR		5
#define UART_MSR		6


I64 comm_base[5]={0,0x3F8,0x2F8,0x3E8,0x2E8};

CFifoU8 *comm_fifos[5]={NULL,NULL,NULL,NULL,NULL};

interrupt U0 CommInt3Handler()
{
  I64 base=comm_base[2];
  if (comm_fifos[2] && (InU8(base+UART_IIR) & 0xFE)==8)   //RX IRQ
    FifoU8Ins(comm_fifos[2],InU8(base+UART_RDR));

  base=comm_base[4];
  if (comm_fifos[4] && (InU8(base+UART_IIR) & 0xFE)==8)   //RX IRQ
    FifoU8Ins(comm_fifos[4],InU8(base+UART_RDR));
  OutU8(0x20,0x20);
}

interrupt U0 CommInt4Handler()
{
  I64 base=comm_base[1];
  if (comm_fifos[1] && (InU8(base+UART_IIR) & 0xFE)==8)   //RX IRQ
    FifoU8Ins(comm_fifos[1],InU8(base+UART_RDR));

  base=comm_base[3];
  if (comm_fifos[3] && (InU8(base+UART_IIR) & 0xFE)==8)   //RX IRQ
    FifoU8Ins(comm_fifos[3],InU8(base+UART_RDR));
  OutU8(0x20,0x20);
}

IntEntrySet(0x23,&CommInt3Handler);
IntEntrySet(0x24,&CommInt4Handler);

public U0 CommInit8n1(I64 port,I64 baud)
{
  I64 base=comm_base[port];

  PUSHFD
  CLI
  if (comm_fifos[port])
    FifoU8Del(comm_fifos[port]);
  comm_fifos[port]=FifoU8New(256);
  OutU8(base+UART_LCR,0);     //Set for IER
  OutU8(base+UART_IER,0);     //DIsable all IRQ
  OutU8(base+UART_LCR,0x80);  //Enable baud rate control
  OutU8(base+UART_BRDL,(0x180/(baud/300)) & 0xFF); //LSB
  OutU8(base+UART_BRDL,(0x180/(baud/300)) / 256);  //MSB
  OutU8(base+UART_LCR,3);     // 8-none-1

  InU8(base+UART_RDR);	//read garbage
  InU8(base+UART_LSR);

  OutU8(base+UART_MCR,4);
  OutU8(base+UART_IER,0);     //DIsable all IRQ
  OutU8(base+UART_MCR,0xA);	  //out2 and rts
  OutU8(0x21,InU8(0x21) & (0xFF-0x18)); //Enable 8259 IRQ 3 & 4
  OutU8(base+UART_IER,1);	  //Enable RX IRQ
  POPFD
}

public U0 CommBusyPutChar(I64 port,U8 b)
{
  I64 base=comm_base[port];
  while (!(InU8(base+UART_LSR) & 0x20))
    Yield;
  OutU8(base+UART_THR,b);
  while (!(InU8(base+UART_LSR) & 0x20))
    Yield;
}

public U0 CommBusyPutS(I64 port,U8 *st)
{
  I64 base=comm_base[port];
  U8 b;
  while (b=*(st++)) {
    while (!(InU8(base+UART_LSR) & 0x20))
      Yield;
    OutU8(base+UART_THR,b);
  }
  while (!(InU8(base+UART_LSR) & 0x20))
    Yield;
}

public U0 CommBusyWriteBlk(I64 port,U8 *buf,I64 count)
{
  I64 base=comm_base[port];
  while (count--) {
    while (!(InU8(base+UART_LSR) & 0x20))
      Yield;
    OutU8(base+UART_THR,*(buf++));
  }
  while (!(InU8(base+UART_LSR) & 0x20))
    Yield;
}

#help_index ""
