#help_index "DolDoc/Output;StdOut/DolDoc"

public U0 DocPutKey(CDoc *doc,I64 ch=0,I64 sc=0)
{//$LK,"PutKey",A="MN:PutKey"$(ch,sc) at doc insert pt, cur_entry.
  I64 i,x,y;
  CDoc *m;
  CDocEntry *doc_ce;
  U8 *st,*st2;
  Bool unlock;

  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return;
  if (doc->user_put_key && (*doc->user_put_key)(doc,doc->user_put_data,ch,sc))
    return;
  unlock=DocLock(doc);
  if (!Bt(doldoc.clean_scan_codes,sc.u8[0]))
    doc->flags|=DOCF_UNDO_DIRTY;
  DocCaptureUndo(doc);
  if (Bt(char_bmp_printable,ch) && !(sc&(SCF_CTRL|SCF_ALT))) {
    if (sc&SCF_KEY_DESC) {
      st=Char2KeyName(ch,FALSE);
      KeyDescSet("Char  /'%s'",st);
      Free(st);
    } else
      EdCharIns(ch,sc,doc);
  } else {
    doc_ce=doc->cur_entry;
    x=doc->x; y=doc->y;
    if (sc&SCF_ALT)
      switch (ch) {
	case CH_BACKSPACE: //<CTRL-H>
	  if (!(sc&(SCF_SHIFT|SCF_CTRL))) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Undo");
	    else
	      DocUndoRestore(doc);
	  }
	  break;
      }
    else
      switch (ch) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_DOWN:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Cursor Down, Sel");
		  else
		    KeyDescSet("Edit/Cursor Down");
		} else
		  EdLineDown(doc,sc);
		break;
	      } else
		sc&=~SCF_CTRL;
//Fall Through to SC_END
	    case SC_END:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/GoTo Doc End, Sel");
		  else
		    KeyDescSet("Edit/GoTo Doc End");
		} else {
		  while (doc_ce!=doc) {
		    BEqu(&doc_ce->type,DOCEt_SEL,sc&SCF_SHIFT);
		    doc_ce=doc->cur_entry=doc_ce->next;
		  }
		  doc->cur_col=doc_ce->min_col;
		  DocFormBwd(doc);
		}
	      }
	      break;
	    case SC_CURSOR_UP:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Cursor Up, Sel");
		  else
		    KeyDescSet("Edit/Cursor Up");
		} else
		  EdLineUp(doc,sc);
		break;
	      } else
		sc&=~SCF_CTRL;
