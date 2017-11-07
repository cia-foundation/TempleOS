U0 PsmNoteDel(PsmNote *tmpn)
{
  Free(tmpn->word);
  Free(tmpn);
}

PsmNote *PsmNoteCopy(PsmNote *tmpn)
{
  PsmNote *tmpn1=MAllocIdent(tmpn);
  if (tmpn->word)
    tmpn1->word=StrNew(tmpn->word);
  else
    tmpn1->word=NULL;
  return tmpn1;
}

U0 PsmSongDel(PsmNote *head)
{
  PsmNote *tmpn,*tmpn1;
  tmpn=head->next;
  while (tmpn!=head) {
    tmpn1=tmpn->next;
    PsmNoteDel(tmpn);
    tmpn=tmpn1;
  }
  QueInit(head);
}

U0 PsmCutToClip()
{
  PsmNote *tmpn,*tmpn1;
  PsmSongDel(&psm.clip);
  tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    tmpn1=tmpn->next;
    if (tmpn->flags&PSMF_SEL) {
      if (psm.cur_note==tmpn)
	psm.cur_note=tmpn->next;
      QueRem(tmpn);
      tmpn->flags&=~PSMF_SEL;
      QueIns(tmpn,psm.clip.last);
    }
    tmpn=tmpn1;
  }
}

U0 PsmPasteClip()
{
  PsmNote *tmpn,*tmpn1;
  tmpn=psm.clip.next;
  while (tmpn!=&psm.clip) {
    tmpn1=PsmNoteCopy(tmpn);
    QueIns(tmpn1,psm.cur_note->last);
    tmpn=tmpn->next;
  }
}

U0 PsmCopyToClip()
{
  PsmNote *tmpn,*tmpn1;
  PsmSongDel(&psm.clip);
  tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    if (tmpn->flags&PSMF_SEL) {
      tmpn->flags&=~PSMF_SEL;
      tmpn1=PsmNoteCopy(tmpn);
      QueIns(tmpn1,psm.clip.last);
    }
    tmpn=tmpn->next;
  }
}

PsmNote *PsmFindNote(I64 x,I64)
{
  PsmNote *tmpn=psm.head.next;
  PsmRecalcNoteXY;
  x+=PSM_NOTE_SPACING/2;
  while (x>tmpn->next->x && tmpn!=&psm.head)
    tmpn=tmpn->next;
  return tmpn;
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
  PsmNote *tmpn,*tmpn1;
  I64 note,i=0;
  while (*st) {
    tmpn=CAlloc(sizeof(PsmNote));
    while (*st && !('A'<=*st<='G') && *st!='R') {
      if (*st=='M') {
	tmpn1=CAlloc(sizeof(PsmNote));
	tmpn1->type=PSMT_METER;
	st++;
	if ('1'<=*st<='9')
	  tmpn1->meter_top=*st++-'0';
	else
	  tmpn1->meter_top=4;
	if (*st=='/')
	  st++;
	if ('1'<=*st<='9')
	  tmpn1->meter_bottom=*st++-'0';
	else
	  tmpn1->meter_bottom=4;
	PsmSetWidth(tmpn1);
	QueIns(tmpn1,psm.head.last);
      }
      while (*st=='(') {
	Bts(&tmpn->flags,PSMf_TIE);
	st++;
      }
      st=PsmMusicSetOctave(st,psm_octave);
      st=PsmMusicSetNoteLen(st,psm_duration);
    }
    if (!*st) {
      PsmNoteDel(tmpn);
      break;
    }
    note=*st++-'A';
    if (note<7) {
      note=music.note_map[note];
      if (*st=='b') {
	Bts(&tmpn->flags,PSMf_FLAT);
	note--;
	st++;
	if (note<0) //Ab
	  note=11;
	else if (note==2) //Cb
	  *psm_octave-=1;
      } else if (*st=='#') {
	Bts(&tmpn->flags,PSMf_SHARP);
	note++;
	st++;
	if (note>11) //G#
	  note=0;
	else if (note==3) //B#
	  *psm_octave+=1;
      }
      tmpn->ona=Note2Ona(note,*psm_octave);
    } else
      tmpn->ona=0;
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
    tmpn->duration=i;
    tmpn->type=PSMT_NOTE;
    PsmSetWidth(tmpn);
    QueIns(tmpn,psm.cur_note->last);
  }
}

