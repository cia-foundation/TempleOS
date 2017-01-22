U0 GridInit()
{//Init mouse grid struct. See $LK,"::/Demo/Graphics/Grid.HC"$.
  ms_grid.x=ms_grid.y=ms_grid.z=8;
  ms_grid.x_offset=ms_grid.y_offset=ms_grid.z_offset=0;
  ms_grid.x_speed =ms_grid.y_speed =ms_grid.z_speed =1;
  ms_grid.show=ms_grid.snap=ms_grid.coord=FALSE;
}

U0 MsUpdate(I64 x,I64 y,I64 z,Bool l,Bool r)
{
  ms.presnap.x=ToI64(ms.scale.x*x)+ms.offset.x;
  ms.presnap.y=ToI64(ms.scale.y*y)+ms.offset.y;
  ms.presnap.z=ToI64(ms.scale.z*z)+ms.offset.z;
  if (ms_grid.snap) {
    ms.pos.x=Trunc(ms.presnap.x/ms_grid.x)*ms_grid.x+ms_grid.x_offset;
    ms.pos.y=Trunc(ms.presnap.y/ms_grid.y)*ms_grid.y+ms_grid.y_offset;
    ms.pos.z=Trunc(ms.presnap.z/ms_grid.z)*ms_grid.z+ms_grid.z_offset;
  } else {
    ms.pos.x=ms.presnap.x;
    ms.pos.y=ms.presnap.y;
    ms.pos.z=ms.presnap.z;
  }

  ms.pos.x=ClampI64(ms.pos.x,0,GR_WIDTH-1);
  ms.pos.y=ClampI64(ms.pos.y,0,GR_HEIGHT-1);
  ms.pos_text.x=ms.pos.x/FONT_WIDTH;
  if (ms.pos_text.x>=text.cols) {
    ms.pos_text.x=text.cols-1;
    ms.pos.x=text.cols*FONT_WIDTH-1;
  }
  ms.pos_text.y=ms.pos.y/FONT_HEIGHT;
  if (ms.pos_text.y>=text.rows) {
    ms.pos_text.y=text.rows-1;
    ms.pos.y=text.rows*FONT_HEIGHT-1;
  }
  ms.lb=l;
  ms.rb=r;
  LBEqu(&kbd.scan_code,SCf_MS_L_DOWN,ms.lb);
  LBEqu(&kbd.scan_code,SCf_MS_R_DOWN,ms.rb);
}

U0 MsSet(I64 x=I64_MAX,I64 y=I64_MAX,I64 z=I64_MAX,I64 l=I64_MAX,I64 r=I64_MAX)
{//Note: Generates a message. See $LK,"MsSet",A="FF:::/Demo/Games/Zing.HC,MsSet"$().
  if (!(0<=x<GR_WIDTH))
    x=ms.pos.x;
  if (!(0<=y<GR_HEIGHT))
    y=ms.pos.y;
  if (z==I64_MAX)
    z=ms.pos.z;

  if (!(FALSE<=l<=TRUE))
    l=ms.lb;
  if (!(FALSE<=r<=TRUE))
    r=ms.rb;

  x=(x-ms.offset.x)/ms.scale.x;
  y=(y-ms.offset.y)/ms.scale.y;
  z=(z-ms.offset.z)/ms.scale.z;
  MsUpdate(x,y,z,l,r);
  MsHardSet(x,y,z,l,r);
}

U0 MsInit()
{
  MemSet(&ms,0,sizeof(CMsStateGlbls));
  MemSet(&ms_last,0,sizeof(CMsStateGlbls));
  ms.offset.x=ms.offset.y=ms.offset.z=0;
  ms.scale.x=ms.scale.y=ms.scale.z=1.0;
  ms.pos_text.x=ms.pos_text.y=ms.pos_text.z=0;
  ms.has_wheel=FALSE;
  ms.show=TRUE;
  ms.speed=0;
  ms.timestamp=GetTSC;
  ms.dbl_time=0.350;
  GridInit;
}

U0 MsHardPktRead()
{
  U8 j;
  if (GetTSC>ms_hard.timestamp+cnts.time_stamp_freq>>3)
    FifoU8Flush(ms_hard.fifo);
  ms_hard.timestamp=GetTSC;
  FifoU8Ins(ms_hard.fifo,InU8(KBD_PORT));
  if (FifoU8Cnt(ms_hard.fifo)==ms_hard.pkt_size)
    while (FifoU8Rem(ms_hard.fifo,&j))
      FifoU8Ins(ms_hard.fifo2,j);
}

interrupt U0 IRQMsHard()
{
  CLD
  OutU8(0xA0,0x20);
  OutU8(0x20,0x20);
  ms_hard.irqs_working=TRUE;
  if (ms_hard.install_in_progress || !ms_hard.installed) {
    kbd.rst=TRUE;
    return;
  }
  MsHardPktRead;
}

