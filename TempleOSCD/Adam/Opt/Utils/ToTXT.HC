#help_index "DolDoc/Conversion"

U0 TXTPutS(CDoc *doc,U8 *st,I64 *_col)
{
  U8 *ch,*ptr;
  while (ch=*st++) {
    switch (ch) {
      case '\t':
	do {
	  DocPutKey(doc,CH_SPACE,0);
	  *_col=*_col+1;
	} while (*_col&7);
	break;

      start:
	case 'ã':	ptr="pi";	break;
	case 'é':	ptr="theta";	break;
	case 'è':	ptr="phi";	break;
	case 'ê':	ptr="omega";	break;
	case 'ì':	ptr="inf";	break;
	case 'æ':	ptr="u";	break;
	case CH_SHIFT_SPACE: ptr=" ";	break;
      end:
	DocPrint(doc,ptr);
	*_col=*_col+StrLen(ptr);
	break;

      default:
	if (CH_SPACE<=ch<0x7F || ch=='\n')
	  DocPutKey(doc,ch,0);
	else
	  DocPrint(doc,".");
	*_col=*_col+1;
    }
  }
}

public CDoc *Doc2TXT(CDoc *doc_in)
{//Cvt $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ doc to plain text.
  CDocEntry *doc_e,*doc_e2;
  I64 y,col;
  CDoc *doc_out=DocNew;
  Bool unlock_doc_in=DocLock(doc_in),no_bwd,line_has_html_code=FALSE;
  DocRecalc(doc_in);

  doc_out->flags|=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;

  doc_e=doc_in->head.next;
  col=doc_e->x;
  y=doc_e->y;
  while (doc_e!=doc_in) {
    if (!(doc_e->de_flags&DOCEF_SKIP)) {
      while (y<doc_e->y) {
	TXTPutS(doc_out,"\n",&col);
	y++;
	col=0;
	line_has_html_code=FALSE;
      }
      if (doc_e->type_u8==DOCT_HTML_CODE)
	line_has_html_code=TRUE;

      no_bwd=TRUE;
      doc_e2=doc_e->next;
      while (doc_e2!=doc_in && doc_e2->y==doc_e->y) {
	if (doc_e2->x<doc_e->x) {
	  no_bwd=FALSE;
	  break;
	}
	doc_e2=doc_e2->next;
      }
      if (no_bwd && !(doc_e->type_u8==DOCT_NEW_LINE && line_has_html_code))
	while (col<doc_e->x)
	  TXTPutS(doc_out," ",&col);

      switch (doc_e->type_u8) {
	case DOCT_TEXT:
	  TXTPutS(doc_out,doc_e->tag,&col);
	  break;
	case DOCT_TAB:
	  TXTPutS(doc_out,"\t",&col);
	  break;
	case DOCT_HTML_CODE:
	  break;
	case DOCT_SPRITE:
	  break;
	default:
	  if (doc_e->de_flags&DOCEF_TAG)
	    TXTPutS(doc_out,doc_e->tag,&col);
      }
    }
    doc_e=doc_e->next;
  }
  while (y<doc_e->y) {
    TXTPutS(doc_out,"\n",&col);
    y++;
    col=0;
  }
  doc_out->cur_entry=&doc_out->head;
  DocRecalc(doc_out);

  if (unlock_doc_in)
    DocUnlock(doc_in);
  return doc_out;
}

#help_index "Cmd Line (Typically);DolDoc/Conversion;DolDoc/Cmd Line (Typically)"
public U0 ToTXT(U8 *_in_name,U8 *_out_name=NULL,I64 width=70)
{//Convert $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$file to plain text.
  U8 *in_name,*out_name;
  CDoc *doc_in,*doc_out;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinHorz(0,width-1); //Sets doc width for word wrap.

  in_name=ExtDft(_in_name,"DD.Z");
  if (_out_name)
    out_name=ExtDft(_out_name,"TXT");
  else
    out_name=ExtChg(_in_name,"TXT");

  doc_in=DocRead(in_name);
  doc_out=Doc2TXT(doc_in);
  StrCpy(&doc_out->filename.name,out_name);

  SettingsPop;

  DocWrite(doc_out);
  DocDel(doc_in);
  DocDel(doc_out);
  Free(in_name);
  Free(out_name);
}
