//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.500;
  music.stacatto_factor= 0.900;
  try {
    task=PopUpViewPrint(
	  "$$WW+H,1$$You get what you pray for.  "
	  "Most people are fools like King Midas.  People have laughable notions of Heaven.  "
	  "You'd be surprised -- I can't imagine things being much better than Earth "
	  "because we need a sense of purpose.  "
	  "Don't ask for stuff, let God pick, but He might get annoyed at that.  "
	  "God wants us to be happy.  He tries hard.  If you do get what you pray for, you damn "
	  "well better be happy, lil' sonfabitch!\n\n"
	  "$LK,"Ecclesiastes,3:13",A="BF:Ecclesiastes,3:13"$\n"
	  "$LK,"John,16:24",A="BF:John,16:24"$\n"
	  );
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    DocRecalc(DocPut(task)); //Reformat word-wrap with new window size.
    while (!ScanKey) {
      Play("3qBeCDqCAetAACeECsF2G3F2GqG",
	    "I'm \0on \0a \0mis\0sion \0from \0 \0 \0my \0 \0God.\n\0 \0 \0 \0 \0");
      Play("3BeCDqCAetAACeECsF2G3F2GqG",
	    "I'm \0on \0a \0mis\0sion \0from \0 \0 \0my \0 \0God.\n\0 \0 \0 \0 \0");
      Play("3etEDDDGABG2GeG3E2G3GqF2G3etGCD",
	    "He \0 \0 \0told \0 \0 \0me \0 \0 \0 \0to \0 \0be \0hap\0py.\n\0 \0 \0 \0");
      Play("EDDDGABG2GeG3E2G3GqF2G3etGCD",
	    "He \0 \0 \0said \0 \0 \0to \0 \0 \0 \0make \0 \0it \0snap\0py.\n\0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
