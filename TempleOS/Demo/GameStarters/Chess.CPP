


$SP,"wpawn",1$



$SP,"wcastle",2$




$SP,"wknight",3$





$SP,"wbishop",4$





$SP,"wqueen",5$





$SP,"wking",6$




$SP,"bpawn",7$



$SP,"bcastle",8$




$SP,"bknight",9$





$SP,"bbishop",10$





$SP,"bqueen",11$






$SP,"bking",12$

//These are weights for scoring positions.
#define SHIFT_PIECE_SCORE	4
#define SHIFT_POSSIBLE_MOVES	0

#define P_PAWN		0
#define P_ROOK		1
#define P_KNIGHT	2
#define P_BISHOP	3
#define P_QUEEN		4
#define P_KING		5

U8 *imgs[12]={$IB,"<1>",1$,$IB,"<7>",7$,$IB,"<2>",2$,$IB,"<8>",8$,$IB,"<3>",3$,$IB,"<9>",9$,$IB,"<4>",4$,$IB,"<10>",10$,$IB,"<5>",5$,$IB,"<11>",11$,$IB,"<6>",6$,$IB,"<12>",12$};

I64 piece_scores[6]={
1<<SHIFT_PIECE_SCORE,
5<<SHIFT_PIECE_SCORE,
3<<SHIFT_PIECE_SCORE,
3<<SHIFT_PIECE_SCORE,
9<<SHIFT_PIECE_SCORE,
100<<SHIFT_PIECE_SCORE
};

U8 piece_letters[]="PRNBQK";

class Move
{
  I8 piece,x,y,pad;
  I32 score;
};

class PieceState
{
  I8 player,type,x,y;
};

class GameState
{
  GameState *next,*last,*parent;
  PieceState pieces[32];
  I64 cur_player,
      mv_num,
      moved_mask,
      raw_score;	    //Positive favors player#0
  Bool is_human[2];
  Move potential_en_passant_victim;
};

GameState setup_state=
{ NULL,NULL,NULL, {
  {0,P_KING,0,4},
  {0,P_QUEEN,0,3},
  {0,P_BISHOP,0,2},
  {0,P_BISHOP,0,5},
  {0,P_KNIGHT,0,1},
  {0,P_KNIGHT,0,6},
  {0,P_ROOK,0,0},
  {0,P_ROOK,0,7},

  {0,P_PAWN,1,0},
  {0,P_PAWN,1,1},
  {0,P_PAWN,1,2},
  {0,P_PAWN,1,3},
  {0,P_PAWN,1,4},
  {0,P_PAWN,1,5},
  {0,P_PAWN,1,6},
  {0,P_PAWN,1,7},

  {1,P_KING,7,4},
  {1,P_QUEEN,7,3},
  {1,P_BISHOP,7,2},
  {1,P_BISHOP,7,5},
  {1,P_KNIGHT,7,1},
  {1,P_KNIGHT,7,6},
  {1,P_ROOK,7,0},
  {1,P_ROOK,7,7},

  {1,P_PAWN,6,0},
  {1,P_PAWN,6,1},
  {1,P_PAWN,6,2},
  {1,P_PAWN,6,3},
  {1,P_PAWN,6,4},
  {1,P_PAWN,6,5},
  {1,P_PAWN,6,6},
  {1,P_PAWN,6,7},
},0,0,0,0,{TRUE,FALSE},{0,-1,-1,0,0}};

GameState cur_state;
CDoc *chess_doc;

U0 GameSnapShot()
{
  GameState *tempg=MAlloc(sizeof(GameState));
  MemCpy(tempg,&cur_state,sizeof(GameState));
  QueIns(tempg,cur_state.last);
}

I64 PieceFind(I64 x,I64 y,I64 player)
{
  I64 i;
  for (i=0;i<32;i++)
    if (cur_state.pieces[i].player==player && cur_state.pieces[i].x==x && cur_state.pieces[i].y==y)
      return i;
  return -1;
}


#define BD_BORDER	4
#define BD_SIDE		(BD_BORDER+8+BD_BORDER)
#define T_BLKED		-1
#define T_PLAYER0	0
#define T_PLAYER1	1
#define T_CLR		2

U0 BrdFill(I8 *brd,PieceState *pieces)
{
  I64 i;
  MemSet(brd,T_BLKED,sizeof(I8)*BD_SIDE*BD_SIDE);

  for (i=BD_BORDER;i<8+BD_BORDER;i++)
    brd[BD_SIDE*i+BD_BORDER](I64)=0x0202020202020202;
#assert T_CLR==2

  for (i=0;i<32;i++)
    if (!pieces[i].player)
      brd[BD_SIDE*(BD_BORDER+pieces[i].y)+BD_BORDER+pieces[i].x]=T_PLAYER0;
    else if (pieces[i].player==1)
      brd[BD_SIDE*(BD_BORDER+pieces[i].y)+BD_BORDER+pieces[i].x]=T_PLAYER1;
}

