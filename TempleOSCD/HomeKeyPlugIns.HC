//Place this file in /Home and change
//anything you want.

U0 TimeIns()
{
  CDate cdt;
  cdt=Now;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}

Bool MyPutKey(I64 ch,I64 sc)
{//ch=ASCII; sc=scan_code

  //See $LK,"Char",A="HI:Char"$ for definition of scan codes.
  //See $LK,"Key Allocations",A="FI:::/Doc/KeyAlloc.DD"$.
  //See $LK,"Keyboard Devices",A="HI:Keyboard Devices/System"$.

  //You can customize keys.  This routine
  //is called before the main editor
  //key hndlr $LK,"DocPutKey",A="MN:DocPutKey"$().
  //You can intercept any key.

  //Return TRUE if you completely
  //handled the key.
  I64 i;
  U8 *st1,*st2;
  if (sc&SCF_ALT && !(sc&SCF_CTRL)) {
    switch (ch) {
      case 0:
	switch (sc.u8[0]) {
	  case SC_F1:
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /LTPURPLE");
	      else
		"$$LTPURPLE$$";
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /PURPLE");
	      else
		"$$PURPLE$$";
	    }
	    return TRUE;
	  case SC_F2:
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /LTRED");
	      else
		"$$LTRED$$";
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /RED");
	      else
		"$$RED$$";
	    }
	    return TRUE;
	  case SC_F3:
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /LTGREEN");
	      else
		"$$LTGREEN$$";
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /GREEN");
	      else
		"$$GREEN$$";
	    }
	    return TRUE;
	  case SC_F4:
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /Default Color");
	      else
		"$$FG$$";
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /BLUE");
	      else
		"$$BLUE$$";
	    }
	    return TRUE;
	  case SC_F7:
	    if (!(sc&SCF_SHIFT)) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Cmd /TimeIns");
	      else
		TimeIns;
	    }
	    return TRUE;
	}
	break;
      case 'a':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /AutoComplete On");
	else
	  AutoComplete(ON);
	return TRUE;
      case 'A':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /AutoComplete Off");
	else
	  AutoComplete;
	return TRUE;
      case 'h':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /WinTileHorz");
	else
	  WinTileHorz;
	return TRUE;
      case 'm':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /WinMax");
	else {
	  WinBorder;
	  WinMax;
	}
	return TRUE;
      case 'v':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /WinTileVert");
	else
	  WinTileVert;
	return TRUE;
      case 'l':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Edit/Put Link to Cur Pos on Clip");
	else {
	  ClipDel;
	  st1=FileNameAbs(BIBLE_FILENAME);
	  st2=FileNameAbs(DocPut->filename.name);
	  if (!StrCmp(st1,st2)) {
	    Free(st1);
	    st1=BibleLine2Verse(DocPut->cur_entry->y+1,',');
	    DocPrint(sys_clip_doc,"$$LK,\"BF:%s\"$$",st1);
	  } else
	    DocPrint(sys_clip_doc,"$$LK,\"FL:%s,%d\"$$",
		  st2,DocPut->cur_entry->y+1);
	  Free(st1);
	  Free(st2);
	}
	return TRUE;
      case 'L':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Edit/Place Anchor, Put Link to Clip");
	else {
	  i=RandU32;
	  ClipDel;
	  DocPrint(sys_clip_doc,"$$LK,\"<TODO>\",A=\"FA:%s,ANC%d\"$$",
		DocPut->filename.name,i);
	  "$$AN,\"<TODO>\",A=\"ANC%d\"$$",i;
	}
	return TRUE;

	//Ins your own ALT-key plug-ins
      case '1':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /ã");
	else
	  'ã';
	return TRUE;
      case '2':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /é");
	else
	  'é';
	return TRUE;
      case '3':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /è");
	else
	  'è';
	return TRUE;
      case '4':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /ê");
	else
	  'ê';
	return TRUE;
      case '9':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /Indent 5");
	else
	  "$$ID,5$$";
	return TRUE;
      case '0':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Dol /Unindent 5");
	else
	  "$$ID,-5$$";
	return TRUE;
    }
  }
  return FALSE;
}

Bool MyPutS(U8 *)
{
  return FALSE;
}

KeyDevAdd(&MyPutKey,&MyPutS,0x20000000,TRUE);
