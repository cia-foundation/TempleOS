U0 TS(I64 i)
{
  progress4=i;
  progress4_max=171;
  *progress4_desc=0;
}
U0 TSFile(U8 *name,I64 ms=750)
{
  CTask *task=User("#include \"%s\";Sleep(%d);\n",name,ms);
  DeathWait(&task,TRUE);
}
U0 TSFileChar(U8 *name,I64 ms=750,I64 ch=CH_SPACE,Bool wait=TRUE)
{
  CTask *task=User;
  if (wait)
    XTalkWait(task,"#include \"%s\";\n",name);
  else
    XTalk(task,"#include \"%s\";\n",name);
  Sleep(ms);
  if (ch)
    PostMsgWait(task,MSG_KEY_DOWN_UP,ch,0);
  DeathWait(&task,TRUE);
}
/*
U0 DoInputPointer()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/InputPointer\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoPullDownMenu()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/PullDownMenu\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoTicTacToe()
{
}

U0 DoLife()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Life\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoZing()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Zing\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoSlider()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Slider\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoScrollBars()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/ScrollBars\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoWhap()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Whap\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoDataBase()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/DataBase\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoDskRaw()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/DskRaw\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoTimeClock()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/TimeClock\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoLectures()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoInFile()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/InFile\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoSpy()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Spy\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoUnusedDefine()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/UnusedDefine\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}
*/
$PL,1000$
U0 DoRandDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/RandDemo\";\n");
  XTalkWait(task," ");
  XTalkWait(task," ");
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoLowPassFilter()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/LowPassFilter\";\n");
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoMathAudioDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/MathAudioDemo\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoMsgLoop()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MsgLoop\";\n");
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoASCIIOrgan()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Snd/ASCIIOrgan\";\n");
  Sleep(100);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'A',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'B',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'C',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoDoodle()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Doodle\";\n");
  PostMsgWait(task,MSG_IP_L_DOWN,10,10);
  PostMsgWait(task,MSG_IP_L_UP,100,200);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoSpeedLine()
{
  I64 i=PURPLE+1; //+1 because TRANSPARENT
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Speedline\";\n");
  PostMsgWait(task,MSG_IP_L_DOWN,10,10);
  PostMsgWait(task,MSG_IP_L_UP,100,200);
  PostMsg(task,MSG_IP_R_DOWN_UP,0,0);
  BirthWait(&task->popup_task);
  while (i--)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_ESC,0);
  TaskWait(task);
  PostMsgWait(task,MSG_IP_L_DOWN,50,10);
  PostMsgWait(task,MSG_IP_L_UP,150,200);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoKeyBitMap()
{
  I64 i_sc=Char2ScanCode('i');
  CTask *task=User("#include \"::/Demo/KeyBitMap\";\n");
  Sleep(50);
  LBts(kbd.down_bitmap,i_sc);
  Sleep(500);
  LBtr(kbd.down_bitmap,i_sc);
  Sleep(50);
  DeathWait(&task,TRUE);
}

U0 DoMountain()
{
  CTask *task=User("#include \"::/Demo/Graphics/Mountain\";\n");
  Sleep(1500);
  Kill(task);
}

U0 DoDigits()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Digits\";\n");
  Sleep(100);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'1',0);
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoSymmetry()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Symmetry\";\n");
  PostMsgWait(task,MSG_IP_R_DOWN,100,100);
  PostMsgWait(task,MSG_IP_R_UP,200,200);
  Sleep(100);
  PostMsgWait(task,MSG_IP_L_DOWN,10,10);
  PostMsgWait(task,MSG_IP_L_UP,100,200);
  Sleep(250);
  PostMsgWait(task,MSG_IP_L_DOWN,100,200);
  PostMsgWait(task,MSG_IP_L_UP,400,400);
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoBSpline()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/BSpline\";\n");
  PostMsgWait(task,MSG_IP_L_DOWN_UP,50,50);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,300,100);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,150,300);
  PostMsgWait(task,MSG_IP_R_DOWN_UP,0,0);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoScrnCapture()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/ScrnCapture\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
  Del("~/DemoScrnShot.GR*");
}