I8 BrdPeek(I8 *brd,I64 x,I64 y)
{
  return brd[(y+BD_BORDER)*BD_SIDE+BD_BORDER+x];
}


I64 knight_x_offsets[8]={-1, 1, 2, 2,-1, 1,-2,-2},
    knight_y_offsets[8]={-2,-2,-1, 1, 2, 2,-1, 1},
    bishop_x_offsets3[4]={-1, 1,-1, 1},
    bishop_y_offsets3[4]={-1,-1, 1, 1};

I64 PieceMoves(GameState *state,I64 piece_num,Move *mvs)
{
  PieceState *p=state->pieces,*cur_p=&p[piece_num];
  I64 result=0,i,j,k,x,y,xx=cur_p->x,yy=cur_p->y;
  I8 brd[BD_SIDE*BD_SIDE*sizeof(I8)];
  BrdFill(brd,p);
  nobound_switch (state->pieces[piece_num].type) {
    case P_PAWN:
      if (!state->cur_player) {
	if (BrdPeek(brd,xx+1,yy)==T_CLR) {
	  mvs[result].x=xx+1;
	  mvs[result].y=yy;
	  result++;
	  if (xx==1 && BrdPeek(brd,xx+2,yy)==T_CLR) {
	    mvs[result].x=xx+2;
	    mvs[result].y=yy;
	    result++;
	  }
	}
	if (BrdPeek(brd,xx+1,yy-1)==T_PLAYER1 ||
	  state->potential_en_passant_victim.x==xx &&
	  state->potential_en_passant_victim.y==yy-1) {
	  mvs[result].x=xx+1;
	  mvs[result].y=yy-1;
	  result++;
	}
	if (BrdPeek(brd,xx+1,yy+1)==T_PLAYER1 ||
	  state->potential_en_passant_victim.x==xx &&
	  state->potential_en_passant_victim.y==yy+1) {
	  mvs[result].x=xx+1;
	  mvs[result].y=yy+1;
	  result++;
	}
      } else {
	if (BrdPeek(brd,xx-1,yy)==T_CLR) {
	  mvs[result].x=xx-1;
	  mvs[result].y=yy;
	  result++;
	  if (xx==6 && BrdPeek(brd,xx-2,yy)==T_CLR) {
	    mvs[result].x=xx-2;
	    mvs[result].y=yy;
	    result++;
	  }
	}
	if (!BrdPeek(brd,xx-1,yy-1) ||
	  state->potential_en_passant_victim.x==xx &&
	  state->potential_en_passant_victim.y==yy-1) {
	  mvs[result].x=xx-1;
	  mvs[result].y=yy-1;
	  result++;
	}
	if (!BrdPeek(brd,xx-1,yy+1) ||
	  state->potential_en_passant_victim.x==xx &&
	  state->potential_en_passant_victim.y==yy+1) {
	  mvs[result].x=xx-1;
	  mvs[result].y=yy+1;
	  result++;
	}
      }
      break;
    case P_ROOK:
      for (j=0;j<4;j++) {
	x=xx; y=yy;
	for (i=1;i<8;i++) {
	  x+=gr_x_offsets2[j];
	  y+=gr_y_offsets2[j];
	  k=BrdPeek(brd,x,y);
	  if (k==T_CLR || k==cur_p->player^1) {
	    mvs[result].x=x;
	    mvs[result].y=y;
	    result++;
	    if (k!=T_CLR)
	      break;
	  } else
	    break;
	}
      }
      break;
    case P_KNIGHT:
      for (j=0;j<8;j++) {
	x=xx+knight_x_offsets[j];
	y=yy+knight_y_offsets[j];
	k=BrdPeek(brd,x,y);
	if (k==T_CLR || k==cur_p->player^1) {
	  mvs[result].x=x;
	  mvs[result].y=y;
	  result++;
	}
      }
      break;
    case P_BISHOP:
      for (j=0;j<4;j++) {
	x=xx; y=yy;
	for (i=1;i<8;i++) {
	  x+=bishop_x_offsets3[j];
	  y+=bishop_y_offsets3[j];
	  k=BrdPeek(brd,x,y);
	  if (k==T_CLR || k==cur_p->player^1) {
	    mvs[result].x=x;
	    mvs[result].y=y;
	    result++;
	    if (k!=T_CLR)
	      break;
	  } else
	    break;
	}
      }
      break;
    case P_QUEEN:
      for (j=0;j<8;j++) {
	x=xx; y=yy;
	for (i=1;i<8;i++) {
	  x+=gr_x_offsets[j];
	  y+=gr_y_offsets[j];
	  k=BrdPeek(brd,x,y);
	  if (k==T_CLR || k==cur_p->player^1) {
	    mvs[result].x=x;
	    mvs[result].y=y;
	    result++;
	    if (k!=T_CLR)
	      break;
	  } else
	    break;
	}
      }
      break;
    case P_KING:
      for (j=0;j<8;j++) {
	x=xx+gr_x_offsets[j];
	y=yy+gr_y_offsets[j];
	k=BrdPeek(brd,x,y);
	if (k==T_CLR || k==cur_p->player^1) {
	  mvs[result].x=x;
	  mvs[result].y=y;
	  result++;
	}
      }
      if (!Bt(&state->moved_mask,piece_num)) {
	if (!state->cur_player) {
	  if (state->pieces[0].player>=0 &&
	      !Bt(&state->moved_mask,0) &&
	      BrdPeek(brd,0,1)==T_CLR &&
	      BrdPeek(brd,0,2)==T_CLR &&
	      BrdPeek(brd,0,3)==T_CLR) {
	    mvs[result].x=0;
	    mvs[result].y=2;
	    result++;
	  }
	  if (state->pieces[7].player>=0 &&
	      !Bt(&state->moved_mask,7) &&
	      BrdPeek(brd,0,6)==T_CLR &&
	      BrdPeek(brd,0,5)==T_CLR) {
	    mvs[result].x=0;
	    mvs[result].y=6;
	    result++;
	  }
	} else {
	  if (state->pieces[24].player>=0 &&
	      !Bt(&state->moved_mask,24) &&
	      BrdPeek(brd,7,1)==T_CLR &&
	      BrdPeek(brd,7,2)==T_CLR &&
	      BrdPeek(brd,7,3)==T_CLR) {
	    mvs[result].x=7;
	    mvs[result].y=2;
	    result++;
	  }
	  if (state->pieces[31].player>=0 &&
	      !Bt(&state->moved_mask,31) &&
	      BrdPeek(brd,7,6)==T_CLR &&
	      BrdPeek(brd,7,5)==T_CLR) {
	    mvs[result].x=7;
	    mvs[result].y=6;
	    result++;
	  }
	}
      }
      break;
  }
  for (i=0;i<result;i++)
    mvs[i].piece=piece_num;
  return result;
}

