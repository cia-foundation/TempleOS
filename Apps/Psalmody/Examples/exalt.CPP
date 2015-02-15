//9 has graphics
U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,cx=0.85*task->pix_width,cy=task->pix_height+10;
  dc->pen_width=5;
  for (i=2;i<=7;i++) {
    dc->color=gr_rainbow_10[9-i];
    GrCircle3(dc,cx,cy,0,(i+5)*5);
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      DocClear;
      "$$BG,LTCYAN$$$$BLACK$$";
      Play("3eEDsBABA2qG3etGECFDBECFCBReBE",
	  "Come \0and \0sing \0 \0a\0 \0long.  \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0\n\0 \0 \0 \0");
      Play("EDsBABA2qG3etGECFDBECFCBReBE",
	  "Of\0fer \0praise \0 \0to \0 \0God.  \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0\n\0 \0 \0 \0");
      Play("qBeDF2qG3eFRqBetFFGeACDF",
	  "Up \0in \0His \0hea\0ven \0 \0He \0is \0 \0 \0glor\0i\0ous.\n\0 \0");
      Play("qBeDF2qG3eFRqBetFFGeACDF",
	  "Break \0in\0to \0song.  \0 \0 \0Strike-\0up \0 \0 \0the \0chor\0us.\n\0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