//Fall Through to SC_HOME
	    case SC_HOME:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/GoTo Top of Doc, Sel");
		  else
		    KeyDescSet("Edit/GoTo Top of Doc");
		} else {
		  if (doc_ce==doc) doc_ce=doc_ce->last;
		  while (doc_ce!=doc) {
		    BEqu(&doc_ce->type,DOCEt_SEL,sc&SCF_SHIFT);
		    doc_ce=doc->cur_entry=doc_ce->last;
		  }
		  doc_ce=doc->cur_entry=doc->head.next;
		  doc->cur_col=doc_ce->min_col;
		  DocFormFwd(doc);
		}
	      }
	      break;
	    case SC_PAGE_DOWN:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Page Down, Sel");
		  else
		    KeyDescSet("Edit/Page Down");
		} else {
		  i=doc_ce->y+doc->win_task->win_height-1;
		  if (doc_ce->type_u8==DOCT_HEX_ED)
		    i+=doc->cur_col/3/doc_ce->hex_ed_width;
		  while (doc_ce!=doc &&
			(doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y<i ||
			doc_ce->type_u8==DOCT_HEX_ED &&
			doc_ce->y+doc->cur_col/3/doc_ce->hex_ed_width<i)) {
		    EdLineDown(doc,sc);
//paranoid check for stuck on same node
		    if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		      break;
		    doc_ce=doc->cur_entry;
		  }
		}
	      }
	      break;
	    case SC_PAGE_UP:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Page Up, Sel");
		  else
		    KeyDescSet("Edit/Page Up");
		}else {
		  i=doc_ce->y-(doc->win_task->win_height-1);
		  if (doc_ce->type_u8==DOCT_HEX_ED)
		    i+=doc->cur_col/3/doc_ce->hex_ed_width;
		  while (doc_ce->last!=doc &&
			(doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y>i ||
			doc_ce->type_u8==DOCT_HEX_ED &&
			doc_ce->y+doc->cur_col/3/doc_ce->hex_ed_width>i) &&
			doc_ce->y!=doc->head.next->y) {
		    EdLineUp(doc,sc);
//paranoid check for stuck on same node
		    if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		      break;
		    doc_ce=doc->cur_entry;
		  }
		}
	      }
	      break;
	    case SC_CURSOR_LEFT:
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_CTRL)
		  KeyDescSet("Edit/GoTo Start of Line");
		else {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Cursor Left, Sel");
		  else
		    KeyDescSet("Edit/Cursor Left");
		}
	      } else
		EdCursorLeft(doc,sc);
	      break;
	    case SC_CURSOR_RIGHT:
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_CTRL)
		  KeyDescSet("Edit/GoTo End of Line");
		else {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Cursor Right, Sel");
		  else
		    KeyDescSet("Edit/Cursor Right");
		}
	      } else
		EdCursorRight(doc,sc);
	      break;
	    case SC_DELETE:
	      if (!(sc&SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Cut To Clip");
		  else
		    KeyDescSet("Char  /Delete");
		} else {
		  if (sc&SCF_SHIFT)
		    ClipCut(doc);
		  else
		    EdCharDel(doc);
		}
	      }
	      break;
	    case SC_INS:
	      if (sc&(SCF_SHIFT|SCF_CTRL)!=(SCF_SHIFT|SCF_CTRL)) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Edit/Paste Clip");
		  else if (sc&SCF_CTRL)
		    KeyDescSet("Edit/Copy to Clip");
		  else
		    KeyDescSet("Edit/Toggle Overstrike");
		} else {
		  if (sc&SCF_SHIFT)
		    ClipPaste(doc);
		  else if (sc&SCF_CTRL)
		    ClipCopy(doc);
		  else
		    doc->flags^=DOCF_OVERSTRIKE;
		}
	      }
	      break;
	    case SC_F1...SC_F10:
	      if (sc&SCF_CTRL) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    KeyDescSet("Cmd /Src Code of Sym");
		  else
		    KeyDescSet("Edit/Autocomplete Sym");
		} else {
		  DocUnlock(doc);
		  if (AutoComplete(ON)) {
		    if (sc&SCF_SHIFT)
		      ACMan(sc.u8[0]-SC_F1+1,Fs);
		    else
		      ACFillIn(sc.u8[0]-SC_F1+1);
		  }
		  DocLock(doc);
		}
	      } else {
		switch (sc.u8[0]) {
		  case SC_F1:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Cmd /About");
		      else
			KeyDescSet("Cmd /Help");
		    } else {
		      if (sc&SCF_SHIFT)
			Ed("::/Doc/AboutTempleOS.DD.Z");
		      else
			Ed("::/Doc/HelpIndex.DD.Z");
		    }
		    break;
		  case SC_F2:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Edit/Play Macro");
		      else
			KeyDescSet("Edit/Macro");
		    } else {
		      DocUnlock(doc);
		      if (sc&SCF_SHIFT) {
			if (TaskValidate(sys_macro_task))
			  PostMsgWait(sys_macro_task,
				MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
			SysMacroStripKey(&sys_macro_head,ch,sc);
			PlaySysMacro;
		      } else
			EdMacroUtil;
		      DocLock(doc);
		    }
		    break;
		  case SC_F3:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Edit/Find Last");
		      else
			KeyDescSet("Edit/Find Next");
		    }else {
		      doc->find_replace->scan_fwd=!(sc&SCF_SHIFT);
		      EdFindNext(doc);
		    }
		    break;
		  case SC_F4:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Cmd /Insert Directory Name");
		      else
			KeyDescSet("Cmd /Insert FileName");
		    } else {
		      DocUnlock(doc);
		      if (sc&SCF_SHIFT)
			st=PopUpPickDir;
		      else
			st=PopUpPickFile;
		      DocLock(doc);
		      if (st) {
			DocPrintPartial(doc,"%s",st);
			Free(st);
		      }
		    }
		    break;
		  case SC_F5:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Cmd /Adam Include");
		      else
			KeyDescSet("Cmd /Run (Execute)");
		    } else {
		      if (st2=DocEntryLink(doc,doc_ce)) {
			st=DocLinkFile(st2);
			Free(st2);
		      } else {
			DocWrite(doc);
			st=StrNew(doc->filename.name);
		      }
		      if (st2=DirFile(st,"Run","HC.Z")) {
			if (FileFind(st2)) {
			  Free(st);
			  st=st2;
			} else
			  Free(st2);
		      }
		      if (st) {
			if (sc&SCF_SHIFT)
			  AdamFile(st);
			else
			  PopUpFile(st);
			Free(st);
		      }
		    }
		    break;
		  case SC_F6:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Cmd /God Doodle");
		      else
			KeyDescSet("Cmd /God Song");
		    } else {
//$LK,"::/Adam/God/HSNotes.DD"$
		      if (sc&SCF_SHIFT) {
			DocUnlock(doc);
			GodDoodle;
			DocLock(doc);
		      } else
			GodSong;
		    }
		    break;
		  case SC_F7:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			KeyDescSet("Cmd /God Passage");
		      else
			KeyDescSet("Cmd /God Word");
		    } else {
//$LK,"::/Adam/God/HSNotes.DD"$
		      FifoU8Flush(god.fifo);
		      GodBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
		      if (sc&SCF_SHIFT)
			GodBiblePassage;
		      else
			GodWord;
		    }
		    break;
		}
	      }
	      break;
	  }
	  break;
	case CH_CTRLA:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Save As");
	    else if (DocWrite(doc,TRUE)&&(st=FileNameAbs(doc->filename.name))) {
	      DirContextDel(doc->filename.dirc);
	      doc->filename.dirc=DirContextNew(st);
	      Free(st);
	    }
	  }
	  break;
	case CH_CTRLB:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Toggle Border");
	    else
	      WinBorder(Bt(&doc->win_task->display_flags,
		    DISPLAYf_NO_BORDER),doc->win_task);
	  }
	  break;
	case CH_CTRLC:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Copy to Clip");
	    else
	      ClipCopy(doc);
	  }
	  break;
	case CH_CTRLD:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /File Manager");
	    else {
	      DocUnlock(doc);
	      FileMgr;
	      DocLock(doc);
	    }
	  }
	  break;
	case CH_CTRLF:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Search Files");
	    else
	      FindWiz;
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Find & Replace");
	    else
	      EdFindReplace(doc);
	  }
	  break;
	case CH_CTRLG:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/GoTo Line Num");
	    else
	      EdGoToLine(doc);
	  }
	  break;
	case CH_BACKSPACE: //<CTRL-H>
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Char  /Back Space");
	    else {
	      DocCaptureUndo(doc);
	      doc_ce=doc->cur_entry;
	      if (doc->cur_col<=doc_ce->min_col) {
		doc_ce=doc->cur_entry=doc_ce->last;
		if (doc_ce!=doc && doc_ce->type_u8==DOCT_SOFT_NEW_LINE)
		  doc_ce=doc->cur_entry=doc_ce->last;
		if (doc_ce==doc || doc_ce->type_u8==DOCT_PMT) {
		  doc_ce=doc->cur_entry=doc_ce->next;
		  doc->cur_col=doc_ce->min_col;
		} else {
		  doc->cur_col=doc_ce->max_col;
		  if (doc->cur_col>doc_ce->min_col)
		    doc->cur_col--;
		  EdCharDel(doc);
		}
	      } else {
		doc->cur_col--;
		EdCharDel(doc);
	      }
	    }
	  }
	  break;
	case CH_CTRLI:
	  if (sc.u8[0]!=SC_TAB) {
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /Unindent 2");
	      else
		DocPrint(doc,"$$ID,-2$$");
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Dol /Indent 2");
	      else
		DocPrint(doc,"$$ID,2$$");
	    }
	  }
	  break;
	case '\n':
	  if (sc&SCF_KEY_DESC) {
	    if (sc&SCF_SHIFT)
	      KeyDescSet("Char  /Return");
	    else
	      KeyDescSet("Char  /Page Break");
	  } else
	    EdCharIns(ch,sc,doc);
	  break;
	case CH_CTRLK:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Blinking Text Off");
	    else
	      DocPrint(doc,"$$BK,0$$");
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Blinking Text On");
	    else
	      DocPrint(doc,"$$BK,1$$");
	  }
	  break;
	case CH_CTRLL:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Code Tools");
	    else {
	      DocUnlock(doc);
	      EdCodeTools(doc);
	      DocLock(doc);
	    }
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Insert Text Widgets Wizard");
	    else {
	      DocUnlock(doc);
	      EdInsWidgetWiz;
	      DocLock(doc);
	    }
	  }
	  break;
	case CH_CTRLM:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Personal Notes");
	    else
	      Ed("~/PersonalNotes.DD.Z");
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Personal Menu");
	    else {
	      m=DocRead("~/PersonalMenu.DD.Z");
	      DocMenu(m);
	      DocDel(m);
	    }
	  }
	  break;
	case CH_CTRLO:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Collapse");
	    else
	      DocCollapse(TRUE,doc);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Uncolapse");
	    else
	      DocCollapse(FALSE,doc);
	  }
	  break;
	case CH_CTRLP:
	  if (doc->flags & (DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Toggle Super or Sub script");
	    else {
	      DocPrint(doc,"$$SY,0$$");
	      doc->flags&=~(DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE);
	    }
	  } else if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Toggle Subscript");
	    else {
	      DocPrint(doc,"$$SY,3$$");
	      doc->flags|=DOCF_SUBSCRIPT_MODE;
	    }
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Toggle Superscript");
	    else {
	      DocPrint(doc,"$$SY,-3$$");
	      doc->flags|=DOCF_SUPERSCRIPT_MODE;
	    }
	  }
	  break;
	case CH_CTRLQ:
	  break;
	case CH_CTRLR:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Cmd /Sprite Graphic Resource");
	    else
	      if (!(doc->flags&DOCF_FORM) &&
		    !(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS))) {
		DocUnlock(doc);
		if (doc_ce->type_u8==DOCT_SPRITE)
		  EdSpriteEd(doc);
		else
		  EdSpriteIns(doc);
		DocLock(doc);
	      }
	  }
	  break;
	case CH_CTRLS:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Toggle AutoSave");
	    else
	      LBtc(&doc->flags,DOCf_AUTO_SAVE);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Save");
	    else
	      DocWrite(doc);
	  }
	  break;
	case CH_CTRLT:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Single Entry Toggle Plain Text");
	    else if (!(doc->flags&DOCF_FORM))
	      DocEntryToggle(doc);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Toggle Plain Text Display");
	    else if (!(doc->flags&DOCF_FORM))
	      DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
	  }
	  break;
	case CH_CTRLU:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Underline Off");
	    else
	      DocPrint(doc,"$$UL,0$$");
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Underline On");
	    else
	      DocPrint(doc,"$$UL,1$$");
	  }
	  break;
	case CH_CTRLV:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Paste Clip");
	    else
	      ClipPaste(doc);
	  }
	  break;
	case CH_CTRLW:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Word Wrap Off");
	    else
	      DocPrint(doc,"$$WW,0$$");
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Word Wrap On");
	    else
	      DocPrint(doc,"$$WW,1$$");
	  }
	  break;
	case CH_CTRLX:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Cut To Clip");
	    else
	      ClipCut(doc);
	  }
	  break;
	case CH_CTRLY:
	  if (!(sc&SCF_SHIFT)) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Edit/Delete Line");
	    else
	      EdLineDel(doc);
	  }
	  break;
	case CH_CTRLZ:
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Inverted Text Off");
	    else
	      DocPrint(doc,"$$IV,0$$");
	  } else {
	    if (sc&SCF_KEY_DESC)
	      KeyDescSet("Dol /Inverted Text On");
	    else
	      DocPrint(doc,"$$IV,1$$");
	  }
	  break;
	case '0'...'9':
	  if (sc&SCF_CTRL) {
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		KeyDescSet("Cmd /Word Definition");
	      else
		KeyDescSet("Edit/Autocomplete Word");
	    } else {
	      if (AutoComplete(ON)) {
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  ACDDef(ch-'0',Fs);
		else
		  ACDFillin(ch-'0');
		DocLock(doc);
	      }
	    }
	  }
	  break;
	case '[':
	  if (sc&SCF_CTRL) {
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Edit/GoTo matching brace");
	      else
		EdFindPaired(doc,'}','{',FALSE);
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Edit/GoTo matching bracket");
	      else
		EdFindPaired(doc,']','[',FALSE);
	    }
	  }
	  break;
	case ']':
	  if (sc&SCF_CTRL) {
	    if (sc&SCF_SHIFT) {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Edit/GoTo matching brace");
	      else
		EdFindPaired(doc,'{','}',TRUE);
	    } else {
	      if (sc&SCF_KEY_DESC)
		KeyDescSet("Edit/GoTo matching bracket");
	      else
		EdFindPaired(doc,'[',']',TRUE);
	    }
	  }
	  break;
      }
  }
  if (unlock)
    DocUnlock(doc);
  if (!(doc->flags&DOCF_DONT_SWAP_OUT))
    Yield;
}

Bool KDDocPutKey(I64 ch,I64 scan_code)
{
  CDoc *doc;
  if (doc=DocPut)
    DocPutKey(doc,ch,scan_code);
  return FALSE;
}