U0 MsHardGetType()
{
  I64 b;
  KbdMsCmdAck(0xF2);
  b=KbdCmdRead;
  if (b==3)
    ms_hard.has_wheel=TRUE;
  else if (b==4)
    ms_hard.has_ext_bttns=TRUE;
}

Bool MsHardRst()
{
  U8 b,*_b;
  F64 timeout;
  Bool res=FALSE;

  ms_hard.has_wheel=FALSE;
  ms_hard.has_ext_bttns=FALSE;

  if (*0x40E(U16 *)==0x9FC0) {
    _b=0x9FC00+0x30;
    *_b=1; //This enables my mouse.  It might be for one machine.
//USB DMA packets, set-up by BIOS to make legacy PS/2?
  }

  try {
    KbdCmdFlush;
    KbdCmdSend(KBD_CTRL,0xAD); //Disable Kbd
    KbdCmdSend(KBD_CTRL,0xA8); //Enable Mouse

    KbdMsCmdAck(0xFF); //Rst

    timeout=tS+10.0;
    do
      try {
	KbdCmdRead;
	timeout=0; //force exit
      } catch
	Fs->catch_except=TRUE;
    while (tS<timeout);

    try
	    KbdCmdRead;
    catch
      Fs->catch_except=TRUE;

    KbdMsCmdAck(0xF3,200,0xF3,100,0xF3,80);
    MsHardGetType;
    KbdMsCmdAck(0xF3,10);
    MsHardGetType;
    KbdMsCmdAck(0xE8,0x03,0xE6,0xF3,100,0xF4);
    res=TRUE;

    //Enable IRQ 12
    KbdCmdSend(KBD_CTRL,0x20);
    b=KbdCmdRead;
    KbdCmdSend(KBD_CTRL,0x60);
    KbdCmdSend(KBD_PORT,(b|2)&~0x20);

  } catch
    Fs->catch_except=TRUE;

    //This is been added to override failure
    //because the mouse sometimes still works.
  res=TRUE;

  try
	  KbdCmdSend(KBD_CTRL,0xAE); //Enable Keyboard
  catch
    Fs->catch_except=TRUE;
  if (ms_hard.has_wheel || ms_hard.has_ext_bttns)
    ms_hard.pkt_size=4;
  else
    ms_hard.pkt_size=3;
  if (!res)
    try
	    KbdCmdSend(KBD_CTRL,0xA7); //Disable Mouse
    catch
      Fs->catch_except=TRUE;
  return res;
}

U0 MsHardSpeedSet()
{
  I64 dd,tmp;
  if ((dd=SqrI64(ms_hard_last.pos.x-ms_hard.pos.x)
	+SqrI64(ms_hard_last.pos.y-ms_hard.pos.y)) &&
	(tmp=ms_hard.timestamp-ms_hard_last.timestamp))
    ms_hard.speed=Sqrt(dd)*cnts.time_stamp_freq/tmp;
  ms_hard_last.timestamp=ms_hard.timestamp;
}

U0 MsHardSetPre()
{
  I64 old_timestamp=ms_hard_last.timestamp;
  MemCpy(&ms_hard_last,&ms_hard,sizeof(CMsHardStateGlbls));
  ms_hard_last.timestamp=old_timestamp;
}

U0 MsHardSetPost()
{
  I64 i;
  ms_hard.pos.x=ms_hard.prescale.x*ms_hard.scale.x*ms_grid.x_speed;
  ms_hard.pos.y=ms_hard.prescale.y*ms_hard.scale.y*ms_grid.y_speed;
  ms_hard.pos.z=ms_hard.prescale.z*ms_hard.scale.z*ms_grid.z_speed;

  i=Trunc(ms.scale.x*ms_hard.pos.x/ms_grid.x)*ms_grid.x+ms.offset.x;
//TODO ms_grid.x_offset?
  if (i<0)
    ms.offset.x-=i;
  else if (i>=GR_WIDTH)
    ms.offset.x+=GR_WIDTH-1-i;

  i=Trunc(ms.scale.y*ms_hard.pos.y/ms_grid.y)*ms_grid.y+ms.offset.y;
  if (i<0)
    ms.offset.y-=i;
  else if (i>=GR_HEIGHT)
    ms.offset.y+=GR_HEIGHT-1-i;

  if (ms_hard.pos.x!=ms_hard_last.pos.x || ms_hard.pos.y!=ms_hard_last.pos.y ||
	ms_hard.pos.z!=ms_hard_last.pos.z) {
    ms_hard.evt=TRUE;
    MsHardSpeedSet;
  } else
    for (i=0;i<5;i++)
      if (ms_hard.bttns[i]!=ms_hard_last.bttns[i]) {
	ms_hard.evt=TRUE;
	break;
      }
}