Bool SqrInChk(GameState *state,I64 x,I64 y)
{
  I64 i,j,mv_cnt;
  Move mvs[32];
  for (i=0;i<32;i++)
    if (state->pieces[i].player==state->cur_player) {
      mv_cnt=PieceMoves(state,i,mvs);
      for (j=0;j<mv_cnt;j++)
	if (mvs[j].x==x && mvs[j].y==y)
	  return TRUE;
    }
  return FALSE;
}

I64 PopUpPiece(I64 player)
{
  U8 buf[64];
  I64 i;
  CDoc *doc;
  do {
    doc=DocNew;
    DocPutS(doc,"\n\n\n\n\n   ");
    for (i=P_ROOK;i<=P_QUEEN;i++) {
      SPrintF(buf,"$$SP+LE+X,\"\",%d,%%d$$    ",i);
      DocPutSprite(doc,imgs[2*i+player],buf);
    }
    DocPutS(doc,"\n\n\n");
    i=PopUpMenu(doc);
    DocDel(doc);
  } while (i<0);
  return i;
}

CDC *brd_dc;
I64 cx,cy,cx2,cy2,g,cursor_x,cursor_y,
    cur_mv_cnt,*s2w,*w2s;
Move cur_mvs[32],last_mv;
Bool ai_thinking,game_over;

extern I64 MoveCnt(GameState *state);

