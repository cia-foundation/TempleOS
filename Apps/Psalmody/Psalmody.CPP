/*$WW,1$
This uses four types of ctrls

  1) The pull-down menu.

  2) The $LK,"CCtrl",A="MN:CCtrl"$ type for the tempo/stacatto sliders.

  3) The active $LK,"CDoc",A="MN:CDoc"$ sprite buttons with macros for left/record/play/right.

  4) Hand-made ctrls for the kbdgraphic, notes and staff.

This uses four types of output

  1) The $LK,"CDoc",A="MN:CDoc"$ for the text and buttons and kbdgraphic.

  2) The Fs->draw_it() for the staff region, drawn every refresh.

  3) The gr.dc persistent layer for the note chooser, meter chooser.  The persistent layer is used during drag-and-drop.

  4) The Fs->next_ctrl for the tempo/stacatto sliders.

See $LK,"GrUpdateWins",A="MN:GrUpdateWins"$() and $LK,"GrUpdateScreen",A="MN:GrUpdateScreen"$().
$WW,1$$WW,0$*/

#define NOTE_REST	MIN_I64
#define OCTAVE_REST	MIN_I64

#define OT_ROOT		0
#define OT_NOTE		1
#define OT_METER	2

#define Of_SELECTED	0
#define OF_SELECTED	1
#define Of_SHARP	1
#define Of_FLAT		2
#define Of_TIE		3

class PsmNote
{
  PsmNote *next,*last;
  I64 x,y;
  U8 *word;
  I64 type,flags;
  union {
     I64 note;
     I64 meter_top;
  };
  union {
     I64 octave;
     I64 meter_bottom;
  }
  I64 duration,width;
  U8 ascii[32];
};

//Tool types
#define OTT_PTR_TOOL	0
#define OTT_BOX_TOOL	1

class PsmCtrl
{
  PsmNote root;
  PsmNote clipboard;
  CMenuEntry *incomplete_entry,*record_entry;
  I64 screen_x,
     tool;
  PsmNote *cur_note;
  CDC *dc2;
  Bool playing;
} oc;

U8 *psm_note_lst="A\0A#\0B\0C\0C#\0D\0D#\0E\0F\0F#\0G\0G#\0";
U8 psm_note_map[12]={6,6,5,4,4,3,3,2,1,1,0,0};
U8 psm_note_inverse_map[7]={10,8,7,5,3,2,0};

$ID,8$
$SP,"<1>",BI=1$

$SP,"<2>",BI=2$

$SP,"<3>",BI=3$

$SP,"<4>",BI=4$

$SP,"<5>",BI=5$

$SP,"<6>",BI=6$


$SP,"<7>",BI=7$

	
$SP,"<8>",BI=8$


$SP,"<9>",BI=9$


$SP,"<10>",BI=10$


$SP,"<11>",BI=11$







$SP,"<12>",BI=12$


	

$SP,"<13>",BI=13$

$SP,"<14>",BI=14$

$SP,"<15>",BI=15$


$SP,"<16>",BI=16$
$ID,-8$
#define PSM_NOTE_SPACING	9
#define PSM_NUM_DURATIONS	12

F64 psm_durations[PSM_NUM_DURATIONS+1]={
2*.25/3,.25,2*.5/3,.5,2.0/3.0,0.5*1.5,1.0,1.5,2.0,3.0,4.0,6.0,1000000.0};
U8 *psm_duration_lst="st\0s\0et\0e\0qt\0e.\0q\0q.\0h\0h.\0w\0w.\0";
CSprite *psm_duration_imgs[PSM_NUM_DURATIONS]={$IB,"<1>",BI=1$,$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,
			$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<3>",BI=3$,
			$IB,"<4>",BI=4$,$IB,"<4>",BI=4$,$IB,"<5>",BI=5$,$IB,"<5>",BI=5$};
Bool psm_triplet_durations[PSM_NUM_DURATIONS]={TRUE,FALSE,TRUE,
			FALSE,TRUE,FALSE,FALSE,FALSE,
			FALSE,FALSE,FALSE,FALSE};

Bool PsmHasWords(U8 *st)
{
  if (st && *st && (StrLen(st)>1 || *st!=CH_SPACE && *st!=CH_SHIFT_SPACE))
    return TRUE;
  else
    return FALSE;
}

Bool PsmIsDotted(I64 duration)
{
  U8 *st=LstSub(duration,psm_duration_lst);
  if (st[1]=='.')
    return TRUE;
  else
    return FALSE;
}

U0 PsmSetWidth(PsmNote *tempo)
{
  if (tempo->type==OT_METER)
    tempo->width=12;
  else {
    tempo->width=PSM_NOTE_SPACING;
    if (PsmIsDotted(tempo->duration))
      tempo->width+=PSM_NOTE_SPACING/2;
    if (Bt(&tempo->flags,Of_SHARP) || Bt(&tempo->flags,Of_FLAT))
      tempo->width+=PSM_NOTE_SPACING;
  }
}

U0 PsmRecalcNoteXY()
{
  F64 measure_len=4,measure_left=measure_len;
  PsmNote *tempo=oc.root.next;
  I64 x=8-oc.screen_x,note,octave;
  while (TRUE) {
    tempo->x=x;
    tempo->y=50;
    if (tempo==&oc.root)
      break;
    else {
      if (tempo->type!=OT_METER) {
	octave=tempo->octave;
	if (octave!=OCTAVE_REST) {
	  note=tempo->note;
	  if (Bt(&tempo->flags,Of_SHARP))
	    note--;
	  if (Bt(&tempo->flags,Of_FLAT))
	    note++;
	  if (note<0) {
	    note=11;
	    octave--;
	  } else if (note>11) {
	    note=0;
	    octave++;
	  }
	  note=music.note_map[*LstSub(note,psm_note_lst)-'A'];
	  tempo->y=(15+(psm_note_map[note]-7*(octave-2)))*4;
	}
      }
      x+=tempo->width;
      if (tempo->type==OT_METER) {
	measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
	measure_left=0;
      } else
	measure_left-=psm_durations[tempo->duration];
      if (measure_left<0.001) {
	x+=PSM_NOTE_SPACING;
	measure_left=measure_len;
      }
    }
    tempo=tempo->next;
  }
}

U0 PsmMarkSelected(I64 x1,I64 x2,Bool sel)
{
  PsmNote *tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (sel) {
      if (x1<=tempo->x<=x2)
	tempo->flags|=OF_SELECTED;
    } else
      tempo->flags&=~OF_SELECTED;
    tempo=tempo->next;
  }
}

PsmNote *PsmNoteCopy(PsmNote *tempo)
{
  PsmNote *tempo1=MAllocIdentical(tempo);
  if (tempo->word)
    tempo1->word=StrNew(tempo->word);
  else
    tempo1->word=NULL;
  return tempo1;
}

U0 PsmNoteDel(PsmNote *tempo)
{
  Free(tempo->word);
  Free(tempo);
}

U0 PsmSongDel(PsmNote *root)
{
  PsmNote *tempo,*tempo1;
  tempo=root->next;
  while (tempo!=root) {
    tempo1=tempo->next;
    PsmNoteDel(tempo);
    tempo=tempo1;
  }
  root->next=root->last=root;
}

U0 PsmCutToClipboard()
{
  PsmNote *tempo,*tempo1;
  PsmSongDel(&oc.clipboard);
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    tempo1=tempo->next;
    if (tempo->flags&OF_SELECTED) {
      if (oc.cur_note==tempo)
	oc.cur_note=tempo->next;
      QueRem(tempo);
      tempo->flags&=~OF_SELECTED;
      QueIns(tempo,oc.clipboard.last);
    }
    tempo=tempo1;
  }
}

