//Change anything you want in this
//acct file or others.

#help_index "Utils;Cmd Line (Typically)"

//These are customized quick commands.
//Feel free to add more or change.
public I64 F(U8 *pattern,U8 *fu_flags=NULL)
{
  return Grep(pattern,"/TempleOS/"FILENAME_MASK_TXT,fu_flags);
}

public I64 R(U8 *pattern,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{
  return Grep(pattern,"/TempleOS/"FILENAME_MASK_TXT,fu_flags,replace_text);
}

public I64 FD(U8 *pattern,U8 *fu_flags=NULL)
{
  return Grep(pattern,FILENAME_MASK_TXT,fu_flags);
}

public I64 RD(U8 *pattern,U8 *replace_text=NULL,U8 *fu_flags="+l-i")
{
  return Grep(pattern,FILENAME_MASK_TXT,fu_flags,replace_text);
}
#help_index ""
