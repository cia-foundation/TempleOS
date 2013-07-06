//This uses $LK,"fixed-point","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.
















$SP+BP,"<1>",1,"::/TempleOS/Demo/Graphics/Camp.CPP.Z,6"$ //Pointer to sprite <6> in Camp.CPP.Z file

#define NUM_CLOUDS		16
#define SKY_LINES		30
#define CLOUD_PTS		512
#define CLOUD_PENS		8
#define CLOUD_PEN_PTS	16
#define CLOUD_PEN_SIZE		16

class Cloud
{
  I64 i,x,y,w,h,dx,dy;
  I64 color;
  I64 px[CLOUD_PTS],py[CLOUD_PTS],pc[CLOUD_PTS];
} clouds[NUM_CLOUDS];

class CloudPen
{
  CDC *img;
  I64 px[CLOUD_PEN_PTS],py[CLOUD_PEN_PTS];
} cloud_pens[CLOUD_PENS];

CDC *cloud_pen;

I64 mp_not_done_flags;
U0 MPDrawClouds(CTask *task)
{
  Cloud *c;
  CDC *dc=DCAlias(gr_dc2,task);
  I64 i,j,k,xx,yy,
     lo= Gs->num   *CLOUD_PTS/mp_cnt,
     hi=(Gs->num+1)*CLOUD_PTS/mp_cnt;

  for (j=0,c=clouds;j<NUM_CLOUDS;j++,c++)
    for (i=lo;i<hi;i++) {
      k=c->pc[i];
      if (k<c->color)
	dc->color=ROP_MONO+LTGRAY;
      else
	dc->color=ROP_MONO+WHITE;
      xx=c->x.i32[1]+c->px[i];
      yy=c->y.i32[1]+c->py[i];

      k=RandU16&31-16;
      if (k==-16)
	k=-c->px[i];
      c->px[i]+=SignI64(k);

      k=RandU16&31-16;
      if (k==-16)
	k=-c->py[i];
      c->py[i]+=SignI64(k);

      GrBlot(dc,xx,yy,cloud_pens[i&(CLOUD_PENS-1)].img);
    }
  DCDel(dc);
  LBtr(&mp_not_done_flags,Gs->num);
}

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i;
  SpritePlot3(dc,0,SKY_LINES*FONT_HEIGHT,0,$IB,"<1>",1$);
  mp_not_done_flags=1<<mp_cnt-1;
  for (i=0;i<mp_cnt;i++)
    JobQue(&MPDrawClouds,task,i);
  while (mp_not_done_flags)
    Yield;
}

U0 AnimateTask(I64)
{
  I64 i,j;
  Cloud *c;
  CloudPen *cp;
  while (TRUE) {
    WinMgrSync;

    for (i=0,cp=cloud_pens;i<CLOUD_PENS;i++,cp++) {
      DCClear(cp->img);
      cp->img->color=COLOR_MONO;
      for (j=0;j<CLOUD_PEN_PTS;j++) {
	GrPlot(cp->img,cp->px[j],cp->py[j]);
	cp->px[j]=LimitI64(cp->px[j]+RandU16%3-1,0,CLOUD_PEN_SIZE-1);
	cp->py[j]=LimitI64(cp->py[j]+RandU16%3-1,0,CLOUD_PEN_SIZE-1);
      }
    }

    for (i=0,c=clouds;i<NUM_CLOUDS;i++,c++) {
      c->x+=c->dx;
      c->y=LimitI64(c->y+c->dy,0,0.7*(SKY_LINES*FONT_HEIGHT)<<32);
      c->color=MAX_U16*c->y.i32[1]/(0.8*(SKY_LINES*FONT_HEIGHT));
    }

    Sleep(20);
  }
}

#define SAMPLES	6
U0 Init()
{
  Cloud *c=clouds;
  CloudPen *cp=cloud_pens;
  I64 i,j,k,l,
     w=Fs->win_pixel_width,
     h=SKY_LINES*FONT_HEIGHT;

  for (i=0;i<NUM_CLOUDS;i++,c++) {
    c->x=ToI64(w/2+Rand*w/2-w/4)<<32;
    c->y=ToI64(h/2+Rand*h/2-h/4)<<32;
    c->dx=RandI32;
    c->dy=RandI32;
    c->w=100;
    c->h=50;
    c->color=RandU16;
    for (l=0;l<CLOUD_PTS;l++) {
      k=0;
      for (j=0;j<SAMPLES;j++)
	k+=RandI16;
      c->px[l]=(k*c->w/MAX_I16)/SAMPLES;

      k=0;
      for (j=0;j<SAMPLES;j++)
	k+=RandI16;
      c->py[l]=(k*c->h/MAX_I16)/SAMPLES;

      c->pc[l]=RandU16;
    }
  }

  for (i=0;i<CLOUD_PENS;i++,cp++) {
    cp->img=DCNew(DCT_COLOR8,CLOUD_PEN_SIZE,CLOUD_PEN_SIZE);
    for (j=0;j<CLOUD_PEN_PTS;j++) {
      cp->px[j]=RandU16%CLOUD_PEN_SIZE;
      cp->py[j]=RandU16%CLOUD_PEN_SIZE;
    }
  }
}

U0 CleanUp()
{
  I64 i;
  CloudPen *cp=cloud_pens;
  for (i=0;i<CLOUD_PENS;i++,cp++)
    DCDel(cp->img);
}

U0 SongTask(I64)
{ //Song by the Holy Spirit
//This specifies a call-back routine
//to be called when the task dies.
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("3qBetDA2G3qD2sG3E2G3EetCEDqFEeDC");
    Play("qBetDA2G3qD2sG3E2G3EetCEDqFEeDC");
    Play("CGqDeADsDCDCqGEetDAD2sG3D2G3D");
    Play("eCGqDeADsDCDCqGEetDAD2sG3D2G3D");
  }
}

U0 CloudScene()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  try {
    Fs->text_attr=YELLOW<<4+BLUE;
    Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
    DocCursor;
    WinMax;
    WinBorder;
    WordStat;
    Init;
    Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);

    DocClear;
    "$$BG,LTCYAN$$"; NewLine(SKY_LINES);
    "$$BG,YELLOW$$"; NewLine(5);
    PutBibleVerse("Exodus,14:19",7);

    Preempt(OFF);
    Fs->draw_it=&DrawIt;
    GetChar;
    WinMgrSync;
  } catch
    CatchAll;
  SettingsPop;
  CleanUp;
}

//This is so I can use this file stand alone
//or as a component of a program.
#if IncludeDepth<2
CloudScene;
#endif
