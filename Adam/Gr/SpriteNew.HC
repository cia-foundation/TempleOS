#help_index "Graphics/Sprite;Sprites"
#help_file "::/Doc/Sprite"

U8 polypt_map[9]={0,1,2,3,0,4,5,6,7};

I64 sprite_elem_base_sizes[SPT_NUM_TYPES]={
  sizeof(CSpriteBase),		//SPT_END
  sizeof(CSpriteColor),		//SPT_COLOR
  sizeof(CSpriteDitherColor),	//SPT_DITHER_COLOR
  sizeof(CSpriteW),		//SPT_WIDTH
  sizeof(CSpritePtPt),		//SPT_PLANAR_SYMMETRY
  sizeof(CSpriteBase),		//SPT_TRANSFORM_ON
  sizeof(CSpriteBase),		//SPT_TRANSFORM_OFF
  sizeof(CSpritePt),		//SPT_SHIFT
  sizeof(CSpritePt),		//SPT_PT
  sizeof(CSpriteNumPtU8s),	//SPT_POLYPT
  sizeof(CSpritePtPt),		//SPT_LINE
  sizeof(CSpriteNumU8s),		//SPT_POLYLINE
  sizeof(CSpritePtPt),		//SPT_RECT
  sizeof(CSpritePtPtAng),		//SPT_ROTATED_RECT
  sizeof(CSpritePtRad),		//SPT_CIRCLE
  sizeof(CSpritePtWHAng),		//SPT_ELLIPSE
  sizeof(CSpritePtWHAngSides),	//SPT_POLYGON
  sizeof(CSpriteNumU8s),		//SPT_BSPLINE2
  sizeof(CSpriteNumU8s),		//SPT_BSPLINE2_CLOSED
  sizeof(CSpriteNumU8s),		//SPT_BSPLINE3
  sizeof(CSpriteNumU8s),		//SPT_BSPLINE3_CLOSED
  sizeof(CSpritePt),		//SPT_FLOOD_FILL
  sizeof(CSpritePt),		//SPT_FLOOD_FILL_NOT
  sizeof(CSpritePtWHU8s),		//SPT_BITMAP
  sizeof(CSpriteMeshU8s),		//SPT_MESH
  sizeof(CSpritePtMeshU8s),	//SPT_SHIFTABLE_MESH
  sizeof(CSpritePtPt),		//SPT_ARROW
  sizeof(CSpritePtStr),		//SPT_TEXT
  sizeof(CSpritePtStr),		//SPT_TEXT_BOX
  sizeof(CSpritePtStr),		//SPT_TEXT_DIAMOND
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
      i+=tempg->nu.num*sizeof(CD2I32);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      i+=StrLen(tempg->ps.st)+1;
      break;
    case SPT_BITMAP:
      i+=((tempg->pwhu.width+7)&~7)*tempg->pwhu.height;
      break;
    case SPT_POLYPT:
      i+=(tempg->npu.num*3+7)>>3;
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      i+=tempg->nu.num*sizeof(CD3I32);
      break;
    case SPT_MESH:
      i+=tempg->mu.vertex_cnt*sizeof(CD3I32)+
	    tempg->mu.tri_cnt*sizeof(CMeshTri);
      break;
    case SPT_SHIFTABLE_MESH:
      i+=tempg->pmu.vertex_cnt*sizeof(CD3I32)+
	    tempg->pmu.tri_cnt*sizeof(CMeshTri);
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
  I64 res=0;
  CSprite *tempg=elems-offset(CSprite.start);
  while (tempg->type) {
    if (tempg->type>=SPT_NUM_TYPES)
      return 1<<SPT_NUM_TYPES;
    Bts(&res,tempg->type);
    tempg(U8 *)+=SpriteElemSize(tempg);
  }
  return res;
}

public U8 *DC2Sprite(CDC *tempb)
{//Convert device context to sprite.
  CSprite *tempg;
  tempg=CAlloc(sprite_elem_base_sizes[SPT_BITMAP]+
	tempb->width_internal*tempb->height+
	sprite_elem_base_sizes[SPT_END])
  (U8 *)-offset(CSprite.start);
  tempg->type=SPT_BITMAP;
  tempg->pwhu.width=tempb->width;
  tempg->pwhu.height=tempb->height;
  tempg->pwhu.x1=0;
  tempg->pwhu.y1=0;
  MemCpy(&tempg->pwhu.u,tempb->body,tempb->width_internal*tempb->height);
  return tempg(U8 *)+offset(CSprite.start);
}

public U8 *SpriteElem2Summary(CSprite *tempg)
{//Study $LK,"::/Demo/Graphics/SpriteText.HC"$.
  U8 buf[STR_LEN],buf2[STR_LEN];
  I32 *ptr;
  StrPrint(buf,"%Z",tempg->type,"ST_SPRITE_ELEM_TYPES");
  switch (tempg->type) {
    case SPT_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,tempg->c.color));
      break;
    case SPT_DITHER_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,ROPF_DITHER|tempg->d.dither_color.u8[0]|
	    tempg->d.dither_color.u8[1]<<COLORROP_BITS));
      break;
    case SPT_PT:
    case SPT_FLOOD_FILL:
    case SPT_FLOOD_FILL_NOT:
    case SPT_SHIFT:
      CatPrint(buf," (%d,%d)",tempg->p.x1,tempg->p.y1);
      break;
    case SPT_LINE:
    case SPT_ARROW:
    case SPT_PLANAR_SYMMETRY:
    case SPT_RECT:
    case SPT_ROTATED_RECT:
      CatPrint(buf," (%d,%d),(%d,%d)",tempg->pp.x1,tempg->pp.y1,
	    tempg->pp.x2,tempg->pp.y2);
      break;
    case SPT_CIRCLE:
      CatPrint(buf," (%d,%d):%dR",tempg->pr.x1,tempg->pr.y1,tempg->pr.radius);
      break;
    case SPT_WIDTH:
      CatPrint(buf," %d",tempg->w.width);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      CatPrint(buf," %d,%d:%-16tQ",tempg->ps.x1,tempg->ps.y1,tempg->ps.st);
      break;
    case SPT_POLYLINE:
    case SPT_POLYPT:
      ptr=&tempg->npu.x;
      CatPrint(buf," %d (%d,%d)",tempg->npu.num,ptr[0],ptr[1]);
      break;
    case SPT_ELLIPSE:
    case SPT_POLYGON:
    case SPT_BITMAP:
      CatPrint(buf," (%d,%d):%dW,%dH",tempg->pwhu.x1,tempg->pwhu.y1,
	    tempg->pwhu.width,tempg->pwhu.height);
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      CatPrint(buf," %d",tempg->nu.num);
      break;
    case SPT_MESH:
      CatPrint(buf," %dV,%dT",tempg->mu.vertex_cnt,tempg->mu.tri_cnt);
      break;
    case SPT_SHIFTABLE_MESH:
      CatPrint(buf," %dV,%dT",tempg->pmu.vertex_cnt,tempg->pmu.tri_cnt);
      break;
  }
  return StrNew(buf);
}
