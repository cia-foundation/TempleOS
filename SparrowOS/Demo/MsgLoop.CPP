U0 MsgLoop()
{
  I64 p1,p2,msg_code;
  "Use $LK,"these","MN:MSG_CMD"$ defines in your programs instead of hardcoded nums.\n"
  "<ESC> to Exit\n";

  //We don't want the mouse buttons to reposition the cursor,
  //so we inhibit the window mgr from processing them.

  Fs->win_inhibit=WIF_IP_L|WIF_IP_R;

  do {
    msg_code=GetMsg(&p1,&p2,~(1<<MSG_NULL));
    "CMD:%02X P1:%08X P2:%016X\n",msg_code,p1,p2;
  } while (msg_code!=MSG_KEY_DOWN || p1!=CH_ESC && p1!=CH_SHIFT_ESC);

  Fs->win_inhibit=0;
}

MsgLoop;
