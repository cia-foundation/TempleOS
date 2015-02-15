#help_index "Graphics/Windows BMP Files"

class CFileBMP
{
  U16 type;
  U32 file_size;
  U32 reserved;
  U32 data_offset;
 
  U32 header_size;
  U32 width;
  U32 height;
  U16 planes;
  U16 bit_cnt;
  U32 compression;
  U32 image_size;
  U32 x_pixs_per_meter;
  U32 y_pixs_per_meter;
  U32 colors_used;
  U32 important_colors;

  U0 end;

  CBGR palette[16];
};

public CFileBMP *BMP4To(CDC *dc)
{//To Windows 4-bit BMP.
  U8 *src,*ptr;
  I64 x,y,w=dc->width>>1,
    size=sizeof(CFileBMP)+dc->width*dc->height>>1;
  CFileBMP *r	=CAlloc(size);
  r->type	='BM';
  r->planes	=1;
  r->file_size	=size;
  r->data_offset=sizeof(CFileBMP);
  r->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  r->width	=dc->width;
  r->height	=dc->height;
  r->bit_cnt	=4;
  r->image_size	=dc->width*dc->height>>1;
  GetVGAPalette4(r->palette);
  ptr=r(U8 *)+r->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    for (x=0;x<w;x++) {
      *ptr|=(*src++&15)<<4;
      *ptr|=*src++&15;
      ptr++;
    }
  }
  return r;
}

public CFileBMP *BMPRLE4To(CDC *dc)
{//To Windows RLE4 bit BMP.
  U8 *src,*ptr,*start;
  I64 x,y,w=dc->width,cnt,pattern;
  CFileBMP *r	=CAlloc(sizeof(CFileBMP)+2*(dc->width+1)*dc->height);
  r->type	='BM';
  r->planes	=1;
  r->data_offset=sizeof(CFileBMP);
  r->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  r->width	=dc->width;
  r->height	=dc->height;
  r->bit_cnt	=4;
  r->compression=2; //RLE4
  GetVGAPalette4(r->palette);
  start=ptr=r(U8 *)+r->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    x=0;
    while (x<w) {
      pattern=(src[0]&15)<<4+src[1]&15;
      if (x+1<w && src[0]&15==src[1]&15) {
	src+=2;
	cnt=2;
	x+=2;
	while (x<w && cnt<MAX_U8) {
	  if (*src&15==pattern&15) {
	    src++;
	    cnt++;
	    x++;
	  } else
	    break;
	}
      } else {
	src+=2;
	if (x+1<w)
	  cnt=2;
	else
	  cnt=1;
	x+=2;
      }
      *ptr++=cnt;
      *ptr++=pattern;
    }
    *ptr(U16 *)++=0;
  }
  r->image_size=ptr-start;
  r->file_size=sizeof(CFileBMP)+r->image_size;
  return r;
}

public CFileBMP *BMP24To(CDC *dc)
{//To Windows 24-bit BMP.
  U8 *src;
  I64 i,x,y,size=offset(CFileBMP.end)+dc->width*dc->height*sizeof(CRGB);
  CBGR *bgr;
  CFileBMP *r	=CAlloc(size);
  r->type	='BM';
  r->planes	=1;
  r->file_size	=size;
  r->data_offset=offset(CFileBMP.end);
  r->header_size=offset(CFileBMP.end)-offset(CFileBMP.header_size);
  r->width	=dc->width;
  r->height	=dc->height;
  r->bit_cnt	=32;
  r->image_size	=dc->width*dc->height<<2;

  bgr=r(U8 *)+r->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    for (x=0;x<dc->width;x++) {
      i=*src++;
      if (i&BLUE)  bgr->b=0x7F;
      if (i&GREEN) bgr->g=0x7F;
      if (i&RED)   bgr->r=0x7F;
      if (i&8) {
	if (bgr->b) bgr->b=0xFF;
	if (bgr->g) bgr->g=0xFF;
	if (bgr->r) bgr->r=0xFF;
      }
      bgr(U8 *)+=4;
    }
  }
  return r;
}

public I64 BMPWrite(U8 *filename,CDC *dc,I64 bits=4)
{//Window's BMP Files.
  I64 size;
  CFileBMP *r;
  if (bits==4) {
    if (IsDotZ(filename)) //.Z compression is better than RLE
      r=BMP4To(dc);
    else {
      r=BMPRLE4To(dc);
      if (r->file_size>sizeof(CFileBMP)+dc->width*dc->height>>1) {
	Free(r);
	r=BMP4To(dc);
      }
    }
  } else if (bits==24)
    r=BMP24To(dc);
  else {
    "Format Not Supported.\n";
    return 0;
  }
  size=r->file_size;
  FileWrite(filename,r,r->file_size);
  Free(r);
  return size;
}

