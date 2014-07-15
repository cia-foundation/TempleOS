#help_index "DolDoc/Output"

public U0 DocPutKey(CDoc *doc,I64 ch=0,I64 sc=0)
{//$LK,"PutKey",A="MN:PutKey"$(ch,sc) at doc insert pt, cur_entry.
  I64 i,x,y,sc2,ext_ascii_cnt=0;
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
  if (Bt(chars_bitmap_getkey,ch) &&
	!(sc&(SCF_CTRL|SCF_ALT))) {
    if (sc&SCF_KEY_DESC) {
      st=Char2KeyName(ch,FALSE);
      SetKeyDesc("Char/'%s'",st);
      Free(st);
    } else
      EdInsChar(ch,sc,doc);
  } else {
    doc_ce=doc->cur_entry;
    x=doc->x; y=doc->y;
    sc2=sc.u8[0];
    switch (ch) {
      case CH_CTRLA:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Save As");
	else
	  DocWrite(doc,TRUE);
	break;
      case CH_CTRLB:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Win/Toggle Border");
	else
	  WinBorder(Bt(&doc->win_task->display_flags,DISPLAYf_NO_BORDER),doc->win_task);
	break;
      case CH_CTRLC:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Copy to Clipboard");
	else
	  EdCopyToClipboard(doc);
	break;
      case CH_CTRLD:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Cmd/Jump to File Manager");
	else {
	  DocUnlock(doc);
	  FileMgr;
	  DocLock(doc);
	}
	break;
      case CH_CTRLF:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Cmd/Grep Wizard");
	  else
	    GrepWiz;
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Find & Replace");
	  else
	    EdFindReplace(doc);
	}
	break;
      case CH_CTRLG:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/GoTo Line Num");
	else
	  EdGoToLine(doc);
	break;
      case CH_BACKSPACE: //<CTRL-H>
	if (sc&SCF_ALT && !(sc&SCF_CTRL)) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Undo");
	  else
	    DocUndoRestore(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Char/Back Space");
	  else {
	    DocCaptureUndo(doc);
	    doc_ce=doc->cur_entry;
	    if (doc->cur_data_col<=doc_ce->min_col) {
	      doc_ce=doc->cur_entry=doc_ce->last;
	      if (doc_ce!=doc && doc_ce->type_u8==DOCT_SOFT_NEW_LINE)
		doc_ce=doc->cur_entry=doc_ce->last;
	      if (doc_ce==doc || doc_ce->type_u8==DOCT_PMT) {
		doc_ce=doc->cur_entry=doc_ce->next;
		doc->cur_data_col=doc_ce->min_col;
	      } else {
		doc->cur_data_col=doc_ce->max_col;
		if (doc->cur_data_col>doc_ce->min_col)
		  doc->cur_data_col--;
		EdDelChar(doc);
	      }
	    } else {
	      doc->cur_data_col--;
	      EdDelChar(doc);
	    }
	  }
	}
	break;
      case CH_CTRLI:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Indent Out");
	  else
	    DocPrint(doc,"$$ID,-2$$");
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Indent In");
	  else
	    DocPrint(doc,"$$ID,2$$");
	}
	break;
      case '\n':
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Char/Return");
	  else
	    SetKeyDesc("Char/Page Break");
	} else
	  EdInsChar(ch,sc,doc);
	break;
      case CH_CTRLK:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Blinking Text");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$BK,0$$");
	else
	  DocPrint(doc,"$$BK,1$$");
	break;
      case CH_CTRLL:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Edit/Code Tools");
	  else
	    SetKeyDesc("Dollar/Wizard");
	} else {
	  if (sc&SCF_SHIFT)
	    EdCodeTools(doc);
	  else {
	    DocUnlock(doc);
	    EdInsDollarCmd;
	    DocLock(doc);
	  }
	}
	break;
      case CH_CTRLM:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Cmd/Jump to Notes File");
	  else
	    SetKeyDesc("Cmd/Jump to Menu");
	} else {
	  if (sc&SCF_SHIFT)
	    Ed("HOME/PersonalNotes.TXT.Z");
	  else {
	    m=DocRead("HOME/PersonalMenu.TXT.Z");
	    DocMenuDo(m);
	    DocDel(m);
	  }
	}
	break;
      case CH_CTRLO:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Edit/Collapse");
	  else
	    SetKeyDesc("Edit/Uncolapse");
	} else {
	  if (sc&SCF_SHIFT)
	    DocCollapse(TRUE,doc);
	  else
	    DocCollapse(FALSE,doc);
	}
	break;
      case CH_CTRLP:
	if (doc->flags & (DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE)) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Super or Sub script");
	  else {
	    DocPrint(doc,"$$SY,0$$");
	    doc->flags&=~(DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE);
	  }
	} else if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Subscript");
	  else {
	    DocPrint(doc,"$$SY,3$$");
	    doc->flags|=DOCF_SUBSCRIPT_MODE;
	  }
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Superscript");
	  else {
	    DocPrint(doc,"$$SY,-3$$");
	    doc->flags|=DOCF_SUPERSCRIPT_MODE;
	  }
	}
	break;
      case CH_CTRLQ:
	break;
      case CH_CTRLR:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Edit or Create Graphic");
	else {
	  if (!(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))) {
	    DocUnlock(doc);
	    if (doc_ce->type_u8==DOCT_SPRITE)
	      EdSpriteEd(doc,
		    (doc_ce->x-doc->line_start_col+doc_ce->max_col)*FONT_WIDTH,
		    (doc_ce->y-doc->cur_top_line_num)*FONT_HEIGHT);
	    else
	      EdSpriteIns(doc,
		    (doc_ce->x+doc->cur_data_col-doc->line_start_col)*FONT_WIDTH,
		    (doc_ce->y-doc->cur_top_line_num)*FONT_HEIGHT);
	    DocLock(doc);
	  }
	}
	break;
      case CH_CTRLS:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Find Misspelled");
	  else
	    WSFindMisspelled(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Save");
	  else
	    DocWrite(doc);
	}
	break;
      case CH_CTRLT:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Single Entry Toggle Plain Text");
	  else
	    DocEntryToggle(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Toggle Plain Text Display");
	  else
	    if (!(doc->flags&DOCF_FORM) || doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))
	      DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
	}
	break;
      case CH_CTRLU:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Underline");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$UL,0$$");
	else
	  DocPrint(doc,"$$UL,1$$");
	break;
      case CH_CTRLV:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Paste Clipboard");
	else
	  EdPasteClipboard(doc);
	break;
      case CH_CTRLW:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Word Wrap");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$WW,0$$");
	else
	  DocPrint(doc,"$$WW,1$$");
	break;
      case CH_CTRLX:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Cut To Clipboard");
	else
	  EdCutToClipboard(doc);
	break;
      case CH_CTRLY:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Delete Line");
	else
	  EdDelLine(doc);
	break;
      case CH_CTRLZ:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Inverted Text");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$IV,0$$");
	else
	  DocPrint(doc,"$$IV,1$$");
	break;
      case '0'...'9':
	if (!(sc&SCF_ALT)) {
	  if (sc&SCF_CTRL) {
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Jump to Word Definition");
	      else
		SetKeyDesc("Edit/Autocomplete Word");
	    } else {
	      if (WordStat(ON)) {
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  WSDDef(ch-'0',Fs);
		else
		  WSDFillin(ch-'0');
		DocLock(doc);
	      }
	    }
	  }
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Enter extended-ASCII");
	  else {
	    if (++doc->ext_ascii_cnt==1)
	      doc->ext_ascii_val=ch-'0';
	    else
	      doc->ext_ascii_val=doc->ext_ascii_val*10+ch-'0';
	    if (doc->ext_ascii_cnt==3)
	      EdInsChar(doc->ext_ascii_val.u8[0],sc,doc);
	    else
	      ext_ascii_cnt=doc->ext_ascii_cnt;
	  }
	}
	break;
      case '[':
	if (!(sc&SCF_ALT) && sc&SCF_CTRL) {
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching brace");
	    else
	      EdFindPaired(doc,'}','{',FALSE);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching bracket");
	    else
	      EdFindPaired(doc,']','[',FALSE);
	  }
	}
	break;
      case ']':
	if (!(sc&SCF_ALT) && sc&SCF_CTRL) {
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching brace");
	    else
	      EdFindPaired(doc,'{','}',TRUE);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching bracket");
	    else
	      EdFindPaired(doc,'[',']',TRUE);
	  }
	}
	break;
      default:
	switch (sc2) {
	  case SC_F1...SC_F10:
	    if (!(sc&SCF_ALT)) {
	      if (sc&SCF_CTRL) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    SetKeyDesc("Edit/Jump to Src of Sym");
		  else
		    SetKeyDesc("Edit/Autocomplete Sym");
		} else {
		  DocUnlock(doc);
		  if (WordStat(ON)) {
		    if (sc&SCF_SHIFT)
		      WSMan(sc2-SC_F1+1,Fs);
		    else
		      WSFillIn(sc2-SC_F1+1);
		  }
		  DocLock(doc);
		}
	      } else {
		switch (sc2) {
		  case SC_F1:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("About");
		      else
			SetKeyDesc("Help");
		    } else {
		      if (sc&SCF_SHIFT)
			Ed("::/Doc/Splash.TXT.Z");
		      else
			Ed("::/Doc/HelpIndex.TXT.Z");
		    }
		    break;
		  case SC_F2:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Edit/Play Macro");
		      else
			SetKeyDesc("Edit/Macro");
		    } else {
		      DocUnlock(doc);
		      if (sc&SCF_SHIFT) {
			if (TaskValidate(sys_macro_util_task))
			  SendMsg(sys_macro_util_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
			SysMacroStripKey(&sys_macro_root,ch,sc);
			PlaySysMacro;
		      } else
			EdMacroUtil;
		      DocLock(doc);
		    }
		    break;
		  case SC_F3:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Edit/Find Last");
		      else
			SetKeyDesc("Edit/Find Next");
		    }else {
		      doc->find_replace->scan_fwd=!(sc&SCF_SHIFT);
		      EdFindNext(doc);
		    }
		    break;
		  case SC_F4:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Misc/Insert Directory Name");
		      else
			SetKeyDesc("Misc/Insert FileName");
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
			SetKeyDesc("Cmd/Adam Include");
		      else
			SetKeyDesc("Cmd/Run (Execute);Dbg/Go");
		    } else {
		      if (st2=DocEntryLink(doc,doc_ce)) {
			st=DocLinkFile(st2);
			Free(st2);
		      } else {
			DocWrite(doc);
			st=StrNew(doc->filename.name);
		      }
		      if (st2=DirFile(st,"Run","CPP.Z")) {
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
		      if (sc & SCF_SHIFT)
			SetKeyDesc("God Doodle");
		      else
			SetKeyDesc("God Song");
		    } else {
		      //$LK,"::/Adam/God/HSNotes.TXT"$
		      if (sc & SCF_SHIFT)
			GodDoodle;
		      else
			GodSong;
		    }
		    break;
		  case SC_F7:
		    if (sc&SCF_KEY_DESC) {
		      if (sc & SCF_SHIFT)
			SetKeyDesc("God Passage");
		      else
			SetKeyDesc("God Word");
		    } else {
		      //$LK,"::/Adam/God/HSNotes.TXT"$
		      FifoU8Flush(god.fifo);
		      GodBitsIns(GOD_GOOD_BITS,KbdMouseEvtTime>>GOD_BAD_BITS);
		      if (sc & SCF_SHIFT)
			GodPassage;
		      else
			GodWord;
		    }
		    break;
		}
	      }
	    }
	    break;
	  case SC_CURSOR_DOWN:
	  case SC_END:
	    if (sc2==SC_END || sc&SCF_CTRL) {
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/GoTo Doc End, Select");
		else
		  SetKeyDesc("Edit/GoTo Doc End");
	      } else {
		while (doc_ce!=doc) {
		  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
		  doc_ce=doc->cur_entry=doc_ce->next;
		}
		doc->cur_data_col=doc_ce->min_col;
		DocFormBwd(doc);
	      }
	    } else if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cursor Down, Select");
	      else
		SetKeyDesc("Edit/Cursor Down");
	    } else
	      EdLineDown(doc,sc);
	    break;
	  case SC_CURSOR_UP:
	  case SC_HOME:
	    if (sc2==SC_HOME || sc&SCF_CTRL) {
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/GoTo Top of Doc, Select");
		else
		  SetKeyDesc("Edit/GoTo Top of Doc");
	      } else {
		if (doc_ce==doc) doc_ce=doc_ce->last;
		while (doc_ce!=doc) {
		  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
		  doc_ce=doc->cur_entry=doc_ce->last;
		}
		doc_ce=doc->cur_entry=doc->root.next;
		doc->cur_data_col=doc_ce->min_col;
		DocFormFwd(doc);
	      }
	    } else if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cursor Up, Select");
	      else
		SetKeyDesc("Edit/Cursor Up");
	    } else
	      EdLineUp(doc,sc);
	    break;
	  case SC_PAGE_DOWN:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Page Down, Select");
	      else
		SetKeyDesc("Edit/Page Down");
	    } else {
	      i=doc_ce->y+doc->win_task->win_height-1;
	      if (doc_ce->type_u8==DOCT_HEX_ED)
		i+=doc->cur_data_col/3/doc_ce->hex_ed_width;
	      while (doc_ce!=doc &&
		    (doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y<i ||
		    doc_ce->type_u8==DOCT_HEX_ED && doc_ce->y+doc->cur_data_col/3/doc_ce->hex_ed_width<i)) {
		EdLineDown(doc,sc);
//paranoid check for stuck on same node
		if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		  break;
		doc_ce=doc->cur_entry;
	      }
	    }
	    break;
	  case SC_PAGE_UP:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Page Up, Select");
	      else
		SetKeyDesc("Edit/Page Up");
	    }else {
	      i=doc_ce->y-(doc->win_task->win_height-1);
	      if (doc_ce->type_u8==DOCT_HEX_ED)
		i+=doc->cur_data_col/3/doc_ce->hex_ed_width;
	      while (doc_ce->last!=doc &&
		    (doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y>i ||
		    doc_ce->type_u8==DOCT_HEX_ED && doc_ce->y+doc->cur_data_col/3/doc_ce->hex_ed_width>i) &&
		    doc_ce->y!=doc->root.next->y) {
		EdLineUp(doc,sc);
//paranoid check for stuck on same node
		if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		  break;
		doc_ce=doc->cur_entry;
	      }
	    }
	    break;
	  case SC_CURSOR_LEFT:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_CTRL)
		SetKeyDesc("Edit/GoTo Start of Line");
	      else {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/Cursor Left, Select");
		else
		  SetKeyDesc("Edit/Cursor Left");
	      }
	    } else
	      EdCursorLeft(doc,sc);
	    break;
	  case SC_CURSOR_RIGHT:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_CTRL)
		SetKeyDesc("Edit/GoTo End of Line");
	      else {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/Cursor Right, Select");
		else
		  SetKeyDesc("Edit/Cursor Right");
	      }
	    } else
	      EdCursorRight(doc,sc);
	    break;
	  case SC_DELETE:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cut To Clipboard");
	      else
		SetKeyDesc("Char/Delete");
	    } else {
	      if (sc&SCF_SHIFT)
		EdCutToClipboard(doc);
	      else
		EdDelChar(doc);
	    }
	    break;
	  case SC_INS:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Paste Clipboard");
	      else if (sc&SCF_CTRL)
		SetKeyDesc("Edit/Copy to Clipboard");
	      else
		SetKeyDesc("Edit/Toggle Overstrike");
	    } else {
	      if (sc&SCF_SHIFT)
		EdPasteClipboard(doc);
	      else if (sc&SCF_CTRL)
		EdCopyToClipboard(doc);
	      else
		doc->flags^=DOCF_OVERSTRIKE;
	    }
	    break;
	}  //switch (sc2)
    }
  }
  doc->ext_ascii_cnt=ext_ascii_cnt;
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

