#help_index "Utils;Cmd Line (Typically);Doc/Html"

//See $LK-A,"::/TempleOS/Doc/Acknowledgements.TXT"$ (12).

U0 HtmlPutS(CDoc *doc,I64 c,I64 *_old_c,U8 *st,I64 *_col,U8 *style_bitmap=NULL)
{
  U8 *ch;
  if (c!=*_old_c) {
    if (*_old_c!=-1)
      DocPutS(doc,"</span>");
    DocPrintF(doc,"<span class=c%02X>",c);
    *_old_c=c;
    if (style_bitmap)
      LBts(style_bitmap,c);
  }
  while (ch=*st++) {
    switch (ch) {
      case '\t':
	do {
	  DocPutKey(doc,CH_SPACE,0);
	  *_col=*_col+1;
	} while (*_col&7);
	break;
      case '&':
	DocPutS(doc,"&amp;");
	*_col=*_col+1;
	break;
      case '<':
	DocPutS(doc,"&lt;");
	*_col=*_col+1;
	break;
      case '>':
	DocPutS(doc,"&gt;");
	*_col=*_col+1;
	break;
      case '"':
	DocPutS(doc,"&quot;");
	*_col=*_col+1;
	break;
      case 'ã':
	DocPutS(doc,"pi");
	*_col=*_col+1;
	break;
      case 'é':
	DocPutS(doc,"theta");
	*_col=*_col+1;
	break;
      case 'è':
	DocPutS(doc,"phi");
	*_col=*_col+1;
	break;
      case 'ê':
	DocPutS(doc,"omega");
	*_col=*_col+1;
	break;
      case 'ì':
	DocPutS(doc,"inf");
	*_col=*_col+1;
	break;
      case 'æ':
	DocPutS(doc,"u");
	*_col=*_col+1;
	break;
      case CH_SHIFT_SPACE:
	DocPutS(doc," ");
	*_col=*_col+1;
	break;
      default:
	if (CH_SPACE<=ch<0x7F || ch=='\n')
	  DocPutKey(doc,ch,0);
	else
	  DocPutS(doc,".");
	*_col=*_col+1;
    }
  }
}

U8 *TSLinkCvt2(U8 *filename,I64 line_num)
{ // ::/TempleOS/  --> http://www.templeos.org/Wb/
  // Make your own LinkCvt routine
  U8 *result=NULL,*st;
  if (filename) {
    st=ChgExt(filename,"html");
    if (st && StrLen(st)>12 && !StrNCmp(st+1,":/TempleOS/",11))
      result=MSPrintF("http://www.templeos.org/Wb/%s#l%d",st+12,line_num);
    Free(st);
  }
  return result;
}

U8 *TSLinkCvt1(U8 *link_st)
{
  static CDoc *bible=NULL;
  static I64 locks=0;
  U8 *result=NULL,*filename,*pattern;
  I64 i,num;
  if (link_st) {
    switch (i=EdLinkCvt(link_st,&filename,&pattern,&num)) {
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_FILE:
	result=TSLinkCvt2(filename,num);
	break;
      case -1:
      case LK_DEF:
      case LK_HELP_INDEX:
	break;
      case LK_BIBLE_FIND:
	while (LBts(&locks,0))
	  Yield;
	if (!bible)
	  bible=Adam("DocRead(\"%s\");",filename);
	if (DocPatternFind(bible,num,pattern))
	  result=TSLinkCvt2(filename,bible->cur_entry->y+1);
	LBtr(&locks,0);
	break;
      default:
	if (DocFileEd(i,filename,pattern,&num,EDF_UNCOLLAPSE|EDF_BAIL))
	  result=TSLinkCvt2(filename,num);
    }
    Free(filename);
    Free(pattern);
  }
  return result;
}

