#define BTTN_BORDER	2

#define BTF_DONT_FREE	1

class CCtrlBttnState
{
  I64 state,num_states,flags;
  U8 *state_texts;
  CColorROPU32 *state_colors;
};

U0 DrawCtrlBttn(CDC *dc,CCtrl *c)
{
  CCtrlBttnState *s=c->state;
  I64 l;
  U8 *st;

  dc->color=BLACK;
  GrRect(dc,c->left,c->top,c->right-c->left+1,c->bottom-c->top+1);
  if (!(st=LstSub(s->state,s->state_texts)))
    st=s->state_texts;
  dc->color=s->state_colors[s->state];
  l=StrLen(st);
  GrRect(dc,c->left+BTTN_BORDER,c->top+BTTN_BORDER,
	c->right-c->left+1-BTTN_BORDER*2,
	c->bottom-c->top+1-BTTN_BORDER*2);
  dc->color=s->state_colors[s->state]^(WHITE<<16+WHITE);
  GrPrint(dc,(c->left+c->right+1-l*FONT_WIDTH)>>1,
	(c->top+c->bottom+1-FONT_HEIGHT)>>1,"%s",st);
}

U0 LeftClickCtrlBttn(CCtrl *c,I64,I64,Bool down)
{
  CCtrlBttnState *s=c->state;
  if (down) {
    s->state++;
    if (s->state==s->num_states)
      s->state=0;
  }
}

public CCtrl *CtrlBttnNew(I64 x,I64 y,I64 width=-1,I64 height=-1,
	I64 num_states=1,U8 *state_texts,
	I32 *state_colors,CCtrlBttnState *_s=NULL)
{//Create bttn ctrl. See $LK,"::/Apps/Strut/Strut.HC"$.
  CCtrl *res;
  CCtrlBttnState *s;
  I64 i,j,l;
  U8 *st;
  if (width<0) {
    l=1;
    for (i=0;i<num_states;i++)
      if (st=LstSub(i,state_texts)) {
	j=StrLen(st);
	if (j>l) l=j;
      }
    width=BTTN_BORDER*4+l*FONT_WIDTH;
  }
  if (height<0) height=BTTN_BORDER*4+FONT_HEIGHT;
  res=CAlloc(sizeof(CCtrl));
  if (_s) {
    s=_s;
    MemSet(s,0,sizeof(CCtrlBttnState));
  } else {
    s=CAlloc(sizeof(CCtrlBttnState));
    s->flags=BTF_DONT_FREE;
  }
  s->num_states=num_states;
  s->state_texts=state_texts;
  s->state_colors=state_colors;
  res->win_task=Fs;
  res->flags=CTRLF_SHOW;
  res->type=CTRLT_GENERIC;
  res->state=s;
  res->draw_it=&DrawCtrlBttn;
  res->left_click=&LeftClickCtrlBttn;
  res->left=x;
  res->top=y;
  res->right=res->left+width-1;
  res->bottom=res->top+height-1;
  QueIns(res,Fs->last_ctrl);
  return res;
}

public U0 CtrlBttnDel(CCtrl *c)
{//Free bttn ctrl.
  QueRem(c);
  if (!(c->flags&BTF_DONT_FREE))
    Free(c->state);
  Free(c);
}
