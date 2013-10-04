#help_index "Graphics/DCs"

public U0 DCRst(CDC *dc)
{
  dc->color=BLACK;
  dc->color2=BLACK;
  dc->bkcolor=BLACK;
  dc->collision_cnt=0;
  dc->pen_width=1;
  dc->ls.x=37837; //1<<16/Sqrt(3)
  dc->ls.y=37837;
  dc->ls.z=37837;
  dc->x=0;
  dc->y=0;
  dc->z=0;
  dc->transform=&GrTransform;
  dc->lighting =&GrLighting;
  GrIdentEqu(dc->r);
  dc->r_norm=gr_scale;
  dc->flags&=~(DCF_SYMMETRY|DCF_TRANSFORMATION|DCF_JUST_MIRROR);
}

public CDC *DCAlias(CDC *dc=NULL,CTask *task=NULL)
{
  CDC *result;
  if (!dc)   dc=gr_dc;
  if (!task) task=Fs;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    Dbg;
  result=MAlloc(sizeof(CDC),task);
  MemCpy(result,dc,sizeof(CDC));
  result->win_task=task;
  result->mem_task=task;
  result->r=MAlloc(16*sizeof(I64),task);
  DCRst(result);
  result->flags|=DCF_ALIAS;
  result->alias=dc;
  return result;
}


public CDC *DCNew(I64 type,I64 width,I64 height,CTask *task=NULL,Bool null_bitmap=FALSE)
{  //This internally only allows widths which are divisible by 8
//Don't forget these $MA+A-X+PU,"sizeof(CDC)","Grep(\"sizeof(CDC)\",\"/TempleOS/*\");View;"$.
  CDC *result;
  if (!task) task=Fs;
  result=CAlloc(sizeof(CDC),task);
  result->win_task=task;
  result->mem_task=task;
  result->type=type;
  result->width=width;
  result->width_internal=(width+7)&~7;
  result->height=height;
  switch (type) {
    case DCT_COLOR4:
      result->plane_size=(result->width_internal*result->height)>>3;
      if (!null_bitmap)
	result->body=CAlloc(result->plane_size<<2,task);
      break;
    case DCT_COLOR8:
      result->plane_size=result->width_internal*result->height;
      if (!null_bitmap)
	result->body=CAlloc(result->plane_size,task);
      break;
    default:
      throw(EXCEPT_GRAPHICS,1);
  }
  if (null_bitmap)
    result->flags|=DCF_DONT_DRAW;
  result->r=MAlloc(16*sizeof(I64),task);
  DCRst(result);
  result->dc_signature=DCS_SIGNATURE_VAL;
  return result;
}

public U0 DCDel(CDC *dc)
{
  if (!dc) return;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    Dbg;
  dc->dc_signature=0;
  Free(dc->r);
  if (!(dc->flags & DCF_ALIAS))
    Free(dc->body);
  Free(dc->depth_buf);
  Free(dc);
}

public I64 DCSize(CDC *dc)
{
  if (dc)
    return MSize2(dc)+MSize2(dc->body)+MSize2(dc->depth_buf);
  else
    return 0;
}

public U0 DCDepthBufRst(CDC *dc)
{
  if (dc->depth_buf)
    MemSetU32(dc->depth_buf,MAX_I32,dc->width_internal*dc->height);
}

public U0 DCAllocDepthBuf(CDC *dc)
{
  Free(dc->depth_buf);
  dc->depth_buf=MAlloc(dc->width_internal*dc->height*sizeof(I32),dc->mem_task);
  DCDepthBufRst(dc);
}

public CDC *DCCopy(CDC *dc,CTask *task=NULL)
{
  CDC *result;
  if (!dc) return NULL;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    Dbg;
  result=MAllocIdentical(dc,task);
  GrSetRotMat(result,MAllocIdentical(dc->r,task));
  result->mem_task=task;
  result->body=MAllocIdentical(dc->body,task);
  return result;
}

public U0 DCMono(CDC *dc,
  I64 quest=COLOR_TRANSPARENT,I64 true_color=0,I64 false_color=COLOR_MONO)
{
  I64 i;
  U8 *dst;
  if (dc->type==DCT_COLOR8) {
    dst=dc->body;
    i=dc->width_internal*dc->height;
    while (i--)
      if (*dst==quest)
	*dst++=true_color;
      else
	*dst++=false_color;
  }
}

public I64 DCColorChg(CDC *dc,I64 src_color,I64 dst_color=COLOR_TRANSPARENT)
{
  I64 i,result=0;
  U8 *dst;
  if (dc->type==DCT_COLOR8) {
    dst=dc->body;
    i=dc->width_internal*dc->height;
    while (i--)
      if (*dst==src_color) {
	*dst++=dst_color;
	result++;
      } else
	dst++;
  }
  return result;
}

