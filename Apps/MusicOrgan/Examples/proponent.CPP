//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  while (TRUE) {
    Play("qCqAqAsEDEDqBqBetGGEs2G3C2G3C");
    Play("qCqAqAsEDEDqBqBetGGEs2G3C2G3C");
    Play("etCD2G3etEE2G3qFqFqAsACACsAFAFetECC",
      "In \0my \0 \0corn\0 \0 \0er? \0 \0 \0 \0 \0 \0 \0Hope \0we're \0right.\n\0");
    Play("etCD2G3etEE2G3qFqFqAsACACsAFAFetECC",
      "In \0my \0 \0corn\0 \0 \0er? \0 \0 \0 \0 \0 \0 \0Hope \0we're \0right.\n\0");
  }
}

Song;
