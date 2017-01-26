#help_index "Misc/Blog"

public U8 *BlogLocalDir(CDate cdt=0)
{//MAlloc dirname of blog for date.
  static U8 dir[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(dir,BLOG_LOCAL_BASE "/%d%Z",ds.year,ds.mon-1,"ST_MONTHS");
  if (!FileFind(dir,,FUF_JUST_DIRS))
    DirMk(dir);
  return dir;
}

public U8 *BlogLocalFile(CDate cdt=0)
{//MAlloc filename of blog for date.
  static U8 file[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(file,"%s/" INS_REG_PERSONAL_INITIALS "%02d%02d%02d.DD.Z",
	BlogLocalDir(cdt),ds.year%100,ds.mon,ds.day_of_mon);
  return file;
}

public U8 *BlogWebDir(CDate cdt=0)
{//MAlloc D:/Wb/dirname of blog for date.
  static U8 dir[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(dir,BLOG_WEB_BASE "/%d%Z",
	ds.year,ds.mon-1,"ST_MONTHS");
  return dir;
}

public U8 *BlogWebFile(CDate cdt=0)
{//MAlloc D:/Wb/filename of blog for date.
  static U8 file[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(file,"%s/" INS_REG_PERSONAL_INITIALS "%02d%02d%02d.DD.Z",
	BlogWebDir(cdt),ds.year%100,ds.mon,ds.day_of_mon);
  return file;
}

public U8 *FileNameNumNext(U8 *files_find_mask,U8 *fmt)
{//Cnt files in a mask. Create next filename.
  CDirEntry *tmpde=FilesFind(files_find_mask);
  U8 *res=MStrPrint(fmt,FileCnt(tmpde));
  DirTreeDel(tmpde);
  return res;
}

public U0 BlogDCImgWrite(CDC *dc,CDate cdt=0)
{//Add dc to blog as BMP file.
  U8 file_mask[STR_LEN],file_name[STR_LEN],*st1,*st2;
  if (!cdt) cdt=Now+local_time_offset;
  StrPrint(file_mask,"%s/Img*",BlogLocalDir(cdt));
  StrPrint(file_name,"%s/Img%%03d.BMP",BlogLocalDir(cdt));
  st1=FileNameNumNext(file_mask,file_name);
  st2=MStrPrint("http://www.templeos.org/Wb%s",st1+2);
  BMPWrite(st1,dc);
  Free(st1);
  "$$HC,\"<img src=\\\"%s\\\" width=\\\"%d\\\" "
	"height=\\\"%d\\\" alt=\\\"\\\">\"$$\n",st2,dc->width,dc->height;
  Free(st2);
}

public U0 BlogScrnShot()
{//Add scrn shot to blog as BMP file.
  Bool old_cursor=DocCursor,old_silent;
  CDC *dc1,*dc2;
  U8 *old_fp_draw_ms=gr.fp_draw_ms;
  gr.fp_draw_ms=NULL;
  Refresh(2);
  dc1=DCScrnCapture;
  dc2=DCExt(dc1,0,FONT_HEIGHT,GR_WIDTH-1,GR_HEIGHT-1);
  BlogDCImgWrite(dc2);
  gr.fp_draw_ms=old_fp_draw_ms;
  DCDel(dc1);
  DCDel(dc2);
  DocCursor(old_cursor);
  old_silent=Silent;
  FreshenBlog;
  Silent(old_silent);
}

public U0 FreshenBlog()
{//Regen html files for D:/Home/Web.
  DirMk("D:/Wb");
  DirMk("D:/Wb/Home");
  DirMk("D:/Wb/Home/Web");
  DirMk("D:/Wb/Home/Web/" INS_REG_PERSONAL_INITIALS);
  DirMk("D:/Wb/Home/Web/" INS_REG_PERSONAL_INITIALS "/BlogDir");

  FreshenWebDir("/Home/Web");
  FreshenWebDir("/Home/Web/" INS_REG_PERSONAL_INITIALS);
  FreshenWebDir(BlogLocalDir+2);
  Drv('C');
}

public U0 Blog(Bool god_misc2=FALSE)
{//Make entry in blog.
  if (!FileFind(BlogLocalFile,,FUF_JUST_FILES))
    Copy("~/Web/" INS_REG_PERSONAL_INITIALS "/DailyBlogHeader.DD.Z",
	  BlogLocalFile);
  AutoComplete;
  WinBorder;
  WinMax;
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN|SCF_CTRL);
  if (god_misc2)
    Msg(MSG_KEY_DOWN_UP,0,SC_F7|SCF_SHIFT|SCF_ALT);
  else {
    Msg(MSG_KEY_DOWN_UP,0,SC_F6|SCF_ALT);
    Msg(MSG_KEY_DOWN_UP,'\n',0);
  }
  if (Ed(BlogLocalFile,EDF_WAS_WRITE)) {
    DocOpt(BlogLocalFile,"-r");
    Copy(BlogLocalFile,"::" DAILY_BASE ".DD.Z");
    Copy(BlogLocalFile,"D:" DAILY_BASE ".DD.Z");
    Copy(BlogLocalFile,"D:/Wb" DAILY_BASE ".DD.Z");
    TOSWebFile("D:/Wb" DAILY_BASE ".DD.Z");
  }
}
