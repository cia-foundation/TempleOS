#help_index "Graphics/Math/3D Transformation"
#help_file "::/Doc/Transform"

#define	GR_SCALE	(1<<32)

public U0 Mat4x4MulXYZ(I64 *r,I64 *_x,I64 *_y,I64 *_z)
{//Rotate 3D point using 4x4 matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  I64 x1,y1,z1,xx=*_x,yy=*_y,zz=*_z;
  x1=(r[0*4+0]*xx+r[0*4+1]*yy+r[0*4+2]*zz+r[0*4+3])>>32;
  y1=(r[1*4+0]*xx+r[1*4+1]*yy+r[1*4+2]*zz+r[1*4+3])>>32;
  z1=(r[2*4+0]*xx+r[2*4+1]*yy+r[2*4+2]*zz+r[2*4+3])>>32;
  *_x=x1;*_y=y1;*_z=z1;
}

public U0 DCTransform(CDC *dc,I64 *_x,I64 *_y,I64 *_z)
{//This is the dft dc->transform() callback.
//Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  Mat4x4MulXYZ(dc->r,_x,_y,_z);
  *_x+=dc->x;
  *_y+=dc->y;
  *_z+=dc->z;
}

public I64 *Mat4x4IdentEqu(I64 *r)
{//Set matrix to identity. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  MemSet(r,0,sizeof(I64)*16);
  r[0*4+0].i32[1]=1;
  r[1*4+1].i32[1]=1;
  r[2*4+2].i32[1]=1;
  r[3*4+3].i32[1]=1;
  return r;
}

public I64 *Mat4x4IdentNew(CTask *mem_task=NULL)
{//MAlloc an identity matrix. Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
  return Mat4x4IdentEqu(MAlloc(sizeof(I64)*16,mem_task));
}

public I64 Mat4x4NormSqr65536(I64 *r)
{//Norm Squared of r.
//(1.0/Sqrt(3))*65536=37837.22
  return SqrI64((r[0*4+0]*37838+r[0*4+1]*37838+r[0*4+2]*37838)>>32)+
	SqrI64((r[1*4+0]*37837+r[1*4+1]*37837+r[1*4+2]*37837)>>32)+
	SqrI64((r[2*4+0]*37837+r[2*4+1]*37837+r[2*4+2]*37837)>>32);
}

public U0 DCMat4x4Set(CDC *dc=NULL,I64 *r)
{//Set device context's rot matrix. Will be $LK,"Free",A="MN:Free"$d() in $LK,"DCDel",A="MN:DCDel"$().Uses $LK,"fixed-point",A="FI:::/Demo/Lectures/FixedPoint.HC"$.
//The main purpose is to set matrix norm for thick scaling.
  //NULL as dc means gr.dc
  if (!dc)   dc=gr.dc;
  dc->r=r;
  dc->r_norm=Sqrt(Mat4x4NormSqr65536(r))*65536; //scaled 32 bits
}

#help_index "Graphics/Mesh"
public U0 DCLighting(CDC *dc,
	CD3I32 *p1,CD3I32 *p2,CD3I32 *p3,CColorROPU32 color)
{//This is the dft dc->lighting() callback.
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
  //Probability dithering does not work with thick>1 at this time.
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
  dc->thick=1;
  dc->ls.x=37837; //1<<16/Sqrt(3)
  dc->ls.y=37837;
  dc->ls.z=37837;
  dc->x=0;
  dc->y=0;
  dc->z=0;
  dc->transform=&DCTransform;
  dc->lighting =&DCLighting;
  Mat4x4IdentEqu(dc->r);
  dc->r_norm=GR_SCALE;
  dc->flags&=~(DCF_SYMMETRY|DCF_TRANSFORMATION|DCF_JUST_MIRROR);
  MemCpy(dc->palette,gr_palette_std,sizeof(CBGR48)*COLORS_NUM);
}

public U0 DCExtentsInit(CDC *dc=NULL)
{//Init markers for extent of next newly drawn graphics.
//NULL means gr.dc
  //See $LK,"::/Demo/Graphics/Extents.HC"$
  //You should clear the record flag yourself
  if (!dc)   dc=gr.dc;
  dc->flags|=DCF_RECORD_EXTENTS;
  dc->min_x=I64_MAX;
  dc->max_x=I64_MIN;
  dc->min_y=I64_MAX;
  dc->max_y=I64_MIN;
}

