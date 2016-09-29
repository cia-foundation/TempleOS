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
  PsmNote *tmpn,*tmpn1;
  I64 i,old_doc_flags;
  if (DocPut) old_doc_flags=DocPut->flags;
  psm.cur_note=tmpn=PsmFindNote(x,y);
  if (tmpn!=&psm.head) {
    Fs->win_inhibit=WIG_USER_TASK_DFT;
    i=PopUpDuration;
    if (0<=i<PSM_NUM_DURATIONS) {
      if (tmpn->type==PSMT_NOTE)
	tmpn->duration=i;
    } else {
      switch (i) {
	case PSMR_REST:
	  if (tmpn->type==PSMT_NOTE) {
	    tmpn->octave=OCTAVE_REST;
	    tmpn->note=NOTE_REST;
	  }
	  break;
	case PSMR_SHARP:
	  if (tmpn->type==PSMT_NOTE && tmpn->octave!=OCTAVE_REST) {
	    if (Btr(&tmpn->flags,PSMf_FLAT))
	      tmpn->note++;
	    if (Btc(&tmpn->flags,PSMf_SHARP))
	      tmpn->note--;
	    else
	      tmpn->note++;
	    if (tmpn->note<0) {
	      tmpn->note=11;
	      tmpn->octave--;
	    } else if (tmpn->note>11) {
	      tmpn->note=0;
	      tmpn->octave++;
	    }
	  }
	  break;
	case PSMR_FLAT:
	  if (tmpn->type==PSMT_NOTE && tmpn->octave!=OCTAVE_REST) {
	    if (Btr(&tmpn->flags,PSMf_SHARP))
	      tmpn->note--;
	    if (Btc(&tmpn->flags,PSMf_FLAT))
	      tmpn->note++;
	    else
	      tmpn->note--;
	    if (tmpn->note<0) {
	      tmpn->note=11;
	      tmpn->octave--;
	    } else if (tmpn->note>11) {
	      tmpn->note=0;
	      tmpn->octave++;
	    }
	  }
	  break;
	case PSMR_TIE:
	  if (tmpn->type==PSMT_NOTE)
	    Btc(&tmpn->flags,PSMf_TIE);
	  break;
	case PSMR_SET_WORD:
	  if (tmpn->type==PSMT_NOTE) {
	    if (DocPut) DocPut->flags&=~DOCF_FORM;
	    if (PsmHasWords(tmpn->word))
	      st2=MStrPrint("\nWord(\"%Q\"):",tmpn->word);
	    else
	      st2=MStrPrint("\nWord(\"\"):");
	    DocBottom;
	    st=GetStr(st2);
	    Free(st2);
	    Free(tmpn->word);
	    if (*st) {
	      tmpn->word=MStrPrint("%q",st);
	      Free(st);
	    } else
	      tmpn->word=StrNew(" ");
	    if (DocPut) DocPut->flags=DocPut->flags&
		    ~DOCF_FORM|old_doc_flags&DOCF_FORM;
	  }
	  break;
	case PSMR_INS_NOTE:
	  tmpn1=PsmNoteCopy(tmpn);
	  QueIns(tmpn1,tmpn);
	  break;
	case PSMR_DELETE_NOTE:
	  psm.cur_note=tmpn->next;
	  QueRem(tmpn);
	  PsmNoteDel(tmpn);
	  break;
      }
    }
    PsmSetWidth(psm.cur_note);
    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	  -WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
  }
}

U0 PsmLeftClickPickNoteBox(I64 duration)
{
  I64 o,n,msg_code,a1,a2;
  PsmNote *tmpn,*tmpn1;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawNote(psm.dc2,a1,a2,duration);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (a2<13*FONT_HEIGHT) {
    if (a1>psm.head.last->x)
      tmpn1=psm.head.last;
    else if (a1<psm.head.next->x)
      tmpn1=&psm.head;
    else
      tmpn1=PsmFindNote(a1-PSM_NOTE_SPACING/2,a2);
    tmpn=CAlloc(sizeof(PsmNote));
    tmpn->type=PSMT_NOTE;
    a2=a2/4-15;
    n=-a2%7;
    o=2+a2/-7;
    n=-n;
    if (n<0) {
      n+=7;
      o++;
    }
    n=psm_note_inverse_map[n];
    tmpn->note=n;
    tmpn->octave=o;
    tmpn->duration=duration;
    PsmSetWidth(tmpn);
    QueIns(tmpn,tmpn1);
    psm.cur_note=tmpn->next;
  }
  DrawDC2;
}