public U0 DocPrintPartial(CDoc *doc,U8 *fmt,...)
{//Lets you print half a doc cmd, if you like.
  U8 *buf,*b,*src,*dst,*ptr,*ptr2;
  Bool unlock;
  CDocEntry *doc_ce,*doc_ne;
  I64 ch,i,j;
  if (!fmt)
    return;
  buf=StrPrintJoin(NULL,fmt,argc,argv);
  ptr=buf;
  if (doc->user_put_s && (*doc->user_put_s)(doc,doc->user_put_data,buf)) {
    Free(buf);
    return;
  }
  unlock=DocLock(doc);
  if (doc->cur_entry->type_u8==DOCT_DATA)
    while (ch=*ptr++)
      DocPutKey(doc,ch,0);
  else
    while (ch=*ptr) {
      if (!Bt(chars_bitmap_safe_dollar,ch) ||
	  doc->flags & (DOCF_OVERSTRIKE | DOCF_IN_DOLLAR)) {
	DocPutKey(doc,ch,0);
	ptr++;
      } else {
	ptr2=ptr++;
	while (TRUE) {
	  ch=*ptr++;
	  if (!Bt(chars_bitmap_safe_dollar,ch))
	    break;
	}
	ptr--;
	*ptr=0;
	doc_ce=doc->cur_entry;
	j=ptr-ptr2;
	if (IsEditableText(doc_ce)) {
	  dst=b=MAlloc(doc_ce->max_col+j+1,doc->mem_task);
	  src=doc_ce->tag;
	  i=doc->cur_data_col;
	  doc->cur_data_col+=j;
	  doc_ce->max_col+=j;
	  while (i-->0)
	    *dst++=*src++;
	  while (j-->0)
	    *dst++=*ptr2++;
	  while (*dst++=*src++);
	  Free(doc_ce->tag);
	  doc_ce->tag=b;
	} else {
	  dst=MAlloc(j+1,doc->mem_task);
	  MemCpy(dst,ptr2,j+1);
	  doc_ne=DocEntryNew(doc,doc_ce,dst);
	  doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
	  doc_ne->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
	  QueIns(doc_ne,doc_ce->last);
	  doc->cur_entry=doc_ne;
	  doc->cur_data_col=StrLen(ptr2);
	}
	*ptr=ch;
	DocRemSoftNewLines(doc,doc->cur_entry);
      }
    }
  if (unlock)
    DocUnlock(doc);
  if (!(doc->flags&DOCF_DONT_SWAP_OUT))
    Yield;
  Free(buf);
}

