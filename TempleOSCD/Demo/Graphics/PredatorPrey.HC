//Classic 2nd order nonlinear ordinary differential equation

I64 mp_not_done_flags;

#define OX	10
#define OY	GR_HEIGHT/2-FONT_HEIGHT

#define	MICRO_STEP	0.000001

class State
{
  F64 wabbits,hawks,
	d_wabbits,d_hawks; //Technically, these are not part of the state.
};

U0 Steps(State *s)
{
  I64 i;
  for (i=0;i<10000;i++) {
    s->d_wabbits=s->wabbits*(1.0-0.10*s->hawks);
    s->d_hawks	=-s->hawks *(1.0-0.01*s->wabbits);
    s->hawks  +=s->d_hawks  *MICRO_STEP;
    s->wabbits+=s->d_wabbits*MICRO_STEP;
  }
}

U0 PlotTrajectory(CTask *parent)
{
  I64 i;
  CDC *dc=DCAlias(,parent);
  State s;

  MemSet(&s,0,sizeof(State));
  s.wabbits=RandU16%100 +10;
  s.hawks  =RandU16%10	+1;

  for (i=0;i<1000;i++) {
    dc->color=LTGREEN;
    GrPlot(dc,s.wabbits+OX,OY-s.d_wabbits);
    dc->color=LTRED;
    GrPlot(dc,s.hawks  +OX,OY-s.d_hawks);
    Steps(&s);
  }
  DCDel(dc);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 PredatorPrey()
{
  I64 i;
  CDC *dc=DCAlias;

  PopUpOk("This will plot multiple predator-prey\n"
	"trajectories.  It restarts many times\n"
	"with different, random, initial populations.\n");
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  try {
    AutoComplete;
    WinBorder;
    WinMax;
    DocClear;
    Refresh;

    dc->color=BLACK;
    GrLine(dc,OX,0,OX,GR_HEIGHT-FONT_HEIGHT-1);
    GrLine(dc,0,OY,GR_WIDTH-1,OY);
    while (!ScanChar) {
      mp_not_done_flags=1<<mp_cnt-1;
      for (i=0;i<mp_cnt;i++)
	JobQue(&PlotTrajectory,Fs,i);
      do Yield;
      while (mp_not_done_flags);
    }
  } catch
    PutExcept;
  SettingsPop;
  DCFill(dc);
  DCDel(dc);
}

PredatorPrey;
