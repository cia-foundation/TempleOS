#help_index "Misc/TOS"

#define VIDEO_FRAME_RATE	29.97

#define FG_JUST_AUDIO	0
#define FG_BOOT_DRV	1
#define FG_RAM_DRV	2
I64	fg_mode=FG_RAM_DRV;
Bool	fg_on =FALSE;

U0 FrameGrabberTask(I64)
{//#!/bin/bash
//for f in ./ScrnShots/*.Z; do ./Bin/TOSZ $$f; done
  //ffmpeg -r 8 -i VID%05d.BMP -y Movie.avi
  I64 frame_num=0;
  U8  buf[STR_LEN],buf2[STR_LEN],*st_dir;
  F64 end_time=tS;
  Silent;	//no output to scrn
  if (fg_mode==FG_RAM_DRV)
    st_dir="B:/Tmp/ScrnShots";
  else
    st_dir="::/Tmp/ScrnShots";
  MkDir(st_dir);
  while (fg_on) {
    StrPrint(buf,"%s/VID%05d.BMP.Z",st_dir,frame_num++);
    StrCpy(buf2,buf);
    BMPScrnCapture(buf);
    while (TRUE) {
      end_time+=1.0/VIDEO_FRAME_RATE;
      if (end_time<tS) {
	StrPrint(buf,"%s/VID%05d.BMP.Z",st_dir,frame_num++);
	Copy(buf2,buf);
      } else
	break;
    }
    SleepUntil(cnts.jiffies+(end_time-tS)*JIFFY_FREQ);
  }
}

I64 CopyVideo()
{
  I64 res=0;
  Del("D:/Tmp/*.SND");
  if (fg_mode==FG_JUST_AUDIO)
    SndFileCreate("D:/Tmp/Audio",0.115,0.0,WF_SQUARE,0.45,0.35,2.5);
  else {
    SndFileCreate("D:/Tmp/Audio",0.115,0.0,WF_SQUARE,0.45,0.35);
    DelTree("D:/Tmp/ScrnShots");
    if (fg_mode==FG_RAM_DRV) {
      res=CopyTree("B:/Tmp/ScrnShots","D:/Tmp/ScrnShots");
      "Unused Space: %12.6fMeg\n",UnusedDrvSpace('B')/1024.0/1024.0;
    } else
      res=CopyTree("::/Tmp/ScrnShots","D:/Tmp/ScrnShots");
    "$$BK,1$$Files Copied:%d$$BK,0$$ Time:%8.3fm\n",
	  res,res/60.0/VIDEO_FRAME_RATE;
  }
  return res;
}

public U0 FrameGrabberToggle()
{//The frame grabber saves BMP files to D:/Tmp/ScrnShots.
  static F64 last_time=0;
  if (tS-last_time>3.0) {
    last_time=tS;
    if (fg_on) {
      fg_on=FALSE;
      Snd;
      wall->record=snd.record=FALSE;
      Sleep(1000);
      User("CopyVideo;Exit;\n");
    } else {
      fg_on=TRUE;
      if (fg_mode!=FG_JUST_AUDIO)
	Spawn(&FrameGrabberTask,NULL,"Frame Grabber");
      wall->record=snd.record=TRUE;
      Snd;
    }
  }
}

public U0 JukeSong(I64 num)
{//Make movie of one song.
  AutoComplete;
  if (!fg_on)
    FrameGrabberToggle;
  JukeSongsPuppet("~/Sup1/Sup1Hymns",,num,num+1);
  if (fg_on)
    FrameGrabberToggle;
}

public U0 JukeLines(I64 start_line,I64 end_line)
{//Make movie of many lines of songs.
  AutoComplete;
  if (!fg_on)
    FrameGrabberToggle;
  JukeSongsPuppet("~/Sup1/Sup1Hymns",,start_line*5,end_line*5);
  if (fg_on)
    FrameGrabberToggle;
}

public U0 ChkAll()
{//ChkDsk on C & D.
  U8 *ptr=TOS_HDS;
  while (*ptr)
    ChkDsk(*ptr++);
}

public CDoc *DC2Doc(CDC *dc,I64 dx=0,I64 dy=0,I64 *_total_score=NULL)
{//Use OCR to make a text DolDoc from CDC.
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

      best_score=I64_MAX;
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

public U0 BMP2Doc(U8 *_in_name,U8 *_out_name=NULL)
{//Use OCR to make a text DolDoc from BMP file.
  U8 *in_name,*out_name;
  I64 dx,dy,score,best_score=I64_MAX;
  CDC *dc;
  CDoc *doc;
  in_name=DftExt(_in_name,"BMP");
  if (_out_name)
    out_name=DftExt(_out_name,"DD.Z");
  else
    out_name=ChgExt(_in_name,"DD.Z");
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

U0 TOSTheme()
{
  if (!fg_on)
    PopUp("Sleep(50);FrameGrabberToggle;");
  ExeFile("~/Sup1/Sup1Graphics/TOS/TOSTheme.HC");
  DocClear;
}

U0 TOSJukeVideo(I64 start=0,I64 end=10)
{
  U8 *buf=FileRead("::/Misc/Bible.TXT.Z"); //Put in cache
  Free(buf);
  AutoComplete;
  FrameGrabberToggle;
  JukeSongsPuppet("~/Sup1/Sup1Hymns",,start,end);
  FrameGrabberToggle;
}

#define MEM_TEST_SIZE	1024*1024
U0 MemTest()
{
  U8 *b;
  while (sys_data_bp->alloced_u8s-sys_data_bp->used_u8s>0x1000000) {
    b=MAlloc(MEM_TEST_SIZE,Fs->data_heap);
    MemSet(b,0x88,MSize(b));
    "Data:%X\n",sys_data_bp->alloced_u8s-sys_data_bp->used_u8s;
    Yield;
  }
  while (sys_code_bp->alloced_u8s-sys_code_bp->used_u8s>0x1000000) {
    b=MAlloc(MEM_TEST_SIZE,Fs->code_heap);
    MemSet(b,0x88,MSize(b));
    "Code:%X\n",sys_code_bp->alloced_u8s-sys_code_bp->used_u8s;
    Yield;
  }
}

public I64 Profanity(
	U8 *needle_filename="~/Sup1/Sup1Blog/Profanity.DD.Z",
	U8 *files_find_mask="/*",U8 *fu_flags=NULL)
{//Scan for profanity.
  CCmpCtrl *cc;
  I64 res=0;
  if (FileFind(needle_filename)) {
    cc=CmpCtrlNew(MStrPrint("#include \"%s\"",needle_filename));
    while (Lex(cc))
      if (cc->token==TK_IDENT)
	res+=Find(cc->cur_str,files_find_mask,fu_flags);
    CmpCtrlDel(cc);
  }
  return res;
}

U0 TOSChgLog2()
{
  InStr("\"Ed(\\\"::/Doc/ChgLog.DD.Z\\\");\n\";
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_UP|SCF_CTRL);
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Msg(MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Msg(MSG_KEY_DOWN_UP,CH_CTRLY,0);
  Msg(MSG_KEY_DOWN_UP,0,SC_F6|SCF_ALT);
  Msg(MSG_KEY_DOWN_UP,CH_ESC,0);"
  "\"Exit;\n\";");
}

public U0 TOSChgLog()
{//Opdate ChgLog file.
  User("TOSChgLog2;\n");
}
