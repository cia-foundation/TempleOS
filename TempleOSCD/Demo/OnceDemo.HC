U0 OnceDemo()
{//Set code to execute next reboot.
  Once("Beep;");
  Once("Dir;"); //You can issue multiple and they get queued.
  "Reboot ";
  if (YorN)
    BootRAM;
}

OnceDemo;

//There are variants for adam $LK,"AOnce",A="MN:AOnce"$().
