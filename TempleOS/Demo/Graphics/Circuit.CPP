
$SP,"<1>",1$

$SP,"<2>",2$

$SP,"<3>",3$

$SP,"<4>",4$

$SP,"<5>",5$

$SP,"<6>",6$

#define A_EAST	0
#define A_NORTH	1
#define A_WEST	2
#define A_SOUTH	3

U0 OrthRot(I64 *_x,I64 *_y,I64 é)
{
  nobound_switch (é&3) {
    case A_EAST:
      break;
    case A_NORTH:
      *_y=-*_y;
      SwapU64(_x,_y);
      break;
    case A_WEST:
      *_x=-*_x;
      *_y=-*_y;
      break;
    case A_SOUTH:
      *_x=-*_x;
      SwapU64(_x,_y);
      break;
  }
}

#define MAX_TERMINALS	6

#define ET_R		0
#define ET_L		1
#define ET_C		2
#define ET_NUM_TYPES	3

class ElementType
{
  U8 *black_img,*red_img;
  I64 terminals;
  CD2I64 term[MAX_TERMINALS];
  I64 min_x,max_x,min_y,max_y;
};

ElementType et[ET_NUM_TYPES] =
{
  {$IB,"<1>",1$,$IB,"<2>",2$,2,{{-12,1},{13,1},{0,0},{0,0},{0,0},{0,0}},0,0,0,0},
  {$IB,"<3>",3$,$IB,"<4>",4$,2,{{-14,0},{15,0},{0,0},{0,0},{0,0},{0,0}},0,0,0,0},
  {$IB,"<5>",5$,$IB,"<6>",6$,2,{{-4 ,0},{6 ,0},{0,0},{0,0},{0,0},{0,0}},0,0,0,0},
};

extern class Element;
class Terminal
{
  Element *e;
  I64 terminal;
};


#define Df_PROCESSED		0
#define Df_JUNCTION		1
#define Df_TERMINAL		2
#define Df_TERMINAL_LEFT	3
#define Df_TERMINAL_RIGHT	4
#define Df_TERMINAL_TOP		5
#define Df_TERMINAL_BOTTOM	6

class Dot  //These are points on a node
{
  Dot *next,*last;
  I64 x,y,saved_x,saved_y;
  Element *e;
  U16  flags[4];
  Dot *left,*right,*top,*bottom;
};

class Node
{
  Node *next,*last;
  I64 cnt;
  Terminal *terms;
  Dot *dot_root;
} node_root;

class Element
{
  Element *next,*last;
  ElementType *type;
  I64 x,y,é,min_x,max_x,min_y,max_y;
  Node *nodes[MAX_TERMINALS];
} el_root;


#define MD_CONNECT	0
#define MD_MOVE		1
#define MD_NUM_MODES	2
I64 mode;

Element *rubber_band;
I64 rubber_band_terminal,rubber_band_x,rubber_band_y;

U0 DotDel(Dot *root)
{
  if (root) {
    QueDel(root,TRUE);
    Free(root);
  }
}

U0 DotRst(Dot *root)
{
  Dot *tempd;
  if (root) {
    Bts(root->flags,Df_PROCESSED);
    tempd=root->next;
    while (tempd!=root) {
      Btr(tempd->flags,Df_PROCESSED);
      tempd=tempd->next;
    }
  }
}

#define DIR_LT	0
#define DIR_CT	1
#define DIR_RT	2
#define DIR_LC	3
#define DIR_CC	4
#define DIR_RC	5
#define DIR_LB	6
#define DIR_CB	7
#define DIR_RB	8

I64 ElementSqrDist(Element *tempe,I64 x,I64 y)
{
  I64 i,dx,dy;
  if (x<tempe->min_x)
    i=0;
  else if (x<=tempe->max_x)
    i=1;
  else
    i=2;
  if (y>=tempe->min_y) {
    if (y<=tempe->max_y)
      i+=3;
    else
      i+=6;
  }
  switch (i) {
    case DIR_LT:
      dx=tempe->min_x-x;
      dy=tempe->min_y-y;
      break;
    case DIR_CT:
      dx=0;
      dy=tempe->min_y-y;
      break;
    case DIR_RT:
      dx=x-tempe->max_x;
      dy=tempe->min_y-y;
      break;
    case DIR_LC:
      dx=tempe->min_x-x;
      dy=0;
      break;
    case DIR_CC:
      dx=0;
      dy=0;
      break;
    case DIR_RC:
      dx=x-tempe->max_x;
      dy=0;
      break;
    case DIR_LB:
      dx=tempe->min_x-x;
      dy=y-tempe->max_y;
      break;
    case DIR_CB:
      dx=0;
      dy=y-tempe->max_y;
      break;
    case DIR_RB:
      dx=x-tempe->max_x;
      dy=y-tempe->max_y;
      break;
  }
  return SqrI64(dx)+SqrI64(dy);
}

#define SPACING 10

