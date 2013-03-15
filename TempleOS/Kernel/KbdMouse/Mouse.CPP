U0 MousePktRead()
{
  U8 j;
  if (GetTimeStamp>mouse_evt_time+time_stamp_freq>>3)
    FifoU8Flush(mouse_fifo);
  mouse_evt_time=GetTimeStamp;
  FifoU8Ins(mouse_fifo,InU8(KBD_PORT));
  if (FifoU8Cnt(mouse_fifo)==mouse_pkt_size)
    while (FifoU8Rem(mouse_fifo,&j))
      FifoU8Ins(mouse2_fifo,j);
}

interrupt U0 MouseIntHandler()
{
  Cld;
  OutU8(0xA0,0x20);
  OutU8(0x20,0x20);
  mouse_irqs_working=TRUE;
  if (mouse_install_attempt_in_progress || !mouse_installed) {
    kbdmouse_rst=TRUE;
    return;
  }
  MousePktRead;
}

U0 MouseGetDevType()
{
  U8 b;
  KbdMouseCmdAck(0xF2);
  b=KbdCmdRead;
  if (b==3)
    mouse_has_wheel=TRUE;
  else if (b==4)
    mouse_has_ext_buttons=TRUE;
}

Bool MouseRst()
{
  U8 b,*_b;
  U16 *_w;
  I64 i;
  Bool result=FALSE,timeout,present;

  mouse_has_wheel=FALSE;
  mouse_has_ext_buttons=FALSE;

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
    for (i=0;i<250;i++) {
      try {
	KbdCmdRead;
	timeout=FALSE;
      } catch {
	Fs->catch_except=TRUE;
	timeout=TRUE;
      }
      if (!timeout) break;
    }
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
  if (mouse_has_wheel || mouse_has_ext_buttons)
    mouse_pkt_size=4;
  else
    mouse_pkt_size=3;
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
  if (dd=(old_mxx-mxx)*(old_mxx-mxx)+
	 (old_myy-myy)*(old_myy-myy))
    if (temp=mouse_evt_time-mouse_last_evt_time)
      mouse_speed=Sqrt(dd)*time_stamp_freq/temp;
  mouse_last_evt_time=mouse_evt_time;
}

U0 MouseUpdatePre(Bool *old_buttons)
{
  I64 i;
  old_mxx=mxx; old_myy=myy; old_mzz=mzz;
  for (i=0;i<5;i++)
    old_buttons[i]=mouse_buttons[i];
}

U0 MouseUpdatePost(Bool *old_buttons)
{
  I64 i;
  CSnap s;

  GetSnap(&s);

  if (ip_snap_last.dx!=s.dx)
    mxx_prescale*=ip_snap_last.dx/s.dx;
  if (ip_snap_last.dy!=s.dy)
    myy_prescale*=ip_snap_last.dy/s.dy;
  if (ip_snap_last.dz!=s.dz)
    mzz_prescale*=ip_snap_last.dz/s.dz;

  mxx=mxx_prescale*mxx_scale*s.dx;
  myy=myy_prescale*myy_scale*s.dy;
  mzz=mzz_prescale*mzz_scale*s.dz;

  MemCpy(&ip_snap_last,&s,sizeof(CSnap));

  i=ToI64(ipx_scale*mxx/s.x)*s.x+ipx_offset;
  if (i<0)
    ipx_offset-=i;
  else if (i>=gr_width)
    ipx_offset+=gr_width-1-i;

  i=ToI64(ipy_scale*myy/s.y)*s.y+ipy_offset;
  if (i<0)
    ipy_offset-=i;
  else if (i>=gr_height)
    ipy_offset+=gr_height-1-i;

  if (mxx!=old_mxx||myy!=old_myy||mzz!=old_mzz) {
    mouse_evt=TRUE;
    SetMouseSpeed;
  } else
    for (i=0;i<5;i++)
      if (mouse_buttons[i]!=old_buttons[i]) {
	mouse_evt=TRUE;
	break;
      }
}

U0 MouseHandler()
{
  I64 i,dx,dy,dz;
  U8 mouse_buf[4];
  Bool old_buttons[5];

  MouseUpdatePre(old_buttons);

  for (i=0;i<4;i++)
    mouse_buf[i]=0;
  for (i=0;i<mouse_pkt_size;i++)
    if (!FifoU8Rem(mouse2_fifo,&mouse_buf[i]))
      mouse_buf[i]=0;

  mouse_buttons[0] = mouse_buf[0] & 1;
  mouse_buttons[1] = (mouse_buf[0] & 2) >> 1;
  mouse_buttons[2] = (mouse_buf[0] & 4) >> 2;
  mouse_buttons[3] = (mouse_buf[3] & 0x10) >> 4;
  mouse_buttons[4] = (mouse_buf[3] & 0x20) >> 5;
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

  mxx_prescale+=dx;
  myy_prescale+=dy;
  mzz_prescale+=dz;

  MouseUpdatePost(old_buttons);
}