U0 DoStadium()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Stadium/Stadium\";\n");
  Sleep(50);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,100,10);
  Sleep(300);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,320,20);
  Sleep(300);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,520,10);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoPalette()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Palette\";\n");
  Sleep(400);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(400);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoElephantWalk()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/ElephantWalk\";\n");
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoHalogen()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Halogen\";\n");
  PostMsgWait(task,MSG_KEY_DOWN,0,SC_CURSOR_UP);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_UP,0,SC_CURSOR_UP);
  PostMsgWait(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);
  Sleep(333);
  PostMsgWait(task,MSG_KEY_UP,0,SC_CURSOR_RIGHT);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoTheDead()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/TheDead\";\n");
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(50);
  }
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoBomberGolf()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/BomberGolf\";\n");
  for (i=0;i<7;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<7;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(200);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoZoneOut()
{
  I64 i;
  CTask *task=User("#include \"::/Demo/Games/ZoneOut\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoVaroom()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Varoom\";\n");
  for (i=0;i<10;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoFlatTops()
{
  CTask *task=User("#include \"::/Demo/Games/FlatTops\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  PostMsgWait(task,MSG_IP_R_DOWN_UP,RandI16%400+200,RandI16%300+150);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoWenceslas()
{
  I64 i;
  CTask *task=User("#include \"::/Demo/Games/Wenceslas\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  for (i=0;i<25;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(25);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoTreeCheckers()
{
  I64 task_num;
  CTask *task=User("#include \"::/Demo/Games/TreeCheckers\";\n");

  task_num=BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  Sleep(500);

  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoMorseCode()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Snd/MorseCode\";\n");
  Sleep(50);

  PostMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  Sleep(200);
  PostMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(350);

  PostMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  Sleep(50);
  PostMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(350);

  PostMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  Sleep(50);
  PostMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(25);
  PostMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  Sleep(200);
  PostMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(25);
  PostMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  Sleep(50);
  PostMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(500);

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoPixCollision()
{
  I64 w,h;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Collision\";\n");
  w=task->pix_width>>1 +task->pix_left+task->scroll_x;
  h=task->pix_height>>1+task->pix_top +task->scroll_y;
  IPSet(w-35,h-35);
  InSetIP(10,w+35,w+35);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoBlackDiamond()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/BlackDiamond\";\n");
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(75);
  }
  for (i=0;i<12;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(75);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoCtrlR1()
{
  I64 i,task_num;
  CTask *task=User;
  XTalkWait(task,"//");
  PostMsg(task,MSG_KEY_DOWN_UP,CH_CTRLR,0);
  task_num=BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  for (i=0;i<1;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  //Color
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Width
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<4;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<4;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Line
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<7;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  PostMsgWait(task,MSG_IP_L_DOWN,150,150);
  PostMsgWait(task,MSG_IP_L_UP,150,95);
  Sleep(250);
  PostMsgWait(task,MSG_IP_L_DOWN,150,150);
  PostMsgWait(task,MSG_IP_L_UP,190,190);
  Sleep(250);
  PostMsgWait(task,MSG_IP_L_DOWN,150,150);
  PostMsgWait(task,MSG_IP_L_UP,110,190);
  Sleep(250);
  PostMsg(task,MSG_IP_R_DOWN_UP,100,100);

  //Color
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<3;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Circle
  task_num=BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  for (i=0;i<10;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  PostMsgWait(task,MSG_IP_L_DOWN,150,150);
  PostMsgWait(task,MSG_IP_L_UP,190,190);
  Sleep(250);
  PostMsg(task,MSG_IP_R_DOWN_UP,100,100);

  //Exit
  BirthWait(&task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN+SCF_CTRL);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  Sleep(1000);
  XTalk(task,"\n");
  DeathWait(&task,TRUE);
}

U0 DoF2Macro()
{
  I64 i;
  U8 *ptr="\"Boo!\\n\";\n";
  CTask *task=User;
  DeathWait(&sys_macro_task);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_F2);
  BirthWait(&sys_macro_task);
  TaskWait(sys_macro_task);

  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT|SCF_CTRL);
  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Press RECORD
  while (!Bt(&sys_semas[SYS_SEMA_RECORD_MACRO],0))
    Yield;

  while (*ptr)
    PostMsgWait(task,MSG_KEY_DOWN,*ptr++,0);

  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(sys_macro_task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Press STOP

  for (i=0;i<10;i++)
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_F2|SCF_SHIFT);

  PostMsg(sys_macro_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&sys_macro_task);

  Sleep(1000);
  XTalk(task,"\n");
  DeathWait(&task,TRUE);
}

U0 DoPredatorPrey()
{
  CTask *task=User("#include \"::/Demo/Graphics/PredatorPrey\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoCharDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/CharDemo\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
  Sleep(750);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoLattice()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Lattice\";\n");
  for (i=0;i<20;i++)
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  for (i=0;i<6;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,'+',0);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
  }
  for (i=0;i<6;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
  }
  for (i=0;i<16;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  }
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoCartesian()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Cartesian\";\n");
  XTalkWait(task,"0.2*x`1.5\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoMPAdd()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/MPAdd\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoFlapBat()
{
  I64 i;
  CTask *task=User("#include \"::/Demo/Games/FlapBat\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  for (i=0;i<4;i++) {
    PostMsgWait(task,MSG_KEY_DOWN,CH_SPACE,0);
    Sleep(100);
    PostMsgWait(task,MSG_KEY_UP,CH_SPACE,0);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoBattleLines()
{
  CTask *task=User("#include \"::/Demo/Games/BattleLines\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  IPSet(430,300,,TRUE);
  InSetIP(10,530,400);
  IPSet(,,,FALSE);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoBigGuns()
{
  I64 i,task_num;
  CTask *task=User("#include \"::/Demo/Games/BigGuns\";\n");
  task_num=BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  BirthWait(&task->popup_task,task_num);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(200);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoDunGen()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/DunGen\";\n");
  for (i=0;i<10;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  for (i=0;i<12;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(100);
  }
  for (i=0;i<6;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoTimeOut()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/TimeOut/Run\";\n");
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<3;i++) {
    PostMsgWait(task,MSG_KEY_DOWN,CH_SPACE,0);
    LBts(kbd.down_bitmap,SC_CURSOR_RIGHT);
    Sleep(200);
    PostMsgWait(task,MSG_KEY_UP,CH_SPACE,0);
    LBtr(kbd.down_bitmap,SC_CURSOR_RIGHT);
    Sleep(200);
    PostMsgWait(task,MSG_KEY_DOWN,CH_SPACE,0);
    LBts(kbd.down_bitmap,SC_CURSOR_LEFT);
    Sleep(200);
    PostMsgWait(task,MSG_KEY_UP,CH_SPACE,0);
    LBtr(kbd.down_bitmap,SC_CURSOR_LEFT);
    Sleep(200);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoToTheFront()
{
  I64 task_num;
  CTask *task=User("#include \"::/Apps/ToTheFront/Run\";\n");
  task_num=BirthWait(&task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,'\n',0);
  task_num=BirthWait(&task->popup_task,task_num);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  task_num=BirthWait(&task->popup_task,task_num);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  BirthWait(&task->popup_task,task_num);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN,CH_SPACE,0);
  DeathWait(&task->popup_task);
  Sleep(5000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoPsalmody()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Psalmody/Run\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,'z',0);
  PostMsgWait(task,MSG_KEY_DOWN,'h',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_UP,'h',0);
  PostMsgWait(task,MSG_KEY_DOWN,'g',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_UP,'g',0);
  PostMsgWait(task,MSG_KEY_DOWN,'h',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_UP,'h',0);
  PostMsgWait(task,MSG_KEY_DOWN,'j',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_UP,'j',0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT+SCF_CTRL);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'x',0);
  Sleep(1250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoJukeBox()
{
  CTask *task=User("#include \"::/Apps/Psalmody/Load\";"
	"JukeBox(\"::/Apps/Psalmody/Examples\");\n");
  BirthWait(&task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task->popup_task);
  DeathWait(&task,TRUE);
}

U0 DoSpan()
{
  CTask *task=User("#include \"::/Apps/Span/Run\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoStrut()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Strut/Run\";\n");

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  Refresh(2);

  IPSet(GR_WIDTH-200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);

  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);

  PostMsgWait(task,MSG_KEY_DOWN_UP,'s',0);

  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH-200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(GR_WIDTH-200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(GR_WIDTH/2,GR_HEIGHT/2,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH-200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(GR_WIDTH/2,GR_HEIGHT/2,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(GR_WIDTH/2,GR_HEIGHT/2,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  PostMsgWait(task,MSG_KEY_DOWN_UP,'t',0);

  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(GR_WIDTH-200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(GR_WIDTH/2,400,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_UP,0,0);

  IPSet(,,,FALSE);

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  LBts(kbd.down_bitmap,Char2ScanCode('1'));
  Sleep(600);
  LBtr(kbd.down_bitmap,Char2ScanCode('1'));
  PostMsgWait(task,MSG_KEY_DOWN_UP,'Z',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'Z',0);
  Sleep(200);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'Z',0);
  Sleep(200);

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoRawHide()
{
  CTask *task=User("#include \"::/Demo/Games/RawHide\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoAfterEgypt()
{
  CTask *task=User("#include \"::/Apps/AfterEgypt/Run\";\n");
  BirthWait(&task->popup_task);
  Sleep(600); //It flushes messages!
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task->popup_task);
  TaskWait(task);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Camp
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Map
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Map
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Camp
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Main Menu
  DeathWait(&task,TRUE);
}

U0 DoPoleZeros()
{
  CTask *task=User("#include \"::/Demo/Graphics/PoleZeros\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoWhap()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Whap\";\n");
  IPSet(300,200);
  Sleep(500);
  InSetIP(3,350,300);
  Sleep(500);
  InSetIP(3,450,200);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoGrid()
{
  CTask *task=User("#include \"::/Demo/Graphics/Grid\";\n");
  IPSet(200,200);
  Sleep(150);
  InSetIP(2,400,400);
  InSetIP(2,200,400);
  InSetIP(2,200,200);
  IPSet(,,,TRUE);
  Sleep(50);
  IPSet(,,,FALSE);
  DocBottom(DocPut(task));
  DeathWait(&task,TRUE);
}

U0 DoPick()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Pick\";\n");
  IPSet(200,200);
  Sleep(500);
  IPSet(,,,TRUE);
  InSetIP(4,400,400);
  IPSet(,,,FALSE);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoPick3D()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Pick3D\";\n");
  IPSet(200,200);
  Sleep(500);
  IPSet(,,,TRUE);
  InSetIP(4,400,400);
  IPSet(,,,FALSE);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoGrModels()
{
  CTask *task=User;
  I64 i,j,task_num=-1;
  XTalkWait(task,"#include \"::/Apps/GrModels/Run\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  for (i=0;i<4;i++) {
    task_num=BirthWait(&task->popup_task,task_num);
    TaskWait(task->popup_task);
    for (j=0;j<=i;j++)
      PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'n',0);
  DeathWait(&task,TRUE);
}

U0 DoKeepAway()
{
  CTask *task=User("#include \"::/Apps/KeepAway/Run\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  IPSet(100,100);
  Sleep(1500);
  PostMsgWait(task,MSG_IP_R_DOWN_UP,ip.pos.x,ip.pos.y);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoRocket()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Rocket\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  Bts(kbd.down_bitmap,SC_CURSOR_UP);
  Sleep(1000);
  Btr(kbd.down_bitmap,SC_CURSOR_UP);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  Bts(kbd.down_bitmap,SC_CURSOR_RIGHT);
  Sleep(1000);
  Btr(kbd.down_bitmap,SC_CURSOR_RIGHT);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoRocketScience()
{
  CTask *task=User("#include \"::/Demo/Games/RocketScience\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoMassSpring()
{
  CTask *task=User("#include \"::/Demo/Games/MassSpring\";\n");
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  PostMsgWait(task,MSG_IP_L_DOWN_UP,50,50);  //#1
  PostMsgWait(task,MSG_IP_L_DOWN_UP,50,200); //#2
  PostMsgWait(task,MSG_IP_L_DOWN_UP,210,50); //#3
  PostMsgWait(task,MSG_IP_L_DOWN_UP,310,200);//#4
  Sleep(500);
  PostMsgWait(task,MSG_IP_R_DOWN,50,50); //1-2
  PostMsgWait(task,MSG_IP_R_UP,50,200);
  Sleep(500);
  PostMsgWait(task,MSG_IP_R_DOWN,210,50);//3-4
  PostMsgWait(task,MSG_IP_R_UP,310,200);
  Sleep(500);
  PostMsgWait(task,MSG_IP_R_DOWN,50,200);//2-4
  PostMsgWait(task,MSG_IP_R_UP,310,200);
  Sleep(500);
  PostMsgWait(task,MSG_IP_R_DOWN,50,50); //1-3
  PostMsgWait(task,MSG_IP_R_UP,210,50);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoSquirt()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Squirt\";\n");
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoXCaliber()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/X-Caliber/Run\";\n");
  for (i=0;i<5;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SCF_SHIFT|SC_CURSOR_UP);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SCF_SHIFT|SC_CURSOR_RIGHT);
  }
  for (i=0;i<10;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    PostMsgWait(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT,0);
    Sleep(100);
    PostMsgWait(task,MSG_KEY_UP,0,SC_CURSOR_RIGHT,0);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoFPS()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/CastleFrankenstein\";\n");
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<8;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(50);
  }
  for (i=0;i<9;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<6;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoEagleDive()
{
  I64 i;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/EagleDive\";\n");
  Sleep(500);
  for (i=0;i<10;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<10;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(50);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoPhoneNumWords()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/PhoneNumWords\";\n");
  XTalkWait(task,"702-254-4223\n\n");
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoSuggestedSpelling()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/SuggestSpelling\";\n");
  XTalkWait(task,"effecient\n\n");
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoFPrintF()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/FPrintF\";\n");
  XTalkWait(task,"~/DemoFPrintF.DD.Z");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  XTalkWait(task,"Type(\"~/DemoFPrintF.DD.Z\");\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_CTRLO,0);
  XTalkWait(task,"Del(\"~/DemoFPrintF.DD*\");\n");
  Sleep(500);
  DeathWait(&task,TRUE);
}

U0 DoLastClass()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/LastClass\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  DeathWait(&task,TRUE);
}

U0 DoMiniCompiler()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures/MiniCompiler\";\n");
  XTalkWait(task,"1+2*(3+4)\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoWebLog()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/WebLogDemo/WebLogRep\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoRevFile()
{
  CTask *task;
  Del("~/DemoPoemFwd.DD*");
  task=User("#include \"::/Demo/RevFileDemo/Rev\";"
	"Type(\"~/DemoPoemFwd.DD.Z\");Sleep(750);\n");
  DeathWait(&task,TRUE);
}

U0 DoSortFile()
{
  CTask *task=User("#include \"::/Demo/SortFileDemo/F64FileGen\";"
	"#include \"::/Demo/SortFileDemo/F64FileSort\";Sleep(750);\n");
  DeathWait(&task,TRUE);
}

U0 DoToHtmlToTXT()
{
  CTask *task=User("#include \"::/Demo/ToHtmlToTXTDemo/HtmlGen\";"
	"#include \"::/Demo/ToHtmlToTXTDemo/TXTGen\";Sleep(750);\n");
  DeathWait(&task,TRUE);
}

U0 DoLogic()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Logic/Run\";\n");
  XTalkWait(task,"NAND\nNOR\n\n0x100\n0xF0\n0xCC\n0xAA\n\n0x12\n\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoPrompt()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Prompt\";\n");
  XTalkWait(task,"1+2*3<<4\n");
  XTalkWait(task,"1.0+2*3`2\n");
  XTalkWait(task,"Terry Davis\n");
  XTalkWait(task,"*-1\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoNumBible()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/NumBible\";\n");
  Del("~/DemoNumBible.DD*");
  DeathWait(&task,TRUE);
}

U0 DoForm()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/Form\";\n");
  Sleep(250);
  XTalk(task,"54321");
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SCF_CTRL|SC_CURSOR_LEFT);
  XTalk(task,"77777");
  Sleep(250);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(250);
  XTalk(task,"MyName");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoClickCallBack()
{
  I64 i;
  CTask *task=User("#include \"::/Demo/DolDoc/ClickCallBack\";\n");
  for (i=0;i<3;i++) {
    BirthWait(&task->popup_task);
    TaskWait(task->popup_task);
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(250);
  }
  Sleep(500);
  DeathWait(&task,TRUE);
}

U0 DoMenuButton()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/MenuButton\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(500);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  BirthWait(&task->popup_task);
  TaskWait(task->popup_task);
  Sleep(500);
  PostMsgWait(sys_focus_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  Sleep(500);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task->popup_task);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoMenuSprite()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/MenuSprite\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoExceptions()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Exceptions\";\n");
  XTalkWait(task,"yy");
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoDemoDoc()
{
  CTask *task=User;
  XTalkWait(task,"Ed(\"::/Demo/DolDoc/DemoDoc.DD\");\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_CTRLG,0);
  XTalkWait(task,"100");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  DeathWait(&task,TRUE);
}

U0 DoFileRead()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/FileRead\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  DeathWait(&task,TRUE);
}

U0 DoDefine()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Define\";\n");
  XTalkWait(task,"YS\n");
  Sleep(750);
  DeathWait(&task,TRUE);
}

U0 DoAsmAndC1()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmAndC1\";\n2\n");
  DeathWait(&task,TRUE);
}

U0 DoAsmAndC2()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmAndC2\";\n2\n");
  DeathWait(&task,TRUE);
}

U0 OSTestSuite()
{ //$MA+LIS-X,"RENUM",LM="I64 i;for (i=0;i<171;i++){if(i)Msg(MSG_KEY_DOWN_UP,0,SC_F3);else\"%cTS(%c\",CH_CTRLF,CH_ESC;Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);\"%03d\",i;Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);\"%c%c%c\",CH_BACKSPACE,CH_BACKSPACE,CH_BACKSPACE;}"$
  CTask *task;
  TS(000); DoBlackDiamond;
  TS(001); DoEagleDive;
  TS(002); DoFlatTops;
  TS(003); DoDunGen;
  TS(004); DoFPS;
  TS(005); DoZoneOut;
  TS(006); DoVaroom;
  TS(007); DoRocket;
  TS(008); DoRocketScience;
  TS(009); DoBattleLines;
  TS(010); DoBigGuns;
  TS(011); DoFlapBat;
  TS(012); DoTimeOut;
  TS(013); DoToTheFront;
  TS(014); DoPsalmody;
  TS(015); DoJukeBox;
  TS(016); if (!sys_heap_init_flag) DoSpan; //Has FloodFill
  TS(017); DoStrut;
  TS(018); DoRawHide;
  TS(019); DoKeepAway;
  TS(020); DoAfterEgypt;
  TS(021); DoXCaliber;
  TS(022); DoWenceslas;
  TS(023); DoBomberGolf;
  TS(024); DoTheDead;
  TS(025); DoTreeCheckers;
  TS(026); DoMPAdd;
  TS(027); TSFile("::/Demo/MultiCore/Primes");
  TS(028); if (FileFind(BIBLE_FILENAME))
    TSFile("::/Demo/MultiCore/Palindrome");
  TS(029); TSFileChar("::/Demo/MultiCore/Clouds");
  TS(030); if (mp_cnt>1) TSFile("::/Demo/MultiCore/RadixSort");
  TS(031); if (mp_cnt>1 && DrvIsWritable(':'))
    TSFileChar("::/Demo/MultiCore/LoadTest");
  TS(032); if (mp_cnt>1) TSFile("::/Demo/MultiCore/MPPrint");
  TS(033); if (mp_cnt>1) TSFile("::/Demo/MultiCore/Lock");
  TS(034); if (mp_cnt>1) TSFile("::/Demo/MultiCore/Interrupts");
  TS(035); TSFileChar("::/Demo/Graphics/SpritePlot");
  TS(036); TSFileChar("::/Demo/Graphics/Elephant",,CH_SHIFT_ESC);
  TS(037); TSFileChar("::/Demo/Graphics/SpritePlot3D");
  TS(038); TSFile("::/Demo/Graphics/SpritePut");
  TS(039); TSFile("::/Demo/Graphics/SpritePutExt");
  TS(040); TSFile("::/Demo/Graphics/SpriteText");
  TS(041); TSFile("::/Demo/Graphics/SpriteRaw");
  TS(042); TSFile("::/Demo/Graphics/EdSprite");
  TS(043); TSFile("::/Demo/Graphics/Balloon",0);
  TS(044); TSFile("::/Demo/Carry");
  TS(045); TSFile("::/Demo/Directives");
  TS(046); DoRandDemo;
  TS(047); TSFileChar("::/Demo/Graphics/Extents");
  TS(048); TSFileChar("::/Demo/Graphics/NetOfDots");
  TS(049); TSFileChar("::/Demo/Graphics/SunMoon");
  TS(050); DoLowPassFilter;
  TS(051); DoMathAudioDemo;
  TS(052); TSFileChar("::/Demo/Graphics/Lines");
  TS(053); TSFileChar("::/Demo/Graphics/Bounce",,CH_SHIFT_ESC,FALSE);
  TS(054); DoMsgLoop;
  TS(055); DoASCIIOrgan;
  TS(056); DoMorseCode;
  TS(057); DoPixCollision;
  TS(058); DoDoodle;
  TS(059); DoMassSpring;
  TS(060); DoSpeedLine;
  TS(061); DoKeyBitMap;
  TS(062); DoMountain;
  TS(063); DoBSpline;
  TS(064); DoGrModels;
  TS(065); TSFileChar("::/Demo/Graphics/Blot",1800);
  TS(066); if (DrvIsWritable('~')) DoScrnCapture;
  TS(067); DoGrid;
  TS(068); DoPick;
  TS(069); DoPick3D;
  TS(070); DoWhap;
  TS(071); DoPalette;
  TS(072); TSFile("::/Demo/Print");
  TS(073); TSFileChar("::/Demo/Graphics/Shading",1500);
  TS(074); TSFileChar("::/Demo/Graphics/RotateTank",1500);
  TS(075); TSFileChar("::/Demo/Graphics/3DPoly",1500);
  TS(076); TSFileChar("::/Demo/Graphics/Box",1500);
  TS(077); DoSymmetry;
  TS(078); TSFileChar("::/Demo/Graphics/Shadow",1500);
  TS(079); TSFileChar("::/Demo/Graphics/Transform",,CH_SHIFT_ESC);
  TS(080); DoPredatorPrey;
  TS(081); DoPoleZeros;
  TS(082); DoDigits;
  TS(083); DoStadium;
  TS(084); DoElephantWalk;
  TS(085); DoHalogen;
  TS(086); TSFileChar("::/Demo/Games/Maze",,CH_SHIFT_ESC);
  TS(087); TSFileChar("::/Demo/Graphics/FontEd",,CH_SHIFT_ESC);
  TS(088); DoLattice;
  TS(089); DoCtrlR1;
  TS(090); DoF2Macro;
  TS(091); TSFile("::/Demo/SubIntAccess");
  TS(092); DoDemoDoc;
  TS(093); TSFile("::/Demo/DolDoc/TreeDemo");
  TS(094); TSFile("::/Demo/DolDoc/TextDemo");
  TS(095); TSFile("::/Demo/DolDoc/CursorMove");
  TS(096); DoMiniCompiler;
  TS(097); TSFile("::/Demo/Lectures/MiniGrLib",0);
  TS(098); TSFileChar("::/Demo/TimeIns");
  TS(099); DoPhoneNumWords;
  TS(100); TSFile("::/Demo/Dsk/UnusedSpaceRep");
  TS(101); TSFile("::/Demo/Dsk/BlkDevRep");
  TS(102); DoLastClass;
  TS(103); if (DrvIsWritable('~')) DoFPrintF;
  TS(104); TSFile("::/Demo/Dsk/SerializeTree");
  TS(105); DoExceptions;
  TS(106); TSFileChar("::/Demo/ScrnCodes");
  TS(107); TSFile("::/Demo/ExtChars");
  TS(108); TSFile("::/Demo/Graphics/PanText",0);
  TS(109); TSFile("::/Demo/Graphics/CharAnimation");
  TS(110); DoCharDemo;
  TS(111); TSFile("::/Demo/DateTime");
  TS(112); TSFile("::/Demo/SubSwitch");
  TS(113); TSFile("::/Demo/NullCase");
  TS(114); TSFile("::/Demo/MagicPairs");
  TS(115); TSFileChar("::/Demo/Graphics/Hanoi",3000);
  TS(116); DoSquirt;
  TS(117); TSFileChar("::/Demo/Graphics/CommonAncestor",1500,CH_SHIFT_ESC);
  TS(118); DoCartesian;
  TS(119); TSFileChar("::/Demo/Games/RainDrops",,CH_SHIFT_ESC);
  TS(120); TSFileChar("::/Demo/Games/Collision",1500);
  TS(121); DoLogic;
  TS(122); TSFile("::/Demo/CompileDemo");
  TS(123); DoPrompt;
  TS(124); if (DrvIsWritable('~')) DoWebLog;
  TS(125); if (DrvIsWritable('~')) DoRevFile;
  TS(126); if (DrvIsWritable('~')) DoSortFile;
  TS(127); if (DrvIsWritable('~')) DoToHtmlToTXT;
  TS(128); if (DrvIsWritable('~')) TSFile("::/Demo/RegistryDemo");
  TS(129); DoDefine;
  TS(130); TSFile("::/Demo/GlblVars");
  TS(131); DoFileRead;
  TS(132); TSFile("::/Demo/ParenWarn");
  TS(133); TSFile("::/Demo/DolDoc/DefineStr");
  TS(134); TSFile("::/Demo/DolDoc/Data");
  TS(135); TSFile("::/Demo/DolDoc/CallBack");
  TS(136); TSFile("::/Demo/ClassMeta");
  TS(137); if (DrvIsWritable('~')) DoNumBible;
  TS(138); DoForm;
  TS(139); DoClickCallBack;
  TS(140); DoMenuButton;
  TS(141); DoMenuSprite;
  TS(142); DoSuggestedSpelling;
  TS(143); TSFileChar("::/Demo/WordSearch");
  TS(144); TSFile("::/Demo/StkGrow");
  TS(145); TSFile("::/Demo/MemDemo");
  TS(146); TSFileChar("::/Demo/Snd/WaterFowl");
  TS(147); TSFile("::/Demo/Asm/AsmHelloWorld");
  TS(148); DoAsmAndC1;
  TS(149); DoAsmAndC2;
  TS(150); TSFile("::/Demo/Asm/AsmAndC3");
  TS(151); TSFile("::/Demo/Asm/MulByHand");
  TS(152); TSFile("::/Demo/Asm/DivByHand");
  TS(153); TSFile("::/Demo/Asm/BuzzFizz");
  TS(154); TSFile("::/Demo/Asm/PutDec");

  task=User;
  TS(155); XTalkWait(task,"DocMax;Prof;HeapLog(ON);\n");
  TS(156); XTalkWait(task,"PCIRep;Sleep(750);\n");
  TS(157); XTalkWait(task,"MemBIOSRep;Sleep(750);\n");
  TS(158); XTalkWait(task,"MemRep;Sleep(750);\n");
  TS(159); XTalkWait(task,"ZipRep;Sleep(750);\n");
  TS(160); XTalkWait(task,"ProfRep;Sleep(750);\n");
  TS(161); XTalkWait(task,"HeapLogSizeRep;Sleep(750);\n");
  TS(162); if (mp_cnt>1) XTalkWait(task,"CPURep(TRUE);Sleep(750);\n");
  TS(163); if (DrvIsWritable(':')) XTalkWait(task,"ChkDsk;Sleep(750);\n");
  TS(164); if (DrvIsWritable(':')) {
    XTalkWait(task,"DrvView;\n");
    Sleep(750);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  TS(165); if (DrvIsWritable(':')) {
    XTalkWait(task,"DskView;\n");
    Sleep(250); //Short because ATARep takes time.
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  TS(166); XTalkWait(task,"ATARep;\npSleep(750);\n");
  TS(167); XTalkWait(task,
	"HashDepthRep(adam_task->hash_table);Sleep(750);\n");
  TS(168); XTalkWait(task,"Who;Sleep(750);\n");
  TS(169); XTalkWait(task,"DrvRep;Sleep(750);\n");
  TS(170); XTalkWait(task,"TaskRep;Sleep(750);\n");
  DeathWait(&task,TRUE);

  ProgressBarsRst;
}

OSTestSuite;
