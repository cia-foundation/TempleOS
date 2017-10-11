//Place this file in /Home and change
//anything you want.

U0 TimeIns()
{
  CDate cdt;
  cdt=Now;
  "$$IV,1$$----%D %T----$$IV,0$$\n",cdt,cdt;
}

U0 FileLinkIns()
{
  U8 *st=PopUpPickFile;
  st[0]=':';  //This is my personal code, not production. LOL
  "$$LK,\"%s\",A=\"FI:%s\"$$",st+2,st;
  Free(st);
}

U0 DirLinksIns()
{
  CDirEntry *tmpde,*tmpde1;
  U8 *st=PopUpPickDir,*st2;
  st[0]=':';  //This is my personal code, not production. LOL
  st2=MStrPrint("%s/*",st);
  tmpde=tmpde1=FilesFind(st2,FUF_JUST_FILES);
  while (tmpde) {
    tmpde->full_name[0]=':';
    "$$LK,\"%s\",A=\"FI:%s\"$$\n",tmpde->full_name+2,tmpde->full_name;
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  Free(st);
  Free(st2);
}

U0 DocHiddenDel(CDoc *doc=NULL)
{
  Bool unlock;
  CDocEntry *doc_e,*doc_e1;
  if (!doc) doc=DocPut;
  if (doc) {
    unlock=DocLock(doc);
    DocRecalc(doc);
    doc_e=doc->head.next;
    while (doc_e!=doc) {
      doc_e1=doc_e->next;
      if (doc_e->de_flags&(DOCEF_FILTER_SKIP|DOCEF_SKIP))
	DocEntryDel(doc,doc_e);
      doc_e=doc_e1;
    }
    DocRecalc(doc);
    if (unlock)
      DocUnlock(doc);
  }
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
	  case SC_F8:
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Cmd /DirLinksIns");
	      else
		DirLinksIns;
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Cmd /FileLinkIns");
	      else
		FileLinkIns;
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
      case 'f': //With sync
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /FrameGrabber Sync");
	else if (fg_on)
	  FrameGrabberToggle(FALSE,FALSE);
	else
	  FrameGrabberToggle(TRUE,FALSE);
	return TRUE;
      case 'F': //With sync and intro TOS theme
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /FrameGrabber Intro");
	else if (fg_on)
	  FrameGrabberToggle(FALSE,FALSE);
	else
	  FrameGrabberToggle(TRUE,TRUE);
	return TRUE;
      case 'h':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /WinTileHorz");
	else
	  WinTileHorz;
	return TRUE;
      case 'H':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Edit/Del Hidden Doc Entries");
	else
	  DocHiddenDel;
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
      case 'p':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /JukeBox");
	else
	  PopUp("#include \"::/Apps/Psalmody/Load\";"
		"JukeBox(\"::/Home/Sup3/Sup3Hymns\");");
	return TRUE;
      case 'P':
	if (sc&SCF_KEY_DESC)
	  KeyDescSet("Cmd /Psalmody");
	else
	  PopUp("#include \"::/Apps/Psalmody/Load\";"
		"Psalmody(\"~/Sup3/Sup3Hymns\");");
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
