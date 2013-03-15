I64 ts_num;
U0 TSStart()
{
  ts_num=0;
}
U0 TS()
{
  progress4=ts_num++;
  progress4_max=156;
  *progress4_desc=0;
}
U0 TSEnd()
{
  if (ts_num!=progress4_max)
    "$$FG,RED$$Progress4:%d$$FG$$\n",progress4;
  else
    "$$FG,GREEN$$Test Suite Completed$$FG$$\n";
  ProgressBarsRst;
}

U0 DoSpritePlot()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SpritePlot\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePlot3D()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SpritePlot3D\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePut()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SpritePut\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePutExt()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SpritePutExt\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoSpriteText()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SpriteText\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoEdCSprite()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/EdCSprite\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoExtents()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Extents\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}


U0 DoDotNet()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/DotNet\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLowPassFilter()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/LowPassFilter\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoMathAudioDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/MathAudioDemo\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);

  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLines()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Lines\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoBounce()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Graphics/Bounce\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMsgLoop()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MsgLoop\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoASCIIOrgan()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/ASCIIOrgan\";\n");
  Sleep(100);
  SendMsg(task,MSG_KEY_DOWN_UP,'A',0);
  Sleep(200);
  SendMsg(task,MSG_KEY_DOWN_UP,'B',0);
  Sleep(200);
  SendMsg(task,MSG_KEY_DOWN_UP,'C',0);
  Sleep(200);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoDoodle()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Doodle\";\n");
  SendMsg(task,MSG_IP_L_DOWN,10,10);
  SendMsg(task,MSG_IP_L_UP,100,200);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpeedLine()
{
  I64 i=PURPLE;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Speedline\";\n");
  SendMsg(task,MSG_IP_L_DOWN,10,10);
  SendMsg(task,MSG_IP_L_UP,100,200);
  PostMsg(task,MSG_IP_R_DOWN_UP,0,0);
  while (!TaskValidate(task->popup_task))
    Yield;
  while (i--)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(100);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_ESC,0);
  TaskWait(task);
  SendMsg(task,MSG_IP_L_DOWN,50,10);
  SendMsg(task,MSG_IP_L_UP,150,200);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoKeyBitMap()
{
  I64 i_sc=A2ScanCode('i');
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/KeyBitMap\" Exit;\n");
  Sleep(20);
  LBts(key_down_bitmap,i_sc);
  Sleep(500);
  LBtr(key_down_bitmap,i_sc);
}

U0 DoGolf()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Golf\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMountain()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Graphics/Mountain\";\n");
  Sleep(1500);
  Kill(task);
}

U0 DoCoach()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Coach\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoDigits()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Digits\";\n");
  Sleep(100);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,'1',0);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCPUPlot()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/CPUPlot\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoDither()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Dither\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSymmetry()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Symmetry\";\n");
  SendMsg(task,MSG_IP_R_DOWN,100,100);
  SendMsg(task,MSG_IP_R_UP,200,200);
  Sleep(100);
  SendMsg(task,MSG_IP_L_DOWN,10,10);
  SendMsg(task,MSG_IP_L_UP,100,200);
  Sleep(250);
  SendMsg(task,MSG_IP_L_DOWN,100,200);
  SendMsg(task,MSG_IP_L_UP,400,400);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoBSpline()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/BSpline\";\n");
  SendMsg(task,MSG_IP_L_DOWN_UP,50,50);
  SendMsg(task,MSG_IP_L_DOWN_UP,300,100);
  SendMsg(task,MSG_IP_L_DOWN_UP,150,300);
  SendMsg(task,MSG_IP_R_DOWN_UP,0,0);
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoBlot()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Blot\";\n");
  Sleep(1800);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCaptureScreen()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/CaptureScreen\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
  Del("HOME/ScreenShot.GRA.Z");
}

