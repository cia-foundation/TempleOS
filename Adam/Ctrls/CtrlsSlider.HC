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
  CTemplateCSSlider *tmps;
  U8 *st,pmt[STR_LEN];
  CTemplateCS *res=CAlloc(sizeof(CTemplateCS));

  "$$PURPLE$$Ctrl Slider$$FG$$\n\n";

  res->glbl_name=GetStr("Glbl Struct Name (ENTER for No Glbl):");
  res->range=GetI64	("Range   (%4d):",30);
  res->spacing=GetI64("Spacing (%4d):",20);
  res->border=GetI64 ("Border  (%4d):",2);

  res->next_slider=res->last_slider=&res->next_slider;
  while (TRUE) {
    StrPrint(pmt,"Slider #%d Name:",res->num_sliders+1);
    st=GetStr(pmt);
    if (!*st) {
      Free(st);
      break;
    }
    tmps=CAlloc(sizeof(CTemplateCSSlider));
    tmps->name=st;
    QueIns(tmps,res->last_slider);
    tmps->num=res->num_sliders++;
  }
  return res;
}

U0 TemplateCtrlSlider(CDoc *doc)
{
  CTask *task;
  CTemplateCS *m=NULL;
  CTemplateCSSlider *tmps;
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

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,"  I64 %s;\n",tmps->name);
    tmps=tmps->next;
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
    %d*SLIDER_SPACING+%d-2*SLIDER_BORDER,SLIDER_SPACING*2
    +SLIDER_RANGE-2*SLIDER_BORDER);
  dc->color=BLACK;
",m->num_sliders+1,m->num_sliders,m->num_sliders+1,m->num_sliders);

  for (i=0;i<m->num_sliders;i++)
    DocPrint(doc,
"  GrLine(dc,c->left+%d*SLIDER_SPACING+%d,c->top+SLIDER_SPACING,
             c->left+%d*SLIDER_SPACING+%d,c->top+SLIDER_SPACING+SLIDER_RANGE-1);
",i+1,i,i+1,i);

  DocPrint(doc,"  dc->color=LTRED;\n");

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,
"  GrPrint(dc,c->left+%d*SLIDER_SPACING+%d-FONT_WIDTH/2,
    c->top+SLIDER_SPACING+SLIDER_RANGE+3,
    \"%%d\",s->%s*10/SLIDER_RANGE);\n",
      tmps->num+1,tmps->num,tmps->name);
    tmps=tmps->next;
  }

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,
	"  GrRect(dc,c->left+%d*SLIDER_SPACING+%d-3,"
	"c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->%s-2,7,5);\n",
      tmps->num+1,tmps->num,tmps->name);
    tmps=tmps->next;
  }
  DocPrint(doc,"  dc->color=YELLOW;\n");

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,"  GrRect(dc,c->left+%d*SLIDER_SPACING+%d-2,"
	"c->top+SLIDER_SPACING+SLIDER_RANGE-1-s->%s-1,5,3);\n",
      tmps->num+1,tmps->num,tmps->name);
    tmps=tmps->next;
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

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,"  s->%s=ClampI64(s->%s,0,SLIDER_RANGE-1);\n",
	tmps->name,tmps->name);
    tmps=tmps->next;
  }
  DocPrint(doc,
"}

U0 LeftClickSlider(CCtrl *c,I64 x,I64 y,Bool down)
{
  no_warn down;
  CSliderState *s=c->state;
");

  tmps=m->next_slider;
  while (tmps!=&m->next_slider) {
    DocPrint(doc,"  ");
    if (tmps!=m->next_slider)
      DocPrint(doc,"else ");
    if (tmps->next==&m->next_slider)
      DocPrint(doc,"\n");
    else
      DocPrint(doc,"if (x<c->left+%d*SLIDER_SPACING+%d+SLIDER_SPACING/2)\n",
	tmps->num+1,tmps->num);
    DocPrint(doc,"    s->%s=SLIDER_RANGE-1-(y-(c->top+SLIDER_SPACING));\n",
       tmps->name);
    tmps=tmps->next;
  }
 
  DocPrint(doc,
"  if (c->update_derived_vals)
    (*c->update_derived_vals)(c);
}

CCtrl *SliderNew()
{
  CCtrl *c=CAlloc(sizeof(CCtrl));
  c->win_task=Fs;
  c->flags=CTRLF_SHOW|CTRLF_CAPTURE_LEFT_MS;
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
  QueIns(c,Fs->last_ctrl);
  TaskDerivedValsUpdate;
  return c;
}

U0 SliderDel(CCtrl *c)
{
  QueRem(c);
");
  if (!*m->glbl_name)
    DocPrint(doc,"  Free(c->state);\n");
  DocPrint(doc,
"  Free(c);
}
");

  Kill(task);  //This frees memory.

  RegOneTimePopUp(ARf_MANAGE_SLIDER,
    "Modify the code to your heart's content.\n"
    "The code is no longer managed.\n");

  DocLock(doc);
}