F64 DotNetScore(Dot *root)
{ //higher is better
  F64 d,distdist1=0.1,distdist2=0.1,num=1.0,den=0.1;
  I64 dx,dy,nonzero_cnt=1;
  Element *tempe;
  Dot *tempd,*tempd1;
  if (root) {
    tempd=root->next;
    while (tempd!=root) {

      //Calculate clearance from elements like parallel resistors
      tempe=el_root.next;
      while (tempe!=&el_root) {
	if (tempd->e!=tempe) {
	  d=Sqrt(ElementSqrDist(tempe,tempd->x,tempd->y))+0.1;
	  if (d>SPACING) d=SPACING;
	  num*=d;
	  den+=d;
	}
	tempe=tempe->next;
      }

      //Calculate clearance from junctions like parallel resistors
      if (Bt(tempd->flags,Df_TERMINAL)) {
	tempd1=root->next;
	while (tempd1!=root) {
	  if (tempd1!=tempd && Bt(tempd1->flags,Df_JUNCTION)) {
	    d=Sqrt(SqrI64(tempd->x-tempd1->x)+SqrI64(tempd->y-tempd1->y))+0.1;
	    if (d>SPACING) d=SPACING;
	    num*=d;
	    den+=d;
	  }
	  tempd1=tempd1->next;
	}
      }

      if (tempd1=tempd->left) {
	dx=SqrI64(tempd->x-tempd1->x);
	dy=SqrI64(tempd->y-tempd1->y);
	if (d=dx+dy) {
	  distdist1+=d;
	  nonzero_cnt++;
	}
	distdist2+=dy;
      }
      if (tempd1=tempd->right) {
	dx=SqrI64(tempd->x-tempd1->x);
	dy=SqrI64(tempd->y-tempd1->y);
	if (d=dx+dy) {
	  distdist1+=d;
	  nonzero_cnt++;
	}
	distdist2+=dy;
      }
      if (tempd1=tempd->top) {
	dx=SqrI64(tempd->x-tempd1->x);
	dy=SqrI64(tempd->y-tempd1->y);
	if (d=dx+dy) {
	  distdist1+=d;
	  nonzero_cnt++;
	}
	distdist2+=dx;
      }
      if (tempd1=tempd->bottom) {
	dx=SqrI64(tempd->x-tempd1->x);
	dy=SqrI64(tempd->y-tempd1->y);
	if (d=dx+dy) {
	  distdist1+=d;
	  nonzero_cnt++;
	}
	distdist2+=dx;
      }
      tempd=tempd->next;
    }
  }
  return num/(nonzero_cnt*nonzero_cnt*nonzero_cnt*distdist1*distdist2);
}

Bool NodeAdd(Node *tempn,Element *tempe,I64 terminal,CTask *mem_task=NULL)
{
  I64 i;
  Terminal *a;
  for (i=0;i<tempn->cnt;i++)
    if (tempn->terms[i].e==tempe&&tempn->terms[i].terminal==terminal)
      return TRUE;
  a=MAlloc((tempn->cnt+1)*sizeof(Terminal),mem_task);
  if (tempn->terms) {
    MemCpy(a,tempn->terms,tempn->cnt*sizeof(Terminal));
    Free(tempn->terms);
  }
  tempn->terms=a;
  i=tempn->cnt++;
  tempn->terms[i].e=tempe;
  tempn->terms[i].terminal=terminal;
  return FALSE;
}

U0 NodeDel(Node *tempn)
{
  if (tempn) {
    QueRem(tempn);
    Free(tempn->terms);
    if (tempn->dot_root)
      DotDel(tempn->dot_root);
    Free(tempn);
  }
}

extern Dot *DotNew(Node *tempn,CTask *mem_task=NULL);

U0 NodeReroute(Node *tempn=NULL,CTask *mem_task=NULL)
{
  if (tempn) {
    DotDel(tempn->dot_root);
    tempn->dot_root=DotNew(tempn,mem_task);
  } else {
    tempn=node_root.next;
    while (tempn!=&node_root) {
      DotDel(tempn->dot_root);
      tempn->dot_root=DotNew(tempn,mem_task);
      tempn=tempn->next;
    }
  }
}

U0 ElementExtents(Element *tempe)
{
  ElementType *tempt=tempe->type;
  I64 x,y;
  x=tempt->min_x; y=tempt->min_y;
  OrthRot(&x,&y,tempe->é);
  x+=tempe->x;	  y+=tempe->y;
  tempe->min_x=tempe->max_x=x;
  tempe->min_y=tempe->max_y=y;
  x=tempt->max_x; y=tempt->max_y;
  OrthRot(&x,&y,tempe->é);
  x+=tempe->x;	  y+=tempe->y;
  if (x<tempe->min_x) tempe->min_x=x;
  if (x>tempe->max_x) tempe->max_x=x;
  if (y<tempe->min_y) tempe->min_y=y;
  if (y>tempe->max_y) tempe->max_y=y;
  if (tempe==rubber_band) {
    tempe->min_x+=ipx-rubber_band_x;
    tempe->max_x+=ipx-rubber_band_x;
    tempe->min_y+=ipy-rubber_band_y;
    tempe->max_y+=ipy-rubber_band_y;
  }
}

