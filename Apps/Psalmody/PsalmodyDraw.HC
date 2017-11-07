
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
#define PSM_DURATIONS_NUM	12

F64 psm_durations[PSM_DURATIONS_NUM+1]={
2*.25/3,.25,2*.5/3,.5,2.0/3.0,0.5*1.5,1.0,1.5,2.0,3.0,4.0,6.0,1000000.0};
U8 *psm_duration_lst="st\0s\0et\0e\0qt\0e.\0q\0q.\0h\0h.\0w\0w.\0";
U8 *psm_duration_imgs[PSM_DURATIONS_NUM]={
	$IB,"<1>",BI=1$,$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$,$IB,"<3>",BI=3$,$IB,"<4>",BI=4$,$IB,"<4>",BI=4$,$IB,"<5>",BI=5$,$IB,"<5>",BI=5$};
Bool psm_triplet_durations[PSM_DURATIONS_NUM]={TRUE,FALSE,TRUE,
	FALSE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};

Bool PsmBlink()
{
  if (Blink || psm.playing)
    return TRUE;
  else
    return FALSE;
}

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

U0 PsmMarkSel(I64 x1,I64 x2,Bool sel)
{
  PsmNote *tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    if (sel) {
      if (x1<=tmpn->x<=x2)
	tmpn->flags|=PSMF_SEL;
    } else
      tmpn->flags&=~PSMF_SEL;
    tmpn=tmpn->next;
  }
}

U0 PsmSetWidth(PsmNote *tmpn)
{
  if (tmpn->type==PSMT_METER)
    tmpn->width=12;
  else {
    tmpn->width=PSM_NOTE_SPACING;
    if (PsmIsDotted(tmpn->duration))
      tmpn->width+=PSM_NOTE_SPACING/2;
    if (Bt(&tmpn->flags,PSMf_SHARP) || Bt(&tmpn->flags,PSMf_FLAT))
      tmpn->width+=PSM_NOTE_SPACING;
  }
}

U0 PsmSetOctave(I64 octave)
{
  I64 i;
  U8 buf[STR_LEN];
  CMenuEntry *tmpse;
  for (i=1;i<=7;i++) {
    StrPrint(buf,"Snd/Octave%d",i);
    if (tmpse=MenuEntryFind(Fs->cur_menu,buf)) {
      if (i==octave)
	tmpse->checked=TRUE;
      else
	tmpse->checked=FALSE;
    }
  }
}

