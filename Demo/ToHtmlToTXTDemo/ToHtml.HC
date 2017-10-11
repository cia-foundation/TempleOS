#help_index "DolDoc/Conversion"

//See $LK,"::/Doc/Credits.DD"$.

#define DOCET_UNDEF_COLOR	0x100000000
#define DOCET_LINK_UNDERLINE	0x200000000
U0 HtmlPutS(CDoc *doc,I64 u32_attr,I64 *_old_u32_attr,Bool underline_update,
	U8 *st,I64 *_col,U8 *style_bitmap=NULL)
{
  U8 *ch,*ptr;
  u32_attr&=0xFFFFFF00;
  if (u32_attr&DOCET_INVERT)
    u32_attr.u8[1]=(u32_attr.u8[1]&15)<<4|u32_attr.u8[1]>>4;
  if (underline_update &&
	!(u32_attr&DOCET_UNDERLINE) && *_old_u32_attr&DOCET_UNDERLINE) {
    if (!(*_old_u32_attr&DOCET_LINK_UNDERLINE))
      DocPrint(doc,"</u>");
    *_old_u32_attr&=~DOCET_LINK_UNDERLINE;
  }
  if (!(u32_attr&DOCET_BLINK) && *_old_u32_attr&DOCET_BLINK)
    DocPrint(doc,"</blink>");
  if (u32_attr&0xFF00!=*_old_u32_attr&0xFF00) {
    if (!(*_old_u32_attr&DOCET_UNDEF_COLOR))
      DocPrint(doc,"</span>");
    DocPrint(doc,"<span class=c%02X>",u32_attr.u8[1]);
    if (style_bitmap)
      LBts(style_bitmap,u32_attr.u8[1]);
  }
  if (u32_attr&DOCET_BLINK && !(*_old_u32_attr&DOCET_BLINK))
    DocPrint(doc,"<blink>");
  if (underline_update) {
    if (u32_attr&DOCET_UNDERLINE && !(*_old_u32_attr&DOCET_UNDERLINE))
      DocPrint(doc,"<u>");
  } else //Keep same underline status.
    u32_attr=u32_attr&~DOCET_UNDERLINE | *_old_u32_attr&DOCET_UNDERLINE;

  *_old_u32_attr=u32_attr | *_old_u32_attr&DOCET_LINK_UNDERLINE;

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
      end:
	DocPrint(doc,ptr);
	*_col=*_col+StrLen(ptr);
	break;

      start:
	case CH_SHIFT_SPACE: ptr=" ";	break;
	case '&':	ptr="&amp;";	break;
	case '<':	ptr="&lt;";	break;
	case '>':	ptr="&gt;";	break;
	case '"':	ptr="&quot;";	break;
      end:
	DocPrint(doc,ptr);
	*_col=*_col+1;
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

U8 *TOSLinkCvt2(U8 *filename,I64 line_num)
{// ::/  --> http://www.templeos.org/Wb/
//Make your own LinkCvt routine
  U8 *res=NULL,*st;
  if (filename) {
    st=ExtChg(filename,"html");
    if (st && StrLen(st)>3 && !StrNCmp(st+1,":/",2))
      res=MStrPrint("http://www.templeos.org/Wb/%s#l%d",st+3,line_num);
    Free(st);
  }
  return res;
}

U8 *URLBibleGateway(U8 *src)
{
  U8 buf[STR_LEN],*dst=buf;
  if (!MemCmp(src,"BF:",3))
    src+=3;
  while (*src) {
    if (*src==CH_SPACE || *src==',')
      *dst++='+';
    else
      *dst++=*src;
    src++;
  }
  *dst=0;
  if (StrOcc(buf,'-'))
    return MStrPrint(
	  "http://www.biblegateway.com/passage/?search=%s&version=NIV",buf);
  else
    return MStrPrint(
	  "http://www.biblegateway.com/verse/en/%s",buf);
}

U8 *TOSLinkCvt1(U8 *link_st)
{
  static CDoc *bible=NULL;
  static I64 locks=0;
  U8 *res=NULL,*filename,*needle;
  I64 i,num;
  if (link_st) {
    switch (i=EdLinkCvt(link_st,&filename,&needle,&num)) {
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_FILE:
	res=TOSLinkCvt2(filename,num);
	break;
      case -1:
      case LK_DEF:
      case LK_HELP_INDEX:
      case LK_DOC:
      case LK_DOC_ANCHOR:
      case LK_DOC_FIND:
      case LK_DOC_LINE:
	break;
      case LK_BIBLE_FIND:
	while (LBts(&locks,0))
	  Yield;
	if (!bible)
	  bible=Adam("DocRead(\"%s\");",filename);
	if (DocFind(bible,num,needle))
	  res=URLBibleGateway(link_st);
	LBtr(&locks,0);
	break;
      default:
	if (DocFileEd(i,filename,needle,&num,EDF_UNCOLLAPSE|EDF_BAIL))
	  res=TOSLinkCvt2(filename,num);
    }
    Free(filename);
    Free(needle);
  }
  return res;
}

public CDoc *Doc2Html(CDoc *doc_in,U8 *html_header=NULL,U8 *body_header=NULL,
	U8 *body_footer=NULL,U8 *html_footer=NULL,Bool line_anchors=TRUE,
	U8 (*link_cvt)(U8 *link_st)=&TOSLinkCvt1,Bool line_nums=FALSE)
{//Cvt $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$doc to HTML file.
  CDocEntry *doc_e,*style,*doc_e2;
  I64 i,y,old_y=I64_MIN,col,old_u32_attr=DOCET_UNDEF_COLOR,old_attr,digits;
  U32 *hl,*src;
  U8 *st,st_2[2],*link_st,*style_bitmap=CAlloc(256/8);
  CBGR48 p[COLORS_NUM];
  GrPaletteGet(p);
  CDoc *doc_out=DocNew;
  Bool unlock_doc_in=DocLock(doc_in),no_bwd;

  old_attr=doc_in->win_task->text_attr;
  doc_in->win_task->text_attr=DOC_ATTR_DFT_TEXT;

  for (i=0xF0;i<=0xFF;i++)
    LBts(style_bitmap,i);

  DocRecalc(doc_in,RECALCt_NORMAL|RECALCF_TO_HTML);

  digits=Log10(doc_in->head.last->y+1)+1;

  st_2[0]=0; st_2[1]=0;
  doc_out->flags|=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;

  if (!html_header) html_header=
	  "<!DOCTYPE HTML>\n"
	  "<html>\n"
	  "<head>\n"
	  "<meta http-equiv=\"Content-Type\" "
	  "content=\"text/html;charset=US-ASCII\">\n"
	  "<meta name=\"generator\" content=\"$TX,"TempleOS V5.03",D="DD_OS_NAME_VERSION"$\">\n";
  if (!body_header) body_header=
	  "<body>\n"
	  "<pre style=\"font-family:courier;font-size:10pt\">\n";
  if (!body_footer) body_footer=
	  "</pre></body>\n";
  if (!html_footer) html_footer=
	  "</html>\n";

  DocPrint(doc_out,"%s",html_header);

  DocPrint(doc_out,"<style type=\"text/css\">\n");
  style=doc_out->cur_entry->last;
  DocPrint(doc_out,
	"</style>\n"
	"</head>\n");
  DocPrint(doc_out,"%s",body_header);

  doc_e=doc_in->head.next;
  col=doc_e->x;
  y=doc_e->y;
  while (doc_e!=doc_in) {
    if (!(doc_e->de_flags&DOCEF_SKIP)) {
      if (y!=old_y) {
	if (line_anchors)
	  DocPrint(doc_out,"<a name=\"l%d\"></a>",y+1);
	if (line_nums)
	  DocPrint(doc_out,"%0*d ",digits,y+1);
	old_y=y;
      }
      while (y<doc_e->y) {
	HtmlPutS(doc_out,doc_e->settings.final_u32_attr,&old_u32_attr,FALSE,
	      "\n",&col,style_bitmap);
	if (++y!=old_y) {
	  if (line_anchors)
	    DocPrint(doc_out,"<a name=\"l%d\"></a>",y+1);
	  if (line_nums)
	    DocPrint(doc_out,"%0*d ",digits,y+1);
	  old_y=y;
	}
	col=0;
      }

      no_bwd=TRUE;
      doc_e2=doc_e->next;
      while (doc_e2!=doc_in && doc_e2->y==doc_e->y) {
	if (doc_e2->x<doc_e->x) {
	  no_bwd=FALSE;
	  break;
	}
	doc_e2=doc_e2->next;
      }

      if (no_bwd)
	while (col<doc_e->x)
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr&~DOCET_UNDERLINE,
		&old_u32_attr,TRUE," ",&col,style_bitmap);

      link_st=NULL;
      if (doc_e->de_flags&DOCEF_HTML_LINK)
	link_st=StrNew(doc_e->html_link);
      else if (st=DocEntryLink(doc_in,doc_e)) {
	link_st=link_cvt(st);
	Free(st);
      }

      if (link_st) {
	if (old_u32_attr&DOCET_UNDERLINE &&!(old_u32_attr&DOCET_LINK_UNDERLINE))
	  DocPrint(doc_out,"</u>");
	if (old_u32_attr&DOCET_BLINK)
	  DocPrint(doc_out,"</blink>");
	if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	  DocPrint(doc_out,"</span>");
	old_u32_attr=DOCET_UNDEF_COLOR|DOCET_UNDERLINE|DOCET_LINK_UNDERLINE;
	DocPrint(doc_out,"<a href=\"%s\">",link_st);
      }

      switch (doc_e->type_u8) {
	case DOCT_TEXT:
	  if (doc_e->de_flags&DOCEF_HIGHLIGHT) {
	    if (doc_e->last==doc_in)
	      MemCpy(&doc_e->settings,&doc_in->settings_head,
		    sizeof(CDocSettings));
	    else
	      MemCpy(&doc_e->settings,&doc_e->last->settings,
		    sizeof(CDocSettings));
	    src=hl=DocHighlight(doc_e,doc_e->tag,StrLen(doc_e->tag),
		  doc_e->type&0xFF00);
	    while (*src) {
	      st_2[0]=*src&0xFF;
	      HtmlPutS(doc_out,*src++,&old_u32_attr,TRUE,
		    st_2,&col,style_bitmap);
	    }
	    Free(hl);
	  } else
	    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		  &old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
	  break;
	case DOCT_TAB:
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr,&old_u32_attr,TRUE,
		"\t",&col,style_bitmap);
	  break;
	case DOCT_HTML_CODE:
	  if (old_u32_attr&DOCET_UNDERLINE &&
		!(old_u32_attr&DOCET_LINK_UNDERLINE))
	    DocPrint(doc_out,"</u>");
	  if (old_u32_attr&DOCET_BLINK)
	    DocPrint(doc_out,"</blink>");
	  if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	    DocPrint(doc_out,"</span>");
	  old_u32_attr=DOCET_UNDEF_COLOR;
	  DocPrint(doc_out,"%s",doc_e->tag);
	  break;
	case DOCT_SPRITE:
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		&old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
	  HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		&old_u32_attr,TRUE,"/* Graphics Not Rendered in HTML */",
		&col,style_bitmap);
	  break;
	default:
	  if (doc_e->de_flags&DOCEF_TAG)
	    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
		  &old_u32_attr,TRUE,doc_e->tag,&col,style_bitmap);
      }
      if (link_st) {
	if (old_u32_attr&DOCET_UNDERLINE &&!(old_u32_attr&DOCET_LINK_UNDERLINE))
	  DocPrint(doc_out,"</u>");
	if (old_u32_attr&DOCET_BLINK)
	  DocPrint(doc_out,"</blink>");
	if (!(old_u32_attr&DOCET_UNDEF_COLOR))
	  DocPrint(doc_out,"</span>");
	DocPrint(doc_out,"</a>");
	old_u32_attr=DOCET_UNDEF_COLOR|DOCET_UNDERLINE|DOCET_LINK_UNDERLINE;
	Free(link_st);
      }
    }
    doc_e=doc_e->next;
  }
  while (y++<doc_e->y) {
    HtmlPutS(doc_out,doc_e->settings.final_u32_attr,
	  &old_u32_attr,TRUE,"\n",&col,style_bitmap);
    col=0;
  }
  if (old_u32_attr&DOCET_UNDERLINE && !(old_u32_attr&DOCET_LINK_UNDERLINE))
    DocPrint(doc_out,"</u>");
  if (old_u32_attr&DOCET_BLINK)
    DocPrint(doc_out,"</blink>");
  if (!(old_u32_attr&DOCET_UNDEF_COLOR))
    DocPrint(doc_out,"</span>");
  old_u32_attr=DOCET_UNDEF_COLOR;
  DocPrint(doc_out,"%s",body_footer);
  DocPrint(doc_out,"%s",html_footer);

  doc_out->cur_entry=style->next;
  for (i=0;i<256;i++)
    if (Bt(style_bitmap,i))
      DocPrint(doc_out,
	    ".c%02X{color:#%02x%02x%02x;background-color:#%02x%02x%02x;}\n",
	    i,p[i&15].r>>8,p[i&15].g>>8,p[i&15].b>>8,
	    p[i/16].r>>8,p[i/16].g>>8,p[i/16].b>>8);
  doc_out->cur_entry=&doc_out->head;
  DocRecalc(doc_out);

  doc_in->win_task->text_attr=old_attr;

  if (unlock_doc_in)
    DocUnlock(doc_in);
  return doc_out;
}

