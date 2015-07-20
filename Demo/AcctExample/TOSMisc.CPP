#define VIDEO_FRAME_RATE	8

#define FG_JUST_AUDIO	0
#define FG_BOOT_DRV	1
#define FG_RAM_DRV	2
I64	fg_mode=FG_RAM_DRV;
Bool	fg_on =FALSE;

U0 FrameGrabberTask(I64)
{//ffmpeg -r 8 -i VID%05d.BMP -y Movie.avi
  I64 frame_num=0;
  U8  buf[STR_LEN],buf2[STR_LEN];
  F64 end_time=tS;
  Silent;	//no output to screen
  if (fg_mode==FG_RAM_DRV) {
    MkDir("B:/Temp/ScreenShots");
    while (fg_on) {
      StrPrint(buf,"B:/Temp/ScreenShots/VID%05d.BMP",frame_num++);
      StrCpy(buf2,buf);
      BMPScreenCapture(buf);
      while (TRUE) {
	end_time+=1.0/VIDEO_FRAME_RATE;
	if (end_time<tS) {
	  StrPrint(buf,"B:/Temp/ScreenShots/VID%05d.BMP",frame_num++);
	  Copy(buf2,buf);
	} else
	  break;
      }
      SleepUntil(cnts.jiffies+(end_time-tS)*JIFFY_FREQ);
    }
  } else {
    MkDir("::/Temp/ScreenShots");
    while (fg_on) {
      StrPrint(buf,"::/Temp/ScreenShots/VID%05d.BMP.Z",frame_num++);
      StrCpy(buf2,buf);
      BMPScreenCapture(buf);
      while (TRUE) {
	end_time+=1.0/VIDEO_FRAME_RATE;
	if (end_time<tS) {
	  StrPrint(buf,"::/Temp/ScreenShots/VID%05d.BMP.Z",frame_num++);
	  Copy(buf2,buf);
	} else
	  break;
      }
      SleepUntil(cnts.jiffies+(end_time-tS)*JIFFY_FREQ);
    }
  }
}

I64 CopyVideo()
{
  I64 result=0;
  Del("D:/Temp/*.SND");
  if (fg_mode==FG_JUST_AUDIO)
    SndFileCreate("D:/Temp/Audio",0.015,0.0,WF_SQUARE,0.45,0.35,2.5);
  else {
    SndFileCreate("D:/Temp/Audio",0.23,0.0,WF_SQUARE,0.45,0.35);
    DelTree("D:/Temp/ScreenShots");
    if (fg_mode==FG_RAM_DRV) {
      result=CopyTree("B:/Temp/ScreenShots","D:/Temp/ScreenShots");
      "Unused Space: %12.6fMeg\n",UnusedDrvSpace('B')/1024.0/1024.0;
    } else {
      result=CopyTree("::/Temp/ScreenShots","D:/Temp/ScreenShots");
      Unzip("D:/Temp/ScreenShots/*.Z");
    }
    "$$BK,1$$Files Copied:%d$$BK,0$$ Time:%8.3fm\n",
	  result,result/60.0/VIDEO_FRAME_RATE;
  }
  return result;
}

public U0 FrameGrabberToggle()
{
  static F64 last_time=0;
  if (tS-last_time>3.0) {
    last_time=tS;
    if (fg_on) {
      fg_on=FALSE;
      Snd(0);
      wall->record=snd.record=FALSE;
      Sleep(1000);
      CopyVideo;
    } else {
      fg_on=TRUE;
      if (fg_mode!=FG_JUST_AUDIO)
	Spawn(&FrameGrabberTask,NULL,"Frame Grabber");
      wall->record=snd.record=TRUE;
      Snd(0);
    }
  }
}

public U0 JukeLines(I64 start_line,I64 end_line)
{
  AutoComplete;
  if (!fg_on)
    FrameGrabberToggle;
  JukeSongsPuppet("::/Apps/Psalmody/Examples",,start_line*5,end_line*5);
  if (fg_on)
    FrameGrabberToggle;
}

public U0 ChkAll()
{
  U8 *ptr=TOS_HDS;
  while (*ptr)
    ChkDsk(*ptr++);
}

public CDoc *DC2Doc(CDC *dc,I64 dx=0,I64 dy=0,I64 *_total_score=NULL)
{
  U8 byte_bit_cnts[256];
  I64 i,j,*ptr,row,col,ch,best_ch,score,best_score,
	cur_char_image,diff_image,total_score=0;
  CDoc *doc=DocNew;

  MemSet(byte_bit_cnts,0,sizeof(byte_bit_cnts));
  for (i=0;i<256;i++)
    for (j=0;j<7;j++)
      if (Bt(&i,j))
	byte_bit_cnts[i]++;

  for (row=0;row<dc->height/FONT_HEIGHT;row++) {
    for (col=0;col<dc->width/FONT_WIDTH;col++) {

      cur_char_image=0;
      for (i=0;i<FONT_HEIGHT;i++)
	for (j=0;j<FONT_WIDTH;j++)
	  if (GrPeek(dc,col*FONT_WIDTH+j+dx,row*FONT_HEIGHT+i+dy)!=WHITE)
	    LBts(&cur_char_image,i*8+j);

      best_score=MAX_I64;
      best_ch=0;
      ptr=&text.font[32];
      for (ch=32;ch<127;ch++) {
	diff_image=*ptr++ ^ cur_char_image;
	score=0;
	for (i=0;i<8;i++)
	  score+=byte_bit_cnts[diff_image.u8[i]];
	if (score<best_score) {
	  best_score=score;
	  best_ch=ch;
	}
      }
      if (best_ch=='$$')
	DocPrint(doc,"$$$$");
      else
	DocPrint(doc,"%c",best_ch);
      total_score+=best_score;
    }
    DocPrint(doc,"\n");
  }
  if (_total_score) *_total_score=total_score;
  return doc;
}

