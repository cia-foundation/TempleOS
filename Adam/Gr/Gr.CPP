#help_index "Graphics"
U0 GrInit1()
{
  DefineLstLoad("ST_SPRITE_TYPES",
	"End\0Color 8Bit\0Dither Color 8Bit\0"
	"Width\0Planar Symmetry\0Transform On\0Transform Off\0Shift\0"
	"Point\0PolyPoint\0Line\0PolyLine\0Rect\0Rotated Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2 Closed\0BSpline3\0BSpline3 Closed\0"
	"Flood Fill\0Flood Fill Not Color\0BitMap 8Bit\0Mesh\0Shiftable Mesh\0"
	"Arrow\0Text\0Text Box\0Text Diamond\0");
}
GrInit1;

#help_index "Graphics/Sprite"
#define GRST_END		0
#define GRST_COLOR8		1
#define GRST_DITHER_COLOR8	2
#define GRST_WIDTH		3
#define GRST_PLANAR_SYMMETRY	4
#define GRST_TRANSFORM_ON	5
#define GRST_TRANSFORM_OFF	6
#define GRST_SHIFT		7
#define GRST_PT			8
#define GRST_POLYPT		9
#define GRST_LINE		10
#define GRST_POLYLINE		11
#define GRST_RECT		12
#define GRST_ROTATED_RECT	13
#define GRST_CIRCLE		14
#define GRST_ELLIPSE		15
#define GRST_POLYGON		16
#define GRST_BSPLINE2		17
#define GRST_BSPLINE2_CLOSED	18
#define GRST_BSPLINE3		19
#define GRST_BSPLINE3_CLOSED	20
#define GRST_FLOOD_FILL		21
#define GRST_FLOOD_FILL_NOT	22
#define GRST_BITMAP8		23
#define GRST_MESH		24
#define GRST_SHIFTABLE_MESH	25
#define GRST_ARROW		26
#define GRST_TEXT		27
#define GRST_TEXT_BOX		28
#define GRST_TEXT_DIAMOND	29
#define GRST_NUM_TYPES		30

class CMeshTri
{
  I32 color; //Give one of 0-7 colors.	Colors 8-15 are 0-7 with intensity bit set.
  I32 nums[3]; //Vertex number
};

public class CSprite
{
//See $LK,"Graphics/Sprite",A="HI:Graphics/Sprite"$
//See $LK,"::/Adam/Gr/GrSpritePlot.CPP",A="FL:::/Adam/Gr/GrSpritePlot.CPP,1"$ for how CSprite are stored.
  CSprite *next,*last;

  U0 start;
  U8 type;
  union {
    U8 color8;
    U16 dither_color8;
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

#include "GrGlbls"
#include "GrAsm"
#include "GrDC"
#include "GrPalette"
#include "GrInit"
#include "MathGr"
#include "GrScreen"
#include "GrBitMap"
#include "GrPrimatives"
#include "GrComposites"
#include "GrChar"

#define GE_ABORT	(-1)
#define GE_EXIT		0
#define GE_CONT		1
#include "SpriteNew"
#include "GrSpritePlot"
#include "SpriteMesh"
#include "SpriteBitMap"
#include "SpriteEd"
#include "SpriteMain"

#include "GrEnd"

#help_index ""
