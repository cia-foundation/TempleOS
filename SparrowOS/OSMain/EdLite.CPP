class CLine
{
  CLine *next,*last;
  U8 *line;
};

U0 EdLiteUpdate(CLine *root,CLine *cur_line,I64 cur_col,I64 line_start_col)
{
  I64 ch,i,j,k,k2,cursor_col,cursor_row=-1;
  U8 *st;
  CLine *templ=cur_line;
  Bool done_eof=FALSE;
  sys_raw_dc_col=0;
  for (i=0;i<text_rows/2;i++)
    if (templ->last!=root)
      templ=templ->last;
  for (i=0;i<text_rows;i++) {
    if (cursor_row<0 && templ==cur_line) {
      k=0;
      for (j=0;j<cur_col;j++)
	if (templ->line[j]==CH_TAB)
	  k=(k+8)&~7;
	else
	  k++;
      cursor_col=k;
      cursor_row=i;
    }
    if (templ!=root) {
      st=templ->line;
      k=0;
      j=0;
      while (ch=*st++) {
	if (ch==CH_TAB)
	  k2=(k+8)&~7;
	else
	  k2=k+1;
	if (line_start_col<=k<line_start_col+text_cols) {
	  '' ch;
	  j=k2-line_start_col;
	}
	k=k2;
      }
      if (j<text_cols)
	NewLine;
      templ=templ->next;
    } else {
      if (!done_eof) {
	"<EOF>";
	done_eof=TRUE;
      }
      NewLine;
    }
  }
  sys_raw_dc_col=text_cols*cursor_row+cursor_col-line_start_col;
  RawDrawChar(0x7F);
}


