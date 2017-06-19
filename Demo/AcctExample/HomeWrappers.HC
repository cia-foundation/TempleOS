#help_index "Cmd Line (Typically)"

#define FILEMASK_OFF_LIMITS "!*/Misc/PCIDevices.DD*;!*/Misc/Bible.TXT*;"\
	"!*/Books?/*;!*/Sup1/Sup1Words/*;!*/Sup1/Sup1Bin/*;"\
	"!*/God/Vocab.DD*"

public I64 F(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in all text files.
  return Find(needle_str,"/*",fu_flags);
}

public I64 F2(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in most text files.
  return Find(needle_str,"/*;" FILEMASK_OFF_LIMITS,fu_flags);
}

public I64 R(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find text and replace in most text files.
  return Find(needle_str,"/*;" FILEMASK_OFF_LIMITS,fu_flags,replace_text);
}

public I64 FD(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in cur dir text files.
  return Find(needle_str,"*",fu_flags);
}

public I64 FD2(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in most cur dir text files.
  return Find(needle_str,"*;" FILEMASK_OFF_LIMITS,fu_flags);
}

public I64 RD(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find text and replace in most cur dir text files.
  return Find(needle_str,"*;" FILEMASK_OFF_LIMITS,fu_flags,replace_text);
}
