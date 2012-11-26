#help_index "Graphics/Windows BMP Files"

//This provides support for Window's BMP files.

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
  U32 x_pixels_per_meter;
  U32 y_pixels_per_meter;
  U32 colors_used;
  U32 important_colors;

  U0 end;

  CBGR palette[16];
};


CFileBMP *BMPFileNew(I64 width,I64 height,I64 bits)
{
  CFileBMP *r;
  I64 size;
  if (bits==24)
    size=offset(CFileBMP.end)+
	 width*height*sizeof(CRGB);
  else if (bits==4)
    size=offset(CFileBMP.end)+
	 width*height>>1+16*sizeof(CBGR);
  else {
    "Format Not Supported.\n";
    return NULL;
  }
  r=CAlloc(size);
  r->type=0x4D42;
  r->planes=1;
  r->file_size=size;
  r->data_offset=offset(CFileBMP.end);
  r->header_size=offset(CFileBMP.end)-
		 offset(CFileBMP.header_size);
  r->width=width;
  r->height=height;
  if (bits==24) {
    r->bit_cnt=32;
    r->image_size=width*height<<2;
  } else {
    r->bit_cnt=4;
    r->image_size=width*height>>1;
    r->data_offset+=16*sizeof(CBGR);
    GetVGAPalette4(r->palette);
  }
  return r;
}


public CFileBMP *BMPFile4To(CDC *dc)
{
//To Windows 4 bit BMP
  U8 *src;
  CFileBMP *r=BMPFileNew(dc->width,dc->height,4);
  I64 x,y,w=dc->width>>1;
  U8 *bgr;
  bgr=r(U8 *)+r->data_offset;
  for (y=dc->height-1;y>=0;y--) {
    src=y*dc->width_internal+dc->body;
    for (x=0;x<w;x++) {
      *bgr|=(*src++&15)<<4;
      *bgr|=*src++&15;
      bgr++;
    }
  }
  return r;
}

public CFileBMP *BMPFile24To(CDC *dc)
{
//To Windows 24 bit BMP
  U8 *src;
  CFileBMP *r=BMPFileNew(dc->width,dc->height,24);
  I64 i,x,y;
  CBGR *bgr;

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

public I64 BMPFileWrite(U8 *filename,CDC *dc,I64 bits=4)
{ //Window's BMP Files
  I64 size;
  CFileBMP *r;
  if (bits==4)
    r=BMPFile4To(dc);
  else if (bits==24)
    r=BMPFile24To(dc);
  else {
    "Format Not Supported.\n";
    return 0;
  }
  size=r->file_size;
  FileWrite(filename,r,r->file_size);
  Free(r);
  return size;
}

U8 *BMPFilePaletteNew(CFileBMP *r)
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

public CDC *BMPFileRead(U8 *filename,Bool use_ms_paint_palette=FALSE)
{ //Window's BMP Files
  I64 i,j;
  U8 *palette_map,*ptr1;
  CFileBMP *r;
  CDC *result=NULL;
  if (ptr1=FileRead(filename)) {
    r=ptr1;
    if (0<r->width<MAX_I32 && 0<r->height<MAX_I32) {
      result=DCNew(DCT_COLOR8,r->width,r->height);
      ptr1+=r->data_offset;
      if (r->bit_cnt==4) {
	if (use_ms_paint_palette)
	  palette_map=ms_paint_palette;
	else
	  palette_map=BMPFilePaletteNew(r);
	for (i=r->height-1;i>=0;i--)
	  for (j=0;j<(r->width+7)&-8;) {
	    result->color=palette_map[*ptr1&15];
	    GrPlot(result,j+1,i);
	    result->color=palette_map[*ptr1>>4];
	    GrPlot(result,j,i);
	    ptr1++;
	    j+=2;
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

#help_index "Graphics/CSprite;Graphics/Windows BMP Files;Doc/Output"
public U0 DocPutBMPFile(CDoc *doc=NULL,U8 *fname,Bool use_ms_paint_palette=FALSE)
{
  CDC *dc=BMPFileRead(fname,use_ms_paint_palette);
  CSprite *elems=DC2Sprite(dc);
  DocPutSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Windows BMP Files;Graphics/Screen"
public I64 BMPFileCaptureScreen(U8 *filename,I64 bits=4)
{
  CDC *dc=DCCaptureScreen;
  I64 size=BMPFileWrite(filename,dc,bits);
  DCDel(dc);
  return size;
}

#help_index ""
