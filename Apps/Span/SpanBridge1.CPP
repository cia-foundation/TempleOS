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

  MyMass *tempm;
  tempm=PlaceMass(GR_WIDTH-FIXED_X1,FIXED_Y1);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(GR_WIDTH-FIXED_X2,FIXED_Y1);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(GR_WIDTH-FIXED_X1,FIXED_Y2);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(FIXED_X1,FIXED_Y1);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(FIXED_X2,FIXED_Y1);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(FIXED_X1,FIXED_Y2);
  tempm->flags|=MSF_FIXED;
  tempm=PlaceMass(FIXED_X3,FIXED_Y3);
  tempm->flags|=MSF_FIXED;

  for (i=0;i<LOAD_WEIGHTS;i++) {
    tempm=PlaceMass(FIXED_X1+(i+1)*(GR_WIDTH-2*FIXED_X1)/(LOAD_WEIGHTS+1),
		    FIXED_Y1);
    tempm->load_t=(i+1.0)/LOAD_WEIGHTS;
    tempm->color=RED;
  }

  DocClear;
  "$$BG,LTCYAN$$"; NewLine(SKY_LINES);
  for (i=0;i<10;i++) {
    "$$BG,BROWN$$";  Spaces(RIVER_BANK_COLS);
    "$$BG,LTCYAN$$"; Spaces(TEXT_COLS-2*RIVER_BANK_COLS);
    "$$BG,BROWN$$";  Spaces(RIVER_BANK_COLS);
    NewLine;
  }
  for (i=0;i<5;i++)  {
    "$$BG,BROWN$$";  Spaces(RIVER_BANK_COLS+i);
    "$$BG,BLUE$$";   Spaces(TEXT_COLS/2-RIVER_BANK_COLS-2*i);
    "$$BG,BROWN$$";  Spaces(2*i);
    "$$BG,BLUE$$";   Spaces(TEXT_COLS/2-RIVER_BANK_COLS-2*i);
    "$$BG,BROWN$$";  Spaces(RIVER_BANK_COLS+i);
    NewLine;
  }
}

U0 AdjustLoads(CMathODE *ode)
{
  MyMass *tempm=ode->next_mass;
  F64 d,tt=SpanTime/10.0;
  while (tempm!=&ode->next_mass) {
    if (tempm->load_t) {
      if (tt) {
	d=Abs(Sin(tempm->load_t*ã+tt)); //0.0<=d<=1.0
	tempm->mass=100.0*Sqr(Sqr(d+1.0));
	tempm->radius=7.0*d+2.0;
      } else {
	tempm->mass=MASS_MASS;
	tempm->radius=MASS_RADIUS;
      }
    }
    tempm=tempm->next;
  }
}
