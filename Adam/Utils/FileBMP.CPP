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

public CFileBMP *BMPFile4To(CDC *dc)
{//To Windows 4-bit BMP.
  U8 *src,*ptr;
  I64 x,y,w=dc->width>>1,
    size=offset(CFileBMP.end)+dc->width*dc->height>>1+16*sizeof(CBGR);
  CFileBMP *r=CAlloc(size);
  r->type='BM';
  r->planes=1;
  r->file_size=size;
  r->data_offset=offset(CFileBMP.end);
  r->header_size=offset(CFileBMP.end)-
		 offset(CFileBMP.header_size);
  r->width=dc->width;
  r->height=dc->height;
  r->bit_cnt=4;
  r->image_size=dc->width*dc->height>>1;
  r->data_offset+=16*sizeof(CBGR);
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

public CFileBMP *BMPFileRLE4To(CDC *dc)
{//To Windows RLE4 bit BMP.
  U8 *src,*ptr;
  I64 x,y,w=dc->width,cnt,pattern;
  CFileBMP *r=CAlloc(offset(CFileBMP.end)+2*(dc->width+1)*dc->height+16*sizeof(CBGR));
  r->type='BM';
  r->planes=1;
  r->data_offset=offset(CFileBMP.end);
  r->header_size=offset(CFileBMP.end)-
		 offset(CFileBMP.header_size);
  r->width=dc->width;
  r->height=dc->height;
  r->bit_cnt=4;
  r->compression=2; //RLE4
  r->data_offset+=16*sizeof(CBGR);
  GetVGAPalette4(r->palette);
  ptr=r(U8 *)+r->data_offset;
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
  r->image_size=ptr-(&r->data_offset)(U8 *);
  r->file_size=offset(CFileBMP.end)+r->image_size+16*sizeof(CBGR);
  return r;
}

public CFileBMP *BMPFile24To(CDC *dc)
{//To Windows 24-bit BMP.
  U8 *src;
  I64 i,x,y,size=offset(CFileBMP.end)+dc->width*dc->height*sizeof(CRGB);
  CBGR *bgr;
  CFileBMP *r=CAlloc(size);
  r->type='BM';
  r->planes=1;
  r->file_size=size;
  r->data_offset=offset(CFileBMP.end);
  r->header_size=offset(CFileBMP.end)-
		 offset(CFileBMP.header_size);
  r->width=dc->width;
  r->height=dc->height;
  r->bit_cnt=32;
  r->image_size=dc->width*dc->height<<2;

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
{//Window's BMP Files.
  I64 size;
  CFileBMP *r;
  if (bits==4) {
    if (IsDotZ(filename)) //.Z compression is better than RLE
      r=BMPFile4To(dc);
    else {
      r=BMPFileRLE4To(dc);
      if (r->file_size>offset(CFileBMP.end)+dc->width*dc->height>>1+16*sizeof(CBGR)) {
	Free(r);
	r=BMPFile4To(dc);
      }
    }
  } else if (bits==24)
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
{//Window's BMP Files.
  I64 i,j,cnt;
  U8 *palette_map,*ptr;
  Bool rle4;
  CFileBMP *r;
  CDC *result=NULL;
  if (ptr=FileRead(filename)) {
    r=ptr;
    if (0<r->width<MAX_I32 && 0<r->height<MAX_I32) {
      result=DCNew(r->width,r->height);
      ptr+=r->data_offset;
      if (r->bit_cnt==4) {
	if (r->compression==2)
	  rle4=TRUE;
	else
	  rle4=FALSE;
	if (use_ms_paint_palette)
	  palette_map=ms_paint_palette;
	else
	  palette_map=BMPFilePaletteNew(r);
	for (i=r->height-1;i>=0;i--)
	  if (rle4) {//We don't support full RLE4, just our own subset
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
	    for (j=0;j<(r->width+7)&-8;) {
	      result->color=palette_map[*ptr&15];
	      GrPlot(result,j+1,i);
	      result->color=palette_map[*ptr>>4];
	      GrPlot(result,j,i);
	      ptr++;
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

#help_index "Graphics/Sprite;Graphics/Windows BMP Files;DolDoc/Output"
public U0 DocBMPFile(CDoc *doc=NULL,U8 *filename,Bool use_ms_paint_palette=FALSE)
{//Put a BMP file into a document as a sprite.
  CDC *dc=BMPFileRead(filename,use_ms_paint_palette);
  CSprite *elems=DC2Sprite(dc);
  DocSprite(doc,elems);
  Free(elems);
  DCDel(dc);
}

#help_index "Graphics/Windows BMP Files;Graphics/Screen"
public I64 BMPFileCaptureScreen(U8 *filename,I64 bits=4)
{//Capture screen as BMP file.
  CDC *dc=DCCaptureScreen;
  I64 size=BMPFileWrite(filename,dc,bits);
  DCDel(dc);
  return size;
}
