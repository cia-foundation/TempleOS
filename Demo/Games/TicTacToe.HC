I8  board  [9];
I64 board_x[9]={150,250,350,150,250,350,150,250,350};
I64 board_y[9]={150,150,150,250,250,250,350,350,350};

U0 DrawX(CDC *dc,I64 x,I64 y)
{
  dc->thick=5;
  GrLine3(dc,x-20,y-20,0,x+20,y+20,0);
  GrLine3(dc,x+20,y-20,0,x-20,y+20,0);
}

U0 DrawO(CDC *dc,I64 x,I64 y)
{
  dc->thick=5;
  GrCircle3(dc,x,y,0,25);
}

U0 DrawBoard(I64 player)
{
  I64 i;
  CDC *dc=DCAlias;
  DCFill;
  dc->thick=5;
  GrLine3(dc,200,100,0,200,400,0);
  GrLine3(dc,300,100,0,300,400,0);
  GrLine3(dc,100,200,0,400,200,0);
  GrLine3(dc,100,300,0,400,300,0);
  for (i=0;i<9;i++)
    switch (board[i]) {
      case 1:
	DrawX(dc,board_x[i],board_y[i]);
	break;
      case 2:
	DrawO(dc,board_x[i],board_y[i]);
	break;

    }
//ms.pos.x,ms.pos.y are mouse x,y.
    //ms.lb	is mouse left bttn
  if (player==1)
    DrawX(dc,ms.pos.x-Fs->pix_left,ms.pos.y-Fs->pix_top);
  else
    DrawO(dc,ms.pos.x-Fs->pix_left,ms.pos.y-Fs->pix_top);
  DCDel(dc);
}

U0 TakeTurn(I64 player)
{
  I64 i,x=0,y=0;
  while (!(100<=x<400) || !(100<=y<400) || board[i]) {
    if (player==1)
      Beep;
    else
      Beep(74);
    while (!ms.lb) {
      DrawBoard(player);
      Refresh;
    }
    while (ms.lb) {
      DrawBoard(player);
      Refresh;
    }
    x=ms.pos.x-Fs->pix_left;
    y=ms.pos.y-Fs->pix_top;
    i=(x-100)/100+(y-100)/100*3;
  }
  board[i]=player;
}

Bool CheckWinner(I64 player)
{
  if (	player==board[0]==board[1]==board[2] ||
	player==board[3]==board[4]==board[5] ||
	player==board[6]==board[7]==board[8] ||

	player==board[0]==board[3]==board[6] ||
	player==board[1]==board[4]==board[7] ||
	player==board[2]==board[5]==board[8] ||

	player==board[0]==board[4]==board[8] ||
	player==board[2]==board[4]==board[6]   )
    return TRUE;
  else
    return FALSE;
}

U0 DoGame()
{
  I64 moves=0;
  MemSet(board,0,sizeof(board));
  while (TRUE) {
    TakeTurn(1);
    if (CheckWinner(1)) {
      Beep;
      Beep;
      Beep;
      break;
    }
    if (++moves==9) break;

    TakeTurn(2);
    if (CheckWinner(2)) {
      Beep(74);
      Beep(74);
      Beep(74);
      break;
    }
    if (++moves==9) break;
  }
}

U0 TicTacToe()
{
  WinMax;   //Maximize window
  DocClear; //Clear window text.
  "$$GREEN$$CTRL-ALT-c$$FG$$ to exit.\n";
  try
    while (TRUE)
      DoGame;
  catch
    PutExcept;
  DCFill;
}

TicTacToe;
