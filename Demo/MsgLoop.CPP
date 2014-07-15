U0 MsgLoop()
{
  I64 a1,a2,msg_code;
  "Use $LK,"msg_code",A="MN:MSG_CMD"$ defines in your programs instead of hardcoded nums.\n"
  "<ESC> to Exit\n";

  //We don't want the mouse buttons to reposition the cursor,
  //so we inhibit the window mgr from processing them.

  Fs->win_inhibit=WIF_SELF_IP_L|WIF_SELF_IP_R;

  do {
    msg_code=GetMsg(&a1,&a2,~(1<<MSG_NULL));
    "CMD:%02X A1:%08X A2:%016X\n",msg_code,a1,a2;
  } while (msg_code!=MSG_KEY_DOWN || a1!=CH_ESC && a1!=CH_SHIFT_ESC);
  GetMsg(,,1<<MSG_KEY_UP);

  Fs->win_inhibit=WIG_USER_TASK_DFT;
}

MsgLoop;

// Press <CTRL-SHIFT-L> and "Insert $LK,"ASCII",A="MN:CH_CTRLA"$/$LK,"ScanCode",A="FI:::/Doc/CharOverview.TXT"$".
