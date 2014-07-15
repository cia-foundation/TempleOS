U0 MousePktRead()
{
  U8 j;
  if (GetTSC>mouse.timestamp+cnts.time_stamp_freq>>3)
    FifoU8Flush(mouse.fifo);
  mouse.timestamp=GetTSC;
  FifoU8Ins(mouse.fifo,InU8(KBD_PORT));
  if (FifoU8Cnt(mouse.fifo)==mouse.pkt_size)
    while (FifoU8Rem(mouse.fifo,&j))
      FifoU8Ins(mouse.fifo2,j);
}

interrupt U0 MouseIntHandler()
{
  CLD
  OutU8(0xA0,0x20);
  OutU8(0x20,0x20);
  mouse.irqs_working=TRUE;
  if (mouse.install_in_progress || !mouse.installed) {
    kbd.rst=TRUE;
    return;
  }
  MousePktRead;
}

U0 MouseGetDevType()
{
  I64 b;
  KbdMouseCmdAck(0xF2);
  b=KbdCmdRead;
  if (b==3)
    mouse.has_wheel=TRUE;
  else if (b==4)
    mouse.has_ext_buttons=TRUE;
}

Bool MouseRst()
{
  U8 b,*_b;
  U16 *_w;
  F64 timeout;
  Bool result=FALSE,present;

  mouse.has_wheel=FALSE;
  mouse.has_ext_buttons=FALSE;

  _w=0x40E;
  present=MemPagePresentMark(_w,TRUE);
  if (*_w==0x9FC0) {
    _b=0x9FC00+0x30;
    *_b=1; //This enables my mouse.  It might be for one machine.
	   //USB DMA packets, set-up by BIOS to make legacy PS/2?
  }
  if (!present)
    MemPagePresentMark(_w,FALSE);

  try {
    KbdCmdFlush;
    KbdCmdSend(KBD_CTRL,0xAD); //Disable Kbd
    KbdCmdSend(KBD_CTRL,0xA8); //Enable Mouse

    KbdMouseCmdAck(0xFF); //Rst

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

    KbdMouseCmdAck(0xF3);
    KbdMouseCmdAck(200);
    KbdMouseCmdAck(0xF3); // enable Mouse
    KbdMouseCmdAck(100); // Set Rate
    KbdMouseCmdAck(0xF3);
    KbdMouseCmdAck(80); //Resolution

    MouseGetDevType; // enable Mouse

    KbdMouseCmdAck(0xF3);
    KbdMouseCmdAck(10);
    MouseGetDevType; // enable Mouse

    KbdMouseCmdAck(0xE8); //Resolution
    KbdMouseCmdAck(0x03);
    KbdMouseCmdAck(0xE6);

    KbdMouseCmdAck(0xF3); // Set Rate
    KbdMouseCmdAck(100);

    KbdMouseCmdAck(0xF4); // enable Mouse
    result=TRUE;

//Enable IRQ 12
    KbdCmdSend(KBD_CTRL,0x20);
    b=KbdCmdRead;
    KbdCmdSend(KBD_CTRL,0x60);
    KbdCmdSend(KBD_PORT,(b|2)&~0x20);

  } catch
    Fs->catch_except=TRUE;

//This is been added to override failure
//because the mouse sometimes still works.
	result=TRUE;

  try
    KbdCmdSend(KBD_CTRL,0xAE); //Enable Keyboard
  catch
    Fs->catch_except=TRUE;
  if (mouse.has_wheel || mouse.has_ext_buttons)
    mouse.pkt_size=4;
  else
    mouse.pkt_size=3;
  if (!result)
    try
      KbdCmdSend(KBD_CTRL,0xA7); //Disable Mouse
    catch
      Fs->catch_except=TRUE;
  return result;
}

U0 SetMouseSpeed()
{
  I64 dd,temp;
  if ((dd=SqrI64(mouse_last.pos.x-mouse.pos.x)+SqrI64(mouse_last.pos.y-mouse.pos.y)) &&
      (temp=mouse.timestamp-mouse_last.timestamp))
    mouse.speed=Sqrt(dd)*cnts.time_stamp_freq/temp;
  mouse_last.timestamp=mouse.timestamp;
}

U0 MouseUpdatePre()
{
  I64 old_timestamp=mouse_last.timestamp;
  MemCpy(&mouse_last,&mouse,sizeof(CMouseStateGlbls));
  mouse_last.timestamp=old_timestamp;
}

U0 MouseUpdatePost()
{
  I64 i;
  mouse.pos.x=mouse.prescale.x*mouse.scale.x*ip_grid.x_speed;
  mouse.pos.y=mouse.prescale.y*mouse.scale.y*ip_grid.y_speed;
  mouse.pos.z=mouse.prescale.z*mouse.scale.z*ip_grid.z_speed;

  i=Trunc(ip.scale.x*mouse.pos.x/ip_grid.x)*ip_grid.x+ip.offset.x; //TODO ip_grid.x_offset?
  if (i<0)
    ip.offset.x-=i;
  else if (i>=GR_WIDTH)
    ip.offset.x+=GR_WIDTH-1-i;

  i=Trunc(ip.scale.y*mouse.pos.y/ip_grid.y)*ip_grid.y+ip.offset.y;
  if (i<0)
    ip.offset.y-=i;
  else if (i>=GR_HEIGHT)
    ip.offset.y+=GR_HEIGHT-1-i;

  if (mouse.pos.x!=mouse_last.pos.x || mouse.pos.y!=mouse_last.pos.y || mouse.pos.z!=mouse_last.pos.z) {
    mouse.evt=TRUE;
    SetMouseSpeed;
  } else
    for (i=0;i<5;i++)
      if (mouse.buttons[i]!=mouse_last.buttons[i]) {
	mouse.evt=TRUE;
	break;
      }
}

