I64 Str2I64(U8 *st,I64 radix=10,U8 **_end_ptr=NULL)
{//String to I64. Similar to strtoul().
//Allows radix change with "0x20" "0b1010" "0d123" "0o18".
  //Be careful of Str2I64("0b101",16)-->0xB101.
  Bool neg=FALSE;
  I64 ch,res=0;
  if (!st || !(2<=radix<=36)) {
    if (_end_ptr) *_end_ptr=st;
    return 0;
  }
  while (Bt(char_bmp_white_space,*st))
    st++;
  while (TRUE)
    switch (*st) {
      case '-':
	st++;
	neg=!neg;
	break;
      case '+':
	st++;
	break;
      case '0':
	st++;
	ch=ToUpper(*st);
	if (ch>='B' && (radix<=10 || ch>'A'+radix-11))
	  switch (ch) {
	    case 'B': radix=2;	st++; break;
	    case 'D': radix=10; st++; break;
	    case 'X': radix=16; st++; break;
	  }
      default:
	goto ai_cont;
    }
ai_cont:
  while (ch=ToUpper(*st++)) {
    if (radix>10) {
      if ('0'<=ch<='9')
	res=res*radix+ch-'0';
      else if ('A'<=ch<='A'+radix-11)
	res=res*radix+ch-'A'+10;
      else
	break;
    } else if ('0'<=ch<='0'+radix-1)
      res=res*radix+ch-'0';
    else
      break;
  }
  if (_end_ptr) *_end_ptr=st-1;
  if (neg)
    return -res;
  else
    return res;
}

F64 Str2F64(U8 *src,U8 **_end_ptr=NULL)
{/*String to F64.
Does not allow more than 18-digits
before or after the decimal point
because the numbers before and after
the decimal point are stored
in 64-bits.  Use exponentiated forms
to avoid this.
*/
  I64 i,j,k,ch;
  F64 d;
  Bool neg=FALSE,neg_e=FALSE;

  ch=*src++;
  while (Bt(char_bmp_white_space,ch))
    ch=*src++;
  if (ch=='-') {
    neg=TRUE;
    ch=*src++;
  }
  if (!StrNCmp(src-1,"inf",3)) {
    d=ì;
    src+=3;
    goto a2f_end;
  }
  if (*src=='ì') {
    d=ì;
    src++;
    goto a2f_end;
  }
  i=0;
  while (TRUE) {
    if (Bt(char_bmp_dec_numeric,ch))
      i=i*10+ch-'0';
    else {
      if (ch=='.' || ch=='e' || ch=='E')
	break;
      d=i;
      goto a2f_end;
    }
    ch=*src++;
  }
  if (ch=='.')
    ch=*src++;
  k=0;
  while (TRUE) {
    if (Bt(char_bmp_dec_numeric,ch)) {
      i=i*10+ch-'0';
      k++;
    } else {
      if (ch=='e' || ch=='E')
	break;
      d=i*Pow10I64(-k);
      goto a2f_end;
    }
    ch=*src++;
  }
  ch=*src++;
  if (ch=='-') {
    neg_e=TRUE;
    ch=*src++;
  }
  j=0;
  while (TRUE) {
    if (Bt(char_bmp_dec_numeric,ch))
      j=j*10+ch-'0';
    else {
      if (neg_e)
	d=i*Pow10I64(-j-k);
      else
	d=i*Pow10I64(j-k);
      goto a2f_end;
    }
    ch=*src++;
  }
a2f_end:
  if (_end_ptr) *_end_ptr=src-1;
  if (neg)
    return -d;
  else
    return d;
}

