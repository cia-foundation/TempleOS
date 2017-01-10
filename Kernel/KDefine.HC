CHashDefineStr *DefineLoad(U8 *dname,U8 *st)
{//Create DEFINE hash entry with string.
  CHashDefineStr *tmph=CAlloc(sizeof(CHashDefineStr));
  tmph->type=HTT_DEFINE_STR;
  tmph->str=StrNew(dname);
  tmph->data=StrNew(st);
  tmph->cnt=-1;
  tmph->src_link=MStrPrint("AD:0x%X",Caller);
  HashAdd(tmph,Fs->hash_table);
  return tmph;
}

CHashDefineStr *DefineLstLoad(U8 *dname,U8 *lst)
{//Create DEFINE list. Not efficient, but handy.
  I64 cnt=0;
  U8 *ptr,**idx;
  CHashDefineStr *tmph=CAlloc(sizeof(CHashDefineStr));
  tmph->type=HTT_DEFINE_STR;
  tmph->str=StrNew(dname);
  tmph->src_link=MStrPrint("AD:0x%X",Caller);
  ptr=lst;
  while (*ptr) {
    if (*ptr!='@')
      cnt++;
    while (*ptr++);
  }
  tmph->data=MAlloc(ptr+1-lst);
  MemCpy(tmph->data,lst,ptr+1-lst);
  tmph->cnt=cnt;

  idx=tmph->sub_idx=MAlloc(cnt*sizeof(U8 *));
  ptr=lst;
  while (*ptr) {
    if (*ptr!='@')
      *idx++=ptr;
    while (*ptr++);
  }

  HashAdd(tmph,Fs->hash_table);
  return tmph;
}

U0 UndefinedDefine(U8 *dname)
{
  ST_ERR_ST "Undefined Define: '%s'.\n",dname;
  throw('UndefDef');
}

U8 *Define(U8 *dname)
{//Look for DEFINE named in hash table, return ptr string.
  CHashDefineStr *tmph;
  if (tmph=HashFind(dname,Fs->hash_table,HTT_DEFINE_STR))
    return tmph->data;
  else if (dname)
    UndefinedDefine(dname);
  else
    return NULL;
}

U8 *DefineSub(I64 sub,U8 *dname)
{//Return DEFINE list entry indexed by number.
  CHashDefineStr *tmph;
  if (tmph=HashFind(dname,Fs->hash_table,HTT_DEFINE_STR)) {
    if (0<=sub<tmph->cnt)
      return tmph->sub_idx[sub];
    else
      return NULL;
  } else if (dname)
    UndefinedDefine(dname);
  else
    return NULL;
}

I64 DefineCnt(U8 *dname)
{//Return cnt of entries in define list.
  CHashDefineStr *tmph;
  if (tmph=HashFind(dname,Fs->hash_table,HTT_DEFINE_STR))
    return tmph->cnt;
  else if (dname)
    UndefinedDefine(dname);
  else
    return -1;
}

I64 DefineMatch(U8 *needle,U8 *haystack_lst_dname,I64 flags=0)
{//Find match for string in define list.
  return LstMatch(needle,Define(haystack_lst_dname),flags);
}

U0 DefinePrint(U8 *dname,U8 *src,...)
{//Create DEFINE entry with $LK,"Print",A="FI:::/Doc/Print.DD"$()ed string.
  U8 *buf=StrPrintJoin(NULL,src,argc,argv);
  DefineLoad(dname,buf);
  Free(buf);
}

U0 SysDefinesLoad()
{
  DefineLstLoad("ST_OFF_ON","Off\0On\0");
  DefineLstLoad("ST_HTT_TYPES","ExportSysSym\0ImportSysSym\0DefineStr\0GlbVar\0"
	"Class\0IntType\0Funct\0Word\0DictWord\0KeyWord\0AsmKeyWord\0OpCode\0"
	"Reg\0File\0Module\0HelpFile\0Frame Ptr\0 \0 \0 \0 \0 \0 \0Private\0"
	"Public\0Export\0Import\0Imm\0Goto\0Res\0Unres\0Local\0");
  DefineLstLoad("ST_DAYS_OF_WEEK","Sunday\0Monday\0Tuesday\0Wednesday\0"
	"Thursday\0Friday\0Saturday\0");
  DefineLstLoad("ST_MONTHS","January\0February\0March\0April\0May\0"
	"June\0July\0August\0September\0October\0November\0December\0");
  DefineLstLoad("ST_FILE_ATTRS","R\0H\0S\0V\0D\0A\0 \0 \0X\0T\0Z\0C\0F\0");
  DefineLstLoad("ST_FILE_UTIL_FLAGS","r\0d\0i\0a\0c\0R\0p\0m\0x\0s\0"
	"D\0F\0T\0$$\0S\0A\0J\0G\0Z\0O\0P\0f\0l\0lb\0la\0");
  DefineLstLoad("ST_BLKDEV_TYPES",
	"NULL\0RAM\0ATA\0FILE_READ\0FILE_WRITE\0ATAPI\0");
  DefineLstLoad("ST_DRV_TYPES",
	"NULL\0REDSEA\0FAT32\0ISO9660\0ISO13490\0ISO13346\0NTFS\0UNKNOWN\0");
  DefineLstLoad("ST_COLORS","BLACK\0BLUE\0GREEN\0CYAN\0"
	"RED\0PURPLE\0BROWN\0LTGRAY\0DKGRAY\0LTBLUE\0LTGREEN\0"
	"LTCYAN\0LTRED\0LTPURPLE\0YELLOW\0WHITE\0");
  DefineLstLoad("ST_INT_NAMES","Divide Error\0SingleStep\0NMI\0Breakpoint\0"
	"Overflow\0BOUND Range Exceeded\0Invalid Opcode\0No Math Coprocessor\0"
	"Double Fault\0Coprocessor Segment Fault\0Invalid TASK\0"
	"Segment Not Present\0Stk Segment Fault\0General Protection\0"
	"Page Fault\0 \0Math Fault\0Alignment Check\0Machine Check\0"
	"SIMD Exception\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0"
	" \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0MP Crash\0Wake\0Dbg\0");
}