U8 *BMPPaletteNew(CFileBMP *r)
{
  I64 i,j,best,score,best_score;
  CBGR palette[16];
  U8 *result=CAlloc(16*sizeof(U8));
  GetVGAPalette4(palette);
  for (i=0;i<16;i++) {
    best=i;
    best_score=MAX_I64;
    for (j=0;j<16;j++) {
      score=SqrI64(r->palette[i].r-palette[j].r)+
	  SqrI64(r->palette[i].g-palette[j].g)+
	  SqrI64(r->palette[i].b-palette[j].b);
      if (score<best_score) {
	best=j;
	best_score=score;
      }
    }
    result[i]=best;
  }
  return result;
}

U8 ms_paint_palette[16]={0,4,2,6,1,5,3,8,7,12,10,14,9,13,11,15};

I64 BMP24Color(CBGR *ptr,Bool dither_probability)
{
  I64 result,k;
  if (dither_probability) {
    k=RandU32;
    if (SqrI64(ptr->r)+SqrI64(ptr->g)+SqrI64(ptr->b)>=3*SqrI64(k.u8[0]))
      result=8;
    else
      result=0;
    if (ptr->r>=k.u8[1]) result|=RED;
    if (ptr->g>=k.u8[2]) result|=GREEN;
    if (ptr->b>=k.u8[3]) result|=BLUE;
  } else {
    if (SqrI64(ptr->r)+SqrI64(ptr->g)+SqrI64(ptr->b)>=SqrI64(0x80)) {
      result=8;
      if (ptr->r>=0x80) result|=RED;
      if (ptr->g>=0x80) result|=GREEN;
      if (ptr->b>=0x80) result|=BLUE;
    } else {
      result=0;
      if (ptr->r>=0x40) result|=RED;
      if (ptr->g>=0x40) result|=GREEN;
      if (ptr->b>=0x40) result|=BLUE;
    }
  }
  return result;
}

public CDC *BMPRead(U8 *filename,Bool dither_probability=FALSE,Bool use_ms_paint_palette=FALSE)
{//Window's BMP Files.
  I64 i,j,cnt;
  U8 *palette_map,*ptr;
  Bool rle;
  CFileBMP *r;
  CDC *result=NULL;
  if (ptr=FileRead(filename)) {
    r=ptr;
    if (0<r->width<MAX_I32 && 0<r->height<MAX_I32) {
      result=DCNew(r->width,r->height);
      ptr+=r->data_offset;
      if (r->compression==2)
	rle=TRUE;
      else
	rle=FALSE;
      if (use_ms_paint_palette)
	palette_map=ms_paint_palette;
      else
	palette_map=BMPPaletteNew(r);
      if (r->bit_cnt==4) {
	for (i=r->height-1;i>=0;i--)
	  if (rle) {//We don't support full RLE4, just our own subset
	    j=0;
	    while (cnt=*ptr++) {
	      if (cnt==1) {
		result->color=palette_map[*ptr++&15];
		GrPlot(result,j++,i);
	      } else {
		if (cnt==2 && *ptr>>4!=*ptr&15) {
		  result->color=palette_map[*ptr&15];
		  GrPlot(result,j+1,i);
		  result->color=palette_map[*ptr>>4];
		  GrPlot(result,j,i);
		  ptr++;
		  j+=2;
		} else {
		  result->color=palette_map[*ptr++&15];
		  while (cnt--)
		    GrPlot(result,j++,i);
		}
	      }
	    }
	    ptr++;
	  } else
	    for (j=0;j<(r->width+7)&~7;) {
	      result->color=palette_map[*ptr&15];
	      GrPlot(result,j+1,i);
	      result->color=palette_map[*ptr>>4];
	      GrPlot(result,j,i);
	      ptr++;
	      j+=2;
	    }
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else if (r->bit_cnt==24) {
	for (i=r->height-1;i>=0;i--) {
	  for (j=0;j<r->width;j++,ptr+=3) {
	    result->color=BMP24Color(ptr,dither_probability);
	    GrPlot(result,j,i);
	  }
	  ptr+=r->width&3;
	}
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else if (r->bit_cnt>=32) {
	for (i=r->height-1;i>=0;i--)
	  for (j=0;j<r->width;j++,ptr+=4) {
	    result->color=BMP24Color(ptr,dither_probability);
	    GrPlot(result,j,i);
	  }
	if (!use_ms_paint_palette)
	  Free(palette_map);
      } else {
	"Format Not Supported.\n";
	DCDel(result);
	result=NULL;
      }
    } else
      "Invalid BMP File\n";
    Free(r);
  }
  return result;
}

#help_index "Graphics/Sprite;Graphics/Windows BMP Files;DolDoc/Output"
public U0 DocBMP(CDoc *doc=NULL,U8 *filename,Bool dither_probability=FALSE,Bool use_ms_paint_palette=FALSE)
{//Put a BMP file into a document as a sprite.
  CDC *dc=BMPRead(filename,dither_probability,use_ms_paint_palette);
  CSprite *elems=DC2Sprite(dc);
  DocSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Windows BMP Files;Graphics/Screen"
public I64 BMPCaptureScreen(U8 *filename,I64 bits=4,Bool include_zoom=TRUE)
{//Capture screen as BMP file.
  CDC *dc=DCCaptureScreen(include_zoom);
  I64 size=BMPWrite(filename,dc,bits);
  DCDel(dc);
  return size;
}