U0 PsmPasteClipboard()
{
  PsmNote *tempo,*tempo1;
  tempo=oc.clipboard.next;
  while (tempo!=&oc.clipboard) {
    tempo1=PsmNoteCopy(tempo);
    QueIns(tempo1,oc.cur_note->last);
    tempo=tempo->next;
  }
}

U0 PsmCopyToClipboard()
{
  PsmNote *tempo,*tempo1;
  PsmSongDel(&oc.clipboard);
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (tempo->flags&OF_SELECTED) {
      tempo->flags&=~OF_SELECTED;
      tempo1=PsmNoteCopy(tempo);
      QueIns(tempo1,oc.clipboard.last);
    }
    tempo=tempo->next;
  }
}

PsmNote *PsmFindNote(I64 x,I64)
{
  PsmNote *tempo=oc.root.next;
  PsmRecalcNoteXY;
  x+=PSM_NOTE_SPACING/2;
  while (x>tempo->next->x && tempo!=&oc.root)
    tempo=tempo->next;
  return tempo;
}

Bool PsmBlink()
{
  if (Blink || oc.playing)
    return TRUE;
  else
    return FALSE;
}

U0 DrawNote(CDC *dc,I64 x,I64 y,I64 duration)
{
  if (0<=duration<=PSM_NUM_DURATIONS) {
    Sprite3(dc,x,y,0,psm_duration_imgs[duration]);
    if (psm_triplet_durations[duration])
      Sprite3(dc,x,y,0,$IB,"<16>",BI=16$);
    if (PsmIsDotted(duration))
      Sprite3(dc,x,y,0,$IB,"<15>",BI=15$);
  }
}

U0 DrawTimeSignature(CDC *dc,I64 x,I64 y,I64 top,I64 bottom)
{
  GrPrint(dc,x,y,"%d",top);
  GrPrint(dc,x,y+FONT_HEIGHT,"%d",bottom);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  PsmNote *tempo;
  I64 i,x,y,
     w=task->pix_width;
  F64 measure_len=4,measure_left=measure_len;
  Bool old_preempt=Preempt(OFF);

  dc->color=BLACK;
  for (i=1;i<6;i++)
    GrLine(dc,0,i*8,w,i*8);
  for (i=7;i<12;i++)
    GrLine(dc,0,i*8,w,i*8);

  PsmRecalcNoteXY;
  if (oc.cur_note->x<64) {
    oc.screen_x-=128;
    PsmRecalcNoteXY;
  }
  if (oc.cur_note->x>=GR_WIDTH-64) {
    oc.screen_x+=128;
    PsmRecalcNoteXY;
  }
 
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    x=tempo->x;
    y=tempo->y;
    if (measure_left<0.001) {
      dc->color=BLACK;
      GrLine(dc,x-PSM_NOTE_SPACING,8,x-PSM_NOTE_SPACING,11*8);
      measure_left=measure_len;
    }
    if (tempo->type==OT_METER) {
      if (tempo==oc.cur_note && PsmBlink)
	dc->color=BROWN;
      else
	dc->color=BLACK;
      DrawTimeSignature(dc,x,5*8,tempo->meter_top,tempo->meter_bottom);
      measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
      measure_left=0;
    } else {
      if (tempo==oc.cur_note && PsmBlink) {
	dc->color=BROWN;
	GrPrint(dc,x+8,y,"%s",tempo->word);
      } else if (tempo->flags&OF_SELECTED)
	dc->color=RED;
      else {
	if (tempo->note!=NOTE_REST)
	  dc->color=BLACK;
	else
	  dc->color=LTGRAY;
	if (PsmHasWords(tempo->word))
	  dc->color=GREEN;
      }
      DrawNote(dc,x,y,tempo->duration);
      if (PsmIsDotted(tempo->duration))
	x+=PSM_NOTE_SPACING/2;
      if (Bt(&tempo->flags,Of_SHARP))
	Sprite3(dc,x,y,0,$IB,"<13>",BI=13$);
      else if (Bt(&tempo->flags,Of_FLAT))
	Sprite3(dc,x,y,0,$IB,"<14>",BI=14$);
      measure_left-=psm_durations[tempo->duration];
    }
    tempo=tempo->next;
  }

  if (oc.cur_note==&oc.root && PsmBlink)
    dc->color=BROWN;
  else
    dc->color=BLACK;
  Sprite3(dc,oc.root.x,50,0,$IB,"<12>",BI=12$);
  Preempt(old_preempt);
}

#define PSMR_FLAT		-8
#define PSMR_SHARP		-7
#define PSMR_TIE		-6
#define PSMR_REST		-5
#define PSMR_INS_NOTE		-4
#define PSMR_DELETE_NOTE	-3
#define PSMR_SET_WORD		-2

