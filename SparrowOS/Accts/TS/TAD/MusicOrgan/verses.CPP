//8 special
U8 *bible;
I64 size;

U0 TaskEndCB()
{
  Snd(0);
  Free(bible);
  Exit;
}

#define LEN 256

U0 PutVerse()
{
  U8 *ch;
  I64 *start;

  "$$CL$$";
  start=GetTimeStamp/16%size;
  if (start>size-LEN) start=size-LEN;
  ch=bible[start+LEN-1];
  bible[start+LEN-1]=0;
  "" bible+start;
  bible[start+LEN-1]=ch;
}

U0 Song()
{
  U8 *bible_filename=Define("ST_BIBLE_FILE");
  Fs->task_end_cb=&TaskEndCB;
  music_tempo=2.5;
  bible=FileRead(bible_filename,&size);
  "$$WW+H,1$$";
  try {
  while (!ScanKey) {
	PutVerse;
	Play("3qAeABCBqCetABBqCetAAREE2G");
	Play("3qAeABCBqCetABBqCetAAREE2G");
	Play("3eDRRRqGReRRGGGGqE");
	Play("eDRRRqGReRRGGGGqE");
  }
  } catch
    CatchAll;
  Snd(0);
  Free(bible);
}

Song;