CDate Str2Date(U8 *_src)
{/*"*+nnnn", "*-nnnn", "mm/dd", "mm/dd/yy"
It also supports some funs
SM() start of mon
EM() end of mon
SY() start of year
EY() end of year
Full expressions are not implimented
but you can do stuff like SM(*-7)+3
and it will return the 3rd day after
the start of mon for seven days before
today.
*/
  CDate res=0;
  CDateStruct ds,ds_now;
  U8 *src=MStrUtil(_src,SUF_REM_SPACES|SUF_TO_UPPER),
	*v=StrNew(src),
	*ptr=src;
  Bool start_mon=FALSE,end_mon=FALSE,
	start_year=FALSE,end_year=FALSE;

  MemSet(&ds,0,sizeof(CDateStruct));
  if (!StrNCmp(ptr,"SM(",3)) {
    ptr+=3;
    start_mon=TRUE;
  } else if (!StrNCmp(ptr,"EM(",3)) {
    ptr+=3;
    end_mon=TRUE;
  } else if (!StrNCmp(ptr,"SY(",3)) {
    ptr+=3;
    start_year=TRUE;
  } else if (!StrNCmp(ptr,"EY(",3)) {
    ptr+=3;
    end_year=TRUE;
  }
  if (*ptr=='*') {
    ptr++;
    if (*ptr=='+' || *ptr=='-')
      res.date=Str2I64(ptr,,&ptr);
    res+=Now+local_time_offset;
  } else {
    StrFirstRem(ptr,"/",v); //Put mon into v
    ds.mon=Str2I64(v);
    if (StrOcc(ptr,'/')) {
      StrFirstRem(ptr,"/",v); //Put day into v leaving year in ptr
      ds.day_of_mon=Str2I64(v);
      ds.year=Str2I64(ptr,,&ptr);
      if (ds.year<100) //if not 4 digit year
	ds.year+=2000;
    } else {
      ds.day_of_mon=Str2I64(ptr,,&ptr);
      Date2Struct(&ds_now,Now+local_time_offset);
      ds.year=ds_now.year;
    }
    res=Struct2Date(&ds);
  }
  if (*ptr==')') ptr++;

  if (start_mon)
    res.date=FirstDayOfMon(res.date);
  else if (end_mon)
    res.date=LastDayOfMon(res.date);
  else if (start_year)
    res.date=FirstDayOfYear(res.date);
  else if (end_year)
    res.date=LastDayOfYear(res.date);

  if (*ptr=='+' || *ptr=='-')
    res.date+=Str2I64(ptr);
  Free(src);
  Free(v);
  return res-local_time_offset;
}

U8 *StrScan(U8 *src,U8 *fmt,...)
{/*Opposite of sprintf().  Pass ptrs to data to be scanned-in.
For "%s", pass ptr to ptr (be careful because addr
of array is the same as array--create ptr to array
and take addr.
*/
  U8 *buf,*ptr,**pptr;
  Bool left_justify=FALSE;
  I64 ch,cur_arg=0,i,len,*i_ptr,dec_len;
  F64 *d_ptr;
  if (!fmt)
    throw('Scan');
  while (ch = *fmt++) {
    if (ch=='%') {
      if (*fmt=='%') {
	src++;
	fmt++;
      } else {
	if (*fmt=='-') {
	  left_justify=TRUE;
	  fmt++;
	} else
	  left_justify=FALSE;
	len=0;
	while ('0'<=*fmt<='9')
	  len=len*10+ (*fmt++ -'0');
	if (*fmt=='*') {
	  fmt++;
	  if (cur_arg>=argc)
	    throw('Scan');
	  len=argv[cur_arg++];
	}
	ch=*fmt++;
	if (ch && !len) {
	  ptr=src;
	  while (*ptr && *ptr!=*fmt)
	    ptr++;
	  len=ptr-src;
	} else {
	  if (ch=='.') {
	    dec_len=0;
	    while ('0'<=*fmt<='9')
	      dec_len=dec_len*10+ (*fmt++-'0');
	    if (*fmt=='*') {
	      fmt++;
	      if (cur_arg>=argc)
		throw('Scan');
	      dec_len=argv[cur_arg++];
	    }
	    ch=*fmt++;
	  }
	}
	buf=MAlloc(len+1);
	for (i=0;i<len;i++)
	  buf[i]=*src++;
	buf[i]=0;
	switch (ch) {
	  case 's':
	    if (cur_arg>=argc)
	      throw('Scan');
	    pptr=argv[cur_arg++];
	    StrCpy(*pptr,buf);
	    break;
	  case 'c':
	    if (cur_arg>=argc)
	      throw('Scan');
	    ptr=argv[cur_arg++];
	    *ptr=*buf;
	    break;
	  case 'C':
	    if (cur_arg>=argc)
	      throw('Scan');
	    ptr=argv[cur_arg++];
	    *ptr=ToUpper(*buf);
	    break;
	  case 'z':
	    if (cur_arg+1>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=LstMatch(buf,argv[cur_arg++]);
	    break;
	  case 'Z':
	    if (cur_arg+1>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=DefineMatch(buf,argv[cur_arg++]);
	    break;
	  case 'd':
	    if (cur_arg>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=Str2I64(buf);
	    break;
	  case 'X':
	    if (cur_arg>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=Str2I64(buf,16);
	    break;
	  case 'b':
	    if (cur_arg>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=Str2I64(buf,2);
	    break;
	  case 'e':
	  case 'f':
	  case 'g':
	  case 'n':
	    if (cur_arg>=argc)
	      throw('Scan');
	    d_ptr=argv[cur_arg++];
	    *d_ptr=Str2F64(buf);
	    break;
	  case 'D':
	    if (cur_arg>=argc)
	      throw('Scan');
	    i_ptr=argv[cur_arg++];
	    *i_ptr=Str2Date(buf);
	    break;
	}
	Free(buf);
      }
    } else
      src++;
  }
  return src;
}