U0 ElementReroute(Element *tempe,CTask *mem_task=NULL)
{
  I64 i;
  ElementType *tempt;
  if (tempe) {
    tempt=tempe->type;
    for (i=0;i<tempt->terminals;i++)
      if (tempe->nodes[i])
	NodeReroute(tempe->nodes[i],mem_task);
  }
}

U0 ElementDel(Element *tempe,CTask *mem_task=NULL)
{
  I64 i,cnt;
  Terminal *src,*dst;
  Node *tempn=node_root.next,*tempn1;
  while (tempn!=&node_root) {
    tempn1=tempn->next;
    cnt=0;
    dst=src=tempn->terms;
    for (i=0;i<tempn->cnt;i++) {
      if (src->e!=tempe) {
	if (dst!=src)
	  MemCpy(dst,src,sizeof(Terminal));
	cnt++;
	dst++;
      }
      src++;
    }
    if (tempn->cnt!=cnt) {
      tempn->cnt=cnt;
      if (!cnt)
	NodeDel(tempn);
      else if (cnt==1) {
	tempn->terms[0].e->nodes[tempn->terms[0].terminal]=NULL;
	NodeDel(tempn);
      } else {
	DotDel(tempn->dot_root);
	tempn->dot_root=NULL;
	NodeReroute(tempn,mem_task);
      }
    }
    tempn=tempn1;
  }
  QueRem(tempe);
  Free(tempe);
}

Node *NodeNew(CTask *mem_task=NULL)
{
  Node *result=CAlloc(sizeof(Node),mem_task);
  QueIns(result,node_root.last);
  return result;
}

Node *NodeCat(Node *dst,Node *src,CTask *mem_task=NULL)
{
  I64 i;
  if (!dst) dst=NodeNew(mem_task);
  if (src && dst!=src) {
    for (i=0;i<src->cnt;i++) {
      NodeAdd(dst,src->terms[i].e,src->terms[i].terminal,mem_task);
      src->terms[i].e->nodes[src->terms[i].terminal]=dst;
    }
    NodeDel(src);
  }
  return dst;
}

U0 TerminalXY(Element *tempe,I64 terminal,I64 *_x,I64 *_y)
{
  ElementType *tempt=tempe->type;
  I64 x=tempt->term[terminal].x,y=tempt->term[terminal].y;
  OrthRot(&x,&y,tempe->é);
  if (_x)
    *_x=tempe->x+x;
  if (_y)
    *_y=tempe->y+y;
}

#define CT_NONE		0
#define CT_LEFT		1
#define CT_RIGHT	2
#define CT_TOP		3
#define CT_BOTTOM	4

I64 DotHVDecideConnect(Dot *tempd,Dot *tempd1)
{
  I64 d;
  d=tempd->x-tempd1->x;
  if (d<0) {
    if (!tempd->right  && !tempd1->left ||
	 tempd->right==tempd1  && tempd1->left==tempd)
      return CT_RIGHT;
  } else if (d>0) {
    if (!tempd->left   && !tempd1->right ||
	 tempd->left==tempd1   && tempd1->right==tempd)
      return CT_LEFT;
  }
  d=tempd->y-tempd1->y;
  if (d<0) {
    if (!tempd->bottom && !tempd1->top ||
	 tempd->bottom==tempd1 && tempd1->top==tempd)
      return CT_BOTTOM;
  } else if (d>0) {
    if (!tempd->top    && !tempd1->bottom ||
	 tempd->top==tempd1    && tempd->bottom==tempd)
      return CT_TOP;
  }
  return CT_NONE;
}

Bool DotHVConnect(Dot *tempd,Dot *tempd1)
{
  switch (DotHVDecideConnect(tempd,tempd1)) {
    case CT_RIGHT:
      tempd ->right=tempd1;
      tempd1->left =tempd;
      return TRUE;
    case CT_LEFT:
      tempd ->left =tempd1;
      tempd1->right=tempd;
      return TRUE;
    case CT_BOTTOM:
      tempd ->bottom=tempd1;
      tempd1->top   =tempd;
      return TRUE;
    case CT_TOP:
      tempd ->top   =tempd1;
      tempd1->bottom=tempd;
      return TRUE;
    default:
      return FALSE;
  }
}

