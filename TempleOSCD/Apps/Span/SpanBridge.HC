#define STRENGTH_SCALE	1.5e7
#define SPRING_SCALE	6.0e6
#define COST_SCALE	375.0
#define MASS_RADIUS	3.0
#define MASS_MASS	10.0
#define WIRE_PERCENT	0.99

#define RIVER_BANK_COLS	5
#define SKY_LINES	40

#define FIXED_X1	((RIVER_BANK_COLS-1)*FONT_WIDTH)
#define FIXED_X2	((RIVER_BANK_COLS-4)*FONT_WIDTH)
#define FIXED_X3	(TEXT_COLS*FONT_WIDTH>>1)
#define FIXED_Y1	((SKY_LINES+1)*FONT_HEIGHT)
#define FIXED_Y2	((SKY_LINES+5)*FONT_HEIGHT)
#define FIXED_Y3	((TEXT_ROWS-8)*FONT_HEIGHT)

#define LOAD_WEIGHTS	8

U0 SpanBridge1Init(CMathODE *)
{
  I64 i;

  MyMass *tmpm;
  tmpm=PlaceMass(GR_WIDTH-FIXED_X1,FIXED_Y1);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(GR_WIDTH-FIXED_X2,FIXED_Y1);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(GR_WIDTH-FIXED_X1,FIXED_Y2);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(FIXED_X1,FIXED_Y1);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(FIXED_X2,FIXED_Y1);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(FIXED_X1,FIXED_Y2);
  tmpm->flags|=MSF_FIXED;
  tmpm=PlaceMass(FIXED_X3,FIXED_Y3);
  tmpm->flags|=MSF_FIXED;

  for (i=0;i<LOAD_WEIGHTS;i++) {
    tmpm=PlaceMass(FIXED_X1+(i+1)*(GR_WIDTH-2*FIXED_X1)/(LOAD_WEIGHTS+1),
	  FIXED_Y1);
    tmpm->load_t=(i+1.0)/LOAD_WEIGHTS;
    tmpm->color=RED;
  }

  DocClear;
  "$$BG,LTCYAN$$%h*c",SKY_LINES,'\n';
  for (i=0;i<10;i++) {
    "$$BG,BROWN$$%h*c",RIVER_BANK_COLS,CH_SPACE;
    "$$BG,LTCYAN$$%h*c",TEXT_COLS-2*RIVER_BANK_COLS,CH_SPACE;
    "$$BG,BROWN$$%h*c\n",RIVER_BANK_COLS,CH_SPACE;
  }
  for (i=0;i<5;i++)  {
    "$$BG,BROWN$$%h*c",RIVER_BANK_COLS+i,CH_SPACE;
    "$$BG,BLUE$$%h*c",TEXT_COLS/2-RIVER_BANK_COLS-2*i,CH_SPACE;
    "$$BG,BROWN$$%h*c",2*i,CH_SPACE;
    "$$BG,BLUE$$%h*c",TEXT_COLS/2-RIVER_BANK_COLS-2*i,CH_SPACE;
    "$$BG,BROWN$$%h*c\n",RIVER_BANK_COLS+i,CH_SPACE;
  }
}

U0 AdjustLoads(CMathODE *ode)
{
  MyMass *tmpm=ode->next_mass;
  F64 d,tt=SpanTime/10.0;
  while (tmpm!=&ode->next_mass) {
    if (tmpm->load_t) {
      if (tt) {
	d=Abs(Sin(tmpm->load_t*ã+tt)); //0.0<=d<=1.0
	tmpm->mass=100.0*Sqr(Sqr(d+1.0));
	tmpm->radius=7.0*d+2.0;
      } else {
	tmpm->mass=MASS_MASS;
	tmpm->radius=MASS_RADIUS;
      }
    }
    tmpm=tmpm->next;
  }
}
