//***************************************************
DefineLstLoad("ST_SPRITE_TYPES_OLD","End\0 \0Color 8Bit\0 \0Point\0"
	"Line\0PolyLine\0PolyPoint\0Width\0Shift\0"
	"BSpline2\0BSpline3\0BSpline2 Closed\0BSpline3 Closed\0"
	" \0 \0 \0Rect\0Circle\0Flood Fill Not Color\0Flood Fill\0BitMap 8Bit\0"
	" \0Text\0Ellipse\0Polygon\0Planar Symmetry\0"
	"Dither Color 8Bit\0Rotated Rect\0Mesh\0Shiftable Mesh\0"
	"Transform On\0Transform Off\0Arrow\0Text Box\0Text Diamond\0");

#define GRST_END		0
//Unused
#define GRST_COLOR8		2
//Unused
#define GRST_PT			4
#define GRST_LINE		5
#define GRST_POLYLINE		6
#define GRST_POLYPT		7
#define GRST_WIDTH		8
#define GRST_SHIFT		9
#define GRST_BSPLINE2		10
#define GRST_BSPLINE3		11
#define GRST_BSPLINE2_CLOSED	12
#define GRST_BSPLINE3_CLOSED	13
//Unused
//Unused
//Unused
#define GRST_RECT		17
#define GRST_CIRCLE		18
#define GRST_FLOOD_FILL_NOT	19
#define GRST_FLOOD_FILL		20
#define GRST_BITMAP8		21
//Unused
#define GRST_TEXT		23
#define GRST_ELLIPSE		24
#define GRST_POLYGON		25
#define GRST_PLANAR_SYMMETRY	26
#define GRST_DITHER_COLOR8	27
#define GRST_ROTATED_RECT	28
#define GRST_MESH		29
#define GRST_SHIFTABLE_MESH	30
#define GRST_TRANSFORM_ON	31
#define GRST_TRANSFORM_OFF	32
#define GRST_ARROW		33
#define GRST_TEXT_BOX		34
#define GRST_TEXT_DIAMOND	35
#define GRST_NUM_TYPES		36

I64 gr_sprite_base_size_old[GRST_NUM_TYPES]=
{
offset(CSprite.x1)-offset(CSprite.start), //GRST_END
1,
1+offset(CSprite.color8)-offset(CSprite.start), //GRST_COLOR8
1,
offset(CSprite.x2)-offset(CSprite.start), //GRST_PT
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_LINE
offset(CSprite.y1)-offset(CSprite.start), //GRST_POLYLINE
offset(CSprite.y1)-offset(CSprite.start), //GRST_POLYPT
offset(CSprite.y1)-offset(CSprite.start), //GRST_WIDTH
offset(CSprite.x2)-offset(CSprite.start), //GRST_SHIFT
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE2
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE3
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE2_CLOSED
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE3_CLOSED
1,
1,
1,
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_RECT
offset(CSprite.y2)-offset(CSprite.start), //GRST_CIRCLE
offset(CSprite.x2)-offset(CSprite.start), //GRST_FLOOD_FILL_NOT
offset(CSprite.x2)-offset(CSprite.start), //GRST_FLOOD_FILL
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_BITMAP8
1,
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT
offset(CSprite.sides)-offset(CSprite.start), //GRST_ELLIPSE
sizeof(CSprite)-offset(CSprite.start), //GRST_POLYGON
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_PLANAR_SYMMETRY
2+offset(CSprite.dither_color8)-offset(CSprite.start), //GRST_DITHER_COLOR8
offset(CSprite.sides)-offset(CSprite.start), //GRST_ROTATED_RECT
offset(CSprite.x2)-offset(CSprite.start), //GRST_MESH
offset(CSprite.shiftable_tri_cnt)+sizeof(I32)-offset(CSprite.start), //GRST_SHIFTABLE_MESH
offset(CSprite.x1)-offset(CSprite.start), //GRST_TRANSFORM_ON
offset(CSprite.x1)-offset(CSprite.start), //GRST_TRANSFORM_OFF
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_ARROW
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT_BOX
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT_DIAMOND
};

I64 SpriteElemSizeOld(CSprite *tempg)
{
  I64 i=gr_sprite_base_size_old[tempg->type];
  switch (tempg->type) {
    case GRST_POLYLINE:
      i+=tempg->num<<1*sizeof(I32);
      break;
    case GRST_TEXT:
    case GRST_TEXT_BOX:
    case GRST_TEXT_DIAMOND:
      i+=StrLen(tempg->st)+1;
      break;
    case GRST_BITMAP8:
      i+=((tempg->width+7)&~7)*tempg->height;
      break;
    case GRST_POLYPT:
      i+=sizeof(I32)<<1+(tempg->num*3+7)>>3;
      break;
    case GRST_BSPLINE2:
    case GRST_BSPLINE3:
    case GRST_BSPLINE2_CLOSED:
    case GRST_BSPLINE3_CLOSED:
      i+=tempg->num*sizeof(CD3I32);
      break;
    case GRST_MESH:
      i+=tempg->vertex_cnt*sizeof(CD3I32)+tempg->tri_cnt*sizeof(CMeshTri);
      break;
    case GRST_SHIFTABLE_MESH:
      i+=tempg->shiftable_vertex_cnt*sizeof(CD3I32)+tempg->shiftable_tri_cnt*sizeof(CMeshTri);
      break;
  }
  return i;
}