Dot *DotHVInterconnect(Dot *tempd,Dot *tempd1,CTask *mem_task=NULL)
{
  I64 type;
  Dot *result=CAlloc(sizeof(Dot),mem_task);
  if (Bt(tempd ->flags,Df_TERMINAL_LEFT)||
      Bt(tempd ->flags,Df_TERMINAL_RIGHT)) {	 //d horizontal
    type=0;
    if (Bt(tempd1->flags,Df_TERMINAL_TOP)||
	Bt(tempd1->flags,Df_TERMINAL_BOTTOM)) {//d1 vertical
      if (tempd->y<tempd1->y && Bt(tempd1->flags,Df_TERMINAL_TOP)||
	  tempd->y>tempd1->y && Bt(tempd1->flags,Df_TERMINAL_BOTTOM))
	type=1;
    } else {
      if (tempd->x>tempd1->x && Bt(tempd1->flags,Df_TERMINAL_LEFT)||
	  tempd->x<tempd1->x && Bt(tempd1->flags,Df_TERMINAL_RIGHT))
	type=1;
    }
  } else {				     //d vertical
    type=1;
    if (Bt(tempd1->flags,Df_TERMINAL_LEFT)||
	Bt(tempd1->flags,Df_TERMINAL_RIGHT)) { //d1 horizontal
      if (tempd->x<tempd1->x && Bt(tempd1->flags,Df_TERMINAL_LEFT)||
	  tempd->x>tempd1->x && Bt(tempd1->flags,Df_TERMINAL_RIGHT))
	type=0;
    } else {
      if (tempd->y>tempd1->y && Bt(tempd1->flags,Df_TERMINAL_TOP)||
	  tempd->y<tempd1->y && Bt(tempd1->flags,Df_TERMINAL_BOTTOM))
	type=0;
    }
  }
  if (type==1) {
    result->x=tempd1->x;
    result->y=tempd ->y;
  } else {
    result->x=tempd ->x;
    result->y=tempd1->y;
  }
  return result;
}

Dot *DotNearest(Dot *root,Dot *tempd,Bool just_processed)
{
  I64 best_dd=MAX_I64,dx,dy,dd;
  Dot *result=NULL,*tempd1,*tempd2;
  if (root && tempd) {
    tempd1=root->next;
    while (tempd1!=root) {
      if (tempd1!=tempd && (!just_processed||Bt(tempd1->flags,Df_PROCESSED))) {
	dx=tempd->x-tempd1->x;
	dy=tempd->y-tempd1->y;
	dd=SqrI64(dx)+SqrI64(dy);
	if (0<dd<best_dd) {
	  if (dx && dy) {
	    tempd2=DotHVInterconnect(tempd,tempd1);
	    if (DotHVDecideConnect(tempd ,tempd2) &&
		DotHVDecideConnect(tempd2,tempd1)) {
	      best_dd=dd;
	      result=tempd1;
	    }
	    Free(tempd2);
	  } else {
	    if (DotHVDecideConnect(tempd ,tempd1)) {
	      best_dd=dd;
	      result=tempd1;
	    }
	  }
	}
      }
      tempd1=tempd1->next;
    }
  }
  return result;
}

Bool DotMarkConnected(Dot *tempd)
{
  if (tempd && !Bts(tempd->flags,Df_PROCESSED)) {
    DotMarkConnected(tempd->left);
    DotMarkConnected(tempd->right);
    DotMarkConnected(tempd->top);
    DotMarkConnected(tempd->bottom);
    return TRUE;
  } else
    return FALSE;
}

U0 DotSpacersAdd(Dot *root,CTask *mem_task=NULL)
{
  Dot *tempd,*tempd1,*tempd2;
  if (root) {
    tempd=root->next;
    if (tempd!=root) {
      tempd2=root->last;
      do {
	if (Bt(tempd->flags,Df_TERMINAL)) {
	  tempd1=CAlloc(sizeof(Dot),mem_task);
	  tempd1->x=tempd->x;
	  tempd1->y=tempd->y;
	  tempd1->flags[0]=tempd->flags[0];
	  tempd1->e=tempd->e;
	  if (Bt(tempd->flags,Df_TERMINAL_LEFT)) {
	    tempd->right=tempd1;
	    tempd1->left=tempd;
	    tempd->x--;
	  }
	  if (Bt(tempd->flags,Df_TERMINAL_RIGHT)) {
	    tempd->left=tempd1;
	    tempd1->right=tempd;
	    tempd->x++;
	  }
	  if (Bt(tempd->flags,Df_TERMINAL_TOP)) {
	    tempd->bottom=tempd1;
	    tempd1->top=tempd;
	    tempd->y--;
	  }
	  if (Bt(tempd->flags,Df_TERMINAL_BOTTOM)) {
	    tempd->top=tempd1;
	    tempd1->bottom=tempd;
	    tempd->y++;
	  }
	  tempd->flags[0]=0;
	  tempd->e=NULL;
	  QueIns(tempd1,root->last);
	}
	tempd=tempd->next;
      } while (tempd!=tempd2);
    }
  }
}

