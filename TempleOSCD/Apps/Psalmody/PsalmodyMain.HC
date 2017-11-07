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
  for (i=0;i<PSM_DURATIONS_NUM;i++)
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
    if (0<=i<PSM_DURATIONS_NUM) {
      if (tmpn->type==PSMT_NOTE)
	tmpn->duration=i;
    } else {
      switch (i) {
	case PSMR_REST:
	  if (tmpn->type==PSMT_NOTE)
	    tmpn->ona=0;
	  break;
	case PSMR_SHARP:
	  if (tmpn->type==PSMT_NOTE && tmpn->ona) {
	    if (Btr(&tmpn->flags,PSMf_FLAT))
	      tmpn->ona++;
	    if (Btc(&tmpn->flags,PSMf_SHARP))
	      tmpn->ona--;
	    else
	      tmpn->ona++;
	  }
	  break;
	case PSMR_FLAT:
	  if (tmpn->type==PSMT_NOTE && tmpn->ona) {
	    if (Btr(&tmpn->flags,PSMf_SHARP))
	      tmpn->ona--;
	    if (Btc(&tmpn->flags,PSMf_FLAT))
	      tmpn->ona++;
	    else
	      tmpn->ona--;
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
  I64 o,n,msg_code,arg1,arg2;
  PsmNote *tmpn,*tmpn1;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP|1<<MSG_MS_MOVE);
    if (msg_code==MSG_MS_MOVE) {
      DrawDC2;
      DrawNote(psm.dc2,arg1,arg2,duration);
    }
  } while (msg_code!=MSG_MS_L_UP);
  if (arg2<13*FONT_HEIGHT) {
    if (arg1>psm.head.last->x)
      tmpn1=psm.head.last;
    else if (arg1<psm.head.next->x)
      tmpn1=&psm.head;
    else
      tmpn1=PsmFindNote(arg1-PSM_NOTE_SPACING/2,arg2);
    tmpn=CAlloc(sizeof(PsmNote));
    tmpn->type=PSMT_NOTE;
    arg2=arg2/4-15;
    n=arg2%7;
    o=4+arg2/-7;
    if (n<0) {
      n+=7;
      o++;
    }
    n=psm_note_inverse_map[n];
    if (n<3)
      o--;
    tmpn->ona=Note2Ona(n,o);
    tmpn->duration=duration;
    PsmSetWidth(tmpn);
    QueIns(tmpn,tmpn1);
    psm.cur_note=tmpn->next;
  }
  DrawDC2;
}

U0 PsmLeftClickPickMeterBox(I64 top,I64 bottom)
{
  I64 msg_code,arg1,arg2;
  PsmNote *tmpn,*tmpn1;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP|1<<MSG_MS_MOVE);
    if (msg_code==MSG_MS_MOVE) {
      DrawDC2;
      DrawTimeSignature(psm.dc2,arg1,arg2,top,bottom);
    }
  } while (msg_code!=MSG_MS_L_UP);
  if (arg2<13*FONT_HEIGHT) {
    if (arg1>=psm.head.x)
      tmpn1=psm.head.last;
    else if (arg1<psm.head.next->x)
      tmpn1=&psm.head;
    else
      tmpn1=PsmFindNote(arg1-PSM_NOTE_SPACING/2,arg2);
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
  I64 o,n,msg_code,arg1,arg2,n_original,o_original;
  psm.cur_note=tmpn=PsmFindNote(x,y);
  if (tmpn!=&psm.head) {
    if (tmpn->type==PSMT_NOTE) {
      o_original=Ona2Octave(tmpn->ona);
      n_original=Ona2Note  (tmpn->ona);
      do {
	msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP|1<<MSG_MS_MOVE);
	if (msg_code==MSG_MS_L_UP) {
	  tmpn1=PsmFindNote(arg1,arg2);
	  if (tmpn1==&psm.head || tmpn1==tmpn)
	    goto move_note;
	  else {
	    Free(tmpn1->word);
	    tmpn1->word=tmpn->word;
	    tmpn->word=NULL;
	    tmpn->ona=Note2Ona(n_original,o_original);
	  }
	} else {
move_note:
	  arg2=arg2/4-15;
	  n=arg2%7;
	  o=4+arg2/-7;
	  if (n<0) {
	    n+=7;
	    o++;
	  }
	  n=psm_note_inverse_map[n];
	  if (n<3)
	    o--;
	  tmpn->ona=Note2Ona(n,o);
	}
      } while (msg_code!=MSG_MS_L_UP);
      PsmSetWidth(tmpn);
    }
  }
}