U0 PsmLoadSong(U8 *filename,I64 *psm_octave,F64 *psm_duration)
{
  U8 *st;
  PsmNote *tmpn;
  CCmpCtrl *cc=CmpCtrlNew(MStrPrint("#include \"%s\"",filename));
  if (FileOcc("incomplete",filename,""))
    psm.incomplete_entry->checked=TRUE;
  else
    psm.incomplete_entry->checked=FALSE;
  while (Lex(cc)) {
    if (cc->token==TK_IDENT)
      if (!StrCmp(cc->cur_str,"Play")) {
	if (Lex(cc)=='(')
	  if (Lex(cc)==TK_STR) {
	    tmpn=psm.head.last;
	    st=LexExtStr(cc);
	    PsmLoadSongStr(st,psm_octave,psm_duration);
	    if (cc->token==',') {
	      if (Lex(cc)==TK_STR) {
		st=LexExtStr(cc);
		do {
		  do tmpn=tmpn->next;
		  while (tmpn!=&psm.head && tmpn->type==PSMT_METER);
		  if (tmpn!=&psm.head)
		    tmpn->word=StrNew(st);
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
  PsmNote *tmpn;
  U8 *st,*src,*dst;
  I64 i,ona,note,octave,last_octave,last_duration;

  i=0;
  tmpn=psm.head.next;
  last_octave=I64_MIN;
  last_duration=-1;
  while (tmpn!=&psm.head) {
    dst=&tmpn->ascii;
    if (tmpn->type==PSMT_METER) {
      *dst++='M';
      *dst++=tmpn->meter_top+'0';
      *dst++='/';
      *dst++=tmpn->meter_bottom+'0';
    } else {
      if (tmpn->ona) {
	ona=tmpn->ona;
	if (Bt(&tmpn->flags,PSMf_SHARP))
	  ona--;
	if (Bt(&tmpn->flags,PSMf_FLAT))
	  ona++;
	octave=Ona2Octave(ona);
	note  =Ona2Note  (ona);
	note=music.note_map[*LstSub(note,psm_note_lst)-'A'];
      }
      if (Bt(&tmpn->flags,PSMf_TIE))
	*dst++='(';
      if (octave!=last_octave && tmpn->ona) {
	*dst++=octave+'0';
	last_octave=octave;
      }
      if (tmpn->duration!=last_duration) {
	src=LstSub(tmpn->duration,psm_duration_lst);
	*dst++=src[0];
	if (src[1])
	  *dst++=src[1];
	last_duration=tmpn->duration;
      }
      if (tmpn->ona) {
	src=LstSub(note,psm_note_lst);
	*dst++=src[0];
	if (src[1])
	  *dst++=src[1];
	else if (Bt(&tmpn->flags,PSMf_FLAT))
	  *dst++='b';
	else if (Bt(&tmpn->flags,PSMf_SHARP))
	  *dst++='#';
      } else
	*dst++='R';
    }
    *dst++=0;
    i+=StrLen(tmpn->ascii);
    tmpn=tmpn->next;
  }

  st=MAlloc(i+1);
  dst=st;
  tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    StrCpy(dst,tmpn->ascii);
    dst+=StrLen(tmpn->ascii);
    tmpn=tmpn->next;
  }
  *dst++=0;
  return st;
}

U8 *PsmSaveSong(U8 *dirname,U8 *full_filename)
{
  CDoc *doc=DocNew(full_filename);
  Bool has_words;
  PsmNote *tmpn,*tmpn1;
  F64 measure_len=4,two_measure_left=2*measure_len;
  I64 ch;
  U8 *ptr;

  Free(PsmCvtSong); //set tmpn->ascii;

  music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
  music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

  has_words=FALSE;
  tmpn=psm.head.next;
  while (tmpn!=&psm.head) {
    if (PsmHasWords(tmpn->word)) has_words=TRUE;
    tmpn=tmpn->next;
  }
  if (psm.incomplete_entry->checked)
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

  tmpn=psm.head.next;
  tmpn1=tmpn;
  has_words=FALSE;
  while (tmpn!=&psm.head) {
    DocPrint(doc,"%s",tmpn->ascii);
    if (PsmHasWords(tmpn->word)) has_words=TRUE;
    if (tmpn->type==PSMT_METER) {
      measure_len=tmpn->meter_top*4.0/tmpn->meter_bottom;
      two_measure_left=0;
    } else
      two_measure_left-=psm_durations[tmpn->duration];
    tmpn=tmpn->next;
    if (two_measure_left<0.001 && tmpn!=&psm.head) {
      if (has_words) {
	DocPrint(doc,"\",\n\t\t\"");
	while (tmpn1!=tmpn) {
	  if (tmpn1->type!=PSMT_METER) {
	    if (ptr=tmpn1->word) {
	      while (ch=*ptr) {
		if (ch==CH_SPACE)
		  *ptr=CH_SHIFT_SPACE;
		ptr++;
	      }
	      DocPrint(doc,"%Q\\0",tmpn1->word);
	    } else
	      DocPrint(doc,"%c\\0",CH_SHIFT_SPACE);
	  }
	  tmpn1=tmpn1->next;
	}
      }
      DocPrint(doc,"\");\n"
	    "\tPlay(\"");
      two_measure_left=2*measure_len;
      tmpn1=tmpn;
      has_words=FALSE;
    }
  }
  if (has_words) {
    DocPrint(doc,"\",\n\t\t\"");
    while (tmpn1!=tmpn) {
      if (tmpn1->type!=PSMT_METER) {
	if (ptr=tmpn1->word) {
	  while (ch=*ptr) {
	    if (ch==CH_SPACE)
	      *ptr=CH_SHIFT_SPACE;
	    ptr++;
	  }
	  DocPrint(doc,"%Q\\0",tmpn1->word);
	} else
	  DocPrint(doc,"%c\\0",CH_SHIFT_SPACE);
      }
      tmpn1=tmpn1->next;
    }
  }
  DocPrint(doc,"\");\n"
	"    }\n"
	"  } catch\n"
	"    PutExcept;\n"
	"  Snd;\n"
	"}\n"
	"\n"
	"Song;\n");
  DocRecalc(doc);
  if (full_filename)
    Free(full_filename);
  else
    StrPrint(doc->filename.name,"%s/Tmp.HC.Z",dirname);
  DocWrite(doc,TRUE);
  full_filename=StrNew(doc->filename.name);
  DocDel(doc);
  return full_filename;
}
