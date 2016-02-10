$PL,1000$I64 ts_num;
U0 TSStart()
{
  ts_num=0;
}
U0 TS()
{
  progress4=ts_num++;
  progress4_max=167;
  *progress4_desc=0;
}
U0 TSEnd()
{
  if (ts_num!=progress4_max)
    ST_ERR_ST "Cnt %d should be %d.\n",ts_num,progress4_max;
  else
    "$$GREEN$$Test Suite Completed$$FG$$\n";
  ProgressBarsRst;
}

U0 DoSpritePlot()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpritePlot\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoElephant()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Elephant\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePlot3D()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpritePlot3D\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePut()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpritePut\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePutExt()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpritePutExt\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSpriteText()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpriteText\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSpriteRaw()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SpriteRaw\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCarry()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Carry\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoDirectives()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Directives\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoRandDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/RandDemo\";\n");
  XTalkWait(task," ");
  XTalkWait(task," ");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoBalloon()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Balloon\";\n");
  XTalk(task,"Exit;\n");
}

U0 DoEdCSprite()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/EdSprite\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoExtents()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Extents\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoDotNet()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/NetOfDots\";\n");
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSunMoon()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/SunMoon\";\n");
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoLines()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Lines\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoBounce()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Graphics/Bounce\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMsgLoop()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MsgLoop\";\n");
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoDoodle()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Doodle\";\n");
  PostMsgWait(task,MSG_IP_L_DOWN,10,10);
  PostMsgWait(task,MSG_IP_L_UP,100,200);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpeedLine()
{
  I64 i=PURPLE;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Speedline\";\n");
  PostMsgWait(task,MSG_IP_L_DOWN,10,10);
  PostMsgWait(task,MSG_IP_L_UP,100,200);
  PostMsg(task,MSG_IP_R_DOWN_UP,0,0);
  while (!TaskValidate(task->popup_task))
    Yield;
  while (i--)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(100);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_ESC,0);
  TaskWait(task);
  PostMsgWait(task,MSG_IP_L_DOWN,50,10);
  PostMsgWait(task,MSG_IP_L_UP,150,200);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoKeyBitMap()
{
  I64 i_sc=Char2ScanCode('i');
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/KeyBitMap\";\n");
  Sleep(50);
  LBts(kbd.down_bitmap,i_sc);
  Sleep(500);
  LBtr(kbd.down_bitmap,i_sc);
  Sleep(50);
  XTalk(task,"Exit;\n");
}

U0 DoMountain()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Graphics/Mountain\";\n");
  Sleep(1500);
  Kill(task);
}

U0 DoCoach()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Coach\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoBlot()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Blot\";\n");
  Sleep(1800);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoScreenCapture()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/ScreenCapture\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
  Del("/Home/ScreenShot.GRA.Z");
}

/*
U0 DoPullDownMenu()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/PullDownMenu\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

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
  XTalk(task,"Exit;\n");
}

U0 DoPalette()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Palette\";\n");
  Sleep(400);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(400);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoZoneOut()
{
  I64 i;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/ZoneOut\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<15;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(100);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoFlatTops()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/FlatTops\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  PostMsgWait(task,MSG_IP_R_DOWN_UP,RandI16%400+200,RandI16%300+150);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoWenceslas()
{
  I64 i;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/Wenceslas\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  for (i=0;i<25;i++) {
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(25);
  }
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTreeCheckers()
{
  I64 task_num;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/TreeCheckers\";\n");

  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  Sleep(500);

  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRainDrops()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/RainDrops\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoPixCollision()
{
  I64 w,h;
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Collision\";\n");
  w=task->pix_width>>1 +task->pix_left+task->scroll_x;
  h=task->pix_height>>1+task->pix_top +task->scroll_y;
  IPSet(w-35,h-35);
  AFSetIP(10,w+35,w+35);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoCtrlR1()
{
  I64 i,task_num=-1;
  CTask *task=User;
  XTalkWait(task,"//");
  PostMsg(task,MSG_KEY_DOWN_UP,CH_CTRLR,0);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<1;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  //Color
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Width
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<4;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<4;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Line
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<7;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
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
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<2;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<3;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  //Circle
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  task_num=task->popup_task->task_num;
  for (i=0;i<10;i++)
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  PostMsgWait(task,MSG_IP_L_DOWN,150,150);
  PostMsgWait(task,MSG_IP_L_UP,190,190);
  Sleep(250);
  PostMsg(task,MSG_IP_R_DOWN_UP,100,100);

  //Exit
  while (!TaskValidate(task->popup_task))
    Yield;
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN+SCF_CTRL);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  Sleep(1000);
  XTalk(task,"\nExit;\n");
}

U0 DoF2Macro()
{
  I64 i;
  U8 *ptr="\"Boo!\\n\";\n";
  CTask *task=User;
  while (TaskValidate(sys_macro_task))
    Yield;
  PostMsgWait(task,MSG_KEY_DOWN_UP,0,SC_F2);
  while (!TaskValidate(sys_macro_task))
    Yield;
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
  while (TaskValidate(sys_macro_task))
    Yield;

  Sleep(1000);
  XTalk(task,"\nExit;\n");
}

/*
U0 DoTicTacToe()
{
}
*/