U0 PsmLeftClickStaffBox(I64 x,I64 y)
{
  I64 msg_code,arg1,arg2;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP|1<<MSG_MS_MOVE);
    DrawDC2;
    psm.dc2->color=ROPF_DITHER+WHITE<<16+BLACK;
    GrBorder(psm.dc2,x,y,arg1,arg2);
    if (msg_code==MSG_MS_L_UP) {
      if (x>arg1) SwapI64(&x,&arg1);
      PsmMarkSel(x,arg1,TRUE);
    }
  } while (msg_code!=MSG_MS_L_UP);
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
    if (0<=duration<PSM_DURATIONS_NUM)
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
  for (j=0;j<PSM_DURATIONS_NUM;j++) {
    d1=psm_durations[j];
    d2=psm_durations[j+1];
    if (d<d1*d2/(d1+d2))
      return j;
  }
  return 0;
}

#define PSM_KEYS_NUM	20
class PsmKey
{
  U8 x,w,h,ascii;
};

#define PSM_W_W	16
#define PSM_W_H	36
#define PSM_B_W	8
#define PSM_B_H	20

PsmKey psm_kbd[PSM_KEYS_NUM]={
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
    for (i=0;i<PSM_KEYS_NUM;i++) {
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
    for (i=0;i<PSM_KEYS_NUM;i++) {
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
  Bool was_playing,is_null=TRUE,was_null=TRUE;
  I64 arg1,arg2,msg_code=0,col,ona=0,last_ona=0,
	psm_octave=4,timeout_val,timeout_val2,old_doc_flags;
  U8 *filename=NULL,*st,*st2;
  PsmNote *tmpn;
  F64 psm_duration=1.0,d,evt_time=tS,note_down_time=tS;
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
  psm.clip.next=psm.clip.last=&psm.clip;
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
	"  Octave1(,'1');"
	"  Octave2(,'2');"
	"  Octave3(,'3');"
	"  Octave4(,'4');"
	"  Octave5(,'5');"
	"  Octave6(,'6');"
	"  Octave7(,'7');"
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
      if (ms.pos_text.y-Fs->win_top<18)
	msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|
	      1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|1<<MSG_MS_R_UP|
	      1<<MSG_MS_MOVE|1<<MSG_CMD);
      else
	msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP|
	      1<<MSG_MS_MOVE|1<<MSG_CMD);
mo_got_msg:
      if (msg_code==MSG_KEY_DOWN && arg1==CH_SPACE && !arg2) {
//The Window Mgr sets the Doc cur_entry to a bttn
	//and generates a <SPACE> when the Doc Bttns are clicked.
	//This is so that kbd and mouse are the same for Doc's.
	//We must now pass the <SPACE> onto the Doc hndlr.
	PutKey(arg1,arg2);
	goto mo_start;
      }
      if (msg_code!=MSG_MS_MOVE) {
	DocBottom;
	if (was_playing || DocPut->cur_entry->y>=Fs->win_height-2) {
	  PsmMenu(psm_octave);
	  col=0;
	}
      }

      ona=Note2Ona(3,psm_octave+1); //C
      is_null=TRUE;
      switch (msg_code) {
	case MSG_CMD:
	  PsmPopMode;
	  switch (arg1) {
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
	  if ('0'<=arg1<='9') {
	    psm_octave=arg1-'0';
	    PsmMenu(psm_octave);
	    col=0;
	  } else {
	    switch (arg1) {
	      start:
		case 'a':	ona-=8;		break;
		case 's':	ona-=7;		break;
		case 'e':	ona-=6;		break;
		case 'd':	ona-=5;		break;
		case 'r':	ona-=4;		break;
		case 'f':	ona-=3;		break;
		case 't':	ona-=2;		break;
		case 'g':	ona--;		break;
		case 'h':			break;
		case 'u':	ona++;		break;
		case 'j':	ona+=2;		break;
		case 'i':	ona+=3;		break;
		case 'k':	ona+=4;		break;
		case 'l':	ona+=5;		break;
		case 'p':	ona+=6;		break;
		case ';':	ona+=7;		break;
		case '[':	ona+=8;		break;
		case '\'':	ona+=9;		break;
		case ']':	ona+=10;	break;
		case CH_SPACE:	ona=0;		break;
	      end:
		is_null=FALSE;
		break;

	      case 0:
		switch (arg2.u8[0]) {
		  case SC_CURSOR_LEFT:
		    if (arg2&SCF_CTRL) {
		      while (psm.cur_note->last!=&psm.head) {
			psm.cur_note=psm.cur_note->last;
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,arg2&SCF_SHIFT);
		      }
		    } else {
		      if (psm.cur_note->last!=&psm.head) {
			psm.cur_note=psm.cur_note->last;
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,arg2&SCF_SHIFT);
		      }
		    }
		    break;
		  case SC_CURSOR_RIGHT:
		    if (arg2&SCF_CTRL) {
		      while (psm.cur_note!=&psm.head) {
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,arg2&SCF_SHIFT);
			psm.cur_note=psm.cur_note->next;
		      }
		    } else {
		      if (psm.cur_note!=&psm.head) {
			if (psm.cur_note!=&psm.head)
			  LBEqu(&psm.cur_note->flags,PSMf_SEL,arg2&SCF_SHIFT);
			psm.cur_note=psm.cur_note->next;
		      }
		    }
		    break;
		  case SC_DELETE:
		    if (arg2&SCF_SHIFT)
		      PsmCutToClip;
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
		    if (arg2&SCF_SHIFT)
		      PsmPasteClip;
		    else if (arg2&SCF_CTRL)
		      PsmCopyToClip;
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
		  psm_octave=4;
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
		PsmCopyToClip;
		break;
	      case CH_CTRLV:
		PsmPasteClip;
		break;
	      case CH_CTRLX:
		PsmCutToClip;
		break;
	      case '.':
		PsmMenu(psm_octave);
		col=0;
		Free(filename);
		filename=NULL;
		psm_duration=1.0;
		psm_octave=4;
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
		    if (ms.pos_text.y-Fs->win_top<18)
		      msg_code=ScanMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|
			    1<<MSG_MS_L_DOWN|1<<MSG_MS_R_UP|1<<MSG_CMD);
		    else
		      msg_code=ScanMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|
			    1<<MSG_MS_L_DOWN|1<<MSG_CMD);
		    if (msg_code) {
		      Snd;
		      psm.playing=FALSE;
		      was_playing=TRUE;
		      if (ms.pos_text.y-Fs->win_top>=18 &&
			    msg_code==MSG_MS_L_DOWN)
			goto mo_start;
		      else
			goto mo_got_msg;
		    }
		    psm.cur_note=tmpn;
		    psm.scrn_x+=tmpn->x-0.33*GR_WIDTH;
		    if (PsmHasWords(tmpn->word))
		      "%s",tmpn->word;
		    Snd(tmpn->ona);

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
		    Snd;
		    SleepUntil(timeout_val2);
		  }
		  tmpn=tmpn->next;
		}
		psm.cur_note=&psm.head;
		psm.scrn_x+=psm.cur_note->x-GR_WIDTH/2;
		psm.playing=FALSE;
		PsmMenu(psm_octave);
		col=0;
		Snd;
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
		  psm_octave=4;
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
	  break;
	case MSG_MS_MOVE:
	  if (arg2>18*FONT_HEIGHT)
	    Fs->win_inhibit=WIG_USER_TASK_DFT;
	  else
	    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER
		  -WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
	  break;
	case MSG_MS_L_DOWN:
	  PsmDownKey(arg1,arg2);
	  PsmLeftClick(arg1,arg2);
	  break;
	case MSG_MS_L_UP:
	  PsmUpKey(arg1,arg2);
	  break;
	default:
	  PsmRightClick(arg1,arg2);
      }
      if (is_null)
	ona=0;
      if (ona!=last_ona || is_null!=was_null) {
	if (!ona) {
	  if (is_null)
	    st="";
	  else
	    st="R";
	} else
	  st=LstSub(Ona2Note(ona),psm_note_lst);
	Snd(ona);
	if (psm.record_entry->checked) {
	  if (!was_null) {
	    music.tempo=4.4*tempo_state.tempo/TEMPO_RANGE+0.5;
	    music.stacatto_factor=0.88*tempo_state.stacatto/TEMPO_RANGE+0.12;
	    tmpn->duration=PsmCvtDuration(
		  music.tempo*(evt_time-note_down_time));
	    PsmSetWidth(tmpn);
	    QueIns(tmpn,psm.cur_note->last);
	  }
	  if (!is_null) {
	    note_down_time=tS;
	    tmpn=CAlloc(sizeof(PsmNote));
	    tmpn->type=PSMT_NOTE;
	    tmpn->ona=ona;
	    if (st[1]=='#')
	      Bts(&tmpn->flags,PSMf_SHARP);
	  }
	}
	last_ona=ona;
	was_null=is_null;
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
  PsmSongDel(&psm.clip);
  TempoDel(c);
  DCFill;
  DCDel(psm.dc2);
  DocClear;
  SettingsPop;
  if (DocPut) DocPut->flags=DocPut->flags&~DOCF_FORM|old_doc_flags&DOCF_FORM;
  Free(dirname);
  MenuPop;
}
