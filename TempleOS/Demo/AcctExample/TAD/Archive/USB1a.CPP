#define USBP_CMD	0x00
#define USBP_STS	0x02
#define USBP_INTR	0x04
#define USBP_FRNUM	0x06
#define USBP_FRBASEADD	0x08
#define USBP_SOFMOD	0x0C
#define USBP_PORTSC0	0x10
#define USBP_PORTSC1	0x12
#define MAX_USB	16

class CUSBTD //Not implemented
{
  U32 td[4];
};

class CUSB //Not implemented
{
  U8 num, //USB dev num
     bus,dev,fun;
  U16 ports,pad;
  U32 *frame_lst;
};

//USB Pkt ID's
#define PID_OUT		0xE1
#define PID_IN		0x69
#define PID_SOF		0xA5
#define PID_SETUP	0x2D
#define PID_DATA0	0xC3
#define PID_DATA1	0x4B
#define PID_DATA2	0x87
#define PID_MDATA	0x0F
#define PID_ACK		0xD2
#define PID_NAK		0x5A
#define PID_STALL	0x1E
#define PID_NYET	0x96
#define PID_PRE		0x3C
#define PID_ERR		0x3C
#define PID_SPLIT	0x78
#define PID_PING	0xB4

//USB Std Rqsts
#define RQ_GET_STAT	0x0
#define RQ_CLR_FEAT	0x1
#define RQ_SET_FEAT	0x3
#define RQ_SET_ADDR	0x5
#define RQ_SET_DESC	0x7
#define RQ_GET_CFG	0x8
#define RQ_SET_CFG	0x9
#define RQ_GET_INTERFACE 0xA
#define RQ_SET_INTERFACE 0xB
#define RQ_SYNC_FRAME	0xC

I64 sys_num_usb=0;
CUSB sys_usb_devs[MAX_USB];
MemSet(sys_usb_devs,0,MAX_USB*sizeof(CUSB));


U0 USBInitOne(I64 b,I64 d,I64 f)
{
  CUSB *u;
  if (sys_num_usb<MAX_USB &&
      PCIReadU16(b,d,f,0)==0x8086) {
    u=&sys_usb_devs[sys_num_usb++];
    u->num=sys_num_usb;
    u->bus=b;
    u->dev=d;
    u->fun=f;
    u->ports=PCIReadU32(b,d,f,0x20)&~0x1F;
  }
}

U0 USBEndAll()
{
  sys_num_usb=0;
}

U0 USBInitAll() //This is only valid for my ICH10 dev
{
  if (sys_num_usb)
    USBEndAll;
  USBInitOne(0,29,0);
  USBInitOne(0,29,1);
  USBInitOne(0,29,2);
  USBInitOne(0,29,3);
  USBInitOne(0,26,0);
  USBInitOne(0,26,1);
  USBInitOne(0,26,2);
}

CUSBTD *USBAllocTD()
{
  return MAllocAligned(sizeof(CUSBTD),0x10,sys_uncached_heap);
}

U0 USBFreeTD(CUSBTD *temptd)
{
  Free(temptd);
}

U32 *USBAllocFrameLst(I64 usb_num,I64 size)
{ //aligned to 0x1000
  CUSB *u;
  if (0<=usb_num<sys_num_usb) {
    u=&sys_usb_devs[usb_num];
    Free(u->frame_lst);
    u->frame_lst=MAllocAligned(size*sizeof(U32),0x1000,sys_uncached_heap);
    OutU16(u->ports+USBP_CMD,0); //Stop
    OutU16(u->ports+USBP_FRNUM,0);
    OutU32(u->ports+USBP_FRBASEADD,u->frame_lst);
    return u->frame_lst;
  }
  return NULL;
}
