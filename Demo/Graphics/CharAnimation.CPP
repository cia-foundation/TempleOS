U8 *old_font=text.font;

// See $LK,"::/Kernel/Font8x8.CPP"$, $LK,"::/Demo/ScreenCodes.CPP"$,
//$LK,"::/Demo/ExtendedChars.CPP"$, and $LK,"::/Demo/Graphics/FontEd.CPP"$.

U64 waves[4]={
0x0011AA440011AA44,0x0022558800225588,
0x0044AA110044AA11,0x0088552200885522};

U0 AnimateEndCB()
{
  text.font=old_font;
  Exit;
}

U0 AnimateTask(I64)
{
  I64 i;
  U64 *font=MAlloc(256*8);
  Fs->task_end_cb=&AnimateEndCB;
  MemCpy(font,text.font,256*8);
  text.font=font;
  while (TRUE) {
    font[CH_SPACE]=waves[i++&0x3];
    Sleep(100);
  }
}

Spawn(&AnimateTask,NULL,"Animate",,Fs);
TaskRep;
