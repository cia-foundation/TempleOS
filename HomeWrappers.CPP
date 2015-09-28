//Place this file in /Home and change
//anything you want.

#help_index "Cmd Line (Typically)"

//These are customized quick commands.
//Feel free to add more or change.
public I64 F(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in all text files.
  return Find(needle_str,"/*",fu_flags);
}

public I64 R(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find text and replace in all text files.
  return Find(needle_str,"/*",fu_flags,replace_text);
}

public I64 FD(U8 *needle_str,U8 *fu_flags=NULL)
{//Find text in cur dir text files.
  return Find(needle_str,"*",fu_flags);
}

public I64 RD(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find text and replace in cur dir text files.
  return Find(needle_str,"*",fu_flags,replace_text);
}
