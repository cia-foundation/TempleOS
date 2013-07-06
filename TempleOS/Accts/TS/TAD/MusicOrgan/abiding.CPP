//9 has graphics

#define X	330
#define Y	15

#define NUM_BEAMS	256

F64 beams_r[NUM_BEAMS],beams_é[NUM_BEAMS];

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 r,x1,y1,x2,y2;
  dc->color=YELLOW;
  GrCircle(dc,X,Y,10);
  GrFloodFill(dc,X,Y,10);
  dc->color=BROWN;
  GrCircle(dc,X,Y,10);
  dc->color=YELLOW;
  for (i=0;i<NUM_BEAMS;i++) {
    r=125.0*((0.25*tMBeat+beams_r[i])%1.0);
    P2R(&x1,&y1,r     ,beams_é[i]);
    P2R(&x2,&y2,r+10.0,beams_é[i]);
    GrLine(dc,x1+X,y1+Y,x2+X,y2+Y);
  }
}

U0 Song()
{
  CTask *task;
  I64 i;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  for (i=0;i<NUM_BEAMS;i++) {
    beams_r[i]=Rand;
    beams_é[i]=2*ã*Rand;
  }
  try {
    task=PopUpViewStr(
    "$LK,"Numbers,6:25","BF:Numbers,6:25"$\n"
    "$LK,"Psalms,31:16","BF:Psalms,31:16"$\n"
    "$LK,"Psalms,67:1","BF:Psalms,67:1"$\n"
    "$LK,"Psalms,80:3","BF:Psalms,80:3"$\n"
    "$LK,"Psalms,80:19","BF:Psalms,80:19"$\n"
    "$LK,"Psalms,119:135","BF:Psalms,119:135"$\n"
    "$LK,"Daniel,9:17","BF:Daniel,9:17"$\n"
    );
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    "$$BG+H,LTCYAN$$$$FD+H,BLACK$$";
    while (!ScanKey) {
      "$$FG,WHITE$$To God...\n$$FG$$";
      Play("3etCFEsDDDDD2G3D2G3qC2eG3EqEetAAF2qG",
      "I \0am \0a\0biding \0 \0 \0 \0in \0 \0Your \0 \0presence.\n\0 \0 \0 \0 \0 \0 \0 \0");
      Play("3etCFEsDDDDD2G3D2G3qC2eG3EqEetAAF2qG",
      "I \0am \0a\0biding \0 \0 \0 \0in \0 \0Your \0 \0presence.\n\0 \0 \0 \0 \0 \0 \0 \0");
      Play("3DBsFGFG2eG3FRDqEGeBE",
      "What \0a \0joy!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("qDBsFGFG2eG3FRDqEGeBE",
      "I'm \0so \0coy!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
    } catch
      CatchAll;
  SettingsPop;
}

Song;
