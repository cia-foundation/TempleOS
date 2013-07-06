//Change anything you want in this
//acct file or others.

U0 FillInTime()
{
  CDate cdt;
  cdt=GetCurTimeDate;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}

U0 RandQuote(I64 num)
{ //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
  I64 i,size,l=512;
  U8 *bible_filename=Define("ST_BIBLE_FILE"),
     *buf=FileRead(bible_filename,&size);
  if (buf) {
    i=num%(size-l);
    buf[i+l]=0;
    "" buf+i;
    Free(buf);
  }
}

U0 BadCodeJump(I64 num)
{ //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
  CDirEntry *root=FilesFind("/TempleOS/*",FUF_JUST_FILES|FUF_RECURSE|FUF_FLATTEN_TREE|FUF_JUST_SRC_FILES),
	     *tempm=root;
  I64 cnt=0;
  CDoc *doc;
  U8 *st;
  while (tempm) {
    doc=DocRead(tempm->full_name);
    tempm->user_data=doc->root.y;
    cnt+=doc->root.y;
    DocDel(doc);
    tempm=tempm->next;
  }
  num=num%cnt;
  tempm=root;
  while (tempm) {
    num-=tempm->user_data;
    if (num<0) {
      st=MSPrintF("FL:%s,%d",tempm->full_name,-num);
      break;
    }
    tempm=tempm->next;
  }
  CDirLstDel(root);
  Ed(st);
  Free(st);
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
      case SC_F5:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("God Passage");
	  else
	    SetKeyDesc("God Word");
	} else {
	  //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
	  if (sc & SCF_SHIFT)
	    RandQuote(KbdMouseEvtTime>>4);
	  else
	    "%s ",ws_sorted_words[KbdMouseEvtTime>>4%ws_num_words]->str;
	}
	break;
      case SC_F6:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Cmd/Init WordStat Spirit");
	  else
	    SetKeyDesc("Cmd/Init WordStat TempleOS");
	} else {
	  if (sc & SCF_SHIFT)
	    PopUp("WSInit(\"::/TempleOS/Apps/HolySpirit/Vocab.TXT*\");");
	  else
	    PopUp("WSInit(\"::/TempleOS/\" FILENAME_MASK_TXT \";~*Bible.TXT*\");");
	}
	break;
      case SC_F7:
	if (sc&SCF_KEY_DESC) {
	  if (sc & SCF_SHIFT)
	    SetKeyDesc("Dollar/Insert Hidden Widget");
	  else
	    SetKeyDesc("Misc/Insert Datetime");
	} else {
	  if (sc & SCF_SHIFT)
	    DocInsSafe(DocPut,
		"$$HS+C$$$$FG$$$$HE$$");
	  else
	    FillInTime();
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
	      ClipBoardDel;
	      DocPrintF(sys_clipboard_doc,"$$LK-A,\"FL:%s,%d\"$$",
		DocPut->filename.name,DocPut->cur_entry->y+1);
	    }
	    return TRUE;
	  case 'L':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/Place Anchor, Put Link to Clipboard");
	    else {
	      i=RandU32;
	      ClipBoardDel;
	      DocPrintF(sys_clipboard_doc,"$$LK,\"<TODO>\",\"FA:%s,ANC%d\"$$",
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

//Ins your own ALT-key plug-ins
	  case 'b':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Cmd/Jump to Bad Code");
	    else //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
	      BadCodeJump(KbdMouseEvtTime>>4);
	    return TRUE;

	  case 'p':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("JukeBox(TAD)");
	    else
	      JukeBox("::/TempleOS/Accts/TS/TAD/MusicOrgan");
	    return TRUE;
	  case 'P':
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("JukeBox(Examples)");
	    else
	      JukeBox("::/TempleOS/Apps/MusicOrgan/Examples");
	    return TRUE;
	}
      }
    }
  }
  return FALSE;
}

Bool MyPutS(U8 *st)
{
  nounusedwarn st;
  return FALSE;
}

KeyDevAdd(&MyPutKey,&MyPutS,0x20000000,TRUE);