F64 PopUpDuration()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$GREEN$$$$MU,\"Set Word\",LE=PSMR_SET_WORD$$\n"
	"$$MU,\"Toggle Sharp\",LE=PSMR_SHARP$$\n"
	"$$MU,\"Toggle Flat\",LE=PSMR_FLAT$$\n"
	"$$MU,\"Toggle Tie\",LE=PSMR_TIE$$\n"
	"$$MU,\"Make Rest\",LE=PSMR_REST$$\n"
	"$$MU,\"Insert Note\",LE=PSMR_INS_NOTE$$\n"
	"$$MU,\"Delete Note\",LE=PSMR_DELETE_NOTE$$\n\n");
  for (i=0;i<PSM_NUM_DURATIONS;i++)
    DocPrint(doc,"$$MU,\"%7.5f\",LE=%d$$\n",psm_durations[i],i);
  DocPrint(doc,"\n$$MU,\"CANCEL\",LE=DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 PsmRightClick(I64 x,I64 y)
{
  U8 *st,*st2;
  PsmNote *tempo,*tempo1;
  I64 i,old_doc_flags;
  if (DocPut) old_doc_flags=DocPut->flags;
  oc.cur_note=tempo=PsmFindNote(x,y);
  if (tempo!=&oc.root) {
    Fs->win_inhibit=WIG_USER_TASK_DFT;
    i=PopUpDuration;
    if (0<=i<PSM_NUM_DURATIONS) {
      if (tempo->type==OT_NOTE)
	tempo->duration=i;
    } else {
      switch (i) {
	case PSMR_REST:
	  if (tempo->type==OT_NOTE) {
	    tempo->octave=OCTAVE_REST;
	    tempo->note=NOTE_REST;
	  }
	  break;
	case PSMR_SHARP:
	  if (tempo->type==OT_NOTE && tempo->octave!=OCTAVE_REST) {
	    if (Btr(&tempo->flags,Of_FLAT))
	      tempo->note++;
	    if (Btc(&tempo->flags,Of_SHARP))
	      tempo->note--;
	    else
	      tempo->note++;
	    if (tempo->note<0) {
	      tempo->note=11;
	      tempo->octave--;
	    } else if (tempo->note>11) {
	      tempo->note=0;
	      tempo->octave++;
	    }
	  }
	  break;
	case PSMR_FLAT:
	  if (tempo->type==OT_NOTE && tempo->octave!=OCTAVE_REST) {
	    if (Btr(&tempo->flags,Of_SHARP))
	      tempo->note--;
	    if (Btc(&tempo->flags,Of_FLAT))
	      tempo->note++;
	    else
	      tempo->note--;
	    if (tempo->note<0) {
	      tempo->note=11;
	      tempo->octave--;
	    } else if (tempo->note>11) {
	      tempo->note=0;
	      tempo->octave++;
	    }
	  }
	  break;
	case PSMR_TIE:
	  if (tempo->type==OT_NOTE)
	    Btc(&tempo->flags,Of_TIE);
	  break;
	case PSMR_SET_WORD:
	  if (tempo->type==OT_NOTE) {
	    if (DocPut) DocPut->flags&=~DOCF_FORM;
	    if (PsmHasWords(tempo->word))
	      st2=MStrPrint("\nWord(\"%Q\"):",tempo->word);
	    else
	      st2=MStrPrint("\nWord(\"\"):");
	    DocBottom;
	    st=GetStr(st2);
	    Free(st2);
	    Free(tempo->word);
	    if (*st) {
	      tempo->word=MStrPrint("%q",st);
	      Free(st);
	    } else
	      tempo->word=StrNew(" ");
	    if (DocPut) DocPut->flags=DocPut->flags&~DOCF_FORM|old_doc_flags&DOCF_FORM;
	  }
	  break;
	case PSMR_INS_NOTE:
	  tempo1=PsmNoteCopy(tempo);
	  QueIns(tempo1,tempo);
	  break;
	case PSMR_DELETE_NOTE:
	  oc.cur_note=tempo->next;
	  QueRem(tempo);
	  PsmNoteDel(tempo);
	  break;
      }
    }
    PsmSetWidth(oc.cur_note);
    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
  }
}

#define PSM_NOTE_BOX_X	220
#define PSM_NOTE_BOX_Y	(13*FONT_HEIGHT+14)

U0 PsmSetPickNoteBoxX(I64 duration,I64 *x)
{
  I64 i;
  *x=PSM_NOTE_BOX_X;
  for (i=0;i<duration;i++) {
    if (PsmIsDotted(i))
      *x+=PSM_NOTE_SPACING/2;
    *x+=PSM_NOTE_SPACING+4;
  }
}

I64 PsmGetPickNoteBoxDuration(I64 xx,I64 yy)
{
  I64 i,x1,x2;
  if (PSM_NOTE_BOX_Y-14<=yy<PSM_NOTE_BOX_Y+6) {
    for (i=0;i<PSM_NUM_DURATIONS;i++) {
      PsmSetPickNoteBoxX(i,&x1);
      PsmSetPickNoteBoxX(i+1,&x2);
      if (x1<=xx+PSM_NOTE_SPACING/2<x2)
	return i;
    }
  }
  return -1;
}

U0 DrawPickNoteBox()
{
  I64 i,x;
  for (i=0;i<PSM_NUM_DURATIONS;i++) {
    PsmSetPickNoteBoxX(i,&x);
    if (PsmIsDotted(i))
      oc.dc2->color=RED;
    else if (psm_triplet_durations[i])
      oc.dc2->color=LTRED;
    else
      oc.dc2->color=BLACK;
    DrawNote(oc.dc2,x,PSM_NOTE_BOX_Y,i);
  }
}

#define PSM_TOOLS_X	450
#define PSM_TOOLS_Y	13*FONT_HEIGHT

U0 DrawPickTools()
{
  if (oc.tool==OTT_BOX_TOOL)
    oc.dc2->color=ROPF_DITHER+WHITE<<16+RED;
  else
    oc.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
  GrBorder(oc.dc2,PSM_TOOLS_X,PSM_TOOLS_Y,
    PSM_TOOLS_X+10,PSM_TOOLS_Y+10);

  if (oc.tool==OTT_PTR_TOOL)
    oc.dc2->color=RED;
  else
    oc.dc2->color=BLACK;
  (*gr.fp_draw_input_ptr)(oc.dc2,PSM_TOOLS_X+15,PSM_TOOLS_Y);
  oc.dc2->color=BLACK;
}

Bool PsmGetPickToolBox(I64 xx,I64 yy)
{
  if (PSM_TOOLS_X<=xx<PSM_TOOLS_X+27 &&
      PSM_TOOLS_Y<=yy<PSM_TOOLS_Y+15) {
    PsmMarkSelected(0,0,FALSE);
    if (xx<PSM_TOOLS_X+13)
      oc.tool=OTT_BOX_TOOL;
    else
      oc.tool=OTT_PTR_TOOL;
    return TRUE;
  } else
    return FALSE;
}

#define PSM_NUM_METERS	7
I64 meter_tops[PSM_NUM_METERS]	={2,3,4,5,6,7,9},
   meter_bottoms[PSM_NUM_METERS]={4,4,4,4,8,8,8};

#define PSM_METER_X	485
#define PSM_METER_Y	13*FONT_HEIGHT
#define PSM_METER_W	12
Bool PsmGetPickMeterBox(I64 xx,I64 yy,I64 *top,I64 *bottom)
{
  I64 i;
  if (PSM_METER_X<=xx<PSM_METER_X+PSM_METER_W*PSM_NUM_METERS &&
      PSM_METER_Y<=yy<PSM_METER_Y+2*FONT_HEIGHT) {
    i=(xx-PSM_METER_X)/PSM_METER_W;
    *top=meter_tops[i];
    *bottom=meter_bottoms[i];
    return TRUE;
  } else
    return FALSE;
}

U0 DrawPickMeterBox()
{
  I64 i;
  oc.dc2->color=BLACK;
  for (i=0;i<PSM_NUM_METERS;i++)
    DrawTimeSignature(oc.dc2,PSM_METER_X+i*PSM_METER_W,PSM_METER_Y,meter_tops[i],meter_bottoms[i]);
}

U0 DrawDC2()
{
  DCFill;
  DrawPickNoteBox;
  DrawPickMeterBox;
  DrawPickTools;
}

U0 PsmLeftClickPickNoteBox(I64 duration)
{
  I64 o,n,msg_code,a1,a2;
  PsmNote *tempo,*tempo1;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawNote(oc.dc2,a1,a2,duration);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (a2<13*FONT_HEIGHT) {
    if (a1>oc.root.last->x)
      tempo1=oc.root.last;
    else if (a1<oc.root.next->x)
      tempo1=&oc.root;
    else
      tempo1=PsmFindNote(a1-PSM_NOTE_SPACING/2,a2);
    tempo=CAlloc(sizeof(PsmNote));
    tempo->type=OT_NOTE;
    a2=a2/4-15;
    n=-a2%7;
    o=2+a2/-7;
    n=-n;
    if (n<0) {
      n+=7;
      o++;
    }
    n=psm_note_inverse_map[n];
    tempo->note=n;
    tempo->octave=o;
    tempo->duration=duration;
    PsmSetWidth(tempo);
    QueIns(tempo,tempo1);
    oc.cur_note=tempo->next;
  }
  DrawDC2;
}

U0 PsmLeftClickPickMeterBox(I64 top,I64 bottom)
{
  I64 msg_code,a1,a2;
  PsmNote *tempo,*tempo1;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawTimeSignature(oc.dc2,a1,a2,top,bottom);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (a2<13*FONT_HEIGHT) {
    if (a1>=oc.root.x)
      tempo1=oc.root.last;
    else if (a1<oc.root.next->x)
      tempo1=&oc.root;
    else
      tempo1=PsmFindNote(a1-PSM_NOTE_SPACING/2,a2);
    tempo=CAlloc(sizeof(PsmNote));
    tempo->type=OT_METER;
    tempo->meter_top=top;
    tempo->meter_bottom=bottom;
    PsmSetWidth(tempo);
    QueIns(tempo,tempo1);
    oc.cur_note=tempo->next;
  }
  DrawDC2;
}

U0 PsmLeftClickStaffPtr(I64 x,I64 y)
{
  PsmNote *tempo,*tempo1;
  I64 o,n,msg_code,a1,a2,n_original,o_original;
  oc.cur_note=tempo=PsmFindNote(x,y);
  if (tempo!=&oc.root) {
    if (tempo->type==OT_NOTE) {
      n_original=tempo->note;
      o_original=tempo->octave;
      do {
	msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
	if (msg_code==MSG_IP_L_UP) {
	  tempo1=PsmFindNote(a1,a2);
	  if (tempo1==&oc.root || tempo1==tempo)
	    goto move_note;
	  else {
	    Free(tempo1->word);
	    tempo1->word=tempo->word;
	    tempo->word=NULL;
	    tempo->note=n_original;
	    tempo->octave=o_original;
	  }
	} else {
move_note:
	  a2=a2/4-15;
	  n=-a2%7;
	  o=2+a2/-7;
	  n=-n;
	  if (n<0) {
	    n+=7;
	    o++;
	  }
	  n=psm_note_inverse_map[n];
	  tempo->note=n;
	  tempo->octave=o;
	}
      } while (msg_code!=MSG_IP_L_UP);
      PsmSetWidth(tempo);
    }
  }
}

U0 PsmLeftClickStaffBox(I64 x,I64 y)
{
  I64 msg_code,a1,a2;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    DrawDC2;
    oc.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrBorder(oc.dc2,x,y,a1,a2);
    if (msg_code==MSG_IP_L_UP) {
      if (x>a1) SwapI64(&x,&a1);
      PsmMarkSelected(x,a1,TRUE);
    }
  } while (msg_code!=MSG_IP_L_UP);
  DrawDC2;
}