Bool MoveDo(GameState *state,I64 piece_num,I64 x2,I64 y2,Bool final)
{ //returns TRUE if killed a piece.
  PieceState *p=&state->pieces[piece_num];
  I64 j,ty=-1,score_adjust=0,x1=p->x,y1=p->y;

  j=PieceFind(x2,y2,state->cur_player^1);
  if (j>=0) {
    state->pieces[j].player-=2; //dead
    ty=state->pieces[j].type;
    score_adjust=piece_scores[ty];
  } else if (p->type==P_PAWN && AbsI64(y1-y2)==1) { //en passant?
    if (!state->cur_player)
      j=PieceFind(x2-1,y2,1);
    else
      j=PieceFind(x2+1,y2,0);
    if (j>=0) {
      state->pieces[j].player-=2; //dead
      ty=state->pieces[j].type;
      score_adjust=piece_scores[ty];
    }
  }
  if (p->type==P_PAWN && AbsI64(x1-x2)==2) {
    state->potential_en_passant_victim.x=x2;
    state->potential_en_passant_victim.y=y2;
  } else {
    state->potential_en_passant_victim.x=-1;
    state->potential_en_passant_victim.y=-1;
  }
  if (p->type==P_PAWN && (!x2||x2==7)) {
    if (!state->is_human[state->cur_player] || !final)
      p->type=P_QUEEN;
    else
      p->type=PopUpPiece(state->cur_player);
    score_adjust+=piece_scores[p->type]-piece_scores[P_PAWN];
  } else if (p->type==P_KING && AbsI64(y1-y2)==2) { //castling?
    if (!state->cur_player) {
      if (y2-y1<0)
	state->pieces[6].y=y2+1;
      else
	state->pieces[7].y=y2-1;
    } else {
      if (y2-y1<0)
	state->pieces[22].y=y2+1;
      else
	state->pieces[23].y=y2-1;
    }
  }
  p->x=x2;
  p->y=y2;
  LBts(&state->moved_mask,piece_num);
  if (!state->cur_player)
    state->raw_score+=score_adjust;
  else
    state->raw_score-=score_adjust;
  state->cur_player^=1;
  state->mv_num++;
  if (final) {
    last_mv.x=x2;
    last_mv.y=y2;
    DocPrintF(chess_doc,"%02d)",state->mv_num);
    if (state->cur_player)
      DocPutS(chess_doc,"$$FG,LTGRAY$$");
    else
      DocPutS(chess_doc,"$$FG,BLACK");
    DocPrintF(chess_doc,"%c:%c%c-%c%c",piece_letters[p->type],
      y1+'A',x1+'1',y2+'A',x2+'1');
    if (ty>=0) {
      if (state->cur_player)
	DocPutS(chess_doc,"$$FG$$*$$FG,BLACK$$");
      else
	DocPutS(chess_doc,"$$FG$$*$$FG,LTGRAY$$");
      DocPrintF(chess_doc,"%c",piece_letters[ty]);
    }
    DocPutS(chess_doc,"$$FG$$\n");
    if (!MoveCnt(&cur_state)) {
      DocPutS(chess_doc,"$$FG,RED$$Game Over$$FG$$\n");
      game_over=TRUE;
    }
    GameSnapShot;
  }
  return ToBool(score_adjust);
}

I64 MoveFind(I64 x,I64 y,Move *mvs,I64 mv_cnt)
{
  I64 i;
  for (i=0;i<mv_cnt;i++)
    if (mvs[i].x==x && mvs[i].y==y)
      return i;
  return -1;
}

Bool HumanMove(GameState *state,I64 piece_num,Move *mvs,I64 mv_cnt,I64 x,I64 y)
{
  if (piece_num>=0 && MoveFind(x,y,mvs,mv_cnt)>=0) {
    MoveDo(state,piece_num,x,y,TRUE);
    return TRUE;
  } else
    return FALSE;
}

I64 ChkPieceMoves(GameState *state,I64 piece_num,Move *src,I64 mv_cnt)
{ //Goes through moves lst for one piece, eliminating invalid moves.
  I64 k,cnt=mv_cnt,result=0,x,y,dy;
  GameState state2,*tempg;
  Move *dst=src;

  if (state->cur_player) //Find King
    k=16;
  else
    k=0;
  while (cnt--) {
    MemCpy(&state2,state,sizeof(GameState));
    MoveDo(&state2,piece_num,src->x,src->y,FALSE);

    //Don't repeat board position
    tempg=cur_state.next;
    while (tempg!=&cur_state) {
      if (!MemCmp(tempg->pieces,state2.pieces,sizeof(PieceState)*32))
	goto cm_skip;
      tempg=tempg->next;
    }

    if (piece_num==k) {  //If king is moving, check his new location.
      x=state2.pieces[k].x;
      y=state2.pieces[k].y;
      dy=y-state->pieces[k].y;
      if (AbsI64(dy)==2 && //castling?
	(SqrInChk(&state2,x,y-dy) || SqrInChk(&state2,x,y-SignI64(dy))))
	goto cm_skip; // This really needs to go into PieceMoves
    } else {
      x=state->pieces[k].x;
      y=state->pieces[k].y;
    }
    if (!SqrInChk(&state2,x,y)) {
      MemCpy(dst++,src,sizeof(Move));
      result++;
    }
cm_skip:
    src++;
  }
  return result;
}

I64 MoveCnt(GameState *state)
{
  I64 i,result=0,mv_cnt;
  Move mvs[32];
  for (i=0;i<32;i++)
    if (state->pieces[i].player==state->cur_player) {
      mv_cnt=PieceMoves(state,i,mvs);
      mv_cnt=ChkPieceMoves(state,i,mvs,mv_cnt);
      result+=mv_cnt;
    }
  return result;
}