U0 DotAddSpace(Dot *root)
{
  F64 score,best_score;
  I64 i,x,y,best_x,best_y;
  Dot *tempd,*tempd1;
  if (root) {
    tempd=root->next;
    while (tempd!=root) {
      tempd->saved_x=tempd->x;
      tempd->saved_y=tempd->y;
      tempd=tempd->next;
    }
    best_score=DotNetScore(root);
    tempd=root->next;
    while (tempd!=root) {
      if (!Bt(tempd->flags,Df_TERMINAL)) {
	best_x=0;
	best_y=0;
	for (i=0;i<4*SPACING;i++) {
	  nobound_switch (i&3) {
	    case A_EAST:  x= i>>2; y=0; break;
	    case A_NORTH: y= i>>2; x=0; break;
	    case A_WEST:  x=-i>>2; y=0; break;
	    case A_SOUTH: y=-i>>2; x=0; break;
	  }
	  tempd->x=tempd->saved_x+x;
	  tempd1=tempd->top;
	  while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	    tempd1->x=tempd1->saved_x+x;
	    tempd1=tempd1->top;
	  }
	  tempd1=tempd->bottom;
	  while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	    tempd1->x=tempd1->saved_x+x;
	    tempd1=tempd1->bottom;
	  }
	  tempd->y=tempd->saved_y+y;
	  tempd1=tempd->left;
	  while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	    tempd1->y=tempd1->saved_y+y;
	    tempd1=tempd1->left;
	  }
	  tempd1=tempd->right;
	  while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	    tempd1->y=tempd1->saved_y+y;
	    tempd1=tempd1->right;
	  }
	  score=DotNetScore(root);
	  if (score>best_score) {
	    best_score=score;
	    best_x=x;
	    best_y=y;
	  }
	}
	tempd->saved_x=tempd->x=tempd->saved_x+best_x;
	tempd1=tempd->top;
	while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	  tempd1->saved_x=tempd1->x=tempd1->saved_x+best_x;
	  tempd1=tempd1->top;
	}
	tempd1=tempd->bottom;
	while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	  tempd1->saved_x=tempd1->x=tempd1->saved_x+best_x;
	  tempd1=tempd1->bottom;
	}
	tempd->saved_y=tempd->y=tempd->saved_y+best_y;
	tempd1=tempd->left;
	while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	  tempd1->saved_y=tempd1->y=tempd1->saved_y+best_y;
	  tempd1=tempd1->left;
	}
	tempd1=tempd->right;
	while (tempd1 && !Bt(tempd1->flags,Df_TERMINAL)) {
	  tempd1->saved_y=tempd1->y=tempd1->saved_y+best_y;
	  tempd1=tempd1->right;
	}
      }
      tempd=tempd->next;
    }
  }
}

U0 DotJunctions(Dot *root)
{
  I64 cnt;
  Dot *tempd;
  tempd=root->next;
  while (tempd!=root) {
    if (Bt(tempd->flags,Df_TERMINAL))
      cnt=1;
    else
      cnt=0;
    if (tempd->left)   cnt++;
    if (tempd->right)  cnt++;
    if (tempd->top)    cnt++;
    if (tempd->bottom) cnt++;
    BEqu(tempd->flags,Df_JUNCTION,cnt>2);
    tempd=tempd->next;
  }
}

Dot *DotTerminalsNew(Node *tempn,CTask *mem_task=NULL)
{
  I64 i,min_x,max_x,min_y,max_y,dx,dy,x,y;
  Element *tempe;
  Dot *result=CAlloc(sizeof(Dot),mem_task),*tempd;
  QueInit(result);
  for (i=0;i<tempn->cnt;i++)
    if (tempe=tempn->terms[i].e) {
      tempd=CAlloc(sizeof(Dot),mem_task);
      Bts(tempd->flags,Df_TERMINAL);
      tempd->e=tempe;
      TerminalXY(tempe,tempn->terms[i].terminal,&tempd->x,&tempd->y);
      x=tempe->x;
      y=tempe->y;
      if (mode==MD_MOVE && tempe==rubber_band) {
	tempd->x+=ipx-rubber_band_x;
	tempd->y+=ipy-rubber_band_y;
	x+=ipx-rubber_band_x;
	y+=ipy-rubber_band_y;
      }

      min_x=AbsI64(tempd->x-tempe->min_x);
      max_x=AbsI64(tempd->x-tempe->max_x);
      if (min_x<max_x)
	dx=min_x;
      else
	dx=max_x;

      min_y=AbsI64(tempd->y-tempe->min_y);
      max_y=AbsI64(tempd->y-tempe->max_y);
      if (min_y<max_y)
	dy=min_y;
      else
	dy=max_y;

      if (dx<=dy) {
	if (tempd->x<=x)
	  Bts(tempd->flags,Df_TERMINAL_LEFT);
	else
	  Bts(tempd->flags,Df_TERMINAL_RIGHT);
      } else {
	if (tempd->y<=y)
	  Bts(tempd->flags,Df_TERMINAL_TOP);
	else
	  Bts(tempd->flags,Df_TERMINAL_BOTTOM);
      }
      QueIns(tempd,result->last);
    }
  return result;
}