U0 DoBox()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Box\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoShadow()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Shadow\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPredatorPrey()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Graphics/PredatorPrey\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do3DPoly()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/3DPoly\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRotateTank()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/RotateTank\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoScreenCodes()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/ScreenCodes\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPanText()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/PanText\";\n");
  XTalk(task,"Exit;\n");
}

U0 DoExtendedChars()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/ExtendedChars\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCharGraphics()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/CharGraphics\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoDateTime()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DateTime\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSubSwitch()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/SubSwitch\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoNullCase()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/NullCase\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoMaze()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Maze\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoFontEd()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/FontEd\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoTimeIns()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/TimeIns\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoWaterFowl()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Snd/WaterFowl\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Psalmody/Examples\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoMPPrint()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/MPPrint\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCartesian()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Cartesian\";\n");
  XTalkWait(task,"0.2*x`1.5\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoHanoi()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Hanoi\";\n");
  Sleep(3000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoLife()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Life\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoZing()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Zing\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoCommonAncestor()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/CommonAncestor\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPalindrome()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/Palindrome\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoMagicPairs()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MagicPairs\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoMPAdd()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/MPAdd\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLock()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/Lock\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoClouds()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/Clouds\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Slider\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/ScrollBars\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoRegistryDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/RegistryDemo\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoBugFlapper()
{
  I64 i;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/FlapBat\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
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
  XTalk(task,"Exit;\n");
}

U0 DoBattleLines()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/BattleLines\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  IPSet(430,300,,TRUE);
  AFSetIP(10,530,400);
  IPSet(,,,FALSE);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoBigGuns()
{
  I64 i;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/BigGuns\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  task->popup_task->user_data=Fs;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->user_data==Fs)
    Yield;
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoCollision()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Collision\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLoadTest()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/LoadTest\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoPrimes()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/Primes\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoRadixSort()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/RadixSort\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoPageTableEntries2()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures/Mem/PageTableEntries2\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoInterrupts()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MultiCore/Interrupts\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoStkGrow()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/StkGrow\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoMemDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/MemDemo\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSpan()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Apps/Span/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoStrut()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Strut/Run\";\n");

  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  IPSet(200,200,,TRUE);
  PostMsgWait(task,MSG_IP_L_DOWN,0,0);
  WinMgrSync(2);

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
  XTalk(task,"Exit;\n");
}

U0 DoRawHide()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/RawHide\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoAfterEgypt()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Apps/AfterEgypt/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  Sleep(600); //It flushes messages!
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  while (TaskValidate(task->popup_task))
    Yield;
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
  XTalk(task,"Exit;\n");
}

U0 DoShading()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Shading\";\n");
  Sleep(1500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPoleZeros()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Graphics/PoleZeros\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoWhap()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Whap\";\n");
  IPSet(300,200);
  Sleep(500);
  AFSetIP(3,350,300);
  Sleep(500);
  AFSetIP(3,450,200);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoGrid()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Graphics/Grid\";\n");
  IPSet(200,200);
  Sleep(150);
  AFSetIP(2,400,400);
  AFSetIP(2,200,400);
  AFSetIP(2,200,200);
  IPSet(,,,TRUE);
  Sleep(50);
  IPSet(,,,FALSE);
  DocBottom(DocPut(task));
  XTalk(task,"Exit;\n");
}

