#help_index "Misc/TOS"

Bool	fg_on =FALSE;

I64 CopyVideo()
{
  I64 res=0;
  Bool old_silent=Silent;
  Del("D:/Tmp/*.AU");
  Del("D:/Tmp/*.MV");
  Silent(old_silent);
  SndShift(&scrncast.snd_head,0.185);
  AUWrite("D:/Tmp/AUDIO",&scrncast.snd_head,scrncast.t0_now,scrncast.t0_tS);
  QueDel(&scrncast.snd_head,TRUE);
  GR2MV("D:/Tmp/VID%03d.MV","B:/Tmp","+d");
  return res;
}

#help_index "ScrnCast/TOS"
U0 DelScrnShots()
{
  Bool old_silent=Silent;
  DelTree("B:/Tmp");
  DirMk("B:/Tmp");
  Del("D:/Tmp/*.AU");
  Del("D:/Tmp/*.MV");
  Del("C:/Tmp/*.AU");
  Del("C:/Tmp/*.MV");
  Silent(old_silent);
}

public U0 FrameGrabberToggle(Bool sync_tone,Bool tos_theme,
	Bool just_audio=FALSE)
{//The frame grabber saves GR files to B:/Tmp.
  static F64 last_time=0;
  if (tS-last_time>3.0) {
    last_time=tS;
    if (fg_on) {
      fg_on=FALSE;
      ScrnCast(OFF);
      User("CopyVideo;Exit;\n");
    } else {
      DelScrnShots;
      fg_on=TRUE;
      ScrnCast(ON,just_audio);
      if (sync_tone) {Beep;}
      if (tos_theme) {User("ExeFile(\"~/TOS/TOSTheme\");Exit;\n");}
    }
  }
}

public U0 JukeSongTAD(I64 num,I64 passes=2)
{//Make movie of one song.
  if (!fg_on)
    FrameGrabberToggle(FALSE,FALSE);
  Sleep(200);
  JukeSongsPuppet("~/TAD/Songs",passes,num,num+1);
  if (fg_on)
    FrameGrabberToggle(FALSE,FALSE);
}

public U0 JukeSongSup(I64 vol,I64 num,I64 passes=2)
{//Make movie of one song.
  U8 *st=MStrPrint("~/Sup%d/Sup%dHymns",vol,vol);
  if (!fg_on)
    FrameGrabberToggle(FALSE,FALSE);
  JukeSongsPuppet(st,passes,num,num+1);
  if (fg_on)
    FrameGrabberToggle(FALSE,FALSE);
  Free(st);
}

public U0 JukeLines(I64 vol,I64 start_line,I64 end_line)
{//Make movie of many lines of songs, starting at 0.
  U8 *st=MStrPrint("~/Sup%d/Sup%dHymns",vol,vol);
  if (!fg_on)
    FrameGrabberToggle(FALSE,FALSE);
  JukeSongsPuppet(st,,start_line*5,end_line*5);
  if (fg_on)
    FrameGrabberToggle(FALSE,FALSE);
  Free(st);
}

public U0 TADHymns(I64 vol,I64 let)
{//Make 2-lines of songs movie
  I64 line=2*(ToUpper(let)-'A');
  JukeLines(vol,line,line+2);
}

#help_index "Misc/TOS"
public U0 DskChkAll()
{//DskChk on C & D.
  U8 *ptr=TOS_HDS;
  while (*ptr) {
    "DskChk('%c')\n",*ptr;
    DskChk(*ptr++,TRUE);
  }
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
