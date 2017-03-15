#help_index "Graphics/Sprite;Sprites"
#help_file "::/Doc/Sprite"

U8 polypt_map[9]={0,1,2,3,0,4,5,6,7};

I64 sprite_elem_base_sizes[SPT_TYPES_NUM]={
  sizeof(CSpriteBase),		//SPT_END
  sizeof(CSpriteColor),		//SPT_COLOR
  sizeof(CSpriteDitherColor),	//SPT_DITHER_COLOR
  sizeof(CSpriteT),		//SPT_THICK
  sizeof(CSpritePtPt),		//SPT_PLANAR_SYMMETRY
  sizeof(CSpriteBase),		//SPT_TRANSFORM_ON
  sizeof(CSpriteBase),		//SPT_TRANSFORM_OFF
  sizeof(CSpritePt),		//SPT_SHIFT
  sizeof(CSpritePt),		//SPT_PT
  sizeof(CSpriteNumPtU8s),	//SPT_POLYPT
  sizeof(CSpritePtPt),		//SPT_LINE
  sizeof(CSpriteNumU8s),	//SPT_POLYLINE
  sizeof(CSpritePtPt),		//SPT_RECT
  sizeof(CSpritePtPtAng),	//SPT_ROTATED_RECT
  sizeof(CSpritePtRad),		//SPT_CIRCLE
  sizeof(CSpritePtWHAng),	//SPT_ELLIPSE
  sizeof(CSpritePtWHAngSides),	//SPT_POLYGON
  sizeof(CSpriteNumU8s),	//SPT_BSPLINE2
  sizeof(CSpriteNumU8s),	//SPT_BSPLINE2_CLOSED
  sizeof(CSpriteNumU8s),	//SPT_BSPLINE3
  sizeof(CSpriteNumU8s),	//SPT_BSPLINE3_CLOSED
  sizeof(CSpritePt),		//SPT_FLOOD_FILL
  sizeof(CSpritePt),		//SPT_FLOOD_FILL_NOT
  sizeof(CSpritePtWHU8s),	//SPT_BITMAP
  sizeof(CSpriteMeshU8s),	//SPT_MESH
  sizeof(CSpritePtMeshU8s),	//SPT_SHIFTABLE_MESH
  sizeof(CSpritePtPt),		//SPT_ARROW
  sizeof(CSpritePtStr),		//SPT_TEXT
  sizeof(CSpritePtStr),		//SPT_TEXT_BOX
  sizeof(CSpritePtStr),		//SPT_TEXT_DIAMOND
};

I64 SpriteElemQuedBaseSize(I64 type)
{
  return sprite_elem_base_sizes[type&SPG_TYPE_MASK]+offset(CSprite.start);
}

I64 SpriteElemSize(CSprite *tmpg)
{
  I64 i=sprite_elem_base_sizes[tmpg->type&SPG_TYPE_MASK];
  switch (tmpg->type&SPG_TYPE_MASK) {
    case SPT_POLYLINE:
      i+=tmpg->nu.num*sizeof(CD2I32);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      i+=StrLen(tmpg->ps.st)+1;
      break;
    case SPT_BITMAP:
      i+=((tmpg->pwhu.width+7)&~7)*tmpg->pwhu.height;
      break;
    case SPT_POLYPT:
      i+=(tmpg->npu.num*3+7)>>3;
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      i+=tmpg->nu.num*sizeof(CD3I32);
      break;
    case SPT_MESH:
      i+=tmpg->mu.vertex_cnt*sizeof(CD3I32)+
	    tmpg->mu.tri_cnt*sizeof(CMeshTri);
      break;
    case SPT_SHIFTABLE_MESH:
      i+=tmpg->pmu.vertex_cnt*sizeof(CD3I32)+
	    tmpg->pmu.tri_cnt*sizeof(CMeshTri);
      break;
  }
  return i;
}

public I64 SpriteSize(U8 *elems)
{//Walk sprite elements and return size of sprite as binary data.
  CSprite *tmpg=elems-offset(CSprite.start),*tmpg1=tmpg;
  while (tmpg->type&SPG_TYPE_MASK)
    tmpg(U8 *)+=SpriteElemSize(tmpg);
  return tmpg(U8 *)-tmpg1(U8 *)+sprite_elem_base_sizes[SPT_END];
}

