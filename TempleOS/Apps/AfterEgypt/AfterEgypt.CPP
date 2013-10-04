I64 num_people=100;


U0 SongTask(I64)
{ //Song by the Holy Spirit
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("3eGFqGetBEFeBEEFetBDCeCFqF");
    Play("eGFqGetBEFeBEEFetBDCeCFqF");
    Play("AetDAAqBDeBEqEetECDGBG");
    Play("qAetDAAqBDeBEqEetECDGBG");
  }
}

U0 HoldCourt()
{
  I64 i;
  I64 accused,crime,victim;
  Bool old_form=LBts(&(DocPut)->flags,DOCf_FORM);

  FlushMsgs;
  DocDblBufStart;

  NewLine;
  accused=RandU32%3;
  "%s",LstSub(accused,"A man \0A woman \0A child \0");

  crime=RandU32&3;
  "%s",LstSub(crime,"commits murder\0commits adultery\0commits blasphemy\0commits idolatry\0");

  if (crime<=1) {
    '' CH_SPACE;
    victim=RandU32%3;
    "%s",LstSub(victim,"to a man\0to a woman\0to a child\0to an animal\0");
  }
  if (!(RandU32%5))
    ", again!";
  else
    '.';
  NewLine(4);

  "$$LM,4$$"
  "\n\n$$BT,\"Show Mercy\",1$$\n\n"
  "\n\n$$BT,\"Punish\",2$$\n\n"
  "\n\n$$BT,\"Really Punish\",3$$\n\n"
  "$$LM,0$$";
  DocDblBufEnd;
  i=DocMenuDo(DocPut);

  LBEqu(&(DocPut)->flags,DOCf_FORM,old_form);
  DocBottom;
}

#define T_BREAK_CAMP	1
#define T_TALK_WITH_GOD	2
#define T_VIEW_CLOUDS	3
#define T_HOLD_COURT	4
#define T_VIEW_MAP	5
#define T_WATER_ROCK	6
#define T_BATTLE	7
#define T_QUAIL		8
#define T_MOSES_COMICS	9
#define T_MUTE		10
#define T_HELP		11
#define T_QUIT		12


Bool TakeTurn()
{
  I64 i;
  Bool result,old_form=LBts(&(DocPut)->flags,DOCf_FORM);
  num_people*=1.0+0.01*((RandU16%100)-30);
  if (num_people>MAX_PEOPLE)
    num_people=MAX_PEOPLE;
  Camp(num_people);

  FlushMsgs;
  DocDblBufStart;
	"$$LM,4$$"
	"\n\n$$BT,\"Break Camp\",T_BREAK_CAMP$$\n\n"
	"\n\n$$BT,\"Talk with God\",T_TALK_WITH_GOD$$\n\n"
	"\n\n$$BT,\"View Clouds\",T_VIEW_CLOUDS$$\n\n"
	"\n\n$$BT,\"Hold Court\",T_HOLD_COURT$$\n\n"
	"\n\n$$BT,\"View Map\",T_VIEW_MAP$$\n\n"
	"\n\n$$BT,\"Make Water\",T_WATER_ROCK$$\n\n"
	"\n\n$$BT,\"Battle\",T_BATTLE$$\n\n"
	"\n\n$$BT,\"Beg for Meat\",T_QUAIL$$\n\n"
	"\n\n$$BT,\"Moses Comics\",T_MOSES_COMICS$$\n\n"
	"\n\n$$BT,\"Toggle Mute\",T_MUTE$$\n\n"
	"\n\n$$BT,\"Help\",T_HELP$$\n\n"
	"\n\n$$BT,\"Quit\",T_QUIT$$\n\n"
	"$$LM,0$$";
  DocDblBufEnd;
  i=DocMenuDo(DocPut,DOF_DONT_TEXT_ATTR);

  LBEqu(&(DocPut)->flags,DOCf_FORM,old_form);
  DocBottom;

  if (i<0 || i==T_QUIT)
    result=FALSE;
  else {
    switch (i) {
      case T_BREAK_CAMP:
	break;
      case T_TALK_WITH_GOD:
	UpTheMountain;
	break;
      case T_VIEW_CLOUDS:
	CloudScene;
	break;
      case T_HOLD_COURT:
	HoldCourt;
	break;
      case T_VIEW_MAP:
	AEMap;
	break;
      case T_WATER_ROCK:
	WaterRock;
	break;
      case T_BATTLE:
	Battle;
	break;
      case T_QUAIL:
	Quail;
	break;
      case T_MOSES_COMICS:
	Suspend(Fs->song_task);
	Snd(0);
	ViewComics;
	Suspend(Fs->song_task,FALSE);
	break;
      case T_MUTE:
	Mute(!IsMute);
	break;
      case T_HELP:
	PopUp("Ed(\"::/TempleOS/Apps/HolySpirit/HSNotes.TXT.Z\");",Fs);
	PopUpOk("Add your own story-line...\nLike old school toys.\n");
	break;
    }
    result=TRUE;
  }
  WinMgrSync;
  return result;
}

U0 TMsg(CDC *dc,U8 *msg)
{
  F64 t0=tT;
  while (tT-t0<1.5) {
    if (Blink(5))
      dc->color=BLACK;
    else
      dc->color=RED;
    GrRect(dc,0,GR_HEIGHT-FONT_HEIGHT*3,GR_WIDTH,FONT_HEIGHT*2);
    dc->color=BLACK;
    GrRect(dc,2,GR_HEIGHT-FONT_HEIGHT*3+2,GR_WIDTH-4,FONT_HEIGHT*2-4);
    dc->color=YELLOW;
    GrPrint(dc,(GR_WIDTH-StrLen(msg)*FONT_WIDTH)/2,GR_HEIGHT-5*FONT_HEIGHT/2,"%s",msg);
    WinMgrSync;
    if (ScanChar)
      throw;
  }
}

U0 Trailer()
{
  CDC *dc=DCAlias;
  WinMax;
  WinBorder;
  Cd(Fs->parent_task->cur_dir);
  Type("AESplash.TXT");
  Sleep(500);
  FlushMsgs;
  try {
    TMsg(dc,"Leaving all behind, they fled.");
    TMsg(dc,"Found themselves in a desert.");
    TMsg(dc,"God!  We're gonna die!");
    TMsg(dc,"\"Trust Me!\"");
  } catch
    Fs->catch_except=TRUE;
  DCFill;
  DCDel(dc);
}

U0 AfterEgypt()
{
  Bool was_mute=IsMute;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinBorder;
  WordStat;
  DocClear;
  WinMax;
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  PopUp("Trailer;",Fs);

  WSInit("::/TempleOS/Apps/HolySpirit/Vocab.TXT*");
  WordStat;

  try //This is to catch <CTRL-ALT-C>.
    while (TakeTurn);
  catch
    CatchAll;
  DocClear;
  SettingsPop;
  Mute(was_mute);
}