U0 SetMouseIP(I64 x,I64 y,I64 z,I64 l,I64 r,I64 c)
{
  Bool old_buttons[5];
  CSnap s;
  GetSnap(&s);
  mouse_evt_time=GetTimeStamp;
  MouseUpdatePre(old_buttons);
  mxx_prescale=x/mxx_scale/s.dx;
  myy_prescale=y/myy_scale/s.dy;
  mzz_prescale=z/mzz_scale/s.dz;
  mouse_buttons[0]=l;
  mouse_buttons[1]=r;
  mouse_buttons[2]=c; //unsure which one
  MouseUpdatePost(old_buttons);
}

U0 KbdMouseRst()
{
  KbdCmdFlush;
  FifoU8Flush(kbd2_fifo);
  FifoU8Flush(mouse2_fifo);
  FifoI64Flush(scan_code_fifo);
  sys_cur_scan_code=0;
  kbdmouse_rst=FALSE;
}

Bool MouseDriverInstall(I64 dummy=0) //can be spawned
{
  nounusedwarn dummy;
  I64 i;
  Bool old_preempt=IsPreempt;
  if (!IsSysDeterministic)
    Preempt(ON);
  mouse_install_attempt_in_progress=TRUE;
  OutU8(0xA1,InU8(0xA1)|0x10);
  mouse_installed=mouse_irqs_working=FALSE;
  IntEntrySet(0x2C,&MouseIntHandler);
  for(i=0;i<5;i++)
    mouse_buttons[i]=0;
  if (i=MouseRst)
    OutU8(0xA1,InU8(0xA1)&~0x10);
  KbdMouseRst;
  mouse_install_attempts++;
  mouse_installed=mouse_evt=i;
  mouse_install_attempt_in_progress=FALSE;
  Preempt(old_preempt);
  return mouse_installed;
}

U0 KbdMouseHandler(Bool poll_kbd,Bool poll_mouse)
{
  if (mouse_install_attempt_in_progress) {
    Yield;
    return;
  }
  if (kbdmouse_rst)
    KbdMouseRst;
  else {
    if (poll_mouse &&
	mouse_installed &&
	!mouse_irqs_working) {
      PushFD;
      Cli;
      while (InU8(KBD_CTRL)&1)
	MousePktRead;
      PopFD;
    }

    if (poll_kbd)
      while (InU8(KBD_CTRL)&1)
	KbdPktRead;

    if (kbdmouse_rst)
      KbdMouseRst;
    else {
      while (FifoU8Cnt(kbd2_fifo))
	KbdHandler;
      while (FifoU8Cnt(mouse2_fifo))
	if (mouse_installed)
	  MouseHandler;
	else
	  KbdMouseRst;
    }
  }
}

U0 KbdMouseVarsInit()
{
  sys_key_cnt=0;
  scan_code_fifo=FifoI64New(128);
  kbd_fifo=FifoU8New(8);
  kbd2_fifo=FifoU8New(64);
  mouse_fifo=FifoU8New(8);
  mouse2_fifo=FifoU8New(64);
  MemSet(key_down_bitmap,0,256/8);
  MemSet(key_down_bitmap2,0,256/8);
  sys_last_down_scan_code=0;
  sys_cur_scan_code=0;
  kbdmouse_rst=FALSE;
  kbd_irqs_working=FALSE;
  mouse_irqs_working=FALSE;
  mxx_scale=0.5;
  myy_scale=0.5;
  mzz_scale=1.0;
  mxx_prescale=gr_width/mxx_scale/2.0;
  myy_prescale=gr_height/myy_scale/2.0;
  mzz_prescale=0/mzz_scale;
  mxx=gr_width>>1;
  myy=gr_height>>1;
  mzz=0;
  mouse_speed=0.0;
  old_mxx=mxx;
  old_myy=myy;
  old_mzz=mzz;
  mouse_installed=FALSE;
  mouse_install_attempts=0;
  mouse_last_evt_time=0;
  mouse_has_wheel=FALSE;
  mouse_has_ext_buttons=FALSE;
  mouse_evt=FALSE;
}

