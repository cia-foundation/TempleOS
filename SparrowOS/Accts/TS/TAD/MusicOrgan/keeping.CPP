//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.151;
  music_stacatto_factor= 0.780;
  try {
  while (!ScanKey) {
	Play("W13eCsBBeA#BCDCBCsBBeA#BCDCB");
	Play("etDCBeCBCBABetDCBeCBCBAB");
	Play("!A2G!3sBG!B!GFC!FCBFBFetF!GAsFGF!G!qG!D",
		"I \0have \0kept \0 \0the \0 \0Lord's \0 \0day, \0 \0 \0 \0 \0 \0 \0 \0 \0all \0 \0 \0 \0day!\n\0 \0");
	Play("!eA2G!3sBG!B!GFC!FCBFBFetF!GAsFGF!G!qG!D",
		"O\0kay, \0 \0God... \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0where's \0 \0 \0is \0 \0my \0 \0pay?\n\0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
