U0 Song()
{//Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  try //Catch <CTRL-ALT-c> exit
    while (!ScanChar) {
      Play("4hEqDCBCDhE",
	    "God \0is \0a \0g\0od \0of \0love.\n\0");
      Play("3GqFEFhG4E",
	    " \0 \0 \0 \0 \0He \0");
      Play("qDCBCDhE3G",
	    "wat\0ches \0us \0from \0a\0bove.\n\0 \0");
      Play("qFEFhG4BqC",
	    " \0 \0 \0 \0Our \0world \0");
      Play("DhCeDChB3G",
	    "is\0n't \0al\0ways \0nice.\n\0 \0");
      Play("qFEFhG4EqD",
	    " \0 \0 \0 \0Be\0fore \0");
      Play("CBCDhE3G",
	    "you \0gr\0ipe \0think \0twice.\n\0 \0");
      Play("qFEFhG4EqD",
	    " \0 \0 \0 \0He \0wat\0");
      Play("CBCDhE3G",
	    "ches \0us \0from \0a\0bove.\n\0 \0");
      Play("qFEFhG4EqD",
	    " \0 \0 \0 \0He'll \0smack \0");
      Play("CBCDhE3G",
	    "you \0with \0out \0a \0glove.\n\0 \0");
      Play("qFEFhG4BqC",
	    " \0 \0 \0 \0Our \0world \0");
      Play("DhCeDChB3G",
	    "is\0n't \0al\0ways \0nice.\n\0 \0");
      Play("qFEFhG");
    }
  catch
    PutExcept;
  '\n';
  Snd(0);
}

Song;