U0 PsmLeftClick(I64 x,I64 y)
{
  I64 duration,top,bottom;
  if (y<13*FONT_HEIGHT) {
    if (oc.tool==OTT_PTR_TOOL)
      PsmLeftClickStaffPtr(x,y);
    else
      PsmLeftClickStaffBox(x,y);
  } else {
    duration=PsmGetPickNoteBoxDuration(x,y);
    if (0<=duration<PSM_NUM_DURATIONS)
      PsmLeftClickPickNoteBox(duration);
    else if (PsmGetPickMeterBox(x,y,&top,&bottom))
      PsmLeftClickPickMeterBox(top,bottom);
    else if (PsmGetPickToolBox(x,y))
      DrawDC2;
  }
}

#define PSMPM_NORMAL		0
#define PSMPM_REVERB1		1
#define PSMPM_NUM_PLAY_MODES	2

I64 psm_play_mode=0;
F64 psm_play_f=0;

U0 PsmPlayTask(I64)
{
  I64 i=0;
  F64 f=0,f2=0;
  Fs->task_end_cb=&SndTaskEndCB;
  while (TRUE) {
    i++;
    switch (psm_play_mode) {
      case PSMPM_NORMAL:
	if (psm_play_f!=f) {
	  f=psm_play_f;
	  Snd(f);
	}
	break;
      case PSMPM_REVERB1:
	if (psm_play_f!=f) {
	  if (f>0)
	    f2=f;
	  f=psm_play_f;
	}
	if (!(i&15)) {
	  if (i&16)
	    Snd(f);
	  else
	    Snd(f2);
	}
	break;
    }
    Sleep(1);
  }
}

U8 PsmCvtDuration(F64 d)
{
  F64 d1,d2;
  I64 j;
  for (j=0;j<PSM_NUM_DURATIONS;j++) {
    d1=psm_durations[j];
    d2=psm_durations[j+1];
    if (d<d1*d2/(d1+d2))
      return j;
  }
  return 0;
}

U8 *PsmMusicSetOctave(U8 *st,I64 *psm_octave)
{
  while ('0'<=*st<='9')
    *psm_octave=*st++ -'0';
  return st;
}

U8 *PsmMusicSetNoteLen(U8 *st,F64 *psm_duration)
{
  Bool cont=TRUE;
  do {
    switch (*st++) {
      case 'w': *psm_duration=4.0;  break;
      case 'h': *psm_duration=2.0;  break;
      case 'q': *psm_duration=1.0;  break;
      case 'e': *psm_duration=0.5;  break;
      case 's': *psm_duration=0.25; break;
      case 't': *psm_duration=2.0* *psm_duration/3.0;	break;
      case '.': *psm_duration=1.5* *psm_duration;	break;
      default:
	st--;
	cont=FALSE;
    }
  } while (cont);
  return st;
}

U0 PsmLoadSongStr(U8 *st,I64 *psm_octave,F64 *psm_duration)
{
  PsmNote *tempo,*tempo1;
  I64 note,i=0;
  while (*st) {
    tempo=CAlloc(sizeof(PsmNote));
    while (*st && !('A'<=*st<='G') && *st!='R') {
      if (*st=='M') {
	tempo1=CAlloc(sizeof(PsmNote));
	tempo1->type=OT_METER;
	st++;
	if ('1'<=*st<='9')
	  tempo1->meter_top=*st++-'0';
	else
	  tempo1->meter_top=4;
	if (*st=='/')
	  st++;
	if ('1'<=*st<='9')
	  tempo1->meter_bottom=*st++-'0';
	else
	  tempo1->meter_bottom=4;
	PsmSetWidth(tempo1);
	QueIns(tempo1,oc.root.last);
      }
      while (*st=='(') {
	Bts(&tempo->flags,Of_TIE);
	st++;
      }
      st=PsmMusicSetOctave(st,psm_octave);
      st=PsmMusicSetNoteLen(st,psm_duration);
    }
    if (!*st) {
      PsmNoteDel(tempo);
      break;
    }
    note=*st++-'A';
    if (note<7) {
      note=music.note_map[note];
      if (*st=='b') {
	Bts(&tempo->flags,Of_FLAT);
	note--;
	st++;
      } else if (*st=='#') {
	Bts(&tempo->flags,Of_SHARP);
	note++;
	st++;
      }
      if (note<0) {
	note=11;
	*psm_octave-=1;
      } else if (note>11) {
	note=0;
	*psm_octave+=1;
      }
      tempo->note=note;
      tempo->octave=*psm_octave;
    } else {
      tempo->note=NOTE_REST;
      tempo->octave=OCTAVE_REST;
    }
    if (*psm_duration<=2*.25/3)
      i=0;
    else if (*psm_duration<=.25)
      i=1;
    else if (*psm_duration<=2*.5/3)
      i=2;
    else if (*psm_duration<=.5)
      i=3;
    else if (*psm_duration<=2.0/3)
      i=4;
    else if (*psm_duration<=.5*1.5)
      i=5;
    else if (*psm_duration<=1.0)
      i=6;
    else if (*psm_duration<=1.5)
      i=7;
    else if (*psm_duration<=2.0)
      i=8;
    else if (*psm_duration<=3.0)
      i=9;
    else if (*psm_duration<=4.0)
      i=10;
    else
      i=11;
    tempo->duration=i;
    tempo->type=OT_NOTE;
    PsmSetWidth(tempo);
    QueIns(tempo,oc.cur_note->last);
  }
}