#help_index "Cmd Line (Typically);DolDoc/Conversion;DolDoc/Cmd Line (Typically)"
public U0 ToHtml(U8 *_in_name,U8 *_out_name=NULL,U8 *html_header=NULL,
	U8 *body_header=NULL,U8 *body_footer=NULL,U8 *html_footer=NULL,
	I64 width=80,Bool line_anchors=TRUE,
	U8 (*link_cvt)(U8 *link_st)=&TOSLinkCvt1,Bool line_nums=FALSE)
{//Convert $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$file to HTML.
//Supply your own link_cvt routine.
  U8 *in_name,*out_name;
  CDoc *doc_in,*doc_out;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinHorz(0,width-1); //Sets doc width for word wrap.

  in_name=ExtDft(_in_name,"HC.Z");
  if (_out_name)
    out_name=ExtDft(_out_name,"html");
  else
    out_name=ExtChg(_in_name,"html");

  doc_in=DocRead(in_name);
  doc_out=Doc2Html(doc_in,html_header,body_header,body_footer,html_footer,
	line_anchors,link_cvt,line_nums);
  StrCpy(&doc_out->filename.name,out_name);

  SettingsPop;

  DocWrite(doc_out);
  DocDel(doc_in);
  DocDel(doc_out);
  Free(in_name);
  Free(out_name);
}