U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,x,y,z,k0,k1;
  PieceState *s=cur_state.pieces;
  brd_dc->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,cx-cx2,cy-cy2,brd_dc);

  task->text_attr=YELLOW<<4;
  dc->x=cx;
  dc->y=cy;

  dc->flags|=DCF_TRANSFORMATION;
  MemCpy(dc->r,w2s,sizeof(I64)*16);
 
  if (ai_thinking) {
    if (cur_state.cur_player)
      dc->color=BLACK;
    else
      dc->color=WHITE;
    if (Blink)
      GrPutS(dc,-11*FONT_WIDTH/2,2*FONT_HEIGHT-cy,"Thinking...");
  }

  dc->color=RED;
  for (i=0;i<cur_mv_cnt;i++)
    GrRect3(dc,(cur_mvs[i].x-4)*g+g/4,(cur_mvs[i].y-4)*g+g/4,0,g/2,g/2);

  dc->color=LTRED;
  dc->pen_width=2;
  GrLineRect4(dc,(cursor_x-4)*g+2,(cursor_y-4)*g+2,(cursor_x-3)*g-1,(cursor_y-3)*g-1);

  DCAllocDepthBuf(dc);
  k0=k1=0;
  for (i=0;i<32;i++,s++) {
    if (s->player>=0) { //Alive?
      if (s->x!=last_mv.x || s->y!=last_mv.y || Blink) {
	x=g/2+(s->x-4)*g;
	y=g/2+(s->y-4)*g;
	z=0;
	GrTransform(dc,&x,&y,&z);
	z=GR_Z_ALL+s->x-s->y;

	dc->flags&=~DCF_TRANSFORMATION;
	SpritePlot3(dc,x,y,z,imgs[s->type*2+s->player]);
	dc->flags|=DCF_TRANSFORMATION;
      }
    } else { //Dead
      if (s->player==-2) {
	if (k1<8) {
	  x=cx+100+k1++*g/2;
	  y=cy-180;
	} else {
	  x=cx+100+(k1++-8)*g/2;
	  y=cy-130;
	}
	z=0;
      } else {
	if (k0<8) {
	  x=cx+100+k0++*g/2;
	  y=cy+220;
	} else {
	  x=cx+100+(k0++-8)*g/2;
	  y=cy+170;
	}
	z=0;
      }
      dc->flags&=~DCF_TRANSFORMATION;
      SpritePlot3(dc,x,y,z,imgs[s->type*2+s->player+2]);
      dc->flags|=DCF_TRANSFORMATION;
    }
  }
  if (game_over && Blink) {
    dc->color=LTRED;
    GrPutS3(dc,-9*FONT_WIDTH/2,-FONT_HEIGHT/2,0,"Game Over");
  }
}

#define DEPTH	-5

CDC *BrdNew()
{
  I64 i,j,k,w=Fs->win_pixel_width,h=Fs->win_pixel_height;
  CDC *dc;

  g=42;
  cx=w/2+40;
  cy=h/2;

  w=Sqrt(2.0)*(8*g+2*FONT_WIDTH);
  h=8*g+2*FONT_HEIGHT;
  cx2=w/2;
  cy2=h/2;

  dc=DCNew(DCT_COLOR8,w,h);
  dc->x=cx2;
  dc->y=cy2;

  dc->flags|=DCF_TRANSFORMATION;
  MemCpy(dc->r,w2s,sizeof(I64)*16);

  dc->color=YELLOW;
  GrRect(dc,0,0,w,h);

  dc->color=BLACK;
  for (i=-4;i<4;i++) {
    GrPrintF3(dc,-4*g-4-FONT_WIDTH,i*g+g/2-4,0,"%C",i+4+'A');
    GrPrintF3(dc,4*g+14,i*g+g/2-4,0,"%C",i+4+'A');
    GrPrintF3(dc,i*g+g/2+6,-4*g-12-FONT_HEIGHT,0,"%C",i+4+'1');
    GrPrintF3(dc,i*g+g/2-2,4*g+9,0,"%C",i+4+'1');
  }

  dc->pen_width=1;
  dc->color=BLACK;
  for (i=-4;i<=4;i++) {
     GrLine3(dc,-4*g,i*g,0,4*g,i*g,0);
     GrLine3(dc,i*g,-4*g,0,i*g,4*g,0);
     GrLine3(dc,-4*g,i*g,0,-4*g,i*g,DEPTH);
     GrLine3(dc,i*g,4*g,0,i*g,4*g,DEPTH);
  }
  GrLine3(dc,-4*g,4*g,DEPTH,-4*g,-4*g,DEPTH);
  GrLine3(dc,-4*g,4*g,DEPTH,4*g,4*g,DEPTH);

  k=0;
  for (j=-4;j<4;j++) {
    for (i=-4;i<4;i++) {
      if ((i+j)&1)
	dc->color=WHITE;
      else
	dc->color=DKGRAY;
      GrFloodFill3(dc,i*g+g/2,j*g+g/2,0);
    }
    if (j&1)
      dc->color=DKGRAY;
    else
      dc->color=WHITE;
    GrFloodFill3(dc,-4*g,j*g+g/2,DEPTH/2);
    if (j&1)
      dc->color=WHITE;
    else
      dc->color=DKGRAY;
    GrFloodFill3(dc,j*g+g/2,4*g,DEPTH/2);
  }

  return dc;
}