U0 PsmLeftClickPickMeterBox(I64 top,I64 bottom)
{
  I64 msg_code,a1,a2;
  PsmNote *tmpn,*tmpn1;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    if (msg_code==MSG_IP_MOVE) {
      DrawDC2;
      DrawTimeSignature(psm.dc2,a1,a2,top,bottom);
    }
  } while (msg_code!=MSG_IP_L_UP);
  if (a2<13*FONT_HEIGHT) {
    if (a1>=psm.head.x)
      tmpn1=psm.head.last;
    else if (a1<psm.head.next->x)
      tmpn1=&psm.head;
    else
      tmpn1=PsmFindNote(a1-PSM_NOTE_SPACING/2,a2);
    tmpn=CAlloc(sizeof(PsmNote));
    tmpn->type=PSMT_METER;
    tmpn->meter_top=top;
    tmpn->meter_bottom=bottom;
    PsmSetWidth(tmpn);
    QueIns(tmpn,tmpn1);
    psm.cur_note=tmpn->next;
  }
  DrawDC2;
}

U0 PsmLeftClickStaffPtr(I64 x,I64 y)
{
  PsmNote *tmpn,*tmpn1;
  I64 o,n,msg_code,a1,a2,n_original,o_original;
  psm.cur_note=tmpn=PsmFindNote(x,y);
  if (tmpn!=&psm.head) {
    if (tmpn->type==PSMT_NOTE) {
      n_original=tmpn->note;
      o_original=tmpn->octave;
      do {
	msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
	if (msg_code==MSG_IP_L_UP) {
	  tmpn1=PsmFindNote(a1,a2);
	  if (tmpn1==&psm.head || tmpn1==tmpn)
	    goto move_note;
	  else {
	    Free(tmpn1->word);
	    tmpn1->word=tmpn->word;
	    tmpn->word=NULL;
	    tmpn->note=n_original;
	    tmpn->octave=o_original;
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
	  tmpn->note=n;
	  tmpn->octave=o;
	}
      } while (msg_code!=MSG_IP_L_UP);
      PsmSetWidth(tmpn);
    }
  }
}

U0 PsmLeftClickStaffBox(I64 x,I64 y)
{
  I64 msg_code,a1,a2;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP|1<<MSG_IP_MOVE);
    DrawDC2;
    psm.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrBorder(psm.dc2,x,y,a1,a2);
    if (msg_code==MSG_IP_L_UP) {
      if (x>a1) SwapI64(&x,&a1);
      PsmMarkSel(x,a1,TRUE);
    }
  } while (msg_code!=MSG_IP_L_UP);
  DrawDC2;
}