public CDC *DCAlias(CDC *dc=NULL,CTask *task=NULL)
{//Create alias of dc, so can change pen, color, etc.
//NULL means gr.dc
  CDC *res;
  if (!dc)   dc=gr.dc;
  if (!task) task=Fs;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    throw('Graphics');
  res=MAlloc(sizeof(CDC),task);
  MemCpy(res,dc,sizeof(CDC));
  res->win_task=res->mem_task=task;
  res->r=MAlloc(16*sizeof(I64),task);
  DCRst(res);
  res->flags|=DCF_ALIAS;
  res->alias=dc;
  return res;
}

public CDC *DCNew(I64 width,I64 height,CTask *task=NULL,Bool null_bitmap=FALSE)
{//Create new width x height device context.
//Internally only allows widths which are divisible by 8.
  //Don't forget these $MA-X+PU,"sizeof(CDC)",LM="Find(\"sizeof(CDC)\",\"/*\");View;"$.
  CDC *res;
  if (!task) task=Fs;
  res=CAlloc(sizeof(CDC),task);
  res->win_task=task;
  res->mem_task=task;
  res->width=width;
  res->width_internal=(width+7)&~7;
  res->height=height;
  if (null_bitmap)
    res->flags|=DCF_DONT_DRAW;
  else
    res->body=CAlloc(res->width_internal*res->height,task);
  res->r=MAlloc(16*sizeof(I64),task);
  DCRst(res);
  res->dc_signature=DCS_SIGNATURE_VAL;
  return res;
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

public I32 *DCDepthBufRst(CDC *dc)
{//Reset device context depth buf to far away.
  if (dc->depth_buf)
    MemSetU32(dc->depth_buf,I32_MAX,dc->width_internal*dc->height);
  return dc->depth_buf;
}

public I32 *DCDepthBufAlloc(CDC *dc)
{//Alloc a 32-bit depth buffer for device context.
  Free(dc->depth_buf);
  dc->depth_buf=MAlloc(dc->width_internal*dc->height*sizeof(I32),dc->mem_task);
  return DCDepthBufRst(dc);
}

public CDC *DCCopy(CDC *dc,CTask *task=NULL)
{//Alloc copy of dc, including image body, rot mat and depth buf.
  CDC *res;
  if (!dc) return NULL;
  if (dc->dc_signature!=DCS_SIGNATURE_VAL)
    throw('Graphics');
  res=MAllocIdent(dc,task);
  DCMat4x4Set(res,Mat4x4New(dc->r,task));
  res->mem_task=task;
  res->body=MAllocIdent(dc->body,task);
  res->depth_buf=MAllocIdent(dc->depth_buf,task);
  return res;
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
  I64 i,res=0;
  U8 *dst;
  dst=dc->body;
  i=dc->width_internal*dc->height;
  while (i--)
    if (*dst==src_color) {
      *dst++=dst_color;
      res++;
    } else
      dst++;
  return res;
}

public U8 *DCSave(CDC *dc,I64 *_size=NULL,I64 dcsf_flags=DCSF_COMPRESSED)
{//Stores device context to mem, perhaps, with compression.
  U8 *res,*ptr,*body;
  CArcCompress *arc;
  I64 body_size=dc->width_internal*dc->height,total_size,flags;
  CBGR48 palette[COLORS_NUM];

  if (dcsf_flags&DCSF_COMPRESSED) {
    arc=CompressBuf(dc->body,body_size);
    body_size=arc->compressed_size;
    body=arc;
  } else {
    arc=NULL;
    body=dc->body;
  }

  total_size=offset(CDC.end)-offset(CDC.start)+body_size;
  if (dcsf_flags&DCSF_COMPRESSED)
    flags=DCF_COMPRESSED;
  else
    flags=0;

  if (dcsf_flags&DCSF_PALETTE_GET)
    GrPaletteGet(palette);
  else
    MemCpy(palette,&dc->palette,COLORS_NUM*sizeof(CBGR48));
  if (MemCmp(palette,gr_palette_std,COLORS_NUM*sizeof(CBGR48))) {
    flags|=DCF_PALETTE;
    total_size+=COLORS_NUM*sizeof(CBGR48);
  }

  ptr=res=MAlloc(total_size);

#assert !offset(CDC.start)
  MemCpy(ptr,&dc->start,offset(CDC.end)-offset(CDC.start));
  ptr(CDC *)->flags=flags;
  ptr+=offset(CDC.end)-offset(CDC.start);

#assert offset(CDC.end)==offset(CDC.palette)
  if (flags&DCF_PALETTE) {
    MemCpy(ptr,palette,COLORS_NUM*sizeof(CBGR48));
    ptr+=COLORS_NUM*sizeof(CBGR48);
  }

  MemCpy(ptr,body,body_size);
  ptr+=body_size;

  Free(arc);
  if (_size) *_size=total_size;
  return res;
}

public CDC *DCLoad(U8 *src,I64 *_size=NULL,CTask *task=NULL)
{//Loads device context from mem.
  CDC *res;
  U8 *ptr=src;
  CArcCompress *arc;
  I64 body_size;
  if (!task) task=Fs;
  res=CAlloc(sizeof(CDC),task);
  res->win_task=task;
  res->mem_task=task;
  MemCpy(&res->start,ptr,offset(CDC.end)-offset(CDC.start));
  ptr+=offset(CDC.end)-offset(CDC.start);

  if (res->flags&DCF_PALETTE) {
    MemCpy(&res->palette,ptr,COLORS_NUM*sizeof(CBGR48));
    ptr+=COLORS_NUM*sizeof(CBGR48);
  } else
    MemCpy(&res->palette,gr_palette_std,COLORS_NUM*sizeof(CBGR48));

  body_size=res->width_internal*res->height;
  if (res->flags&DCF_COMPRESSED) {
    res->flags&=~DCF_COMPRESSED;
    arc=ptr;
    res->body=ExpandBuf(arc,task);
    ptr+=arc->compressed_size;
  } else {
    res->body=MAlloc(body_size,task);
    MemCpy(res->body,ptr,body_size);
    ptr+=body_size;
  }
  res->thick=1;
  res->r=Mat4x4IdentNew(task);
  res->r_norm.u32[1]=1;
  res->dc_signature=DCS_SIGNATURE_VAL;
  if (_size) *_size=ptr-src;
  return res;
}

#help_index "Graphics/GR Files"
#help_file "::/Doc/GRFiles"
#help_index "Graphics/Device Contexts;Graphics/GR Files"

#define GR_FILE_MAX	(offset(CDC.end)-offset(CDC.start)+\
	COLORS_NUM*sizeof(CBGR48)+sizeof(CArcCtrl)+GR_WIDTH*GR_HEIGHT)

