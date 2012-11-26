I64 ts_num;
U0 TSStart()
{
  ts_num=0;
}
U0 TS()
{
  progress4=ts_num++;
  SPrintF(progress4_desc,"%d",progress4);
  progress4_max=157;
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SpritePlot.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePlot3D()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SpritePlot3D.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePut()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SpritePut.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoSpritePutExt()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SpritePutExt.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoSpriteText()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SpriteText.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoEdCSprite()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/EdCSprite.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoExtents()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Extents.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}


U0 DoDotNet()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/DotNet.CPP.Z\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLowPassFilter()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/LowPassFilter.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/MathAudioDemo.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Lines.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoBounce()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Demo/Graphics/Bounce.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMsgLoop()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MsgLoop.CPP.Z\";\n");
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(300);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoASCIIOrgan()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/ASCIIOrgan.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Doodle.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Speedline.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/KeyBitMap.CPP.Z\" Exit;\n");
  Sleep(20);
  LBts(key_down_bitmap,i_sc);
  Sleep(500);
  LBtr(key_down_bitmap,i_sc);
}

U0 DoBlink()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Blink.CPP.Z\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoGolf()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Golf.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/Graphics/Mountain.CPP.Z\";\n");
  Sleep(1500);
  Kill(task);
}

U0 DoCoach()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Coach.CPP.Z\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoDigits()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Digits.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/CPUPlot.CPP.Z\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoDither()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Dither.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoSymmetry()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Symmetry.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/BSpline.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Blot.CPP.Z\";\n");
  Sleep(1800);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCaptureScreen()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/CaptureScreen.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/PullDownMenu.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoStadium()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Stadium/Stadium\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Palette.CPP.Z\";\n");
  Sleep(400);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(400);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoHalogen()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Halogen.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/TheDead.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Bomber.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/ZoneOut.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/Varoom.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/Rocks.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/Navy.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/Wenceslas.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/Chess.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/ChessNet.CPP.Z\";\n");
  while (!TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_NEW_LINE,0);
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoRainDrops()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/RainDrops.CPP.Z\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMorseCode()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/MorseCode.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/PixelCollision.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBlackDiamond()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/BlackDiamond.CPP.Z\";\n");
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
  I64 i;
  CTask *task=SpawnUser,*task1;
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
  task1=task->popup_task;
  TaskWait(task->popup_task);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<2;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task1=task->popup_task;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Width
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task1=task->popup_task;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<4;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task1=task->popup_task;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Line
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
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
  task1=task->popup_task;
  TaskWait(task->popup_task);
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
    Yield;
  TaskWait(task->popup_task);
  for (i=0;i<3;i++)
    SendMsg(task->popup_task,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
  task1=task->popup_task;
  PostMsg(task->popup_task,MSG_KEY_DOWN_UP,CH_SPACE,0);

//Circle
  while (task->popup_task==task1 || !TaskValidate(task->popup_task))
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
  XTalk(task,"#include \"::/SparrowOS/Demo/Graphics/Circuit.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/SpaceShip.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Box.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoShadow()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Shadow.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPredatorPrey()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Demo/Graphics/PredatorPrey.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/3DPoly.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLogo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Logo.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRotateTank()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/RotateTank.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoPic2ASCII()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Pic2ASCII.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoScreenCodes()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/ScreenCodes.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoExtendedChars()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/ExtendedChars.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoNeatTrick()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/NeatTrick.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCharGraphics()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/CharGraphics.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoMaze()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Maze.CPP.Z\";\n");
  Sleep(750);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoFontEd()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/FontEd.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoTimeIns()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/TimeIns.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoWaterFowl()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/WaterFowl.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/MusicOrgan/Examples\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoMPPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/MPPrintF.CPP.Z\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoLayers3DJob()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Layers3DJob.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoCartesian()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Cartesian.CPP.Z\";\n");
  XTalkWait(task,"0.2*x`1.5\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoHanoi()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Hanoi.CPP.Z\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 DoLife()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Life.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoZing()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Zing.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoFlowers()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Flowers.CPP.Z\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoCommonAncestor()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/CommonAncestor.CPP.Z\";\n");
  Sleep(3000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoPalindrome()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Palindrome.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoMagicPairs()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MagicPairs.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoMPAdd()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/MPAdd.CPP.Z\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLock()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Lock.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoLayersSpawn()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/LayersSpawn.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoClouds()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Clouds.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoNoise()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Noise.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/SliderCtrl.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBling()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Bling.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/ScrollBars.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/RegistryDemo.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoBirdLand()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/BirdLand.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/MissileDefense.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/WarGame.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/BigGuns.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/DunGen.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Apps/Timeout/Run\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Collision.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoLoadTest()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/LoadTest.CPP.Z\";\n");
  Sleep(2000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoPrimes()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Primes.CPP.Z\";\n");
  Sleep(2500);
  XTalk(task,"Exit;\n");
}