/*
U0 DoPullDownMenu()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/PullDownMenu\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoStadium()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Stadium/Stadium\";\n");
  Sleep(50);
  SendMsg(task,MSG_IP_L_DOWN_UP,100,10);
  Sleep(300);
  SendMsg(task,MSG_IP_L_DOWN_UP,320,20);
  Sleep(300);
  SendMsg(task,MSG_IP_L_DOWN_UP,520,10);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPalette()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Palette\";\n");
  Sleep(400);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(400);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoHalogen()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Halogen\";\n");
  SendMsg(task,MSG_KEY_DOWN,0,SC_CURSOR_UP);
  Sleep(1000);
  SendMsg(task,MSG_KEY_UP,0,SC_CURSOR_UP);
  SendMsg(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);
  Sleep(333);
  SendMsg(task,MSG_KEY_UP,0,SC_CURSOR_RIGHT);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTheDead()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/TheDead\";\n");
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(50);
  }
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(50);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoBomber()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Bomber\";\n");
  for (i=0;i<7;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  for (i=0;i<7;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(200);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoZoneOut()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/ZoneOut\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(100);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoVaroom()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/Varoom\";\n");
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(50);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRocks()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/Rocks\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SendMsg(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);
  Sleep(300);
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    Sleep(100);
  }
  SendMsg(task,MSG_KEY_UP,0,SC_CURSOR_RIGHT);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoNavy()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/Navy\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SendMsg(task,MSG_IP_R_DOWN_UP,RandI16%400+200,RandI16%300+150);
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoWenceslas()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/Wenceslas\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  for (i=0;i<25;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(25);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoChess()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/Chess\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(100);
  for (i=0;i<8;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(25);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  SendMsg(task,MSG_KEY_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoChessNet()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/ChessNet\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,'\n',0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRainDrops()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/RainDrops\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMorseCode()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/MorseCode\";\n");
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
  Sleep(1350);

  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoPixelCollision()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/PixelCollision\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBlackDiamond()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/BlackDiamond\";\n");
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(75);
  }
  for (i=0;i<12;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(75);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCtrlR1()
{
  I64 i,task_num=-1;
  CTask *task=SpawnUser;
  XTalkWait(task,"//");
  PostMsg(task,MSG_KEY_DOWN_UP,CH_CTRLR,0);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;

//Tag text
  PostMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);

//Color
  while (!TaskValidate(task->popup_task))
    Yield;
  task_num=task->popup_task->task_num;
  TaskWait(task->popup_task);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Width
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<4;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Line
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<6;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SendMsg(task,MSG_IP_L_DOWN,150,150);
  SendMsg(task,MSG_IP_L_UP,150,95);
  Sleep(250);
  SendMsg(task,MSG_IP_L_DOWN,150,150);
  SendMsg(task,MSG_IP_L_UP,190,190);
  Sleep(250);
  SendMsg(task,MSG_IP_L_DOWN,150,150);
  SendMsg(task,MSG_IP_L_UP,110,190);
  Sleep(250);
  PostMsg(task,MSG_IP_R_DOWN_UP,100,100);

//Color
  while (!TaskValidate(task->popup_task))
    Yield;
  task_num=task->popup_task->task_num;
  TaskWait(task->popup_task);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<3;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task_num=task->popup_task->task_num;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Circle
  while (!TaskValidate(task->popup_task)||task->popup_task->task_num==task_num)
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<9;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SendMsg(task,MSG_IP_L_DOWN,150,150);
  SendMsg(task,MSG_IP_L_UP,190,190);
  Sleep(250);
  PostMsg(task,MSG_IP_R_DOWN_UP,100,100);

//Exit
  while (!TaskValidate(task->popup_task))
    Yield;
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN+SCF_CTRL);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  Sleep(1000);
  XTalk(task,"\nExit;\n");
}

/*
U0 DoTicTacToe()
{
}
*/

U0 DoCircuit()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Graphics/Circuit\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SetIP(300,350);
  SendMsg(task,MSG_KEY_DOWN_UP,'r',0);
  SetIP(400,350);
  SendMsg(task,MSG_KEY_DOWN_UP,'l',0);
  SetIP(350,400);
  SendMsg(task,MSG_KEY_DOWN_UP,'c',0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  SendMsg(task,MSG_IP_L_DOWN,350,385);
  SendMsg(task,MSG_IP_L_UP,390,350);
  Sleep(500);
  SendMsg(task,MSG_IP_L_DOWN,350,385);
  SendMsg(task,MSG_IP_L_UP,310,350);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpaceShip()
{
  CTask *task=SpawnUser;
  SetIP(300,300);
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/SpaceShip\";\n");
  Sleep(150);
  AFSetIP(3,125,125);
  SetIP(,,,TRUE);
  Sleep(50);
  SetIP(,,,FALSE);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoBox()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Box\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoShadow()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Shadow\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPredatorPrey()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Graphics/PredatorPrey\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do3DPoly()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/3DPoly\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLogo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Logo\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRotateTank()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/RotateTank\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoPic2ASCII()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Pic2ASCII\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoScreenCodes()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/ScreenCodes\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoExtendedChars()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/ExtendedChars\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoNeatTrick()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/NeatTrick\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCharGraphics()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/CharGraphics\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoMaze()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Maze\";\n");
  Sleep(750);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoFontEd()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/FontEd\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTimeIns()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/TimeIns\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoWaterFowl()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/WaterFowl\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/MusicOrgan/Examples\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoMPPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/MPPrintF\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoLayers3DJob()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Layers3DJob\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCartesian()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Cartesian\";\n");
  XTalkWait(task,"0.2*x`1.5\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoHanoi()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Hanoi\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoLife()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Life\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoZing()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Zing\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoFlowers()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Flowers\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCommonAncestor()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/CommonAncestor\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPalindrome()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Palindrome\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoMagicPairs()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MagicPairs\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoMPAdd()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/MPAdd\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLock()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Lock\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoLayersSpawn()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/LayersSpawn\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoClouds()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Clouds\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoNoise()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Noise\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/SliderCtrl\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBling()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Bling\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/ScrollBars\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/RegistryDemo\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBirdLand()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/BirdLand\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  for (i=0;i<4;i++) {
    SendMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
    Sleep(100);
    SendMsg(task,MSG_KEY_UP,CH_SPACE,0);
    Sleep(100);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}


