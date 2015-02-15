#help_index "Graphics/Math/3D Transformation"
#help_file "::/Doc/Transform"

#define	GR_SCALE	(1<<32)

public U0 GrRot(I64 r[4][4],I64 *_x,I64 *_y,I64 *_z)
{//Rotate 3D point using 4x4 matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
  I64 x1,y1,z1,xx=*_x,yy=*_y,zz=*_z;
  x1=(r[0][0]*xx+r[0][1]*yy+r[0][2]*zz+r[0][3])>>32;
  y1=(r[1][0]*xx+r[1][1]*yy+r[1][2]*zz+r[1][3])>>32;
  z1=(r[2][0]*xx+r[2][1]*yy+r[2][2]*zz+r[2][3])>>32;
  *_x=x1;*_y=y1;*_z=z1;
}

public U0 GrTransform(CDC *dc,I64 *_x,I64 *_y,I64 *_z)
{//This is a callback.
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
  GrRot(dc->r,_x,_y,_z);
  *_x+=dc->x;
  *_y+=dc->y;
  *_z+=dc->z;
}

public I64 *GrIdentNew(CTask *mem_task=NULL)
{//MAlloc an identity matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
  I64 *r=CAlloc(sizeof(I64)*16,mem_task);
  r[0] .i32[1]=1;
  r[5] .i32[1]=1;
  r[10].i32[1]=1;
  r[15].i32[1]=1;
  return r;
}

public I64 *GrIdentEqu(I64 r[4][4])
{//Set matrix to identity. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
  MemSet(r,0,sizeof(I64)*16);
  r[0][0].i32[1]=1;
  r[1][1].i32[1]=1;
  r[2][2].i32[1]=1;
  r[3][3].i32[1]=1;
  return r;
}

public I64 GrRotNormSqr65536(I64 r[4][4])
{//Norm Squared of r.
//(1.0/Sqrt(3))*65536=37837.22
  return SqrI64((r[0][0]*37838+r[0][1]*37838+r[0][2]*37838)>>32)+
	 SqrI64((r[1][0]*37837+r[1][1]*37837+r[1][2]*37837)>>32)+
	 SqrI64((r[2][0]*37837+r[2][1]*37837+r[2][2]*37837)>>32);
}

public U0 GrSetRotMat(CDC *dc=NULL,I64 r[4][4])
{//Set device context's rot matrix. Will be $LK,"Free",A="MN:Free"$d() in $LK,"DCDel",A="MN:DCDel"$(). Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.CPP"$.
//The main purpose is to set matrix norm for pen width scaling.
//NULL as dc means gr.dc
  if (!dc)   dc=gr.dc;
  dc->r=r;
  dc->r_norm=Sqrt(GrRotNormSqr65536(r))*65536; //scaled 32 bits
}

#help_index "Graphics/Mesh"
public U0 GrLighting(CDC *dc,CD3I32 *p1,CD3I32 *p2,CD3I32 *p3,CColorROPU32 color)
{//This is a callback.
  CD3I32 v1,v2;
  I64 i,vn_x,vn_y,vn_z;
  F64 d;

  v1.x=p1->x-p2->x;
  v1.y=p1->y-p2->y;
  v1.z=p1->z-p2->z;

  v2.x=p3->x-p2->x;
  v2.y=p3->y-p2->y;
  v2.z=p3->z-p2->z;

//V1 and V2 are vects along two sides
//of the tri joined at p2.

  vn_x=v1.y*v2.z-v1.z*v2.y;
  vn_y=v1.z*v2.x-v1.x*v2.z;
  vn_z=v1.x*v2.y-v1.y*v2.x;
  if (d=Sqrt(SqrI64(vn_x)+SqrI64(vn_y)+SqrI64(vn_z)))
    d=1<<16/d;
  vn_x*=d;
  vn_y*=d;
  vn_z*=d;
//Vn is the cross product of V1 and V3
//which means it is perpendicular.  It
//is the normal vect to the surface.
//It has been scaled to length 65536.

//Light source has been scaled to length 65536.
  i=(vn_x*dc->ls.x+vn_y*dc->ls.y+vn_z*dc->ls.z)>>16;
//The dot product of the light source
//vect and the surface normal
//gives an illumination number.
//65536*65536>>16=65536

//TempleOS will generate a random U16
//and compare to dither_probability_u16 and
//will pick from two colors.
//Probability dithering does not work with pen_width>1 at this time.
  if (color.c0.rop&ROPBF_TWO_SIDED) {
    color.c0.rop&=~ROPBF_TWO_SIDED;
    i=AbsI64(i)<<1;
  } else
    i+=65536;
  if (color.c0.rop&ROPBF_HALF_RANGE_COLOR) {
    color.c0.rop&=~ROPBF_HALF_RANGE_COLOR;
    i>>=1;
    if (color>=8) {
      color-=8;
      i+=65536;
    }
  }
  if (i<65536) {
    dc->color=ROPF_PROBABILITY_DITHER+color<<16+BLACK;
    dc->dither_probability_u16=i;
  } else {
    dc->color=ROPF_PROBABILITY_DITHER+(color^8)<<16+color;
    dc->dither_probability_u16=i-65536;
  }
}