public CDoc *Doc2Html(CDoc *doc_in,
  U8 *html_header=NULL,
  U8 *body_header=NULL,U8 *body_footer=NULL,Bool line_anchors=TRUE,
  U8 (*link_cvt)(U8 *link_st)=&TSLinkCvt1)
{
  CDocEntry *doc_e,*style,*doc_e2;
  CDocSettings *s;
  I64 i,y,old_y=MIN_I64,cur_u8_attr,col,old_c=-1;
  U32 *hl,*src;
  U8 *st,st_2[2],*link_st,*style_bitmap=CAlloc(256/8);
  CBGR p[16];
  GetVGAPalette4(p);
  CDoc *doc_out=DocNew;
  Bool unlock_doc_in=DocLock(doc_in),no_backward;

  for (i=0xF0;i<=0xFF;i++)
    LBts(style_bitmap,i);

  DocRecalc(doc_in);

  st_2[0]=0; st_2[1]=0;
  doc_out->flags|=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;
  if (!html_header) html_header="";
  if (!body_header) body_header="";
  if (!body_footer) body_footer="";
  DocPrintF(doc_out,
    "<!DOCTYPE HTML>\n"
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=US-ASCII\">\n"
    "%s"
    "<meta name=\"generator\" content=\"TempleOS V%4.2f\">\n"
    "<style type=\"text/css\">\n",html_header,os_version);
  style=doc_out->cur_entry->last;
  DocPutS(doc_out,
    "</style>\n"
    "</head>\n"
    "<body>\n"
    "<pre style=\"font-family:courier;font-size:10pt\">\n");
  DocPutS(doc_out,body_header);

  doc_e=doc_in->root.next;
  s=DocSettingsLast(doc_e);
  col=doc_e->x;
  y=doc_e->y;
  cur_u8_attr=doc_e->type.u8[1];
  while (doc_e!=doc_in) {
    if (!(doc_e->flags1&(DOCEF1_SKIP|DOCEF1_HARD_SKIP))) {
      if (line_anchors && y!=old_y) {
	DocPrintF(doc_out,"<a name=\"l%d\"></a>",y+1);
	old_y=y;
      }
      while (y<doc_e->y) {
	HtmlPutS(doc_out,cur_u8_attr,&old_c,"\n",&col,style_bitmap);
	y++;
	if (line_anchors && y!=old_y) {
	  DocPrintF(doc_out,"<a name=\"l%d\"></a>",y+1);
	  old_y=y;
	}
	col=0;
      }

      no_backward=TRUE;
      doc_e2=doc_e->next;
      while (doc_e2!=doc_in && doc_e2->y==doc_e->y) {
	if (doc_e2->x<doc_e->x) {
	  no_backward=FALSE;
	  break;
	}
	doc_e2=doc_e2->next;
      }
      if (no_backward)
	while (col<doc_e->x)
	  HtmlPutS(doc_out,cur_u8_attr,&old_c," ",&col,style_bitmap);

      link_st=NULL;
      if (doc_e->flags1&DOCEF1_HTML_LINK)
	link_st=StrNew(doc_e->html_link);
      else if (st=DocEntryLink(doc_in,doc_e)) {
	link_st=link_cvt(st);
	Free(st);
      }

      if (link_st) {
	if (old_c!=-1) {
	  DocPutS(doc_out,"</span>");
	  old_c=-1;
	}
	DocPrintF(doc_out,"<a href=\"%s\">",link_st);
      }

      if (doc_e->settings)
	s=doc_e->settings;
      cur_u8_attr=doc_e->type.u8[1];
      switch (doc_e->type_u8) {
	case DOCT_TEXT:
	  if (doc_e->flags1&DOCEF1_HIGHLIGHT) {
	    src=hl=DocHighlight(doc_in,FALSE,doc_e,&s,doc_e->tag,StrLen(doc_e->tag),cur_u8_attr<<8);
	    while (*src) {
	      st_2[0]=*src&0xFF;
	      HtmlPutS(doc_out,*src>>8,&old_c,st_2,&col,style_bitmap);
	      src++;
	    }
	    Free(hl);
	  } else
	    HtmlPutS(doc_out,cur_u8_attr,&old_c,doc_e->tag,&col,style_bitmap);
	  break;
	case DOCT_TAB:
	  HtmlPutS(doc_out,cur_u8_attr,&old_c,"\t",&col,style_bitmap);
	  break;
	case DOCT_HTML_IMG:
	  if (old_c!=-1) {
	    DocPutS(doc_out,"</span>");
	    old_c=-1;
	  }
	  DocPrintF(doc_out,"<img src=\"%s\" width=\"640\" height=\"480\" alt=\"\">\n",doc_e->tag);
	  break;
	case DOCT_HTML_VIDEO:
	  if (old_c!=-1) {
	    DocPutS(doc_out,"</span>");
	    old_c=-1;
	  }
	  DocPrintF(doc_out,
		"<!--begin exclude-->\n"
		"<object classid=\"clsid:22D6F312-B0F6-11D0-94AB-0080C74C7E95\" "
		"codebase=\"http://activex.microsoft.com/activex/controls/mplayer/en/nsmp2inf.cab\" "
		"width=\"640\" height=\"512\">\n"
		"<param name=\"FileName\" value=\"%s\">\n"
		"<param name=\"AutoStart\" value=\"false\">\n"
		"<param name=\"ShowTracker\" value=\"true\">\n"
		"<param name=\"ShowControls\" value=\"true\">\n"
		"<param name=\"ShowGotoBar\" value=\"false\">\n"
		"<param name=\"ShowDisplay\" value=\"false\">\n"
		"<param name=\"ShowStatusBar\" value=\"false\">\n"
		"<param name=\"AutoSize\" value=\"false\">\n"
		"<embed src=\"%s\" AutoStart=\"false\" ShowTracker=\"true\" ShowControls=\"true\" "
		"ShowGotoBar=\"false\" ShowDisplay=\"false\" ShowStatusBar=\"false\" "
		"AutoSize=\"false\" pluginspage=\"http://www.microsoft.com/windows/windowsmedia/download/\" "
		"width=\"640\" height=\"512\"></embed></object>\n"
		"<!--end exclude-->\n",doc_e->tag,doc_e->tag);
	  break;
	case DOCT_HTML_CODE:
	  if (old_c!=-1) {
	    DocPutS(doc_out,"</span>");
	    old_c=-1;
	  }
	  DocPutS(doc_out,doc_e->tag);
	  break;
	case DOCT_SPRITE:
	  HtmlPutS(doc_out,cur_u8_attr,&old_c,doc_e->tag,&col,style_bitmap);
	  HtmlPutS(doc_out,s->comment_attr&15+old_c&0xF0,&old_c,"/* Graphics Not Rendered in HTML */",&col,style_bitmap);
	  break;
	default:
	  if (Bt(doc_tag_types,doc_e->type_u8))
	    HtmlPutS(doc_out,cur_u8_attr,&old_c,doc_e->tag,&col,style_bitmap);
      }
      if (link_st) {
	if (old_c!=-1) {
	  DocPutS(doc_out,"</span>");
	  old_c=-1;
	}
	DocPutS(doc_out,"</a>");
	Free(link_st);
      }
    }
    doc_e=doc_e->next;
  }
  while (y<doc_e->y) {
    HtmlPutS(doc_out,cur_u8_attr,&old_c,"\n",&col,style_bitmap);
    y++;
    col=0;
  }
  if (old_c!=-1) {
    DocPutS(doc_out,"</span>");
    old_c=-1;
  }
  DocPutS(doc_out,body_footer);
  DocPutS(doc_out,"</pre></body>\n</html>\n");

  doc_out->cur_entry=style->next;
  for (i=0;i<256;i++)
    if (Bt(style_bitmap,i))
      DocPrintF(doc_out,".c%02X{color:#%02x%02x%02x;background-color:#%02x%02x%02x;}\n",i,p[i&15].r,p[i&15].g,p[i&15].b,p[i/16].r,p[i/16].g,p[i/16].b);
  doc_out->cur_entry=&doc_out->root;
  DocRecalc(doc_out);

  if (unlock_doc_in)
    DocUnlock(doc_in);
  return doc_out;
}

public U0 ToHtml(U8 *_in_name,U8 *_out_name=NULL,U8 *html_header=NULL,
  U8 *body_header=NULL,U8 *body_footer=NULL,I64 width=80,
  Bool line_anchors=TRUE,U8 (*link_cvt)(U8 *link_st)=&TSLinkCvt1)
//Supply your own link_cvt routine.
{
  U8 *in_name,*out_name;
  CDoc *doc_in,*doc_out;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  SetWinHorz(0,width-1); //This sets doc width for word wrap

  in_name=DftExt(_in_name,"CPP.Z"),
  if (_out_name)
    out_name=DftExt(_out_name,"html");
  else
    out_name=ChgExt(_in_name,"html");

  doc_in=DocRead(in_name);
  doc_out=Doc2Html(doc_in,html_header,body_header,body_footer,line_anchors,link_cvt);
  StrCpy(&doc_out->filename.name,out_name);

  SettingsPop;

  DocWrite(doc_out);
  DocDel(doc_in);
  DocDel(doc_out);
  Free(in_name);
  Free(out_name);
}

#help_index ""