U0 DoPick()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Pick\";\n");
  IPSet(200,200);
  Sleep(500);
  IPSet(,,,TRUE);
  AFSetIP(4,400,400);
  IPSet(,,,FALSE);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPick3D()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Pick3D\";\n");
  IPSet(200,200);
  Sleep(500);
  IPSet(,,,TRUE);
  AFSetIP(4,400,400);
  IPSet(,,,FALSE);
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTransform()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Graphics/Transform\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/ToTheFront\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoGrModels()
{
  CTask *task=User;
  I64 i,j,task_num=-1;
  XTalkWait(task,"#include \"::/Apps/GrModels/Run\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  for (i=0;i<4;i++) {
    while (!TaskValidate(task->popup_task) ||
	  task->popup_task->task_num==task_num)
      Yield;
    TaskWait(task->popup_task);
    task_num=task->popup_task->task_num;
    for (j=0;j<=i;j++)
      PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'n',0);
  XTalk(task,"Exit;\n");
}

U0 DoKeepAway()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Apps/KeepAway/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  IPSet(100,100);
  Sleep(1500);
  PostMsgWait(task,MSG_IP_R_DOWN_UP,ip.pos.x,ip.pos.y);
  Sleep(1000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoRocketScience()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/RocketScience\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2000);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMassSpring()
{
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/Games/MassSpring\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
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
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Games/Whap\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Psalmody\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPhoneNumWords()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/PhoneNumWords\";\n");
  XTalkWait(task,"702-254-4223\n\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSuggestedSpelling()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/SuggestSpelling\";\n");
  XTalkWait(task,"effecient\n\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoWordSearch()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/WordSearch\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoFPrintF()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/FPrintF\";\n");
  XTalkWait(task,"::/Demo/Dsk/FPrintF.TXT");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  XTalkWait(task,"Type(\"::/Demo/Dsk/FPrintF.TXT\");\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_CTRLO,0);
  XTalkWait(task,"Del(\"::/Demo/Dsk/FPrintF.TXT\");\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoSerializeTree()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/SerializeTree\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/DataBase\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRaw()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/Raw\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoUnusedSpaceRep()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/UnusedSpaceRep\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoBlkDevRep()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Dsk/BlkDevRep\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoMiniCompiler()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures/MiniCompiler\";\n");
  XTalkWait(task,"1+2*(3+4)\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMiniGrLib()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures/MiniGrLib\";\n");
  XTalk(task,"Exit;\n");
}

