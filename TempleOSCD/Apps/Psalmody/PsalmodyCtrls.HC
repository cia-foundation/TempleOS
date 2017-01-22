#define TEMPO_SPACING 15
#define TEMPO_RANGE   80
#define TEMPO_BORDER  2

class TempoState
{
  I64 tempo,stacatto;
} tempo_state;

U0 DrawTempoCtrl(CDC *dc,CCtrl *c)
{
  TempoState *s=c->state;

  dc->color=LTGREEN;
  GrRect(dc, c->left,c->top,TEMPO_SPACING*3+2,TEMPO_SPACING*2+TEMPO_RANGE);
  dc->color=BLACK;
  GrRect(dc, c->left+TEMPO_BORDER,c->top+TEMPO_BORDER,
	TEMPO_SPACING*3+2-2*TEMPO_BORDER,
	TEMPO_SPACING*2+TEMPO_RANGE-2*TEMPO_BORDER);
  dc->color=WHITE;
  GrLine(dc,c->left+TEMPO_SPACING,c->top+TEMPO_SPACING,
	c->left+TEMPO_SPACING,c->top+TEMPO_SPACING+TEMPO_RANGE-1);
  GrLine(dc,c->left+2*TEMPO_SPACING+1,c->top+TEMPO_SPACING,
	c->left+2*TEMPO_SPACING+1,c->top+TEMPO_SPACING+TEMPO_RANGE-1);

  dc->color=LTGREEN;
  GrPrint(dc,c->left+TEMPO_SPACING-FONT_WIDTH/2,
	c->top+TEMPO_SPACING+TEMPO_RANGE+3,"%d",s->tempo*10/TEMPO_RANGE);
  GrPrint(dc,c->left+2*TEMPO_SPACING+1-FONT_WIDTH/2,
	c->top+TEMPO_SPACING+TEMPO_RANGE+3,"%d",
	s->stacatto*10/TEMPO_RANGE);
  GrRect(dc,c->left+TEMPO_SPACING-3,
	c->top+TEMPO_SPACING+TEMPO_RANGE-1-s->tempo-2 ,7,5);
  GrRect(dc,c->left+2*TEMPO_SPACING+1-3,
	c->top+TEMPO_SPACING+TEMPO_RANGE-1-s->stacatto-2,7,5);
  dc->color=YELLOW;
  GrRect(dc,c->left+TEMPO_SPACING-2,
	c->top+TEMPO_SPACING+TEMPO_RANGE-1-s->tempo-1 ,5,3);
  GrRect(dc,c->left+2*TEMPO_SPACING+1-2,
	c->top+TEMPO_SPACING+TEMPO_RANGE-1-s->stacatto-1,5,3);

  dc->color=GREEN;
  GrVPrint(dc,c->left+TEMPO_BORDER+2,c->top+TEMPO_SPACING+2,"Tempo");
  GrVPrint(dc,c->right-TEMPO_BORDER-2-FONT_WIDTH,
	c->top+TEMPO_SPACING+2,"Stacatto");
}

U0 UpdateDerivedTempoCtrl(CCtrl *c)
{
  TempoState *s=c->state;
  c->right=c->left+TEMPO_SPACING*3+2;
  c->bottom=c->top+TEMPO_SPACING*2+TEMPO_RANGE;
  s->tempo =ClampI64(s->tempo,0,TEMPO_RANGE-1);
  s->stacatto=ClampI64(s->stacatto,0,TEMPO_RANGE-1);
}

U0 LeftClickTempo(CCtrl *c,I64 x,I64 y,Bool)
{
  TempoState *s=c->state;
  if (x<(c->right+c->left)/2)
    s->tempo=TEMPO_RANGE-1-(y-(c->top+TEMPO_SPACING));
  else
    s->stacatto=TEMPO_RANGE-1-(y-(c->top+TEMPO_SPACING));
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

CCtrl *TempoNew()
{
  CCtrl *c=CAlloc(sizeof(CCtrl));
  c->win_task=Fs;
  c->flags=CTRLF_SHOW|CTRLF_CAPTURE_LEFT_MS;
  c->type=CTRLT_GENERIC;
  c->state=&tempo_state;
  c->draw_it=&DrawTempoCtrl;
  c->left_click=&LeftClickTempo;
  c->update_derived_vals=&UpdateDerivedTempoCtrl;
  c->left=396;
  c->top=96;
  QueIns(c,Fs->last_ctrl);
  TaskDerivedValsUpdate;
  return c;
}

U0 TempoDel(CCtrl *c)
{
  QueRem(c);
  Free(c);
}
