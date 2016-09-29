/*
Char graphics are how games on the C64 were made.  You don't need to do it
this way, unless for fun.  You can just make device context bigger than
the scrn and scroll around.

See  $LK,"::/Demo/Games/RawHide.HC"$ or $LK,"::/Demo/Games/BigGuns.HC"$.

The nice thing about character graphics are the animations.
*/

#define MAP_WIDTH	(TEXT_COLS*2)
#define MAP_HEIGHT	(TEXT_ROWS*2)

#define CH_WATER	'^'
#define CH_LAND		CH_SPACE
#define CH_TREE		'*'

U16 map[MAP_HEIGHT][MAP_WIDTH];

I64 scrn_pix_x,scrn_pix_y;

U0 DrawIt(CTask *task,CDC *)
{
  U16 *ptr;
  I64 i,j,y=scrn_pix_y>>3;
  gr.hide_col=gr.hide_row=TRUE;
  gr.pan_text_x=7-scrn_pix_x&7;
  gr.pan_text_y=7-scrn_pix_y&7;
  for (i=0;i<task->win_height;i++) {
    ptr=&map[y++][scrn_pix_x>>3];
    for (j=0;j<task->win_width;j++)
	//By the time you clip to window and handle Fs->scroll_x,Fs->scroll_y,
	//it is too much trouble to do raw access to gr.text_base like we do
	//in $LK,"::/Demo/Games/Maze.HC"$.
      TextChar(task,FALSE,j,i,*ptr++);
  }
}

U8 *old_font=text.font;
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
  font[CH_TREE]=0x18187E7E3C3C18;
  while (TRUE) {
    font[CH_WATER]=waves[i++&0x3];
    Sleep(200);
    Refresh;
  }
}

U0 ScrollTaskX(I64 sign)
{
  I64 i;
  for (i=0;i<32;i++) {
    scrn_pix_x=ClampI64(scrn_pix_x+sign,0,
	  (MAP_WIDTH-TEXT_COLS+1)*FONT_WIDTH);
    Refresh;
  }
}

U0 ScrollTaskY(I64 sign)
{
  I64 i;
  for (i=0;i<32;i++) {
    scrn_pix_y=ClampI64(scrn_pix_y+sign,0,
	  (MAP_HEIGHT-TEXT_ROWS+2)*FONT_HEIGHT);
    Refresh;
  }
}

U0 Init()
{
  I64 i,j,x,y;
  scrn_pix_x=(MAP_WIDTH-TEXT_COLS)>>1*FONT_WIDTH;
  scrn_pix_y=(MAP_HEIGHT-TEXT_ROWS)>>1*FONT_HEIGHT;

  MemSetU16(map,CH_WATER+(BLUE<<4+LTBLUE)<<8,MAP_WIDTH*MAP_HEIGHT);
  for (i=1;i<MAP_WIDTH-1;i++)
    map[0][i]=map[MAP_HEIGHT-1][i]='Í'+(BLUE<<4+RED)<<8;
  for (i=1;i<MAP_HEIGHT-1;i++)
    map[i][0]=map[i][MAP_WIDTH-1]='º'+(BLUE<<4+RED)<<8;
  map[0][0]			='É'+(BLUE<<4+RED)<<8;
  map[0][MAP_WIDTH-1]		='»'+(BLUE<<4+RED)<<8;
  map[MAP_HEIGHT-1][0]		='È'+(BLUE<<4+RED)<<8;
  map[MAP_HEIGHT-1][MAP_WIDTH-1]='¼'+(BLUE<<4+RED)<<8;

  for (i=0;i<20;i++) {
    x=MAP_WIDTH*Rand;
    y=MAP_HEIGHT*Rand;
    for (j=0;j<1000;j++) {
      map[y][x]=CH_LAND+(YELLOW<<4+BLACK)<<8;
      x=ClampI64(x+RandU16%3-1,0,MAP_WIDTH-1);
      y=ClampI64(y+RandU16%3-1,0,MAP_HEIGHT-1);
    }
  }
  for (i=0;i<100;i++) {
    do {
      x=MAP_WIDTH*Rand;
      y=MAP_HEIGHT*Rand;
    } while (map[y][x].u8[0]!=CH_LAND);
    for (j=0;j<100;j++) {
      map[y][x]=CH_TREE+(YELLOW<<4+GREEN)<<8;
      x=ClampI64(x+RandU16%3-1,0,MAP_WIDTH-1);
      y=ClampI64(y+RandU16%3-1,0,MAP_HEIGHT-1);
    }
  }
}

U0 CleanUp()
{
  gr.pan_text_x=gr.pan_text_y=0;
  gr.hide_col=gr.hide_row=FALSE;
}

U0 CharDemo()
{
  I64 sc;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=YELLOW<<4+BLUE;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  Up(,,SC_CURSOR_UP);"
	"  Down(,,SC_CURSOR_DOWN);"
	"}"
	);
  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      switch (GetKey(&sc)) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_LEFT:
	      Spawn(&ScrollTaskX,-1,"Scroll",,Fs);
	      break;
	    case SC_CURSOR_RIGHT:
	      Spawn(&ScrollTaskX,1,"Scroll",,Fs);
	      break;
	    case SC_CURSOR_UP:
	      Spawn(&ScrollTaskY,-1,"Scroll",,Fs);
	      break;
	    case SC_CURSOR_DOWN:
	      Spawn(&ScrollTaskY,1,"Scroll",,Fs);
	      break;
	  }
	  break;
	case '\n':
	  CleanUp;
	  Init;
	  break;
	case CH_SHIFT_ESC:
	case CH_ESC:
	  goto sq_done;
      }
    }
sq_done: //Don't goto out of try
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
}

CharDemo;