#help_index "Graphics/Device Contexts"
public U0 DCFill(CDC *dc=NULL,CColorROPU32 val=TRANSPARENT)
{//Fill entire device context with color.
  if (!dc) dc=gr.dc;
  MemSet(dc->body,val,dc->width_internal*dc->height);
}

public U0 DCClear(CDC *dc=NULL)
{//Set entire device context image body to 0 (BLACK).
  if (!dc) dc=gr.dc;
  DCFill(dc,0);
}

public U0 DCRst(CDC *dc)
{//Reset $LK,"CDC",A="MN:CDC"$ structure members but not image body, itself.
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
  dc->r_norm=GR_SCALE;
  dc->flags&=~(DCF_SYMMETRY|DCF_TRANSFORMATION|DCF_JUST_MIRROR);
}

public U0 DCExtentsInit(CDC *dc=NULL)
{//Init markers for extent of next newly drawn graphics.
//NULL means gr.dc
//See $LK,"::/Demo/Graphics/Extents.CPP"$
//You should clear the record flag yourself
  if (!dc)   dc=gr.dc;
  dc->flags|=DCF_RECORD_EXTENTS;
  dc->min_x=MAX_I64;
  dc->max_x=MIN_I64;
  dc->min_y=MAX_I64;
  dc->max_y=MIN_I64;
}

public CDC *DCAlias(CDC *dc=NULL,CTask *task=NULL)
{//Create alias of dc, so can change pen, color, etc.
//NULL means gr.dc
  CDC *result;
  if (!dc)   dc=gr.dc;
  if (!task) task=Fs;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    throw('Graphics');
  result=MAlloc(sizeof(CDC),task);
  MemCpy(result,dc,sizeof(CDC));
  result->win_task=result->mem_task=task;
  result->r=MAlloc(16*sizeof(I64),task);
  DCRst(result);
  result->flags|=DCF_ALIAS;
  result->alias=dc;
  return result;
}

public CDC *DCNew(I64 width,I64 height,CTask *task=NULL,Bool null_bitmap=FALSE)
{//Create new width x height device context.
//Internally only allows widths which are divisible by 8.
//Don't forget these $MA-X+PU,"sizeof(CDC)",LM="Find(\"sizeof(CDC)\",\"/*\");View;"$.
  CDC *result;
  if (!task) task=Fs;
  result=CAlloc(sizeof(CDC),task);
  result->win_task=task;
  result->mem_task=task;
  result->width=width;
  result->width_internal=(width+7)&~7;
  result->height=height;
  if (null_bitmap)
    result->flags|=DCF_DONT_DRAW;
  else
    result->body=CAlloc(result->width_internal*result->height,task);
  result->r=MAlloc(16*sizeof(I64),task);
  DCRst(result);
  result->dc_signature=DCS_SIGNATURE_VAL;
  return result;
}

public U0 DCDel(CDC *dc)
{//Free dc, image body, rot mat and depth buf.
  if (!dc) return;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    throw('Graphics');
  dc->dc_signature=0;
  Free(dc->r);
  if (!(dc->flags & DCF_ALIAS))
    Free(dc->body);
  Free(dc->depth_buf);
  Free(dc);
}

public I64 DCSize(CDC *dc)
{//Mem size of header, image body and depth buffer.
  if (dc)
    return MSize2(dc)+MSize2(dc->body)+MSize2(dc->depth_buf);
  else
    return 0;
}

public U0 DCDepthBufRst(CDC *dc)
{//Reset device context depth buf to far away.
  if (dc->depth_buf)
    MemSetU32(dc->depth_buf,MAX_I32,dc->width_internal*dc->height);
}

public U0 DCAllocDepthBuf(CDC *dc)
{//Alloc a 32-bit depth buffer for device context.
  Free(dc->depth_buf);
  dc->depth_buf=MAlloc(dc->width_internal*dc->height*sizeof(I32),dc->mem_task);
  DCDepthBufRst(dc);
}

public CDC *DCCopy(CDC *dc,CTask *task=NULL)
{//Alloc copy of dc, including image body, rot mat and depth buf.
  CDC *result;
  if (!dc) return NULL;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    throw('Graphics');
  result=MAllocIdentical(dc,task);
  GrSetRotMat(result,MAllocIdentical(dc->r,task));
  result->mem_task=task;
  result->body=MAllocIdentical(dc->body,task);
  result->depth_buf=MAllocIdentical(dc->depth_buf,task);
  return result;
}

public U0 DCMono(CDC *dc,
  I64 quest=TRANSPARENT,I64 true_color=0,I64 false_color=COLOR_MONO)
{//Set entire device context to one of two colors.
  I64 i;
  U8 *dst;
  dst=dc->body;
  i=dc->width_internal*dc->height;
  while (i--)
    if (*dst==quest)
      *dst++=true_color;
    else
      *dst++=false_color;
}

