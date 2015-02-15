class CTemplateCSSlider
{
  CTemplateCSSlider *next,*last;
  U8 *name;
  I64 num;
};

class CTemplateCS
{
  CTemplateCSSlider *next_slider,*last_slider;
  I64 num_sliders,range,spacing,border;
  U8 *glbl_name;
};

CTemplateCS *CtrlSliderGet()
{
  CTemplateCSSlider *temps;
  U8 *st,pmt[136];
  CTemplateCS *result=CAlloc(sizeof(CTemplateCS));

  "$$PURPLE$$Ctrl Slider$$FG$$\n\n";

  result->glbl_name=GetStr("Glbl Struct Name (ENTER for No Glbl):");
  result->range=GetI64	("Range   (%4d):",30);
  result->spacing=GetI64("Spacing (%4d):",20);
  result->border=GetI64 ("Border  (%4d):",2);

  result->next_slider=result->last_slider=&result->next_slider;
  while (TRUE) {
    StrPrint(pmt,"Slider #%d Name:",result->num_sliders+1);
    st=GetStr(pmt);
    if (!*st) {
      Free(st);
      break;
    }
    temps=CAlloc(sizeof(CTemplateCSSlider));
    temps->name=st;
    QueIns(temps,result->last_slider);
    temps->num=result->num_sliders++;
  }
  return result;
}

U0 TemplateCtrlSlider(CDoc *doc)
{
  CTask *task;
  CTemplateCS *m=NULL;
  CTemplateCSSlider *temps;
  I64 i;

  DocUnlock(doc);
  DocRecalc(doc);
  DocCaptureUndo(doc,TRUE);

  task=User("CTemplateCS **_m=0x%X;*_m=CtrlSliderGet;\n",&m);
  while (!m)
    Yield;

  DocPrint(doc,
"#define SLIDER_RANGE\t%d
#define SLIDER_SPACING\t%d
#define SLIDER_BORDER\t%d

class CSliderState
{
",m->range,m->spacing,m->border);

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,"  I64 %s;\n",temps->name);
    temps=temps->next;
  }
  if (*m->glbl_name)
    DocPrint(doc,"} %s;\n",m->glbl_name);
  else
    DocPrint(doc,"};\n");

  DocPrint(doc,
"
U0 DrawCtrlSlider(CDC *dc,CCtrl *c)
{
  CSliderState *s=c->state;

  dc->color=LTRED;
  GrRect(dc, c->left,c->top,%d*SLIDER_SPACING+%d,SLIDER_SPACING*2+SLIDER_RANGE);
  dc->color=BLUE;
  GrRect(dc, c->left+SLIDER_BORDER,c->top+SLIDER_BORDER,
    %d*SLIDER_SPACING+%d-2*SLIDER_BORDER,SLIDER_SPACING*2+SLIDER_RANGE-2*SLIDER_BORDER);
  dc->color=BLACK;
",m->num_sliders+1,m->num_sliders,m->num_sliders+1,m->num_sliders);

  for (i=0;i<m->num_sliders;i++)
    DocPrint(doc,
"  GrLine(dc,c->left+%d*SLIDER_SPACING+%d,c->top+SLIDER_SPACING,
	      c->left+%d*SLIDER_SPACING+%d,c->top+SLIDER_SPACING+SLIDER_RANGE-1);
",i+1,i,i+1,i);

  DocPrint(doc,"  dc->color=LTRED;\n");

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,
"  GrPrint(dc,c->left+%d*SLIDER_SPACING+%d-FONT_WIDTH/2,
    c->top+SLIDER_SPACING+SLIDER_RANGE+3,
    \"%%d\",s->%s*10/SLIDER_RANGE);\n",
      temps->num+1,temps->num,temps->name);
    temps=temps->next;
  }

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,"  GrRect(dc,c->left+%d*SLIDER_SPACING+%d-3,c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->%s-2,7,5);\n",
      temps->num+1,temps->num,temps->name);
    temps=temps->next;
  }
  DocPrint(doc,"  dc->color=YELLOW;\n");

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,"  GrRect(dc,c->left+%d*SLIDER_SPACING+%d-2,c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->%s-1,5,3);\n",
      temps->num+1,temps->num,temps->name);
    temps=temps->next;
  }
  DocPrint(doc,
"}

U0 UpdateDerivedCtrlSlider(CCtrl *c)
{
  CSliderState *s=c->state;
  c->left=c->win_task->pix_width/2-(SLIDER_SPACING*3+2)/2;
  c->right=c->left+%d*SLIDER_SPACING+%d;
  c->top=c->win_task->pix_height/2-(SLIDER_SPACING*2+SLIDER_RANGE)/2;
  c->bottom=c->top+SLIDER_SPACING*2+SLIDER_RANGE;
",m->num_sliders+1,m->num_sliders);

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,"  s->%s=ClampI64(s->%s,0,SLIDER_RANGE-1);\n",temps->name,temps->name);
    temps=temps->next;
  }
  DocPrint(doc,
"}

U0 LeftClickSlider(CCtrl *c,I64 x,I64 y,Bool down)
{
  nounusedwarn down;
  CSliderState *s=c->state;
");

  temps=m->next_slider;
  while (temps!=&m->next_slider) {
    DocPrint(doc,"  ");
    if (temps!=m->next_slider)
      DocPrint(doc,"else ");
    if (temps->next==&m->next_slider)
      DocPrint(doc,"\n");
    else
      DocPrint(doc,"if (x<c->left+%d*SLIDER_SPACING+%d+SLIDER_SPACING/2)\n",
	temps->num+1,temps->num);
    DocPrint(doc,"    s->%s=SLIDER_RANGE-1-(y-(c->top+SLIDER_SPACING));\n",
       temps->name);
    temps=temps->next;
  }
 
  DocPrint(doc,
"  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

CCtrl *SliderNew()
{
  Bool old_preempt;
  CCtrl *c=CAlloc(sizeof(CCtrl));
  c->win_task=Fs;
  c->flags=CTRLF_SHOW|CTRLF_CAPTURE_LEFT_IP;
  c->type=CTRLT_GENERIC;
");
  if (*m->glbl_name)
    DocPrint(doc,"  c->state=&%s;\n"
		"  MemSet(&%s,0,sizeof(CSliderState));\n",m->glbl_name,m->glbl_name);
  else
    DocPrint(doc,	"  c->state=CAlloc(sizeof(CSliderState));\n");
  DocPrint(doc,
"  c->draw_it=&DrawCtrlSlider;
  c->left_click=&LeftClickSlider;
  c->update_derived_vals=&UpdateDerivedCtrlSlider;
  old_preempt=Preempt(OFF);
  QueIns(c,Fs->last_ctrl);
  TaskDerivedValsUpdate;
  Preempt(old_preempt);
  return c;
}

U0 SliderDel(CCtrl *c)
{
  Bool old_preempt=Preempt(OFF);
  QueRem(c);
  Preempt(old_preempt);
");
  if (!*m->glbl_name)
    DocPrint(doc,"  Free(c->state);\n");
  DocPrint(doc,
"  Free(c);
}
");

  Kill(task);  //This frees memory.

  AcctOneTimePopUp(ARf_MANAGE_SLIDER,
    "Modify the code to your heart's content.\n"
    "The code is no longer managed.\n");

  DocLock(doc);
}
