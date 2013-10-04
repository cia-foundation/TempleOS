/*$WW,1$
This uses four types of ctrls

  1) The pull-down menu.

  2) The $LK,"CCtrl","MN:CCtrl"$ type for the tempo/stacatto sliders.

  3) The active $LK,"CDoc","MN:CDoc"$ sprite buttons with macros for left/record/play/right.

  4) Hand-made ctrls for the kbdgraphic, notes and staff.




This uses four types of output

  1) The $LK,"CDoc","MN:CDoc"$ for the text and buttons and kbdgraphic.

  2) The Fs->draw_it() for the staff region, drawn every refresh.

  3) The gr_dc persistent layer for the note chooser, meter chooser.  The persistent layer is used during drag-and-drop.

  4) The Fs->next_ctrl for the tempo/stacatto sliders.


See $LK,"GrUpdateWins","MN:GrUpdateWins"$() and $LK,"GrUpdateScreen","MN:GrUpdateScreen"$().
$WW,1$

$WW,0$*/

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

class OrgNote
{
  OrgNote *next,*last;
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

class OrgCtrl
{
  OrgNote root;
  OrgNote clipboard;
  CMenuEntry *incomplete_entry,*record_entry;
  I64 screen_x,
     tool;
  OrgNote *cur_note;
  CDC *dc2;
  Bool playing;
} oc;

U8 *org_note_lst="A\0A#\0B\0C\0C#\0D\0D#\0E\0F\0F#\0G\0G#\0";
U8 org_note_map[12]={6,6,5,4,4,3,3,2,1,1,0,0};
U8 org_note_inverse_map[7]={10,8,7,5,3,2,0};


	$SP,"<1>",1$



	$SP,"<2>",2$



	$SP,"<3>",3$



	$SP,"<4>",4$



	$SP,"<5>",5$


	$SP,"<6>",6$


	$SP,"<7>",7$

	
	$SP,"<8>",8$


	$SP,"<9>",9$


	$SP,"<10>",10$


	$SP,"<11>",11$







	$SP,"<12>",12$



	


	$SP,"<13>",13$


	$SP,"<14>",14$


	$SP,"<15>",15$




	$SP,"<16>",16$

#define ORG_NOTE_SPACING	9
#define ORG_NUM_DURATIONS	12

F64 org_durations[ORG_NUM_DURATIONS+1]={
2*.25/3,.25,2*.5/3,.5,2.0/3.0,0.5*1.5,1.0,1.5,2.0,3.0,4.0,6.0,1000000.0};
U8 *org_duration_lst="st\0s\0et\0e\0qt\0e.\0q\0q.\0h\0h.\0w\0w.\0";
U8 *org_duration_imgs[ORG_NUM_DURATIONS]={$IB,"<1>",1$,$IB,"<1>",1$,$IB,"<2>",2$,
			$IB,"<2>",2$,$IB,"<3>",3$,$IB,"<2>",2$,$IB,"<3>",3$,$IB,"<3>",3$,
			$IB,"<4>",4$,$IB,"<4>",4$,$IB,"<5>",5$,$IB,"<5>",5$};
Bool org_triplet_durations[ORG_NUM_DURATIONS]={TRUE,FALSE,TRUE,
			FALSE,TRUE,FALSE,FALSE,FALSE,
			FALSE,FALSE,FALSE,FALSE};

Bool OrgHasWords(U8 *st)
{
  if (st && *st && (StrLen(st)>1 || *st!=CH_SPACE && *st!=CH_SHIFT_SPACE))
    return TRUE;
  else
    return FALSE;
}

Bool OrgIsDotted(I64 duration)
{
  U8 *st=LstSub(duration,org_duration_lst);
  if (st[1]=='.')
    return TRUE;
  else
    return FALSE;
}

U0 OrgSetWidth(OrgNote *tempo)
{
  if (tempo->type==OT_METER)
    tempo->width=12;
  else {
    tempo->width=ORG_NOTE_SPACING;
    if (OrgIsDotted(tempo->duration))
      tempo->width+=ORG_NOTE_SPACING/2;
    if (Bt(&tempo->flags,Of_SHARP) || Bt(&tempo->flags,Of_FLAT))
      tempo->width+=ORG_NOTE_SPACING;
  }
}

U0 OrgRecalcNoteXY()
{
  F64 measure_len=4,measure_left=measure_len;
  OrgNote *tempo=oc.root.next;
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
	  note=music_note_map[*LstSub(note,org_note_lst)-'A'];
	  tempo->y=(15+(org_note_map[note]-7*(octave-2)))*4;
	}
      }
      x+=tempo->width;
      if (tempo->type==OT_METER) {
	measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
	measure_left=0;
      } else
	measure_left-=org_durations[tempo->duration];
      if (measure_left<0.001) {
	x+=ORG_NOTE_SPACING;
	measure_left=measure_len;
      }
    }
    tempo=tempo->next;
  }
}

U0 OrgMarkSelected(I64 x1,I64 x2,Bool sel)
{
  OrgNote *tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (sel) {
      if (x1<=tempo->x<=x2)
	tempo->flags|=OF_SELECTED;
    } else
      tempo->flags&=~OF_SELECTED;
    tempo=tempo->next;
  }
}

OrgNote *OrgNoteCopy(OrgNote *tempo)
{
  OrgNote *tempo1=MAllocIdentical(tempo);
  if (tempo->word)
    tempo1->word=StrNew(tempo->word);
  else
    tempo1->word=NULL;
  return tempo1;
}

U0 OrgNoteDel(OrgNote *tempo)
{
  Free(tempo->word);
  Free(tempo);
}

U0 OrgSongDel(OrgNote *root)
{
  OrgNote *tempo,*tempo1;
  tempo=root->next;
  while (tempo!=root) {
    tempo1=tempo->next;
    OrgNoteDel(tempo);
    tempo=tempo1;
  }
  root->next=root->last=root;
}