public I64 DCColorChg(CDC *dc,I64 src_color,I64 dst_color=TRANSPARENT)
{//Find and replace src color with dst in device context.
  I64 i,result=0;
  U8 *dst;
  dst=dc->body;
  i=dc->width_internal*dc->height;
  while (i--)
    if (*dst==src_color) {
      *dst++=dst_color;
      result++;
    } else
      dst++;
  return result;
}

public U8 *DCSave(CDC *dc,I64 *_size=NULL,Bool compressed=TRUE)
{//Stores device context to mem, perhaps, with compression.
  U8 *result,*ptr,*body;
  Bool old_preempt;
  CArcCompress *ac;
  I64 i=dc->width_internal*dc->height,old_flags=dc->flags;

  if (compressed) {
    ac=CompressBuf(dc->body,i);
    i=ac->compressed_size;
    body=ac;
  } else {
    ac=NULL;
    body=dc->body;
  }

  ptr=result=MAlloc(offset(CDC.end)-offset(CDC.start)+i);

  old_preempt=Preempt(OFF); //in case it is a screen bitmap
  if (compressed) dc->flags|=DCF_COMPRESSED;
  dc->flags&=DCF_COMPRESSED;
  MemCpy(result,&dc->start,offset(CDC.end)-offset(CDC.start));
  dc->flags=old_flags;
  Preempt(old_preempt);

  ptr+=offset(CDC.end)-offset(CDC.start);
  MemCpy(ptr,body,i);
  ptr+=i;

  Free(ac);
  if (_size) *_size=ptr-result;
  return result;
}

public CDC *DCLoad(U8 *src,I64 *_size=NULL,CTask *task=NULL)
{//Loads device context from mem.
  CDC *result;
  U8 *ptr=src;
  CArcCompress *ac;
  I64 body_size;
  if (!task) task=Fs;
  result=CAlloc(sizeof(CDC),task);
  result->win_task=task;
  result->mem_task=task;
  MemCpy(&result->start,src,
      offset(CDC.end)-offset(CDC.start));
  ptr+=offset(CDC.end)-offset(CDC.start);
  body_size=result->width_internal*result->height;
  if (result->flags&DCF_COMPRESSED) {
    result->flags&=~DCF_COMPRESSED;
    ac=ptr;
    result->body=ExpandBuf(ac,task);
    ptr+=ac->compressed_size;
  } else {
    result->body=MAlloc(body_size,task);
    MemCpy(result->body,ptr,body_size);
    ptr+=body_size;
  }
  result->pen_width=1;
  result->r=GrIdentNew(task);
  result->r_norm.u32[1]=1;
  result->dc_signature=DCS_SIGNATURE_VAL;
  if (_size) *_size=ptr-src;
  return result;
}

#help_index "Graphics/GRA Files"
#help_file "::/Doc/GRAFiles"
#help_index "Graphics/Device Contexts;Graphics/GRA Files"
public I64 GRAWrite(U8 *filename,CDC *dc)
{//TempleOS GRA File.
  I64 size;
  U8 *st=DftExt(filename,"GRA.Z"),
     *src=DCSave(dc,&size);
  FileWrite(st,src,size);
  Free(st);
  Free(src);
  return size;
}

public CDC *GRARead(U8 *filename,CTask *task=NULL)
{//TempleOS GRA File.
  CDC *dc=NULL;
  U8 *st=DftExt(filename,"GRA.Z"),
     *src=FileRead(st);
  if (src)
    dc=DCLoad(src,,task);
  Free(src);
  Free(st);
  return dc;
}

#help_index "Graphics/Sprite;Graphics/GRA Files;DolDoc/Output"
public U0 DocGRA(CDoc *doc=NULL,U8 *filename)
{//Put a GRA file into a document as a	sprite.
  CDC *dc=GRARead(filename);
  CSprite *elems=DC2Sprite(dc);
  DocSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Device Contexts;Graphics/Screen"
public CDC *DCCaptureScreen(Bool include_zoom=TRUE,CTask *task=NULL)
{//Capture screen to a device context.
  CDC *dc;
  U8 *dst;
  WinMgrSync(2,TRUE);
  if (include_zoom)
    dc=DCCopy(gr.screen_image,task);
  else
    dc=DCCopy(gr.dc1,task);
  dc->flags&=~DCF_SCREEN_BITMAP;
  dst=MAlloc(dc->width_internal*dc->height,task);
  //Pick background color that never occurs. COLOR_INVALID
  GrBitMap4ToBitMap8(dst,dc->body,(dc->width_internal*dc->height)>>1,COLOR_INVALID);
  Free(dc->body);
  dc->body=dst;
  return dc;
}

#help_index "Graphics/GRA Files;Graphics/Screen"
public I64 GRACaptureScreen(U8 *filename,Bool include_zoom=TRUE)
{//Capture screen to a TempleOS GRA File.
  I64 size;
  CDC *dc=DCCaptureScreen(include_zoom);
  size=GRAWrite(filename,dc);
  DCDel(dc);
  return size;
}