#help_index "DolDoc"
Bool DocFileEd(I64 _type,U8 *filename,
	U8 *needle_str,I64 *_num,I64 edf_dof_flags)
{
  I64 type=_type,flags=0,old_border_src=Fs->border_src;
  CDocEntry *doc_e;
  CDoc *doc;
  Bool old_silent=Bt(&Fs->display_flags,DISPLAYf_SILENT),
	result=FALSE,other_found=FALSE;
  U8 *st1,*st2;
  try {
    switch (type) {
      case LK_PLAIN:
	flags=DOCF_PLAIN_TEXT;
      case LK_DOC:
	type=LK_FILE;
	break;
      case LK_DOC_ANCHOR:
	type=LK_FILE_ANCHOR;
	break;
      case LK_PLAIN_FIND:
	flags=DOCF_PLAIN_TEXT;
      case LK_DOC_FIND:
	type=LK_FILE_FIND;
	break;
      case LK_PLAIN_LINE:
	flags=DOCF_PLAIN_TEXT;
      case LK_DOC_LINE:
	type=LK_FILE_LINE;
	break;
      case LK_BIBLE_FIND:
	flags=DOCF_PLAIN_TEXT;
	break;
    }

    flags|=DOCF_ALLOW_UNDO;

    if (LK_DOC<=_type<=LK_DOC_LINE) {
      doc=Str2I64(filename);//See $LK,"SpriteEdText",A="MN:SpriteEdText"$()
      result=TRUE;
    } else {

      st1=StrNew(filename);
      st2=StrNew(filename);
      StrLastRem(st1,"/",st2); //st2 is name without dir

      if (!FileNameChk(st2))
	doc=NULL;
      else {
	Silent(ON);
	if (Bt(&edf_dof_flags,EDf_BAIL)) //if bail, scan parents
	  result=FileFind(filename,,
		FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	else if (!(result=FileFind(filename,,
	      FUF_JUST_FILES)))
	  other_found=FileFind(filename,,
		FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	doc=DocRead(filename,flags);
	doc->desc='Edit';
	Silent(old_silent);
	Fs->border_src=BDS_ED_FILENAME_PRT;
      }

      Free(st1);
      Free(st2);
    }
    if (!doc||doc->doc_signature!=DOC_SIGNATURE_VAL)
      result=FALSE;
    else {
      if (Bt(&edf_dof_flags,EDf_COLLAPSE))
	DocCollapse(TRUE,doc);
      else if (Bt(&edf_dof_flags,EDf_UNCOLLAPSE))
	DocCollapse(FALSE,doc);

      if (result || other_found)
	switch (type) {
	  case LK_FILE_LINE:
	    result=DocLineNumGoTo(doc,*_num);
	    break;
	  case LK_FILE_ANCHOR:
	    result=DocAnchorFind(doc,needle_str);
	    break;
	  case LK_FILE_FIND:
	    result=DocFind(doc,,needle_str,*_num);
	    break;
	  case LK_BIBLE_FIND:
	    result=DocFind(doc,*_num,needle_str);
	    break;
	  default:
	    DocCenter(doc);
	}
      *_num=doc->cur_entry->y+1;

      if (!(edf_dof_flags&EDF_BAIL) && DocDo(doc,edf_dof_flags|DOF_DONT_HOME)) {
	DocLock(doc);
	doc_e=doc->cur_entry;
	if (doc_e!=doc)
	  DocEntryRun(doc,doc_e,TRUE);
	DocUnlock(doc);
	if (!(LK_DOC<=_type<=LK_DOC_LINE))
	  DocWrite(doc);
      }
      if (!(LK_DOC<=_type<=LK_DOC_LINE))
	DocDel(doc);
    }
  } catch {
    Silent(old_silent);
    result=FALSE;
  }
  Fs->border_src=old_border_src;
  return result;
}

public I64 EdLinkCvt(U8 *link_st,U8 **_filename=NULL,U8 **_needle_str=NULL,
  I64 *_num=NULL,I64 edf_dof_flags=0)
{//$LK,"Editor Link",A="MN:LK_FILE"$ --> filename, needle_str and line number.
  U8 *st,*ptr,*filename=NULL,*needle_str=NULL,*filename2;
  I64 result,i,num=1;
  CHashSrcSym *temph;
  if (!link_st||!*link_st) {
    if (edf_dof_flags&EDF_BAIL)
      return -1;
    link_st=dsk.temp_filename;
  }
  st=StrNew(link_st);
  result=LK_FILE;
  if (StrLen(st)>3 && st[2]==':') {
    st[2]=0;
    filename2=st+3;
    switch (result=DefineMatch(st,"ST_LINK_TYPES",LMF_IGNORE_CASE)) {
      case LK_MAN_PAGE:
	if (temph=HashFind(filename2,Fs->hash_table,HTG_SRC_SYM))
	  result=EdLinkCvt(temph->src_link,&filename,&needle_str,&num,edf_dof_flags);
	else
	  result=-1;
	goto lc_done;
      case LK_ADDR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=Str2I64(ptr+1);
	} else
	  i=DFT_ADDR_LINK_BIN_SIZE;
	if (ptr=SrcEdLink(ExePrint("%s;",filename2),i)) {
	  result=EdLinkCvt(ptr,&filename,&needle_str,&num,edf_dof_flags);
	  Free(ptr);
	} else
	  result=-1;
	goto lc_done;
      case LK_DEF:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=Str2I64(ptr+1);
	} else
	  i=-1;
	filename=StrNew(filename2);
	num=i;
	goto lc_done;
      case LK_HELP_INDEX:
	filename=StrNew(filename2);
	goto lc_done;
      case LK_BIBLE_FIND:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  needle_str=StrNew(ptr+1);
	}
	i=DefineMatch(filename2,"ST_BIBLE_BOOKS",LMF_IGNORE_CASE);
	if (i<0)
	  result=-1;
	else {
	  num=Str2I64(DefineSub(i,"ST_BIBLE_BOOK_LINES"));
	  filename2=Define("ST_BIBLE_FILE");
	}
	break;
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_DOC_LINE:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  num=Str2I64(ptr+1);
	}
	break;
      case LK_FILE_ANCHOR:
      case LK_DOC_ANCHOR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  needle_str=StrNew(ptr+1);
	}
	break;
      case LK_FILE_FIND:
      case LK_PLAIN_FIND:
      case LK_DOC_FIND:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  needle_str=StrNew(ptr+1);
	  if (ptr=StrLastOcc(needle_str,":")) {
	    *ptr=0;
	    num=Str2I64(ptr+1);
	  }
	}
	break;
    }
  } else
    filename2=st;
  if (result>=0) {
    if (LK_DOC<=result<=LK_DOC_LINE)
      filename=StrNew(filename2); //Holds document address as number.
    else
      filename=FileNameAbs(filename2,FUF_Z_OR_NOT_Z);
  }
lc_done:
  Free(st);
  if (_filename)
    *_filename=filename;
  else
    Free(filename);
  if (_needle_str)
    *_needle_str=needle_str;
  else
    Free(needle_str);
  if (_num)
    *_num=num;
  return result;
}
