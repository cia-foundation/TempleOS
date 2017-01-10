class CLine
{
  CLine *next,*last;
  U8 *line;
};

U0 EdLiteUpdate(CLine *head,CLine *cur_line,I64 cur_col,I64 line_start_col)
{
  I64 ch,i,j,k,k2,cursor_col,cursor_row=-1;
  U8 *st;
  CLine *tmpl=cur_line;
  Bool done_eof=FALSE;
  text.raw_col=0;
  for (i=0;i<text.rows/2;i++)
    if (tmpl->last!=head)
      tmpl=tmpl->last;
  for (i=0;i<text.rows;i++) {
    if (cursor_row<0 && tmpl==cur_line) {
      k=0;
      for (j=0;j<cur_col;j++)
	if (tmpl->line[j]=='\t')
	  k=(k+8)&~7;
	else
	  k++;
      cursor_col=k;
      cursor_row=i;
    }
    if (tmpl!=head) {
      st=tmpl->line;
      k=0;
      j=0;
      while (ch=*st++) {
	if (ch=='\t')
	  k2=(k+8)&~7;
	else
	  k2=k+1;
	if (line_start_col<=k<line_start_col+text.cols) {
	  '' ch;
	  j=k2-line_start_col;
	}
	k=k2;
      }
      if (j<text.cols)
	'\n';
      tmpl=tmpl->next;
    } else {
      if (!done_eof) {
	'<EOF>';
	done_eof=TRUE;
      }
      '\n';
    }
  }
  text.raw_col=text.cols*cursor_row+cursor_col-line_start_col;
  RawPutChar(0x7F);
}

