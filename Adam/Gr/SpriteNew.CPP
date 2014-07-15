#help_index "Graphics/Sprite"
#help_file "::/Doc/Sprite"

U8 polypt_map[9]={0,1,2,3,0,4,5,6,7};

I64 sprite_elem_base_sizes[SPT_NUM_TYPES]=
{
offset(CSprite.x1)-offset(CSprite.start), //SPT_END
1+offset(CSprite.color)-offset(CSprite.start), //SPT_COLOR
2+offset(CSprite.dither_color)-offset(CSprite.start), //SPT_DITHER_COLOR
offset(CSprite.y1)-offset(CSprite.start), //SPT_WIDTH
offset(CSprite.linespeed)-offset(CSprite.start), //SPT_PLANAR_SYMMETRY
offset(CSprite.x1)-offset(CSprite.start), //SPT_TRANSFORM_ON
offset(CSprite.x1)-offset(CSprite.start), //SPT_TRANSFORM_OFF
offset(CSprite.x2)-offset(CSprite.start), //SPT_SHIFT
offset(CSprite.x2)-offset(CSprite.start), //SPT_PT
offset(CSprite.y2)-offset(CSprite.start), //SPT_POLYPT
offset(CSprite.linespeed)-offset(CSprite.start), //SPT_LINE
offset(CSprite.y1)-offset(CSprite.start), //SPT_POLYLINE
offset(CSprite.linespeed)-offset(CSprite.start), //SPT_RECT
offset(CSprite.sides)-offset(CSprite.start), //SPT_ROTATED_RECT
offset(CSprite.y2)-offset(CSprite.start), //SPT_CIRCLE
offset(CSprite.sides)-offset(CSprite.start), //SPT_ELLIPSE
sizeof(CSprite)-offset(CSprite.start), //SPT_POLYGON
offset(CSprite.y1)-offset(CSprite.start), //SPT_BSPLINE2
offset(CSprite.y1)-offset(CSprite.start), //SPT_BSPLINE2_CLOSED
offset(CSprite.y1)-offset(CSprite.start), //SPT_BSPLINE3
offset(CSprite.y1)-offset(CSprite.start), //SPT_BSPLINE3_CLOSED
offset(CSprite.x2)-offset(CSprite.start), //SPT_FLOOD_FILL
offset(CSprite.x2)-offset(CSprite.start), //SPT_FLOOD_FILL_NOT
offset(CSprite.linespeed)-offset(CSprite.start), //SPT_BITMAP
offset(CSprite.x2)-offset(CSprite.start), //SPT_MESH
offset(CSprite.shiftable_tri_cnt)+sizeof(I32)-offset(CSprite.start), //SPT_SHIFTABLE_MESH
offset(CSprite.linespeed)-offset(CSprite.start), //SPT_ARROW
offset(CSprite.st)-offset(CSprite.start), //SPT_TEXT
offset(CSprite.st)-offset(CSprite.start), //SPT_TEXT_BOX
offset(CSprite.st)-offset(CSprite.start), //SPT_TEXT_DIAMOND
};

I64 SpriteElemQuedBaseSize(I64 type)
{
  return sprite_elem_base_sizes[type]+offset(CSprite.start);
}

I64 SpriteElemSize(CSprite *tempg)
{
  I64 i=sprite_elem_base_sizes[tempg->type];
  switch (tempg->type) {
    case SPT_POLYLINE:
      i+=tempg->num*sizeof(CD2I32);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      i+=StrLen(tempg->st)+1;
      break;
    case SPT_BITMAP:
      i+=((tempg->width+7)&~7)*tempg->height;
      break;
    case SPT_POLYPT:
      i+=(tempg->num*3+7)>>3;
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      i+=tempg->num*sizeof(CD3I32);
      break;
    case SPT_MESH:
      i+=tempg->vertex_cnt*sizeof(CD3I32)+tempg->tri_cnt*sizeof(CMeshTri);
      break;
    case SPT_SHIFTABLE_MESH:
      i+=tempg->shiftable_vertex_cnt*sizeof(CD3I32)+tempg->shiftable_tri_cnt*sizeof(CMeshTri);
      break;
  }
  return i;
}