U0 OrgCutToClipboard()
{
  OrgNote *tempo,*tempo1;
  OrgSongDel(&oc.clipboard);
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

U0 OrgPasteClipboard()
{
  OrgNote *tempo,*tempo1;
  tempo=oc.clipboard.next;
  while (tempo!=&oc.clipboard) {
    tempo1=OrgNoteCopy(tempo);
    QueIns(tempo1,oc.cur_note->last);
    tempo=tempo->next;
  }
}

U0 OrgCopyToClipboard()
{
  OrgNote *tempo,*tempo1;
  OrgSongDel(&oc.clipboard);
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (tempo->flags&OF_SELECTED) {
      tempo->flags&=~OF_SELECTED;
      tempo1=OrgNoteCopy(tempo);
      QueIns(tempo1,oc.clipboard.last);
    }
    tempo=tempo->next;
  }
}

OrgNote *OrgFindNote(I64 x,I64)
{
  OrgNote *tempo=oc.root.next;
  OrgRecalcNoteXY;
  x+=ORG_NOTE_SPACING/2;
  while (x>tempo->next->x && tempo!=&oc.root)
    tempo=tempo->next;
  return tempo;
}

Bool OrgBlink()
{
  if (Blink || oc.playing)
    return TRUE;
  else
    return FALSE;
}

U0 DrawNote(CDC *dc,I64 x,I64 y,I64 duration)
{
  if (0<=duration<=ORG_NUM_DURATIONS) {
    SpritePlot3(dc,x,y,0,org_duration_imgs[duration]);
    if (org_triplet_durations[duration])
      SpritePlot3(dc,x,y,0,$IB,"<16>",16$);
    if (OrgIsDotted(duration))
      SpritePlot3(dc,x,y,0,$IB,"<15>",15$);
  }
}

U0 DrawTimeSignature(CDC *dc,I64 x,I64 y,I64 top,I64 bottom)
{
  GrPrint(dc,x,y,"%d",top);
  GrPrint(dc,x,y+FONT_HEIGHT,"%d",bottom);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  OrgNote *tempo;
  I64 i,x,y,
     w=task->win_pixel_width;
  F64 measure_len=4,measure_left=measure_len;
  Bool old_preempt=Preempt(OFF);

  dc->color=BLACK;
  for (i=1;i<6;i++)
    GrLine(dc,0,i*8,w,i*8);
  for (i=7;i<12;i++)
    GrLine(dc,0,i*8,w,i*8);

  OrgRecalcNoteXY;
  if (oc.cur_note->x<64) {
    oc.screen_x-=128;
    OrgRecalcNoteXY;
  }
  if (oc.cur_note->x>=GR_WIDTH-64) {
    oc.screen_x+=128;
    OrgRecalcNoteXY;
  }
 
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    x=tempo->x;
    y=tempo->y;
    if (measure_left<0.001) {
      dc->color=BLACK;
      GrLine(dc,x-ORG_NOTE_SPACING,8,x-ORG_NOTE_SPACING,11*8);
      measure_left=measure_len;
    }
    if (tempo->type==OT_METER) {
      if (tempo==oc.cur_note && OrgBlink)
	dc->color=BROWN;
      else
	dc->color=BLACK;
      DrawTimeSignature(dc,x,5*8,tempo->meter_top,tempo->meter_bottom);
      measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
      measure_left=0;
    } else {
      if (tempo==oc.cur_note && OrgBlink) {
	dc->color=BROWN;
	GrPrint(dc,x+8,y,"%s",tempo->word);
      } else if (tempo->flags&OF_SELECTED)
	dc->color=RED;
      else {
	if (tempo->note!=NOTE_REST)
	  dc->color=BLACK;
	else
	  dc->color=LTGRAY;
	if (OrgHasWords(tempo->word))
	  dc->color=GREEN;
      }
      DrawNote(dc,x,y,tempo->duration);
      if (OrgIsDotted(tempo->duration))
	x+=ORG_NOTE_SPACING/2;
      if (Bt(&tempo->flags,Of_SHARP))
	SpritePlot3(dc,x,y,0,$IB,"<13>",13$);
      else if (Bt(&tempo->flags,Of_FLAT))
	SpritePlot3(dc,x,y,0,$IB,"<14>",14$);
      measure_left-=org_durations[tempo->duration];
    }
    tempo=tempo->next;
  }

  if (oc.cur_note==&oc.root && OrgBlink)
    dc->color=BROWN;
  else
    dc->color=BLACK;
  SpritePlot3(dc,oc.root.x,50,0,$IB,"<12>",12$);
  Preempt(old_preempt);
}

#define ORGR_STRONG_ACCENT	-11
#define ORGR_ACCENT		-10
#define ORGR_NO_ACCENT		-9
#define ORGR_FLAT		-8
#define ORGR_SHARP		-7
#define ORGR_TIE		-6
#define ORGR_REST		-5
#define ORGR_INS_NOTE	-4
#define ORGR_DELETE_NOTE	-3
#define ORGR_SET_WORD		-2

