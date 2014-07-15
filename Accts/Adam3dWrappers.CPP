//Change anything you want in this
//acct file or others.

#help_index "Cmd Line (Typically)"

//These are customized quick commands.
//Feel free to add more or change.
public I64 F(U8 *needle_str,U8 *fu_flags=NULL)
{//Find (Grep) all text files.
  return Grep(needle_str,"/*",fu_flags);
}

public I64 R(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find (Grep) and replace string with string in all files.
  return Grep(needle_str,"/*",fu_flags,replace_text);
}

public I64 FD(U8 *needle_str,U8 *fu_flags=NULL)
{//Find (Grep) text files in cur dir.
  return Grep(needle_str,"*",fu_flags);
}

public I64 RD(U8 *needle_str,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{//Find (Grep) and replace string with string in cur dir.
  return Grep(needle_str,"*",fu_flags,replace_text);
}
