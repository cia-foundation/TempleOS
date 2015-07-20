I64 num_people=100;

U0 SongTask(I64)
{//Song by the Holy Spirit
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
  I64 accused,crime,victim;
  Bool old_form=LBts(&(DocPut)->flags,DOCf_FORM);

  DocDblBufStart;
  '\n';
  accused=RandU32%3;
  "%s",LstSub(accused,
	"A man \0A woman \0A child \0");

  crime=RandU32&3;
  "%s",LstSub(crime,
	"commits murder\0commits adultery\0"
	"commits blasphemy\0commits idolatry\0");

  if (crime<=1) {
    '' CH_SPACE;
    victim=RandU32%3;
    "%s",LstSub(victim,"to a man\0to a woman\0to a child\0to an animal\0");
  }
  if (!(RandU32%5))
    ", again!";
  else
    '.';
  "\n\n\n\n"
	"$$LM,4$$"
	"\n\n$$BT,\"Show Mercy\",LE=0$$\n\n"
	"\n\n$$BT,\"Punish\",LE=1$$\n\n"
	"\n\n$$BT,\"Really Punish\",LE=2$$\n\n"
	"$$LM,0$$";
  DocDblBufEnd;
  DocMenu(DocPut);

  LBEqu(&(DocPut)->flags,DOCf_FORM,old_form);
  DocBottom;
}

#define T_BREAK_CAMP	0
#define T_TALK_WITH_GOD	1
#define T_VIEW_CLOUDS	2
#define T_HOLD_COURT	3
#define T_VIEW_MAP	4
#define T_WATER_ROCK	5
#define T_BATTLE	6
#define T_QUAIL		7
#define T_MOSES_COMICS	8
#define T_HELP		9
#define T_QUIT		10

Bool TakeTurn()
{
  I64 i;
  Bool result,old_form=LBts(&(DocPut)->flags,DOCf_FORM);
  num_people*=1.0+0.01*((RandU16%100)-30);
  if (num_people>MAX_PEOPLE)
    num_people=MAX_PEOPLE;

  if (!Fs->song_task)
    Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  Camp(num_people);
  Kill(Fs->song_task);

  DocDblBufStart;
  "$$LM,4$$"
	"\n\n$$BT,\"Break Camp\",LE=T_BREAK_CAMP$$\n\n"
	"\n\n$$BT,\"Talk with God\",LE=T_TALK_WITH_GOD$$\n\n"
	"\n\n$$BT,\"View Clouds\",LE=T_VIEW_CLOUDS$$\n\n"
	"\n\n$$BT,\"Hold Court\",LE=T_HOLD_COURT$$\n\n"
	"\n\n$$BT,\"View Map\",LE=T_VIEW_MAP$$\n\n"
	"\n\n$$BT,\"Make Water\",LE=T_WATER_ROCK$$\n\n"
	"\n\n$$BT,\"Battle\",LE=T_BATTLE$$\n\n"
	"\n\n$$BT,\"Beg for Meat\",LE=T_QUAIL$$\n\n"
	"\n\n$$BT,\"Moses Comics\",LE=T_MOSES_COMICS$$\n\n"
	"\n\n$$BT,\"Help\",LE=T_HELP$$\n\n"
	"\n\n$$BT,\"Quit\",LE=T_QUIT$$\n\n"
	"$$LM,0$$";
  DocDblBufEnd;
  i=DocMenu(DocPut,DOF_DONT_TEXT_ATTR);

  LBEqu(&(DocPut)->flags,DOCf_FORM,old_form);
  DocBottom;

  if (!(0<=i<T_QUIT))
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
	ViewComics;
	break;
      case T_HELP:
	PopUpEd("::/Adam/God/HSNotes.TXT.Z",Fs);
	PopUpOk("Add your own story-line...\nLike old school toys.\n");
	break;
    }
    result=TRUE;
  }
  WinMgrSync;
  Fs->song_task=NULL;
  return result;
}

U0 TMsg(CDC *dc,U8 *msg)
{
  F64 t0=tS;
  while (tS-t0<1.5) {
    if (Blink(5))
      dc->color=BLACK;
    else
      dc->color=RED;
    GrRect(dc,0,GR_HEIGHT-FONT_HEIGHT*3,GR_WIDTH,FONT_HEIGHT*2);
    dc->color=BLACK;
    GrRect(dc,2,GR_HEIGHT-FONT_HEIGHT*3+2,GR_WIDTH-4,FONT_HEIGHT*2-4);
    dc->color=YELLOW;
    GrPrint(dc,(GR_WIDTH-StrLen(msg)*FONT_WIDTH)/2,
	  GR_HEIGHT-5*FONT_HEIGHT/2,"%s",msg);
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
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  WinBorder;
  AutoComplete;
  DocClear;
  WinMax;
  PopUp("Trailer;",Fs);
  AutoComplete;
  try //Catch <CTRL-ALT-c>.
    while (TakeTurn);
  catch
    PutExcept;
  DocClear;
  SettingsPop;
}