U0 MouseHandler()
{
  I64 i,dx,dy,dz;
  U8 mouse_buf[4];

  MouseUpdatePre;
  for (i=0;i<4;i++)
    mouse_buf[i]=0;
  for (i=0;i<mouse.pkt_size;i++)
    if (!FifoU8Rem(mouse.fifo2,&mouse_buf[i]))
      mouse_buf[i]=0;

  mouse.buttons[0] = mouse_buf[0] & 1;
  mouse.buttons[1] = (mouse_buf[0] & 2) >> 1;
  mouse.buttons[2] = (mouse_buf[0] & 4) >> 2;
  mouse.buttons[3] = (mouse_buf[3] & 0x10) >> 4;
  mouse.buttons[4] = (mouse_buf[3] & 0x20) >> 5;
  if (mouse_buf[0] & 0x10)
    dx=mouse_buf[1]-256;
  else
    dx=mouse_buf[1];
  if (mouse_buf[0] & 0x20)
    dy=256-mouse_buf[2];
  else
    dy=-mouse_buf[2];
  if (mouse_buf[3] & 0x08)
    dz=mouse_buf[3]&7-8;
  else
    dz=mouse_buf[3]&7;

  mouse.prescale.x+=dx;
  mouse.prescale.y+=dy;
  mouse.prescale.z+=dz;

  MouseUpdatePost;
}

U0 SetMouseIP(I64 x,I64 y,I64 z,I64 l,I64 r)
{
  mouse.timestamp=GetTSC;
  MouseUpdatePre;
  mouse.prescale.x=x/mouse.scale.x/ip_grid.x_speed;
  mouse.prescale.y=y/mouse.scale.y/ip_grid.y_speed;
  mouse.prescale.z=z/mouse.scale.z/ip_grid.z_speed;
  mouse.buttons[0]=l;
  mouse.buttons[1]=r;
  MouseUpdatePost;
}

U0 KbdMouseRst()
{
  KbdCmdFlush;
  FifoU8Flush(kbd.fifo2);
  FifoU8Flush(mouse.fifo2);
  FifoI64Flush(kbd.scan_code_fifo);
  kbd.scan_code=0;
  kbd.rst=FALSE;
}

Bool MouseDriverInstall(I64 dummy=0) //can be spawned
{
  nounusedwarn dummy;
  I64 i;
  Bool old_preempt=IsPreempt;
  Preempt(ON);
  mouse.install_in_progress=TRUE;
  OutU8(0xA1,InU8(0xA1)|0x10);
  mouse.installed=mouse.irqs_working=FALSE;
  IntEntrySet(0x2C,&MouseIntHandler);
  for(i=0;i<5;i++)
    mouse.buttons[i]=0;
  if (i=MouseRst)
    OutU8(0xA1,InU8(0xA1)&~0x10);
  KbdMouseRst;
  mouse.install_attempts++;
  mouse.installed=mouse.evt=i;
  mouse.install_in_progress=FALSE;
  Preempt(old_preempt);
  return mouse.installed;
}

U0 KbdMouseHandler(Bool poll_kbd,Bool poll_mouse)
{
  if (mouse.install_in_progress) {
    Yield;
    return;
  }
  if (kbd.rst)
    KbdMouseRst;
  else {
    if (poll_mouse &&
	mouse.installed &&
	!mouse.irqs_working) {
      PUSHFD
      CLI
      while (InU8(KBD_CTRL)&1)
	MousePktRead;
      POPFD
    }

    if (poll_kbd)
      while (InU8(KBD_CTRL)&1)
	KbdPktRead;

    if (kbd.rst)
      KbdMouseRst;
    else {
      while (FifoU8Cnt(kbd.fifo2))
	KbdHandler;
      while (FifoU8Cnt(mouse.fifo2))
	if (mouse.installed)
	  MouseHandler;
	else
	  KbdMouseRst;
    }
  }
}

U0 KbdMouseVarsInit()
{
  MemSet(&kbd,0,sizeof(CKbdStateGlbls));
  kbd.fifo=FifoU8New(8);
  kbd.fifo2=FifoU8New(64);
  kbd.scan_code_fifo=FifoI64New(128);
  kbd.irqs_working=FALSE;
  MemSet(&mouse,0,sizeof(CMouseStateGlbls));
  mouse.fifo=FifoU8New(8);
  mouse.fifo2=FifoU8New(64);
  mouse.scale.x=0.5;
  mouse.scale.y=0.5;
  mouse.scale.z=1.0;
  mouse.prescale.x=GR_WIDTH/mouse.scale.x/2.0;
  mouse.prescale.y=GR_HEIGHT/mouse.scale.y/2.0;
  mouse.prescale.z=0/mouse.scale.z;
  mouse.pos.x=GR_WIDTH>>1;
  mouse.pos.y=GR_HEIGHT>>1;
  MemCpy(&mouse_last,&mouse,sizeof(CMouseStateGlbls));
}