Bool EdLite(U8 *filename,I64 num=1,I64 edf_dof_flags=0)
{//Light weight text editor for debugging.
  U8 *src,*src2,*src3,*dst,*buf,*bin_data=NULL;
  I64 i,cnt=0,ch,sc,size,bin_size=0,line_start_col=0,cur_col=0,
	old_raw_flags=text.raw_flags;
  CLine head,*tmpl,*tmpl1,*cur_line;
  Bool	res=FALSE,
	old_raw=Raw(ON),
	old_debug=DbgMode(ON),
	old_single=SingleUser(ON);

  if (!filename) filename=blkdev.tmp_filename;
  buf=FileRead(filename,&size);

  PUSHFD
  CLI
  text.raw_flags=text.raw_flags&~RWF_SCROLL|RWF_SHOW_DOLLAR;
  kbd.scan_code=0;
  QueInit(&head);
  head.line=StrNew("");

  if (buf) {
    src=buf;
    while (*src) {
      src2=src;
      while ((ch=*src++) && ch!='\r' && ch!='\n');
      src--;
      *src++=0;
      if (!ch)
	src--;
      while (ch=='\r' && *src=='\n' || *src==CH_CURSOR)
	src++;
      dst=src3=src2;
      while (ch=*src3++)
	if (ch!='\n' && ch!=CH_CURSOR)
	  *dst++=ch;
      *dst=0;

      tmpl=MAlloc(sizeof(CLine));
      tmpl->line=StrNew(src2);
      QueIns(tmpl,head.last);
      cnt++;
    }

    if (src+1-buf<size) {
      bin_data=MAlloc(bin_size=size-(src-buf));
      MemCpy(bin_data,src,bin_size);
    }
    Free(buf);
    res=TRUE;
  }

  cur_line=head.next;
  if (--num<0)
    res=FALSE;
  else {
    if (num<=cnt)
      while (num--)
	cur_line=cur_line->next;
    else {
      cur_line=&head;
      res=FALSE;
    }
  }
  do {
    if (cur_line==&head)
      cur_col=0;
    while (cur_col-line_start_col<0)
      line_start_col-=8;
    while (cur_col-line_start_col>=text.cols)
      line_start_col+=8;
    EdLiteUpdate(&head,cur_line,cur_col,line_start_col);
    switch (ch=GetKey(&sc,FALSE,TRUE)) {
      case 0:
	switch (sc.u8[0]) {
	  case SC_CURSOR_UP:
	    if (cur_line->last!=&head)
	      cur_line=cur_line->last;
	    if (cur_col>StrLen(cur_line->line))
	      cur_col=StrLen(cur_line->line);
	    break;
	  case SC_CURSOR_DOWN:
	    if (cur_line!=&head)
	      cur_line=cur_line->next;
	    if (cur_col>StrLen(cur_line->line))
	      cur_col=StrLen(cur_line->line);
	    break;
	  case SC_CURSOR_RIGHT:
	    cur_col++;
	    if (cur_col>StrLen(cur_line->line)) {
	      tmpl=cur_line->next;
	      if (tmpl!=&head) {
		cur_col=0;
		cur_line=tmpl;
	      } else
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_CURSOR_LEFT:
	    if (cur_col)
	      cur_col--;
	    else {
	      tmpl=cur_line->last;
	      if (tmpl!=&head) {
		cur_line=tmpl;
		cur_col=StrLen(tmpl->line);
	      }
	    }
	    break;
	  case SC_PAGE_UP:
	    for (i=1;i<text.rows;i++) {
	      if (cur_line->last!=&head)
		cur_line=cur_line->last;
	      if (cur_col>StrLen(cur_line->line))
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_PAGE_DOWN:
	    for (i=1;i<text.rows;i++) {
	      if (cur_line!=&head)
		cur_line=cur_line->next;
	      if (cur_col>StrLen(cur_line->line))
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_DELETE:
	    if (cur_col==StrLen(cur_line->line)) {
	      tmpl=cur_line->next;
	      if (cur_line!=&head && tmpl!=&head) {
		src=MStrPrint("%s%s",cur_line->line,tmpl->line);
		Free(cur_line->line);
		Free(tmpl->line);
		cur_line->line=src;
		QueRem(tmpl);
		Free(tmpl);
	      }
	    } else
	      StrCpy(cur_line->line+cur_col,cur_line->line+cur_col+1);
	    break;
	}
	break;
      case '\n':
      case '\r':
	tmpl=MAlloc(sizeof(CLine));
	tmpl->line=StrNew(cur_line->line+cur_col);
	cur_line->line[cur_col]=0;
	QueIns(tmpl,cur_line);
	cur_line=tmpl;
	cur_col=0;
	break;
      case CH_BACKSPACE:
	if (cur_col) {
	  StrCpy(cur_line->line+cur_col-1,cur_line->line+cur_col);
	  cur_col--;
	} else if (cur_line!=&head && cur_line->last!=&head) {
	  tmpl=cur_line->last;
	  src=MStrPrint("%s%s",tmpl->line,cur_line->line);
	  cur_col=StrLen(tmpl->line);
	  Free(cur_line->line);
	  Free(tmpl->line);
	  tmpl->line=src;
	  QueRem(cur_line);
	  Free(cur_line);
	  cur_line=tmpl;
	}
	break;
      case CH_CTRLY:
	if (cur_line!=&head) {
	  tmpl=cur_line;
	  cur_line=cur_line->next;
	  QueRem(tmpl);
	  Free(tmpl->line);
	  Free(tmpl);
	  cur_col=0;
	}
	break;
      default:
	if (Bt(char_bmp_printable,ch)) {
	  if (cur_line==&head) {
	    cur_line=MAlloc(sizeof(CLine));
	    cur_line->line=StrNew("");
	    QueIns(cur_line,head.last);
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
    }
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);

  if (ch!=CH_ESC) {
    if (edf_dof_flags&EDF_WAS_WRITE)
      res=FALSE;
  } else {
    size=bin_size;

    tmpl=head.next;
    while (tmpl!=&head) {
      size+=StrLen(tmpl->line)+1;
      tmpl=tmpl->next;
    }

    buf=dst=MAlloc(size);
    tmpl=head.next;
    while (tmpl!=&head) {
      i=StrLen(tmpl->line);
      MemCpy(dst,tmpl->line,i);
      dst+=i;
      *dst++='\n';
      tmpl=tmpl->next;
    }
    if (bin_data)
      MemCpy(dst,bin_data,bin_size);
    FileWrite(filename,buf,size);
    Free(buf);

    if (edf_dof_flags&EDF_WAS_WRITE)
      res=TRUE;
  }

  tmpl=head.next;
  while (tmpl!=&head) {
    tmpl1=tmpl->next;
    QueRem(tmpl);
    Free(tmpl->line);
    Free(tmpl);
    tmpl=tmpl1;
  }
  Free(head.line);
  Free(bin_data);
  Raw(old_raw);
  DbgMode(old_debug);
  SingleUser(old_single);
  text.raw_flags=text.raw_flags&~RWF_SHOW_DOLLAR|old_raw_flags&RWF_SHOW_DOLLAR;
  POPFD
  return res;
}

U0 ToFileLine(U8 *_fl_file_line,U8 **_filename,I64 *_linenum)
{//"FI:D:/Dir/File.HC,123" to "D:/Dir/File.HC" and 123.
  U8 *st,*fl_file_line=StrNew(_fl_file_line);
  I64 linenum;
  StrFirstRem(fl_file_line,":");
  st=StrNew(fl_file_line);
  StrLastRem(fl_file_line,",",st);
  linenum=Str2I64(st);
  Free(st);
  *_filename=fl_file_line;
  *_linenum=linenum;
}

Bool EdLiteFileLine(U8 *fl_file_line,I64 edf_dof_flags=0)
{
  Bool res;
  U8 *filename;
  I64 linenum;
  ToFileLine(fl_file_line,&filename,&linenum);
  res=EdLite(filename,linenum,edf_dof_flags);
  Free(filename);
  return res;
}

U0 FixSet(U8 *filename,I64 line)
{//Compiler calls this to set file line for Fix
  U8 *st=MStrPrint("FL:%s,%d",filename,line);
  while (LBts(&sys_semas[SEMA_FIX],0))
    Yield;
  Free(dbg.fix_file_line);
  dbg.fix_file_line=AStrNew(st);
  LBtr(&sys_semas[SEMA_FIX],0);
}

Bool Fix(I64 edf_dof_flags=0)
{//Jump to last err src code to fix it.
  U8 *st;
  Bool res=FALSE;

  while (LBts(&sys_semas[SEMA_FIX],0))
    Yield;
  st=StrNew(dbg.fix_file_line);
  LBtr(&sys_semas[SEMA_FIX],0);

  if (st) {
    if (IsRaw)
      res=EdLiteFileLine(st,edf_dof_flags);
    else
      res=Ed(st,edf_dof_flags);
  }
  Free(st);
  return res;
}
