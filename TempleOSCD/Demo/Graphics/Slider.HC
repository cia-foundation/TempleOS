/*This was done with <CTRL-SHIFT-L> template code.
It's conceptually easy to do a ctrl,
but tedious.  You have complete freedom.

See $LK,"CCtrl",A="MN:CCtrl"$.
*/

//Start of autocreated Code
#define SLIDER_RANGE   30
#define SLIDER_SPACING 20
#define SLIDER_BORDER  2

class CSliderState
{
  I64 left_pos;
  I64 right_pos;
} sld;

U0 DrawCtrlSlider(CDC *dc,CCtrl *c)
{
  CSliderState *s=c->state;

  dc->color=LTRED;
  GrRect(dc, c->left,c->top,SLIDER_SPACING*3+2,SLIDER_SPACING*2+SLIDER_RANGE);
  dc->color=BLUE;
  GrRect(dc, c->left+SLIDER_BORDER,c->top+SLIDER_BORDER,
	SLIDER_SPACING*3+2-2*SLIDER_BORDER,
	SLIDER_SPACING*2+SLIDER_RANGE-2*SLIDER_BORDER);
  dc->color=BLACK;
  GrLine(dc,c->left+1*SLIDER_SPACING+0,c->top+SLIDER_SPACING,
	c->left+1*SLIDER_SPACING+0,c->top+SLIDER_SPACING+SLIDER_RANGE-1);
  GrLine(dc,c->left+2*SLIDER_SPACING+1,c->top+SLIDER_SPACING,
	c->left+2*SLIDER_SPACING+1,c->top+SLIDER_SPACING+SLIDER_RANGE-1);
  dc->color=LTRED;
  GrPrint(dc,c->left+1*SLIDER_SPACING+0-FONT_WIDTH/2,
	c->top+SLIDER_SPACING+SLIDER_RANGE+3,
	"%d",s->left_pos*10/SLIDER_RANGE);
  GrPrint(dc,c->left+2*SLIDER_SPACING+1-FONT_WIDTH/2,
	c->top+SLIDER_SPACING+SLIDER_RANGE+3,
	"%d",s->right_pos*10/SLIDER_RANGE);
  GrRect(dc,c->left+1*SLIDER_SPACING+0-3,
	c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->left_pos-2,7,5);
  GrRect(dc,c->left+2*SLIDER_SPACING+1-3,
	c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->right_pos-2,7,5);
  dc->color=YELLOW;
  GrRect(dc,c->left+1*SLIDER_SPACING+0-2,
	c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->left_pos-1,5,3);
  GrRect(dc,c->left+2*SLIDER_SPACING+1-2,
	c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->right_pos-1,5,3);
}

U0 UpdateDerivedCtrlSlider(CCtrl *c)
{
  CSliderState *s=c->state;
  c->left=c->win_task->pix_width/2-(SLIDER_SPACING*3+2)/2;
  c->right=c->left+3*SLIDER_SPACING+2;
  c->top=c->win_task->pix_height/2-(SLIDER_SPACING*2+SLIDER_RANGE)/2;
  c->bottom=c->top+SLIDER_SPACING*2+SLIDER_RANGE;
  s->left_pos=ClampI64(s->left_pos,0,SLIDER_RANGE-1);
  s->right_pos=ClampI64(s->right_pos,0,SLIDER_RANGE-1);
}

U0 LeftClickSlider(CCtrl *c,I64 x,I64 y,Bool)
{
  CSliderState *s=c->state;
  if (x<c->left+1*SLIDER_SPACING+0+SLIDER_SPACING/2)
    s->left_pos=SLIDER_RANGE-1-(y-(c->top+SLIDER_SPACING));
  else
    s->right_pos=SLIDER_RANGE-1-(y-(c->top+SLIDER_SPACING));
  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

CCtrl *SliderNew()
{
  CCtrl *c=CAlloc(sizeof(CCtrl));
  c->win_task=Fs;
  c->flags=CTRLF_SHOW|CTRLF_CAPTURE_LEFT_MS;
  c->type=CTRLT_GENERIC;
  c->state=&sld;
  MemSet(&sld,0,sizeof(CSliderState));
  c->draw_it=&DrawCtrlSlider;
  c->left_click=&LeftClickSlider;
  c->update_derived_vals=&UpdateDerivedCtrlSlider;
  QueIns(c,Fs->last_ctrl);
  TaskDerivedValsUpdate;
  return c;
}

U0 SliderDel(CCtrl *c)
{
  QueRem(c);
  Free(c);
}

//End of autocreated Code

U0 Main()
{
  "This is to demo ctrls.\n";
  CCtrl *c=SliderNew;
  PressAKey;
  "Left: %d  Right: %d\n",
	sld.left_pos*10/SLIDER_RANGE,
	sld.right_pos*10/SLIDER_RANGE;
  SliderDel(c);
  DocBottom;
}

Main;