U0 DoWebLog()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/WebLogExample/WebLogRep\";\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,'\n',0);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoReverseFile()
{
  CTask *task=User;
  Del("::/Demo/ReverseFileExample/PoemForward.TXT.Z");
  XTalkWait(task,"#include \"::/Demo/ReverseFileExample/Reverse\";\n");
  XTalkWait(task,"Type(\"::/Demo/ReverseFileExample/PoemForeward.TXT.Z\");\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSortFile()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/SortFileExample/F64FileGen\";\n");
  XTalkWait(task,"#include \"::/Demo/SortFileExample/F64FileSort\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoToHtmlToTXT()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/ToHtmlToTXTExample/HtmlGen\";\n");
  XTalkWait(task,"#include \"::/Demo/ToHtmlToTXTExample/TXTGen\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/TimeClock\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoLogic()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Apps/Logic/Run\";\n");
  XTalkWait(task,"NAND\nNOR\n\n0x100\n0xF0\n0xCC\n0xAA\n\n0x12\n\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Lectures\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/AutoFile\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPrint()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Print\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoSubIntAccess()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/SubIntAccess\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoParenWarn()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/ParenWarn\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCompileDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/CompileDemo\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoClassMeta()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/ClassMeta\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoNumBible()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/NumBible\";\n");
  Del("::/Home/NumBible.TXT.Z");
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoClickCallBack()
{
  I64 i;
  CTask *task=User;
  XTalk(task,"#include \"::/Demo/DolDoc/ClickCallBack\";\n");
  for (i=0;i<3;i++) {
    while (!TaskValidate(task->popup_task))
      Yield;
    TaskWait(task->popup_task);
    PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(250);
  }
  Sleep(500);
  XTalk(task,"Exit;\n");
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
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  Sleep(500);
  PostMsgWait(sys_focus_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  Sleep(500);
  PostMsgWait(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
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
  XTalk(task,"Exit;\n");
}

U0 DoTreeDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/TreeDemo\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoExceptions()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Exceptions\";\n");
  XTalkWait(task,"yy");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoTextDemo()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/TextDemo\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCursorMove()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/CursorMove\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoDemoDoc()
{
  CTask *task=User;
  XTalkWait(task,"Ed(\"::/Demo/DolDoc/DemoDoc.TXT\");\n");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_CTRLG,0);
  XTalkWait(task,"100");
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Spy\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoFileRead()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/FileRead\";\n");
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoData()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/Data\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoDefineStr()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/DefineStr\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoCallBack()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/CallBack\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoDefine()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Define\";\n");
  XTalkWait(task,"YS\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/DolDoc/UnusedDefine\";\n");
  Sleep(500);
  PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoAsmHelloWorld()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmHelloWorld\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoAsmAndC1()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmAndC1\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoAsmAndC2()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmAndC2\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoAsmAndC3()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/AsmAndC3\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoBuzzFizz()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/BuzzFizz\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoMulByHand()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/MulByHand\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoDivByHand()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/DivByHand\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoPutDec()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/Asm/PutDec\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoGlblVars()
{
  CTask *task=User;
  XTalkWait(task,"#include \"::/Demo/GlblVars\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoReps()
{
  CTask *task=User;
  TS; XTalkWait(task,"DocMax;Prof;HeapLog(ON);\n");
  TS; XTalkWait(task,"PCIRep;Sleep(750);\n");
  TS; XTalkWait(task,"MemBIOSRep;Sleep(750);\n");
  TS; XTalkWait(task,"MemRep;Sleep(750);\n");
  TS; XTalkWait(task,"ZipRep;Sleep(750);\n");
  TS; XTalkWait(task,"ProfRep;Sleep(750);\n");
  TS; XTalkWait(task,"HeapLogSizeRep;Sleep(750);\n");
  TS; if (DrvIsWritable(':')) XTalkWait(task,"ChkDsk;Sleep(750);\n");
  TS; if (DrvIsWritable(':')) {
    XTalkWait(task,"DrvView;\n");
    Sleep(750);
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  TS; if (DrvIsWritable(':')) {
    XTalkWait(task,"DskView;\n");
    Sleep(250); //Short because ATARep takes time.
    PostMsgWait(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  TS; XTalkWait(task,"ATARep;\npSleep(750);\n");
  TS; XTalkWait(task,"HashDepthRep(adam_task->hash_table);Sleep(750);\n");
  TS; XTalkWait(task,"Who;Sleep(750);\n");
  TS; XTalkWait(task,"DrvRep;Sleep(750);\n");
  TS; XTalkWait(task,"TaskRep;Sleep(750);\n");
  XTalk(task,"Exit;\n");
}

U0 OSTestSuite()
{
  TSStart;
  TS; DoBlackDiamond;
  TS; if (DrvIsWritable(':')) DoEagleDive;
  TS; DoFlatTops;
  TS; DoDunGen;
  TS; DoFPS;
  TS; DoZoneOut;
  TS; DoVaroom;
  TS; DoRocket;
  TS; DoRocketScience;
  TS; DoBattleLines;
  TS; DoBigGuns;
  TS; DoBugFlapper;
  TS; DoTimeOut;
  TS; if (!sys_heap_init_flag) /* FloodFill */ DoSpan;
  TS; DoStrut;
  TS; DoRawHide;
  TS; DoKeepAway;
  TS; DoAfterEgypt;
  TS; DoXCaliber;
  TS; DoWenceslas;
  TS; DoBomberGolf;
  TS; DoTheDead;
  TS; DoTreeCheckers;
  TS; DoMPAdd;
  TS; DoPrimes;
  TS; if (FileFind(BIBLE_FILENAME)) DoPalindrome;
  TS; DoClouds;
  TS; if (mp_cnt>1) DoRadixSort;
  TS; if (mp_cnt>1) DoLoadTest;
  TS; if (mp_cnt>1) DoMPPrint;
  TS; if (mp_cnt>1) DoLock;
  TS; if (mp_cnt>1) DoInterrupts;
  TS; DoSpritePlot;
  TS; DoElephant;
  TS; DoSpritePlot3D;
  TS; DoSpritePut;
  TS; DoSpritePutExt;
  TS; DoSpriteText;
  TS; DoSpriteRaw;
  TS; DoEdCSprite;
  TS; DoBalloon;
  TS; DoCarry;
  TS; DoDirectives;
  TS; DoRandDemo;
  TS; DoExtents;
  TS; DoDotNet;
  TS; DoSunMoon;
  TS; DoLowPassFilter;
  TS; DoMathAudioDemo;
  TS; DoLines;
  TS; DoBounce;
  TS; DoMsgLoop;
  TS; DoASCIIOrgan;
  TS; DoMorseCode;
  TS; DoPixCollision;
  TS; DoDoodle;
  TS; DoMassSpring;
  TS; DoSpeedLine;
  TS; DoKeyBitMap;
  TS; DoMountain;
  TS; DoCoach;
  TS; DoBSpline;
  TS; DoGrModels;
  TS; DoBlot;
  TS; if (DrvIsWritable(':')) DoScreenCapture;
  TS; DoGrid;
  TS; DoPick;
  TS; DoPick3D;
  TS; DoWhap;
  TS; DoPalette;
  TS; DoPrint;
  TS; DoShading;
  TS; DoRotateTank;
  TS; Do3DPoly;
  TS; DoBox;
  TS; DoSymmetry;
  TS; DoShadow;
  TS; DoTransform;
  TS; DoPredatorPrey;
  TS; DoPoleZeros;
  TS; DoDigits;
  TS; DoStadium;
  TS; DoElephantWalk;
  TS; DoHalogen;
  TS; DoMaze;
  TS; DoFontEd;
  TS; DoLattice;
  TS; DoCtrlR1;
  TS; DoF2Macro;
  TS; DoSubIntAccess;
  TS; DoDemoDoc;
  TS; DoTreeDemo;
  TS; DoTextDemo;
  TS; DoCursorMove;
  TS; DoMiniCompiler;
  TS; DoMiniGrLib;
  TS; DoPhoneNumWords;
  TS; DoUnusedSpaceRep;
  TS; DoBlkDevRep;
  TS; DoLastClass;
  TS; if (DrvIsWritable(':')) DoFPrintF;
  TS; DoSerializeTree;
  TS; DoExceptions;
  TS; DoScreenCodes;
  TS; DoExtendedChars;
  TS; DoPanText;
  TS; DoCharGraphics;
  TS; DoDateTime;
  TS; DoSubSwitch;
  TS; DoNullCase;
  TS; DoMagicPairs;
  TS; DoHanoi;
  TS; DoSquirt;
  TS; DoCommonAncestor;
  TS; DoCartesian;
  TS; DoRainDrops;
  TS; DoCollision;
  TS; DoLogic;
  TS; DoCompileDemo;
  TS; DoPrompt;
  TS; if (DrvIsWritable(':')) DoWebLog;
  TS; if (DrvIsWritable(':')) DoReverseFile;
  TS; if (DrvIsWritable(':')) DoSortFile;
  TS; if (DrvIsWritable(':')) DoToHtmlToTXT;
  TS; if (DrvIsWritable(':')) DoRegistryDemo;
  TS; DoDefine;
  TS; DoGlblVars;
  TS; DoFileRead;
  TS; DoParenWarn;
  TS; DoDefineStr;
  TS; DoData;
  TS; DoCallBack;
  TS; DoClassMeta;
  TS; if (DrvIsWritable(':')) DoNumBible;
  TS; DoForm;
  TS; DoClickCallBack;
  TS; DoMenuButton;
  TS; DoMenuSprite;
  TS; DoSuggestedSpelling;
  TS; DoWordSearch;
  TS; DoStkGrow;
  TS; DoMemDemo;
  TS; DoWaterFowl;
  TS; DoAsmHelloWorld;
  TS; DoAsmAndC1;
  TS; DoAsmAndC2;
  TS; DoAsmAndC3;
  TS; DoMulByHand;
  TS; DoDivByHand;
  TS; DoBuzzFizz;
  TS; DoPutDec;
  TS; DoPageTableEntries2;
  DoReps;
  TSEnd;
}

OSTestSuite;

//$LK,"::/Demo/Dsk/DskRaw.CPP"$
//$LK,"::/Demo/Graphics/InputPointer.CPP"$
