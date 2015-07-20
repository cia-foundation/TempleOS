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
  text.raw_col=0;
  for (i=0;i<text.rows/2;i++)
    if (templ->last!=root)
      templ=templ->last;
  for (i=0;i<text.rows;i++) {
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
	if (line_start_col<=k<line_start_col+text.cols) {
	  '' ch;
	  j=k2-line_start_col;
	}
	k=k2;
      }
      if (j<text.cols)
	'\n';
      templ=templ->next;
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
  CLine root,*templ,*templ1,*cur_line;
  Bool	result=FALSE,
	old_raw=Raw(ON),
	old_debug=InDbg(ON),
	old_single=SingleUser(ON);

  if (!filename) filename=dsk.temp_filename;
  buf=FileRead(filename,&size);

  PUSHFD
  CLI
  text.raw_flags=text.raw_flags&~RWF_SCROLL|RWF_SHOW_DOLLAR;
  kbd.scan_code=0;
  QueInit(&root);
  root.line=StrNew("");

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
    result=TRUE;
  }

  cur_line=root.next;
  if (--num<0)
    result=FALSE;
  else {
    if (num<=cnt)
      while (num--)
	cur_line=cur_line->next;
    else {
      cur_line=&root;
      result=FALSE;
    }
  }
  do {
    if (cur_line==&root)
      cur_col=0;
    while (cur_col-line_start_col<0)
      line_start_col-=8;
    while (cur_col-line_start_col>=text.cols)
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
	    for (i=1;i<text.rows;i++) {
	      if (cur_line->last!=&root)
		cur_line=cur_line->last;
	      if (cur_col>StrLen(cur_line->line))
		cur_col=StrLen(cur_line->line);
	    }
	    break;
	  case SC_PAGE_DOWN:
	    for (i=1;i<text.rows;i++) {
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
		src=MStrPrint("%s%s",cur_line->line,templ->line);
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
      case '\r':
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
	  src=MStrPrint("%s%s",templ->line,cur_line->line);
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
	if (Bt(chars_bmp_getkey,ch)) {
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
    }
  } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);

  if (ch!=CH_ESC) {
    if (edf_dof_flags&EDF_WAS_WRITE)
      result=FALSE;
  } else {
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

    if (edf_dof_flags&EDF_WAS_WRITE)
      result=TRUE;
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
  text.raw_flags=text.raw_flags&~RWF_SHOW_DOLLAR|old_raw_flags&RWF_SHOW_DOLLAR;
  POPFD
  return result;
}

U0 ToFileLine(U8 *_fl_file_line,U8 **_filename,I64 *_linenum)
{//"FI:D:/Dir/File.CPP,123" to "D:/Dir/File.CPP" and 123.
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
  Bool result;
  U8 *filename;
  I64 linenum;
  ToFileLine(fl_file_line,&filename,&linenum);
  result=EdLite(filename,linenum,edf_dof_flags);
  Free(filename);
  return result;
}

U0 FixSet(U8 *filename,I64 line)
{//Compiler calls this to set file line for Fix
  U8 *st=MStrPrint("FL:%s,%d",filename,line);
  while (LBts(&sys_semas[SYS_SEMA_FIX],0))
    Yield;
  Free(dbg.fix_file_line);
  dbg.fix_file_line=AStrNew(st);
  LBtr(&sys_semas[SYS_SEMA_FIX],0);
}

Bool Fix(I64 edf_dof_flags=0)
{//Jump to last err src code to fix it.
  U8 *st;
  Bool result=FALSE;

  while (LBts(&sys_semas[SYS_SEMA_FIX],0))
    Yield;
  st=StrNew(dbg.fix_file_line);
  LBtr(&sys_semas[SYS_SEMA_FIX],0);

  if (st) {
    if (IsRaw)
      result=EdLiteFileLine(st,edf_dof_flags);
    else
      result=Ed(st,edf_dof_flags);
  }
  Free(st);
  return result;
}
