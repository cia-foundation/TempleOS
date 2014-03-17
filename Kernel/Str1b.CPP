U8 *Tabs2Spaces(U8 *src)
{
  I64 ch,i,j,l=StrLen(src)<<1+2,col=0;
  U8 *dst=MAlloc(l),*temp;
  while (ch=*src++) {
    if (ch=='\t') {
      j=(col+8) & ~7;
      for (i=col;i<j;i++) {
	dst[i]=CH_SPACE;
	if (i>=l-2) {
	  temp=MAlloc(l<<1);
	  MemCpy(temp,dst,i+1);
	  Free(dst);
	  l<<=1;
	  dst=temp;
	}
      }
      col=j;
    } else {
      dst[col]=ch;
      if (col>=l-2) {
	temp=MAlloc(l<<1);
	MemCpy(temp,dst,col+1);
	Free(dst);
	l<<=1;
	dst=temp;
      }
      col++;
    }
  }
  dst[col]=0;
  return dst;
}

U8 *ScaleIndent(U8 *src,F64 indent_scale_factor)
{
  I64 ch,i,col=0;
  U8 *dst,*dst2;
  while (ch=*src++) {
    if (ch=='\t')
      col=(col+8) & -0x8;
    else if (ch==CH_SPACE)
      col++;
    else
      break;
  }
  src--;
  col=Round(indent_scale_factor*col);
  dst=dst2=MAlloc(StrLen(src)+col/8+col&7+1);
  for (i=col/8;i>0;i--)
    *dst2++='\t';
  for (i=col&7;i>0;i--)
    *dst2++=CH_SPACE;
  StrCpy(dst2,src);
  return dst;
}

U8 *MStrUtil(U8 *src,I64 flags,F64 indent_scale_factor=0)
//Return a $LK,"MAlloc",A="MN:MAlloc"$()ed result from StrUtil
{
  U8 *dst=StrNew(src),*dst2,*temp;
  StrUtil(dst,flags);
  if (flags & SUF_T2S) {
    temp=Tabs2Spaces(dst);
    Free(dst);
    dst=temp;
  }
  if (flags & SUF_SCALE_INDENT)
    dst2=ScaleIndent(dst,indent_scale_factor);
  else
    dst2=StrNew(dst); //Shorten
  Free(dst);
  return dst2;
}

U0 GetOutOfDollar()
{
  CDoc *doc;
  if (IsRaw) {
    if (sys_raw_flags&RWF_IN_DOLLAR)
      '$$';
  } else {
    if (fp_doc_put && (doc=(*fp_doc_put)(Fs)) && doc->flags&DOCF_IN_DOLLAR)
      '$$';
  }
}

U0 NewLine(I64 cnt=1)
{
  while (cnt-->0)
    '\n';
}

U0 Spaces(I64 cnt=1)
{
  while (cnt-->0)
    '' CH_SPACE;
}

Bool YorN()
{
  I64 ch;
  "(y or n)? ";
  while (TRUE) {
    ch=ToUpper(GetChar(,FALSE));
    if (ch=='Y') {
      "$$PT$$YES$$FG$$\n";
      return TRUE;
    } else if (ch=='N') {
      "$$PT$$NO$$FG$$\n";
      return FALSE;
    }
  }
}

I64 PressAKey()
{
  "$$BK,1$$PRESS A KEY$$BK,0$$\n";
  return GetChar(,FALSE);
}

Bool AreYouSure()
{
  "ARE YOU SURE ";
  return YorN;
}

U0 Help()
{
  if (IsDbg)
    DbgHelp;
  else
    PopUp("Type(\"::/Doc/HelpIndex.TXT\");DocHome;View;");
}


U0 ScanFlags(U8 *f,U8 *lst,U8 *st)
{
  I64 i;
  U8 *st2,*ptr;
  if (st) {
    st2=MAlloc(StrLen(st)+1);
    while (*st) {
      while (*st && *st!='+' && *st!='-')
	st++;
      if (*st=='+') {
	st++;
	if (*st) {
	  ptr=st2;
	  while (*st && *st!='+' && *st!='-' && *st!=CH_SPACE && *st!=CH_SHIFT_SPACE)
	    *ptr++=*st++;
	  *ptr=0;
	  i=LstMatch(st2,lst);
	  if (i>=0)
	    LBts(f,i);
	  else {
	    Free(st2);
	    throw('ScanFlag');
	  }
	}
      } else if (*st=='-') {
	st++;
	if (*st) {
	  ptr=st2;
	  while (*st && *st!='+' && *st!='-' && *st!=CH_SPACE && *st!=CH_SHIFT_SPACE)
	    *ptr++=*st++;
	  *ptr=0;
	  i=LstMatch(st2,lst);
	  if (i>=0)
	    LBtr(f,i);
	  else {
	    Free(st2);
	    throw('ScanFlag');
	  }
	}
      }
    }
    Free(st2);
  }
}

U8 *SPrintFlags(U8 *dst,U8 *lst,I64 flags)
{
  I64 i;
  *dst=0;
  while (flags) {
    i=Bsf(flags);
    Btr(&flags,i);
    CatPrint(dst,"+%z",i,lst);
  }
  return dst;
}