U0 DoSort()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Sort.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoRadixSort()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/RadixSort.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoPageTableEntries2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Lectures/PageTableEntries2.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoInterrupts()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/MultiCore/Interrupts.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoStkGrow()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/StkGrow.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoPilgrims()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Apps/Pilgrims/Run\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Apps/Span/Run\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/RawHide.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Apps/AfterEgypt/Run\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Shading.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoPoleZeros()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Demo/Graphics/PoleZeros.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Whap.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Pick.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Pick3D.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Transform.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/Tanks\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

*/

U0 DoGrModels()
{
  CTask *task=SpawnUser,*task1=NULL;
  I64 i,j;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/GrModels/Run\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  PostMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  for (i=0;i<4;i++) {
    while (task->popup_task==task1 || !TaskValidate(task->popup_task))
      Yield;
    task1=task->popup_task;
    TaskWait(task1);
    for (j=0;j<=i;j++)
      SendMsg(task1,MSG_KEY_DOWN_UP,0,SC_CURSOR_DOWN);
    SendMsg(task1,MSG_KEY_DOWN_UP,CH_SPACE,0);
  }
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,'n',0);
  XTalk(task,"Exit;\n");
}

U0 DoKeepAway()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Apps/KeepAway/Run\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Graphics/Camp.CPP.Z\";\n");
  Sleep(1250);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRocket()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Rocket.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/RocketScience.CPP.Z\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/MassSpring.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Whap.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoSquirt()
{
  I64 i;
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GameStarters/Squirt.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Apps/X-Caliber/Run\";\n");
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
  XTalk(task,"#include \"::/SparrowOS/Demo/GameStarters/FPS.CPP.Z\";\n");
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
  XTalkWait(task,"Cd(\"::/SparrowOS/Demo/GameStarters\");Del(\"EagleDive.BIN.Z\");\n");
  XTalkWait(task,"Cmp(\"EagleDive.CPP.Z\");\n");
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
  XTalkWait(task,"Del(\"::/SparrowOS/Demo/GameStarters/EagleDive.BIN.Z\");\n");
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/Span\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/MusicOrgan\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPhoneNumWords()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/PhoneNumWords.CPP.Z\";\n");
  XTalkWait(task,"702-254-4223\n\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoSuggestedSpelling()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/SuggestSpelling.CPP.Z\";\n");
  XTalkWait(task,"effecient\n\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoWordSearch()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/WordSearch.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Dsk/DataBase.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoOrderedIO()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Dsk/OrderedIO.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 DoFPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Dsk/FPrintF.CPP.Z\";\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoRaw()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Dsk/Raw.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoUnusedSpaceRep()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Dsk/UnusedSpaceRep.CPP.Z\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoMiniCompiler()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Lectures/MiniCompiler.CPP.Z\";\n");
  XTalkWait(task,"1+2*(3+4)\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
  XTalk(task,"Exit;\n");
}

