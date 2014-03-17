//See $LK,"Menus",A="HI:Menus"$

#define M_OPTION1	1
#define M_OPTION2	2

U0 PullDownMenuDemo()
{
  I64 msg_code,p1,p2;
  CMenuEntry *tempse;

  MenuPush(
  "File {"
  "  Open(,'O');"
  "  Save(,'S');"
  "  Exit(,CH_SHIFT_ESC);"
  "}"
  "Edit {"
  "  Cut(,,SC_DELETE|SCF_SHIFT);"
  "  Paste(,,SC_INS|SCF_SHIFT);"
  "}"
  "Misc {"
  "  Beep(,CH_SPACE);"
  "  Opt1(MSG_CMD,M_OPTION1);"
  "  Opt2(MSG_CMD,M_OPTION2);"
  "}"
  "Help {"
  "  Help(,,0x3B0000003B);"
  "  About(,,0x23B0000023B);"
      "}");

  "A pull-down menu will appear "
  "if you move the mouse to the top "
  "of the screen.\n\n";

  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN+1<<MSG_CMD);
    "\nCmd:%d P1:%X P2:%X\n\n",msg_code,p1,p2;

    switch (msg_code) {
      case MSG_KEY_DOWN:
	switch (p1) { //ASCII
	  case CH_SPACE:
	    Beep;
	    break;
	  case 0:
	    switch (p2.u8[0]) { //p2=Scan code
	      case SC_F1:
		if (p2&SCF_SHIFT)
		  "About PullDown Demo\n";
		else
		  "Test the menu.\n";
		break;
	      case SC_DELETE:
		if (p2&SCF_SHIFT)
		  "Cut\n";
		break;
	      case SC_INS:
		if (p2&SCF_SHIFT)
		  "Paste\n";
		break;
	    }
	    break;
	}
	break;

      case MSG_CMD:
      //I Recommend making your menu
      //drive by key strokes, but you can
      //use the $LK,"MSG_CMD",A="MN:MSG_CMD"$ val.
	switch (p1) {
	  case M_OPTION1:
	    "Option # 1\n";
	    if (tempse=MenuEntryFind(Fs->cur_menu,"Misc/Opt1"))
	      tempse->checked=!tempse->checked;
	    break;
	  case M_OPTION2:
	    "Option # 2\n";
	    if (tempse=MenuEntryFind(Fs->cur_menu,"Misc/Opt2"))
	      tempse->checked=!tempse->checked;
	    break;
	}
	break;
    }

  } while (msg_code!=MSG_KEY_DOWN||(p1!=CH_SHIFT_ESC && p1!=CH_ESC));
  MenuPop;
}

PullDownMenuDemo;