U0 PsmLoadSong(U8 *filename,I64 *psm_octave,F64 *psm_duration)
{
  U8 *st;
  PsmNote *tempo;
  CLex *lx=LexNew(MStrPrint("#include \"%s\"",filename));
  if (FileOcc("incomplete",filename,""))
    oc.incomplete_entry->checked=TRUE;
  else
    oc.incomplete_entry->checked=FALSE;
  while (Lex(lx)) {
    if (lx->token==TK_IDENT)
      if (!StrCmp(lx->cur_str,"Play")) {
	if (Lex(lx)=='(')
	  if (Lex(lx)==TK_STR) {
	    tempo=oc.root.last;
	    st=LexExtStr(lx);
	    PsmLoadSongStr(st,psm_octave,psm_duration);
	    if (lx->token==',') {
	      if (Lex(lx)==TK_STR) {
		st=LexExtStr(lx);
		do {
		  do tempo=tempo->next;
		  while (tempo!=&oc.root && tempo->type==OT_METER);
		  if (tempo!=&oc.root)
		    tempo->word=StrNew(st);
		  st+=StrLen(st)+1;
		} while (*st);
	      }
	    }
	  }
      } else if (!StrCmp(lx->cur_str,"music.tempo")) {
	if (Lex(lx)=='=' && Lex(lx)==TK_F64) {
	  music.tempo=lx->cur_f64-0.0005;
	  tempo_state.tempo=Round(TEMPO_RANGE*(music.tempo-0.5)/4.4);
	}
      } else if (!StrCmp(lx->cur_str,"music.stacatto_factor")) {
	if (Lex(lx)=='=' && Lex(lx)==TK_F64) {
	  music.stacatto_factor=lx->cur_f64-0.0005;
	  tempo_state.stacatto=Round(TEMPO_RANGE*(music.stacatto_factor-0.12)/0.88);
	}
      }
  }
  LexDel(lx);
}

U8 *PsmCvtSong()
{
  PsmNote *tempo;
  U8 *st,*src,*dst;
  I64 i,note,octave,last_octave,last_duration;

  i=0;
  tempo=oc.root.next;
  last_octave=OCTAVE_REST;
  last_duration=-1;
  while (tempo!=&oc.root) {
    dst=&tempo->ascii;
    if (tempo->type==OT_METER) {
      *dst++='M';
      *dst++=tempo->meter_top+'0';
      *dst++='/';
      *dst++=tempo->meter_bottom+'0';
    } else {
      octave=tempo->octave;
      note=tempo->note;
      if (note!=NOTE_REST) {
	if (Bt(&tempo->flags,Of_SHARP))
	  note--;
	if (Bt(&tempo->flags,Of_FLAT))
	  note++;
	if (note<0) {
	  note=11;
	  octave--;
	} else if (note>11) {
	  note=0;
	  octave++;
	}
	note=music.note_map[*LstSub(note,psm_note_lst)-'A'];
      }
      if (Bt(&tempo->flags,Of_TIE))
	*dst++='(';
      if (octave!=last_octave && note!=NOTE_REST) {
	*dst++=octave+'0';
	last_octave=octave;
      }
      if (tempo->duration!=last_duration) {
	src=LstSub(tempo->duration,psm_duration_lst);
	*dst++=src[0];
	if (src[1])
	  *dst++=src[1];
	last_duration=tempo->duration;
      }
      if (note!=NOTE_REST) {
	src=LstSub(note,psm_note_lst);
	*dst++=src[0];
	if (src[1])
	  *dst++=src[1];
	else if (Bt(&tempo->flags,Of_FLAT))
	  *dst++='b';
	else if (Bt(&tempo->flags,Of_SHARP))
	  *dst++='#';
      } else
	*dst++='R';
    }
    *dst++=0;
    i+=StrLen(tempo->ascii);
    tempo=tempo->next;
  }

  st=MAlloc(i+1);
  dst=st;
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    StrCpy(dst,tempo->ascii);
    dst+=StrLen(tempo->ascii);
    tempo=tempo->next;
  }
  *dst++=0;
  return st;
}

U8 *PsmSaveSong(U8 *dirname,U8 *full_filename)
{
  CDoc *doc=DocNew(full_filename);
  Bool has_words;
  PsmNote *tempo,*tempo1;
  F64 measure_len=4,two_measure_left=2*measure_len;
  I64 ch;
  U8 *ptr;
 
  Free(PsmCvtSong); //set tempo->ascii;

  music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
  music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

  has_words=FALSE;
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (PsmHasWords(tempo->word)) has_words=TRUE;
    tempo=tempo->next;
  }
  if (oc.incomplete_entry->checked)
    DocPrint(doc,"//0 incomplete\n");
  else if (has_words)
    DocPrint(doc,"//0 has words\n");
  else
    DocPrint(doc,"//0 no nothing\n");

  DocPrint(doc,
"U0 Song()\n"
"{\n"
"  Fs->task_end_cb=&SndTaskEndCB;\n"
"  MusicSettingsRst;\n"
"  music.tempo=%6.3f;\n"
"  music.stacatto_factor=%6.3f;\n"
"  try {\n"
"  while (!ScanKey) {\n"
"\tPlay(\"",music.tempo+0.0005,music.stacatto_factor+0.0005);

  tempo=oc.root.next;
  tempo1=tempo;
  has_words=FALSE;
  while (tempo!=&oc.root) {
    DocPrint(doc,"%s",tempo->ascii);
    if (PsmHasWords(tempo->word)) has_words=TRUE;
    if (tempo->type==OT_METER) {
      measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
      two_measure_left=0;
    } else
      two_measure_left-=psm_durations[tempo->duration];
    tempo=tempo->next;
    if (two_measure_left<0.001 && tempo!=&oc.root) {
      if (has_words) {
	DocPrint(doc,"\",\n\t\t\"");
	while (tempo1!=tempo) {
	  if (tempo1->type!=OT_METER) {
	    if (ptr=tempo1->word) {
	      while (ch=*ptr) {
		if (ch==CH_SPACE)
		  *ptr=CH_SHIFT_SPACE;
		ptr++;
	      }
	      DocPrint(doc,"%Q\\0",tempo1->word);
	    } else
	      DocPrint(doc,"%c\\0",CH_SHIFT_SPACE);
	  }
	  tempo1=tempo1->next;
	}
      }
      DocPrint(doc,"\");\n"
		"\tPlay(\"");
      two_measure_left=2*measure_len;
      tempo1=tempo;
      has_words=FALSE;
    }
  }
  if (has_words) {
    DocPrint(doc,"\",\n\t\t\"");
    while (tempo1!=tempo) {
      if (tempo1->type!=OT_METER) {
	if (ptr=tempo1->word) {
	  while (ch=*ptr) {
	    if (ch==CH_SPACE)
	      *ptr=CH_SHIFT_SPACE;
	    ptr++;
	  }
	  DocPrint(doc,"%Q\\0",tempo1->word);
	} else
	  DocPrint(doc,"%c\\0",CH_SHIFT_SPACE);
      }
      tempo1=tempo1->next;
    }
  }
  DocPrint(doc,"\");\n"
"  }\n"
"  } catch\n"
"    PutExcept;\n"
"  Snd(0);\n"
"}\n"
"\n"
"Song;\n");
  DocRecalc(doc);
  if (full_filename)
    Free(full_filename);
  else
    StrPrint(doc->filename.name,"%s/Temp.CPP.Z",dirname);
  DocWrite(doc,TRUE);
  full_filename=StrNew(doc->filename.name);
  DocDel(doc);
  return full_filename;
}

U0 PsmSetOctave(I64 octave)
{
  I64 i;
  U8 buf[136];
  CMenuEntry *tempse;
  for (i=0;i<=6;i++) {
    StrPrint(buf,"Snd/Octave%d",i);
    if (tempse=MenuEntryFind(Fs->cur_menu,buf)) {
      if (i==octave)
	tempse->checked=TRUE;
      else
	tempse->checked=FALSE;
    }
  }
  if (tempse=MenuEntryFind(Fs->cur_menu,"Snd/Reverb"))
    tempse->checked=psm_play_mode==1;
}

$SP,"<17>",BI=17$
 



