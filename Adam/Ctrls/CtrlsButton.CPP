#define BUTTON_BORDER	2

#define BTF_DONT_FREE	1

class CCtrlButtonState
{
  I64 state,num_states,flags;
  U8 *state_texts;
  CColorROPU32 *state_colors;
};

U0 DrawCtrlButton(CDC *dc,CCtrl *c)
{
  CCtrlButtonState *s=c->state;
  I64 l;
  U8 *st;

  dc->color=BLACK;
  GrRect(dc,c->left,c->top,c->right-c->left+1,c->bottom-c->top+1);
  if (!(st=LstSub(s->state,s->state_texts)))
    st=s->state_texts;
  dc->color=s->state_colors[s->state];
  l=StrLen(st);
  GrRect(dc,c->left+BUTTON_BORDER,c->top+BUTTON_BORDER,
	c->right-c->left+1-BUTTON_BORDER*2,
	c->bottom-c->top+1-BUTTON_BORDER*2);
  dc->color=s->state_colors[s->state]^(WHITE<<16+WHITE);
  GrPrint(dc,(c->left+c->right+1-l*FONT_WIDTH)>>1,
	(c->top+c->bottom+1-FONT_HEIGHT)>>1,"%s",st);
}

U0 LeftClickCtrlButton(CCtrl *c,I64,I64,Bool down)
{
  CCtrlButtonState *s=c->state;
  if (down) {
    s->state++;
    if (s->state==s->num_states)
      s->state=0;
  }
}

public CCtrl *CtrlButtonNew(I64 x,I64 y,I64 width=-1,I64 height=-1,
	I64 num_states=1,U8 *state_texts,
	I32 *state_colors,CCtrlButtonState *_s=NULL)
{//Create button ctrl. See $LK,"::/Apps/Strut/Strut.CPP"$.
  CCtrl *res;
  CCtrlButtonState *s;
  I64 i,j,l;
  U8 *st;
  if (width<0) {
    l=1;
    for (i=0;i<num_states;i++)
      if (st=LstSub(i,state_texts)) {
	j=StrLen(st);
	if (j>l) l=j;
      }
    width=BUTTON_BORDER*4+l*FONT_WIDTH;
  }
  if (height<0) height=BUTTON_BORDER*4+FONT_HEIGHT;
  res=CAlloc(sizeof(CCtrl));
  if (_s) {
    s=_s;
    MemSet(s,0,sizeof(CCtrlButtonState));
  } else {
    s=CAlloc(sizeof(CCtrlButtonState));
    s->flags=BTF_DONT_FREE;
  }
  s->num_states=num_states;
  s->state_texts=state_texts;
  s->state_colors=state_colors;
  res->win_task=Fs;
  res->flags=CTRLF_SHOW;
  res->type=CTRLT_GENERIC;
  res->state=s;
  res->draw_it=&DrawCtrlButton;
  res->left_click=&LeftClickCtrlButton;
  res->left=x;
  res->top=y;
  res->right=res->left+width-1;
  res->bottom=res->top+height-1;
  QueIns(res,Fs->last_ctrl);
  return res;
}

public U0 CtrlButtonDel(CCtrl *c)
{//Free button ctrl.
  QueRem(c);
  if (!(c->flags&BTF_DONT_FREE))
    Free(c->state);
  Free(c);
}