U0 DoMiniGrLib()
{
  CTask *task=SpawnUser;
  XTalk(task,"#include \"::/SparrowOS/Demo/Lectures/MiniGrLib.CPP.Z\";Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoWebLog()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/WebLogExample/WebLogRep.CPP.Z\";\n");
  SendMsg(task,MSG_KEY_DOWN_UP,CH_NEW_LINE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_NEW_LINE,0);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_ESC,0);
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/TimeClock\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoLogic()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Apps/Logic/Run\";\n");
  XTalkWait(task,"NAND\nNOR\n\n0x100\n0xF0\n0xCC\n0xAA\n\n0x12\n\n");
  Sleep(1500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Lectures\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/OldSchool\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/AutoFile\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoPrintF()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/PrintF.CPP.Z\";\n");
  Sleep(2000);
  XTalk(task,"Exit;\n");
}

U0 DoDateTime()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/DateTime.CPP.Z\";\n");
  Sleep(2500);
  XTalk(task,"Exit;\n");
}

U0 DoSubIntAccess()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/SubIntAccess.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoParenWarn()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/ParenWarn.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCompileDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/CompileDemo.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoPrompt()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Prompt.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/ClassMeta.CPP.Z\";\n");
  Sleep(2000);
  XTalk(task,"Exit;\n");
}

U0 DoForm()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/Form.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/MenuButton.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/MenuSprite.CPP.Z\";\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/TreeDemo.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoExceptions()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Exceptions.CPP.Z\";\n");
  XTalkWait(task,"yyn");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoTextDemo()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/TextDemo.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoCursorMove()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/CursorMove.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoDemoDoc()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"Ed(\"::/SparrowOS/Demo/Doc/DemoDoc.TXT.Z\");\n");
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
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Spy.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoFileRead()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/FileRead.CPP.Z\";\n");
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  Sleep(1000);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 DoData()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/Data.CPP.Z\";\n");
  Sleep(500);
  XTalk(task,"Exit;\n");
}

U0 DoDefineStr()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/DefineStr.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCallBack()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/CallBack.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoCallBackAux2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/CallBackAux2.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

U0 DoDefine()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Define.CPP.Z\";\n");
  XTalkWait(task,"YS\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Doc/UnusedDefine.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}
*/

U0 DoAsmHelloWorld()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/AsmHelloWorld.CPP.Z\";\n");
  Sleep(750);
  XTalk(task,"Exit;\n");
}

U0 DoAsmAndC()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/AsmAndC.CPP.Z\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoAsmAndC2()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/AsmAndC2.CPP.Z\";\n2\n");
  XTalk(task,"Exit;\n");
  while (TaskValidate(task))
    Yield;
}

U0 DoBuzzFizz()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/BuzzFizz.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoMulByHand()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/MulByHand.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoDivByHand()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Asm/DivByHand.CPP.Z\";\n");
  Sleep(1000);
  XTalk(task,"Exit;\n");
}

U0 DoGlblVars()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/GlblVars.CPP.Z\";\n");
  Sleep(1500);
  XTalk(task,"Exit;\n");
}

/*
U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/WinMgr2.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/PolyPhonic.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/SndScope.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/Synth.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/Record.CPP.Z\";\n");
  Sleep(500);
  SendMsg(task,MSG_KEY_DOWN_UP,CH_SPACE,0);
  XTalk(task,"Exit;\n");
}

U0 Do()
{
  CTask *task=SpawnUser;
  XTalkWait(task,"#include \"::/SparrowOS/Demo/Snd/Echo.CPP.Z\";\n");
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
TS; DoSpan;
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
TS; DoBounce;
TS; DoMsgLoop;
TS; DoASCIIOrgan;
TS; DoMorseCode;
TS; DoDoodle;
TS; DoMassSpring;
TS; DoSpeedLine;
TS; DoKeyBitMap;
TS; DoBlink;
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
