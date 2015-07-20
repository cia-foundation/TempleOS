#help_index "Graphics"
U0 GrInit1()
{
  DefineLstLoad("ST_SPRITE_ELEM_TYPES",
	"End\0Color\0Dither Color\0"
	"Width\0Planar Symmetry\0Transform On\0Transform Off\0Shift\0"
	"Point\0PolyPoint\0Line\0PolyLine\0Rect\0Rotated Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2 Closed\0"
	"BSpline3\0BSpline3 Closed\0"
	"Flood Fill\0Flood Fill Not Color\0BitMap\0Mesh\0Shiftable Mesh\0"
	"Arrow\0Text\0Text Box\0Text Diamond\0");
  DefineLstLoad("ST_SPRITE_ELEM_CODES",
	"End\0Color\0Color\0"
	"Width\0PlanarSymmetry\0Transform\0Transform\0Shift\0"
	"Pt\0PolyPt\0Line\0PolyLine\0Rect\0Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2\0BSpline3\0BSpline3\0"
	"FloodFill\0FloodFill\0BitMap\0Mesh\0Mesh\0"
	"Arrow\0Text\0TextBox\0TextDiamond\0");
}
GrInit1;

#help_index "Sprites/Binary"

class CMeshTri
{
  I32 color;	//Give one of 0-7 colors.
//Colors 8-15 are 0-7 with intensity bit set.
  I32 nums[3];	//Vertex number
};

class CQueMeshTri
{
  CQueMeshTri *next,*last;
  U0 start;
  I32 color;
  I32 nums[3];
};

public class CSpriteBase
{
  U8 type;
};

public class CSpriteColor : CSpriteBase
{
  U8 color;
};

public class CSpriteDitherColor : CSpriteBase
{
  U16 dither_color;
};

public class CSpriteW : CSpriteBase
{
  I32 width;
};

public class CSpritePt : CSpriteBase
{
  I32 x1;
  I32 y1;
};

public class CSpritePtRad : CSpritePt
{
  I32 radius;
};

public class CSpritePtPt : CSpritePt
{
  I32 x2;
  I32 y2;
};

public class CSpritePtPtAng : CSpritePtPt
{
  F64 angle;
};

public class CSpritePtWH : CSpritePt
{
  I32 width;
  I32 height;
};

public class CSpritePtWHU8s : CSpritePtWH
{
  U8 u[0];
};

public class CSpritePtWHAng : CSpritePtWH
{
  F64 angle;
};

public class CSpritePtWHAngSides : CSpritePtWHAng
{
  I32 sides;
};

public class CSpriteNumU8s : CSpriteBase
{
  I32 num;
  U8  u[0];
};

public class CSpriteNumPtU8s : CSpriteBase
{
  I32 num;
  I32 x;
  I32 y;
  U8  u[0];
};

public class CSpritePtStr : CSpritePt
{
  U8  st[0];
};

public class CSpriteMeshU8s : CSpriteBase
{
  I32 vertex_cnt;
  I32 tri_cnt;
  U8  u[0];
};

public class CSpritePtMeshU8s : CSpriteBase
{
  I32 x;
  I32 y;
  I32 z;
  I32 vertex_cnt;
  I32 tri_cnt;
  U8  u[0];
};