public U0 BMP2TXT(U8 *_in_name,U8 *_out_name=NULL)
{
  U8 *in_name,*out_name;
  I64 dx,dy,score,best_score=MAX_I64;
  CDC *dc;
  CDoc *doc;
  in_name=DftExt(_in_name,"BMP"),
	if (_out_name)
  out_name=DftExt(_out_name,"TXT.Z");
  else
  out_name=ChgExt(_in_name,"TXT.Z");
  if (dc=BMPRead(in_name)) {
    for (dy=-3;dy<=4;dy++)
      for (dx=-3;dx<=4;dx++) {
	doc=DC2Doc(dc,dx,dy,&score);
	if (score<best_score) {
	  best_score=score;
	  StrCpy(&doc->filename.name,out_name);
	  DocWrite(doc);
	}
	DocDel(doc);
      }
  }
  Free(in_name);
  Free(out_name);
}

U0 DblSpaceScanFile(U8 *filename)
{
  CDoc *doc=DocRead(filename);
  I64 i;
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_NEW_LINE &&
	  doc_e->next->type_u8==DOCT_NEW_LINE &&
	  doc_e->next->next->type_u8==DOCT_NEW_LINE) {
      i=0;
      while (doc_e->type_u8==DOCT_NEW_LINE) {
	i++;
	doc_e=doc_e->next;
      }
      "%02d:",i;
      PutFileLink(doc->filename.name,doc_e->y);
      '\n';
    } else
      doc_e=doc_e->next;
  }
  DocDel(doc);
}

U0 DblSpaceScanLst(CDirEntry *tempde)
{
  CDirEntry *tempde1;
  while (tempde) {
    tempde1=tempde->next;
    if (tempde->attr & RS_ATTR_DIR) {
      if (tempde->sub) {
	"Scanning Directory: %s\n",tempde->full_name;
	DblSpaceScanLst(tempde->sub);
      }
    } else
      DblSpaceScanFile(tempde->full_name);
    DirEntryDel(tempde);
    tempde=tempde1;
  }
}

public U0 DblSpaceScan(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{
  I64 f=0;
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),"+r+T");
  ScanFlags(&f,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  DblSpaceScanLst(FilesFind(files_find_mask,f));
}

RegSetDftEntry("TempleOS/TOS",
	"I64 tos_code_walk_thru=1,tos_after_egypt_in_action=1;\n");
RegExeBranch("TempleOS/TOS");

U0 TOSRegInit()
{
  DefinePrint("DD_CODE_WALK_THRU","Episode #%d",tos_code_walk_thru);
  DefinePrint("DD_AFTER_EGYPT_IN_ACTION","Episode #%d",
	tos_after_egypt_in_action);
} TOSRegInit;

U0 TOSRegWrite()
{
  RegWriteBranch("TempleOS/TOS",
	"I64 tos_code_walk_thru=%d,tos_after_egypt_in_action=%d;\n",
	tos_code_walk_thru,tos_after_egypt_in_action);
  TOSRegInit;
}

class CYouTubeForm2
{
  U8 special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"SerialNum:%s\"$$\n";
};

U8 *TOSYouTube()
{
  U8 *result;
  CYouTubeForm2 *url=CAlloc(sizeof(CYouTubeForm2));
  if (DocForm(url(CYouTubeForm2 *)) && *url->special)
    result=MStrPrint("$$HC,\"<object width=\\\"640\\\" height=\\\"520\\\">"
	  "<param name=\\\"movie\\\" "
	  "value=\\\"http://www.youtube.com/v/%s\\\"></param><param "
	  "name=\\\"allowscriptaccess\\\" value=\\\"always\\\"></param><embed "
	  "src=\\\"http://www.youtube.com/v/%s\\\" "
	  "type=\\\"application/x-shockwave-flash\\\" "
	  "allowscriptaccess=\\\"always\\\" "
	  "width=\\\"640\\\" height=\\\"520\\\"></embed></object>\"$$",
	  url->special,url->special);
  else
    result=NULL;
  Free(url);
  return result;
}

U0 AfterEgyptInAction()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Adam("GodInit(\"/Adam/God/Vocab.TXT*\");");
  AutoComplete;
  WinMax;
  WinBorder;
  if (!fg_on)
    PopUp("Sleep(50);FrameGrabberToggle;");
  Ed("::/Home/Archive/AfterEgypt.TXT.Z");
  Msg(MSG_KEY_DOWN_UP,CH_SPACE,0x3900000039); //<SPACE>
  ExeFile("::/Apps/AfterEgypt/Run");
  SettingsPop;
  tos_after_egypt_in_action++;
  TOSRegWrite;
}

U0 CodeWalkThru()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinMax;
  WinBorder;
  if (!fg_on)
    PopUp("Sleep(50);FrameGrabberToggle;");
  Ed("::/Home/Archive/RandomCode.TXT.Z");
  SettingsPop;
  Msg(MSG_KEY_DOWN_UP,'b',0x83000000830); //<ALT-b>
  tos_code_walk_thru++;
  TOSRegWrite;
}

U0 TOSJukeVideo(I64 start=0,I64 end=10)
{
  U8 *buf=FileRead("::/Misc/Bible.TXT.Z"); //Put in cache
  Free(buf);
  AutoComplete;
  FrameGrabberToggle;
  JukeSongsPuppet("::/Apps/Psalmody/Examples",,start,end);
  FrameGrabberToggle;
}
