//8 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  task=PopUpViewStr(
    "$$WW+H,1$$$LK-A,"BF:Matthew,6:11"$\n\n"
    "I find two things interesting.  Jesus asks for "
    "the bare minimum... can't ask for less than daily bread.  "
    "Also, He actually demands it.  I find that funny.\n\n"
    "$LK-A,"BF:Matthew,10:31"$\n\n"
    "I have a funny story.  I used to shoot birds with a BB-gun.  "
    "That passage didn't snd so comforting!!  How many sparrows "
    "am I worth?\n"
    );
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
  try {
  while (!ScanKey) {
	Play("3qGsB2G3B2G3eF2G3sGGGGeRFC2G3qFC",
		"Where's \0my \0 \0 \0next \0 \0 \0meal!!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("GsB2G3B2G3eF2G3sGGGGeRFC2G3qFC",
		"God, \0we \0 \0 \0had \0 \0a \0deal!!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("B2GsG3A2G3AeCCsCECEqACsGFGF",
		"Day \0old \0bread?\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qB2GsG3A2G3AeCCsCECEqACsGFGF",
		"'Dai\0ly' \0I \0 \0 \0 \0said.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