I64 SpriteTypeMask(U8 *elems)
{
  I64 res=0;
  CSprite *tmpg=elems-offset(CSprite.start);
  while (tmpg->type&SPG_TYPE_MASK) {
    if (tmpg->type&SPG_TYPE_MASK>=SPT_TYPES_NUM)
      return 1<<SPT_TYPES_NUM;
    Bts(&res,tmpg->type&SPG_TYPE_MASK);
    tmpg(U8 *)+=SpriteElemSize(tmpg);
  }
  return res;
}

U0 SpriteSelAll(U8 *elems,Bool val)
{
  CSprite *tmpg=elems-offset(CSprite.start);
  while (tmpg->type&SPG_TYPE_MASK) {
    BEqu(&tmpg->type,SPf_SEL,val);
    tmpg(U8 *)+=SpriteElemSize(tmpg);
  }
}

public U8 *DC2Sprite(CDC *tmpb)
{//Convert device context to sprite.
  CSprite *tmpg;
  tmpg=CAlloc(sprite_elem_base_sizes[SPT_BITMAP]+
	tmpb->width_internal*tmpb->height+
	sprite_elem_base_sizes[SPT_END])
  (U8 *)-offset(CSprite.start);
  tmpg->type=SPT_BITMAP;
  tmpg->pwhu.width=tmpb->width;
  tmpg->pwhu.height=tmpb->height;
  tmpg->pwhu.x1=0;
  tmpg->pwhu.y1=0;
  MemCpy(&tmpg->pwhu.u,tmpb->body,tmpb->width_internal*tmpb->height);
  return tmpg(U8 *)+offset(CSprite.start);
}

public U8 *SpriteElem2Summary(CSprite *tmpg)
{//Study $LK,"::/Demo/Graphics/SpriteText.HC"$.
  U8 buf[STR_LEN],buf2[STR_LEN];
  I32 *ptr;
  StrPrint(buf,"%Z",tmpg->type&SPG_TYPE_MASK,"ST_SPRITE_ELEM_TYPES");
  switch (tmpg->type&SPG_TYPE_MASK) {
    case SPT_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,tmpg->c.color));
      break;
    case SPT_DITHER_COLOR:
      CatPrint(buf," %s",Color2Str(buf2,ROPF_DITHER|tmpg->d.dither_color.u8[0]|
	    tmpg->d.dither_color.u8[1]<<COLORROP_BITS));
      break;
    case SPT_PT:
    case SPT_FLOOD_FILL:
    case SPT_FLOOD_FILL_NOT:
    case SPT_SHIFT:
      CatPrint(buf," (%d,%d)",tmpg->p.x1,tmpg->p.y1);
      break;
    case SPT_LINE:
    case SPT_ARROW:
    case SPT_PLANAR_SYMMETRY:
    case SPT_RECT:
    case SPT_ROTATED_RECT:
      CatPrint(buf," (%d,%d),(%d,%d)",tmpg->pp.x1,tmpg->pp.y1,
	    tmpg->pp.x2,tmpg->pp.y2);
      break;
    case SPT_CIRCLE:
      CatPrint(buf," (%d,%d):%dR",tmpg->pr.x1,tmpg->pr.y1,tmpg->pr.radius);
      break;
    case SPT_THICK:
      CatPrint(buf," %d",tmpg->t.thick);
      break;
    case SPT_TEXT:
    case SPT_TEXT_BOX:
    case SPT_TEXT_DIAMOND:
      CatPrint(buf," %d,%d:%-16t$$Q",tmpg->ps.x1,tmpg->ps.y1,tmpg->ps.st);
      break;
    case SPT_POLYLINE:
    case SPT_POLYPT:
      ptr=&tmpg->npu.x;
      CatPrint(buf," %d (%d,%d)",tmpg->npu.num,ptr[0],ptr[1]);
      break;
    case SPT_ELLIPSE:
    case SPT_POLYGON:
    case SPT_BITMAP:
      CatPrint(buf," (%d,%d):%dW,%dH",tmpg->pwhu.x1,tmpg->pwhu.y1,
	    tmpg->pwhu.width,tmpg->pwhu.height);
      break;
    case SPT_BSPLINE2:
    case SPT_BSPLINE3:
    case SPT_BSPLINE2_CLOSED:
    case SPT_BSPLINE3_CLOSED:
      CatPrint(buf," %d",tmpg->nu.num);
      break;
    case SPT_MESH:
      CatPrint(buf," %dV,%dT",tmpg->mu.vertex_cnt,tmpg->mu.tri_cnt);
      break;
    case SPT_SHIFTABLE_MESH:
      CatPrint(buf," %dV,%dT",tmpg->pmu.vertex_cnt,tmpg->pmu.tri_cnt);
      break;
  }
  return StrNew(buf);
}