public U8 *DCSave(CDC *dc,I64 *_size=NULL,Bool compressed=TRUE)
{ //stores to mem
  U8 *result,*ptr,*body;
  Bool old_preempt;
  CArcCompress *ac;
  I64 i,j,old_flags=dc->flags;

  if (dc->type==DCT_COLOR8)
    i=dc->plane_size;
  else
    throw(EXCEPT_GRAPHICS,2);
  if (compressed) {
    ac=CompressBuf(dc->body,i);
    j=ac->compressed_size;
    body=ac;
  } else {
    ac=NULL;
    j=i;
    body=dc->body;
  }

  ptr=result=MAlloc(offset(CDC.end_saved_area)-
      offset(CDC.start_saved_area)+j);

  old_preempt=Preempt(OFF); //in case it is a screen bitmap
  if (compressed) dc->flags|=DCF_COMPRESSED;
  dc->flags&=DCF_COMPRESSED;
  MemCpy(result,&dc->start_saved_area,
      offset(CDC.end_saved_area)-
      offset(CDC.start_saved_area));
  dc->flags=old_flags;
  Preempt(old_preempt);

  ptr+=offset(CDC.end_saved_area)-
      offset(CDC.start_saved_area);
  MemCpy(ptr,body,j);
  ptr+=j;

  Free(ac);
  if (_size) *_size=ptr-result;
  return result;
}

public CDC *DCLoad(U8 *src,I64 *_size=NULL,CTask *task=NULL)
{ //loads from mem
  CDC *result;
  I64 i;
  U8 *ptr=src;
  CArcCompress *ac;
  if (!task) task=Fs;
  result=CAlloc(sizeof(CDC),task);
  result->win_task=task;
  result->mem_task=task;
  MemCpy(&result->start_saved_area,src,
      offset(CDC.end_saved_area)-
      offset(CDC.start_saved_area));
  ptr+=offset(CDC.end_saved_area)-
      offset(CDC.start_saved_area);
  if (result->type==DCT_COLOR8)
    i=result->plane_size;
  else
    throw(EXCEPT_GRAPHICS,3);
  if (result->flags&DCF_COMPRESSED) {
    result->flags&=~DCF_COMPRESSED;
    ac=ptr;
    result->body=ExpandBuf(ac,task);
    ptr+=ac->compressed_size;
  } else {
    result->body=MAlloc(i,task);
    MemCpy(result->body,ptr,i);
  }

  result->pen_width=1;
  result->r=GrIdent(task);
  result->r_norm.u32[1]=1;
  result->dc_signature=DCS_SIGNATURE_VAL;
  if (_size) *_size=ptr-src;
  return result;
}

public I64 GRAWrite(U8 *filename,CDC *dc)
{ // TempleOS GRA File
  I64 size;
  U8 *st=DftExt(filename,"GRA.Z"),
     *src=DCSave(dc,&size);
  FileWrite(st,src,size);
  Free(st);
  Free(src);
  return size;
}

public CDC *GRARead(U8 *filename,CTask *task=NULL)
{ // TempleOS GRA File
  CDC *dc=NULL;
  U8 *st=DftExt(filename,"GRA.Z"),
     *src=FileRead(st);
  if (src)
    dc=DCLoad(src,NULL,task);
  Free(src);
  Free(st);
  return dc;
}

#help_index "Graphics/CSprite;Graphics/DCs;Doc/Output"
public U0 DocPutGRAFile(CDoc *doc=NULL,U8 *filename)
{
  CDC *dc=GRARead(filename);
  CSprite *elems=DC2Sprite(dc);
  DocPutSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/DCs;Graphics/Screen"
public CDC *DCCaptureScreen(CTask *task=NULL)
{
  CDC *dc;
  U8 *dst;
  WinMgrSync(2,TRUE);
  dc=DCCopy(gr_screen_image,task);
  if (dc->type==DCT_COLOR4) {
    dc->type=DCT_COLOR8;
    dc->plane_size*=8;
    dst=MAlloc(dc->plane_size,task);
    GrBitMap4ToBitMap8(dst,dc->body,dc->plane_size/2,COLOR_INVALID);
    Free(dc->body);
    dc->body=dst;
  }
  return dc;
}

public I64 GRACaptureScreen(U8 *filename)
{ // TempleOS GRA File
  I64 size;
  CDC *dc=DCCaptureScreen;
  size=GRAWrite(filename,dc);
  DCDel(dc);
  return size;
}

#help_index "Graphics;Graphics/DCs"

public U0 DCExtentsInit(CDC *dc)
{//See $LK-A,"::/TempleOS/Demo/Graphics/Extents.CPP"$
//You should clear the record flag yourself
  dc->flags|=DCF_RECORD_EXTENTS;
  dc->min_x=MAX_I64;
  dc->max_x=MIN_I64;
  dc->min_y=MAX_I64;
  dc->max_y=MIN_I64;
}
