U0 PsmNoteDel(PsmNote *tempo)
{
  Free(tempo->word);
  Free(tempo);
}

PsmNote *PsmNoteCopy(PsmNote *tempo)
{
  PsmNote *tempo1=MAllocIdent(tempo);
  if (tempo->word)
    tempo1->word=StrNew(tempo->word);
  else
    tempo1->word=NULL;
  return tempo1;
}

U0 PsmSongDel(PsmNote *head)
{
  PsmNote *tempo,*tempo1;
  tempo=head->next;
  while (tempo!=head) {
    tempo1=tempo->next;
    PsmNoteDel(tempo);
    tempo=tempo1;
  }
  QueInit(head);
}

U0 PsmCutToClipboard()
{
  PsmNote *tempo,*tempo1;
  PsmSongDel(&oc.clipboard);
  tempo=oc.head.next;
  while (tempo!=&oc.head) {
    tempo1=tempo->next;
    if (tempo->flags&PSMF_SEL) {
      if (oc.cur_note==tempo)
	oc.cur_note=tempo->next;
      QueRem(tempo);
      tempo->flags&=~PSMF_SEL;
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
  tempo=oc.head.next;
  while (tempo!=&oc.head) {
    if (tempo->flags&PSMF_SEL) {
      tempo->flags&=~PSMF_SEL;
      tempo1=PsmNoteCopy(tempo);
      QueIns(tempo1,oc.clipboard.last);
    }
    tempo=tempo->next;
  }
}

PsmNote *PsmFindNote(I64 x,I64)
{
  PsmNote *tempo=oc.head.next;
  PsmRecalcNoteXY;
  x+=PSM_NOTE_SPACING/2;
  while (x>tempo->next->x && tempo!=&oc.head)
    tempo=tempo->next;
  return tempo;
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
	tempo1->type=PSMT_METER;
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
	QueIns(tempo1,oc.head.last);
      }
      while (*st=='(') {
	Bts(&tempo->flags,PSMf_TIE);
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
	Bts(&tempo->flags,PSMf_FLAT);
	note--;
	st++;
      } else if (*st=='#') {
	Bts(&tempo->flags,PSMf_SHARP);
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
    tempo->type=PSMT_NOTE;
    PsmSetWidth(tempo);
    QueIns(tempo,oc.cur_note->last);
  }
}

U0 PsmLoadSong(U8 *filename,I64 *psm_octave,F64 *psm_duration)
{
  U8 *st;
  PsmNote *tempo;
  CCmpCtrl *cc=CmpCtrlNew(MStrPrint("#include \"%s\"",filename));
  if (FileOcc("incomplete",filename,""))
    oc.incomplete_entry->checked=TRUE;
  else
    oc.incomplete_entry->checked=FALSE;
  while (Lex(cc)) {
    if (cc->token==TK_IDENT)
      if (!StrCmp(cc->cur_str,"Play")) {
	if (Lex(cc)=='(')
	  if (Lex(cc)==TK_STR) {
	    tempo=oc.head.last;
	    st=LexExtStr(cc);
	    PsmLoadSongStr(st,psm_octave,psm_duration);
	    if (cc->token==',') {
	      if (Lex(cc)==TK_STR) {
		st=LexExtStr(cc);
		do {
		  do tempo=tempo->next;
		  while (tempo!=&oc.head && tempo->type==PSMT_METER);
		  if (tempo!=&oc.head)
		    tempo->word=StrNew(st);
		  st+=StrLen(st)+1;
		} while (*st);
	      }
	    }
	  }
      } else if (!StrCmp(cc->cur_str,"music") &&
	    Lex(cc)=='.' && Lex(cc)==TK_IDENT) {
	if (!StrCmp(cc->cur_str,"tempo")) {
	  if (Lex(cc)=='=' && Lex(cc)==TK_F64) {
	    music.tempo=cc->cur_f64-0.0005;
	    tempo_state.tempo=Round(TEMPO_RANGE*(music.tempo-0.5)/4.4);
	  }
	} else if (!StrCmp(cc->cur_str,"stacatto_factor")) {
	  if (Lex(cc)=='=' && Lex(cc)==TK_F64) {
	    music.stacatto_factor=cc->cur_f64-0.0005;
	    tempo_state.stacatto=
		  Round(TEMPO_RANGE*(music.stacatto_factor-0.12)/0.88);
	  }
	}
      }
  }
  CmpCtrlDel(cc);
}

U8 *PsmCvtSong()
{
  PsmNote *tempo;
  U8 *st,*src,*dst;
  I64 i,note,octave,last_octave,last_duration;

  i=0;
  tempo=oc.head.next;
  last_octave=OCTAVE_REST;
  last_duration=-1;
  while (tempo!=&oc.head) {
    dst=&tempo->ascii;
    if (tempo->type==PSMT_METER) {
      *dst++='M';
      *dst++=tempo->meter_top+'0';
      *dst++='/';
      *dst++=tempo->meter_bottom+'0';
    } else {
      octave=tempo->octave;
      note=tempo->note;
      if (note!=NOTE_REST) {
	if (Bt(&tempo->flags,PSMf_SHARP))
	  note--;
	if (Bt(&tempo->flags,PSMf_FLAT))
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
      if (Bt(&tempo->flags,PSMf_TIE))
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
	else if (Bt(&tempo->flags,PSMf_FLAT))
	  *dst++='b';
	else if (Bt(&tempo->flags,PSMf_SHARP))
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
  tempo=oc.head.next;
  while (tempo!=&oc.head) {
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
  tempo=oc.head.next;
  while (tempo!=&oc.head) {
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
	"    while (!ScanKey) {\n"
	"\tPlay(\"",music.tempo+0.0005,music.stacatto_factor+0.0005);

  tempo=oc.head.next;
  tempo1=tempo;
  has_words=FALSE;
  while (tempo!=&oc.head) {
    DocPrint(doc,"%s",tempo->ascii);
    if (PsmHasWords(tempo->word)) has_words=TRUE;
    if (tempo->type==PSMT_METER) {
      measure_len=tempo->meter_top*4.0/tempo->meter_bottom;
      two_measure_left=0;
    } else
      two_measure_left-=psm_durations[tempo->duration];
    tempo=tempo->next;
    if (two_measure_left<0.001 && tempo!=&oc.head) {
      if (has_words) {
	DocPrint(doc,"\",\n\t\t\"");
	while (tempo1!=tempo) {
	  if (tempo1->type!=PSMT_METER) {
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
      if (tempo1->type!=PSMT_METER) {
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
	"    }\n"
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
    StrPrint(doc->filename.name,"%s/Temp.HC.Z",dirname);
  DocWrite(doc,TRUE);
  full_filename=StrNew(doc->filename.name);
  DocDel(doc);
  return full_filename;
}
