//Change anything you want in this
//acct file or others.

U0 FillInTime()
{
  CDate cdt;
  cdt=GetDateTime;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}


Bool MyPutKey(I64 ch,I64 sc)
{  //ch=ASCII; sc=scancode

//See $LK,"Char","HI:Char"$ for definition of scan codes.
//See $LK,"Keyboard Handling","FI:::/TempleOS/Doc/KeyBoard.TXT"$

//You can customize keys.  This routine
//is called before the main editor
//key handler $LK,"DocPutKey","MN:DocPutKey"$().
//You can intercept any key.

//Return TRUE if you completely
//handled the key.

  I64 i,sc2=sc.u8[0];

  if (SC_F1<=sc2<=SC_F10 &&
      sc&SCF_ALT &&
      !(sc & SCF_CTRL)) {
    switch (sc2) {
      case SC_F1:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Dollar/LTPURPLE");
	  else
	    SetKeyDesc("Dollar/PURPLE");
	} else {
	  if (sc & SCF_SHIFT)
	    "$$FG,LTPURPLE$$";
	  else
	    "$$FG,PURPLE$$";
	}
	break;
      case SC_F2:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Dollar/LTRED");
	  else
	    SetKeyDesc("Dollar/RED");
	} else {
	  if (sc & SCF_SHIFT)
	    "$$FG,LTRED$$";
	  else
	    "$$FG,RED$$";
	}
	break;
      case SC_F3:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Dollar/LTGREEN");
	  else
	    SetKeyDesc("Dollar/GREEN");
	} else {
	  if (sc & SCF_SHIFT)
	    "$$FG,LTGREEN$$";
	  else
	    "$$FG,GREEN$$";
	}
	break;
      case SC_F4:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Dollar/LTBLUE");
	  else
	    SetKeyDesc("Dollar/Default Color");
	} else {
	  if (sc & SCF_SHIFT)
	    "$$FG,LTBLUE$$";
	  else
	    "$$FG$$";
	}
	break;
      case SC_F6:
	if (sc&SCF_KEY_DESC) {
	  if (!(sc & SCF_SHIFT))
	    SetKeyDesc("Cmd/Init WordStat LT");
	} else if (!(sc & SCF_SHIFT))
	  PopUp("WSInit(\"/TempleOS/*;~*Bible.TXT*\");");
	break;
      case SC_F7:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Misc/Insert Datetime");
	else
	  FillInTime;
	break;
    }
    return TRUE;
  } else {
    if (!(sc & SCF_CTRL)) {
      if (sc & SCF_ALT) {
	switch (ch) {
	  case 'h':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Win/Tile Horizontally");
	    else
	      WinTileHorz;
	    return TRUE;
	  case 'm':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Win/Maximize");
	    else {
	      WinMax;
	      WinBorder;
	    }
	    return TRUE;
	  case 'v':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Win/Tile Vertically");
	    else
	      WinTileVert;
	    return TRUE;

	  case 'l':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/Put Link to Cur Pos on Clipboard");
	    else {
	      ClipBoardDel;
	      DocPrint(sys_clipboard_doc,"$$LK-A,\"FL:%s,%d\"$$",
		DocPut->filename.name,DocPut->cur_entry->y+1);
	    }
	    return TRUE;
	  case 'L':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/Place Anchor, Put Link to Clipboard");
	    else {
	      i=RandU32;
	      ClipBoardDel;
	      DocPrint(sys_clipboard_doc,"$$LK,\"<TODO>\",\"FA:%s,ANC%d\"$$",
		DocPut->filename.name,i);
	      "$$AN,\"<TODO>\",\"ANC%d\"$$",i;
	    }
	    return TRUE;

	  case 'w':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Win/WordStat(ON);");
	    else
	      WordStat(ON);
	    return TRUE;
	  case 'W':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Win/WordStat;");
	    else
	      WordStat;
	    return TRUE;

	}
      }
    }
  }
  return FALSE;
}

Bool MyPutS(U8 *)
{
  return FALSE;
}

KeyDevAdd(&MyPutKey,&MyPutS,0x20000000,TRUE);