public I64 GRWrite(U8 *filename,CDC *dc,I64 dcsf_flags=DCSF_COMPRESSED)
{//TempleOS GR File.
  I64	size;
  U8	*st=ExtDft(filename,"GR.Z"),
	*src=DCSave(dc,&size,dcsf_flags);
  FileWrite(st,src,size);
  Free(st);
  Free(src);
  return size;
}

public CDC *GRRead(U8 *filename,CTask *task=NULL)
{//TempleOS GR File.
  CDC	*dc=NULL;
  U8	*st=ExtDft(filename,"GR.Z"),
	*src=FileRead(st);
  if (src)
    dc=DCLoad(src,,task);
  Free(src);
  Free(st);
  return dc;
}

#help_index "Graphics/Sprite;Graphics/GR Files;DolDoc/Output;StdOut/DolDoc"
public U0 DocGR(CDoc *doc=NULL,U8 *filename)
{//Put a GR file into a document as asprite.
  CDC *dc=GRRead(filename);
  CSprite *elems=DC2Sprite(dc);
  DocSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Device Contexts;Graphics/Scrn"
public CDC *DCScrnCapture(Bool include_zoom=TRUE,CTask *task=NULL)
{//Capture scrn to a device context.
  CDC *dc;
  U8 *dst;
  Refresh(0,FALSE);
  if (include_zoom)
    dc=DCCopy(gr.scrn_image,task);
  else
    dc=DCCopy(gr.dc1,task);
  dc->flags&=~DCF_SCRN_BITMAP;
  dst=MAlloc(dc->width_internal*dc->height,task);
//Pick background color that never occurs. COLOR_INVALID
  GrBitMap4ToBitMap8(dst,dc->body,
	(dc->width_internal*dc->height)>>1,COLOR_INVALID);
  Free(dc->body);
  dc->body=dst;
  return dc;
}