U0 DoMissileDefense()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/MissileDefense\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  task->popup_task->user_data0=Fs;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->user_data0==Fs)
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  for (i=0;i<10;i++) {
    SetIP(RandU16%task->win_pixel_width,RandU16%task->win_pixel_height);
    SendMsg(task,MSG_KEY_DOWN_UP,RandU16%3+'1',0);
    Sleep(200);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoWarGame()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/WarGame\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SetIP(430,300,,TRUE);
  AFSetIP(10,530,400);
  SetIP(,,,FALSE);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoBigGuns()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/BigGuns\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  task->popup_task->user_data0=Fs;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task)||task->popup_task->user_data0==Fs)
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(200);
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoDunGen()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/DunGen\";\n");
  for (i=0;i<10;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  for (i=0;i<12;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(100);
  }
  for (i=0;i<6;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTimeOut()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/Timeout/Run\";\n");
  Sleep(200);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  for (i=0;i<3;i++) {
    SendMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
    LBts(key_down_bitmap,SC_CURSOR_RIGHT);
    Sleep(200);
    SendMsg(task,MSG_KEY_UP,CH_SPACE,0);
    LBtr(key_down_bitmap,SC_CURSOR_RIGHT);
    Sleep(200);
    SendMsg(task,MSG_KEY_DOWN,CH_SPACE,0);
    LBts(key_down_bitmap,SC_CURSOR_LEFT);
    Sleep(200);
    SendMsg(task,MSG_KEY_UP,CH_SPACE,0);
    LBtr(key_down_bitmap,SC_CURSOR_LEFT);
    Sleep(200);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCollision()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Collision\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLoadTest()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/LoadTest\";\n");
  Sleep(2000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoPrimes()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Primes\";\n");
  Sleep(2500);
  XTalk(task,"Exit;\n");
}

U0 DoSort()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Sort\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoRadixSort()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/RadixSort\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoPageTableEntries2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Lectures/PageTableEntries2\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoInterrupts()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/MultiCore/Interrupts\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoStkGrow()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/StkGrow\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoPilgrims()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Apps/Pilgrims/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpan()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Apps/Span/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRawHide()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/RawHide\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoAfterEgypt()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Apps/AfterEgypt/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  Sleep(600); //It flushes messages!
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  while (TaskValidate(task->popup_task))
    Yield;
  TaskWait(task);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Camp
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0); //Map
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Map
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Camp
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0); //Main Menu
  XTalk(task,"Exit;\n");
}

U0 DoShading()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Shading\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPoleZeros()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Graphics/PoleZeros\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoWhap()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Whap\";\n");
  SetIP(300,200);
  Sleep(500);
  AFSetIP(3,350,300);
  Sleep(500);
  AFSetIP(3,450,200);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPick()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Pick\";\n");
  SetIP(200,200);
  Sleep(500);
  SetIP(,,,TRUE);
  AFSetIP(4,400,400);
  SetIP(,,,FALSE);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPick3D()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Pick3D\";\n");
  SetIP(200,200);
  Sleep(500);
  SetIP(,,,TRUE);
  AFSetIP(4,400,400);
  SetIP(,,,FALSE);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Transform\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/Tanks\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoGrModels()
{
  CTask *task=SpawnUser;
  I64 i,j,task_num=-1;
  XTalkWait(task,"#include \"::/TempleOS/Apps/GrModels/Run\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  for (i=0;i<4;i++) {
    while (!TaskValidate(task->popup_task)||
	   task->popup_task->task_num==task_num)
      Yield;
    TaskWait(task->popup_task);
    task_num=task->popup_task->task_num;
    for (j=0;j<=i;j++)
      SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,'n',0);
  XTalk(task,"Exit;\n");
}