F64 PopUpDuration()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,GREEN$$$$MU,\"Set Word\",ORGR_SET_WORD$$\n"
	"$$MU,\"Toggle Sharp\",ORGR_SHARP$$\n"
	"$$MU,\"Toggle Flat\",ORGR_FLAT$$\n"
	"$$MU,\"Toggle Tie\",ORGR_TIE$$\n"
	"$$MU,\"Make Rest\",ORGR_REST$$\n"
	"$$MU,\"Insert Note\",ORGR_INS_NOTE$$\n"
	"$$MU,\"Delete Note\",ORGR_DELETE_NOTE$$\n"
	"$$MU,\"No Accent\",ORGR_NO_ACCENT$$\n"
	"$$MU,\"Accent\",ORGR_ACCENT$$\n"
	"$$MU,\"Strong Accent\",ORGR_STRONG_ACCENT$$\n\n");
  for (i=0;i<ORG_NUM_DURATIONS;i++)
    DocPrint(doc,"$$MU,\"%7.5f\",%d$$\n",org_durations[i],i);
  DocPrint(doc,"\n$$MU,\"CANCEL\",DOCM_CANCEL$$\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 OrgRightClick(I64 x,I64 y)
{
  U8 *st,*st2;
  OrgNote *tempo,*tempo1;
  I64 i,old_doc_flags;
  if (DocPut) old_doc_flags=DocPut->flags;
  oc.cur_note=tempo=OrgFindNote(x,y);
  if (tempo!=&oc.root) {
    Fs->win_inhibit=0;
    i=PopUpDuration;
    if (0<=i<ORG_NUM_DURATIONS) {
      if (tempo->type==OT_NOTE)
	tempo->duration=i;
    } else {
      switch (i) {
	case ORGR_REST:
	  if (tempo->type==OT_NOTE) {
	    tempo->octave=OCTAVE_REST;
	    tempo->note=NOTE_REST;
	  }
	  break;
	case ORGR_SHARP:
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
	case ORGR_FLAT:
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
	case ORGR_TIE:
	  if (tempo->type==OT_NOTE)
	    Btc(&tempo->flags,Of_TIE);
	  break;
	case ORGR_SET_WORD:
	  if (tempo->type==OT_NOTE) {
	    if (DocPut) DocPut->flags&=~DOCF_FORM;
	    if (OrgHasWords(tempo->word))
	      st2=MPrint("\nWord(\"%Q\"):",tempo->word);
	    else
	      st2=MPrint("\nWord(\"\"):");
	    DocBottom;
	    st=PmtStr(st2);
	    Free(st2);
	    Free(tempo->word);
	    if (*st) {
	      tempo->word=MPrint("%q",st);
	      Free(st);
	    } else
	      tempo->word=StrNew(" ");
	    if (DocPut) DocPut->flags=DocPut->flags&~DOCF_FORM|old_doc_flags&DOCF_FORM;
	  }
	  break;
	case ORGR_INS_NOTE:
	  tempo1=OrgNoteCopy(tempo);
	  QueIns(tempo1,tempo);
	  break;
	case ORGR_DELETE_NOTE:
	  oc.cur_note=tempo->next;
	  QueRem(tempo);
	  OrgNoteDel(tempo);
	  break;
      }
    }
    OrgSetWidth(oc.cur_note);
    Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU-WIF_CTRLS;
  }
}

#define ORG_NOTE_BOX_X	220
#define ORG_NOTE_BOX_Y	(13*FONT_HEIGHT+14)

U0 OrgSetPickNoteBoxX(I64 duration,I64 *x)
{
  I64 i;
  *x=ORG_NOTE_BOX_X;
  for (i=0;i<duration;i++) {
    if (OrgIsDotted(i))
      *x+=ORG_NOTE_SPACING/2;
    *x+=ORG_NOTE_SPACING+4;
  }
}

I64 OrgGetPickNoteBoxDuration(I64 xx,I64 yy)
{
  I64 i,x1,x2;
  if (ORG_NOTE_BOX_Y-14<=yy<ORG_NOTE_BOX_Y+6) {
    for (i=0;i<ORG_NUM_DURATIONS;i++) {
      OrgSetPickNoteBoxX(i,&x1);
      OrgSetPickNoteBoxX(i+1,&x2);
      if (x1<=xx+ORG_NOTE_SPACING/2<x2)
	return i;
    }
  }
  return -1;
}

U0 DrawPickNoteBox()
{
  I64 i,x;
  for (i=0;i<ORG_NUM_DURATIONS;i++) {
    OrgSetPickNoteBoxX(i,&x);
    if (OrgIsDotted(i))
      oc.dc2->color=RED;
    else if (org_triplet_durations[i])
      oc.dc2->color=LTRED;
    else
      oc.dc2->color=BLACK;
    DrawNote(oc.dc2,x,ORG_NOTE_BOX_Y,i);
  }
}

#define ORG_TOOLS_X	450
#define ORG_TOOLS_Y	13*FONT_HEIGHT

U0 DrawPickTools()
{
  if (oc.tool==OTT_BOX_TOOL)
    oc.dc2->color=ROPF_DITHER+WHITE<<16+RED;
  else
    oc.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
  GrLineRect4(oc.dc2,ORG_TOOLS_X,ORG_TOOLS_Y,
    ORG_TOOLS_X+10,ORG_TOOLS_Y+10);

  if (oc.tool==OTT_PTR_TOOL)
    oc.dc2->color=RED;
  else
    oc.dc2->color=BLACK;
  (*fp_draw_input_ptr)(oc.dc2,ORG_TOOLS_X+15,ORG_TOOLS_Y);
  oc.dc2->color=BLACK;
}


Bool OrgGetPickToolBox(I64 xx,I64 yy)
{
  if (ORG_TOOLS_X<=xx<ORG_TOOLS_X+27 &&
      ORG_TOOLS_Y<=yy<ORG_TOOLS_Y+15) {
    OrgMarkSelected(0,0,FALSE);
    if (xx<ORG_TOOLS_X+13)
      oc.tool=OTT_BOX_TOOL;
    else
      oc.tool=OTT_PTR_TOOL;
    return TRUE;
  } else
    return FALSE;
}

#define ORG_NUM_METERS	7
I64 meter_tops[ORG_NUM_METERS]	={2,3,4,5,6,7,9},
   meter_bottoms[ORG_NUM_METERS]={4,4,4,4,8,8,8};

#define ORG_METER_X	485
#define ORG_METER_Y	13*FONT_HEIGHT
#define ORG_METER_W	12
Bool OrgGetPickMeterBox(I64 xx,I64 yy,I64 *top,I64 *bottom)
{
  I64 i;
  if (ORG_METER_X<=xx<ORG_METER_X+ORG_METER_W*ORG_NUM_METERS &&
      ORG_METER_Y<=yy<ORG_METER_Y+2*FONT_HEIGHT) {
    i=(xx-ORG_METER_X)/ORG_METER_W;
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
  for (i=0;i<ORG_NUM_METERS;i++)
    DrawTimeSignature(oc.dc2,ORG_METER_X+i*ORG_METER_W,ORG_METER_Y,meter_tops[i],meter_bottoms[i]);
}

U0 DrawDC2()
{
  DCFill;
  DrawPickNoteBox;
  DrawPickMeterBox;
  DrawPickTools;
}

U0 OrgLeftClickPickNoteBox(I64 duration)
{
  I64 o,n,msg_code,p1,p2;
  OrgNote *tempo,*tempo1;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawNote(oc.dc2,p1,p2,duration);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (p2<13*FONT_HEIGHT) {
    if (p1>oc.root.last->x)
      tempo1=oc.root.last;
    else if (p1<oc.root.next->x)
      tempo1=&oc.root;
    else
      tempo1=OrgFindNote(p1-ORG_NOTE_SPACING/2,p2);
    tempo=CAlloc(sizeof(OrgNote));
    tempo->type=OT_NOTE;
    p2=p2/4-15;
    n=-p2%7;
    o=2+p2/-7;
    n=-n;
    if (n<0) {
      n+=7;
      o++;
    }
    n=org_note_inverse_map[n];
    tempo->note=n;
    tempo->octave=o;
    tempo->duration=duration;
    OrgSetWidth(tempo);
    QueIns(tempo,tempo1);
    oc.cur_note=tempo->next;
  }
  DrawDC2;
}

U0 OrgLeftClickPickMeterBox(I64 top,I64 bottom)
{
  I64 msg_code,p1,p2;
  OrgNote *tempo,*tempo1;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawTimeSignature(oc.dc2,p1,p2,top,bottom);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (p2<13*FONT_HEIGHT) {
    if (p1>=oc.root.x)
      tempo1=oc.root.last;
    else if (p1<oc.root.next->x)
      tempo1=&oc.root;
    else
      tempo1=OrgFindNote(p1-ORG_NOTE_SPACING/2,p2);
    tempo=CAlloc(sizeof(OrgNote));
    tempo->type=OT_METER;
    tempo->meter_top=top;
    tempo->meter_bottom=bottom;
    OrgSetWidth(tempo);
    QueIns(tempo,tempo1);
    oc.cur_note=tempo->next;
  }
  DrawDC2;
}

U0 OrgLeftClickStaffPtr(I64 x,I64 y)
{
  OrgNote *tempo,*tempo1;
  I64 o,n,msg_code,p1,p2,n_original,o_original;
  oc.cur_note=tempo=OrgFindNote(x,y);
  if (tempo!=&oc.root) {
    if (tempo->type==OT_NOTE) {
      n_original=tempo->note;
      o_original=tempo->octave;
      do {
	msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
	if (msg_code==MSG_IP_L_UP) {
	  tempo1=OrgFindNote(p1,p2);
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
	  p2=p2/4-15;
	  n=-p2%7;
	  o=2+p2/-7;
	  n=-n;
	  if (n<0) {
	    n+=7;
	    o++;
	  }
	  n=org_note_inverse_map[n];
	  tempo->note=n;
	  tempo->octave=o;
	}
      } while (msg_code!=MSG_IP_L_UP);
      OrgSetWidth(tempo);
    }
  }
}

U0 OrgLeftClickStaffBox(I64 x,I64 y)
{
  I64 msg_code,p1,p2;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    DrawDC2;
    oc.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrLineRect4(oc.dc2,x,y,p1,p2);
    if (msg_code==MSG_IP_L_UP) {
      if (x>p1) SwapU64(&x,&p1);
      OrgMarkSelected(x,p1,TRUE);
    }
  } while (msg_code!=MSG_IP_L_UP);
  DrawDC2;
}

U0 OrgLeftClick(I64 x,I64 y)
{
  I64 duration,top,bottom;
  if (y<13*FONT_HEIGHT) {
    if (oc.tool==OTT_PTR_TOOL)
      OrgLeftClickStaffPtr(x,y);
    else
      OrgLeftClickStaffBox(x,y);
  } else {
    duration=OrgGetPickNoteBoxDuration(x,y);
    if (0<=duration<ORG_NUM_DURATIONS)
      OrgLeftClickPickNoteBox(duration);
    else if (OrgGetPickMeterBox(x,y,&top,&bottom))
      OrgLeftClickPickMeterBox(top,bottom);
    else if (OrgGetPickToolBox(x,y))
      DrawDC2;
  }
}


#define ORGPM_NORMAL		0
#define ORGPM_REVERB1		1
#define ORGPM_NUM_PLAY_MODES	2

I64 org_play_mode=0;
F64 org_play_f=0;

U0 OrgPlayTask(I64)
{
  I64 i=0;
  F64 f=0,f2=0;
  Fs->task_end_cb=&SndTaskEndCB;
  while (TRUE) {
    i++;
    switch (org_play_mode) {
      case ORGPM_NORMAL:
	if (org_play_f!=f) {
	  f=org_play_f;
	  Snd(f);
	}
	break;
      case ORGPM_REVERB1:
	if (org_play_f!=f) {
	  if (f>0)
	    f2=f;
	  f=org_play_f;
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


U8 OrgCvtDuration(F64 d)
{
  F64 d1,d2;
  I64 j;
  for (j=0;j<ORG_NUM_DURATIONS;j++) {
    d1=org_durations[j];
    d2=org_durations[j+1];
    if (d<d1*d2/(d1+d2))
      return j;
  }
  return 0;
}


U8 *OrgMusicSetOctave(U8 *st,I64 *org_octave)
{
  while ('0'<=*st<='9')
    *org_octave=*st++ -'0';
  return st;
}

U8 *OrgMusicSetNoteLen(U8 *st,F64 *org_duration)
{
  Bool cont=TRUE;
  do {
    switch (*st++) {
      case 'w': *org_duration=4.0;  break;
      case 'h': *org_duration=2.0;  break;
      case 'q': *org_duration=1.0;  break;
      case 'e': *org_duration=0.5;  break;
      case 's': *org_duration=0.25; break;
      case 't': *org_duration=2.0* *org_duration/3.0; break;
      case '.': *org_duration=1.5* *org_duration; break;
      default:
	st--;
	cont=FALSE;
    }
  } while (cont);
  return st;
}

U0 OrgLoadSongStr(U8 *st,I64 *org_octave,F64 *org_duration)
{
  OrgNote *tempo,*tempo1;
  I64 note,i=0;
  while (*st) {
    tempo=CAlloc(sizeof(OrgNote));
    while (*st && !('A'<=*st<='G') && *st!='R') {
      if (*st=='M') {
	tempo1=CAlloc(sizeof(OrgNote));
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
	OrgSetWidth(tempo1);
	QueIns(tempo1,oc.root.last);
      }
      while (*st=='(') {
	Bts(&tempo->flags,Of_TIE);
	st++;
      }
      st=OrgMusicSetOctave(st,org_octave);
      st=OrgMusicSetNoteLen(st,org_duration);
    }
    if (!*st) {
      OrgNoteDel(tempo);
      break;
    }
    note=*st++-'A';
    if (note<7) {
      note=music_note_map[note];
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
	*org_octave-=1;
      } else if (note>11) {
	note=0;
	*org_octave+=1;
      }
      tempo->note=note;
      tempo->octave=*org_octave;
    } else {
      tempo->note=NOTE_REST;
      tempo->octave=OCTAVE_REST;
    }
    if (*org_duration<=2*.25/3)
      i=0;
    else if (*org_duration<=.25)
      i=1;
    else if (*org_duration<=2*.5/3)
      i=2;
    else if (*org_duration<=.5)
      i=3;
    else if (*org_duration<=2.0/3)
      i=4;
    else if (*org_duration<=.5*1.5)
      i=5;
    else if (*org_duration<=1.0)
      i=6;
    else if (*org_duration<=1.5)
      i=7;
    else if (*org_duration<=2.0)
      i=8;
    else if (*org_duration<=3.0)
      i=9;
    else if (*org_duration<=4.0)
      i=10;
    else
      i=11;
    tempo->duration=i;
    tempo->type=OT_NOTE;
    OrgSetWidth(tempo);
    QueIns(tempo,oc.cur_note->last);
  }
}

U0 OrgLoadSong(U8 *filename,I64 *org_octave,F64 *org_duration)
{
  U8 *st;
  OrgNote *tempo;
  CLex *lx=LexNew(TextFileRead(filename),0,filename);
  Lex(lx);
  if (FileOcc("incomplete",filename,""))
    oc.incomplete_entry->checked=TRUE;
  else
    oc.incomplete_entry->checked=FALSE;
  while (lx->token) {
    if (lx->token==TK_IDENT)
      if (!StrCmp(lx->ident,"Play")) {
	if (Lex(lx)=='(')
	  if (Lex(lx)==TK_STR) {
	    tempo=oc.root.last;
	    st=LexExtStr(lx);
	    OrgLoadSongStr(st,org_octave,org_duration);
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
      } else if (!StrCmp(lx->ident,"music_tempo")) {
	if (Lex(lx)=='=' && Lex(lx)==TK_F64) {
	  music_tempo=lx->cur_f-0.0005;
	  tempo_state.tempo=Round(TEMPO_RANGE*(music_tempo-0.5)/4.4);
	}
      } else if (!StrCmp(lx->ident,"music_stacatto_factor")) {
	if (Lex(lx)=='=' && Lex(lx)==TK_F64) {
	  music_stacatto_factor=lx->cur_f-0.0005;
	  tempo_state.stacatto=Round(TEMPO_RANGE*(music_stacatto_factor-0.12)/0.88);
	}
      }
    Lex(lx);
  }
  LexDel(lx);
}

U8 *OrgCvtSong()
{
  OrgNote *tempo;
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
	note=music_note_map[*LstSub(note,org_note_lst)-'A'];
      }
      if (Bt(&tempo->flags,Of_TIE))
	*dst++='(';
      if (octave!=last_octave && note!=NOTE_REST) {
	*dst++=octave+'0';
	last_octave=octave;
      }
      if (tempo->duration!=last_duration) {
	src=LstSub(tempo->duration,org_duration_lst);
	*dst++=src[0];
	if (src[1])
	  *dst++=src[1];
	last_duration=tempo->duration;
      }
      if (note!=NOTE_REST) {
	src=LstSub(note,org_note_lst);
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

U8 *OrgSaveSong(U8 *dirname,U8 *full_filename)
{
  CDoc *doc=DocNew;
  Bool has_words;
  OrgNote *tempo,*tempo1;
  F64 measure_len=4,two_measure_left=2*measure_len;
  U8 *ptr,ch;
 
  Free(OrgCvtSong); //set tempo->ascii;

  music_tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
  music_stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

  has_words=FALSE;
  tempo=oc.root.next;
  while (tempo!=&oc.root) {
    if (OrgHasWords(tempo->word)) has_words=TRUE;
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
"  music_tempo=%6.3f;\n"
"  music_stacatto_factor=%6.3f;\n"
"  try {\n"
"  while (!ScanKey) {\n"
"\tPlay(\"",music_tempo+0.0005,music_stacatto_factor+0.0005);

  tempo=oc.root.next;
  tempo1=tempo;
  has_words=FALSE;
  while (tempo!=&oc.root) {
    DocPrint(doc,"%s",tempo->ascii);
    if (OrgHasWords(tempo->word)) has_words=TRUE;
    if (tempo->type==OT_METER) {
      measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
      two_measure_left=0;
    } else
      two_measure_left-=org_durations[tempo->duration];
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
"    CatchAll;\n"
"  Snd(0);\n"
"}\n"
"\n"
"Song;\n");
  DocRecalc(doc);
  if (full_filename) {
    StrCpy(doc->filename.name,full_filename);
    Free(full_filename);
  } else
    SPrint(doc->filename.name,"%s/Temp.CPP.Z",dirname);
  DocWrite(doc,TRUE);
  full_filename=StrNew(doc->filename.name);
  DocDel(doc);
  return full_filename;
}


U0 OrgSetOctave(I64 octave)
{
  I64 i;
  U8 buf[136];
  CMenuEntry *tempse;
  for (i=0;i<=6;i++) {
    SPrint(buf,"Snd/Octave%d",i);
    if (tempse=MenuEntryFind(Fs->cur_menu,buf)) {
      if (i==octave)
	tempse->checked=TRUE;
      else
	tempse->checked=FALSE;
    }
  }
  if (tempse=MenuEntryFind(Fs->cur_menu,"Snd/Reverb"))
    tempse->checked=org_play_mode==1;
}


$SP,"<17>",17$
 




#define ORG_NUM_KEYS	20
class OrgKey
{
  U8 x,w,h,ascii;
};

#define ORG_W_W	16
#define ORG_W_H	36
#define ORG_B_W	8
#define ORG_B_H	20

OrgKey org_kbd[ORG_NUM_KEYS]=
{

{ 2*ORG_W_W-4,ORG_B_W,ORG_B_H,'e' },
{ 3*ORG_W_W-4,ORG_B_W,ORG_B_H,'r' },
{ 4*ORG_W_W-4,ORG_B_W,ORG_B_H,'t' },
{ 6*ORG_W_W-4,ORG_B_W,ORG_B_H,'u' },
{ 7*ORG_W_W-4,ORG_B_W,ORG_B_H,'i' },
{ 9*ORG_W_W-4,ORG_B_W,ORG_B_H,'p' },
{10*ORG_W_W-4,ORG_B_W,ORG_B_H,'[' },
{11*ORG_W_W-4,ORG_B_W,ORG_B_H,']' },

{ 0*ORG_W_W,ORG_W_W,ORG_W_H,'a' },
{ 1*ORG_W_W,ORG_W_W,ORG_W_H,'s' },
{ 2*ORG_W_W,ORG_W_W,ORG_W_H,'d' },
{ 3*ORG_W_W,ORG_W_W,ORG_W_H,'f' },
{ 4*ORG_W_W,ORG_W_W,ORG_W_H,'g' },
{ 5*ORG_W_W,ORG_W_W,ORG_W_H,'h' },
{ 6*ORG_W_W,ORG_W_W,ORG_W_H,'j' },
{ 7*ORG_W_W,ORG_W_W,ORG_W_H,'k' },
{ 8*ORG_W_W,ORG_W_W,ORG_W_H,'l' },
{ 9*ORG_W_W,ORG_W_W,ORG_W_H,';' },
{10*ORG_W_W,ORG_W_W,ORG_W_H,'\'' },
{11*ORG_W_W,ORG_W_W,ORG_W_H,'\n'},

};


U0 OrgDownKey(I64 x,I64 y)
{
  I64 i;
  OrgKey *o;
  y-=FONT_HEIGHT*13;
  if (0<=y<ORG_W_H) {
    x-=16;
    for (i=0;i<ORG_NUM_KEYS;i++) {
      o=&org_kbd[i];
      if (o->x<=x<o->x+o->w && y<o->h) {
	Msg(MSG_KEY_DOWN,o->ascii,0);
	return;
      }
    }
  }
}

U0 OrgUpKey(I64 x,I64 y)
{
  I64 i;
  OrgKey *o;
  y-=FONT_HEIGHT*13;
  if (0<=y<ORG_W_H) {
    x-=16;
    for (i=0;i<ORG_NUM_KEYS;i++) {
      o=&org_kbd[i];
      if (o->x<=x<o->x+o->w && y<o->h) {
	Msg(MSG_KEY_UP,o->ascii,0);
	return;
      }
    }
  }
}

U0 OrgMenu(I64 org_octave)
{
  DocClear;
  "$$BD,WHITE$$$$FD,GREEN$$$$CM,0,12$$\n";
  PutSprite($IB,"<17>",17$);
  NewLine(5);

  DrawDC2;

  "$$FG,LTGREEN$$<SPACE BAR>$$FG$$\tRest\n"
  "$$FG,LTGREEN$$<BACKSPACE>$$FG$$\tDeletes Last Note\n"
  "$$FG,LTGREEN$$Left Mouse$$FG$$\tDrag note or shift word\n"
  "$$FG,LTGREEN$$Right Mouse$$FG$$\tChange duration or set word\n";
  PutSprite($IB,"<6>",6$,"$$SP+LM+LA,\"\",\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_LEFT);\",%d$$");
  "   ";
  if (oc.playing)
    PutSprite($IB,"<9>",9$,"$$SP+LM,\"\",\"x\",%d$$");
  else
    PutSprite($IB,"<8>",8$,"$$SP+LM,\"\",\"x\",%d$$");
  "   ";
  if (oc.record_entry->checked) {
    oc.record_entry->checked=TRUE;
    PutSprite($IB,"<10>",10$,"$$SP+LM,\"\",\"z\",%d$$");
  } else
    PutSprite($IB,"<11>",11$,"$$SP+LM,\"\",\"z\",%d$$");
  "   ";
  PutSprite($IB,"<7>",7$,"$$SP+LM+LA,\"\",\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_RIGHT);\",%d$$");
  "$$CM+LX,0,6$$";
  OrgSetOctave(org_octave);
  WinMgrSync(2);
}


U0 OrgPushMode(I64 org_octave)
{
  Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU-WIF_CTRLS;
  OrgMenu(org_octave);
}

U0 OrgPopMode()
{
  Fs->win_inhibit=0;
  DCFill;
}

#define ORGF_CD		1
#define ORGF_INCOMPLETE	2

U0 MusicOrgan(U8 *dirname=NULL)
{
  Bool is_note,was_playing;
  I64 p1,p2,ch,sc,msg_code=0,col,
      note=NOTE_REST,octave=OCTAVE_REST,org_octave=3,timeout_val,timeout_val2,
      old_doc_flags;
  U8 *filename=NULL,*st,*st2;
  CTask *play_task=Spawn(&OrgPlayTask,NULL,"Organ Play",,Fs);
  OrgNote *tempo;
  F64 f=0,last_f=0,org_duration=1.0,
      d,evt_time=tT,note_down_time=tT;
  CCtrl *c=TempoNew;

  if (DocPut) old_doc_flags=DocPut->flags;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinBorder;
  WordStat;
  Preempt(OFF);

  MkDir("HOME/MusicOrgan");
  MusicSettingsRst;
  tempo_state.tempo=Round(TEMPO_RANGE*(music_tempo-0.5)/4.4);
  tempo_state.stacatto=Round(TEMPO_RANGE*(music_stacatto_factor-0.12)/0.88);

  if (DocPut) DocPut->flags|=DOCF_FORM;

  MemSet(&oc,0,sizeof(OrgCtrl));
  oc.screen_x=0;
  oc.root.next=oc.root.last=&oc.root;
  oc.clipboard.next=oc.clipboard.last=&oc.clipboard;
  oc.cur_note=&oc.root;
  oc.dc2=DCAlias;

  MenuPush(
  "File {"
  "  New(,'.');"
  "  ChgDir(MSG_CMD,ORGF_CD);"
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
  "  MarkIncomplete(MSG_CMD,ORGF_INCOMPLETE);"
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
    dirname=StrNew("HOME/MusicOrgan");
  else
    dirname=StrNew(dirname);
  OrgPushMode(org_octave);
  col=0;
  Fs->draw_it=&DrawIt;

  try {
    do {
      was_playing=FALSE;
start:
      if (ipty-Fs->win_top<18)
	msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|
	    1<<MSG_IP_MOVE|1<<MSG_CMD);
      else
	msg_code=GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|1<<MSG_IP_MOVE|1<<MSG_CMD);
got_msg:
      if (msg_code==MSG_KEY_DOWN && p1==CH_SPACE && !p2) {
      //The Window Mgr sets the Doc cur_entry to a button
      //and generates a <SPACE BAR> when the Doc Buttons are clicked.
      //This is so that kbd and mouse are the same for Doc's.
      //We must now pass the <SPACE BAR> onto the Doc handler.
	PutKey(p1,p2);
	goto start;
      }
      if (msg_code!=MSG_IP_MOVE) {
	DocBottom;
	if (was_playing || DocPut->cur_entry->y>=Fs->win_height-2) {
	  OrgMenu(org_octave);
	  col=0;
	}
      }
      octave=org_octave;
      switch (msg_code) {
	case MSG_CMD:
	  OrgPopMode;
	  switch (p1) {
	    case ORGF_CD:
	      st2=dirname;
	      if (dirname=PopUpPickDir) {
		Free(st2);
		Free(filename);
		filename=NULL;
	      } else
		dirname=st2;
	      break;
	    case ORGF_INCOMPLETE:
	      oc.incomplete_entry->checked=!oc.incomplete_entry->checked;
	      break;
	  }
	  OrgPushMode(org_octave);
	  is_note=FALSE;
	  col=0;
	  break;
	case MSG_KEY_DOWN:
	  evt_time=tT;
	  ch=p1;
	  sc=p2;
	  if ('0'<=ch<='9') {
	    org_octave=ch-'0';
	    OrgMenu(org_octave);
	    col=0;
	  } else {
	    is_note=TRUE;
	    switch (ch) {
	      case 0:
		switch (sc.u8[0]) {
		  case SC_CURSOR_LEFT:
		    if (sc&SCF_CTRL) {
		      while (oc.cur_note->last!=&oc.root) {
			oc.cur_note=oc.cur_note->last;
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,sc&SCF_SHIFT);
		      }
		    } else {
		      if (oc.cur_note->last!=&oc.root) {
			oc.cur_note=oc.cur_note->last;
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,sc&SCF_SHIFT);
		      }
		    }
		    break;
		  case SC_CURSOR_RIGHT:
		    if (sc&SCF_CTRL) {
		      while (oc.cur_note!=&oc.root) {
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,sc&SCF_SHIFT);
			oc.cur_note=oc.cur_note->next;
		      }
		    } else {
		      if (oc.cur_note!=&oc.root) {
			if (oc.cur_note!=&oc.root)
			  LBEqu(&oc.cur_note->flags,Of_SELECTED,sc&SCF_SHIFT);
			oc.cur_note=oc.cur_note->next;
		      }
		    }
		    break;
		  case SC_DELETE:
		    if (sc&SCF_SHIFT)
		      OrgCutToClipboard;
		    else {
		      tempo=oc.cur_note;
		      oc.cur_note=tempo->next;
		      if (tempo!=&oc.root) {
			QueRem(tempo);
			OrgNoteDel(tempo);
		      }
		    }
		    break;
		  case SC_INS:
		    if (sc&SCF_SHIFT)
		      OrgPasteClipboard;
		    else if (sc&SCF_CTRL)
		      OrgCopyToClipboard;
		    break;
		  case SC_F1:
		    OrgPopMode;
		    PopUp("Ed(\"::/TempleOS/Apps/MusicOrgan/Help.TXT.Z\");",Fs);
		    OrgPushMode(org_octave);
		    col=0;
		    is_note=FALSE;
		    break;
		}
		is_note=FALSE;
		break;
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
	      case CH_SPACE:
		note=NOTE_REST;
		octave=OCTAVE_REST;
		break;
	      case ',':
		Free(filename);
		filename=NULL;
		OrgPopMode;
		music_octave=org_octave;
		if (st2=CallExtStr("MakeSong")) {
		  OrgLoadSongStr(st2,&org_octave,&org_duration);
		  Free(st2);
		}
		OrgPushMode(org_octave);
		is_note=FALSE;
		col=0;
		break;
	      case CH_CTRLO:
		OrgPopMode;
		AcctOneTimePopUp(ARf_MUSIC_ORGAN_JUKEBOX,
		"Select a song and preview it.\n"
		"$$FG,GREEN$$<SHIFT-ESC>$$FG$$ to load it into MusicOrgan.\n\n"
		ST_WARN_ST " Graphics and other embelishments\n"
		"will be lost because MusicOrgan can't\n"
		    "parse HolyC programs completely.\n");
		Free(filename);
		filename=NULL;
		JukeBox(dirname,&filename);
		if (filename) {
		  oc.screen_x=0;
		  org_duration=1.0;
		  org_octave=3;
		  OrgSongDel(&oc.root);
		  oc.cur_note=&oc.root;
		  OrgLoadSong(filename,&org_octave,&org_duration);
		  oc.record_entry->checked=FALSE;
		  oc.cur_note=oc.root.next;
		}
		OrgPushMode(org_octave);
		is_note=FALSE;
		col=0;
		break;
	      case CH_CTRLA:
		OrgPopMode;
		filename=OrgSaveSong(dirname,filename);
		OrgPushMode(org_octave);
		is_note=FALSE;
		break;
	      case CH_CTRLC:
		OrgCopyToClipboard;
		is_note=FALSE;
		break;
	      case CH_CTRLV:
		OrgPasteClipboard;
		is_note=FALSE;
		break;
	      case CH_CTRLX:
		OrgCutToClipboard;
		is_note=FALSE;
		break;
	      case '.':
		OrgMenu(org_octave);
		col=0;
		Free(filename);
		filename=NULL;
		org_duration=1.0;
		org_octave=3;
		OrgSongDel(&oc.root);
		oc.cur_note=&oc.root;
		oc.screen_x=0;
		is_note=FALSE;
		break;
	      case '\n':
		if (oc.cur_note!=&oc.root)
		  OrgRightClick(oc.cur_note->x,oc.cur_note->y);
		is_note=FALSE;
		break;
	      case 'x':
		if (was_playing) {
		  is_note=FALSE;
		  break;
		}
		col=0;
		oc.playing=TRUE;
		OrgMenu(org_octave);
		tempo=oc.cur_note;
		while (tempo!=&oc.root) {
		  if (tempo->type!=OT_METER) {
		    timeout_val=sys_jiffies;
		    if (ipty-Fs->win_top<18)
		      msg_code=ScanMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_R_UP|1<<MSG_CMD);
		    else
		      msg_code=ScanMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_CMD);
		    if (msg_code) {
		      f=0;
		      org_play_f=0;
		      is_note=FALSE;
		      oc.playing=FALSE;
		      was_playing=TRUE;
		      if (ipty-Fs->win_top>=18 && msg_code==MSG_IP_L_DOWN)
			goto start;
		      else
			goto got_msg;
		    }
		    oc.cur_note=tempo;
		    oc.screen_x+=tempo->x-0.33*GR_WIDTH;
		    if (OrgHasWords(tempo->word))
		      "%s",tempo->word;
		    note=tempo->note;
		    octave=tempo->octave;
		    if (note!=NOTE_REST)
		      f=Note2Freq(note,octave);
		    else
		      f=-1; //rest
		    if (f>0)
		      org_play_f=f;
		    else
		      org_play_f=0;


		    music_tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
		    music_stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;
		    d=JIFFY_FREQ*org_durations[tempo->duration]/music_tempo;
		    if (Bt(&tempo->flags,Of_TIE)) {
		      timeout_val+=d;
		      timeout_val2=timeout_val;
		    } else {
		      timeout_val+=d*music_stacatto_factor;
		      timeout_val2=timeout_val+
			  d*(1.0-music_stacatto_factor);
		    }
		    SleepUntil(timeout_val);
		    org_play_f=0;
		    SleepUntil(timeout_val2);
		  }
		  tempo=tempo->next;
		}
		oc.cur_note=&oc.root;
		oc.screen_x+=oc.cur_note->x-GR_WIDTH/2;
		oc.playing=FALSE;
		OrgMenu(org_octave);
		col=0;
		f=0;
		org_play_f=0;
		is_note=FALSE;
		break;
	      case CH_BACKSPACE:
		tempo=oc.cur_note->last;
		if (tempo!=&oc.root) {
		  QueRem(tempo);
		  OrgNoteDel(tempo);
		}
		is_note=FALSE;
		if (col) {
		  '' CH_BACKSPACE;
		  col--;
		}
		break;
	      case 'c':
		org_play_mode++;
		if (org_play_mode>=ORGPM_NUM_PLAY_MODES) org_play_mode=0;
		is_note=FALSE;
		OrgMenu(org_octave);
		col=0;
		break;
	      case 'z':
		if (oc.record_entry->checked)
		  oc.record_entry->checked=FALSE;
		else {
		  oc.record_entry->checked=TRUE;
		  org_duration=1.0;
		  org_octave=3;
		  oc.screen_x=0;
		}
		OrgMenu(org_octave);
		col=0;
		is_note=FALSE;
		break;
	      case CH_ESC:
		OrgPopMode;
		filename=OrgSaveSong(dirname,filename);
		OrgPushMode(org_octave);
		break;
	      default:
		is_note=FALSE;
	    }
	    if (is_note) {
	      if (note!=NOTE_REST)
		f=Note2Freq(note,octave);
	      else
		f=-1; //rest
	    }
	  }
	  break;
	case MSG_KEY_UP:
	  evt_time=tT;
	  f=0;
	  break;
	case MSG_IP_MOVE:
	  if (p2>18*FONT_HEIGHT)
	    Fs->win_inhibit=0;
	  else
	    Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU-WIF_CTRLS;
	  break;
	case MSG_IP_L_DOWN:
	  OrgDownKey(p1,p2);
	  OrgLeftClick(p1,p2);
	  break;
	case MSG_IP_L_UP:
	  OrgUpKey(p1,p2);
	  break;
	default:
	  OrgRightClick(p1,p2);
      }
      if (f!=last_f) {
	if (f>0) {
	  st=LstSub(note,org_note_lst);
	  org_play_f=f;
	} else {
	  if (f<0)
	    st="R";
	  else
	    st="";
	  org_play_f=0;
	}
	if (oc.record_entry->checked) {
	  if (last_f) {
	    music_tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
	    music_stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

	    tempo->duration=OrgCvtDuration(music_tempo*(evt_time-note_down_time));
	    OrgSetWidth(tempo);
	    QueIns(tempo,oc.cur_note->last);
	  }
	  if (f) {
	    note_down_time=tT;
	    tempo=CAlloc(sizeof(OrgNote));
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
	  NewLine;
	  col=0;
	}
      }
    } while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    CatchAll;
  OrgPopMode;
  OrgSongDel(&oc.root);
  OrgSongDel(&oc.clipboard);
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
           "     
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
þÿÿÿÿÿÿÿ   ÿÿÿÿ                 
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
þÿÿÿÿÿÿÿ   ÿÿÿÿ           Í      
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
þÿÿÿÿÿÿÿ   ÿÿÿÿ           V      
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ
          òÿÿÿ           E      
ÿÿÿÿýÿÿÿüÿÿÿ    
üÿÿÿ           
          ÿÿÿÿ
   ÿÿÿÿ    ýÿÿÿ           a      
                              
            
            
                       a      
                              
            
            
                       Õ      
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
      	      	      	      	                   
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
              ?      
                           
         
   	              #      
þÿÿÿÖÿÿÿþÿÿÿ&   
   Öÿÿÿ   &              E      
   úÿÿÿ      
   úÿÿÿ
      
          ýÿÿÿ
                       V      
	   ýÿÿÿ      
      
      

            
      
      

                                                  E      
þÿÿÿöÿÿÿ    öÿÿÿ
    öÿÿÿ    òÿÿÿ
    òÿÿÿþÿÿÿòÿÿÿ
    ôÿÿÿÿÿÿÿôÿÿÿ           ò      
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
       	       P      
                              

            
	      	       