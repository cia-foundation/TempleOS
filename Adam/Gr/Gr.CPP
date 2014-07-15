#help_index "Graphics"
U0 GrInit1()
{
  DefineLstLoad("ST_SPRITE_ELEM_TYPES",
	"End\0Color\0Dither Color\0"
	"Width\0Planar Symmetry\0Transform On\0Transform Off\0Shift\0"
	"Point\0PolyPoint\0Line\0PolyLine\0Rect\0Rotated Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2 Closed\0BSpline3\0BSpline3 Closed\0"
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

#help_index "Graphics/Sprite"
#define SPT_END		0
#define SPT_COLOR		1
#define SPT_DITHER_COLOR	2
#define SPT_WIDTH		3
#define SPT_PLANAR_SYMMETRY	4
#define SPT_TRANSFORM_ON	5
#define SPT_TRANSFORM_OFF	6
#define SPT_SHIFT		7
#define SPT_PT			8
#define SPT_POLYPT		9
#define SPT_LINE		10
#define SPT_POLYLINE		11
#define SPT_RECT		12
#define SPT_ROTATED_RECT	13
#define SPT_CIRCLE		14
#define SPT_ELLIPSE		15
#define SPT_POLYGON		16
#define SPT_BSPLINE2		17
#define SPT_BSPLINE2_CLOSED	18
#define SPT_BSPLINE3		19
#define SPT_BSPLINE3_CLOSED	20
#define SPT_FLOOD_FILL		21
#define SPT_FLOOD_FILL_NOT	22
#define SPT_BITMAP		23
#define SPT_MESH		24
#define SPT_SHIFTABLE_MESH	25
#define SPT_ARROW		26
#define SPT_TEXT		27
#define SPT_TEXT_BOX		28
#define SPT_TEXT_DIAMOND	29
#define SPT_NUM_TYPES		30

class CMeshTri
{
  I32 color; //Give one of 0-7 colors.	Colors 8-15 are 0-7 with intensity bit set.
  I32 nums[3]; //Vertex number
};

class CQueMeshTri
{
  CQueMeshTri *next,*last;
  U0 start;
  I32 color;
  I32 nums[3];
};

public class CSprite
{//See $LK,"Graphics/Sprite",A="HI:Graphics/Sprite"$
//See $LK,"::/Adam/Gr/GrSpritePlot.CPP",A="FL:::/Adam/Gr/GrSpritePlot.CPP,1"$ for how CSprite are stored.
  CSprite *next,*last;

  U0 start;
  U8 type;
  union {
    U8 color;
    U16 dither_color;
    I32 x1;
    I32 num;
    I32 width;
    I32 vertex_cnt; //Mesh
  }
  union {
    I32 y1;
    I32 height;
    I32 tri_cnt; //Mesh
  }
  union {
    I32 z1;
    I32 x2;
    U8 st[4];
  }
  union {
    I32 y2;
    I32 shiftable_vertex_cnt //Shiftable Mesh
  }
  union {
    F64 linespeed;
    F64 rot_angle;
    I32 shiftable_tri_cnt //Shiftable Mesh
  }
  I32 sides;
};

#help_index "Graphics"

extern I64 GrBlot3(CDC *dc,I64 x1,I64 y1,I64 z1,CDC *img);
extern U8 *GrBitMap4ToBitMap8(U8 *dst,U8 *src,I64 src_size,I64 bkcolor);
extern U8 *SpriteQue2Sprite(CSprite *root,I64 *_size=NULL);

#include "GrGlbls"
#include "GrTextBase"
#include "GrAsm"
#include "GrDC"
#include "GrPalette"
#include "GrInit"
#include "MathGr"
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