I64 AIStateScore(GameState *state,I64 player)
{
  if (player)
    return -state->raw_score;
  else
    return  state->raw_score;
}

I64 move_depth,kill_depth;

I64 AIMove2(GameState *state,I64 depth)
{
  I64 i,possible_moves=0,best_piece,score,best_score=MIN_I64,mv_cnt,score_adjust;
  Move best_mv,mvs[32],*ptr;
  GameState state2;
  Bool first=TRUE;

  if (UnusedStk<0x1000) { //Shouldn't happen
    DocPutS(chess_doc,"StkOverflow\n");
    return AIStateScore(state,state->cur_player);
  }

  for (i=0;i<32;i++)
    if (state->pieces[i].player==state->cur_player) {
      mv_cnt=PieceMoves(state,i,mvs);
      possible_moves+=mv_cnt;
      ptr=mvs;
      while (mv_cnt--) {
	MemCpy(&state2,state,sizeof(GameState));
	state2.parent=state;
	if (MoveDo(&state2,i,ptr->x,ptr->y,FALSE)&&depth<kill_depth ||
	  depth<move_depth)
	  score=-AIMove2(&state2,depth+1);
	else {
	  score=AIStateScore(&state2,state->cur_player);
	  if (score_adjust=AbsI64(state2.raw_score-state->raw_score))
	    score+=score_adjust-piece_scores[state->pieces[i].type];
	}
	if (first || score>best_score) {
	  best_score=score;
	  best_piece=i;
	  MemCpy(&best_mv,ptr,sizeof(Move));
	  first=FALSE;
	}
	ptr++;
      }
    }
  if (!first)
    MoveDo(state,best_piece,best_mv.x,best_mv.y,FALSE);
  return best_score+possible_moves<<SHIFT_POSSIBLE_MOVES;
}

class ChessJob
{
  GameState *state;
  Move *mv;
};

U0 AIMove1(ChessJob *cj)
{
  GameState state2;
  Move *ptr=cj->mv;
  Bool old_preempt=Preempt(ON);
  MemCpy(&state2,cj->state,sizeof(GameState));
  state2.parent=cj->state;
  MoveDo(&state2,ptr->piece,ptr->x,ptr->y,FALSE);
  ptr->score=-AIMove2(&state2,1);
  Free(cj);
  Preempt(old_preempt);
}

I64 MoveCmp(Move *e1,Move *e2)
{
  if (e1->score<e2->score)
    return 1;
  else if (e1->score==e2->score)
    return 0;
  else
    return -1;
}

U0 AIMove(GameState *state)
{
  I64 i,cnt=0,mv_cnt,best_score;
  Move mvs[32],*ptr,*ptr2;
  ChessJob *cj;

  if (cnt=MoveCnt(state)) {
  //Collect AI possible valid moves
    ptr=ptr2=MAlloc(cnt*sizeof(Move));
    for (i=0;i<32;i++)
      if (state->pieces[i].player==state->cur_player) {
	mv_cnt=PieceMoves(state,i,mvs);
	mv_cnt=ChkPieceMoves(state,i,mvs,mv_cnt);
	MemCpy(ptr,mvs,mv_cnt*sizeof(Move));
	ptr+=mv_cnt;
      }

      //Dispatch to MP scoring all moves
    for (i=0,ptr=ptr2;i<cnt;i++,ptr++) {
      ptr->score=-MAX_I32;
      cj=MAlloc(sizeof(ChessJob));
      cj->state=state;
      cj->mv=ptr;
      if (i%mp_cnt==mp_cnt-1)
	AIMove1(cj);
      else
	MPJob(&AIMove1,cj,1+i%(mp_cnt-1));
    }

    //Wait for finish
    for (i=0,ptr=ptr2;i<cnt;i++,ptr++)
      while (ptr->score==-MAX_I32)
	Yield;
    QSort(ptr2,cnt,sizeof(Move),&MoveCmp);
 
    //Cnt ties for first
    best_score=ptr2->score;
    mv_cnt=0;
    ptr=ptr2;
    while (mv_cnt<cnt && ptr->score==best_score) {
      mv_cnt++;
      ptr++;
    }

    ptr=ptr2+RandU16%mv_cnt;
    MoveDo(state,ptr->piece,ptr->x,ptr->y,TRUE);
    Free(ptr2);
  }
}

