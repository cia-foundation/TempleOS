U8 *BlogLocalDir(CDate cdt=0)
{
  static U8 dir[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(dir,BLOG_LOCAL_BASE "/%d%Z",
	ds.year,ds.mon-1,"ST_MONTHS");
  if (!FileFind(dir,,FUF_JUST_DIRS))
    MkDir(dir);
  return dir;
}

U8 *BlogLocalFile(CDate cdt=0)
{
  static U8 file[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(file,"%s/" PERSONAL_INITIALS "%02d%02d%02d.TXT.Z",
	BlogLocalDir(cdt),ds.year%100,ds.mon,ds.day_of_mon);
  return file;
}

U8 *BlogWebDir(CDate cdt=0)
{
  static U8 dir[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(dir,BLOG_WEB_BASE "/%d%Z",
	ds.year,ds.mon-1,"ST_MONTHS");
  return dir;
}

U8 *BlogWebFile(CDate cdt=0)
{
  static U8 file[STR_LEN];
  CDateStruct ds;
  if (!cdt) cdt=Now+local_time_offset;
  Date2Struct(&ds,cdt);
  StrPrint(file,"%s/" PERSONAL_INITIALS "%02d%02d%02d.TXT.Z",
	BlogWebDir(cdt),ds.year%100,ds.mon,ds.day_of_mon);
  return file;
}

public U8 *FileNameNumNext(U8 *files_find_mask,U8 *fmt)
{
  CDirEntry *tempde=FilesFind(files_find_mask);
  U8 *res=MStrPrint(fmt,FileCnt(tempde));
  DirTreeDel(tempde);
  return res;
}

U0 BlogDCImgWrite(CDC *dc,CDate cdt=0)
{
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

U0 BlogScreenShot()
{
  Bool old_cursor=DocCursor;
  CDC *dc1,*dc2;
  U8 *old_fp_draw_input_ptr=gr.fp_draw_input_ptr;
  gr.fp_draw_input_ptr=NULL;
  WinMgrSync(2);
  dc1=DCScreenCapture;
  dc2=DCExtract(dc1,0,FONT_HEIGHT,GR_WIDTH-1,GR_HEIGHT-1);
  BlogDCImgWrite(dc2);
  gr.fp_draw_input_ptr=old_fp_draw_input_ptr;
  DCDel(dc1);
  DCDel(dc2);
  DocCursor(old_cursor);
}

public U0 MakeWebSitePartial()
{
  MkDir("D:/Wb");
  MkDir("D:/Wb/Home");
  MkDir("D:/Wb/Home/Web");
  MkDir("D:/Wb/Home/Web/TAD");
  MkDir("D:/Wb/Home/Web/TAD/Blog");

  FreshenWebDir("/Home/Web");
  FreshenWebDir("/Home/Web/" PERSONAL_INITIALS);
  FreshenWebDir(BlogLocalDir+2);
  Drv('C');
}

public U0 Blog()
{
  if (!FileFind(BlogLocalFile,,FUF_JUST_FILES))
    Copy("~/Web/" PERSONAL_INITIALS "/DailyBlogHeader.TXT.Z",
	  BlogLocalFile);
  AutoComplete;
  WinBorder;
  WinMax;
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN|SCF_CTRL);
  Msg(MSG_KEY_DOWN_UP,0,SC_F6|SCF_ALT);
  Msg(MSG_KEY_DOWN_UP,'\n',0);
  if (Ed(BlogLocalFile,EDF_WAS_WRITE)) {
    Copy(BlogLocalFile,"::" DAILY_BASE ".TXT.Z");
    Copy(BlogLocalFile,"D:" DAILY_BASE ".TXT.Z");
    Copy(BlogLocalFile,"D:/Wb" DAILY_BASE ".TXT.Z");
    TOSWebFile("D:/Wb" DAILY_BASE ".TXT.Z");
  }
}
