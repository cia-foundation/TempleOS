#help_index "Graphics/CSprite"
#help_file "::/TempleOS/Doc/Sprite"

I64 sprite_base_size[GRST_NUM_TYPES]=
{
offset(CSprite.x1)-offset(CSprite.start), //GRST_END
1+offset(CSprite.color8)-offset(CSprite.start), //GRST_COLOR8
2+offset(CSprite.dither_color8)-offset(CSprite.start), //GRST_DITHER_COLOR8
offset(CSprite.y1)-offset(CSprite.start), //GRST_WIDTH
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_PLANAR_SYMMETRY
offset(CSprite.x1)-offset(CSprite.start), //GRST_TRANSFORM_ON
offset(CSprite.x1)-offset(CSprite.start), //GRST_TRANSFORM_OFF
offset(CSprite.x2)-offset(CSprite.start), //GRST_SHIFT
offset(CSprite.x2)-offset(CSprite.start), //GRST_PT
offset(CSprite.y1)-offset(CSprite.start), //GRST_POLYPT
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_LINE
offset(CSprite.y1)-offset(CSprite.start), //GRST_POLYLINE
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_RECT
offset(CSprite.sides)-offset(CSprite.start), //GRST_ROTATED_RECT
offset(CSprite.y2)-offset(CSprite.start), //GRST_CIRCLE
offset(CSprite.sides)-offset(CSprite.start), //GRST_ELLIPSE
sizeof(CSprite)-offset(CSprite.start), //GRST_POLYGON
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE2
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE2_CLOSED
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE3
offset(CSprite.y1)-offset(CSprite.start), //GRST_BSPLINE3_CLOSED
offset(CSprite.x2)-offset(CSprite.start), //GRST_FLOOD_FILL
offset(CSprite.x2)-offset(CSprite.start), //GRST_FLOOD_FILL_NOT
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_BITMAP8
offset(CSprite.x2)-offset(CSprite.start), //GRST_MESH
offset(CSprite.shiftable_tri_cnt)+sizeof(I32)-offset(CSprite.start), //GRST_SHIFTABLE_MESH
offset(CSprite.linespeed)-offset(CSprite.start), //GRST_ARROW
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT_BOX
offset(CSprite.st)-offset(CSprite.start), //GRST_TEXT_DIAMOND
};

I64 SpriteQuedBaseSize(I64 type)
{
  return sprite_base_size[type]+offset(CSprite.start);
}

I64 SpriteElemSize(CSprite *tempg)
{
  I64 i=sprite_base_size[tempg->type];
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

public I64 SpriteSize(U8 *elems)
{
  CSprite *tempg=elems-offset(CSprite.start),*tempg1=tempg;
  while (tempg->type)
    tempg(U8 *)+=SpriteElemSize(tempg);
  return tempg(U8 *)-tempg1(U8 *)+sprite_base_size[GRST_END];
}

I64 SpriteTypeMask(U8 *elems)
{
  I64 result=0;
  CSprite *tempg=elems-offset(CSprite.start);
  while (tempg->type) {
    if (tempg->type>=GRST_NUM_TYPES)
      return 1<<GRST_NUM_TYPES;
    Bts(&result,tempg->type);
    tempg(U8 *)+=SpriteElemSize(tempg);
  }
  return result;
}

public U8 *DC2Sprite(CDC *tempb)
{
  CSprite *tempg;
  CDC *tempb1;
  Bool temp_bitmap;
  if (tempb->type!=DCT_COLOR8) {
    tempb1=DCNew(DCT_COLOR8,tempb->width,tempb->height,Fs);
    tempb1->color=WHITE;
    GrBlot(tempb1,0,0,tempb);
    temp_bitmap=TRUE;
  } else {
    tempb1=tempb;
    temp_bitmap=FALSE;
  }
  tempg=CAlloc(sprite_base_size[GRST_BITMAP8]+
	      tempb1->width_internal*tempb1->height+
	   sprite_base_size[GRST_END])
	(U8 *)-offset(CSprite.start);
  tempg->type=GRST_BITMAP8;
  tempg->width=tempb1->width;
  tempg->height=tempb1->height;
  tempg->x2=0;
  tempg->y2=0;
  MemCpy(tempg(U8 *)+offset(CSprite.linespeed),tempb1->body,
    tempb1->width_internal*tempb1->height);
  if (temp_bitmap)
    DCDel(tempb1);
  return tempg(U8 *)+offset(CSprite.start);
}

public U8 *Sprite2Text(CSprite *tempg)
{ //Study $LK-A,"::/TempleOS/Demo/Graphics/SpriteText.CPP"$.
  U8 buf[256],buf2[256],buf3[136];
  I32 *ptr;
  SPrint(buf,"%Z",tempg->type,"ST_SPRITE_TYPES");
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
