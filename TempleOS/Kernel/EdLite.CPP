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
	if (templ->line[j]=='\t')
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
	if (ch=='\t')
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
  RawPutChar(0x7F);
}


U0 EdLite(U8 *filename,I64 num=1)
{
//Light weight text editor for debugging
  U8 *src,*src2,*src3,*dst,*buf,*bin_data=NULL;
  I64 i,cnt=0,ch,sc,size,bin_size=0,line_start_col=0,cur_col=0,
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
      while ((ch=*src++) && ch!=CH_CARRIAGE_RETURN && ch!='\n');
      src--;
      *src++=0;
      if (!ch)
	src--;
      while (ch==CH_CARRIAGE_RETURN && *src=='\n' || *src==CH_CURSOR)
	src++;
      dst=src3=src2;
      while (ch=*src3++)
	if (ch!='\n' && ch!=CH_CURSOR)
	  *dst++=ch;
      *dst=0;

      templ=MAlloc(sizeof(CLine));
      templ->line=StrNew(src2);
      QueIns(templ,root.last);
      cnt++;
    }

    if (src+1-buf<size) {
      bin_data=MAlloc(bin_size=size-(src-buf));
      MemCpy(bin_data,src,bin_size);
    }
    Free(buf);
  }

  cur_line=root.next;
  if (0<--num) {
    if (num<cnt)
      while (num--)
	cur_line=cur_line->next;
    else
      cur_line=&root;
  }
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
      case '\n':
      case CH_CARRIAGE_RETURN:
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
	if (Bt(getkey_chars_bitmap,ch)) {
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
      *dst++='\n';
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

U0 EdLiteFileLine(U8 *_fl_file_line)
{
  U8 *st,*fl_file_line=StrNew(_fl_file_line);
  I64 linenum;
  StrFirstRem(fl_file_line,":");
  st=StrNew(fl_file_line);
  StrLastRem(fl_file_line,",",st);
  linenum=A2I(st);
  Free(st);
  EdLite(fl_file_line,linenum);
  Free(fl_file_line);
}

U0 SetFix(U8 *filename,I64 line)
{
  U8 *st=MSPrintF("FL:%s,%d",filename,line);
  while (LBts(&sys_semas[SYS_SEMA_FIX],0))
    Yield;
  Free(sys_fix_file_line);
  sys_fix_file_line=AStrNew(st);
  LBtr(&sys_semas[SYS_SEMA_FIX],0);
}

U0 Fix()
{//Fix last compile err
  U8 *st;

  while (LBts(&sys_semas[SYS_SEMA_FIX],0))
    Yield;
  st=StrNew(sys_fix_file_line);
  LBtr(&sys_semas[SYS_SEMA_FIX],0);

  if (st) {
    if (IsRaw)
      EdLiteFileLine(st);
    else
      Ed(st);
  }
  Free(st);
}