U8 *Color2Str(U8 *buf,CColorROPU32 c)
{//$LK,"CColorROPU32",A="MN:CColorROPU32"$ with flags to $LK,"DCLighting",A="MN:DCLighting"$ str.
  *buf=0;
  if (c.c0.rop&ROPBF_TWO_SIDED)
    CatPrint(buf,"TWO|");
  if (c.c0.rop&ROPBF_HALF_RANGE_COLOR)
    CatPrint(buf,"HALF|");
  if (0<=c.c0.color<16)
    CatPrint(buf,DefineSub(c.c0.color,"ST_COLORS"));
  else if (c.c0.color==TRANSPARENT)
    CatPrint(buf,"TRANSPARENT");
  else
    CatPrint(buf,"INVALID");
  if (c&ROPF_DITHER) {
    CatPrint(buf,"/");
    if (c.c1.rop&ROPBF_TWO_SIDED)
      CatPrint(buf,"TWO|");
    if (c.c1.rop&ROPBF_HALF_RANGE_COLOR)
      CatPrint(buf,"HALF|");
    if (0<=c.c1.color<16)
      CatPrint(buf,DefineSub(c.c1.color,"ST_COLORS"));
    else if (c.c1.color==TRANSPARENT)
      CatPrint(buf,"TRANSPARENT");
    else
      CatPrint(buf,"INVALID");
  }
  return buf;
}

U8 *str2color_lst="/,)}>";

CColorROPU16 Str2ColorU16(U8 *st)
{//$LK,"DCLighting",A="MN:DCLighting"$ color str with flags to $LK,"CColorROPU16",A="MN:CColorROPU16"$.
  CColorROPU16 res=COLOR_INVALID;
  I64 i;
  U8 *ptr,*ptr2,*st2;
  if (!st) return COLOR_INVALID;
  while (TRUE) {
    if (!*st||StrOcc(str2color_lst,*st))
      return res;
    if (Bt(char_bmp_alpha,*st)) {
      ptr=st;
      while (Bt(char_bmp_alpha_numeric,*ptr))
	ptr++;
      st2=ptr2=MAlloc(ptr-st+1);
      while (st<ptr)
	*ptr2++=*st++;
      *ptr2++=0;
      if (!StrICmp(st2,"TWO"))
	res.rop|=ROPBF_TWO_SIDED;
      else if (!StrICmp(st2,"HALF"))
	res.rop|=ROPBF_HALF_RANGE_COLOR;
      else if ((i=DefineMatch(st2,"ST_COLORS",LMF_IGNORE_CASE))>=0)
	res.color=i;
      else if (!StrICmp(st2,"TRANSPARENT"))
	res.color=TRANSPARENT;
      else {
	Free(st2);
	return COLOR_INVALID;
      }
      Free(st2);
    } else if (*st=='+'||*st=='|'||Bt(char_bmp_white_space,*st))
      st++;
    else if ('0'<=*st<='9') {
      i=Str2I64(st,10,&ptr);
      if (0<=i<=0xFF) {
	res.color=i;
	st=ptr;
      } else
	return COLOR_INVALID;
    } else
      return COLOR_INVALID;
  }
}

CColorROPU32 Str2ColorU32(U8 *st)
{//$LK,"DCLighting",A="MN:DCLighting"$ color str with flags to $LK,"CColorROPU32",A="MN:CColorROPU32"$.
  U8 *st2;
  CColorROPU32 res=0;
  if (!st) return COLOR_INVALID;
  st2=MAlloc(StrLen(st)+1);
  StrFirstRem(st,str2color_lst,st2);
  res.c0=Str2ColorU16(st2);
  if (*st) {
    res.c1=Str2ColorU16(st);
    res|=ROPF_DITHER;
  }
  if (res.c0.color==COLOR_INVALID||res.c1.color==COLOR_INVALID)
    return COLOR_INVALID;
  else
    return res;
}