U0 MsHardHndlr()
{
  I64 i,dx,dy,dz;
  U8 ms_buf[4];

  MsHardSetPre;
  for (i=0;i<4;i++)
    ms_buf[i]=0;
  for (i=0;i<ms_hard.pkt_size;i++)
    if (!FifoU8Rem(ms_hard.fifo2,&ms_buf[i]))
      ms_buf[i]=0;

  ms_hard.bttns[0] = ms_buf[0] & 1;
  ms_hard.bttns[1] = (ms_buf[0] & 2) >> 1;
  ms_hard.bttns[2] = (ms_buf[0] & 4) >> 2;
  ms_hard.bttns[3] = (ms_buf[3] & 0x10) >> 4;
  ms_hard.bttns[4] = (ms_buf[3] & 0x20) >> 5;
  if (ms_buf[0] & 0x10)
    dx=ms_buf[1]-256;
  else
    dx=ms_buf[1];
  if (ms_buf[0] & 0x20)
    dy=256-ms_buf[2];
  else
    dy=-ms_buf[2];
  if (ms_buf[3] & 0x08)
    dz=ms_buf[3]&7-8;
  else
    dz=ms_buf[3]&7;

  ms_hard.prescale.x+=dx;
  ms_hard.prescale.y+=dy;
  ms_hard.prescale.z+=dz;

  MsHardSetPost;
}

U0 MsHardSet(I64 x,I64 y,I64 z,I64 l,I64 r)
{
  ms_hard.timestamp=GetTSC;
  MsHardSetPre;
  ms_hard.prescale.x=x/ms_hard.scale.x/ms_grid.x_speed;
  ms_hard.prescale.y=y/ms_hard.scale.y/ms_grid.y_speed;
  ms_hard.prescale.z=z/ms_hard.scale.z/ms_grid.z_speed;
  ms_hard.bttns[0]=l;
  ms_hard.bttns[1]=r;
  MsHardSetPost;
}

U0 KbdMsRst()
{
  KbdCmdFlush;
  FifoU8Flush(kbd.fifo2);
  FifoU8Flush(ms_hard.fifo2);
  FifoI64Flush(kbd.scan_code_fifo);
  kbd.scan_code=0;
  kbd.rst=FALSE;
}

Bool MsHardDrvrInstall(I64 dummy=0) //can be spawned
{
  no_warn dummy;
  I64 i;
  ms_hard.install_in_progress=TRUE;
  OutU8(0xA1,InU8(0xA1)|0x10);
  ms_hard.installed=ms_hard.irqs_working=FALSE;
  IntEntrySet(0x2C,&IRQMsHard);
  for(i=0;i<5;i++)
    ms_hard.bttns[i]=0;
  if (i=MsHardRst)
    OutU8(0xA1,InU8(0xA1)&~0x10);
  KbdMsRst;
  ms_hard.install_attempts++;
  ms_hard.installed=ms_hard.evt=i;
  ms_hard.install_in_progress=FALSE;
  return ms_hard.installed;
}

U0 KbdMsHndlr(Bool poll_kbd,Bool poll_ms)
{
  if (ms_hard.install_in_progress) {
    Yield;
    return;
  }
  if (kbd.rst)
    KbdMsRst;
  else {
    if (poll_ms && ms_hard.installed && !ms_hard.irqs_working) {
      PUSHFD
      CLI
      while (InU8(KBD_CTRL)&1)
	MsHardPktRead;
      POPFD
    }

    if (poll_kbd)
      while (InU8(KBD_CTRL)&1)
	KbdPktRead;

    if (kbd.rst)
      KbdMsRst;
    else {
      while (FifoU8Cnt(kbd.fifo2))
	KbdHndlr;
      while (FifoU8Cnt(ms_hard.fifo2))
	if (ms_hard.installed)
	  MsHardHndlr;
	else
	  KbdMsRst;
    }
  }
}

U0 KbdMsInit()
{
  MemSet(&kbd,0,sizeof(CKbdStateGlbls));
  kbd.fifo=FifoU8New(8);
  kbd.fifo2=FifoU8New(0x1000);
  kbd.scan_code_fifo=FifoI64New(0x1000);
  kbd.irqs_working=FALSE;
  MemSet(&ms_hard,0,sizeof(CMsHardStateGlbls));
  ms_hard.fifo=FifoU8New(8);
  ms_hard.fifo2=FifoU8New(0x1000);
  ms_hard.scale.x=0.5;
  ms_hard.scale.y=0.5;
  ms_hard.scale.z=1.0;
  ms_hard.prescale.x=GR_WIDTH/ms_hard.scale.x/2.0;
  ms_hard.prescale.y=GR_HEIGHT/ms_hard.scale.y/2.0;
  ms_hard.prescale.z=0/ms_hard.scale.z;
  ms_hard.pos.x=GR_WIDTH>>1;
  ms_hard.pos.y=GR_HEIGHT>>1;
  MemCpy(&ms_hard_last,&ms_hard,sizeof(CMsHardStateGlbls));
}