#define SPT_END			0	//$LK,"CSpriteBase",A="MN:CSpriteBase"$
#define SPT_COLOR		1	//$LK,"CSpriteColor",A="MN:CSpriteColor"$
#define SPT_DITHER_COLOR	2	//$LK,"CSpriteDitherColor",A="MN:CSpriteDitherColor"$
#define SPT_WIDTH		3	//$LK,"CSpriteW",A="MN:CSpriteW"$
#define SPT_PLANAR_SYMMETRY	4	//$LK,"CSpritePtPt",A="MN:CSpritePtPt"$
#define SPT_TRANSFORM_ON	5	//$LK,"CSpriteBase",A="MN:CSpriteBase"$
#define SPT_TRANSFORM_OFF	6	//$LK,"CSpriteBase",A="MN:CSpriteBase"$
#define SPT_SHIFT		7	//$LK,"CSpritePt",A="MN:CSpritePt"$
#define SPT_PT			8	//$LK,"CSpritePt",A="MN:CSpritePt"$
#define SPT_POLYPT		9	//$LK,"CSpriteNumPtU8s",A="MN:CSpriteNumPtU8s"$
#define SPT_LINE		10	//$LK,"CSpritePtPt",A="MN:CSpritePtPt"$
#define SPT_POLYLINE		11	//$LK,"CSpriteNumU8s",A="MN:CSpriteNumU8s"$
#define SPT_RECT		12	//$LK,"CSpritePtPt",A="MN:CSpritePtPt"$
#define SPT_ROTATED_RECT	13	//$LK,"CSpritePtPtAng",A="MN:CSpritePtPtAng"$
#define SPT_CIRCLE		14	//$LK,"CSpritePtRad",A="MN:CSpritePtRad"$
#define SPT_ELLIPSE		15	//$LK,"CSpritePtWHAng",A="MN:CSpritePtWHAng"$
#define SPT_POLYGON		16	//$LK,"CSpritePtWHAngSides",A="MN:CSpritePtWHAngSides"$
#define SPT_BSPLINE2		17	//$LK,"CSpriteNumU8s",A="MN:CSpriteNumU8s"$
#define SPT_BSPLINE2_CLOSED	18	//$LK,"CSpriteNumU8s",A="MN:CSpriteNumU8s"$
#define SPT_BSPLINE3		19	//$LK,"CSpriteNumU8s",A="MN:CSpriteNumU8s"$
#define SPT_BSPLINE3_CLOSED	20	//$LK,"CSpriteNumU8s",A="MN:CSpriteNumU8s"$
#define SPT_FLOOD_FILL		21	//$LK,"CSpritePt",A="MN:CSpritePt"$
#define SPT_FLOOD_FILL_NOT	22	//$LK,"CSpritePt",A="MN:CSpritePt"$
#define SPT_BITMAP		23	//$LK,"CSpritePtWHU8s",A="MN:CSpritePtWHU8s"$
#define SPT_MESH		24	//$LK,"CSpriteMeshU8s",A="MN:CSpriteMeshU8s"$ See $MA-X+PU,"::/Apps/GrModels",LM="Cd(\"::/Apps/GrModels\");Dir;View;\n"$.
#define SPT_SHIFTABLE_MESH	25	//$LK,"CSpritePtMeshU8s",A="MN:CSpritePtMeshU8s"$ See $MA-X+PU,"::/Apps/GrModels",LM="Cd(\"::/Apps/GrModels\");Dir;View;\n"$.
#define SPT_ARROW		26	//$LK,"CSpritePtPt",A="MN:CSpritePtPt"$
#define SPT_TEXT		27	//$LK,"CSpritePtStr",A="MN:CSpritePtStr"$
#define SPT_TEXT_BOX		28	//$LK,"CSpritePtStr",A="MN:CSpritePtStr"$
#define SPT_TEXT_DIAMOND	29	//$LK,"CSpritePtStr",A="MN:CSpritePtStr"$
#define SPT_NUM_TYPES		30

#help_index "Graphics/Sprite;Sprites;Sprites/Binary"

//See $LK,"Sprites",A="HI:Sprites"$.  See $LK,"::/Adam/Gr/GrSpritePlot.CPP",A="FL:::/Adam/Gr/GrSpritePlot.CPP,1"$ and
//$LK,"::/Demo/Graphics/SpriteRaw.CPP"$ for how CSprite are stored.
public class CSprite
{
  CSprite *next,*last;
  U0 start;
  union {
    U8 type;
    CSpriteBase		b;
    CSpriteColor	c;
    CSpriteDitherColor	d;
    CSpriteW	w;
    CSpritePt		p;
    CSpritePtPt		pp;
    CSpritePtPtAng	ppa;
    CSpritePtRad	pr;
    CSpritePtWHU8s	pwhu;
    CSpritePtWHAng	pwha;
    CSpritePtWHAngSides pwhas;
    CSpriteNumU8s	nu;
    CSpriteNumPtU8s	npu;
    CSpriteMeshU8s	mu;
    CSpritePtMeshU8s	pmu;
    CSpritePtStr	ps;
  }
};

#help_index "Graphics"

extern I64 GrBlot3(CDC *dc,I64 x1,I64 y1,I64 z1,CDC *img);
extern U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor);
extern I64 *Mat4x4New(I64 *src,CTask *mem_task=NULL);
extern U8 *SpriteQue2Sprite(CSprite *root,I64 *_size=NULL);

#include "GrGlbls"
#include "GrTextBase"
#include "GrAsm"
#include "GrPalette"
#include "GrDC"
#include "GrInit"
#include "GrMath"
#include "GrScreen"
#include "GrBitMap"
#include "GrPrimatives"
#include "GrComposites"

#define GE_ABORT	(-1)
#define GE_EXIT		0
#define GE_CONT		1
#include "SpriteNew"
#include "GrSpritePlot"
#include "SpriteMesh"
#include "SpriteBitMap"
#include "SpriteCode"
#include "SpriteEd"
#include "SpriteMain"

#include "GrEnd"