Dot *DotNew(Node *tempn,CTask *mem_task=NULL)
{
  I64 cnt,pass=0,last_cnt=0;
  Bool cont;
  Dot *tempd,*tempd1,*tempd2,*tempd3;
  Dot *result=DotTerminalsNew(tempn,mem_task);
  do {
    pass++;
    tempd=result->next;
    while (tempd!=result) {
      if (!Bt(tempd->flags,Df_PROCESSED)) {
	if (tempd1=DotNearest(result,tempd,pass>1)) {
	  if (tempd->x==tempd1->x || tempd->y==tempd1->y) {
	    if (tempd->x!=tempd1->x || tempd->y!=tempd1->y) {
	      if (DotHVConnect(tempd,tempd1))
		DotMarkConnected(tempd);
	    }
	  }else {
	    tempd2=DotHVInterconnect(tempd,tempd1,mem_task);
	    tempd3=result->next;
	    while (tempd3!=result) {
	      if (tempd2->x==tempd3->x && tempd2->y==tempd3->y)
		break;
	      tempd3=tempd3->next;
	    }
	    if (tempd3!=result) {
	      Free(tempd2);
	      tempd2=tempd3;
	    } else
	      QueIns(tempd2,result->last);

	    if (DotHVConnect(tempd,tempd2)) {
	      DotMarkConnected(tempd);
	      if (DotHVConnect(tempd2,tempd1))
		DotMarkConnected(tempd1);
	    } else
	      DotHVConnect(tempd2,tempd1);
	  }
	}
      }
      tempd=tempd->next;
    }

    tempd=result->next;
    while (tempd!=result) {
      tempd1=tempd->next;
      while (tempd1!=result) {
	if (tempd1->y==tempd->y) {
	  if ((tempd2=tempd->left  ) && tempd2->x<tempd1->x<tempd ->x) {
	    tempd ->left =tempd1;
	    tempd1->right=tempd ;
	    tempd1->left =tempd2;
	    tempd2->right=tempd1;
	  }
	  if ((tempd2=tempd->right ) && tempd ->x<tempd1->x<tempd2->x) {
	    tempd2->left =tempd1;
	    tempd1->right=tempd2;
	    tempd1->left =tempd ;
	    tempd ->right=tempd1;
	  }
	}
	if (tempd1->x==tempd->x) {
	  if ((tempd2=tempd->top   ) && tempd2->y<tempd1->y<tempd ->y) {
	    tempd ->top	=tempd1;
	    tempd1->bottom=tempd ;
	    tempd1->top	=tempd2;
	    tempd2->bottom=tempd1;
	  }
	  if ((tempd2=tempd->bottom) && tempd ->y<tempd1->y<tempd2->y) {
	    tempd2->top	=tempd1;
	    tempd1->bottom=tempd2;
	    tempd1->top	=tempd ;
	    tempd ->bottom=tempd1;
	  }
	}
	tempd1=tempd1->next;
      }
      tempd=tempd->next;
    }

    cont=FALSE;
    DotRst(result);
    cnt=0;
    tempd=result->next;
    if (tempd!=result) {
      DotMarkConnected(tempd);
      while (tempd!=result) {
	if (Bt(tempd->flags,Df_PROCESSED))
	  cnt++;
	else
	  cont=TRUE;
	tempd=tempd->next;
      }
      if (last_cnt==cnt)
	break;
      last_cnt=cnt;
    }
  } while (cont);
  DotJunctions(result);
  DotSpacersAdd(result,mem_task);
  DotAddSpace(result);
  DotJunctions(result);
  return result;
}

U0 DotDraw(CDC *dc,Dot *root)
{
  Dot *tempd,*tempd1;
  if (root) {
    tempd=root->next;
    while (tempd!=root) {
      Bts(tempd->flags,Df_PROCESSED);
      if ((tempd1=tempd->right)  && !Bt(tempd1->flags,Df_PROCESSED))
	GrLine(dc,tempd->x,tempd->y,tempd1->x,tempd1->y);
      if ((tempd1=tempd->left)	 && !Bt(tempd1->flags,Df_PROCESSED))
	GrLine(dc,tempd->x,tempd->y,tempd1->x,tempd1->y);
      if ((tempd1=tempd->top)	 && !Bt(tempd1->flags,Df_PROCESSED))
	GrLine(dc,tempd->x,tempd->y,tempd1->x,tempd1->y);
      if ((tempd1=tempd->bottom) && !Bt(tempd1->flags,Df_PROCESSED))
	GrLine(dc,tempd->x,tempd->y,tempd1->x,tempd1->y);
      if (Bt(tempd->flags,Df_JUNCTION)) {
	dc->pen_width=3;
	GrPlot3(dc,tempd->x,tempd->y,0);
	dc->pen_width=1;
      }
      tempd=tempd->next;
    }
  }
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,x1,y1,x2,y2;
  U8 *img;
  Bool old_preempt=Preempt(OFF);
  Node *tempn=node_root.next;
  Element *tempe=el_root.next;
  while (tempe!=&el_root) {
    x1=tempe->x;
    y1=tempe->y;
    if (mode==MD_MOVE && tempe==rubber_band) {
      img=tempe->type->red_img;
      x1+=ipx-rubber_band_x;
      y1+=ipy-rubber_band_y;
    } else
      img=tempe->type->black_img;
    SpritePlotRotZ3b(dc,x1,y1,0,img,tempe->é*pi/2);
    tempe=tempe->next;
  }
  while (tempn!=&node_root) {
    dc->color=BLUE;
    if (mode==MD_MOVE)
      for (i=0;i<tempn->cnt;i++)
	if (tempn->terms[i].e==rubber_band) {
	  dc->color=RED;
	  break;
	}
    DotRst(tempn->dot_root);
    DotDraw(dc,tempn->dot_root);
    tempn=tempn->next;
  }
  if (mode==MD_CONNECT && rubber_band && rubber_band_terminal>=0) {
    TerminalXY(rubber_band,rubber_band_terminal,&x1,&y1);
    x2=ipx-task->win_pixel_left-task->win_scroll_x;
    y2=ipy-task->win_pixel_top -task->win_scroll_y;
    dc->color=RED;
    GrLine(dc,x1,y1,x2,y2);
  }
  Preempt(old_preempt);

  dc->color=BLACK;
  GrPutS(dc,0,0,"Right Mouse Change Modes");
  switch (mode) {
    case MD_CONNECT:
      GrPutS(dc,20,FONT_HEIGHT,"Connect Components");
      break;
    case MD_MOVE:
      GrPutS(dc,20,FONT_HEIGHT,"Move Components");
      break;
  }
}

