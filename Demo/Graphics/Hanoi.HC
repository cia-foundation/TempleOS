#define DISKS_NUM	6
#define PEDESTAL_HEIGHT	20
#define DISK_HEIGHT	7
#define DISK_UNIT_WIDTH	5

I64 poles_x[3];
I64 disks_x[DISKS_NUM],disks_y[DISKS_NUM],disks_pole[DISKS_NUM];

I64 OtherPole(I64 pole1,I64 pole2)
{
  return 3-pole1-pole2;
}
 
I64 TopDisk(I64 pole)
{
  I64 i;
  for (i=0;i<DISKS_NUM;i++)
    if (disks_pole[i]==pole)
      return i;
  return -1;
}

I64 PosInStk(I64 pole,I64 disk)
{
  I64 res=0,i;
  for (i=DISKS_NUM-1;i>disk;i--)
    if (disks_pole[i]==pole)
      res++;
  return res;
}

U0 SetDisksRestXY()
{
  I64 i;
  for (i=0;i<DISKS_NUM;i++) {
    disks_x[i]=poles_x[disks_pole[i]];
    disks_y[i]=Fs->pix_height-PEDESTAL_HEIGHT
	  -(DISK_HEIGHT+1)/2-1-(DISK_HEIGHT+1)*PosInStk(disks_pole[i],i);
  }
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i;

  for (i=0;i<3;i++)
    poles_x[i]=(1+i)*task->pix_width/4;

  dc->color=BLACK;
  GrRect(dc,poles_x[0]-50,task->pix_height-PEDESTAL_HEIGHT,
	poles_x[2]-poles_x[0]+100,PEDESTAL_HEIGHT-FONT_HEIGHT);
  dc->color=DKGRAY;
  GrRect(dc,poles_x[0]-49,task->pix_height-PEDESTAL_HEIGHT+1,
	poles_x[2]-poles_x[0]+98,PEDESTAL_HEIGHT-FONT_HEIGHT-2);

  for (i=0;i<3;i++) {
    dc->color=BLACK;
    GrRect(dc,poles_x[i]-3,
	  task->pix_height-PEDESTAL_HEIGHT-(DISKS_NUM+1)*(DISK_HEIGHT+1),
	  7,(DISKS_NUM+1)*(DISK_HEIGHT+1));
    dc->color=YELLOW;
    GrRect(dc,poles_x[i]-2,
	  task->pix_height-PEDESTAL_HEIGHT+1-(DISKS_NUM+1)*(DISK_HEIGHT+1),
	  5,(DISKS_NUM+1)*(DISK_HEIGHT+1)-1);
  }

  for (i=0;i<DISKS_NUM;i++) {
    dc->color=BLACK;
    GrRect(dc,disks_x[i]-(i+1)*DISK_UNIT_WIDTH,
	  disks_y[i]-DISK_HEIGHT/2,(i+1)*(DISK_UNIT_WIDTH*2)+1,DISK_HEIGHT);
    dc->color=gr_rainbow_10[i];
    GrRect(dc,disks_x[i]-(i+1)*DISK_UNIT_WIDTH+1,
	  disks_y[i]-DISK_HEIGHT/2+1,(i+1)*(DISK_UNIT_WIDTH*2)-1,DISK_HEIGHT-2);
  }
}

U0 MySleep()
{
  if (ScanChar)
    throw;
  Sleep(3);
}

U0 MoveDisks(I64 src_pole,I64 dst_pole,I64 num)
{
  I64 top,x,y;
  if (num>1)
    MoveDisks(src_pole,OtherPole(src_pole,dst_pole),num-1);
  DocClear;
  "$$CM+BY,0,0$$Disk:%d from %d to %d\n",TopDisk(src_pole),src_pole,dst_pole;

  top=TopDisk(src_pole);
  for (y=disks_y[top];
	y>Fs->pix_height-PEDESTAL_HEIGHT-(DISK_HEIGHT+1)/2
	-(DISK_HEIGHT+1)*(DISKS_NUM+2);y--) {
    disks_y[top]=y;
    MySleep;
  }
  if (src_pole<dst_pole)
    for (x=poles_x[src_pole];x<=poles_x[dst_pole];x++) {
      disks_x[top]=x;
      MySleep;
    }
  else
    for (x=poles_x[src_pole];x>=poles_x[dst_pole];x--) {
      disks_x[top]=x;
      MySleep;
    }

  disks_pole[top]=dst_pole;
  for (y=disks_y[top];
	y<Fs->pix_height-PEDESTAL_HEIGHT-(DISK_HEIGHT+1)/2-1
	-(DISK_HEIGHT+1)*PosInStk(dst_pole,top);y++) {
    disks_y[top]=y;
    MySleep;
  }
  SetDisksRestXY;
  if (num>1)
    MoveDisks(OtherPole(src_pole,dst_pole),dst_pole,num-1);
}

U0 Init()
{
  I64 i;
  for (i=0;i<3;i++)
    poles_x[i]=(1+i)*Fs->pix_width/4;

  for (i=0;i<DISKS_NUM;i++)
    disks_pole[i]=0;

  SetDisksRestXY;
}

U0 Hanoi()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Init;
  DocClear;
  Fs->draw_it=&DrawIt;

  Sleep(1000);
  try {
    MoveDisks(0,2,DISKS_NUM);
    Beep; Beep;
    DocClear;
    DocBottom;
    PressAKey;
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
}

Hanoi;
