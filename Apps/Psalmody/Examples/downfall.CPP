//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  music.tempo=2.5;
  while (TRUE) {
    Play("et2G32G3Gq2G3qCq2G3sFAFAq2G3etGDAqA",
      "Don't \0 \0need \0 \0help.  \0 \0Got \0 \0it \0 \0all \0figured \0 \0 \0out.\n\0");
    Play("et2G32G3Gq2G3qCq2G3sFAFAq2G3etGDAqA",
      "Don't \0 \0need \0 \0help.  \0 \0Got \0 \0it \0 \0all \0figured \0 \0 \0out.\n\0");
    Play("etCCCq2G3qDeDFetCF2G3qBeGFsAGAG");
    Play("etCCCq2G3qDeDFetCF2G3qBeGFsAGAG");
  }
}

Song;