U0 DrawNote(CDC *dc,I64 x,I64 y,I64 duration)
{
  if (0<=duration<=PSM_DURATIONS_NUM) {
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

U0 PsmRecalcNoteXY()
{
  F64 measure_len=4,measure_left=measure_len;
  PsmNote *tmpn=psm.head.next;
  I64 x=8-psm.scrn_x,ona,note,octave;
  while (TRUE) {
    tmpn->x=x;
    tmpn->y=50;
    if (tmpn==&psm.head)
      break;
    else {
      if (tmpn->type!=PSMT_METER) {
	if (tmpn->ona) {
	  ona=tmpn->ona;
	  if (Bt(&tmpn->flags,PSMf_SHARP))
	    ona--;
	  if (Bt(&tmpn->flags,PSMf_FLAT))
	    ona++;
	  octave=Ona2Octave(ona);
	  note	=Ona2Note  (ona);
	  note=music.note_map[*LstSub(note,psm_note_lst)-'A'];
	  if (note<3)
	    octave++;
	  tmpn->y=(15+(psm_note_map[note]-7*(octave-4)))*4;
	}
      }
      x+=tmpn->width;
      if (tmpn->type==PSMT_METER) {
	measure_len=tmpn->meter_top*4.0/tmpn->meter_bottom;
	measure_left=0;
      } else
	measure_left-=psm_durations[tmpn->duration];
      if (measure_left<0.001) {
	x+=PSM_NOTE_SPACING;
	measure_left=measure_len;
      }
    }
    tmpn=tmpn->next;
  }
}

U0 DrawIt(CTask *task,CDC *dc)
{
  PsmNote *tmpn;
  I64 i,x,y,
	w=task->pix_width;
  F64 measure_len=4,measure_left=measure_len;

  dc->color=BLACK;
  for (i=1;i<6;i++)
    GrLine(dc,0,i*8,w,i*8);
  for (i=7;i<12;i++)
    GrLine(dc,0,i*8,w,i*8);

  PsmRecalcNoteXY;
  if (psm.cur_note->x<64) {
    psm.scrn_x-=128;
    PsmRecalcNoteXY;
  }
  if (psm.cur_note->x>=GR_WIDTH-64) {
    psm.scrn_x+=128;
    PsmRecalcNoteXY;
  }

  tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    x=tmpn->x;
    y=tmpn->y;
    if (measure_left<0.001) {
      dc->color=BLACK;
      GrLine(dc,x-PSM_NOTE_SPACING,8,x-PSM_NOTE_SPACING,11*8);
      measure_left=measure_len;
    }
    if (tmpn->type==PSMT_METER) {
      if (tmpn==psm.cur_note && PsmBlink)
	dc->color=BROWN;
      else
	dc->color=BLACK;
      DrawTimeSignature(dc,x,5*8,tmpn->meter_top,tmpn->meter_bottom);
      measure_len=tmpn->meter_top*4.0/tmpn->meter_bottom;
      measure_left=0;
    } else {
      if (tmpn==psm.cur_note && PsmBlink) {
	dc->color=BROWN;
	GrPrint(dc,x+8,y,"%s",tmpn->word);
      } else if (tmpn->flags&PSMF_SEL)
	dc->color=RED;
      else {
	if (tmpn->ona)
	  dc->color=BLACK;
	else
	  dc->color=LTGRAY;
	if (PsmHasWords(tmpn->word))
	  dc->color=GREEN;
      }
      DrawNote(dc,x,y,tmpn->duration);
      if (PsmIsDotted(tmpn->duration))
	x+=PSM_NOTE_SPACING/2;
      if (Bt(&tmpn->flags,PSMf_SHARP))
	Sprite3(dc,x,y,0,$IB,"<13>",BI=13$);
      else if (Bt(&tmpn->flags,PSMf_FLAT))
	Sprite3(dc,x,y,0,$IB,"<14>",BI=14$);
      measure_left-=psm_durations[tmpn->duration];
    }
    tmpn=tmpn->next;
  }

  if (psm.cur_note==&psm.head && PsmBlink)
    dc->color=BROWN;
  else
    dc->color=BLACK;
  Sprite3(dc,psm.head.x,50,0,$IB,"<12>",BI=12$);
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
    for (i=0;i<PSM_DURATIONS_NUM;i++) {
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
  for (i=0;i<PSM_DURATIONS_NUM;i++) {
    PsmSetPickNoteBoxX(i,&x);
    if (PsmIsDotted(i))
      psm.dc2->color=RED;
    else if (psm_triplet_durations[i])
      psm.dc2->color=LTRED;
    else
      psm.dc2->color=BLACK;
    DrawNote(psm.dc2,x,PSM_NOTE_BOX_Y,i);
  }
}

#define PSM_TOOLS_X	450
#define PSM_TOOLS_Y	13*FONT_HEIGHT

U0 DrawPickTools()
{
  if (psm.tool==PSMTT_BOX_TOOL)
    psm.dc2->color=ROPF_DITHER+WHITE<<16+RED;
  else
    psm.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
  GrBorder(psm.dc2,PSM_TOOLS_X,PSM_TOOLS_Y,PSM_TOOLS_X+10,PSM_TOOLS_Y+10);

  if (psm.tool==PSMTT_PTR_TOOL)
    psm.dc2->color=RED;
  else
    psm.dc2->color=BLACK;
  (*gr.fp_draw_ms)(psm.dc2,PSM_TOOLS_X+15,PSM_TOOLS_Y);
  psm.dc2->color=BLACK;
}

Bool PsmGetPickToolBox(I64 xx,I64 yy)
{
  if (PSM_TOOLS_X<=xx<PSM_TOOLS_X+27 && PSM_TOOLS_Y<=yy<PSM_TOOLS_Y+15) {
    PsmMarkSel(0,0,FALSE);
    if (xx<PSM_TOOLS_X+13)
      psm.tool=PSMTT_BOX_TOOL;
    else
      psm.tool=PSMTT_PTR_TOOL;
    return TRUE;
  } else
    return FALSE;
}

#define PSM_METERS_NUM	7
I64 meter_tops[PSM_METERS_NUM]	={2,3,4,5,6,7,9},
   meter_bottoms[PSM_METERS_NUM]={4,4,4,4,8,8,8};

#define PSM_METER_X	485
#define PSM_METER_Y	13*FONT_HEIGHT
#define PSM_METER_W	12
Bool PsmGetPickMeterBox(I64 xx,I64 yy,I64 *top,I64 *bottom)
{
  I64 i;
  if (PSM_METER_X<=xx<PSM_METER_X+PSM_METER_W*PSM_METERS_NUM &&
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
  psm.dc2->color=BLACK;
  for (i=0;i<PSM_METERS_NUM;i++)
    DrawTimeSignature(psm.dc2,PSM_METER_X+i*PSM_METER_W,PSM_METER_Y,
	  meter_tops[i],meter_bottoms[i]);
}

U0 DrawDC2()
{
  DCFill;
  DrawPickNoteBox;
  DrawPickMeterBox;
  DrawPickTools;
}


$SP,"<17>",BI=17$
 




U0 PsmMenu(I64 psm_octave)
{
  DocClear;
  "$$BG,WHITE$$$$FD,GREEN$$$$CM,0,12$$\n";
  Sprite($IB,"<17>",BI=17$);
  '\n\n\n\n\n';

  DrawDC2;

  "$$LTGREEN$$<SPACE>$$FG$$\t\tRest\n"
	"$$LTGREEN$$<BACKSPACE>$$FG$$\tDeletes Last Note\n"
	"$$LTGREEN$$Left Mouse$$FG$$\tDrag note or shift word\n"
	"$$LTGREEN$$Right Mouse$$FG$$\tChange duration or set word\n";
  Sprite($IB,"<6>",BI=6$,"$$SP+LIS,\"\","
	"LM=\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_LEFT);\",BI=%d$$");
  "   ";
  if (psm.playing)
    Sprite($IB,"<9>",BI=9$,"$$SP,\"\",LM=\"x\",BI=%d$$");
  else
    Sprite($IB,"<8>",BI=8$,"$$SP,\"\",LM=\"x\",BI=%d$$");
  "   ";
  if (psm.record_entry->checked) {
    psm.record_entry->checked=TRUE;
    Sprite($IB,"<10>",BI=10$,"$$SP,\"\",LM=\"z\",BI=%d$$");
  } else
    Sprite($IB,"<11>",BI=11$,"$$SP,\"\",LM=\"z\",BI=%d$$");
  "   ";
  Sprite($IB,"<7>",BI=7$,"$$SP+LIS,\"\","
	"LM=\"Msg(MSG_KEY_DOWN,0,SCF_CTRL|SC_CURSOR_RIGHT);\",BI=%d$$");
  "$$CM+LX,0,6$$";
  PsmSetOctave(psm_octave);
  Refresh(2);
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