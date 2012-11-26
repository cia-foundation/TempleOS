/*
You can play with this if you are
interested in USB.  I'm not planning
on doing USB unless the legacy PS2
keyboard/mouse mode disappears.
There EHCI UHCI in various ICH chips and
others and God knows how many custom packet
formats for different keyboards and mice.  Okay,
I suppose if BIOS can make a universal
packet, it is done just one way for
different mice.  PS2 is more compatible.

*/
#help_index "USB"
extern I64 sys_num_usb;
extern CUSB sys_usb_devs[MAX_USB];
extern CUSBTD *USBAllocTD();
extern U0 USBFreeTD(CUSBTD *temptd);
extern U0 USBEndAll();
extern U0 USBInitAll();
extern U32 *USBAllocFrameList(I64 usb_num,I64 size);

U8 *StatCB(CDoc *,CDocEntry *doc_e,CTask *mem_task)
{ //This routine shows the stat of the USB ports
  U8 *st=MAlloc(128,mem_task);
  U16 w1,w2,w3,w4;
  U32 d1;
  CUSB *u=doc_e->user_data;
  I64 d=u->ports;
  w3=InU16(d+USBP_STS);
  w1=InU16(d+USBP_PORTSC0); w2=InU16(d+USBP_PORTSC1);
  w4=InU16(d+USBP_FRNUM);
  d1=InU32(d+USBP_FRBASEADD);
  SPrintF(st,"%X:Stat:%04X  P0:%04X  P1:%04X FRAME:%04X",d1,w3,w1,w2,w4);
  return st;
}

U0 PutStat()
{
  I64 i;
  Bool old_preempt=Preempt(OFF);
  CDocEntry *doc_e;
  CUSB *u;
  WinMax;

	"$$FG,GREEN$$Dev Stat Bits\n$$FG$$"
	"  0:IRQ\n"
	"  1:IRQ err\n"
	"  2:Resume\n"
	"  3:Host Sys Err\n"
	"  4:Host Process Err\n"
	"  5:Halted\n"
	"$$FG,GREEN$$Port Stat Bits\n$$FG$$"
	"  0:Connection Stat\n"
	"  1:Connection Stat Change\n"
	"  2:Port Enabled\n"
	"  3:Port Enabled Change\n"
	"4-5:Line Stat\n"
	"  6:Resume Detect\n"
	"  8:Low Speed\n"
	"  9:Port Rst\n"
	" 10:Overcurrent Active\n"
	" 11:Overcurrent Indicator\n"
	" 12:Suspend\n\n";

  for (i=0;i<sys_num_usb;i++) {
    u=&sys_usb_devs[i];
    "$$FG,LTRED$$Dev%d:%X:",i,u->ports;
    doc_e=DocPutS(DocCur,"$$TX+TC,\" \"$$");
    doc_e->user_data=&sys_usb_devs[i];
    doc_e->tag_cb=&StatCB;
    "$$FG$$\n";
  }

  Preempt(old_preempt);
  NewLine;
}


extern U0 PutQH(U32 h);

U32 *P(U32 d)
{
  return d&~3;
}


U0 PutTD(U32 t)
{
  "TD:$$FG,GREEN$$%08X$$FG$$\n",t;
  "%08X\n",*P(t);
  DocDm(t+4,8);
  if (*P(t+12))
    DocDm(*P(t+12),16);
//    DocDm(*P(t+12),*P(t+4)>>21&0x7FF);
  if (!(*P(t)&1)) {
    if (*P(t)&2)
      PutQH(*P(t));
    else
      PutTD(*P(t));
  }
}

U0 PutQH(U32 h)
{
  "QH:$$FG,RED$$%08X$$FG$$\n",h;
  "%08X\n",*P(h);
  "%08X\n",*P(h+4);

  if (!(*P(h)&1)) {
    if (*P(h)&2)
      PutQH(*P(h));
    else
      PutTD(*P(h));
  }

  if (!(*P(h+4)&1)) {
    if (*P(h+4)&2)
      PutQH(*P(h+4));
    else
      PutTD(*P(h+4));
  }
}

U0 PutFrame(U32 f)
{
  PutQH(f);
}

U0 PutFrames()
{
  I64 i,d,f,w1,w2;
  CUSB *u;
  for (i=0;i<sys_num_usb;i++) {
    u=&sys_usb_devs[i];
    d=u->ports;
    w1=InU16(d+USBP_PORTSC0); w2=InU16(d+USBP_PORTSC1);
    if (w1&1 || w2&1) {
      f=InU32(d+USBP_FRBASEADD);
      PutFrame(f);
    }
  }

}

U0 Main()
{
  USBInitAll;
  PutStat;
  PutFrames;
  "$$FG,RED$$The BIOS sets-up the USB in PS/2 legacy mode.$$FG$$\n";
}

Main;


/*  Not Finished
#define PORT		5

#define DEV_ADD_INIT	1
#define DEV_ADD		1
#define END_PT0		0
#define END_PT1		1
#define END_PT2		2
#define MAX_LEN		8
#define TERMINATE	1

U0 SetUpTD()
{
  CUSB *u=&sys_usb_devs[PORT];
  I64 i,d=u->ports;
  U32 *frm=CAllocAligned(0x1000,0x1000,Fs->code_heap),
      *tds=CAllocAligned(256,16,Fs->code_heap),
      *buf=CAlloc(128,Fs->code_heap);
  DocD(buf,128);

  "<0>\n"; Sleep(100);
  OutU16(d+USBP_CMD,2); //Reset
  "<1>\n"; Sleep(100);
  OutU16(d+USBP_CMD,0);
  "<2>\n"; Sleep(100);
  OutU16(d+USBP_PORTSC0,4); //Enable
  OutU16(d+USBP_PORTSC1,4);
  "<3>\n"; Sleep(100);

  tds[0]=&tds[4](U8 *);
  tds[1]=0;
  tds[2]=PID_SETUP+DEV_ADD_INIT<<8+END_PT0<<15+MAX_LEN<<21;
  tds[3]=buf;
  buf[0]=0+RQ_SET_ADDR<<8+DEV_ADD<<16;
  buf[1]=0+0<<16;


  tds[4]=TERMINATE;
  tds[5]=0;
  tds[6]=PID_SETUP+DEV_ADD_INIT<<8+END_PT0<<15+MAX_LEN<<21;
  tds[7]=buf(U8 *)+8;
  buf[2]=0+RQ_SET_ADDR<<8+DEV_ADD<<16;
  buf[3]=0+0<<16;

  frm[0]=&tds[0](U8 *);
  for (i=1;i<0x1000/4;i++)
    frm[i]=TERMINATE;

  OutU16(d+USBP_FRNUM,frm);
  OutU32(d+USBP_FRBASEADD,frm);
  "<4>\n"; Sleep(200);
  OutU16(d+USBP_CMD,1);
  "<5>\n"; Sleep(1000);
  OutU16(d+USBP_CMD,0);
  "<6>\n"; Sleep(200);
}

SetUpTD;

*/