U0 PsmLeftClick(I64 x,I64 y)
{
  I64 duration,top,bottom;
  if (y<13*FONT_HEIGHT) {
    if (psm.tool==PSMTT_PTR_TOOL)
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

#define PSM_NUM_KEYS	20
class PsmKey
{
  U8 x,w,h,ascii;
};

#define PSM_W_W	16
#define PSM_W_H	36
#define PSM_B_W	8
#define PSM_B_H	20

PsmKey psm_kbd[PSM_NUM_KEYS]={
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

U0 PsmPushMode(I64 psm_octave)
{
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
  PsmMenu(psm_octave);
}

U0 PsmPopMode()
{
  Fs->win_inhibit=WIG_USER_TASK_DFT;
  DCFill;
}

#define PSMF_CD		1
#define PSMF_INCOMPLETE	2

U0 Psalmody(U8 *dirname="~/Psalmody")
{
  Bool was_playing;
  I64 a1,a2,msg_code=0,col,
	note=NOTE_REST,octave=OCTAVE_REST,psm_octave=3,timeout_val,timeout_val2,
	old_doc_flags;
  U8 *filename=NULL,*st,*st2;
  PsmNote *tmpn;
  F64 f=0,last_f=0,psm_duration=1.0,
	d,evt_time=tS,note_down_time=tS;
  CCtrl *c=TempoNew;

  if (DocPut) old_doc_flags=DocPut->flags;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$

  MusicSettingsRst;
  tempo_state.tempo=Round(TEMPO_RANGE*(music.tempo-0.5)/4.4);
  tempo_state.stacatto=Round(TEMPO_RANGE*(music.stacatto_factor-0.12)/0.88);

  if (DocPut) DocPut->flags|=DOCF_FORM;

  MemSet(&psm,0,sizeof(PsmCtrl));
  psm.scrn_x=0;
  psm.head.next=psm.head.last=&psm.head;
  psm.clipboard.next=psm.clipboard.last=&psm.clipboard;
  psm.cur_note=&psm.head;
  psm.dc2=DCAlias;

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
  psm.incomplete_entry=MenuEntryFind(Fs->cur_menu,"Song/MarkIncomplete");
  psm.record_entry=MenuEntryFind(Fs->cur_menu,"Song/Record");

  AutoComplete;
  WinBorder;
  WinMax;

  dirname=StrNew(dirname);
  PsmPushMode(psm_octave);
  col=0;
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      was_playing=FALSE;
mo_start:
      if (ip.pos_text.y-Fs->win_top<18)
	msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|
	      1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|
	      1<<MSG_IP_MOVE|1<<MSG_CMD);
      else
	msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|
	      1<<MSG_IP_MOVE|1<<MSG_CMD);
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
	      psm.incomplete_entry->checked=!psm.incomplete_entry->checked;
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
	      start:
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
	      end:
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
		      while (psm.cur_note->last!=&psm.head) {
			psm.cur_note=psm.cur_note->last;
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,a2&SCF_SHIFT);
		      }
		    } else {
		      if (psm.cur_note->last!=&psm.head) {
			psm.cur_note=psm.cur_note->last;
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,a2&SCF_SHIFT);
		      }
		    }
		    break;
		  case SC_CURSOR_RIGHT:
		    if (a2&SCF_CTRL) {
		      while (psm.cur_note!=&psm.head) {
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,a2&SCF_SHIFT);
			psm.cur_note=psm.cur_note->next;
		      }
		    } else {
		      if (psm.cur_note!=&psm.head) {
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,a2&SCF_SHIFT);
			psm.cur_note=psm.cur_note->next;
		      }
		    }
		    break;
		  case SC_DELETE:
		    if (a2&SCF_SHIFT)
		      PsmCutToClipboard;
		    else {
		      tmpn=psm.cur_note;
		      psm.cur_note=tmpn->next;
		      if (tmpn!=&psm.head) {
			QueRem(tmpn);
			PsmNoteDel(tmpn);
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
		    PopUpEd("::/Apps/Psalmody/Help.DD.Z",Fs);
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
		RegOneTimePopUp(ARf_PSALMODY_JUKEBOX,
		      "Sel a song and preview it.\n"
		      "$$GREEN$$<SHIFT-ESC>$$FG$$ to load it into Psalmody.\n\n"
		      ST_WARN_ST " Graphics and other embelishments\n"
		      "will be lost because Psalmody can't\n"
		      "parse HolyC programs completely.\n");
		Free(filename);
		filename=NULL;
		JukeBox(dirname,&filename);
		if (filename) {
		  psm.scrn_x=0;
		  psm_duration=1.0;
		  psm_octave=3;
		  PsmSongDel(&psm.head);
		  psm.cur_note=&psm.head;
		  PsmLoadSong(filename,&psm_octave,&psm_duration);
		  psm.record_entry->checked=FALSE;
		  psm.cur_note=psm.head.next;
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
		PsmSongDel(&psm.head);
		psm.cur_note=&psm.head;
		psm.scrn_x=0;
		break;
	      case '\n':
		if (psm.cur_note!=&psm.head)
		  PsmRightClick(psm.cur_note->x,psm.cur_note->y);
		break;
	      case 'x':
		if (was_playing)
		  break;
		col=0;
		psm.playing=TRUE;
		PsmMenu(psm_octave);
		tmpn=psm.cur_note;
		while (tmpn!=&psm.head) {
		  if (tmpn->type!=PSMT_METER) {
		    timeout_val=cnts.jiffies;
		    if (ip.pos_text.y-Fs->win_top<18)
		      msg_code=ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|
			    1<<MSG_IP_L_DOWN|1<<MSG_IP_R_UP|1<<MSG_CMD);
		    else
		      msg_code=ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|
			    1<<MSG_IP_L_DOWN|1<<MSG_CMD);
		    if (msg_code) {
		      f=0;
		      Snd(0);
		      psm.playing=FALSE;
		      was_playing=TRUE;
		      if (ip.pos_text.y-Fs->win_top>=18 &&
			    msg_code==MSG_IP_L_DOWN)
			goto mo_start;
		      else
			goto mo_got_msg;
		    }
		    psm.cur_note=tmpn;
		    psm.scrn_x+=tmpn->x-0.33*GR_WIDTH;
		    if (PsmHasWords(tmpn->word))
		      "%s",tmpn->word;
		    note=tmpn->note;
		    octave=tmpn->octave;
		    if (note!=NOTE_REST)
		      f=Note2Freq(note,octave);
		    else
		      f=-1; //rest
		    if (f>0)
		      Snd(f);
		    else
		      Snd(0);

		    music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
		    music.stacatto_factor=
			  0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;
		    d=JIFFY_FREQ*psm_durations[tmpn->duration]/music.tempo;
		    if (Bt(&tmpn->flags,PSMf_TIE)) {
		      timeout_val+=d;
		      timeout_val2=timeout_val;
		    } else {
		      timeout_val+=d*music.stacatto_factor;
		      timeout_val2=timeout_val+
			    d*(1.0-music.stacatto_factor);
		    }
		    SleepUntil(timeout_val);
		    Snd(0);
		    SleepUntil(timeout_val2);
		  }
		  tmpn=tmpn->next;
		}
		psm.cur_note=&psm.head;
		psm.scrn_x+=psm.cur_note->x-GR_WIDTH/2;
		psm.playing=FALSE;
		PsmMenu(psm_octave);
		col=0;
		f=0;
		Snd(0);
		break;
	      case CH_BACKSPACE:
		tmpn=psm.cur_note->last;
		if (tmpn!=&psm.head) {
		  QueRem(tmpn);
		  PsmNoteDel(tmpn);
		}
		if (col) {
		  '' CH_BACKSPACE;
		  col--;
		}
		break;
	      case 'z':
		if (psm.record_entry->checked)
		  psm.record_entry->checked=FALSE;
		else {
		  psm.record_entry->checked=TRUE;
		  psm_duration=1.0;
		  psm_octave=3;
		  psm.scrn_x=0;
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
	    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER
		  -WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
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
	  Snd(f);
	} else {
	  if (f<0)
	    st="R";
	  else
	    st="";
	  Snd(0);
	}
	if (psm.record_entry->checked) {
	  if (last_f) {
	    music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
	    music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;

	    tmpn->duration=PsmCvtDuration(
		  music.tempo*(evt_time-note_down_time));
	    PsmSetWidth(tmpn);
	    QueIns(tmpn,psm.cur_note->last);
	  }
	  if (f) {
	    note_down_time=tS;
	    tmpn=CAlloc(sizeof(PsmNote));
	    tmpn->type=PSMT_NOTE;
	    tmpn->note=note;
	    tmpn->octave=octave;
	    if (st[1]=='#')
	      Bts(&tmpn->flags,PSMf_SHARP);
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
  PsmSongDel(&psm.head);
  PsmSongDel(&psm.clipboard);
  TempoDel(c);
  DCFill;
  DCDel(psm.dc2);
  DocClear;
  SettingsPop;
  if (DocPut) DocPut->flags=DocPut->flags&~DOCF_FORM|old_doc_flags&DOCF_FORM;
  Free(dirname);
  MenuPop;
}