U0 Init()
{
  chess_doc=DocCur;
  DocClear;
  "$$FD,BLACK$$$$BD,YELLOW$$";
  game_over=FALSE;
  ai_thinking=FALSE;
  move_depth=3;
  kill_depth=5;
  cur_mv_cnt=0;
  last_mv.x=-1;
  last_mv.y=-1;
  MemCpy(&cur_state,&setup_state,sizeof(cur_state));
  QueInit(&cur_state);
  cur_state.is_human[0]=PopUpNoYes("$$FG,BLACK$$WHITE is human?");
  cur_state.is_human[1]=PopUpNoYes("$$FG,BLACK$$BLACK is human?");

  GameSnapShot;
}

U0 CleanUp()
{
  QueDel(&cur_state,TRUE);
}

U0 SetMouseCursor()
{
  I64 x=(cursor_x-4)*g+g/2,
      y=(cursor_y-4)*g+g/2,
      z=0;
  GrRot(w2s,&x,&y,&z);
  x+=cx+Fs->win_pixel_left+Fs->win_scroll_x;
  y+=cy+Fs->win_pixel_top +Fs->win_scroll_y;
  if (ipx!=x || ipy!=y)
    SetIP(x,y);
}

U0 SetKbdCursor(I64 x,I64 y)
{
  I64 z;
  x-=cx;
  y-=cy;
  z=y;
  GrRot(s2w,&x,&y,&z);
  cursor_x=LimitI64((x+4*g)/g,0,7);
  cursor_y=LimitI64((y+4*g)/g,0,7);
}

U0 Chess()
{
  I64 p1,p2,cur_piece;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinMax;
  WinBorder;
  Preempt(OFF);
  Init;
  WordStat;

  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Move(,CH_SPACE);"
  "  Restart(,'\n');"
  "}");


  Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU;

  cursor_x=cursor_y=3;

  w2s=GrIdent; //World-to-Screen
  GrRotZEqu(w2s,-ã/4);
  GrRotXEqu(w2s,ã/4);

  s2w=GrIdent; //Screen-to-World
  GrRotXEqu(s2w,-ã/4);
  GrRotZEqu(s2w,ã/4);

  brd_dc=BrdNew;

  SetMouseCursor;
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      switch (ScanMsg(&p1,&p2,1<<MSG_KEY_DOWN+1<<MSG_KEY_UP+1<<MSG_IP_L_DOWN+1<<MSG_IP_L_UP+1<<MSG_IP_MOVE)) {
	case 0:
	  if (!game_over && !cur_state.is_human[cur_state.cur_player]) {
	    ai_thinking=TRUE;
	    AIMove(&cur_state);
	    ai_thinking=FALSE;
	    Beep(500);
	  }
	  WinMgrSync;
	  break;
	case MSG_IP_MOVE:
	  SetKbdCursor(p1,p2);
	  break;
	case MSG_IP_L_DOWN:
	  SetKbdCursor(p1,p2);
ch_mv_start:
	  if (!game_over && cur_state.is_human[cur_state.cur_player]) {
	    cur_piece=PieceFind(cursor_x,cursor_y,cur_state.cur_player);
	    if (cur_piece>=0) {
	      cur_mv_cnt=PieceMoves(&cur_state,cur_piece,cur_mvs);
	      cur_mv_cnt=ChkPieceMoves(&cur_state,cur_piece,cur_mvs,cur_mv_cnt);
	    } else
	      cur_mv_cnt=0;
	  }
	  break;
	case MSG_IP_L_UP:
	  SetKbdCursor(p1,p2);
ch_mv_end:
	  if (!game_over && cur_state.is_human[cur_state.cur_player]) {
	    if (cur_piece>=0 &&
		HumanMove(&cur_state,cur_piece,cur_mvs,cur_mv_cnt,
		  cursor_x,cursor_y)) {
	      Beep(250);
	      cur_piece=-1;
	      cur_mv_cnt=0;
	    } else {
	      cur_piece=-1;
	      cur_mv_cnt=0;
	    }
	  }
	  break;
	case MSG_KEY_UP:
	  if (p1==CH_SPACE)
	    goto ch_mv_end;
	  break;
	case MSG_KEY_DOWN:
	  switch (p1) {
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto ch_done;
	    case CH_SPACE:
	      goto ch_mv_start;
	    case 0:
	      switch (p2.u8[0]) {
		case SC_CURSOR_RIGHT:
		  cursor_y=LimitI64(cursor_y+1,0,7);
		  SetMouseCursor;
		  break;
		case SC_CURSOR_LEFT:
		  cursor_y=LimitI64(cursor_y-1,0,7);
		  SetMouseCursor;
		  break;
		case SC_CURSOR_DOWN:
		  cursor_x=LimitI64(cursor_x-1,0,7);
		  SetMouseCursor;
		  break;
		case SC_CURSOR_UP:
		  cursor_x=LimitI64(cursor_x+1,0,7);
		  SetMouseCursor;
		  break;
	      }
	  }
	  break;
      }
    }