public U8 *Sprite2TextOld(CSprite *tempg)
{ //Study $LK-A,"::/TempleOS/Demo/Graphics/SpriteText.CPP"$.
  U8 buf[256],buf2[256],buf3[136];
  I32 *ptr;
  SPrint(buf,"%Z",tempg->type,"ST_SPRITE_TYPES_OLD");
  *buf2=0;
  switch (tempg->type) {
    case GRST_COLOR8:
      SPrint(buf2," %s",Color2Text(buf3,tempg->color8));
      break;
    case GRST_PT:
    case GRST_FLOOD_FILL:
    case GRST_FLOOD_FILL_NOT:
    case GRST_SHIFT:
      SPrint(buf2," %d,%d",tempg->x1,tempg->y1);
      break;
    case GRST_LINE:
    case GRST_ARROW:
    case GRST_PLANAR_SYMMETRY:
    case GRST_RECT:
    case GRST_ROTATED_RECT:
      SPrint(buf2," (%d,%d)-(%d,%d)",
	  tempg->x1,tempg->y1,tempg->x2,tempg->y2);
      break;
    case GRST_CIRCLE:
      SPrint(buf2," %d,%d:%dR",
	  tempg->x1,tempg->y1,tempg->x2);
      break;
    case GRST_WIDTH:
      SPrint(buf2," %d",tempg->width);
      break;
    case GRST_TEXT:
    case GRST_TEXT_BOX:
    case GRST_TEXT_DIAMOND:
      SPrint(buf2," %d,%d:%-16tQ",tempg->x1,tempg->y1,tempg->st);
      break;
    case GRST_DITHER_COLOR8:
      SPrint(buf2," %s",Color2Text(buf3,ROPF_DITHER|tempg->dither_color8.u8[0]|tempg->dither_color8.u8[1]<<COLORROP_BITS));
      break;
    case GRST_POLYLINE:
    case GRST_POLYPT:
      ptr=&tempg->y1;
      SPrint(buf2," %d %d,%d",tempg->num,ptr[0],ptr[1]);
      break;
    case GRST_ELLIPSE:
    case GRST_POLYGON:
    case GRST_BITMAP8:
      SPrint(buf2," %dW,%dH:%d,%d",
	  tempg->width,tempg->height,tempg->x2,tempg->y2);
      break;
    case GRST_BSPLINE2:
    case GRST_BSPLINE3:
    case GRST_BSPLINE2_CLOSED:
    case GRST_BSPLINE3_CLOSED:
      SPrint(buf2," %d",tempg->num);
      break;
    case GRST_MESH:
      SPrint(buf2," %dV,%dT",tempg->vertex_cnt,tempg->tri_cnt);
      break;
    case GRST_SHIFTABLE_MESH:
      SPrint(buf2," %dV,%dT",tempg->shiftable_vertex_cnt,tempg->shiftable_tri_cnt);
      break;
  }
  StrCat(buf,buf2);
  return StrNew(buf);
}

I64 cvt_map[36]=
#define GRST_TEXT		23
{
0,-1,1,-1,8,
10,11,9,
3,7,17,
19,18,20,
-1,-1,-1,
12,14,22,
21,23,-1,27,
15,16,4,
2,13,24,
25,5,6,
26,28,29
};

U0 SpriteCvt(U8 *elems)
{
  CSprite *tempg=elems-offset(CSprite.start);
  U8 *st;
  I64 size;
  while (tempg->type) {
    st=Sprite2TextOld(tempg);
    "%s\n",st;
    Free(st);
    size=SpriteElemSizeOld(tempg);
    tempg->type=cvt_map[tempg->type];
    tempg(U8 *)+=size;
  }
}

Bool FileCvt(U8 *name)
{
  CDoc *doc;
  CDocBin *tempb;
  Bool result=FALSE,old_preempt=Preempt(OFF);
  doc=DocRead(name);
  tempb=doc->bin_root.next;
  while (tempb!=&doc->bin_root) {
    if (tempb->type==DOCBT_CSPRITE) {
      result=TRUE;
      "%s\n",name;
      SpriteCvt(tempb->data);
    }
    tempb=tempb->next;
  }
  if (result)
    DocWrite(doc);
  DocDel(doc);
  Preempt(old_preempt);
  return result;
}

U0 FilesCvt(U8 *mask)
{
  CDirEntry *tempde=FilesFind(mask,FUF_RECURSE|FUF_JUST_TXT|FUF_JUST_FILES),
	    *tempde1=tempde;
  Bool old_preempt=Preempt(OFF);
  while (tempde) {
    FileCvt(tempde->full_name);
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
  Preempt(old_preempt);
}

//FilesCvt("/TempleOS/*");
