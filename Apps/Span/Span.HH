#define SPAN_VERSION	1.0

class SpanHeader
{
F64 version;
I32 num_masses,num_springs;
};

class MyMass:CMass
{
F64 radius,cost,load_t;
CColorROPU32 color;
} *cursor_mass;

class MySpring:CSpring
{
F64 compression_strength,tensile_strength,cost;
F64 base_compression_strength,base_tensile_strength,
base_const,base_cost;

CColorROPU32 color;
I32 thick;
};

CMathODE *ode=NULL;

CCtrlBttnState run_bttn,mode_bttn;

CColorROPU32 run_colors[2]={RED,GREEN};

#define MD_MASS		0
#define MD_CONCRETE	1
#define MD_STEEL	2
#define MD_WIRE		3
#define MD_MOVE		4
#define MD_DELETE	5
#define MD_MODES_NUM	6
CColorROPU32 mode_colors[MD_MODES_NUM]=
{BROWN,LTGRAY,DKGRAY,RED,PURPLE,RED};
  DefineLstLoad("ST_SPAN_MODES","Mass\0Concrete\0Steel\0Wire\0Move\0Delete\0");

  class SpanAnimateStruct
	  {
    U8 *saved_ode;
  F64 elapsed_t,start_wall_t;
} a;

extern MyMass *PlaceMass(I64 x, I64 y);
extern F64 SpanTime();
