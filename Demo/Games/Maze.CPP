//See $LK,"ExtScreenCodes",A="FA:::/Adam/Gr/GrScreen.CPP,ExtScreenCodes"$

#define ATTR (BLACK<<12+WHITE<<8)

U32 text[TEXT_ROWS][TEXT_COLS];

U0 DrawIt(CTask *task,CDC *)
{
  MemCpy(gr_text_base+TEXT_COLS,text,(TEXT_ROWS-1)*TEXT_COLS*sizeof(U32));

// You can copy it this way, if you like:
//  I64 i,j;
//  for (j=0;j<TEXT_ROWS;j++)
//    for (i=0;i<TEXT_COLS;i++)
//     PlotWinU8(task,text[j][i],i,j);

  PlotWinStr(task,"Draw a maze with left button.",ATTR>>8,0,0);
  PlotWinStr(task,"Solve maze starting at right click.",ATTR>>8,0,1);
}

#define STK_SIZE 2048
//We would put these as local vars
//In SolveMaze() except the system stk size
//is limited.  The heap is the normal TempleOS
//technique, but it's a pain in this case.
I64	stk_ptr,
	stk_x  [STK_SIZE],
	stk_y  [STK_SIZE],
	stk_dir[STK_SIZE];

//Four directions:
//  0=Up,1=right,2=down,3=left
I64	dir_x[4]={ 0,+1, 0,-1},  // Could use $LK,"gr_x_offsets2",A="MN:gr_x_offsets2"$,$LK,"gr_y_offsets2",A="MN:gr_y_offsets2"$
	dir_y[4]={+1, 0,-1, 0};

U0 SolveMaze(I64 x,I64 y)
{
  I64 dir=0;
  stk_ptr=0;
  stk_x[stk_ptr]=x;
  stk_y[stk_ptr]=y;
  stk_dir[stk_ptr++]=dir;
  while (TRUE) {
    if (!(0<=x<MinI64(Fs->win_width,TEXT_COLS)) ||
	!(0<=y<MinI64(Fs->win_height,TEXT_ROWS)) ) {
      Beep;Beep;
      break;
    }
    if (!text[y][x].u8[0])
      text[y][x]='.'+ATTR;
    x+=dir_x[dir];
    y+=dir_y[dir];
//u8.[0] is the ASCII
    if (text[y][x].u8[0]) {
      x-=dir_x[dir];
      y-=dir_y[dir];
      if (++dir==4) {
	if (--stk_ptr<0) return;
	x=stk_x[stk_ptr];
	y=stk_y[stk_ptr];
	dir=stk_dir[stk_ptr];
      }
    } else {
      dir=0;
      stk_x[stk_ptr]=x;
      stk_y[stk_ptr]=y;
      stk_dir[stk_ptr++]=dir;
      if (stk_ptr==STK_SIZE) return;
      Sleep(100);
      if (ScanChar)
	ThrowBreak;  //CTRL-ALT-C
    }
  }
}

U0 Maze()
{
  I64 ch,x,y;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  WinBorder;
  WordStat;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_FOCUS-WIF_BORDER;

  try
    do {
      MemSet(text,0,sizeof(text));
      while (!(ch=ScanChar)) {
//Input Ptr (mouse) text x,y coordinates.
	x=iptx-Fs->win_left-Fs->scroll_x/FONT_WIDTH;
	y=ipty-Fs->win_top-Fs->scroll_y/FONT_HEIGHT;
	if (ip_lb&&!win_grab_scroll)
	  text[y][x]=CH_SPACE+ATTRF_INVERT+ATTR;
	if (ip_rb&&!win_grab_scroll) {
	  text[y][x]='*'+ATTR;
	  SolveMaze(x,y);
	  ch=GetChar;
	  break;
	}
	WinMgrSync;
      }
    } while (ch!=CH_SHIFT_ESC && ch!=CH_ESC);
  catch
    PutExcept;
  SettingsPop;
}

Maze;