public I64 SpriteSize(U8 *elems)
{//Walk sprite elements and return size of sprite as binary data.
  CSprite *tempg=elems-offset(CSprite.start),*tempg1=tempg;
  while (tempg->type)
    tempg(U8 *)+=SpriteElemSize(tempg);
  return tempg(U8 *)-tempg1(U8 *)+sprite_elem_base_sizes[SPT_END];
}

I64 SpriteTypeMask(U8 *elems)
{
  I64 result=0;
  CSprite *tempg=elems-offset(CSprite.start);
  while (tempg->type) {
    if (tempg->type>=SPT_NUM_TYPES)
      return 1<<SPT_NUM_TYPES;
    Bts(&result,tempg->type);
    tempg(U8 *)+=SpriteElemSize(tempg);
  }
  return result;
}

public U8 *DC2Sprite(CDC *tempb)
{//Convert device context to sprite.
  CSprite *tempg;
  tempg=CAlloc(sprite_elem_base_sizes[SPT_BITMAP]+
	      tempb->width_internal*tempb->height+
	   sprite_elem_base_sizes[SPT_END])
	(U8 *)-offset(CSprite.start);
  tempg->type=SPT_BITMAP;
  tempg->width=tempb->width;
  tempg->height=tempb->height;
  tempg->x2=0;
  tempg->y2=0;
  MemCpy(tempg(U8 *)+offset(CSprite.linespeed),tempb->body,
    tempb->width_internal*tempb->height);
  return tempg(U8 *)+offset(CSprite.start);
}

public U8 *SpriteElem2OneLineSummary(CSprite *tempg)
{//Study $LK,"::/Demo/Graphics/SpriteText.CPP"$.
  U8 buf[256],buf2[136];
  I32 *ptr;
  StrPrint(buf,"%Z",tempg->type,"ST_SPRITE_ELEM_TYPES");
  switch (tempg->type) {
    case SPT_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,tempg->color));
      break;
    case SPT_DITHER_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,ROPF_DITHER|tempg->dither_color.u8[0]|tempg->dither_color.u8[1]<<COLORROP_BITS));
      break;
    case SPT_PT:
    case SPT_FLOOD_FILL:
    case SPT_FLOOD_FILL_NOT:
    case SPT_SHIFT:
      CatPrint(buf," (%d,%d)",tempg->x1,tempg->y1);
      break;
    case SPT_LINE:
    case SPT_ARROW:
    case SPT_PLANAR_SYMMETRY:
    case SPT_RECT:
    case SPT_ROTATED_RECT:
      CatPrint(buf," (%d,%d),(%d,%d)",tempg->x1,tempg->y1,tempg->x2,tempg->y2);
      break;
    case SPT_CIRCLE:
      CatPrint(buf," (%d,%d):%dR",tempg->x1,tempg->y1,tempg->x2);
      break;
    case SPT_WIDTH:
      CatPrint(buf," %d",tempg->width);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      CatPrint(buf," %d,%d:%-16tQ",tempg->x1,tempg->y1,tempg->st);
      break;
    case SPT_POLYLINE:
    case SPT_POLYPT:
      ptr=&tempg->y1;
      CatPrint(buf," %d (%d,%d)",tempg->num,ptr[0],ptr[1]);
      break;
    case SPT_ELLIPSE:
    case SPT_POLYGON:
    case SPT_BITMAP:
      CatPrint(buf," (%d,%d):%dW,%dH",tempg->x2,tempg->y2,tempg->width,tempg->height);
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      CatPrint(buf," %d",tempg->num);
      break;
    case SPT_MESH:
      CatPrint(buf," %dV,%dT",tempg->vertex_cnt,tempg->tri_cnt);
      break;
    case SPT_SHIFTABLE_MESH:
      CatPrint(buf," %dV,%dT",tempg->shiftable_vertex_cnt,tempg->shiftable_tri_cnt);
      break;
  }
  return StrNew(buf);
}