U0 RubberReset()
{
  rubber_band=NULL; rubber_band_terminal=-1;
  rubber_band_x=0; rubber_band_y=0;
}

U0 Init()
{
  I64 i;
  ElementType *tempt;
  for (i=0;i<ET_NUM_TYPES;i++) {
    tempt=&et[i];
    SpriteExtents(tempt->black_img,&tempt->min_x,&tempt->max_x,&tempt->min_y,&tempt->max_y);
  }
  mode=MD_CONNECT;
  RubberReset;
  QueInit(&el_root);
  QueInit(&node_root);
}

U0 CleanUp()
{
  Element *tempe=el_root.next,*tempe1;
  Node *tempn=node_root.next,*tempn1;
  while (tempe!=&el_root) {
    tempe1=tempe->next;
    QueRem(tempe);
    Free(tempe);
    tempe=tempe1;
  }
  while (tempn!=&node_root) {
    tempn1=tempn->next;
    NodeDel(tempn);
    tempn=tempn1;
  }
}

Element *ElementNew(I64 t,CTask *mem_task=NULL)
{
  Element *tempe=CAlloc(sizeof(Element),mem_task);
  tempe->type=&et[t];
  tempe->x=ipx-Fs->win_pixel_left-Fs->win_scroll_x;
  tempe->y=ipy-Fs->win_pixel_top -Fs->win_scroll_y;
  QueIns(tempe,el_root.last);
  ElementExtents(tempe);
  return tempe;
}

Element *ElementFind(I64 x,I64 y,I64 *_terminal)
{
  I64 i,x2,y2,best_dd=MAX_I64,dd;
  Element *result=NULL,*tempe=el_root.next;
  if (_terminal) *_terminal=0;
  while (tempe!=&el_root) {
    for (i=0;i<tempe->type->terminals;i++) {
      TerminalXY(tempe,i,&x2,&y2);
      dd=SqrI64(x-x2)+SqrI64(y-y2);
      if (dd<best_dd) {
	best_dd=dd;
	result=tempe;
	if (_terminal) *_terminal=i;
      }
    }
    tempe=tempe->next;
  }
  return result;
}