U0 EdLite(U8 *filename)
{
//Light weight text editor for debugging
  U8 *src,*src2,*src3,*dst,*buf,*bin_data=NULL;
  I64 i,ch,sc,size,bin_size=0,line_start_col=0,cur_col=0,
      old_dc_flags=sys_raw_dc_flags;
  CLine root,*templ,*templ1,*cur_line;
  Bool old_raw=Raw(ON),
	old_debug=InDbg(ON),
	old_single=SingleUser(ON);

  if (!filename) filename=sys_temp_filename;
  buf=FileRead(filename,&size);

  PushFD;
  Cli;
  sys_raw_dc_flags=sys_raw_dc_flags&~RDCF_SCROLL|RDCF_SHOW_DOLLAR;
  sys_cur_scan_code=0;
  QueInit(&root);
  root.line=StrNew("");

  if (buf) {
    src=buf;
    while (*src) {
      src2=src;
      while ((ch=*src++) && ch!=CH_CARRIAGE_RETURN && ch!=CH_NEW_LINE);
      src--;
      *src++=0;
      if (!ch)
	src--;
      while (ch==CH_CARRIAGE_RETURN && *src==CH_NEW_LINE || *src==CH_CURSOR)
	src++;
      dst=src3=src2;
      while (ch=*src3++)
	if (ch!=CH_NEW_LINE && ch!=CH_CURSOR)
	  *dst++=ch;
      *dst=0;

      templ=MAlloc(sizeof(CLine));
      templ->line=StrNew(src2);
      QueIns(templ,root.last);
    }

    if (src+1-buf<size) {
      bin_data=MAlloc(bin_size=size-(src-buf));
      MemCpy(bin_data,src,bin_size);
    }
    Free(buf);
  }

  cur_line=root.next;
  do {
    if (cur_line==&root)
      cur_col=0;
    while (cur_col-line_start_col<0)
      line_start_col-=8;
    while (cur_col-line_start_col>=text_cols)
      line_start_col+=8;
    EdLiteUpdate(&root,cur_line,cur_col,line_start_col);
    switch (ch=GetKey(&sc,FALSE,TRUE)) {
      case 0:
	switch (sc.u8[0]) {
	  case SC_CURSOR_UP:
	    if (cur_line->last!=&root)
	      cur_line=cur_line->last;
	    if (cur_col>StrLen(cur_line->line))
	      cur_col=StrLen(cur_line->line);
	    break;
	  case SC_CURSOR_DOWN:
	    if (cur_line!=&root)
	      cur_line=cur_line->next;
	    if (cur_col>StrLen(cur_line->line))
	      cur_col=StrLen(cur_line->line);
	    break;
	  case SC_CURSOR_RIGHT:
	    cur_col++;
	    if (cur_col>StrLen(cur_line->line)) {
	      templ=cur_line->next;
	      if (templ!=&root) {
		cur_col=0;
		cur_line=templ;
	      } else
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_CURSOR_LEFT:
	    if (cur_col)
	      cur_col--;
	    else {
	      templ=cur_line->last;
	      if (templ!=&root) {
		cur_line=templ;
		cur_col=StrLen(templ->line);
	      }
	    }
	    break;
	  case SC_PAGE_UP:
	    for (i=1;i<text_rows;i++) {
	      if (cur_line->last!=&root)
		cur_line=cur_line->last;
	      if (cur_col>StrLen(cur_line->line))
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_PAGE_DOWN:
	    for (i=1;i<text_rows;i++) {
	      if (cur_line!=&root)
		cur_line=cur_line->next;
	      if (cur_col>StrLen(cur_line->line))
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_DELETE:
	    if (cur_col==StrLen(cur_line->line)) {
	      templ=cur_line->next;
	      if (cur_line!=&root && templ!=&root) {
		src=MAlloc(StrLen(cur_line->line)+StrLen(templ->line)+1);
		StrCpy(src,cur_line->line);
		StrCat(src,templ->line);
		Free(cur_line->line);
		Free(templ->line);
		cur_line->line=src;
		QueRem(templ);
		Free(templ);
	      }
	    } else
	      StrCpy(cur_line->line+cur_col,cur_line->line+cur_col+1);
	    break;
	}
	break;
      case CH_NEW_LINE:
	templ=MAlloc(sizeof(CLine));
	templ->line=StrNew(cur_line->line+cur_col);
	cur_line->line[cur_col]=0;
	QueIns(templ,cur_line);
	cur_line=templ;
	cur_col=0;
	break;
      case CH_BACKSPACE:
	if (cur_col) {
	  StrCpy(cur_line->line+cur_col-1,cur_line->line+cur_col);
	  cur_col--;
	} else if (cur_line!=&root && cur_line->last!=&root) {
	  templ=cur_line->last;
	  src=MAlloc(StrLen(cur_line->line)+StrLen(templ->line)+1);
	  StrCpy(src,templ->line);
	  StrCat(src,cur_line->line);
	  cur_col=StrLen(templ->line);
	  Free(cur_line->line);
	  Free(templ->line);
	  templ->line=src;
	  QueRem(cur_line);
	  Free(cur_line);
	  cur_line=templ;
	}
	break;
      case CH_CTRLY:
	if (cur_line!=&root) {
	  templ=cur_line;
	  cur_line=cur_line->next;
	  QueRem(templ);
	  Free(templ->line);
	  Free(templ);
	  cur_col=0;
	}
	break;
      default:
	if (CH_SHIFT_SPACE<=ch<=0x7E || ch==CH_TAB || 0x80<=ch<=0xFF) {
	  if (cur_line==&root) {
	    cur_line=MAlloc(sizeof(CLine));
	    cur_line->line=StrNew("");
	    QueIns(cur_line,root.last);
	  }
	  src=MAlloc(StrLen(cur_line->line)+2);
	  MemCpy(src,cur_line->line,cur_col);
	  src[cur_col]=ch;
	  if (cur_col<StrLen(cur_line->line))
	    StrCpy(src+cur_col+1,cur_line->line+cur_col);
	  else
	    src[cur_col+1]=0;
	  Free(cur_line->line);
	  cur_line->line=src;
	  cur_col++;
	}
	break;
    }
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);

  if (ch==CH_ESC) {
    size=bin_size;

    templ=root.next;
    while (templ!=&root) {
      size+=StrLen(templ->line)+1;
      templ=templ->next;
    }

    buf=dst=MAlloc(size);
    templ=root.next;
    while (templ!=&root) {
      i=StrLen(templ->line);
      MemCpy(dst,templ->line,i);
      dst+=i;
      *dst++=CH_NEW_LINE;
      templ=templ->next;
    }
    if (bin_data)
      MemCpy(dst,bin_data,bin_size);
    FileWrite(filename,buf,size);
    Free(buf);
  }

  templ=root.next;
  while (templ!=&root) {
    templ1=templ->next;
    QueRem(templ);
    Free(templ->line);
    Free(templ);
    templ=templ1;
  }
  Free(root.line);
  Free(bin_data);
  Raw(old_raw);
  InDbg(old_debug);
  SingleUser(old_single);
  sys_raw_dc_flags=sys_raw_dc_flags&~RDCF_SHOW_DOLLAR|old_dc_flags&RDCF_SHOW_DOLLAR;
  PopFD;
}