ch_done:
  } catch
    CatchAll;
  DocClear;
  SettingsPop;
  MenuPop;
  DCDel(brd_dc);
  Free(s2w);
  Free(w2s);
  CleanUp;
}

Chess;
           R           úÿÿÿðÿÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ   ÿÿÿÿ   ÿÿ   ÿÿ   ÿÿ   ÿÿ   ÿÿÿÿ   ÿÿÿÿÿÿ   ÿÿÿÿÿÿ   ÿÿÿÿÿÿ   ÿÿÿÿÿÿ   ÿÿÿÿ   ÿÿ               ÿÿ   ÿÿÿÿÿÿ              ò           úÿÿÿæÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿ  ÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿ        ÿÿ  ÿÿÿÿÿÿÿÿ             "        !   úÿÿÿâÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿ  ÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿ  ÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿ        ÿÿ  ÿÿÿÿÿÿÿÿ             2        "   øÿÿÿâÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿ ÿÿÿÿ ÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿ ÿÿ ÿÿ ÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿÿÿ ÿÿÿÿ ÿÿÿÿ ÿÿ    ÿÿ ÿÿÿÿÿÿÿÿ     
       ò           úÿÿÿæÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿ   ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿ     ÿÿÿ  ÿÿ     ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ   ÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿ      ÿÿ  ÿ        ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿÿ        ÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿ    ÿÿ  ÿÿ  ÿÿ  ÿ  ÿ                          ÿ      ÿ  ÿÿÿÿ      ÿÿÿÿ             2        "   øÿÿÿâÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿÿÿ ÿÿÿÿ      ÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ      ÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿÿÿ ÿÿÿÿÿ  ÿÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿ    ÿÿ ÿÿ         ÿÿ ÿÿ  ÿÿ ÿÿÿ  ÿÿÿ ÿÿÿ   ÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿ      ÿÿ ÿ         ÿ ÿ  ÿ ÿÿ  ÿÿ ÿÿÿ         ÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ  ÿÿÿÿ ÿÿÿ  ÿÿÿ ÿÿÿ  ÿÿÿ ÿÿ    ÿÿ ÿÿ  ÿÿ ÿ  ÿ                       ÿ      ÿ ÿÿÿÿ       ÿÿÿÿ            ’           úÿÿÿìÿÿÿÿÿÿÿÿÿÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ        ÿÿ  ÿÿÿÿÿÿÿÿ             Â           úÿÿÿêÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ  ÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿ        ÿÿ  ÿÿÿÿÿÿÿÿ             R           úÿÿÿðÿÿÿÿÿÿÿ     ÿÿÿÿ   ÿÿÿ  ÿÿÿ   ÿÿ   ÿÿ   ÿ   ÿ   ÿ   ÿ   ÿ  ÿ   ÿ  ÿ   ÿÿ  ÿÿ   ÿÿÿ  ÿÿÿ   ÿÿÿ       ÿÿÿ   ÿÿÿ  ÿÿÿ   ÿÿÿ  ÿÿÿ   ÿÿ  ÿÿ   ÿ  ÿ                                  ÿ    ÿ   ÿÿÿ       ÿÿÿ              ’           úÿÿÿëÿÿÿÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿ      ÿ  ÿ  ÿ  ÿ    ÿ  ÿ      ÿ  ÿ      ÿ  ÿ  ÿ  ÿ    ÿ  ÿ          ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿÿ        ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿ  ÿ  ÿÿ  ÿ  ÿ                          ÿ      ÿ  ÿÿÿÿ      ÿÿÿÿ             "        !   øÿÿÿãÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿÿ  ÿ        ÿ  ÿ      ÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ   ÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿ      ÿÿ  ÿ        ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿÿ        ÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿ    ÿÿ  ÿÿ  ÿÿ  ÿ  ÿ                          ÿ      ÿ  ÿÿÿÿ      ÿÿÿÿ      	       Ò           øÿÿÿêÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ ÿÿÿÿÿÿÿ   ÿÿÿÿÿ ÿÿÿÿÿÿ  ÿÿÿÿ ÿÿÿÿÿ    ÿÿÿÿ ÿÿÿÿ   ÿÿÿ ÿÿÿ  ÿÿ ÿÿÿ   ÿÿ ÿÿ   ÿÿ ÿ    ÿÿ ÿ    ÿÿÿ ÿ    ÿÿÿÿ ÿ  ÿ  ÿÿÿÿ ÿ  ÿÿ  ÿÿÿÿ ÿÿÿÿ   ÿÿÿ ÿÿÿÿ  ÿÿÿ ÿÿÿÿ  ÿÿÿ ÿÿÿÿ  ÿÿÿ ÿÿÿÿ  ÿÿÿ ÿÿÿ    ÿÿ ÿÿÿ  ÿÿ ÿÿ  ÿ ÿ         ÿ         ÿ   ÿÿ      ÿ ÿÿÿÿÿ      ÿÿÿÿ ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  