U0 DoKeepAway()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Apps/KeepAway/Run\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  SetIP(100,100);
  Sleep(1500);
  SendMsg(task,MSG_IP_R_DOWN_UP,ipx,ipy);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCamp()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Graphics/Camp\";\n");
  Sleep(1250);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRocket()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Rocket\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
  Bts(key_down_bitmap,SC_CURSOR_UP);
  Sleep(1000);
  Btr(key_down_bitmap,SC_CURSOR_UP);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  Bts(key_down_bitmap,SC_CURSOR_RIGHT);
  Sleep(1000);
  Btr(key_down_bitmap,SC_CURSOR_RIGHT);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRocketScience()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/RocketScience\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(750);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(2000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMassSpring()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/MassSpring\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  SendMsg(task,MSG_IP_L_DOWN_UP,50,50);  //#1
  SendMsg(task,MSG_IP_L_DOWN_UP,50,200); //#2
  SendMsg(task,MSG_IP_L_DOWN_UP,210,50); //#3
  SendMsg(task,MSG_IP_L_DOWN_UP,310,200);//#4
  Sleep(500);
  SendMsg(task,MSG_IP_R_DOWN,50,50); //1-2
  SendMsg(task,MSG_IP_R_UP,50,200);
  Sleep(500);
  SendMsg(task,MSG_IP_R_DOWN,210,50);//3-4
  SendMsg(task,MSG_IP_R_UP,310,200);
  Sleep(500);
  SendMsg(task,MSG_IP_R_DOWN,50,200);//2-4
  SendMsg(task,MSG_IP_R_UP,310,200);
  Sleep(500);
  SendMsg(task,MSG_IP_R_DOWN,50,50); //1-3
  SendMsg(task,MSG_IP_R_UP,210,50);
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Whap\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoSquirt()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GameStarters/Squirt\";\n");
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_LEFT);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(100);
  }
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(100);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoXCaliber()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/X-Caliber/Run\";\n");
  for (i=0;i<5;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SCF_SHIFT|SC_CURSOR_UP);
    SendMsg(task,MSG_KEY_DOWN_UP,0,SCF_SHIFT|SC_CURSOR_RIGHT);
  }
  for (i=0;i<10;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
    SendMsg(task,MSG_KEY_DOWN,0,SC_CURSOR_RIGHT,0);
    Sleep(100);
    SendMsg(task,MSG_KEY_UP,0,SC_CURSOR_RIGHT,0);
    Sleep(50);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoFPS()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/GameStarters/FPS\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  for (i=0;i<15;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<8;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(50);
  }
  for (i=0;i<9;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<6;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
    Sleep(50);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoEagleDive()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"Cd(\"::/TempleOS/Demo/GameStarters\");Del(\"EagleDive.BIN.Z\");\n");
  XTalkWait(task,"Cmp(\"EagleDive.CPP\");\n");
  Sleep(500);
  XTalkWait(task,"Load(\"EagleDive\");\n");
  for (i=0;i<10;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_UP);
    Sleep(50);
  }
  for (i=0;i<10;i++) {
    SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    Sleep(50);
  }
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalkWait(task,"Del(\"::/TempleOS/Demo/GameStarters/EagleDive.BIN.Z\");\n");
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/Span\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/MusicOrgan\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPhoneNumWords()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/PhoneNumWords\";\n");
  XTalkWait(task,"702-254-4223\n\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoSuggestedSpelling()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/SuggestSpelling\";\n");
  XTalkWait(task,"effecient\n\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoWordSearch()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/WordSearch\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Dsk/DataBase\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoOrderedIO()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Dsk/OrderedIO\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 DoFPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Dsk/FPrintF\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRaw()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Dsk/Raw\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoUnusedSpaceRep()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Dsk/UnusedSpaceRep\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoMiniCompiler()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Lectures/MiniCompiler\";\n");
  XTalkWait(task,"1+2*(3+4)\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMiniGrLib()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/TempleOS/Demo/Lectures/MiniGrLib\";Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoWebLog()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/WebLogExample/WebLogRep\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,'\n',0);
  SendMsg(task,MSG_KEY_DOWN_UP,'\n',0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/TimeClock\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoLogic()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Apps/Logic/Run\";\n");
  XTalkWait(task,"NAND\nNOR\n\n0x100\n0xF0\n0xCC\n0xAA\n\n0x12\n\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Lectures\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/OldSchool\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/AutoFile\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/PrintF\";\n");
  Sleep(2000);
  XTalk(task,"Exit;\n");
}