#define PSM_NUM_KEYS	20
class PsmKey
{
  U8 x,w,h,ascii;
};

#define PSM_W_W	16
#define PSM_W_H	36
#define PSM_B_W	8
#define PSM_B_H	20

PsmKey psm_kbd[PSM_NUM_KEYS]=
{

{ 2*PSM_W_W-4,PSM_B_W,PSM_B_H,'e' },
{ 3*PSM_W_W-4,PSM_B_W,PSM_B_H,'r' },
{ 4*PSM_W_W-4,PSM_B_W,PSM_B_H,'t' },
{ 6*PSM_W_W-4,PSM_B_W,PSM_B_H,'u' },
{ 7*PSM_W_W-4,PSM_B_W,PSM_B_H,'i' },
{ 9*PSM_W_W-4,PSM_B_W,PSM_B_H,'p' },
{10*PSM_W_W-4,PSM_B_W,PSM_B_H,'[' },
{11*PSM_W_W-4,PSM_B_W,PSM_B_H,']' },

{ 0*PSM_W_W,PSM_W_W,PSM_W_H,'a' },
{ 1*PSM_W_W,PSM_W_W,PSM_W_H,'s' },
{ 2*PSM_W_W,PSM_W_W,PSM_W_H,'d' },
{ 3*PSM_W_W,PSM_W_W,PSM_W_H,'f' },
{ 4*PSM_W_W,PSM_W_W,PSM_W_H,'g' },
{ 5*PSM_W_W,PSM_W_W,PSM_W_H,'h' },
{ 6*PSM_W_W,PSM_W_W,PSM_W_H,'j' },
{ 7*PSM_W_W,PSM_W_W,PSM_W_H,'k' },
{ 8*PSM_W_W,PSM_W_W,PSM_W_H,'l' },
{ 9*PSM_W_W,PSM_W_W,PSM_W_H,';' },
{10*PSM_W_W,PSM_W_W,PSM_W_H,'\'' },
{11*PSM_W_W,PSM_W_W,PSM_W_H,'\n'},

};

U0 PsmDownKey(I64 x,I64 y)
{
  I64 i;
  PsmKey *o;
  y-=FONT_HEIGHT*13;
  if (0<=y<PSM_W_H) {
    x-=16;
    for (i=0;i<PSM_NUM_KEYS;i++) {
      o=&psm_kbd[i];
      if (o->x<=x<o->x+o->w && y<o->h) {
	Msg(MSG_KEY_DOWN,o->ascii,0);
	return;
      }
    }
  }
}

U0 PsmUpKey(I64 x,I64 y)
{
  I64 i;
  PsmKey *o;
  y-=FONT_HEIGHT*13;
  if (0<=y<PSM_W_H) {
    x-=16;
    for (i=0;i<PSM_NUM_KEYS;i++) {
      o=&psm_kbd[i];
      if (o->x<=x<o->x+o->w && y<o->h) {
	Msg(MSG_KEY_UP,o->ascii,0);
	return;
      }
    }
  }
}

U0 PsmMenu(I64 psm_octave)
{
  DocClear;
  "$$BD,WHITE$$$$FD,GREEN$$$$CM,0,12$$\n";
  Sprite($IB,"<17>",BI=17$);
  '\n\n\n\n\n';

  DrawDC2;

  "$$LTGREEN$$<SPACE>$$FG$$\t\tRest\n"
  "$$LTGREEN$$<BACKSPACE>$$FG$$\tDeletes Last Note\n"
  "$$LTGREEN$$Left Mouse$$FG$$\tDrag note or shift word\n"
  "$$LTGREEN$$Right Mouse$$FG$$\tChange duration or set word\n";
  Sprite($IB,"<6>",BI=6$,"$$SP+LA,\"\",LM=\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_LEFT);\",BI=%d$$");
  "   ";
  if (oc.playing)
    Sprite($IB,"<9>",BI=9$,"$$SP,\"\",LM=\"x\",BI=%d$$");
  else
    Sprite($IB,"<8>",BI=8$,"$$SP,\"\",LM=\"x\",BI=%d$$");
  "   ";
  if (oc.record_entry->checked) {
    oc.record_entry->checked=TRUE;
    Sprite($IB,"<10>",BI=10$,"$$SP,\"\",LM=\"z\",BI=%d$$");
  } else
    Sprite($IB,"<11>",BI=11$,"$$SP,\"\",LM=\"z\",BI=%d$$");
  "   ";
  Sprite($IB,"<7>",BI=7$,"$$SP+LA,\"\",LM=\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_RIGHT);\",BI=%d$$");
  "$$CM+LX,0,6$$";
  PsmSetOctave(psm_octave);
  WinMgrSync(2);
}

U0 PsmPushMode(I64 psm_octave)
{
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
  PsmMenu(psm_octave);
}

U0 PsmPopMode()
{
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  DCFill;
}

#define PSMF_CD		1
#define PSMF_INCOMPLETE	2

U0 Psalmody(U8 *dirname=NULL)
{
  Bool was_playing;
  I64 a1,a2,msg_code=0,col,
	note=NOTE_REST,octave=OCTAVE_REST,psm_octave=3,timeout_val,timeout_val2,
	old_doc_flags;
  U8 *filename=NULL,*st,*st2;
  CTask *play_task=Spawn(&PsmPlayTask,NULL,"Psalmody Play",,Fs);
  PsmNote *tempo;
  F64 f=0,last_f=0,psm_duration=1.0,
	d,evt_time=tS,note_down_time=tS;
  CCtrl *c=TempoNew;

  if (DocPut) old_doc_flags=DocPut->flags;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinBorder;
  WordStat;
  Preempt(OFF);

  MkDir("HOME/Psalmody");
  MusicSettingsRst;
  tempo_state.tempo=Round(TEMPO_RANGE*(music.tempo-0.5)/4.4);
  tempo_state.stacatto=Round(TEMPO_RANGE*(music.stacatto_factor-0.12)/0.88);

  if (DocPut) DocPut->flags|=DOCF_FORM;

  MemSet(&oc,0,sizeof(PsmCtrl));
  oc.screen_x=0;
  oc.root.next=oc.root.last=&oc.root;
  oc.clipboard.next=oc.clipboard.last=&oc.clipboard;
  oc.cur_note=&oc.root;
  oc.dc2=DCAlias;

  MenuPush(
	"File {"
	"  New(,'.');"
	"  ChgDir(MSG_CMD,PSMF_CD);"
	"  Open(,CH_CTRLO);"
	"  SaveAs(,CH_CTRLA);"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  Cut(,CH_CTRLX);"
	"  Copy(,CH_CTRLC);"
	"  Paste(,CH_CTRLV);"
	"  RightMenu(,'\n');"
	"  BackSpace(,CH_BACKSPACE);"
	"  DeleteNote(,,SC_DELETE);"
	"  ClearSong(,'.');"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  GoBegin(,,0x4CB0000044B);"
	"  GoEnd(,,0x4CD0000044D);"
	"}"
	"Song {"
	"  Play(,'x');"
	"  Record(,'z');"
	"  Random(,',');"
	"  MarkIncomplete(MSG_CMD,PSMF_INCOMPLETE);"
	"}"
	"Snd {"
	"  Reverb(,'c');"
	"  Octave0(,'0');"
	"  Octave1(,'1');"
	"  Octave2(,'2');"
	"  Octave3(,'3');"
	"  Octave4(,'4');"
	"  Octave5(,'5');"
	"  Octave6(,'6');"
	"}"
	"Help {"
	"  Help(,,SC_F1);"
	"}"
	);
  oc.incomplete_entry=MenuEntryFind(Fs->cur_menu,"Song/MarkIncomplete");
  oc.record_entry=MenuEntryFind(Fs->cur_menu,"Song/Record");

  WinMax;

  if (!dirname)
    dirname=StrNew("HOME/Psalmody");
  else
    dirname=StrNew(dirname);
  PsmPushMode(psm_octave);
  col=0;
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      was_playing=FALSE;
mo_start:
      if (ip.pos_text.y-Fs->win_top<18)
	msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|
	      1<<MSG_IP_MOVE|1<<MSG_CMD);
      else
	msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|1<<MSG_IP_MOVE|1<<MSG_CMD);
