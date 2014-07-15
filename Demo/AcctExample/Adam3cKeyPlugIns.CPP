//Change anything you want in this
//acct file or others.

U0 FillInTime()
{
  CDate cdt;
  cdt=Now;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}

Bool MyPutKey(I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code

//See $LK,"Char",A="HI:Char"$ for definition of scan codes.
//See $LK,"Key Allocations",A="FI:::/Doc/KeyAlloc.TXT"$.
//See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.

//You can customize keys.  This routine
//is called before the main editor
//key handler $LK,"DocPutKey",A="MN:DocPutKey"$().
//You can intercept any key.

//Return TRUE if you completely
//handled the key.
  I64 i,sc2=sc.u8[0];
  U8 *st1,*st2;

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
	    "$$LTPURPLE$$";
	  else
	    "$$PURPLE$$";
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
	    "$$LTRED$$";
	  else
	    "$$RED$$";
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
	    "$$LTGREEN$$";
	  else
	    "$$GREEN$$";
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
	    "$$LTBLUE$$";
	  else
	    "$$FG$$";
	}
	break;
      case SC_F6:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Misc/Insert My URL");
	  else
	    SetKeyDesc("Misc/Insert Datetime");
	} else {
	  if (sc & SCF_SHIFT)
	    FillInMyURL;
	  else
	    FillInTime;
	}
	break;
      case SC_F7:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("God Book Passage");
	} else {
	  //$LK,"::/Adam/God/HSNotes.TXT"$
	  FifoU8Flush(god.fifo);
	  GodBitsIns(GOD_GOOD_BITS,KbdMouseEvtTime>>GOD_BAD_BITS);
	  if (sc & SCF_SHIFT)
	    GodBooksPassage("HOME/Archive/Text/*");
	}
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
	  case 'V':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Sys/Frame Grabber Toggle");
	    else
	      FrameGrabberToggle;
	    return TRUE;

	  case 'l':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/Put Link to Cur Pos on Clipboard");
	    else {
	      ClipboardDel;
	      st1=FileNameAbs(Define("ST_BIBLE_FILE"));
	      st2=FileNameAbs(DocPut->filename.name);
	      if (!StrCmp(st1,st2)) {
		Free(st1);
		st1=BibleLine2Verse(DocPut->cur_entry->y+1,',');
		DocPrint(sys_clipboard_doc,"$$LK,\"BF:%s\"$$",st1);
	      } else
		DocPrint(sys_clipboard_doc,"$$LK,\"FL:%s,%d\"$$",
		  st2,DocPut->cur_entry->y+1);
	      Free(st1);
	      Free(st2);
	    }
	    return TRUE;
	  case 'L':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/Place Anchor, Put Link to Clipboard");
	    else {
	      i=RandU32;
	      ClipboardDel;
	      DocPrint(sys_clipboard_doc,"$$LK,\"<TODO>\",A=\"FA:%s,ANC%d\"$$",
		DocPut->filename.name,i);
	      "$$AN,\"<TODO>\",A=\"ANC%d\"$$",i;
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

//Ins your own ALT-key plug-ins
	  case 'b':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Cmd/Jump to Bad Code");
	    else //$LK,"::/Adam/God/HSNotes.TXT"$
	      BadCodeJump;
	    return TRUE;

	  case 'g':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("BlogScreenShot");
	    else
	      BlogScreenShot;
	    return TRUE;

	  case 'p':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("JukeBox(TAD)");
	    else
	      JukeBox("::/Accts/TS/TAD/Psalmody");
	    return TRUE;
	  case 'P':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("JukeBox(Examples)");
	    else
	      JukeBox("::/Apps/Psalmody/Examples");
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