U0 DoDateTime()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/DateTime\";\n");
  Sleep(2500);
  XTalk(task,"Exit;\n");
}

U0 DoSubIntAccess()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/SubIntAccess\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoParenWarn()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/ParenWarn\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCompileDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/CompileDemo\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoPrompt()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Prompt\";\n");
  XTalkWait(task,"1+2*3<<4\n");
  XTalkWait(task,"1.0+2*3`2\n");
  XTalkWait(task,"Terry Davis\n");
  XTalkWait(task,"*-1\n");
  Sleep(2000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoClassMeta()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/ClassMeta\";\n");
  Sleep(2000);
  XTalk(task,"Exit;\n");
}

U0 DoForm()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/Form\";\n");
  Sleep(250);
  XTalk(task,"54321");
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SCF_CTRL|SC_CURSOR_LEFT);
  XTalk(task,"77777");
  Sleep(250);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  Sleep(250);
  XTalk(task,"MyName");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoMenuButton()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/MenuButton\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(500);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  Sleep(500);
  SendMsg(sys_focus_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_RIGHT);
  Sleep(500);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (TaskValidate(task->popup_task))
    Yield;
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMenuSprite()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/MenuSprite\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  SendMsg(task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN,0);
  Sleep(50);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTreeDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/TreeDemo\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoExceptions()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Exceptions\";\n");
  XTalkWait(task,"yy");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoTextDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/TextDemo\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoCursorMove()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/CursorMove\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoDemoDoc()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"Ed(\"::/TempleOS/Demo/Doc/DemoDoc.TXT\");\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_CTRLG,0);
  XTalkWait(task,"100");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Spy\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoFileRead()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/FileRead\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoData()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/Data\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoDefineStr()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/DefineStr\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCallBack()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/CallBack\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCallBackAux2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/CallBackAux2\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoDefine()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Define\";\n");
  XTalkWait(task,"YS\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Doc/UnusedDefine\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoAsmHelloWorld()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/AsmHelloWorld\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoAsmAndC()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/AsmAndC\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoAsmAndC2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/AsmAndC2\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoBuzzFizz()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/BuzzFizz\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoMulByHand()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/MulByHand\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoDivByHand()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Asm/DivByHand\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoGlblVars()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/GlblVars\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/WinMgr2\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/PolyPhonic\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/SndScope\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/Synth\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/Record\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/TempleOS/Demo/Snd/Echo\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/


U0 DoReps()
{
  CTask *task=SpawnUser;
TS; XTalkWait(task,"DocMax;ProfAll;HeapLog(ON);\n");
TS; XTalkWait(task,"PCIRep;Sleep(1000);\n");
TS; XTalkWait(task,"MemBIOSRep;Sleep(1000);\n");
TS; XTalkWait(task,"MemRep;Sleep(1000);\n");
TS; XTalkWait(task,"ProfRep;Sleep(1000);\n");
TS; XTalkWait(task,"HeapLogRep; HeapLog(OFF);Sleep(1000);\n");
TS; XTalkWait(task,"BlkDevRep;Sleep(1000);\n");
TS; XTalkWait(task,"ChkDsk;Sleep(1000);\n");
TS; XTalkWait(task,"DrvView;\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalkWait(task,"DskView;\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
TS; XTalkWait(task,"ATARep;\npSleep(1000);\n");
TS; XTalkWait(task,"HashDepthRep(adam_task->hash_table);Sleep(1000);\n");
TS; XTalkWait(task,"HAllRep;Sleep(1000);\n");
TS; XTalkWait(task,"PrtRep;Sleep(1000);\n");
TS; XTalkWait(task,"TaskRep;Sleep(1000);\n");
  XTalk(task,"Exit;\n");
}

U0 OSTestSuite()
{
TSStart;
TS; DoSpaceShip;
TS; DoBlackDiamond;
TS; if (IsDrvWritable(':')) DoEagleDive;
TS; DoRocks;
TS; DoNavy;
TS; DoDunGen;
TS; DoFPS;
TS; DoZoneOut;
TS; DoVaroom;
TS; DoRocket;
TS; DoRocketScience;
TS; DoMissileDefense;
TS; DoWarGame;
TS; DoBigGuns;
TS; DoBirdLand;
TS; DoTimeOut;
TS; DoPilgrims;
TS; if (!sys_heap_init_flag) /* FloodFill */ DoSpan;
TS; DoRawHide;
TS; DoKeepAway;
TS; DoAfterEgypt;
TS; DoXCaliber;
TS; DoWenceslas;
TS; DoBomber;
TS; DoTheDead;
TS; DoChess;
TS; DoChessNet;
TS; DoMPAdd;
TS; DoPrimes;
TS; DoPalindrome;
TS; DoClouds;
TS; DoNoise;
TS; if (mp_cnt>1) DoRadixSort;
TS; if (mp_cnt>1) DoLoadTest;
TS; if (mp_cnt>1) DoMPPrintF;
TS; if (mp_cnt>1) DoLayers3DJob;
TS; if (mp_cnt>1) DoLock;
TS; if (mp_cnt>1) DoLayersSpawn;
TS; if (mp_cnt>1) DoInterrupts;
TS; if (mp_cnt>1) DoSort;
TS; DoSpritePlot;
TS; DoSpritePlot3D;
TS; DoSpritePut;
TS; DoSpritePutExt;
TS; DoSpriteText;
TS; DoEdCSprite;
TS; DoExtents;
TS; DoDotNet;
TS; DoLowPassFilter;
TS; DoMathAudioDemo;
TS; DoLines;
TS; if (!IsSysDeterministic) DoBounce;
TS; DoMsgLoop;
TS; DoASCIIOrgan;
TS; DoMorseCode;
TS; DoDoodle;
TS; DoMassSpring;
TS; DoSpeedLine;
TS; DoKeyBitMap;
TS; DoGolf;
TS; DoMountain;
TS; DoCoach;
TS; DoCPUPlot;
TS; DoDither;
TS; DoBSpline;
TS; DoGrModels;
TS; DoBlot;
TS; if (IsDrvWritable(':')) DoCaptureScreen;
TS; DoPick;
TS; DoPick3D;
TS; DoWhap;
TS; DoPalette;
TS; DoPrintF;
TS; DoCamp;
TS; DoShading;
TS; DoRotateTank;
TS; Do3DPoly;
TS; DoBox;
TS; DoSymmetry;
TS; DoShadow;
TS; DoPredatorPrey;
TS; DoPoleZeros;
TS; DoDateTime;
TS; DoDigits;
TS; DoStadium;
TS; DoHalogen;
TS; DoMaze;
TS; DoFontEd;
TS; DoCtrlR1;
TS; DoLogo;
TS; DoSubIntAccess;
TS; DoDemoDoc;
TS; DoTreeDemo;
TS; DoTextDemo;
TS; DoCursorMove;
TS; DoMiniCompiler;
TS; DoMiniGrLib;
TS; DoPhoneNumWords;
TS; DoUnusedSpaceRep;
TS; DoExceptions;
TS; DoScreenCodes;
TS; DoExtendedChars;
TS; DoNeatTrick;
TS; DoCharGraphics;
TS; DoMagicPairs;
TS; DoHanoi;
TS; DoSquirt;
TS; DoFlowers;
TS; DoCommonAncestor;
TS; DoCartesian;
TS; DoBling;
TS; DoRainDrops;
TS; DoCollision;
TS; DoOrderedIO;
TS; DoLogic;
TS; DoCircuit;
TS; DoCompileDemo;
TS; DoPrompt;
TS; if (IsDrvWritable(':')) DoWebLog;
TS; DoDefine;
TS; DoGlblVars;
TS; DoFileRead;
TS; DoParenWarn;
TS; DoDefineStr;
TS; DoData;
TS; DoCallBack;
TS; DoCallBackAux2;
TS; DoClassMeta;
TS; DoForm;
TS; DoMenuButton;
TS; DoMenuSprite;
TS; DoSuggestedSpelling;
TS; DoWordSearch;
TS; DoStkGrow;
TS; DoWaterFowl;
TS; DoAsmHelloWorld;
TS; DoAsmAndC;
TS; DoAsmAndC2;
TS; DoMulByHand;
TS; DoDivByHand;
TS; DoBuzzFizz;
TS; DoPageTableEntries2;
DoReps;
TSEnd;
}

OSTestSuite;