mo_got_msg:
      if (msg_code==MSG_KEY_DOWN && a1==CH_SPACE && !a2) {
//The Window Mgr sets the Doc cur_entry to a button
	//and generates a <SPACE> when the Doc Buttons are clicked.
	//This is so that kbd and mouse are the same for Doc's.
	//We must now pass the <SPACE> onto the Doc handler.
	PutKey(a1,a2);
	goto mo_start;
      }
      if (msg_code!=MSG_IP_MOVE) {
	DocBottom;
	if (was_playing || DocPut->cur_entry->y>=Fs->win_height-2) {
	  PsmMenu(psm_octave);
	  col=0;
	}
      }

      octave=psm_octave;
      switch (msg_code) {
	case MSG_CMD:
	  PsmPopMode;
	  switch (a1) {
	    case PSMF_CD:
	      st2=dirname;
	      if (dirname=PopUpPickDir) {
		Free(st2);
		Free(filename);
		filename=NULL;
	      } else
		dirname=st2;
	      break;
	    case PSMF_INCOMPLETE:
	      oc.incomplete_entry->checked=!oc.incomplete_entry->checked;
	      break;
	  }
	  PsmPushMode(psm_octave);
	  col=0;
	  break;
	case MSG_KEY_DOWN:
	  evt_time=tS;
	  if ('0'<=a1<='9') {
	    psm_octave=a1-'0';
	    PsmMenu(psm_octave);
	    col=0;
	  } else {
	    switch (a1) {
	      sub_switch_start:
		case 'a': note=7; octave--; break;
		case 's': note=8; octave--; break;
		case 'e': note=9; octave--; break;
		case 'd': note=10; octave--; break;
		case 'r': note=11; octave--; break;
		case 'f': note=0; break;
		case 't': note=1; break;
		case 'g': note=2; break;
		case 'h': note=3; break;
		case 'u': note=4; break;
		case 'j': note=5; break;
		case 'i': note=6; break;
		case 'k': note=7; break;
		case 'l': note=8; break;
		case 'p': note=9; break;
		case ';': note=10; break;
		case '[': note=11; break;
		case '\'': note=0; octave++; break;
		case ']': note=1; octave++; break;
	      sub_switch_end:
		f=Note2Freq(note,octave);
		break;
	      case CH_SPACE:
		note=NOTE_REST;
		octave=OCTAVE_REST;
		f=-1; //rest
		break;

	      case 0:
		switch (a2.u8[0]) {
		  case SC_CURSOR_LEFT:
		    if (a2&SCF_CTRL) {
		      while (oc.cur_note->last!=&oc.root) {
			oc.cur_note=oc.cur_note->last;
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,a2&SCF_SHIFT);
		      }
		    } else {
		      if (oc.cur_note->last!=&oc.root) {
			oc.cur_note=oc.cur_note->last;
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,a2&SCF_SHIFT);
		      }
		    }
		    break;
		  case SC_CURSOR_RIGHT:
		    if (a2&SCF_CTRL) {
		      while (oc.cur_note!=&oc.root) {
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,a2&SCF_SHIFT);
			oc.cur_note=oc.cur_note->next;
		      }
		    } else {
		      if (oc.cur_note!=&oc.root) {
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,a2&SCF_SHIFT);
			oc.cur_note=oc.cur_note->next;
		      }
		    }
		    break;
		  case SC_DELETE:
		    if (a2&SCF_SHIFT)
		      PsmCutToClipboard;
		    else {
		      tempo=oc.cur_note;
		      oc.cur_note=tempo->next;
		      if (tempo!=&oc.root) {
			QueRem(tempo);
			PsmNoteDel(tempo);
		      }
		    }
		    break;
		  case SC_INS:
		    if (a2&SCF_SHIFT)
		      PsmPasteClipboard;
		    else if (a2&SCF_CTRL)
		      PsmCopyToClipboard;
		    break;
		  case SC_F1:
		    PsmPopMode;
		    PopUpEd("::/Apps/Psalmody/Help.TXT.Z",Fs);
		    PsmPushMode(psm_octave);
		    col=0;
		    break;
		}
		break;
	      case ',':
		Free(filename);
		filename=NULL;
		PsmPopMode;
		music.octave=psm_octave;
		if (st2=GodSongStr) {
		  PsmLoadSongStr(st2,&psm_octave,&psm_duration);
		  Free(st2);
		}
		PsmPushMode(psm_octave);
		col=0;
		break;
	      case CH_CTRLO:
		PsmPopMode;
		AcctOneTimePopUp(ARf_PSALMODY_JUKEBOX,
		      "Select a song and preview it.\n"
		      "$$GREEN$$<SHIFT-ESC>$$FG$$ to load it into Psalmody.\n\n"
		      ST_WARN_ST " Graphics and other embelishments\n"
		      "will be lost because Psalmody can't\n"
		      "parse HolyC programs completely.\n");
		Free(filename);
		filename=NULL;
		JukeBox(dirname,&filename);
		if (filename) {
		  oc.screen_x=0;
		  psm_duration=1.0;
		  psm_octave=3;
		  PsmSongDel(&oc.root);
		  oc.cur_note=&oc.root;
		  PsmLoadSong(filename,&psm_octave,&psm_duration);
		  oc.record_entry->checked=FALSE;
		  oc.cur_note=oc.root.next;
		}
		PsmPushMode(psm_octave);
		col=0;
		break;
	      case CH_CTRLA:
		PsmPopMode;
		filename=PsmSaveSong(dirname,filename);
		PsmPushMode(psm_octave);
		break;
	      case CH_CTRLC:
		PsmCopyToClipboard;
		break;
	      case CH_CTRLV:
		PsmPasteClipboard;
		break;
	      case CH_CTRLX:
		PsmCutToClipboard;
		break;
	      case '.':
		PsmMenu(psm_octave);
		col=0;
		Free(filename);
		filename=NULL;
		psm_duration=1.0;
		psm_octave=3;
		PsmSongDel(&oc.root);
		oc.cur_note=&oc.root;
		oc.screen_x=0;
		break;
	      case '\n':
		if (oc.cur_note!=&oc.root)
		  PsmRightClick(oc.cur_note->x,oc.cur_note->y);
		break;
	      case 'x':
		if (was_playing)
		  break;
		col=0;
		oc.playing=TRUE;
		PsmMenu(psm_octave);
		tempo=oc.cur_note;
		while (tempo!=&oc.root) {
		  if (tempo->type!=OT_METER) {
		    timeout_val=cnts.jiffies;
		    if (ip.pos_text.y-Fs->win_top<18)
		      msg_code=ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_R_UP|1<<MSG_CMD);
		    else
		      msg_code=ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_CMD);
		    if (msg_code) {
		      f=0;
		      psm_play_f=0;
		      oc.playing=FALSE;
		      was_playing=TRUE;
		      if (ip.pos_text.y-Fs->win_top>=18 && msg_code==MSG_IP_L_DOWN)
			goto mo_start;
		      else
			goto mo_got_msg;
		    }
		    oc.cur_note=tempo;
		    oc.screen_x+=tempo->x-0.33*GR_WIDTH;
		    if (PsmHasWords(tempo->word))
		      "%s",tempo->word;
		    note=tempo->note;
		    octave=tempo->octave;
		    if (note!=NOTE_REST)
		      f=Note2Freq(note,octave);
		    else
		      f=-1; //rest
		    if (f>0)
		      psm_play_f=f;
		    else
		      psm_play_f=0;

		    music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
		    music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;
		    d=JIFFY_FREQ*psm_durations[tempo->duration]/music.tempo;
		    if (Bt(&tempo->flags,Of_TIE)) {
		      timeout_val+=d;
		      timeout_val2=timeout_val;
		    } else {
		      timeout_val+=d*music.stacatto_factor;
		      timeout_val2=timeout_val+
			    d*(1.0-music.stacatto_factor);
		    }
		    SleepUntil(timeout_val);
		    psm_play_f=0;
		    SleepUntil(timeout_val2);
		  }
		  tempo=tempo->next;
		}
		oc.cur_note=&oc.root;
		oc.screen_x+=oc.cur_note->x-GR_WIDTH/2;
		oc.playing=FALSE;
		PsmMenu(psm_octave);
		col=0;
		f=0;
		psm_play_f=0;
		break;
	      case CH_BACKSPACE:
		tempo=oc.cur_note->last;
		if (tempo!=&oc.root) {
		  QueRem(tempo);
		  PsmNoteDel(tempo);
		}
		if (col) {
		  '' CH_BACKSPACE;
		  col--;
		}
		break;
	      case 'c':
		psm_play_mode++;
		if (psm_play_mode>=PSMPM_NUM_PLAY_MODES) psm_play_mode=0;
		PsmMenu(psm_octave);
		col=0;
		break;
	      case 'z':
		if (oc.record_entry->checked)
		  oc.record_entry->checked=FALSE;
		else {
		  oc.record_entry->checked=TRUE;
		  psm_duration=1.0;
		  psm_octave=3;
		  oc.screen_x=0;
		}
		PsmMenu(psm_octave);
		col=0;
		break;
	      case CH_ESC:
		PsmPopMode;
		filename=PsmSaveSong(dirname,filename);
		PsmPushMode(psm_octave);
	      case CH_SHIFT_ESC:
		goto mo_done;
	    }
	  }
	  break;
	case MSG_KEY_UP:
	  evt_time=tS;
	  f=0;
	  break;
	case MSG_IP_MOVE:
	  if (a2>18*FONT_HEIGHT)
	    Fs->win_inhibit=WIG_USER_TASK_DFT;
	  else
	    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
	  break;
	case MSG_IP_L_DOWN:
	  PsmDownKey(a1,a2);
	  PsmLeftClick(a1,a2);
	  break;
	case MSG_IP_L_UP:
	  PsmUpKey(a1,a2);
	  break;
	default:
	  PsmRightClick(a1,a2);
      }
      if (f!=last_f) {
	if (f>0) {
	  st=LstSub(note,psm_note_lst);
	  psm_play_f=f;
	} else {
	  if (f<0)
	    st="R";
	  else
	    st="";
	  psm_play_f=0;
	}
	if (oc.record_entry->checked) {
	  if (last_f) {
	    music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
	    music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

	    tempo->duration=PsmCvtDuration(music.tempo*(evt_time-note_down_time));
	    PsmSetWidth(tempo);
	    QueIns(tempo,oc.cur_note->last);
	  }
	  if (f) {
	    note_down_time=tS;
	    tempo=CAlloc(sizeof(PsmNote));
	    tempo->type=OT_NOTE;
	    tempo->note=note;
	    tempo->octave=octave;
	    if (st[1]=='#')
	      Bts(&tempo->flags,Of_SHARP);
	  }
	}
	last_f=f;
	"%s",st;
	col+=StrLen(st);
	if (col>=Fs->win_width-1) {
	  '\n';
	  col=0;
	}
      }
    }
