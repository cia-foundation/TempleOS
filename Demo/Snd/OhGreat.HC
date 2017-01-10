U0 Song()
{//Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  try //Catch <CTRL-ALT-c> exit
    while (!ScanChar) {
      Play("6hEqDC5B6CDhE",
	    "God \0is \0a \0g\0od \0of \0love.\n\0");
      Play("5GqFEFhG6E",
	    " \0 \0 \0 \0 \0He \0");
      Play("6qDC5B6CDhE5G",
	    "wat\0ches \0us \0from \0a\0bove.\n\0 \0");
      Play("5qFEFhGB6qC",
	    " \0 \0 \0 \0Our \0world \0");
      Play("6DhCeDC5hBG",
	    "is\0n't \0al\0ways \0nice.\n\0 \0");
      Play("5qFEFhG6EqD",
	    " \0 \0 \0 \0Be\0fore \0");
      Play("6C5B6CDhE5G",
	    "you \0gr\0ipe \0think \0twice.\n\0 \0");
      Play("5qFEFhG6EqD",
	    " \0 \0 \0 \0He \0wat\0");
      Play("6C5B6CDhE5G",
	    "ches \0us \0from \0a\0bove.\n\0 \0");
      Play("5qFEFhG6EqD",
	    " \0 \0 \0 \0He'll \0smack \0");
      Play("6C5B6CDhE5G",
	    "you \0with \0out \0a \0glove.\n\0 \0");
      Play("5qFEFhGB6qC",
	    " \0 \0 \0 \0Our \0world \0");
      Play("6DhCeDC5hBG",
	    "is\0n't \0al\0ways \0nice.\n\0 \0");
      Play("5qFEFhG");
    }
  catch
    PutExcept;
  '\n';
  Snd;
}

Song;