U0 Circuit()
{
  I64 p1,p2,terminal1=-1,terminal2=-1;
  Element *tempe1=NULL,*tempe2=NULL;
  Node *tempn;

  PopUpOk(
	"This is not useful.\n"
	"It just lets you draw\n"
	"a circuit.\n");

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Preempt(OFF);
  WordStat;
  WinMax;
  MenuPush(
"File {"
"  New(,'\n');"
"  Abort(,CH_SHIFT_ESC);"
"  Exit(,CH_ESC);"
"}"
"Edit {"
"  Rotate(,CH_SPACE);"
"  DelElement(,,SC_DELETE);"
"}"
"Mode {"
"  Connect(,0,SC_F2);"
"  Move(,0,SC_F3);"
"}"
"PlaceElement {"
"  Resistor(,'r');"
"  Inductor(,'l');"
"  Capacitor(,'c');"
"}"
  );
  Init;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU-WIF_GRAB_SCROLL;
  try {
    while (TRUE) {
      switch (GetMsg(&p1,&p2,1<<MSG_KEY_DOWN+1<<MSG_IP_MOVE+
			     1<<MSG_IP_L_DOWN+1<<MSG_IP_L_UP+
			     1<<MSG_IP_R_DOWN)) {
	case MSG_KEY_DOWN:
	  switch (p1) {
	    case 0:
	      switch (p2.u8[0]) {
		case SC_F2:
		  mode=MD_CONNECT;
		  tempe1=NULL; terminal1=-1;
		  tempe2=NULL; terminal2=-1;
		  RubberReset;
		  break;
		case SC_F3:
		  mode=MD_MOVE;
		  tempe1=NULL; terminal1=-1;
		  tempe2=NULL; terminal2=-1;
		  RubberReset;
		  break;
		case SC_DELETE:
		  tempe1=ElementFind(ipx-Fs->win_pixel_left-Fs->win_scroll_x,
				     ipy-Fs->win_pixel_top -Fs->win_scroll_y,&terminal1);
		  ElementDel(tempe1,Fs);
		  tempe1=NULL; terminal1=-1;
		  tempe2=NULL; terminal2=-1;
		  RubberReset;
		  break;
	      }
	      break;
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto cs_done;
	    case 'r':
	      tempe1=ElementNew(ET_R);
	      terminal1=-1;
	      break;
	    case 'l':
	      tempe1=ElementNew(ET_L);
	      terminal1=-1;
	      break;
	    case 'c':
	      tempe1=ElementNew(ET_C);
	      terminal1=-1;
	      break;
	    case CH_SPACE:
	      if (tempe1) {
		tempe1->é=(tempe1->é+1)&3;
		ElementExtents(tempe1);
		ElementReroute(tempe1,Fs);
	      }
	      break;
	  }
	  break;
	case MSG_IP_MOVE:
	  if (tempe1) {
	    ElementExtents(tempe1);
	    ElementReroute(tempe1,Fs);
	  }
	  break;
	case MSG_IP_L_DOWN:
	  tempe1=ElementFind(p1,p2,&terminal1);
	  rubber_band=tempe1; rubber_band_terminal=terminal1;
	  rubber_band_x=ipx; rubber_band_y=ipy;
	  break;
	case MSG_IP_L_UP:
	  rubber_band=NULL;
	  switch (mode) {
	    case MD_CONNECT:
	      if (tempe1 && terminal1>=0) {
		if ((tempe2=ElementFind(p1,p2,&terminal2)) &&
		    (tempe2!=tempe1 || terminal2!=terminal1)) {
		  if (!(tempn=tempe1->nodes[terminal1]) &&
		      !(tempn=tempe2->nodes[terminal2]))
		    tempn=NodeNew;

		  NodeAdd(tempn,tempe1,terminal1);
		  NodeAdd(tempn,tempe2,terminal2);

		  if (tempe1->nodes[terminal1])
		    NodeCat(tempn,tempe1->nodes[terminal1]);
		  tempe1->nodes[terminal1]=tempn;

		  if (tempe2->nodes[terminal2])
		    NodeCat(tempn,tempe2->nodes[terminal2]);
		  tempe2->nodes[terminal2]=tempn;

		  NodeReroute(tempn,Fs);
		}
	      }
	      break;
	    case MD_MOVE:
	      if (tempe1 && terminal1>=0) {
		tempe1->x+=ipx-rubber_band_x;
		tempe1->y+=ipy-rubber_band_y;
		ElementExtents(tempe1);
		ElementReroute(tempe1,Fs);
	      }
	      break;
	  }
	  tempe1=NULL; terminal1=-1;
	  tempe2=NULL; terminal2=-1;
	  RubberReset;
	  break;
	case MSG_IP_R_DOWN:
	  if (mode+1==MD_NUM_MODES)
	    mode=0;
	  else
	    mode++;
	  tempe1=NULL; terminal1=-1;
	  tempe2=NULL; terminal2=-1;
	  RubberReset;
	  break;
      }
    }
cs_done:
  } catch
    CatchAll;
  SettingsPop;
  CleanUp;
  MenuPop;
}

Circuit;
           œ       öÿÿÿ   ùÿÿÿþÿÿÿùÿÿÿþÿÿÿüÿÿÿ   üÿÿÿ   ÿÿÿÿþÿÿÿÿÿÿÿþÿÿÿ         ýÿÿÿ            
               þÿÿÿöÿÿÿ   ôÿÿÿ   
                                  òÿÿÿýÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ     ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ    ÿ  ÿÿ ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ ÿÿÿÿ  ÿÿÿ ÿÿ ÿÿ ÿÿ ÿÿ ÿÿ ÿÿ ÿÿ ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             X       üÿÿÿ    ÿÿÿÿ    ÿÿÿÿ    ÿÿÿÿúÿÿÿÿÿÿÿ    ÿÿÿÿ      úÿÿÿ                               œ      öÿÿÿ   ùÿÿÿþÿÿÿùÿÿÿþÿÿÿüÿÿÿ   üÿÿÿ   ÿÿÿÿþÿÿÿÿÿÿÿþÿÿÿ         ýÿÿÿ            
               þÿÿÿöÿÿÿ   ôÿÿÿ   
                                  òÿÿÿýÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             X      üÿÿÿ    ÿÿÿÿ    ÿÿÿÿ    ÿÿÿÿúÿÿÿÿÿÿÿ    ÿÿÿÿ      úÿÿÿ                     