mo_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  PsmPopMode;
  PsmSongDel(&oc.root);
  PsmSongDel(&oc.clipboard);
  TempoDel(c);
  Kill(play_task);
  DCFill;
  DCDel(oc.dc2);
  DocClear;
  SettingsPop;
  if (DocPut) DocPut->flags=DocPut->flags&~DOCF_FORM|old_doc_flags&DOCF_FORM;
  Free(dirname);
  MenuPop;
}
        "     
   òÿÿÿ   ñÿÿÿ
   ñÿÿÿ
   ñÿÿÿ
   õÿÿÿ
   õÿÿÿ
   óÿÿÿ	   òÿÿÿ
   öÿÿÿ
   ÷ÿÿÿ
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ
          òÿÿÿ
þÿÿÿ    ÿÿÿÿÿÿÿÿ
    ýÿÿÿ       
ÿÿÿÿ          
           ýÿÿÿ
    ýÿÿÿÿÿÿÿ    
               
þÿÿÿÿÿÿÿ   ÿÿÿÿ              
   óÿÿÿ
   óÿÿÿ
   óÿÿÿ   óÿÿÿ
   òÿÿÿ   òÿÿÿ
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ
          òÿÿÿ
þÿÿÿ    ÿÿÿÿÿÿÿÿ
    ýÿÿÿ       
ÿÿÿÿ          
           ýÿÿÿ
    ýÿÿÿÿÿÿÿ    
               
þÿÿÿÿÿÿÿ   ÿÿÿÿ        Í      
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ
          òÿÿÿ
þÿÿÿ    ÿÿÿÿÿÿÿÿ
    ýÿÿÿ       
ÿÿÿÿ          
           ýÿÿÿ
    ýÿÿÿÿÿÿÿ    
               
þÿÿÿÿÿÿÿ   ÿÿÿÿ        V      
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ
          òÿÿÿ        E      
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ        a      
                              
            
            
                    a      
                              
            
            
                    Õ      
                              

      
      
      
      
      
      

      
                           
      
      	      	      	      	                
            
                           
         
         
   	      
         
         

   	         
      	      
   
   
      
   
   	   	   
   
      
   
            
   
      
   

      
      
   
      
   
   
      
           ?      
                           
         
   	           #      
þÿÿÿÖÿÿÿþÿÿÿ&   
   Öÿÿÿ   &           E      
   úÿÿÿ      
   úÿÿÿ
      
          ýÿÿÿ
                    V      
	   ýÿÿÿ      
      
      

            
      
      

                                            E      
þÿÿÿöÿÿÿ    öÿÿÿ
    öÿÿÿ    òÿÿÿ
    òÿÿÿþÿÿÿòÿÿÿ
    ôÿÿÿÿÿÿÿôÿÿÿ        ò      
         $   
    $           
0   $   0      
,      4      
4      4       
,      ,       
@   $   @      
<      D      
D      D       
<      <       
|      „      
„      „       
|      |       
l      t      
t      t       
l      l       
   $   À   $   
            
             
Ä                    A $      S 4      D D      F T      G d      H t      J „      K ”      L ¤      ; ´      ' ,      E <      R L      T l      U |      I œ      P ¬      [ ¼      ] 
P      P      
L      T      
T      T       
L      L       
`   $   `       
p   $   p      
€   $   €      
   $          
    $          
°   $   °      
À   $   À      
œ      ¤      
¤      ¤       
œ      œ       
¬      ´      
´      ´       
¬      ¬       
¼      Ä      
Ä      Ä       
¼      ¼       
P   $   P      0      @      P      p      €             °      À      @   
   ~          
    	       P      
                              

            
	      	       