#define NUM_DATA_PTS 256

F64 CPU_idle[MP_MAX_PROCESSORS][NUM_DATA_PTS];
F64 mem[NUM_DATA_PTS];
F64 refresh[NUM_DATA_PTS];

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,x1,y1,x2,y2;
  U8 buf[32];
  for (i=1;i<NUM_DATA_PTS;i++) {
    x1=(i-1)*task->win_pixel_width/(NUM_DATA_PTS-1);
    x2=i*task->win_pixel_width/(NUM_DATA_PTS-1);
    for (j=0;j<mp_cnt;j++) {
      dc->color=gr_rainbow_10[j];
      y1=task->win_pixel_height*CPU_idle[j][i-1];
      y2=task->win_pixel_height*CPU_idle[j][i];
      GrLine(dc,x1,y1,x2,y2);
    }
    dc->color=LTCYAN;
    y1=task->win_pixel_height*mem[i-1];
    y2=task->win_pixel_height*mem[i];
    GrLine(dc,x1,y1,x2,y2);

    dc->color=LTPURPLE;
    y1=task->win_pixel_height*refresh[i-1];
    y2=task->win_pixel_height*refresh[i];
    GrLine(dc,x1,y1,x2,y2);
  }
  for (j=0;j<mp_cnt;j++) {
    SPrintF(buf,"CPU%d",j);
    PlotWinStr(task,buf,gr_rainbow_10[j]+WHITE<<4,0,task->win_height-4-j);
  }
  PlotWinStr(task,"Mem %",LTCYAN+WHITE<<4,0,task->win_height-3);
  PlotWinStr(task,"Refresh Hz",LTPURPLE+WHITE<<4,0,task->win_height-2);
}

U0 CPUPlot()
{
  I64 cur_pt=0,j;
  MemSet(CPU_idle,0,sizeof(CPU_idle));
  MemSet(mem,0,sizeof(F64)*NUM_DATA_PTS);
  MemSet(refresh,0,sizeof(F64)*NUM_DATA_PTS);

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;

  while (!ScanChar) {
    for (j=0;j<mp_cnt;j++)
      CPU_idle[j][cur_pt]=cpu_structs[j].idle_factor;
    mem[cur_pt]=ToF64(UnusedSysMem)/(sys_heap_limit-sys_heap_base+1);
    refresh[cur_pt]=1.0-win_actual_refresh/100.0;  //This plots on a scale of 0-100 Hz
    Sleep(25);
    cur_pt= ++cur_pt % NUM_DATA_PTS;
  }
  SettingsPop;
}

CPUPlot; //start when #included

//Study $LK,"::/SparrowOS/Adam/MemRep.CPP.Z","FI:::/SparrowOS/Adam/MemRep.CPP.Z"$ and $LK,"WallPaper","MN:WallPaper"$() to learn more about
//how the system resources are put together.
