//Place this file in /Home and change
//anything you want.

U0 InsTime()
{
  CDate cdt;
  cdt=Now;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}

U0 InsFileLink()
{
  U8 *st=PopUpPickFile;
  st[0]=':';  //This is my personal code, not production. LOL
  "$$LK,\"%s\",A=\"FI:%s\"$$",st+2,st;
  Free(st);
}

U0 InsDirLinks()
{
  CDirEntry *tempde,*tempde1;
  U8 *st=PopUpPickDir,*st2;
  st[0]=':';  //This is my personal code, not production. LOL
  st2=MStrPrint("%s/*",st);
  tempde=tempde1=FilesFind(st2,FUF_JUST_FILES|FUF_JUST_TXT);
  while (tempde) {
    tempde->full_name[0]=':';
    "$$LK,\"%s\",A=\"FI:%s\"$$\n",tempde->full_name+2,tempde->full_name;
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  Free(st);
  Free(st2);
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
	    KeyDescSet("Dollar/LTPURPLE");
	  else
	    KeyDescSet("Dollar/PURPLE");
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
	    KeyDescSet("Dollar/LTRED");
	  else
	    KeyDescSet("Dollar/RED");
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
	    KeyDescSet("Dollar/LTGREEN");
	  else
	    KeyDescSet("Dollar/GREEN");
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
	    KeyDescSet("Dollar/Default Color");
	  else
	    KeyDescSet("Dollar/BLUE");
	} else {
	  if (sc & SCF_SHIFT)
	    "$$FG$$";
	  else
	    "$$BLUE$$";
	}
	break;
      case SC_F6:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    KeyDescSet("Misc/Ins My URL");
	  else
	    KeyDescSet("Misc/Ins Datetime");
	} else {
	  if (sc & SCF_SHIFT)
	    InsMyURL;
	  else
	    InsTime;
	}
	break;
      case SC_F7:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    KeyDescSet("God Book Passage");
	} else {
	  //$LK,"::/Adam/God/HSNotes.TXT"$
	  FifoU8Flush(god.fifo);
	  GodBitsIns(GOD_GOOD_BITS,KbdMouseEvtTime>>GOD_BAD_BITS);
	  if (sc & SCF_SHIFT)
	    GodBooksPassage("C:/Home/Wb2/Files/Text/*");
	}
	break;
      case SC_F8:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    KeyDescSet("Misc/Ins Dir Links");
	  else
	    KeyDescSet("Misc/Ins File Link");
	} else {
	  if (sc & SCF_SHIFT)
	    InsDirLinks;
	  else
	    InsFileLink;
	}
	break;
    }
    return TRUE;
  } else {
    if (!(sc & SCF_CTRL)) {
      if (sc & SCF_ALT) {
	switch (ch) {
	  case 'a':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Win/AutoComplete(ON);");
	    else
	      AutoComplete(ON);
	    return TRUE;
	  case 'A':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Win/AutoComplete;");
	    else
	      AutoComplete;
	    return TRUE;

	  case 'h':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Win/Tile Horizontally");
	    else
	      WinTileHorz;
	    return TRUE;
	  case 'm':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Win/Maximize");
	    else {
	      WinMax;
	      WinBorder;
	    }
	    return TRUE;
	  case 'v':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Win/Tile Vertically");
	    else
	      WinTileVert;
	    return TRUE;
	  case 'V':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Sys/Frame Grabber Toggle");
	    else
	      FrameGrabberToggle;
	    return TRUE;

	  case 'l':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Put Link to Cur Pos on Clipboard");
	    else {
	      ClipboardDel;
	      st1=FileNameAbs(BIBLE_FILENAME);
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
	      KeyDescSet("Edit/Place Anchor, Put Link to Clipboard");
	    else {
	      i=RandU32;
	      ClipboardDel;
	      DocPrint(sys_clipboard_doc,"$$LK,\"<TODO>\",A=\"FA:%s,ANC%d\"$$",
		DocPut->filename.name,i);
	      "$$AN,\"<TODO>\",A=\"ANC%d\"$$",i;
	    }
	    return TRUE;

//Ins your own ALT-key plug-ins
	  case 'b':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd/Jump to Bad Code");
	    else //$LK,"::/Adam/God/HSNotes.TXT"$
	      BadCodeJump;
	    return TRUE;

	  case 'g':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("BlogScreenShot");
	    else
	      BlogScreenShot;
	    return TRUE;

	  case 'p':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("JukeBox(TAD)");
	    else
	      JukeBox("::/Home/Psalmody");
	    return TRUE;
	  case 'P':
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("JukeBox(Examples)");
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
